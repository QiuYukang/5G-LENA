// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-pm-search-sasaoka.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPmSearchSasaoka");
NS_OBJECT_ENSURE_REGISTERED(NrPmSearchSasaoka);

TypeId
NrPmSearchSasaoka::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrPmSearchSasaoka")
                            .SetParent<NrPmSearchFull>()
                            .AddConstructor<NrPmSearchSasaoka>();
    return tid;
}

NrPmSearchSasaoka::NrPmSearchSasaoka()
    : NrPmSearchFull()
{
}

Ptr<NrPmSearchFull::PrecMatParams>
NrPmSearchSasaoka::FindOptSubbandPrecoding(const NrIntfNormChanMat& sbNormChanMat,
                                           size_t i1,
                                           uint8_t rank) const
{
    // Calculate channel correlation
    auto Hcorr = NrIntfNormChanMat(sbNormChanMat.HermitianTranspose() * sbNormChanMat);

    // Extract codebook for rank and the number of I2 entries
    auto& cb = m_rankParams[m_periodMaxRank].cb;
    auto numI2 = cb->GetNumI2();

    // Create an I2 entry per subband
    auto numSubbands = sbNormChanMat.GetNumPages();
    std::vector<std::tuple<uint8_t, double, ComplexMatrixArray>> maxI2s(numSubbands);

    for (auto i2 = size_t{0}; i2 < numI2; i2++)
    {
        // Get band precoding matrix
        auto basePrecMat = cb->GetBasePrecMat(i1, i2);

        // Copy precoding matrix to test all bands at once
        auto extendedPrecMat = basePrecMat.MakeNCopies(numSubbands);

        // Calculate the determinant of all subbands
        auto det = (ComplexMatrixArray::IdentityMatrix(m_periodMaxRank, numSubbands) +
                    extendedPrecMat.HermitianTranspose() * Hcorr * extendedPrecMat)
                       .Determinant();

        // Filter the best subband precoding matrices
        // that produce the highest mutual information for each band
        for (auto subband = size_t{0}; subband < numSubbands; subband++)
        {
            auto miI2 = log2(std::max(0.0, std::abs(det(subband))));
            if (std::get<2>(maxI2s[subband]).GetNumPages() == 0 ||
                miI2 > std::get<1>(maxI2s[subband]))
            {
                std::get<0>(maxI2s[subband]) = i2;
                std::get<1>(maxI2s[subband]) = miI2;
                std::get<2>(maxI2s[subband]) = basePrecMat;
            }
        }
    }
    // Now that we have all the best band I2s for a given I1, we assemble
    // the W1*W2 matrix
    std::vector<ComplexMatrixArray> pages(numSubbands);
    std::transform(maxI2s.begin(), maxI2s.end(), pages.begin(), [](auto a) {
        return std::get<2>(a);
    });
    auto completePrecodingMatrix = ComplexMatrixArray::JoinPages(pages);

    // We can calculate the mutual information for the entire band
    auto det = (ComplexMatrixArray::IdentityMatrix(m_periodMaxRank, numSubbands) +
                completePrecodingMatrix.HermitianTranspose() * Hcorr * completePrecodingMatrix)
                   .Determinant();
    double mi = 0.0;
    for (auto subband = size_t{0}; subband < sbNormChanMat.GetNumPages(); subband++)
    {
        mi += log2(std::abs(det(subband)));
    }

    // Assemble RI/PMI feedback
    auto res = Create<NrPmSearchFull::PrecMatParams>();
    res->perfMetric = mi;
    res->wbPmi = i1;
    res->sbPrecMat = completePrecodingMatrix;
    res->sbPmis.resize(numSubbands);
    std::transform(maxI2s.begin(), maxI2s.end(), res->sbPmis.begin(), [](auto a) {
        return std::get<0>(a);
    });
    return res;
}

PmCqiInfo
NrPmSearchSasaoka::CreateCqiFeedbackMimo(const NrMimoSignal& rxSignalRb, PmiUpdate pmiUpdate)
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
        auto Hcorr = NrIntfNormChanMat(sbNormChanMat.HermitianTranspose() * sbNormChanMat);

        // Select the maximum rank
        m_periodMaxRank = SelectRank(Hcorr);

        // Find the optimal wideband PMI i1 and sub-band i2
        auto& cb = m_rankParams[m_periodMaxRank].cb;
        auto numI1 = cb->GetNumI1();

        Ptr<PrecMatParams> tempMax;
        for (auto i1 = size_t{0}; i1 < numI1; i1++)
        {
            auto temp = FindOptSubbandPrecoding(sbNormChanMat, i1, m_periodMaxRank);
            if (!tempMax || temp->perfMetric > tempMax->perfMetric)
            {
                tempMax = temp;
            }
        }
        m_rankParams[m_periodMaxRank].precParams = tempMax;
    }
    else if (pmiUpdate.updateSb)
    {
        // Recompute the best subband precoding (W2) for previously found W1 and store results
        auto& optPrec = m_rankParams[m_periodMaxRank].precParams;
        NS_ASSERT(optPrec);
        auto wbPmi = optPrec->wbPmi;
        optPrec = FindOptSubbandPrecoding(sbNormChanMat, wbPmi, m_periodMaxRank);
    }
    // Return corresponding CQI/PMI to optimal rank
    return CreateCqiForRank(m_periodMaxRank, rbNormChanMat);
}

} // namespace ns3
