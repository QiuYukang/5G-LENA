// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-pm-search-ideal.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPmSearchIdeal");
NS_OBJECT_ENSURE_REGISTERED(NrPmSearchIdeal);

TypeId
NrPmSearchIdeal::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrPmSearchIdeal")
                            .SetParent<NrPmSearchFull>()
                            .AddConstructor<NrPmSearchIdeal>();
    return tid;
}

PmCqiInfo
NrPmSearchIdeal::CreateCqiFeedbackMimo(const NrMimoSignal& rxSignalRb, PmiUpdate pmiUpdate)
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

    // Update precoding matrices
    if (pmiUpdate.updateWb || pmiUpdate.updateSb)
    {
        Ptr<NrPmSearchFull::PrecMatParams> best = nullptr;
        uint8_t bestRank = 0;
        int rankLimit = std::min(sbNormChanMat.GetNumRows(), sbNormChanMat.GetNumCols());
        for (auto rank : m_ranks)
        {
            if (rank > rankLimit)
            {
                break;
            }
            auto res = Create<NrPmSearchFull::PrecMatParams>();
            res->wbPmi = 0;                    // todo: put a meaningful placeholder
            res->sbPmis.resize(m_subbandSize); // todo: put a meaningful placeholder
            res->sbPrecMat = sbNormChanMat.ExtractOptimalPrecodingMatrices(rank);

            // Compute wideband capacity of optimal precoders
            auto sinr = sbNormChanMat.ComputeSinrForPrecoding(res->sbPrecMat);
            auto mcsParams = m_amc->GetMaxMcsParams(sinr, m_subbandSize);
            res->perfMetric = mcsParams.tbSize;
            if (!best)
            {
                best = res;
                bestRank = rank;
            }
            else if (res->perfMetric > best->perfMetric)
            {
                if (mcsParams.wbCqi == 0)
                {
                    break;
                }
                best = res;
                bestRank = rank;
            }
        }
        NS_ASSERT_MSG(bestRank > 0, "A valid rank was selected");
        NS_ASSERT_MSG(best, "A precoding matrix was selected");
        m_rankParams[bestRank].precParams = best;
        m_periodMaxRank = bestRank;
    }

    // Return corresponding CQI/PMI to optimal rank
    return CreateCqiForRank(m_periodMaxRank, rbNormChanMat);
}

} // namespace ns3
