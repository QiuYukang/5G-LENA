/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mac-scheduler-lc-qos.h"
#include "ns3/log.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerLcQos");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerLcQos);


NrMacSchedulerLcQos::NrMacSchedulerLcQos()
    : NrMacSchedulerLcAlgorithm()
{
    NS_LOG_FUNCTION (this);
}

NrMacSchedulerLcQos::~NrMacSchedulerLcQos ()
{
}

TypeId
NrMacSchedulerLcQos::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacSchedulerLcQos")
                            .SetParent<NrMacSchedulerLcAlgorithm>()
                            .AddConstructor<NrMacSchedulerLcQos>();
    return tid;
}

TypeId
NrMacSchedulerLcQos::GetInstanceTypeId() const
{
    return NrMacSchedulerLcQos::GetTypeId();
}

std::vector<NrMacSchedulerLcAlgorithm::Assignation>
NrMacSchedulerLcQos::AssignBytesToDlLC(const std::unordered_map<uint8_t, LCGPtr>& ueLCG,
                                       uint32_t tbs, [[maybe_unused]]Time slotPeriod) const
{
    NS_LOG_FUNCTION(this);
    GetFirst GetLCGID;
    GetSecond GetLCG;

    std::vector<NrMacSchedulerLcAlgorithm::Assignation> ret;

    NS_LOG_INFO("To distribute: " << tbs << " bytes over " << ueLCG.size() << " LCG"
                                  << " in Qos manner");

    uint32_t activeLc = 0;
    for (const auto& lcg : ueLCG)
    {
        std::vector<uint8_t> lcs = GetLCG(lcg)->GetLCId();
        for (const auto& lcId : lcs)
        {
            if (GetLCG(lcg)->GetTotalSizeOfLC(lcId) > 0)
            {
                ++activeLc;
            }
        }
    }

    if (activeLc == 0)
    {
        return ret;
    }

    uint32_t amountPerLC = tbs / activeLc;
    NS_LOG_INFO("Total LC: " << activeLc << " each one will receive " << amountPerLC << " bytes");

    for (const auto& lcg : ueLCG)
    {
        std::vector<uint8_t> lcs = GetLCG(lcg)->GetLCId();
        for (const auto& lcId : lcs)
        {
            if (GetLCG(lcg)->GetTotalSizeOfLC(lcId) > 0)
            {
                NS_LOG_INFO("Assigned to LCID " << static_cast<uint32_t>(lcId) << " inside LCG "
                                                << static_cast<uint32_t>(GetLCGID(lcg))
                                                << " an amount of " << amountPerLC << " B");
                ret.emplace_back(Assignation(GetLCGID(lcg), lcId, amountPerLC));
            }
        }
    }

    return ret;
}

std::vector<NrMacSchedulerLcAlgorithm::Assignation>
NrMacSchedulerLcQos::AssignBytesToUlLC(const std::unordered_map<uint8_t, LCGPtr>& ueLCG,
                                    uint32_t tbs) const
{
    NS_LOG_FUNCTION(this);
    GetFirst GetLCGID;
    GetSecond GetLCG;

    std::vector<NrMacSchedulerLcAlgorithm::Assignation> ret;

    NS_LOG_INFO("To distribute: " << tbs << " bytes over " << ueLCG.size() << " LCG"
                                  << " in Qos manner");

    uint32_t activeLc = 0;
    for (const auto& lcg : ueLCG)
    {
        std::vector<uint8_t> lcs = GetLCG(lcg)->GetLCId();
        for (const auto& lcId : lcs)
        {
            if (GetLCG(lcg)->GetTotalSizeOfLC(lcId) > 0)
            {
                ++activeLc;
            }
        }
    }

    if (activeLc == 0)
    {
        return ret;
    }

    uint32_t amountPerLC = tbs / activeLc;
    NS_LOG_INFO("Total LC: " << activeLc << " each one will receive " << amountPerLC << " bytes");

    for (const auto& lcg : ueLCG)
    {
        std::vector<uint8_t> lcs = GetLCG(lcg)->GetLCId();
        for (const auto& lcId : lcs)
        {
            if (GetLCG(lcg)->GetTotalSizeOfLC(lcId) > 0)
            {
                NS_LOG_INFO("Assigned to LCID " << static_cast<uint32_t>(lcId) << " inside LCG "
                                                << static_cast<uint32_t>(GetLCGID(lcg))
                                                << " an amount of " << amountPerLC << " B");
                ret.emplace_back(Assignation(GetLCGID(lcg), lcId, amountPerLC));
            }
        }
    }

    return ret;
}

}
