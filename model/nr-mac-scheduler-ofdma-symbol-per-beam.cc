// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mac-scheduler-ofdma-symbol-per-beam.h"

#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NrMacSchedulerOfdmaSymbolPerBeam");

NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerOfdmaSymbolPerBeam);
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerOfdmaSymbolPerBeamLB);
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerOfdmaSymbolPerBeamRR);
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerOfdmaSymbolPerBeamPF);

TypeId
NrMacSchedulerOfdmaSymbolPerBeam::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrMacSchedulerONrMacSchedulerOfdmaSymbolPerBeam").SetParent<Object>();
    return tid;
}

TypeId
NrMacSchedulerOfdmaSymbolPerBeamLB::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacSchedulerONrMacSchedulerOfdmaSymbolPerBeamLB")
                            .SetParent<NrMacSchedulerOfdmaSymbolPerBeam>()
                            .AddConstructor<NrMacSchedulerOfdmaSymbolPerBeamLB>();
    return tid;
}

NrMacSchedulerOfdma::BeamSymbolMap
NrMacSchedulerOfdmaSymbolPerBeamLB::GetSymPerBeam(
    uint32_t symAvail,
    const NrMacSchedulerNs3::ActiveUeMap& activeDl) const
{
    NS_LOG_FUNCTION(this);

    GetSecond GetUeVector;
    GetSecond GetUeBufSize;
    GetFirst GetBeamId;
    double bufTotal = 0.0;
    uint8_t symUsed = 0;
    NrMacSchedulerNs3::BeamSymbolMap ret;

    // Compute buf total
    for (const auto& el : activeDl)
    {
        for (const auto& ue : GetUeVector(el))
        {
            bufTotal += GetUeBufSize(ue);
        }
    }

    for (const auto& el : activeDl)
    {
        uint32_t bufSizeBeam = 0;
        for (const auto& ue : GetUeVector(el))
        {
            bufSizeBeam += GetUeBufSize(ue);
        }

        double tmp = symAvail / bufTotal;
        auto symForBeam = static_cast<uint32_t>(bufSizeBeam * tmp);
        symUsed += symForBeam;
        ret.emplace(std::make_pair(GetBeamId(el), symForBeam));
        NS_LOG_DEBUG("Assigned to beam " << GetBeamId(el) << " symbols " << symForBeam);
    }

    NS_ASSERT(symAvail >= symUsed);
    if (symAvail - symUsed > 0)
    {
        uint8_t symToRedistribute = symAvail - symUsed;
        while (symToRedistribute > 0)
        {
            auto min = ret.end();
            for (auto it = ret.begin(); it != ret.end(); ++it)
            {
                if (min == ret.end() || it->second < min->second)
                {
                    min = it;
                }
            }
            if (min == ret.end())
            {
                break;
            }
            min->second += 1;
            symToRedistribute--;
            NS_LOG_DEBUG("Assigned to beam "
                         << min->first << " an additional symbol, for a total of " << min->second);
        }
    }

    return ret;
}

TypeId
NrMacSchedulerOfdmaSymbolPerBeamRR::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacSchedulerOfdmaSymbolPerBeamRR")
                            .SetParent<NrMacSchedulerOfdmaSymbolPerBeam>()
                            .AddConstructor<NrMacSchedulerOfdmaSymbolPerBeamRR>();
    return tid;
}

NrMacSchedulerOfdma::BeamSymbolMap
NrMacSchedulerOfdmaSymbolPerBeamRR::GetSymPerBeam(
    uint32_t symAvail,
    const NrMacSchedulerNs3::ActiveUeMap& activeDl) const
{
    NS_LOG_FUNCTION(this);

    NrMacSchedulerNs3::BeamSymbolMap ret;

    for (const auto& el : activeDl)
    {
        // Add new beams to the round-robin queue
        if (m_rrBeamsSet.find(el.first) == m_rrBeamsSet.end())
        {
            m_rrBeams.push_back(el.first);
            m_rrBeamsSet.insert(el.first);
        }
    }

    for (size_t sym = 0; sym < symAvail; sym++)
    {
        // Find first active beam in the round-robin queue
        for (size_t i = 0; i < m_rrBeams.size(); ++i)
        {
            // If front beam in round-robin queue is active,
            // allocate one available symbols to it
            if (activeDl.find(m_rrBeams.front()) != activeDl.end())
            {
                if (ret.find(m_rrBeams.front()) == ret.end())
                {
                    ret.emplace(std::make_pair(BeamId(m_rrBeams.front()), 0));
                }
                ret[m_rrBeams.front()] += 1;
                i = m_rrBeams.size(); // delayed break
            }
            // Move round-robin front queue item to the end
            m_rrBeams.push_back(m_rrBeams.front());
            m_rrBeams.pop_front();
        }
    }

    return ret;
}

TypeId
NrMacSchedulerOfdmaSymbolPerBeamPF::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacSchedulerOfdmaSymbolPerBeamPF")
                            .SetParent<NrMacSchedulerOfdmaSymbolPerBeam>()
                            .AddConstructor<NrMacSchedulerOfdmaSymbolPerBeamPF>();
    return tid;
}

NrMacSchedulerOfdmaSymbolPerBeamPF::NrMacSchedulerOfdmaSymbolPerBeamPF(
    GetAmcFromSchedFunc getAmcFunc,
    GetBwInRbgFromSchedFunc bandwidthInRbgFunc)
    : NrMacSchedulerOfdmaSymbolPerBeam(),
      m_getAmcFunc(getAmcFunc),
      m_getBwFunc(bandwidthInRbgFunc)
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_MSG_IF(!m_getAmcFunc || !m_getBwFunc, "m_amc and m_getBwFunc should be set");
}

NrMacSchedulerOfdma::BeamSymbolMap
NrMacSchedulerOfdmaSymbolPerBeamPF::GetSymPerBeam(
    uint32_t symAvail,
    const NrMacSchedulerNs3::ActiveUeMap& activeDl) const
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_MSG_IF(!m_getAmcFunc(), "An invalid NrAmc was retrieved from scheduler");

    GetSecond GetUeVector;
    NrMacSchedulerNs3::BeamSymbolMap ret;

    // Return if activeDl is empty
    if (activeDl.empty())
    {
        return ret;
    }

    // Retrieve information from scheduler only once
    Ptr<NrAmc> amc = m_getAmcFunc();
    uint16_t bwInRbgs = m_getBwFunc();

    // Holds mean previousAchievableRate and current achievable rate for a given beam
    for (const auto& el : activeDl)
    {
        // Add active beam to set, or reset beam stats after many scheduled symbols
        if (activeBeamsPrevAndCurrTBS.find(el.first) == activeBeamsPrevAndCurrTBS.end() ||
            (std::get<2>(activeBeamsPrevAndCurrTBS.at(el.first)) > 200))
        {
            activeBeamsPrevAndCurrTBS[el.first] =
                std::make_tuple<uint64_t, uint64_t, uint64_t>(1, 1, 1);
        }
    }

    // Copy UE buffer size to a structure we can modify
    std::unordered_map<std::shared_ptr<NrMacSchedulerUeInfo>, unsigned> ueRemainingBuffer;
    for (const auto& el : activeDl)
    {
        for (const auto& ue : GetUeVector(el))
        {
            ueRemainingBuffer[ue.first] = ue.second;
        }
    }
    // We calculate scheduling priority for each additional symbol
    for (size_t sym = 1; sym <= symAvail; sym++)
    {
        BeamId maxPriorityBeam;
        double maxPriorityBeamPFMetric = -1.0;

        // For every beam
        for (const auto& [beam, ueVector] : activeDl)
        {
            double sumThr = 0;
            int activeUesInBeam = 0;
            // And for active UE in each beam, estimate TB size
            for (const auto& [ue, buff] : ueVector)
            {
                if (ueRemainingBuffer.at(ue) > 0)
                {
                    sumThr += amc->GetPayloadSize(ue->m_dlMcs, ue->m_dlRank, bwInRbgs);
                    activeUesInBeam++;
                }
            }
            // In case there is no more UE to serve in a beam, skip it
            if (activeUesInBeam == 0)
            {
                continue;
            }
            // Then save TB size divided by number of active UEs to get the mean TBS
            std::get<1>(activeBeamsPrevAndCurrTBS[beam]) = sumThr / activeUesInBeam;

            double currBeamPFMetric = std::get<1>(activeBeamsPrevAndCurrTBS[beam]) /
                                      ((double)std::get<0>(activeBeamsPrevAndCurrTBS[beam]) /
                                       std::get<2>(activeBeamsPrevAndCurrTBS[beam]));
            if (currBeamPFMetric > maxPriorityBeamPFMetric)
            {
                maxPriorityBeam = beam;
                maxPriorityBeamPFMetric = currBeamPFMetric;
            }
        }
        if (maxPriorityBeamPFMetric == -1.0)
        {
            continue;
        }

        // We schedule maxPriorityBeam, but before that, let's reduce the UEs buffers in the
        // scheduled beam, so we don't get disproportionally high TBS even though there is no data
        // remaining to transmit
        uint64_t bytesAllocated = std::get<1>(activeBeamsPrevAndCurrTBS[maxPriorityBeam]);

        // Make a copy of UE vector, so we can sort it with peace of mind
        auto ueVector = activeDl.at(maxPriorityBeam);
        std::stable_sort(ueVector.begin(), ueVector.end(), [](auto a, auto b) {
            return a.second < b.second;
        });
        for (const auto& [ue, buff] : ueVector)
        {
            auto& remBytes = ueRemainingBuffer.at(ue);
            // If one UE buffer is cleared, we won't use its TBS in the next symbol
            if (remBytes < bytesAllocated)
            {
                bytesAllocated -= remBytes;
                remBytes = 0;
            }
            // In case the UE buffer is bigger than allocated bytes, we stop this
            else
            {
                remBytes -= bytesAllocated;
                break;
            }
        }
        // Now we schedule maxPriorityBeam
        std::get<0>(activeBeamsPrevAndCurrTBS[maxPriorityBeam]) +=
            std::get<1>(activeBeamsPrevAndCurrTBS[maxPriorityBeam]);
        std::get<2>(activeBeamsPrevAndCurrTBS[maxPriorityBeam])++;
        if (ret.find(maxPriorityBeam) == ret.end())
        {
            ret[maxPriorityBeam] = 0;
        }
        ret[maxPriorityBeam]++;
    }
    return ret;
}
