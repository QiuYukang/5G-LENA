// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mac-scheduler-lc-rr.h"

#include "ns3/log.h"

#include <algorithm>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrMacSchedulerLcRR");
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerLcRR);

NrMacSchedulerLcRR::NrMacSchedulerLcRR()
    : NrMacSchedulerLcAlgorithm()
{
    NS_LOG_FUNCTION(this);
}

NrMacSchedulerLcRR::~NrMacSchedulerLcRR()
{
}

TypeId
NrMacSchedulerLcRR::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacSchedulerLcRR")
                            .SetParent<NrMacSchedulerLcAlgorithm>()
                            .AddConstructor<NrMacSchedulerLcRR>();
    return tid;
}

std::vector<NrMacSchedulerLcAlgorithm::Assignation>
NrMacSchedulerLcRR::AssignBytesToDlLC(const std::unordered_map<uint8_t, LCGPtr>& ueLCG,
                                      uint32_t tbs,
                                      [[maybe_unused]] Time slotPeriod) const
{
    return AssignBytesToLC(ueLCG, tbs);
}

std::vector<NrMacSchedulerLcAlgorithm::Assignation>
NrMacSchedulerLcRR::AssignBytesToUlLC(const std::unordered_map<uint8_t, LCGPtr>& ueLCG,
                                      uint32_t tbs) const
{
    return AssignBytesToLC(ueLCG, tbs);
}

std::vector<NrMacSchedulerLcAlgorithm::Assignation>
NrMacSchedulerLcRR::AssignBytesToLC(const std::unordered_map<uint8_t, LCGPtr>& ueLCG,
                                    uint32_t tbs) const
{
    NS_LOG_FUNCTION(this);
    GetSecond GetLCG;

    std::vector<NrMacSchedulerLcAlgorithm::Assignation> ret;

    NS_LOG_INFO("To distribute: " << tbs << " bytes over " << ueLCG.size() << " LCG");

    std::map<std::pair<uint8_t, uint8_t>, std::pair<uint32_t, uint32_t>> activeLc;
    for (const auto& lcg : ueLCG)
    {
        std::vector<uint8_t> lcs = GetLCG(lcg)->GetLCId();
        for (const auto& lcId : lcs)
        {
            if (GetLCG(lcg)->GetTotalSizeOfLC(lcId) > 0)
            {
                activeLc[{lcg.first, lcId}] = {GetLCG(lcg)->GetTotalSizeOfLC(lcId), 0};
            }
        }
    }

    if (activeLc.empty())
    {
        return ret;
    }

    while (tbs > 0)
    {
        // Count how many logical channels still need bytes
        auto numActive = std::count_if(activeLc.begin(), activeLc.end(), [](const auto& lc) {
            return lc.second.first > 0;
        });

        // Calculate bytes to assign per LC this round
        uint32_t assignBlockSize = (numActive > 0) ? tbs / numActive : 1;

        // Cap block size to smallest remaining buffer
        for (const auto& [lcId, lcData] : activeLc)
        {
            if (lcData.first > 0)
            {
                assignBlockSize = std::min(assignBlockSize, lcData.first);
            }
        }

        // Ensure at least 1 byte per assignment when many LCs compete
        assignBlockSize = std::max(assignBlockSize, 1u);

        // Distribute bytes to each LC
        for (auto& [lcId, lcData] : activeLc)
        {
            auto& unallocatedBytes = lcData.first;
            auto& allocatedBytes = lcData.second;

            // Skip satisfied LCs while others remain active
            if (unallocatedBytes == 0 && numActive > 0)
            {
                continue;
            }

            // Assign block to this LC
            tbs -= assignBlockSize;
            unallocatedBytes -= assignBlockSize;
            allocatedBytes += assignBlockSize;

            if (tbs == 0)
            {
                break;
            }
        }
    }

    for (auto& activeLcEntry : activeLc)
    {
        const auto lcg = activeLcEntry.first.first;
        const auto lcId = activeLcEntry.first.second;
        const auto amountPerLC = activeLcEntry.second.second;
        ret.emplace_back(lcg, lcId, amountPerLC);
        NS_LOG_INFO("Assigned to LCID " << static_cast<uint32_t>(lcId) << " inside LCG "
                                        << static_cast<uint32_t>(lcg) << " an amount of "
                                        << amountPerLC << " B");
    }
    return ret;
}

}; // namespace ns3
