// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-pm-search-fast.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPmSearchFast");
NS_OBJECT_ENSURE_REGISTERED(NrPmSearchFast);

TypeId
NrPmSearchFast::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrPmSearchFast").SetParent<NrPmSearchFull>().AddConstructor<NrPmSearchFast>();
    return tid;
}

NrPmSearchFast::NrPmSearchFast()
    : NrPmSearchFull() {

      };

size_t
NrPmSearchFast::GetWidebandI1(Ptr<const NrCbTypeOne> cb, const ComplexMatrixArray& Havg) const
{
    // Instead of calculating all subband i2s to find the best wideband i1,
    // we instead calculate the best wideband i1, then search for subband i2s
    auto numI1 = cb->GetNumI1();
    auto numI2 = cb->GetNumI2();
    int maxI1 = 0;
    double maxCap = 0.0;
    for (auto i1 = size_t{0}; i1 < numI1; i1++)
    {
        for (auto i2 = size_t{0}; i2 < numI2; i2++)
        {
            auto basePrecMat = cb->GetBasePrecMat(i1, i2);
            auto cap =
                ComputeCapacityForPrecoders(Havg,
                                            std::vector<ComplexMatrixArray>{basePrecMat})(0, 0, 0);
            if (cap > maxCap)
            {
                maxI1 = i1;
                maxCap = cap;
            }
        }
    }
    return maxI1;
}

PmCqiInfo
NrPmSearchFast::CreateCqiFeedbackMimo(const NrMimoSignal& rxSignalRb, PmiUpdate pmiUpdate)
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
        // Compute the channel correlation for each band (C = H^h * H)
        auto C = NrIntfNormChanMat(sbNormChanMat.HermitianTranspose() * sbNormChanMat);

        // Select the maximum rank
        m_periodMaxRank = SelectRank(C);

        // Compute the channel average over bands
        auto Cavg = C.GetWidebandChannel();

        // Find the optimal wideband PMI i1
        auto maxI1 = GetWidebandI1(m_rankParams[m_periodMaxRank].cb, Cavg);

        // Find optimal PMI i2
        m_rankParams[m_periodMaxRank].precParams =
            FindOptSubbandPrecoding(sbNormChanMat, maxI1, m_periodMaxRank);
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
