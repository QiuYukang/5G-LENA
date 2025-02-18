// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-pm-search-maleki.h"

#include "nr-cb-type-one-sp.h"
#include "pybind11/numpy.h"
#include "pybind11/pybind11.h"

#include "ns3/angles.h"

#include <algorithm>
#include <pybind11/embed.h>

namespace py = pybind11;

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPmSearchMaleki");
NS_OBJECT_ENSURE_REGISTERED(NrPmSearchMaleki);

TypeId
NrPmSearchMaleki::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrPmSearchMaleki")
                            .SetParent<NrPmSearchFull>()
                            .AddConstructor<NrPmSearchMaleki>();
    return tid;
}

NrPmSearchMaleki::NrPmSearchMaleki()
    : NrPmSearchFull()
{
}

py::scoped_interpreter* g_interpreter = nullptr;

PmCqiInfo
NrPmSearchMaleki::CreateCqiFeedbackMimo(const NrMimoSignal& rxSignalRb, PmiUpdate pmiUpdate)
{
    NS_LOG_FUNCTION(this);

    // Extract parameters from received signal
    auto nRows = rxSignalRb.m_chanMat.GetNumRows();
    auto nCols = rxSignalRb.m_chanMat.GetNumCols();
    NS_ASSERT_MSG(nRows == m_nRxPorts, "Channel mat has {} rows but UE has {} ports");
    NS_ASSERT_MSG(nCols == m_nGnbPorts, "Channel mat has {} cols but gNB has {} ports");

    // Compute the interference-normalized channel matrix
    auto rbNormChanMat = rxSignalRb.m_covMat.CalcIntfNormChannel(rxSignalRb.m_chanMat);

    // Compute downsampled channel per subband
    auto sbNormChanMat = SubbandDownsampling(rbNormChanMat);

    // In case it is time to update the PMI, do it
    if (pmiUpdate.updateWb)
    {
        //  Retrieve number of ports and oversampling factors
        int N1, N2, O1, O2;
        {
            auto codeBook = DynamicCast<NrCbTypeOneSp>(m_rankParams[1].cb);
            N1 = codeBook->m_n1;
            N2 = codeBook->m_n2;
            O1 = codeBook->m_o1;
            O2 = codeBook->m_o2;
        }

        // Start the interpreter and keep it alive
        if (!g_interpreter)
        {
            g_interpreter = new py::scoped_interpreter{};
        }

        // Create a non-owning py::array_t from the valarray data using the same shape
        std::vector<size_t> shape = {sbNormChanMat.GetNumPages(),
                                     sbNormChanMat.GetNumRows(),
                                     sbNormChanMat.GetNumCols()};
        std::vector<size_t> strides = {sizeof(std::complex<double>) * sbNormChanMat.GetNumRows() *
                                           sbNormChanMat.GetNumCols(),
                                       sizeof(std::complex<double>),
                                       sizeof(std::complex<double>) * sbNormChanMat.GetNumCols()};
        py::array HPython = py::array(py::buffer_info(
            const_cast<std::complex<double>*>(&sbNormChanMat.GetValues()[0]), // Pointer to data
            sizeof(std::complex<double>),                                     // Size of one scalar
            py::format_descriptor<std::complex<double>>::format(), // Type format descriptor
            3,                                                     // Number of dimensions
            shape,                                                 // Buffer dimensions
            strides                                                // Strides for each dimension
            ));

        // Transform numpy array into pyttb Tensor
        py::module_ pyttb = py::module_::import("pyttb");
        py::object tensor = pyttb.attr("tensor")(HPython);

        //  Calculate the HOSVD
        auto hosvdResult = pyttb.attr("hosvd")(tensor, 1e-3, -1);

        // Extract U2 and U3 (equivalent of us = hosvdResult.u)
        py::list us = hosvdResult.attr("factor_matrices");

        // Slice the first column of both U2 and U3 matrices (equivalent of us[1][:,0] and
        // us[2][:,0])
        py::object slice_all = py::slice(py::none(), py::none(), py::none());
        py::array u1FirstColumnPython = us[1].attr("__getitem__")(py::make_tuple(slice_all, 0));
        py::array u2FirstColumnPython = us[2].attr("__getitem__")(py::make_tuple(slice_all, 0));

        // Convert the first columns to std::vectors
        std::vector<std::complex<double>> u1, u2;
        for (int i = 0; i < u1FirstColumnPython.attr("__len__")().cast<int>(); i++)
        {
            u1.push_back(u1FirstColumnPython.attr("__getitem__")(i).cast<std::complex<double>>());
        }
        for (int i = 0; i < u2FirstColumnPython.attr("__len__")().cast<int>(); i++)
        {
            u2.push_back(u2FirstColumnPython.attr("__getitem__")(i).cast<std::complex<double>>());
        }

        // At this point we have u1 and u2 first columns
        // of left singular vectors U2 and U3 from HOSVD
        auto sumConjugate = [](std::vector<std::complex<double>>& u, int limit, int offset) {
            std::complex<double> sum{0.0, 0.0};
            for (int k = 0; k < limit; k++)
            {
                sum += std::conj(u[k]) * u[(k + offset) % u.size()];
            }
            return sum;
        };

        // Estimate phases
        double thetaM = WrapTo2Pi(std::arg(sumConjugate(u1, N2 - 1, 1)));
        double thetaL = WrapTo2Pi(std::arg(sumConjugate(u1, N2, N2)));
        double phiN = WrapTo2Pi(std::arg(sumConjugate(u2, N1, N1 * N2)));

        // Estimate DFT coefficients that allows us
        // that can be mapped to i11, i12 and i2
        size_t m = thetaM * N2 * O2 / M_2_PI;
        size_t l = thetaL * N1 * O1 / M_2_PI;
        size_t n = phiN * 2 / M_PI;

        // Perform an exhaustive search to figure out the
        // optimal rank and i1,3 value (in case rank > 2)
        auto besti1 = 0;
        auto bestCap = 0.0;
        ComplexMatrixArray bestPrec;
        for (auto rank : m_ranks)
        {
            auto codeBook = DynamicCast<NrCbTypeOneSp>(m_rankParams[rank].cb);
            if (!codeBook)
            {
                NS_FATAL_ERROR(
                    "Unsupported codebook type for NrPmSearchMaleki. Use NrCbTypeOneSp.");
            }
            auto numI11 = codeBook->GetNumI11();
            auto numI12 = codeBook->GetNumI12();
            auto numI13 = codeBook->GetNumI13();
            // Values should be smaller than numI1x not to go out-of-bounds
            auto i11 = std::min(l, numI11 - 1);
            auto i12 = std::min(m, numI12 - 1);
            auto i2 = n;
            for (size_t i13 = 0; i13 < numI13; i13++)
            {
                auto precMat = codeBook->GetBasePrecMatFromIndex(i11, i12, i13, i2)
                                   .MakeNCopies(rbNormChanMat.GetNumPages());
                auto cap = ComputeCapacityForPrecoders(rbNormChanMat,
                                                       std::vector<ComplexMatrixArray>{precMat})
                               .GetValues()
                               .sum();
                if (cap > bestCap)
                {
                    m_periodMaxRank = rank;
                    besti1 = (i12 * numI11) + i11;
                    bestCap = cap;
                    bestPrec = precMat;
                }
            }
        }
        auto res = Create<NrPmSearchFull::PrecMatParams>();
        res->perfMetric = bestCap;
        res->wbPmi = besti1;
        res->sbPrecMat = bestPrec;
        m_rankParams[m_periodMaxRank].precParams = res;
    }
    else if (pmiUpdate.updateSb)
    {
        // Fallback to normal search when recomputing the best subband precoding (W2)
        // for previously found Rank + W1 found in the previous wideband update
        auto& optPrec = m_rankParams[m_periodMaxRank].precParams;
        NS_ASSERT(optPrec);
        auto wbPmi = optPrec->wbPmi;
        optPrec = FindOptSubbandPrecoding(rbNormChanMat, wbPmi, m_periodMaxRank);
    }
    // Return corresponding CQI/PMI to optimal rank
    return CreateCqiForRank(m_periodMaxRank, rbNormChanMat);
}

} // namespace ns3
