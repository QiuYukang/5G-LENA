/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only and NIST-Software

#include "nr-sl-ue-mac-scheduler-fixed-mcs.h"

#include "nr-sl-ue-mac-harq.h"
#include "nr-ue-mac.h"

#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/uinteger.h>

#include <optional>
#include <queue>

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT                                                                      \
    if (GetMac())                                                                                  \
    {                                                                                              \
        std::clog << "[imsi=" << GetMac()->GetImsi() << "] ";                                      \
    }

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrSlUeMacSchedulerFixedMcs");
NS_OBJECT_ENSURE_REGISTERED(NrSlUeMacSchedulerFixedMcs);

TypeId
NrSlUeMacSchedulerFixedMcs::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::NrSlUeMacSchedulerFixedMcs")
            .SetParent<NrSlUeMacScheduler>()
            .AddConstructor<NrSlUeMacSchedulerFixedMcs>()
            .SetGroupName("nr")
            .AddAttribute("Mcs",
                          "The fixed value of the MCS used by this scheduler",
                          UintegerValue(14),
                          MakeUintegerAccessor(&NrSlUeMacSchedulerFixedMcs::m_mcs),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute("PriorityToSps",
                          "Flag to give scheduling priority to logical channels that are "
                          "configured with SPS in case of priority tie",
                          BooleanValue(true),
                          MakeBooleanAccessor(&NrSlUeMacSchedulerFixedMcs::m_prioToSps),
                          MakeBooleanChecker())
            .AddAttribute("AllowMultipleDestinationsPerSlot",
                          "Allow scheduling of multiple destinations in same slot",
                          BooleanValue(false),
                          MakeBooleanAccessor(
                              &NrSlUeMacSchedulerFixedMcs::m_allowMultipleDestinationsPerSlot),
                          MakeBooleanChecker());
    return tid;
}

NrSlUeMacSchedulerFixedMcs::NrSlUeMacSchedulerFixedMcs()
{
    NS_LOG_FUNCTION(this);
    m_grantSelectionUniformVariable = CreateObject<UniformRandomVariable>();
    m_destinationUniformVariable = CreateObject<UniformRandomVariable>();
    m_ueSelectedUniformVariable = CreateObject<UniformRandomVariable>();
}

NrSlUeMacSchedulerFixedMcs::~NrSlUeMacSchedulerFixedMcs()
{
    // just to make sure
    m_dstMap.clear();
}

void
NrSlUeMacSchedulerFixedMcs::DoCschedNrSlLcConfigReq(
    const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params)
{
    NS_LOG_FUNCTION(this << params.dstL2Id << +params.lcId);

    auto dstInfo = CreateDstInfo(params);
    const auto& lcgMap = dstInfo->GetNrSlLCG(); // Map of unique_ptr should not copy
    auto itLcg = lcgMap.find(params.lcGroup);
    auto itLcgEnd = lcgMap.end();
    if (itLcg == itLcgEnd)
    {
        NS_LOG_INFO("Created new NR SL LCG for destination "
                    << dstInfo->GetDstL2Id()
                    << " LCG ID =" << static_cast<uint32_t>(params.lcGroup));
        itLcg = dstInfo->Insert(CreateLCG(params.lcGroup));
    }

    itLcg->second->Insert(CreateLC(params));
    NS_LOG_INFO("Added LC id " << +params.lcId << " in LCG " << +params.lcGroup);
    // send confirmation to UE MAC
    GetMac()->CschedNrSlLcConfigCnf(params.lcGroup, params.lcId);
}

std::shared_ptr<NrSlUeMacSchedulerDstInfo>
NrSlUeMacSchedulerFixedMcs::CreateDstInfo(
    const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params)
{
    std::shared_ptr<NrSlUeMacSchedulerDstInfo> dstInfo = nullptr;
    auto itDst = m_dstMap.find(params.dstL2Id);
    if (itDst == m_dstMap.end())
    {
        NS_LOG_INFO("Creating destination info. Destination L2 id " << params.dstL2Id);

        dstInfo = std::make_shared<NrSlUeMacSchedulerDstInfo>(params.dstL2Id);
        dstInfo->SetDstMcs(m_mcs);

        itDst = m_dstMap.insert(std::make_pair(params.dstL2Id, dstInfo)).first;
    }
    else
    {
        NS_LOG_DEBUG("Doing nothing. You are seeing this because we are adding new LC "
                     << +params.lcId << " for Dst " << params.dstL2Id);
        dstInfo = itDst->second;
    }

    return dstInfo;
}

void
NrSlUeMacSchedulerFixedMcs::DoRemoveNrSlLcConfigReq(uint8_t lcid, uint32_t dstL2Id)
{
    NS_LOG_FUNCTION(this << lcid << dstL2Id);
    RemoveDstInfo(lcid, dstL2Id);
    // Send confirmation to MAC
    GetMac()->RemoveNrSlLcConfigCnf(lcid);
}

void
NrSlUeMacSchedulerFixedMcs::RemoveDstInfo(uint8_t lcid, uint32_t dstL2Id)
{
    NS_LOG_FUNCTION(this << lcid << dstL2Id);
    auto itDst = m_dstMap.find(dstL2Id);
    if (itDst != m_dstMap.end())
    {
        NS_LOG_INFO("Found Destination L2 ID " << dstL2Id);
        // find LCID in available LCGIDs and remove it
        const auto& lcgMap = itDst->second->GetNrSlLCG();
        for (auto it = lcgMap.begin(); it != lcgMap.end(); it++)
        {
            it->second->Remove(lcid);
        }
    }
    else
    {
        NS_LOG_DEBUG("Already removed! Nothing to do!");
    }
}

NrSlLCGPtr
NrSlUeMacSchedulerFixedMcs::CreateLCG(uint8_t lcGroup) const
{
    NS_LOG_FUNCTION(this << +lcGroup);
    return std::unique_ptr<NrSlUeMacSchedulerLCG>(new NrSlUeMacSchedulerLCG(lcGroup));
}

NrSlLCPtr
NrSlUeMacSchedulerFixedMcs::CreateLC(
    const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params) const
{
    NS_LOG_FUNCTION(this << params.dstL2Id << +params.lcId);
    return std::unique_ptr<NrSlUeMacSchedulerLC>(new NrSlUeMacSchedulerLC(params));
}

void
NrSlUeMacSchedulerFixedMcs::DoSchedNrSlRlcBufferReq(
    const struct NrSlMacSapProvider::NrSlReportBufferStatusParameters& params)
{
    NS_LOG_FUNCTION(this << params.dstL2Id << +params.lcid);

    GetSecond DstInfoOf;
    auto itDst = m_dstMap.find(params.dstL2Id);
    NS_ABORT_MSG_IF(itDst == m_dstMap.end(), "Destination " << params.dstL2Id << " info not found");

    for (const auto& lcg : DstInfoOf(*itDst)->GetNrSlLCG())
    {
        if (lcg.second->Contains(params.lcid))
        {
            NS_LOG_INFO("Updating buffer status for LC in LCG: "
                        << +lcg.first << " LC: " << +params.lcid << " dstL2Id: " << params.dstL2Id
                        << " queue size: " << params.txQueueSize);
            lcg.second->UpdateInfo(params);
            return;
        }
    }
    // Fail miserably because we didn't find any LC
    NS_FATAL_ERROR("The LC does not exist. Can't update");
}

uint8_t
NrSlUeMacSchedulerFixedMcs::GetRandomReselectionCounter(Time rri) const
{
    uint8_t min;
    uint8_t max;
    uint16_t periodInt = static_cast<uint16_t>(rri.GetMilliSeconds());

    switch (periodInt)
    {
    case 100:
    case 150:
    case 200:
    case 250:
    case 300:
    case 350:
    case 400:
    case 450:
    case 500:
    case 550:
    case 600:
    case 700:
    case 750:
    case 800:
    case 850:
    case 900:
    case 950:
    case 1000:
        min = 5;
        max = 15;
        break;
    default:
        if (periodInt < 100)
        {
            min = GetLowerBoundReselCounter(periodInt);
            max = GetUpperBoundReselCounter(periodInt);
        }
        else
        {
            NS_FATAL_ERROR("VALUE NOT SUPPORTED!");
        }
        break;
    }

    NS_LOG_DEBUG("Range to choose random reselection counter. min: " << +min << " max: " << +max);
    return m_ueSelectedUniformVariable->GetInteger(min, max);
}

uint8_t
NrSlUeMacSchedulerFixedMcs::GetLowerBoundReselCounter(uint16_t pRsrv) const
{
    NS_ASSERT_MSG(pRsrv < 100, "Resource reservation must be less than 100 ms");
    uint8_t lBound = (5 * std::ceil(100 / (std::max(static_cast<uint16_t>(20), pRsrv))));
    return lBound;
}

uint8_t
NrSlUeMacSchedulerFixedMcs::GetUpperBoundReselCounter(uint16_t pRsrv) const
{
    NS_ASSERT_MSG(pRsrv < 100, "Resource reservation must be less than 100 ms");
    uint8_t uBound = (15 * std::ceil(100 / (std::max(static_cast<uint16_t>(20), pRsrv))));
    return uBound;
}

void
NrSlUeMacSchedulerFixedMcs::DoSchedNrSlTriggerReq(const SfnSf& sfn)
{
    NS_LOG_FUNCTION(this << sfn);

    if (!GetMacHarq()->GetNumAvailableHarqIds())
    {
        // Cannot create new grants at this time but there may be existing
        // ones to publish
        CheckForGrantsToPublish(sfn);
        return;
    }

    // 1. Obtain which destinations and logical channels are in need of scheduling
    std::map<uint32_t, std::vector<uint8_t>> dstsAndLcsToSched;
    GetDstsAndLcsNeedingScheduling(sfn, dstsAndLcsToSched);
    if (dstsAndLcsToSched.size() > 0)
    {
        NS_LOG_DEBUG("There are " << dstsAndLcsToSched.size()
                                  << " destinations needing scheduling");

        // 2. Allocate as much of the destinations and logical channels as possible,
        //    following the Logical Channel Prioritization (LCP) procedure
        while (dstsAndLcsToSched.size() > 0)
        {
            AllocationInfo allocationInfo;
            std::list<SlResourceInfo> candResources;
            uint32_t dstL2IdtoServe = 0;
            dstL2IdtoServe =
                LogicalChannelPrioritization(sfn, dstsAndLcsToSched, allocationInfo, candResources);

            NS_LOG_DEBUG("Destination L2 Id to allocate: "
                         << dstL2IdtoServe
                         << " Number of LCs: " << allocationInfo.m_allocatedRlcPdus.size()
                         << " Priority: " << +allocationInfo.m_priority << " Is dynamic: "
                         << allocationInfo.m_isDynamic << " TB size: " << allocationInfo.m_tbSize
                         << " HARQ enabled: " << allocationInfo.m_harqEnabled);
            NS_LOG_DEBUG("Resources available (" << candResources.size() << "):");
            for (auto itCandResou : candResources)
            {
                NS_LOG_DEBUG(itCandResou.sfn
                             << " slSubchannelStart: " << +itCandResou.slSubchannelStart
                             << " slSubchannelSize:" << itCandResou.slSubchannelSize);
            }
            if (dstL2IdtoServe > 0)
            {
                if (candResources.size() > 0 && allocationInfo.m_allocatedRlcPdus.size() > 0)
                {
                    AttemptGrantAllocation(sfn, dstL2IdtoServe, candResources, allocationInfo);
                    m_reselCounter = 0;
                    m_cResel = 0;

                    // Remove served logical channels from the dstsAndLcsToSched
                    auto itDstsAndLcsToSched = dstsAndLcsToSched.find(dstL2IdtoServe);
                    if (allocationInfo.m_allocatedRlcPdus.size() ==
                        itDstsAndLcsToSched->second.size())
                    {
                        NS_LOG_DEBUG("All logical channels of destination " << dstL2IdtoServe
                                                                            << " were allocated");
                        // All LCs where served, remove destination
                        dstsAndLcsToSched.erase(dstL2IdtoServe);
                    }
                    else
                    {
                        NS_LOG_DEBUG("Only " << allocationInfo.m_allocatedRlcPdus.size() << "/"
                                             << itDstsAndLcsToSched->second.size()
                                             << " logical channels of destination "
                                             << dstL2IdtoServe << " were allocated");
                        // Remove only the LCs that were served
                        for (auto slRlcPduInfo : allocationInfo.m_allocatedRlcPdus)
                        {
                            auto itLcs = itDstsAndLcsToSched->second.begin();
                            while (itLcs != itDstsAndLcsToSched->second.end())
                            {
                                if (*itLcs == slRlcPduInfo.lcid)
                                {
                                    NS_LOG_DEBUG("Erasing LCID " << slRlcPduInfo.lcid);
                                    itLcs = itDstsAndLcsToSched->second.erase(itLcs);
                                }
                                else
                                {
                                    ++itLcs;
                                }
                            }
                        }
                    }
                }
                else
                {
                    NS_LOG_DEBUG("Unable to allocate destination " << dstL2IdtoServe);
                    // It could happen that we are not able to serve this destination
                    // but could serve any of the other destinations needing scheduling.
                    // This case is not currently considered and we stop trying to allocate
                    // destinations at the first one we are not able to serve.
                    break;
                }
            }
            else
            {
                NS_LOG_DEBUG("No destination found to serve");
                break;
            }
        }
    }
    else
    {
        NS_LOG_DEBUG("No destination needing scheduling");
    }
    CheckForGrantsToPublish(sfn);
}

void
NrSlUeMacSchedulerFixedMcs::DoNotifyNrSlRlcPduDequeue(uint32_t dstL2Id, uint8_t lcId, uint32_t size)
{
    NS_LOG_FUNCTION(this << dstL2Id << +lcId << size);

    const auto itDstInfo = m_dstMap.find(dstL2Id);
    const auto& lcgMap = itDstInfo->second->GetNrSlLCG();
    lcgMap.begin()->second->AssignedData(lcId, size);

    return;
}

bool
NrSlUeMacSchedulerFixedMcs::TxResourceReselectionCheck(const SfnSf& sfn,
                                                       uint32_t dstL2Id,
                                                       uint8_t lcId)
{
    NS_LOG_FUNCTION(this << sfn << dstL2Id << +lcId);
    const auto itDstInfo = m_dstMap.find(dstL2Id);
    const auto& lcgMap = itDstInfo->second->GetNrSlLCG();

    bool isLcDynamic = lcgMap.begin()->second->IsLcDynamic(lcId);
    uint32_t lcBufferSize = lcgMap.begin()->second->GetTotalSizeOfLC(lcId);
    NS_LOG_DEBUG("LcId " << +lcId << " buffer size " << lcBufferSize);
    if (lcBufferSize == 0)
    {
        NS_LOG_DEBUG("Didn't pass, Empty buffer");
        return false;
    }

    // Check if the LC has a grant
    const auto itGrantInfo = m_grantInfo.find(dstL2Id);
    bool grantFoundForDest = itGrantInfo != m_grantInfo.end() ? true : false;
    bool grantFoundForLc = false;
    std::vector<GrantInfo>::iterator itGrantFoundLc;
    if (grantFoundForDest)
    {
        // Look in all the grants of the destination
        for (auto itGrantVector = itGrantInfo->second.begin();
             itGrantVector != itGrantInfo->second.end();
             ++itGrantVector)
        {
            if (itGrantVector->slotAllocations.size() == 0)
            {
                continue;
            }
            // Look if any of the RLC PDUs correspond to the LCID
            for (const auto& it : itGrantVector->slotAllocations.begin()->slRlcPduInfo)
            {
                if (it.lcid == lcId)
                {
                    NS_LOG_DEBUG("LcId " << +lcId << " already has a grant ");
                    grantFoundForLc = true;
                    break;
                }
            }
            if (grantFoundForLc)
            {
                itGrantFoundLc = itGrantVector;
                break;
            }
        }
    }
    bool pass = false;
    if (isLcDynamic)
    {
        // Currently we do not support grant reevaluation/reselection for dynamic grants.
        // Only the LCs with no grant at the moment and data to transmit will pass the check.
        if (!grantFoundForLc && lcBufferSize > 0)
        {
            NS_LOG_DEBUG("Passed, Fresh dynamic grant required");
            pass = true;
        }
    }
    else // SPS
    {
        if (lcBufferSize > 0)
        {
            if (!grantFoundForLc)
            {
                NS_LOG_DEBUG("Passed, Fresh SPS grant required");
                pass = true;
            }
            else
            {
                // Currently the only grant reselection that is supported for SPS grants are those
                // governed by the slResoReselCounter, cReselCounter and slProbResourceKeep
                NS_LOG_DEBUG("slResoReselCounter " << +itGrantFoundLc->slResoReselCounter
                                                   << " cReselCounter "
                                                   << itGrantFoundLc->cReselCounter);
                if (itGrantFoundLc->slResoReselCounter == 0)
                {
                    if (itGrantFoundLc->cReselCounter > 0)
                    {
                        double randProb = m_ueSelectedUniformVariable->GetValue(0, 1);
                        double slProbResourceKeep = GetMac()->GetSlProbResourceKeep();
                        if (slProbResourceKeep > randProb)
                        {
                            NS_LOG_INFO(
                                "slProbResourceKeep ("
                                << slProbResourceKeep << ") > randProb (" << randProb << ")"
                                << ", Keeping the SPS grant, restarting slResoReselCounter");
                            // keeping the resource, reassign the same sidelink resource
                            // re-selection counter we chose while creating the fresh grant
                            itGrantFoundLc->slResoReselCounter =
                                itGrantFoundLc->prevSlResoReselCounter;
                            auto timeout =
                                GetSpsGrantTimeout(sfn,
                                                   itGrantFoundLc->prevSlResoReselCounter,
                                                   itGrantFoundLc->rri);
                            bool renewed [[maybe_unused]] =
                                GetMacHarq()->RenewHarqProcessIdTimer(itGrantFoundLc->harqId,
                                                                      timeout);
                            NS_ASSERT_MSG(renewed, "Timer failed to renew");
                        }
                        else
                        {
                            // Clear the grant.
                            itGrantInfo->second.erase(itGrantFoundLc);
                            NS_LOG_INFO("Passed, slProbResourceKeep ("
                                        << slProbResourceKeep << ") <= randProb (" << randProb
                                        << ")"
                                        << ", Clearing the SPS grant");
                            GetMacHarq()->DeallocateHarqProcessId(itGrantFoundLc->harqId);
                            pass = true;
                        }
                    }
                    else
                    {
                        // Clear the grant
                        itGrantInfo->second.erase(itGrantFoundLc);
                        NS_LOG_INFO("Passed, cReselCounter == 0, Clearing the SPS grant");
                        pass = true;
                        GetMacHarq()->DeallocateHarqProcessId(itGrantFoundLc->harqId);
                    }
                }
                else
                {
                    NS_LOG_DEBUG("slResoReselCounter != 0");
                }
            }
        }
    }
    if (!pass)
    {
        NS_LOG_DEBUG("Didn't pass the check");
    }

    return pass;
}

uint32_t
NrSlUeMacSchedulerFixedMcs::LogicalChannelPrioritization(
    const SfnSf& sfn,
    std::map<uint32_t, std::vector<uint8_t>> dstsAndLcsToSched,
    AllocationInfo& allocationInfo,
    std::list<SlResourceInfo>& candResources)

{
    NS_LOG_FUNCTION(this << dstsAndLcsToSched.size() << candResources.size());

    if (dstsAndLcsToSched.size() == 0)
    {
        return 0;
    }
    m_reselCounter = 0;
    m_cResel = 0;

    // At this point all LCs in dstsAndLcsToSched have data to transmit, so we
    // focus on checking the other conditions for the selection and allocation.

    // 1. Selection of destination and logical channels to allocate
    // 1.1 Select the destination:
    //    - with the LC with the highest priority
    //    - if multiple destination share the same highest priority, select one randomly
    //    Other heuristics that can be developed: closest to PDB, largest queue, longest without
    //    allocation, round robin.
    std::map<uint8_t, std::vector<uint32_t>> dstL2IdsbyPrio;
    for (auto& itDst : dstsAndLcsToSched)
    {
        uint8_t lcHighestPrio = 0;
        auto itDstInfo = m_dstMap.find(itDst.first);
        auto& lcgMap = itDstInfo->second->GetNrSlLCG();
        for (auto& itLc : itDst.second)
        {
            uint8_t lcPriority = lcgMap.begin()->second->GetLcPriority(itLc);
            NS_LOG_DEBUG("Destination L2 ID "
                         << itDst.first << " LCID " << +itLc << " priority " << +lcPriority
                         << " buffer size " << lcgMap.begin()->second->GetTotalSizeOfLC(itLc)
                         << " dynamic scheduling " << lcgMap.begin()->second->IsLcDynamic(itLc)
                         << " RRI " << (lcgMap.begin()->second->GetLcRri(itLc)).GetMilliSeconds()
                         << " ms");
            if (lcPriority > lcHighestPrio)
            {
                lcHighestPrio = lcPriority;
            }
        }
        auto itDstL2IdsbyPrio = dstL2IdsbyPrio.find(lcHighestPrio);
        if (itDstL2IdsbyPrio == dstL2IdsbyPrio.end())
        {
            std::vector<uint32_t> dstIds;
            dstIds.emplace_back(itDst.first);
            dstL2IdsbyPrio.emplace(lcHighestPrio, dstIds);
        }
        else
        {
            itDstL2IdsbyPrio->second.emplace_back(itDst.first);
        }
    }
    // The highest priority will be at the rear of the map and the smallest dstL2Id will be at the
    // front of the vector for that priority
    uint8_t dstHighestPrio = dstL2IdsbyPrio.rbegin()->first;
    NS_ASSERT_MSG(dstL2IdsbyPrio.rbegin()->second.size(), "Unexpected empty vector");
    // Select a dstL2Id randomly
    uint32_t randomIndex =
        m_destinationUniformVariable->GetInteger(0, dstL2IdsbyPrio.rbegin()->second.size() - 1);
    uint32_t dstIdSelected = dstL2IdsbyPrio.rbegin()->second.at(randomIndex);
    NS_LOG_INFO("Selected dstL2ID "
                << dstIdSelected << " (" << dstL2IdsbyPrio.rbegin()->second.size() << "/"
                << dstsAndLcsToSched.size() << " destinations with highest LC priority of "
                << +dstHighestPrio << ")");

    // 1.2.Select destination's logical channels that
    //  - will have the same grant attributes (scheduling type, scheduling attributes,
    //    and HARQ feedback type) than the LC with highest priority
    //  - if multiple LCs with different scheduling type share the same highest priority,
    //    select the one(s) with scheduling type priority indicated by m_prioToSps attribute
    //  - if m_prioToSps and multiple LCs with SPS scheduling type and different RRI share the same
    //  highest priority,
    //    select the one(s) with RRI equal to the LC with lowest LcId
    //  - TODO: how to handle HARQ type in ties
    auto itDstInfo = m_dstMap.find(dstIdSelected);
    const auto& lcgMap = itDstInfo->second->GetNrSlLCG();
    const auto& itDst = dstsAndLcsToSched.find(dstIdSelected);
    std::map<uint8_t, std::vector<uint8_t>> lcIdsbyPrio;
    for (auto& itLc : itDst->second)
    {
        uint8_t lcPriority = lcgMap.begin()->second->GetLcPriority(itLc);
        auto itLcIdsbyPrio = lcIdsbyPrio.find(lcPriority);
        if (itLcIdsbyPrio == lcIdsbyPrio.end())
        {
            std::vector<uint8_t> lcIds;
            lcIds.emplace_back(itLc);
            lcIdsbyPrio.emplace(lcPriority, lcIds);
        }
        else
        {
            itLcIdsbyPrio->second.emplace_back(itLc);
        }
    }
    // Verify type of scheduling of LCs with highest priority (the one at the rear of the map)
    bool dynamicGrant = true;
    uint16_t nDynLcs = 0;
    uint16_t nSpsLcs = 0;
    if (lcIdsbyPrio.rbegin()->second.size() > 1)
    {
        for (auto& itLcsHighestPrio : lcIdsbyPrio.rbegin()->second)
        {
            if (lcgMap.begin()->second->IsLcDynamic(itLcsHighestPrio))
            {
                nDynLcs++;
            }
            else
            {
                nSpsLcs++;
            }
        }
        if ((m_prioToSps && nSpsLcs > 0) || (!m_prioToSps && nDynLcs == 0 && nSpsLcs > 0))
        {
            dynamicGrant = false;
        }
    }
    else
    {
        dynamicGrant = lcgMap.begin()->second->IsLcDynamic(lcIdsbyPrio.rbegin()->second.front());
    }
    if (dynamicGrant)
    {
        allocationInfo.m_isDynamic = true;
        NS_LOG_DEBUG("Selected scheduling type: dynamic grant / per-PDU ");
    }
    else
    {
        allocationInfo.m_isDynamic = false;
        NS_LOG_DEBUG("Selected scheduling type: SPS");
    }

    allocationInfo.m_harqEnabled =
        lcgMap.begin()->second->IsHarqEnabled(lcIdsbyPrio.rbegin()->second.front());

    // Remove all LCs that don't have the selected scheduling type
    // Find LcId of reference belonging to the LC with selected scheduling type, highest priority
    // and smallest LcId
    uint16_t nLcs = 0;
    uint16_t nRemainingLcs = 0;
    uint8_t lcIdOfRef = 0;
    for (auto itlcIdsbyPrio = lcIdsbyPrio.rbegin(); itlcIdsbyPrio != lcIdsbyPrio.rend();
         ++itlcIdsbyPrio)
    {
        uint8_t lowestLcId = std::numeric_limits<uint8_t>::max();
        for (auto itLcs = itlcIdsbyPrio->second.begin(); itLcs != itlcIdsbyPrio->second.end();)
        {
            nLcs++;
            if (lcgMap.begin()->second->IsLcDynamic(*itLcs) != dynamicGrant)
            {
                itLcs = itlcIdsbyPrio->second.erase(itLcs);
            }
            else
            {
                if (*itLcs < lowestLcId)
                {
                    lowestLcId = *itLcs;
                }
                ++itLcs;
                nRemainingLcs++;
            }
        }
        if (itlcIdsbyPrio->second.size() == 0)
        {
            itlcIdsbyPrio = std::reverse_iterator(lcIdsbyPrio.erase(--itlcIdsbyPrio.base()));
        }

        if (lowestLcId != std::numeric_limits<uint8_t>::max() && lcIdOfRef == 0)
        {
            lcIdOfRef = lowestLcId;
        }
    }
    // If SPS, remove all LCs with RRI different than the lcIdOfRef, and assign re-selection
    // counters
    if (!dynamicGrant)
    {
        for (auto itlcIdsbyPrio = lcIdsbyPrio.begin(); itlcIdsbyPrio != lcIdsbyPrio.end();
             ++itlcIdsbyPrio)
        {
            for (auto itLcs = itlcIdsbyPrio->second.begin(); itLcs != itlcIdsbyPrio->second.end();)
            {
                if (lcgMap.begin()->second->GetLcRri(*itLcs) !=
                    lcgMap.begin()->second->GetLcRri(lcIdOfRef))
                {
                    itLcs = itlcIdsbyPrio->second.erase(itLcs);
                    nRemainingLcs--;
                }
                else
                {
                    ++itLcs;
                }
            }
            if (itlcIdsbyPrio->second.size() == 0)
            {
                itlcIdsbyPrio = lcIdsbyPrio.erase(itlcIdsbyPrio);
            }
        }

        allocationInfo.m_rri = lcgMap.begin()->second->GetLcRri(lcIdOfRef);
        // Do it here because we need m_cResel for getting the candidate resources from the MAC
        m_reselCounter = GetRandomReselectionCounter(allocationInfo.m_rri);
        m_cResel = m_reselCounter * 10;
        NS_LOG_DEBUG("SPS Reselection counters: m_reselCounter " << +m_reselCounter << " m_cResel "
                                                                 << m_cResel);
    }
    allocationInfo.m_priority = lcgMap.begin()->second->GetLcPriority(lcIdOfRef);
    allocationInfo.m_castType = lcgMap.begin()->second->GetLcCastType(lcIdOfRef);
    NS_LOG_DEBUG("Number of LCs to attempt allocation for the selected destination: "
                 << nRemainingLcs << "/" << nLcs << ". LcId of reference " << +lcIdOfRef);

    // 2. Allocation of sidelink resources
    NS_LOG_DEBUG("Getting resources");
    // 2.1 Select which logical channels can be allocated
    std::map<uint8_t, std::vector<uint8_t>> selectedLcs = lcIdsbyPrio;
    std::queue<std::vector<uint8_t>> allocQueue;
    uint32_t bufferSize = 0;
    uint32_t nLcsInQueue = 0;
    uint32_t candResoTbSize = 0;
    uint8_t dstMcs = itDstInfo->second->GetDstMcs();
    // XXX Assume here that every slot has only 9 symbols (worst case with PSFCH)
    // We may need to refine this in the future depending on PSFCH configuration
    // If there is no PSFCH, then symbols per slot = 12.  If PSFCH period is 1,
    // then symbols per slot is 9.  If PSFCH period is 2 or 4, then there are
    // varying numbers of PSSCH symbols per slot.  If the number of subchannels
    // needed depends on whether there are 9 or 12 symbols per slot, then
    // this may need to be handled by the scheduler requesting for candidates
    // based on 12 symbols per slot, and then filtering out any resulting
    // candidates with only 9 symbols per slot.
    // with 9 slots
    uint16_t symbolsPerSlot = 9;
    uint16_t subChannelSize = GetMac()->GetNrSlSubChSize();
    auto rItSelectedLcs = selectedLcs.rbegin(); // reverse iterator
    while (selectedLcs.size() > 0)
    {
        allocQueue.emplace(rItSelectedLcs->second);
        // Calculate buffer size of LCs just pushed in the queue
        uint32_t currBufferSize = 0;
        for (auto& itLc : rItSelectedLcs->second)
        {
            currBufferSize = currBufferSize + lcgMap.begin()->second->GetTotalSizeOfLC(itLc);
        }
        nLcsInQueue = nLcsInQueue + rItSelectedLcs->second.size();
        // Calculate buffer size of all LCs currently in the queue
        bufferSize = bufferSize + currBufferSize;

        // Calculate number of needed subchannels
        //  The following do/while loop iterates until providing a transport
        //  block size large enough to cover the buffer size plus 5 bytes for
        //  SCI-2A information.
        uint16_t lSubch = 0;
        uint32_t tbSize = 0;
        do
        {
            lSubch++;
            tbSize = CalculateTbSize(GetAmc(), dstMcs, symbolsPerSlot, lSubch, subChannelSize);
        } while (tbSize < bufferSize + 5 && lSubch < GetTotalSubCh());

        NS_LOG_DEBUG("Trying " << nLcsInQueue << " LCs with total buffer size of " << bufferSize
                               << " bytes in " << lSubch << " subchannels for a TB size of "
                               << tbSize << " bytes");

        // All LCs in the set should have the same attributes than the lcIdOfRef
        NrSlUeMac::NrSlTransmissionParams params{lcgMap.begin()->second->GetLcPriority(lcIdOfRef),
                                                 lcgMap.begin()->second->GetLcPdb(lcIdOfRef),
                                                 lSubch,
                                                 lcgMap.begin()->second->GetLcRri(lcIdOfRef),
                                                 m_cResel};
        // GetCandidateResources() will return the set S_A defined in
        // sec. 8.1.4 of TS 38.214.  The scheduler is responsible for
        // further filtering out any candidates that overlap with already
        // scheduled grants within the selection window.
        auto filteredReso = FilterTxOpportunities(sfn,
                                                  GetMac()->GetCandidateResources(sfn, params),
                                                  lcgMap.begin()->second->GetLcRri(lcIdOfRef),
                                                  m_cResel);
        if (filteredReso.size() == 0)
        {
            NS_LOG_DEBUG("Resources not found");
            break;
        }
        else
        {
            NS_LOG_DEBUG("Resources found");
            candResoTbSize = tbSize;
            candResources = filteredReso;
        }
        rItSelectedLcs = std::reverse_iterator(selectedLcs.erase(--rItSelectedLcs.base()));
    }
    if (candResources.size() == 0)
    {
        NS_LOG_DEBUG("Unable to find resources");
        return 0;
    }
    allocationInfo.m_tbSize = candResoTbSize;
    NS_LOG_DEBUG("Destination L2 ID " << dstIdSelected << " got " << candResources.size()
                                      << " resources (of TB size " << candResoTbSize << ")"
                                      << " available to allocate " << nLcsInQueue
                                      << " LCs with total buffer size of " << bufferSize
                                      << " bytes");

    // 2.2 Allocate the resources to logical channels
    uint32_t allocatedSize = 0;
    while (allocQueue.size() > 0)
    {
        // All LCs of the same priority are served equally
        // Find how much to allocate to each
        uint32_t minBufferSize = std::numeric_limits<uint32_t>::max();
        uint32_t toServeBufferSize = 0;
        for (auto itLc : allocQueue.front())
        {
            if (lcgMap.begin()->second->GetTotalSizeOfLC(itLc) < minBufferSize)
            {
                minBufferSize = lcgMap.begin()->second->GetTotalSizeOfLC(itLc);
            }
        }
        toServeBufferSize = minBufferSize;
        if (allocQueue.front().size() * toServeBufferSize >
            candResoTbSize - allocatedSize - 5) // 5 bytes of SCI-2A
        {
            toServeBufferSize =
                std::floor((candResoTbSize - allocatedSize - 5) / allocQueue.front().size());
        }
        if (toServeBufferSize > 0)
        {
            // Allocate
            for (auto itLc : allocQueue.front())
            {
                SlRlcPduInfo slRlcPduInfo(itLc, toServeBufferSize);
                allocationInfo.m_allocatedRlcPdus.push_back(slRlcPduInfo);
                NS_LOG_INFO("LC ID " << +itLc << " Dst L2ID " << dstIdSelected << " allocated "
                                     << toServeBufferSize << " bytes");
                allocatedSize = allocatedSize + toServeBufferSize;
            }
        }
        else
        {
            break;
        }

        allocQueue.pop();
    }

    return dstIdSelected;
}

void
NrSlUeMacSchedulerFixedMcs::GetDstsAndLcsNeedingScheduling(
    const SfnSf& sfn,
    std::map<uint32_t, std::vector<uint8_t>>& dstsAndLcsToSched)
{
    NS_LOG_FUNCTION(this << sfn);
    for (auto& itDstInfo : m_dstMap)
    {
        const auto& lcgMap = itDstInfo.second->GetNrSlLCG(); // Map of unique_ptr should not copy
        std::vector<uint8_t> lcVector = lcgMap.begin()->second->GetLCId();
        std::vector<uint8_t> passedLcsVector;
        for (auto& itLcId : lcVector)
        {
            if (TxResourceReselectionCheck(sfn, itDstInfo.first, itLcId))
            {
                passedLcsVector.emplace_back(itLcId);
            }
        }
        if (passedLcsVector.size() > 0)
        {
            dstsAndLcsToSched.emplace(itDstInfo.first, passedLcsVector);
        }
        NS_LOG_DEBUG("Destination L2 ID " << itDstInfo.first << " has " << passedLcsVector.size()
                                          << " LCs needing scheduling");
    }
}

void
NrSlUeMacSchedulerFixedMcs::AttemptGrantAllocation(const SfnSf& sfn,
                                                   uint32_t dstL2Id,
                                                   const std::list<SlResourceInfo>& candResources,
                                                   const AllocationInfo& allocationInfo)
{
    NS_LOG_FUNCTION(this << sfn << dstL2Id);

    std::set<SlGrantResource> allocList;

    const auto itDstInfo = m_dstMap.find(dstL2Id);
    bool allocated = DoNrSlAllocation(candResources, itDstInfo->second, allocList, allocationInfo);

    if (!allocated)
    {
        return;
    }

    if (allocationInfo.m_isDynamic)
    {
        CreateSinglePduGrant(sfn, allocList, allocationInfo);
    }
    else
    {
        CreateSpsGrant(sfn, allocList, allocationInfo);
    }
}

Time
NrSlUeMacSchedulerFixedMcs::GetSpsGrantTimeout(const SfnSf& sfn,
                                               uint8_t resoReselCounter,
                                               Time rri) const
{
    NS_LOG_FUNCTION(this << sfn << +resoReselCounter << rri.As(Time::MS));
    auto timePerSlot = MicroSeconds(1000 >> sfn.GetNumerology());
    // Set a conservative timeout value.  The grant will be reselected
    // at (resoReselCounter * RRI) in the future, and add one more
    // RRI to this value to prevent cases where the HARQ process ID timer
    // expires just before the scheduler was about to renew it.
    auto timeout = rri * (resoReselCounter + 1);
    return timeout;
}

void
NrSlUeMacSchedulerFixedMcs::CreateSpsGrant(const SfnSf& sfn,
                                           const std::set<SlGrantResource>& slotAllocList,
                                           const AllocationInfo& allocationInfo)
{
    NS_LOG_FUNCTION(this << sfn);
    // m_grantInfo is a map with key dstL2Id and value std::vector<GrantInfo>
    auto itVecGrantInfo = m_grantInfo.find(slotAllocList.begin()->dstL2Id);
    if (itVecGrantInfo == m_grantInfo.end())
    {
        NS_LOG_DEBUG("New destination " << slotAllocList.begin()->dstL2Id);
        GrantInfo grant = CreateSpsGrantInfo(slotAllocList, allocationInfo);
        auto timeout = GetSpsGrantTimeout(sfn, grant.slResoReselCounter, allocationInfo.m_rri);
        auto harqId =
            GetMacHarq()->AllocateHarqProcessId(slotAllocList.begin()->dstL2Id, true, timeout);
        if (!harqId.has_value())
        {
            NS_LOG_WARN("Unable to create grant, HARQ Id not available");
            return;
        }
        grant.harqId = harqId.value();
        // To this point, the 'harqEnabled' flag means that either blind or
        // HARQ feedback transmissions are enabled.  However, the semantics
        // of this flag for a published grant are that harqEnabled refers
        // only to whether HARQ feedback is enabled
        grant.harqEnabled = allocationInfo.m_harqEnabled && GetMac()->GetPsfchPeriod();
        grant.castType = allocationInfo.m_castType;
        std::vector<GrantInfo> grantVector;
        grantVector.push_back(grant);
        NotifyGrantCreated(grant);
        itVecGrantInfo =
            m_grantInfo.emplace(std::make_pair(slotAllocList.begin()->dstL2Id, grantVector)).first;
        NS_LOG_INFO("New SPS grant created to new destination "
                    << slotAllocList.begin()->dstL2Id << " with HARQ ID " << +grant.harqId
                    << " HARQ enabled " << +grant.harqEnabled);
    }
    else
    {
        NS_LOG_DEBUG("Destination " << slotAllocList.begin()->dstL2Id << " found");
        // Destination exists
        bool grantFound = false;
        auto itGrantVector = itVecGrantInfo->second.begin();
        for (; itGrantVector != itVecGrantInfo->second.end(); ++itGrantVector)
        {
            NS_ASSERT_MSG(itGrantVector->slotAllocations.size(),
                          "No slots associated with grant to " << slotAllocList.begin()->dstL2Id);
            if (itGrantVector->rri == allocationInfo.m_rri &&
                itGrantVector->slotAllocations.begin()->slRlcPduInfo.size() ==
                    slotAllocList.begin()->slRlcPduInfo.size())
            {
                uint16_t foundLcs = 0;
                for (auto itGrantRlcPdu : itGrantVector->slotAllocations.begin()->slRlcPduInfo)
                {
                    for (auto itNewRlcPdu : slotAllocList.begin()->slRlcPduInfo)
                    {
                        if (itGrantRlcPdu.lcid == itNewRlcPdu.lcid)
                        {
                            NS_LOG_DEBUG("Found matching logical channel ID "
                                         << +itGrantRlcPdu.lcid << " in existing grant");
                            foundLcs++;
                            break;
                        }
                    }
                }
                NS_LOG_DEBUG("Checking if the found LCs "
                             << foundLcs << " matches the slRlcPduInfo.size() "
                             << itGrantVector->slotAllocations.begin()->slRlcPduInfo.size());
                if (foundLcs == itGrantVector->slotAllocations.begin()->slRlcPduInfo.size())
                {
                    grantFound = true;
                    break;
                    // itGrantVector normally points to the found grant at this point
                }
            }
        }
        if (grantFound)
        {
            // This case corresponds to slResoReselCounter going to zero but
            // the grant still existing-- can it happen?
            // If this is reachable code, the below needs to be reworked
            // to avoid copying harq ID to a new grant without updating the timer
            NS_FATAL_ERROR("Check whether this code is unreachable");
            // Update
            NS_ASSERT_MSG(
                itGrantVector->slResoReselCounter == 0,
                "Sidelink resource counter must be zero before assigning new grant for dst "
                    << slotAllocList.begin()->dstL2Id);
            uint8_t prevHarqId = itGrantVector->harqId;
            GrantInfo grant = CreateSpsGrantInfo(slotAllocList, allocationInfo);
            *itGrantVector = grant;
            itGrantVector->harqId = prevHarqId; // Preserve previous ID
            NS_LOG_INFO("Updated SPS grant to destination "
                        << slotAllocList.begin()->dstL2Id << " with HARQ ID "
                        << itGrantVector->harqId << " HARQ enabled " << +grant.harqEnabled);
        }
        else
        {
            // Insert
            GrantInfo grant = CreateSpsGrantInfo(slotAllocList, allocationInfo);
            auto timeout = GetSpsGrantTimeout(sfn, grant.slResoReselCounter, allocationInfo.m_rri);
            auto harqId =
                GetMacHarq()->AllocateHarqProcessId(slotAllocList.begin()->dstL2Id, true, timeout);
            if (!harqId.has_value())
            {
                NS_LOG_WARN("Unable to create grant, HARQ Id not available");
                return;
            }
            grant.harqId = harqId.value();
            // To this point, the 'harqEnabled' flag means that either blind or
            // HARQ feedback transmissions are enabled.  However, the semantics
            // of this flag for a published grant are that harqEnabled refers
            // only to whether HARQ feedback is enabled
            grant.harqEnabled = allocationInfo.m_harqEnabled && GetMac()->GetPsfchPeriod();
            grant.castType = allocationInfo.m_castType;
            itVecGrantInfo->second.push_back(grant);
            NotifyGrantCreated(grant);
            NS_LOG_INFO("New SPS grant created to existing destination "
                        << slotAllocList.begin()->dstL2Id << " with HARQ ID " << +grant.harqId
                        << " HARQ enabled " << +grant.harqEnabled);
        }
    }
}

Time
NrSlUeMacSchedulerFixedMcs::GetDynamicGrantTimeout(const SfnSf& sfn,
                                                   const std::set<SlGrantResource>& slotAllocList,
                                                   bool harqEnabled,
                                                   uint16_t psfchPeriod) const
{
    NS_LOG_FUNCTION(this << sfn << slotAllocList.size() << harqEnabled << psfchPeriod);
    NS_ABORT_MSG_UNLESS(slotAllocList.size(), "Grant has no allocated slots");
    auto timePerSlot = MicroSeconds(1000 >> sfn.GetNumerology());
    auto it = slotAllocList.end();
    it--; // it points to the last element of the list
    NS_ASSERT_MSG(it->sfn.Normalize() >= sfn.Normalize(), "allocation occurs in the past");
    // Current time is (sfn.Normalize() * timePerSlot)
    // The last grant transmission time will be at time (it->sfn.Normalize() * timePerSlot)
    // If there is no HARQ feedback, we can set the time to one slot beyond
    // the last grant transmission time
    if (!(harqEnabled && psfchPeriod))
    {
        auto timeout = timePerSlot * (it->sfn.Normalize() + 1 - sfn.Normalize());
        NS_LOG_DEBUG("Timeout (without HARQ FB): " << timeout.As(Time::US));
        return timeout;
    }
    // PSFCH feedback will usually be delivered in the first PSFCH-enabled slot after the
    // MinTimeGapPsfch has elapsed Therefore, find the this PSFCH-enabled slot, and set the timeout
    // value to (PSFCH-enabled slot + 1 - current slot) * timePerSlot
    SfnSf futureSlot = it->sfn;
    futureSlot.Add(1);
    while (!GetMac()->SlotHasPsfch(futureSlot))
    {
        futureSlot.Add(1);
    }
    auto timeout = timePerSlot * (futureSlot.Normalize() + 1 - sfn.Normalize());
    NS_LOG_DEBUG("Timeout (with HARQ FB): " << timeout.As(Time::US));
    return timeout;
}

void
NrSlUeMacSchedulerFixedMcs::CreateSinglePduGrant(const SfnSf& sfn,
                                                 const std::set<SlGrantResource>& slotAllocList,
                                                 const AllocationInfo& allocationInfo)
{
    NS_LOG_FUNCTION(this << sfn);
    auto itGrantInfo = m_grantInfo.find(slotAllocList.begin()->dstL2Id);

    if (itGrantInfo == m_grantInfo.end())
    {
        // New destination
        NS_LOG_DEBUG("New destination " << slotAllocList.begin()->dstL2Id);
        auto timeout = GetDynamicGrantTimeout(sfn,
                                              slotAllocList,
                                              allocationInfo.m_harqEnabled,
                                              GetMac()->GetPsfchPeriod());
        auto harqId =
            GetMacHarq()->AllocateHarqProcessId(slotAllocList.begin()->dstL2Id, false, timeout);
        if (!harqId.has_value())
        {
            NS_LOG_WARN("Unable to create grant, HARQ Id not available");
            return;
        }
        GrantInfo grant = CreateSinglePduGrantInfo(slotAllocList, allocationInfo);
        grant.harqId = harqId.value();
        // To this point, the 'harqEnabled' flag means that either blind or
        // HARQ feedback transmissions are enabled.  However, the semantics
        // of this flag for a published grant are that harqEnabled refers
        // only to whether HARQ feedback is enabled
        grant.harqEnabled = allocationInfo.m_harqEnabled && GetMac()->GetPsfchPeriod();
        grant.castType = allocationInfo.m_castType;
        NotifyGrantCreated(grant);
        std::vector<GrantInfo> grantVector;
        grantVector.push_back(grant);
        itGrantInfo =
            m_grantInfo.emplace(std::make_pair(slotAllocList.begin()->dstL2Id, grantVector)).first;
        NS_LOG_INFO("New dynamic grant created to new destination "
                    << slotAllocList.begin()->dstL2Id << " with HARQ ID " << +grant.harqId
                    << " HARQ enabled " << +grant.harqEnabled);
    }
    else
    {
        // Destination exists
        NS_LOG_DEBUG("Destination " << slotAllocList.begin()->dstL2Id << " found");
        bool grantFound = false;
        auto itGrantVector = itGrantInfo->second.begin();
        for (; itGrantVector != itGrantInfo->second.end(); ++itGrantVector)
        {
            if (itGrantVector->slotAllocations.begin()->slRlcPduInfo.size() ==
                slotAllocList.begin()->slRlcPduInfo.size())
            {
                uint16_t foundLcs = 0;
                for (auto itGrantRlcPdu : itGrantVector->slotAllocations.begin()->slRlcPduInfo)
                {
                    for (auto itNewRlcPdu : slotAllocList.begin()->slRlcPduInfo)
                    {
                        if (itGrantRlcPdu.lcid == itNewRlcPdu.lcid)
                        {
                            foundLcs++;
                            break;
                        }
                    }
                }
                if (foundLcs == itGrantVector->slotAllocations.begin()->slRlcPduInfo.size())
                {
                    grantFound = true;
                    break;
                    // itGrantVector normally points to the found grant at this point
                }
            }
        }
        if (grantFound)
        {
            NS_FATAL_ERROR("Attempt to update dynamic grant-- shouldn't happen");
        }
        else
        {
            // Insert
            auto timeout = GetDynamicGrantTimeout(sfn,
                                                  slotAllocList,
                                                  allocationInfo.m_harqEnabled,
                                                  GetMac()->GetPsfchPeriod());
            NS_LOG_INFO("Inserting dynamic grant with timeout of " << timeout.As(Time::MS));
            auto harqId =
                GetMacHarq()->AllocateHarqProcessId(slotAllocList.begin()->dstL2Id, false, timeout);
            if (!harqId.has_value())
            {
                NS_LOG_WARN("Unable to create grant, HARQ Id not available");
                return;
            }
            GrantInfo grant = CreateSinglePduGrantInfo(slotAllocList, allocationInfo);
            grant.harqId = harqId.value();
            // To this point, the 'harqEnabled' flag means that either blind or
            // HARQ feedback transmissions are enabled.  However, the semantics
            // of this flag for a published grant are that harqEnabled refers
            // only to whether HARQ feedback is enabled
            grant.harqEnabled = allocationInfo.m_harqEnabled && GetMac()->GetPsfchPeriod();
            grant.castType = allocationInfo.m_castType;
            NotifyGrantCreated(grant);
            itGrantInfo->second.push_back(grant);
            NS_LOG_INFO("New dynamic grant created to existing destination "
                        << slotAllocList.begin()->dstL2Id << " with HARQ ID " << +grant.harqId
                        << " HARQ enabled " << +grant.harqEnabled);
        }
    }
}

NrSlUeMacScheduler::GrantInfo
NrSlUeMacSchedulerFixedMcs::CreateSpsGrantInfo(const std::set<SlGrantResource>& slotAllocList,
                                               const AllocationInfo& allocationInfo) const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG((m_reselCounter != 0),
                  "Can not create SPS grants with 0 Resource selection counter");
    NS_ASSERT_MSG((m_cResel != 0), "Can not create SPS grants with 0 cResel counter");
    NS_ASSERT_MSG((!allocationInfo.m_rri.IsZero()), "Can not create SPS grants with 0 RRI");

    NS_LOG_DEBUG("Creating SPS grants for dstL2Id " << slotAllocList.begin()->dstL2Id);
    NS_LOG_DEBUG("Resource reservation interval " << allocationInfo.m_rri.GetMilliSeconds()
                                                  << " ms");
    NS_LOG_DEBUG("Resel Counter " << +m_reselCounter << " and cResel " << m_cResel);

    uint16_t resPeriodSlots = GetMac()->GetResvPeriodInSlots(allocationInfo.m_rri);
    GrantInfo grant;

    grant.cReselCounter = m_cResel;
    // save reselCounter to be used if probability of keeping the resource would
    // be higher than the configured one
    grant.prevSlResoReselCounter = m_reselCounter;
    grant.slResoReselCounter = m_reselCounter;

    // if further IDs are needed and the std::deque needs to be popped from
    // front, need to copy the std::deque to remove its constness
    grant.nSelected = static_cast<uint8_t>(slotAllocList.size());
    grant.rri = allocationInfo.m_rri;
    grant.castType = allocationInfo.m_castType;
    NS_LOG_DEBUG("nSelected = " << +grant.nSelected);

    for (uint16_t i = 0; i < m_reselCounter; i++)
    {
        for (const auto& it : slotAllocList)
        {
            auto slAlloc = it;
            slAlloc.sfn.Add(i * resPeriodSlots);

            if (slAlloc.ndi == 1)
            {
                NS_LOG_INFO("  SPS NDI scheduled at: Frame = "
                            << slAlloc.sfn.GetFrame() << " SF = " << +slAlloc.sfn.GetSubframe()
                            << " slot = " << +slAlloc.sfn.GetSlot()
                            << " normalized = " << slAlloc.sfn.Normalize()
                            << " subchannels = " << slAlloc.slPsschSubChStart << ":"
                            << slAlloc.slPsschSubChStart + slAlloc.slPsschSubChLength - 1);
            }
            else
            {
                NS_LOG_INFO("  SPS rtx scheduled at: Frame = "
                            << slAlloc.sfn.GetFrame() << " SF = " << +slAlloc.sfn.GetSubframe()
                            << " slot = " << +slAlloc.sfn.GetSlot()
                            << " normalized = " << slAlloc.sfn.Normalize()
                            << " subchannels = " << slAlloc.slPsschSubChStart << ":"
                            << slAlloc.slPsschSubChStart + slAlloc.slPsschSubChLength - 1);
            }
            // Future slot may not have the same PSFCH status as the original slot
            slAlloc.slHasPsfch = GetMac()->SlotHasPsfch(slAlloc.sfn);
            slAlloc.slPsschSymLength = slAlloc.slHasPsfch ? 9 : 12;
            bool insertStatus = grant.slotAllocations.emplace(slAlloc).second;
            NS_ASSERT_MSG(insertStatus, "slot allocation already exist");
        }
    }

    return grant;
}

NrSlUeMacScheduler::GrantInfo
NrSlUeMacSchedulerFixedMcs::CreateSinglePduGrantInfo(const std::set<SlGrantResource>& slotAllocList,
                                                     const AllocationInfo& allocationInfo) const
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Creating single-PDU grant for dstL2Id " << slotAllocList.begin()->dstL2Id);

    GrantInfo grant;
    grant.nSelected = static_cast<uint8_t>(slotAllocList.size());
    grant.isDynamic = true;
    grant.castType = allocationInfo.m_castType;

    NS_LOG_DEBUG("nSelected = " << +grant.nSelected);

    for (const auto& it : slotAllocList)
    {
        auto slAlloc = it;
        if (slAlloc.ndi == 1)
        {
            NS_LOG_INFO("  Dynamic NDI scheduled at: Frame = "
                        << slAlloc.sfn.GetFrame() << " SF = " << +slAlloc.sfn.GetSubframe()
                        << " slot = " << +slAlloc.sfn.GetSlot() << " normalized = "
                        << slAlloc.sfn.Normalize() << " subchannels = " << slAlloc.slPsschSubChStart
                        << ":" << slAlloc.slPsschSubChStart + slAlloc.slPsschSubChLength - 1);
        }
        else
        {
            NS_LOG_INFO("  Dynamic rtx scheduled at: Frame = "
                        << slAlloc.sfn.GetFrame() << " SF = " << +slAlloc.sfn.GetSubframe()
                        << " slot = " << +slAlloc.sfn.GetSlot() << " normalized = "
                        << slAlloc.sfn.Normalize() << " subchannels = " << slAlloc.slPsschSubChStart
                        << ":" << slAlloc.slPsschSubChStart + slAlloc.slPsschSubChLength - 1);
        }
        bool insertStatus = grant.slotAllocations.emplace(slAlloc).second;
        NS_ASSERT_MSG(insertStatus, "slot allocation already exist");
    }
    return grant;
}

void
NrSlUeMacSchedulerFixedMcs::CheckForGrantsToPublish(const SfnSf& sfn)
{
    NS_LOG_FUNCTION(this << sfn.Normalize());
    for (auto itGrantInfo = m_grantInfo.begin(); itGrantInfo != m_grantInfo.end(); itGrantInfo++)
    {
        for (auto itGrantVector = itGrantInfo->second.begin();
             itGrantVector != itGrantInfo->second.end();)
        {
            if (!itGrantVector->isDynamic && itGrantVector->slResoReselCounter == 0)
            {
                ++itGrantVector;
                continue;
            }

            if (itGrantVector->slotAllocations.begin()->sfn.Normalize() > sfn.Normalize() + m_t1)
            {
                ++itGrantVector;
                continue;
            }
            // The next set of slots (NDI + any retransmissions) should be added
            // to a grant, and deleted from m_grantInfo
            auto slotIt = itGrantVector->slotAllocations.begin();
            NS_ASSERT_MSG(slotIt->ndi == 1, "New data indication not found");
            NS_ASSERT_MSG(slotIt->sfn.Normalize() >= sfn.Normalize(), "Stale slot in m_grantInfo");
            SlGrantResource currentSlot = *slotIt;
            NS_LOG_DEBUG("Slot at : Frame = " << currentSlot.sfn.GetFrame()
                                              << " SF = " << +currentSlot.sfn.GetSubframe()
                                              << " slot = " << +currentSlot.sfn.GetSlot());
            uint32_t tbSize = 0;
            // sum all the assigned bytes to each LC of this destination
            for (const auto& it : currentSlot.slRlcPduInfo)
            {
                NS_LOG_DEBUG("LC " << static_cast<uint16_t>(it.lcid) << " was assigned " << it.size
                                   << " bytes");
                tbSize += it.size;
            }
            itGrantVector->tbTxCounter = 1;
            NrSlUeMac::NrSlGrant grant;
            grant.harqId = itGrantVector->harqId;
            grant.nSelected = itGrantVector->nSelected;
            grant.tbTxCounter = itGrantVector->tbTxCounter;
            grant.tbSize = tbSize;
            grant.rri = itGrantVector->rri;
            grant.harqEnabled = itGrantVector->harqEnabled;
            grant.castType = itGrantVector->castType;
            // Add the NDI slot and retransmissions to the set of slot allocations
            m_publishedGrants.emplace_back(currentSlot);
            grant.slotAllocations.emplace(currentSlot);
            itGrantVector->slotAllocations.erase(slotIt);
            // Add any retransmission slots and erase them
            slotIt = itGrantVector->slotAllocations.begin();
            while (slotIt != itGrantVector->slotAllocations.end() && slotIt->ndi == 0)
            {
                SlGrantResource nextSlot = *slotIt;
                m_publishedGrants.emplace_back(nextSlot);
                grant.slotAllocations.emplace(nextSlot);
                itGrantVector->slotAllocations.erase(slotIt);
                slotIt = itGrantVector->slotAllocations.begin();
            }
            GetMac()->SchedNrSlConfigInd(currentSlot.dstL2Id, grant);
            NotifyGrantPublished(grant);
            NS_LOG_INFO("Publishing grant with " << grant.slotAllocations.size()
                                                 << " slots to destination " << currentSlot.dstL2Id
                                                 << " HARQ ID " << +grant.harqId);
            if (itGrantVector->isDynamic || !itGrantVector->slotAllocations.size())
            {
                itGrantVector = itGrantInfo->second.erase(itGrantVector);
            }
            else
            {
                // Decrement counters for reselection
                --itGrantVector->slResoReselCounter;
                --itGrantVector->cReselCounter;
                ++itGrantVector;
            }
        }
    }
}

bool
NrSlUeMacSchedulerFixedMcs::OverlappedResources(const SfnSf& firstSfn,
                                                uint16_t firstStart,
                                                uint16_t firstLength,
                                                const SfnSf& secondSfn,
                                                uint16_t secondStart,
                                                uint16_t secondLength) const
{
    NS_ASSERT_MSG(firstLength && secondLength, "Length should not be zero");
    if (firstSfn == secondSfn)
    {
        if (std::max(firstStart, secondStart) <
            std::min(firstStart + firstLength, secondStart + secondLength))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

std::list<SlResourceInfo>
NrSlUeMacSchedulerFixedMcs::FilterTxOpportunities(const SfnSf& sfn,
                                                  std::list<SlResourceInfo> txOppr,
                                                  Time rri,
                                                  uint16_t cResel)
{
    NS_LOG_FUNCTION(this << sfn.Normalize() << txOppr.size() << rri.As(Time::MS) << cResel);

    if (txOppr.empty())
    {
        return txOppr;
    }

    NS_LOG_DEBUG("Filtering txOppr list of size " << txOppr.size() << " resources");
    auto itPublished = m_publishedGrants.begin();
    while (itPublished != m_publishedGrants.end())
    {
        auto itTxOppr = txOppr.begin();
        while (itTxOppr != txOppr.end())
        {
            if (m_allowMultipleDestinationsPerSlot)
            {
                if (OverlappedResources(itPublished->sfn,
                                        itPublished->slPsschSubChStart,
                                        itPublished->slPsschSubChLength,
                                        itTxOppr->sfn,
                                        itTxOppr->slSubchannelStart,
                                        itTxOppr->slSubchannelLength))
                {
                    NS_LOG_DEBUG("Erasing candidate " << itTxOppr->sfn.Normalize()
                                                      << " due to published grant overlap");
                    itTxOppr = txOppr.erase(itTxOppr);
                }
                else
                {
                    ++itTxOppr;
                }
            }
            else
            {
                if (itPublished->sfn == itTxOppr->sfn)
                {
                    NS_LOG_INFO("Erasing candidate " << itTxOppr->sfn.Normalize()
                                                     << " due to published grant overlap");
                    NS_LOG_INFO("Erasing candidate " << itTxOppr->sfn.Normalize());
                    itTxOppr = txOppr.erase(itTxOppr);
                }
                else
                {
                    ++itTxOppr;
                }
            }
        }
        // Erase published records in the past
        if (itPublished->sfn < sfn)
        {
            NS_LOG_INFO("Erasing published grant from " << itPublished->sfn.Normalize());
            itPublished = m_publishedGrants.erase(itPublished);
        }
        else
        {
            ++itPublished;
        }
    }
    for (const auto& itDst : m_grantInfo)
    {
        for (auto itGrantVector = itDst.second.begin(); itGrantVector != itDst.second.end();
             ++itGrantVector)
        {
            for (auto itGrantAlloc = itGrantVector->slotAllocations.begin();
                 itGrantAlloc != itGrantVector->slotAllocations.end();
                 itGrantAlloc++)
            {
                auto itTxOppr = txOppr.begin();
                while (itTxOppr != txOppr.end())
                {
                    // need to consider this txOppr plus its potential repetitions
                    bool foundOverlap = false;
                    for (uint16_t i = 0; i <= cResel; i++)
                    {
                        SfnSf candidateSfn =
                            itTxOppr->sfn.GetFutureSfnSf(i * rri.GetMilliSeconds() * 4);
                        if (itGrantAlloc->sfn < candidateSfn)
                        {
                            break;
                        }
                        if (m_allowMultipleDestinationsPerSlot)
                        {
                            if (OverlappedResources(itGrantAlloc->sfn,
                                                    itGrantAlloc->slPsschSubChStart,
                                                    itGrantAlloc->slPsschSubChLength,
                                                    candidateSfn,
                                                    itTxOppr->slSubchannelStart,
                                                    itTxOppr->slSubchannelLength))
                            {
                                foundOverlap = true;
                                break;
                            }
                        }
                        else
                        {
                            // Disallow scheduling again on a previously scheduled slot
                            if (itGrantAlloc->sfn == candidateSfn)
                            {
                                foundOverlap = true;
                                break;
                            }
                        }
                    }
                    if (foundOverlap)
                    {
                        NS_LOG_DEBUG("Erasing candidate " << itTxOppr->sfn.Normalize());
                        itTxOppr = txOppr.erase(itTxOppr);
                    }
                    else
                    {
                        ++itTxOppr;
                    }
                }
            }
        }
    }
    return txOppr;
}

uint8_t
NrSlUeMacSchedulerFixedMcs::GetTotalSubCh() const
{
    return GetMac()->GetTotalSubCh();
}

uint8_t
NrSlUeMacSchedulerFixedMcs::GetSlMaxTxTransNumPssch() const
{
    return GetMac()->GetSlMaxTxTransNumPssch();
}

uint8_t
NrSlUeMacSchedulerFixedMcs::GetRv(uint8_t txNumTb) const
{
    NS_LOG_FUNCTION(this << +txNumTb);
    uint8_t modulo = txNumTb % 4;
    // we assume rvid = 0, so RV would take 0, 2, 3, 1
    // see TS 38.21 table 6.1.2.1-2
    uint8_t rv = 0;
    switch (modulo)
    {
    case 0:
        rv = 0;
        break;
    case 1:
        rv = 2;
        break;
    case 2:
        rv = 3;
        break;
    case 3:
        rv = 1;
        break;
    default:
        NS_ABORT_MSG("Wrong modulo result to deduce RV");
    }

    return rv;
}

int64_t
NrSlUeMacSchedulerFixedMcs::AssignStreams(int64_t stream)
{
    NS_LOG_FUNCTION(this << stream);
    m_grantSelectionUniformVariable->SetStream(stream);
    m_destinationUniformVariable->SetStream(stream + 1);
    m_ueSelectedUniformVariable->SetStream(stream + 2);
    return 3;
}

void
NrSlUeMacSchedulerFixedMcs::DoDispose()
{
    NS_LOG_FUNCTION(this);
}

uint32_t
NrSlUeMacSchedulerFixedMcs::CalculateTbSize(Ptr<const NrAmc> nrAmc,
                                            uint8_t dstMcs,
                                            uint16_t symbolsPerSlot,
                                            uint16_t availableSubChannels,
                                            uint16_t subChannelSize) const
{
    NS_LOG_FUNCTION(this << nrAmc << dstMcs << symbolsPerSlot << availableSubChannels
                         << subChannelSize);
    NS_ASSERT_MSG(availableSubChannels > 0, "Must have at least one available subchannel");
    NS_ASSERT_MSG(subChannelSize > 0, "Must have non-zero subChannelSize");
    NS_ASSERT_MSG(symbolsPerSlot <= 14, "Invalid number of symbols per slot");
    uint8_t slRank{1}; // XXX get the sidelink rank from somewhere
    return nrAmc->CalculateTbSize(dstMcs,
                                  slRank,
                                  subChannelSize * availableSubChannels * symbolsPerSlot);
}

bool
NrSlUeMacSchedulerFixedMcs::DoNrSlAllocation(
    const std::list<SlResourceInfo>& candResources,
    const std::shared_ptr<NrSlUeMacSchedulerDstInfo>& dstInfo,
    std::set<SlGrantResource>& slotAllocList,
    const AllocationInfo& allocationInfo)
{
    NS_LOG_FUNCTION(this);
    bool allocated = false;
    NS_ASSERT_MSG(candResources.size() > 0,
                  "Scheduler received an empty resource list from UE MAC");

    std::list<SlResourceInfo> selectedTxOpps;
    // blind retransmission corresponds to HARQ enabled AND (PSFCH period == 0)
    if (allocationInfo.m_harqEnabled && (GetMac()->GetPsfchPeriod() == 0))
    {
        // Select up to N_PSSCH_maxTx resources without regard MinTimeGapPsfch
        // i.e., for blind retransmissions
        selectedTxOpps = SelectResourcesForBlindRetransmissions(candResources);
    }
    else
    {
        selectedTxOpps = SelectResourcesWithConstraint(candResources, allocationInfo.m_harqEnabled);
    }
    NS_ASSERT_MSG(selectedTxOpps.size() > 0, "Scheduler should select at least 1 slot from txOpps");
    allocated = true;
    auto itTxOpps = selectedTxOpps.cbegin();
    for (; itTxOpps != selectedTxOpps.cend(); ++itTxOpps)
    {
        SlGrantResource slotAlloc;
        slotAlloc.sfn = itTxOpps->sfn;
        slotAlloc.dstL2Id = dstInfo->GetDstL2Id();
        slotAlloc.priority = allocationInfo.m_priority;
        slotAlloc.slRlcPduInfo = allocationInfo.m_allocatedRlcPdus;
        slotAlloc.mcs = dstInfo->GetDstMcs();
        // PSCCH
        slotAlloc.numSlPscchRbs = itTxOpps->numSlPscchRbs;
        slotAlloc.slPscchSymStart = itTxOpps->slPscchSymStart;
        slotAlloc.slPscchSymLength = itTxOpps->slPscchSymLength;
        // PSSCH
        slotAlloc.slPsschSymStart = itTxOpps->slPsschSymStart;
        slotAlloc.slPsschSymLength = itTxOpps->slPsschSymLength;
        slotAlloc.slPsschSubChStart = itTxOpps->slSubchannelStart;
        slotAlloc.slPsschSubChLength = itTxOpps->slSubchannelLength;
        slotAlloc.maxNumPerReserve = itTxOpps->slMaxNumPerReserve;
        slotAlloc.ndi = slotAllocList.empty() == true ? 1 : 0;
        slotAlloc.rv = GetRv(static_cast<uint8_t>(slotAllocList.size()));
        if (static_cast<uint16_t>(slotAllocList.size()) % itTxOpps->slMaxNumPerReserve == 0)
        {
            slotAlloc.txSci1A = true;
            if (slotAllocList.size() + itTxOpps->slMaxNumPerReserve <= selectedTxOpps.size())
            {
                slotAlloc.slotNumInd = itTxOpps->slMaxNumPerReserve;
            }
            else
            {
                slotAlloc.slotNumInd = selectedTxOpps.size() - slotAllocList.size();
            }
        }
        else
        {
            slotAlloc.txSci1A = false;
            // Slot, which does not carry SCI 1-A can not indicate future TXs
            slotAlloc.slotNumInd = 0;
        }

        slotAllocList.emplace(slotAlloc);
    }
    return allocated;
}

bool
NrSlUeMacSchedulerFixedMcs::OverlappedSlots(const std::list<SlResourceInfo>& resources,
                                            const SlResourceInfo& candidate) const
{
    for (const auto& it : resources)
    {
        if (it.sfn == candidate.sfn)
        {
            return true;
        }
    }
    return false;
}

std::list<SlResourceInfo>
NrSlUeMacSchedulerFixedMcs::SelectResourcesForBlindRetransmissions(std::list<SlResourceInfo> txOpps)
{
    NS_LOG_FUNCTION(this << txOpps.size());

    uint8_t totalTx = GetSlMaxTxTransNumPssch();
    std::list<SlResourceInfo> newTxOpps;

    if (txOpps.size() > totalTx)
    {
        while (newTxOpps.size() != totalTx && txOpps.size() > 0)
        {
            auto txOppsIt = txOpps.begin();
            // Advance to the randomly selected element
            std::advance(txOppsIt,
                         m_grantSelectionUniformVariable->GetInteger(0, txOpps.size() - 1));
            if (!OverlappedSlots(newTxOpps, *txOppsIt))
            {
                // copy the randomly selected slot info into the new list
                newTxOpps.emplace_back(*txOppsIt);
            }
            // erase the selected one from the list
            txOppsIt = txOpps.erase(txOppsIt);
        }
    }
    else
    {
        // Try to use each available slot
        auto txOppsIt = txOpps.begin();
        while (txOppsIt != txOpps.end())
        {
            if (!OverlappedSlots(newTxOpps, *txOppsIt))
            {
                // copy the slot info into the new list
                newTxOpps.emplace_back(*txOppsIt);
            }
            // erase the selected one from the list
            txOppsIt = txOpps.erase(txOppsIt);
        }
    }

    // sort the list by SfnSf before returning
    newTxOpps.sort();
    NS_ASSERT_MSG(newTxOpps.size() <= totalTx,
                  "Number of randomly selected slots exceeded total number of TX");
    return newTxOpps;
}

std::list<SlResourceInfo>
NrSlUeMacSchedulerFixedMcs::SelectResourcesWithConstraint(std::list<SlResourceInfo> txOpps,
                                                          bool harqEnabled)
{
    NS_LOG_FUNCTION(this << txOpps.size() << harqEnabled);
    uint8_t totalTx = 1;
    if (harqEnabled)
    {
        totalTx = GetSlMaxTxTransNumPssch();
    }
    std::list<SlResourceInfo> newTxOpps;
    std::size_t originalSize [[maybe_unused]] = txOpps.size();

    // TS 38.321 states to randomly select a resource from the available
    // pool, and then to proceed to select additional resources at random
    // such that the minimum time gap between any two selected resources
    // in case that PSFCH is configured for this pool of resources and
    // that a retransmission resource can be indicated by the time resource
    // assignment of a prior SCI according to clause 8.3.1.1 of TS 38.212

    // *txOppsIt.sfn is the SfnSf
    // *txOppsIt.slHasPsfch is the SfnSf
    while (newTxOpps.size() < totalTx && txOpps.size() > 0)
    {
        auto txOppsIt = txOpps.begin();
        std::advance(txOppsIt, m_grantSelectionUniformVariable->GetInteger(0, txOpps.size() - 1));
        if (IsCandidateResourceEligible(newTxOpps, *txOppsIt))
        {
            // copy the randomly selected resource into the new list
            newTxOpps.emplace_back(*txOppsIt);
            newTxOpps.sort();
        }
        // erase the selected one from the list
        txOpps.erase(txOppsIt);
    }
    // sort the list by SfnSf before returning
    newTxOpps.sort();
    NS_LOG_INFO("Selected " << newTxOpps.size() << " resources from " << originalSize
                            << " candidates and a maximum of " << +totalTx);
    return newTxOpps;
}

// This logic implements the minimum time gap constraint check.  The time
// resource assignment constraint (which appears to be a constraint on
// assigning SCI 1-A frequently enough, not on slot selection) can be
// handled in DoNrSlAllocation
bool
NrSlUeMacSchedulerFixedMcs::IsMinTimeGapSatisfied(const SfnSf& first,
                                                  const SfnSf& second,
                                                  uint8_t minTimeGapPsfch,
                                                  uint8_t minTimeGapProcessing) const
{
    NS_ASSERT_MSG(minTimeGapPsfch > 0, "Invalid minimum time gap");
    SfnSf sfnsf = first;
    sfnsf.Add(minTimeGapPsfch);
    while (!GetMac()->SlotHasPsfch(sfnsf))
    {
        sfnsf.Add(1);
    }
    sfnsf.Add(minTimeGapProcessing);
    return (sfnsf <= second);
}

bool
NrSlUeMacSchedulerFixedMcs::IsCandidateResourceEligible(const std::list<SlResourceInfo>& txOpps,
                                                        const SlResourceInfo& resourceInfo) const
{
    NS_LOG_FUNCTION(txOpps.size() << resourceInfo.sfn.Normalize());
    if (txOpps.size() == 0)
    {
        NS_LOG_DEBUG("Resource " << resourceInfo.sfn.Normalize()
                                 << " is eligible as the first slot in the list");
        return true; // first slot is always eligible
    }
    auto firstElementIt = txOpps.cbegin();
    auto lastElementIt = std::prev(txOpps.cend(), 1);
    if (resourceInfo.sfn == (*firstElementIt).sfn || resourceInfo.sfn == (*lastElementIt).sfn)
    {
        NS_LOG_DEBUG("Resource " << resourceInfo.sfn.Normalize()
                                 << " overlaps with first or last on the list");
        return false;
    }
    if (resourceInfo.sfn < (*firstElementIt).sfn)
    {
        bool eligible = IsMinTimeGapSatisfied(resourceInfo.sfn,
                                              (*firstElementIt).sfn,
                                              (*firstElementIt).slMinTimeGapPsfch,
                                              (*firstElementIt).slMinTimeGapProcessing);
        if (eligible)
        {
            NS_LOG_DEBUG("Resource " << resourceInfo.sfn.Normalize()
                                     << " is eligible as a new first slot in the list");
        }
        else
        {
            NS_LOG_DEBUG("Resource "
                         << resourceInfo.sfn.Normalize()
                         << " is not outside of minimum time gap to first slot in list");
        }
        return eligible;
    }
    else if ((*lastElementIt).sfn < resourceInfo.sfn)
    {
        bool eligible = IsMinTimeGapSatisfied((*lastElementIt).sfn,
                                              resourceInfo.sfn,
                                              (*lastElementIt).slMinTimeGapPsfch,
                                              (*lastElementIt).slMinTimeGapProcessing);
        if (eligible)
        {
            NS_LOG_DEBUG("Resource " << resourceInfo.sfn.Normalize()
                                     << " is eligible as a new last slot in the list");
        }
        else
        {
            NS_LOG_DEBUG("Resource " << resourceInfo.sfn.Normalize()
                                     << " is not outside of minimum time gap to last slot in list");
        }
        return eligible;
    }
    else
    {
        // Candidate slot lies in between elements of txOpps.  Find the two
        // elements (left, right) that bound the candidate.  Test that
        // the min time gap is satisfied for both intervals (left, candidate)
        // and (candidate, right).  Also, the resource may not overlap.
        auto leftIt = firstElementIt;
        auto rightIt = std::next(leftIt, 1);
        // we have already checked firstElementIt for an SFN match, so only
        // need to check the next one (rightIt)
        if (resourceInfo.sfn == (*rightIt).sfn)
        {
            NS_LOG_DEBUG("Resource " << resourceInfo.sfn.Normalize()
                                     << " overlaps with one on the list");
            return false;
        }
        while ((*rightIt).sfn < resourceInfo.sfn)
        {
            leftIt++;
            rightIt++;
            NS_ASSERT_MSG(leftIt != lastElementIt, "Unexpectedly reached end");
        }
        bool eligible = (IsMinTimeGapSatisfied((*leftIt).sfn,
                                               resourceInfo.sfn,
                                               (*leftIt).slMinTimeGapPsfch,
                                               (*leftIt).slMinTimeGapProcessing) &&
                         IsMinTimeGapSatisfied(resourceInfo.sfn,
                                               (*rightIt).sfn,
                                               (*rightIt).slMinTimeGapPsfch,
                                               (*rightIt).slMinTimeGapProcessing));
        if (eligible)
        {
            NS_LOG_DEBUG("Resource " << resourceInfo.sfn.Normalize() << " is eligible between "
                                     << (*leftIt).sfn.Normalize() << " and "
                                     << (*rightIt).sfn.Normalize());
        }
        else
        {
            NS_LOG_DEBUG("Resource " << resourceInfo.sfn.Normalize()
                                     << " does not meet constraints");
        }
        return eligible;
    }
    return true; // unreachable, but can silence compiler warning
}

Ptr<NrSlUeMacHarq>
NrSlUeMacSchedulerFixedMcs::GetMacHarq(void) const
{
    if (!m_nrSlUeMacHarq)
    {
        PointerValue val;
        GetMac()->GetAttribute("NrSlUeMacHarq", val);
        m_nrSlUeMacHarq = val.Get<NrSlUeMacHarq>();
    }
    return m_nrSlUeMacHarq;
}

} // namespace ns3
