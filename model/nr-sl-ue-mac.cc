/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only AND NIST-Software

#define NS_LOG_APPEND_CONTEXT                                                                      \
    do                                                                                             \
    {                                                                                              \
        std::clog << " [ CellId " << GetCellId() << ", bwpId " << GetBwpId() << ", rnti "          \
                  << GetRnti() << "] ";                                                            \
    } while (false);

#include "nr-sl-ue-mac.h"

#include "nr-control-messages.h"
#include "nr-mac-header-vs.h"
#include "nr-mac-short-bsr-ce.h"
#include "nr-phy-sap.h"
#include "nr-sl-mac-pdu-tag.h"
#include "nr-sl-sci-f1a-header.h"
#include "nr-sl-sci-f2a-header.h"
#include "nr-sl-ue-mac-harq.h"
#include "nr-sl-ue-mac-scheduler.h"

#include "ns3/lte-rlc-tag.h"
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/pointer.h>
#include <ns3/random-variable-stream.h>
#include <ns3/uinteger.h>

#include <algorithm>
#include <bitset>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrSlUeMac");

NS_OBJECT_ENSURE_REGISTERED(NrSlUeMac);

TypeId
NrSlUeMac::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrSlUeMac")
            .SetParent<NrUeMac>()
            .AddConstructor<NrSlUeMac>()
            .AddAttribute("EnableSensing",
                          "Flag to enable NR Sidelink resource selection based on sensing; "
                          "otherwise, use random selection",
                          BooleanValue(false),
                          MakeBooleanAccessor(&NrSlUeMac::EnableSensing),
                          MakeBooleanChecker())
            .AddAttribute("Tproc0",
                          "t_proc0 in slots",
                          UintegerValue(1),
                          MakeUintegerAccessor(&NrSlUeMac::SetTproc0, &NrSlUeMac::GetTproc0),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute("T1",
                          "The start of the selection window in physical slots, accounting for "
                          "physical layer processing delay",
                          UintegerValue(2),
                          MakeUintegerAccessor(&NrSlUeMac::SetT1, &NrSlUeMac::GetT1),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute("T2",
                          "The end of the selection window in physical slots",
                          UintegerValue(33),
                          MakeUintegerAccessor(&NrSlUeMac::SetT2, &NrSlUeMac::GetT2),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute(
                "ActivePoolId",
                "The pool id of the active pool used for TX and RX",
                UintegerValue(0),
                MakeUintegerAccessor(&NrSlUeMac::SetSlActivePoolId, &NrSlUeMac::GetSlActivePoolId),
                MakeUintegerChecker<uint16_t>())
            .AddAttribute("ReservationPeriod",
                          "Resource Reservation Interval for NR Sidelink in ms"
                          "Must be among the values included in LteRrcSap::SlResourceReservePeriod",
                          TimeValue(MilliSeconds(100)),
                          MakeTimeAccessor(&NrSlUeMac::SetReservationPeriod,
                                           &NrSlUeMac::GetReservationPeriod),
                          MakeTimeChecker())
            .AddAttribute("NumSidelinkProcess",
                          "Number of concurrent stop-and-wait Sidelink processes for this UE",
                          UintegerValue(4),
                          MakeUintegerAccessor(&NrSlUeMac::SetNumSidelinkProcess,
                                               &NrSlUeMac::GetNumSidelinkProcess),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute("EnableBlindReTx",
                          "Flag to enable NR Sidelink blind retransmissions",
                          BooleanValue(true),
                          MakeBooleanAccessor(&NrSlUeMac::EnableBlindReTx),
                          MakeBooleanChecker())
            .AddAttribute(
                "SlThresPsschRsrp",
                "A threshold in dBm used for sensing based UE autonomous resource selection",
                IntegerValue(-128),
                MakeIntegerAccessor(&NrSlUeMac::SetSlThresPsschRsrp,
                                    &NrSlUeMac::GetSlThresPsschRsrp),
                MakeIntegerChecker<int>())
            .AddAttribute("NrSlUeMacScheduler",
                          "The scheduler for this MAC instance",
                          PointerValue(),
                          MakePointerAccessor(&NrSlUeMac::m_nrSlUeMacScheduler),
                          MakePointerChecker<NrSlUeMacScheduler>())
            .AddAttribute("ResourcePercentage",
                          "The percentage threshold to indicate the minimum number of"
                          "candidate single-slot resources to be selected using sensing"
                          "procedure",
                          UintegerValue(20),
                          MakeUintegerAccessor(&NrSlUeMac::SetResourcePercentage,
                                               &NrSlUeMac::GetResourcePercentage),
                          MakeUintegerChecker<uint8_t>(1, 100))
            .AddTraceSource("SlPscchScheduling",
                            "Information regarding NR SL PSCCH UE scheduling",
                            MakeTraceSourceAccessor(&NrSlUeMac::m_slPscchScheduling),
                            "ns3::SlPscchUeMacStatParameters::TracedCallback")
            .AddTraceSource("SlPsschScheduling",
                            "Information regarding NR SL PSSCH UE scheduling",
                            MakeTraceSourceAccessor(&NrSlUeMac::m_slPsschScheduling),
                            "ns3::SlPsschUeMacStatParameters::TracedCallback")
            .AddTraceSource("RxRlcPduWithTxRnti",
                            "PDU received trace also exporting TX UE RNTI in SL.",
                            MakeTraceSourceAccessor(&NrSlUeMac::m_rxRlcPduWithTxRnti),
                            "ns3::NrSlUeMac::ReceiveWithTxRntiTracedCallback")
            .AddTraceSource("SensingAlgorithm",
                            "Candidates selected by the mode 2 sensing algorithm",
                            MakeTraceSourceAccessor(&NrSlUeMac::m_tracedSensingAlgorithm),
                            "ns3::NrSlUeMac::SensingAlgorithmTracedCallback");
    return tid;
}

NrSlUeMac::NrSlUeMac()
    : NrUeMac()
{
    NS_LOG_FUNCTION(this);
    m_nrSlMacSapProvider = new MemberNrSlMacSapProvider<NrSlUeMac>(this);
    m_nrSlUeCmacSapProvider = new MemberNrSlUeCmacSapProvider<NrSlUeMac>(this);
    m_nrSlUePhySapUser = new MemberNrSlUePhySapUser<NrSlUeMac>(this);
    m_ueSelectedUniformVariable = CreateObject<UniformRandomVariable>();
    m_nrSlHarq = CreateObject<NrSlUeMacHarq>();
}

void
NrSlUeMac::SchedNrSlConfigInd(const std::set<NrSlSlotAlloc>& slotAllocList)
{
    NS_LOG_FUNCTION(this);
    auto itGrantInfo = m_grantInfo.find(slotAllocList.begin()->dstL2Id);

    if (itGrantInfo == m_grantInfo.end())
    {
        NrSlGrantInfo grant = CreateGrantInfo(slotAllocList);
        itGrantInfo =
            m_grantInfo.emplace(std::make_pair(slotAllocList.begin()->dstL2Id, grant)).first;
    }
    else
    {
        NS_ASSERT_MSG(itGrantInfo->second.slResoReselCounter == 0,
                      "Sidelink resource counter must be zero before assigning new grant for dst "
                          << slotAllocList.begin()->dstL2Id);
        NrSlGrantInfo grant = CreateGrantInfo(slotAllocList);
        itGrantInfo->second = grant;
    }

    NS_ASSERT_MSG(!itGrantInfo->second.slotAllocations.empty(),
                  "CreateGrantInfo failed to create grants");
}

void
NrSlUeMac::DoDispose()
{
    NS_LOG_FUNCTION(this);
    delete m_nrSlMacSapProvider;
    delete m_nrSlUeCmacSapProvider;
    delete m_nrSlUePhySapUser;
    if (m_nrSlHarq)
    {
        m_nrSlHarq->Dispose();
    }
    m_nrSlHarq = nullptr;
    if (m_nrSlUeMacScheduler)
    {
        m_nrSlUeMacScheduler->Dispose();
    }
    m_nrSlUeMacScheduler = nullptr;
    m_slTxPool = nullptr;
    m_slRxPool = nullptr;
}

int64_t
NrSlUeMac::DoAssignStreams(int64_t stream)
{
    NS_LOG_FUNCTION(this << stream);
    int64_t currentStream = stream;
    currentStream += NrUeMac::DoAssignStreams(currentStream);
    m_ueSelectedUniformVariable->SetStream(currentStream);
    currentStream++;
    return (currentStream - stream);
}

void
NrSlUeMac::DoSlotIndication(const SfnSf& sfn)
{
    NS_LOG_FUNCTION(this << sfn);
    SetCurrentSlot(sfn);

    if (m_enableSensing)
    {
        RemoveOldSensingData(
            sfn,
            m_slTxPool->GetNrSlSensWindInSlots(GetBwpId(),
                                               m_poolId,
                                               m_nrSlUePhySapProvider->GetSlotPeriod()),
            m_sensingData,
            GetImsi());
        RemoveOldTransmitHistory(
            sfn,
            m_slTxPool->GetNrSlSensWindInSlots(GetBwpId(),
                                               m_poolId,
                                               m_nrSlUePhySapProvider->GetSlotPeriod()),
            m_transmitHistory,
            GetImsi());
    }

    // trigger SL only when it is a SL slot
    if (m_slTxPool->IsSidelinkSlot(GetBwpId(), m_poolId, sfn.Normalize()))
    {
        DoNrSlSlotIndication(sfn);
    }
}

std::list<NrSlSlotInfo>
NrSlUeMac::FilterNrSlCandidateResources(std::list<NrSlSlotInfo> candidateReso)
{
    NS_LOG_FUNCTION(this);

    if (candidateReso.empty() || m_grantInfo.empty())
    {
        return candidateReso;
    }

    NrSlSlotAlloc dummyAlloc;

    for (const auto& itDst : m_grantInfo)
    {
        auto itCandReso = candidateReso.begin();
        while (itCandReso != candidateReso.end())
        {
            dummyAlloc.sfn = itCandReso->sfn;
            auto itAlloc = itDst.second.slotAllocations.find(dummyAlloc);
            if (itAlloc != itDst.second.slotAllocations.end())
            {
                itCandReso = candidateReso.erase(itCandReso);
            }
            else
            {
                ++itCandReso;
            }
        }
    }
    return candidateReso;
}

std::list<NrSlSlotInfo>
NrSlUeMac::GetNrSlAvailableResources(const SfnSf& sfn, const NrSlTransmissionParams& params)
{
    NS_LOG_FUNCTION(this << sfn.GetFrame() << +sfn.GetSubframe() << sfn.GetSlot() << params);

    std::list<NrSlSlotInfo> availableResources;
    std::list<NrSlSlotInfo> candidateResources;

    candidateResources = GetNrSlCandidateResources(sfn, params);
    availableResources = FilterNrSlCandidateResources(candidateResources);

    return availableResources;
}

std::list<NrSlSlotInfo>
NrSlUeMac::GetNrSlCandidateResources(const SfnSf& sfn, const NrSlTransmissionParams& params)
{
    return GetNrSlCandidateResourcesPrivate(sfn,
                                            params,
                                            m_slTxPool,
                                            m_nrSlUePhySapProvider->GetSlotPeriod(),
                                            GetImsi(),
                                            GetBwpId(),
                                            m_poolId,
                                            GetTotalSubCh(),
                                            m_sensingData,
                                            m_transmitHistory);
}

std::list<NrSlSlotInfo>
NrSlUeMac::GetNrSlCandidateResourcesPrivate(const SfnSf& sfn,
                                            const NrSlTransmissionParams& params,
                                            Ptr<const NrSlCommResourcePool> txPool,
                                            Time slotPeriod,
                                            uint64_t imsi,
                                            uint8_t bwpId,
                                            uint16_t poolId,
                                            uint8_t totalSubCh,
                                            const std::list<SensingData>& sensingData,
                                            const std::list<SfnSf>& transmitHistory) const
{
    NS_LOG_FUNCTION(this << sfn.GetFrame() << +sfn.GetSubframe() << sfn.GetSlot() << params
                         << txPool << slotPeriod << imsi << +bwpId << poolId << +totalSubCh);

    SensingTraceReport report; // for tracing
    report.m_sfn = sfn;
    report.m_t0 = txPool->GetNrSlSensWindInSlots(bwpId, poolId, slotPeriod);
    report.m_tProc0 = m_tproc0;
    report.m_t1 = m_t1;
    report.m_t2 = m_t2;
    report.m_subchannels = totalSubCh;
    report.m_lSubch = params.m_lSubch;
    report.m_resourcePercentage = GetResourcePercentage();

    NS_LOG_DEBUG("Transmit  size: " << transmitHistory.size()
                                    << "; sensing data size: " << sensingData.size());

    // Input parameters (from params) are the priority, packet delay budget,
    // number of subchannels, the RRI, and the C_resel
    // - params.m_priority
    // - params.m_packetDelayBudget
    // - params.m_lSubch
    // - params.m_pRsvpTx
    // - params.m_cResel

    // TR 38.214 Section 8.1.4, return the set 'S_A' (candidate single slot
    // resources).  The size of this list is the algorithm parameter 'M_total'.

    // In this code, the list of candidateSlots is taken from the resource pool,
    // each SlotInfo doesn't have a list of subchannel (indices).
    // The NrUeMac copies each resource to the candidateResources list,
    // the only difference being that this NrSlSlotInfo
    // exists at the mac/scheduler API.  In a future revision, it is
    // planned to convert NrSlSlotInfo into a different structure that is
    // resource, not slot, based.

    std::list<NrSlCommResourcePool::SlotInfo> candidateSlots; // candidate single slots
    std::list<NrSlSlotInfo> candidateResources;               // S_A as per TS 38.214

    uint64_t absSlotIndex = sfn.Normalize();
    uint16_t numerology = sfn.GetNumerology();

    // Check the validity of the resource selection window configuration (T1 and T2)
    // and the following parameters: numerology and reservation period.
    uint16_t nsMs =
        (m_t2 - m_t1 + 1) *
        (1 / pow(2, numerology)); // number of slots multiplied by the slot duration in ms
    uint16_t rsvpMs =
        static_cast<uint16_t>(params.m_pRsvpTx.GetMilliSeconds()); // reservation period in ms
    NS_ABORT_MSG_IF(rsvpMs && nsMs > rsvpMs,
                    "An error may be generated due to the fact that the resource selection window "
                    "size is higher than the resource reservation period value. Make sure that "
                    "(T2-T1+1) x (1/(2^numerology)) < reservation period. Modify the values of T1, "
                    "T2, numerology, and reservation period accordingly.");

    // step 4 as per TS 38.214 sec 8.1.4
    candidateSlots =
        txPool->GetNrSlCommOpportunities(absSlotIndex, bwpId, numerology, poolId, m_t1, m_t2);
    report.m_initialCandidateSlotsSize = candidateSlots.size();
    if (candidateSlots.empty())
    {
        // Since, all the parameters (i.e., T1, T2min, and T2) of the selection
        // window are in terms of physical slots, it may happen that there are no
        // slots available for Sidelink, which depends on the TDD pattern and the
        // Sidelink bitmap.
        return std::list<NrSlSlotInfo>();
    }
    candidateResources =
        GetNrSlCandidateResourcesFromSlots(sfn, params.m_lSubch, totalSubCh, candidateSlots);
    uint32_t mTotal = candidateResources.size(); // total number of candidate single-slot resources
    report.m_initialCandidateResourcesSize = mTotal;
    if (!m_enableSensing)
    {
        NS_LOG_DEBUG("No sensing: Total slots selected " << mTotal);
        return candidateResources;
    }

    // This is an optimization to skip further null processing below
    if (m_enableSensing && sensingData.empty() && transmitHistory.empty())
    {
        NS_LOG_DEBUG("No sensing or data found: Total slots selected " << mTotal);
        m_tracedSensingAlgorithm(report, candidateResources, sensingData, transmitHistory);
        return candidateResources;
    }

    // Copy the buffer so we can trim the buffer as per Tproc0.
    // Note, we do not need to delete the latest measurement
    // from the original buffer because it will be deleted
    // by RemoveOldSensingData method once it is outdated.

    auto updatedSensingData = sensingData;

    // latest sensing data is at the end of the list
    // now remove the latest sensing data as per the value of Tproc0. This would
    // keep the size of the buffer equal to [n – T0 , n – Tproc0)

    auto rvIt = updatedSensingData.crbegin();
    if (rvIt != updatedSensingData.crend())
    {
        while (sfn.Normalize() - rvIt->sfn.Normalize() <= GetTproc0())
        {
            NS_LOG_DEBUG("IMSI " << GetImsi() << " ignoring sensed SCI at sfn " << sfn
                                 << " received at " << rvIt->sfn);
            updatedSensingData.pop_back();
            rvIt = updatedSensingData.crbegin();
        }
    }

    // Perform a similar operation on the transmit .
    // latest  is at the end of the list
    // keep the size of the buffer equal to [n – T0 , n – Tproc0)
    auto updatedHistory = transmitHistory;

    auto rvIt2 = updatedHistory.crbegin();
    if (rvIt2 != updatedHistory.crend())
    {
        while (sfn.Normalize() - rvIt2->Normalize() <= GetTproc0())
        {
            NS_LOG_DEBUG("IMSI " << GetImsi() << " ignoring  at sfn " << sfn << " received at "
                                 << *rvIt2);
            updatedHistory.pop_back();
            rvIt2 = updatedHistory.crbegin();
        }
    }

    // step 5: filter candidateResources based on transmit history, if threshold
    // defined in step 5a) is met
    auto candidatesToCheck = candidateResources;
    ExcludeResourcesBasedOnHistory(sfn,
                                   updatedHistory,
                                   candidatesToCheck,
                                   txPool->GetSlResourceReservePeriodList(bwpId, poolId));
    if (candidatesToCheck.size() >= (GetResourcePercentage() / 100.0) * mTotal)
    {
        NS_LOG_DEBUG("Step 5 filter results: original: "
                     << candidateResources.size() << " updated: " << candidatesToCheck.size()
                     << " X: " << GetResourcePercentage() / 100.0);
        candidateResources = candidatesToCheck;
    }
    else
    {
        candidatesToCheck = candidateResources;
    }
    report.m_candidateResourcesSizeAfterStep5 = candidatesToCheck.size();

    // step 6

    // calculate all possible transmissions based on sensed SCIs,
    // with past transmissions projected into the selection window.
    // Using a vector of SlotSensingData, since we need to check all the SCIs
    // and their possible future transmission that are received during the
    // above trimmed sensing window. Each element of the vector holds a
    // list that holds the info of each received SCI and its possible
    // future transmissions.
    std::vector<std::list<SlotSensingData>> sensingDataProjections;
    for (const auto& itSensedSlot : updatedSensingData)
    {
        uint16_t resvPeriodSlots = txPool->GetResvPeriodInSlots(bwpId,
                                                                poolId,
                                                                MilliSeconds(itSensedSlot.rsvp),
                                                                slotPeriod);
        std::list<SlotSensingData> listFutureSensTx =
            GetFutSlotsBasedOnSens(itSensedSlot, slotPeriod, resvPeriodSlots);
        sensingDataProjections.push_back(listFutureSensTx);
    }

    NS_LOG_DEBUG("Size of sensingDataProjections outer vector: " << sensingDataProjections.size());

    int rsrpThreshold = m_thresRsrp;
    report.m_initialRsrpThreshold = m_thresRsrp;
    do
    {
        // following assignment is needed since we might have to perform
        // multiple do-while over the same list by increasing the rsrpThreshold
        candidateResources = candidatesToCheck;
        NS_LOG_DEBUG("Step 6 loop iteration checking " << candidateResources.size()
                                                       << " resources against threshold "
                                                       << rsrpThreshold);
        auto itCandidate = candidateResources.begin();
        // itCandidate is the candidate single-slot resource R_x,y
        while (itCandidate != candidateResources.end())
        {
            bool erased = false;
            // calculate all proposed transmissions of current candidate resource within selection
            // window
            std::list<NrSlSlotInfo> listFutureCands;
            uint16_t pPrimeRsvpTx =
                txPool->GetResvPeriodInSlots(bwpId, poolId, params.m_pRsvpTx, slotPeriod);
            for (uint16_t i = 0; i < params.m_cResel; i++)
            {
                auto slAlloc = *itCandidate;
                slAlloc.sfn.Add(i * pPrimeRsvpTx);
                listFutureCands.emplace_back(slAlloc);
            }
            // Traverse over all the possible transmissions of each sensed SCI
            for (const auto& itSensingDataProjections : sensingDataProjections)
            {
                // for all proposed transmissions of current candidate resource
                for (auto& itFutureCand : listFutureCands)
                {
                    // Traverse the list of future projected transmissions for the given sensed SCI
                    for (const auto& itSlotSensingDataProjection : itSensingDataProjections)
                    {
                        if (itFutureCand.sfn.Normalize() ==
                            itSlotSensingDataProjection.sfn.Normalize())
                        {
                            if (itSlotSensingDataProjection.slRsrp > rsrpThreshold)
                            {
                                if (OverlappedResource(itSlotSensingDataProjection.sbChStart,
                                                       itSlotSensingDataProjection.sbChLength,
                                                       itCandidate->slSubchannelStart,
                                                       itCandidate->slSubchannelLength))
                                {
                                    NS_LOG_DEBUG("Overlapped resource "
                                                 << itCandidate->sfn.Normalize() << " occupied "
                                                 << +itSlotSensingDataProjection.sbChLength
                                                 << " subchannels index "
                                                 << +itSlotSensingDataProjection.sbChStart);
                                    itCandidate = candidateResources.erase(itCandidate);
                                    NS_LOG_DEBUG("Resource "
                                                 << itCandidate->sfn.Normalize() << ":["
                                                 << itCandidate->slSubchannelStart << ","
                                                 << (itCandidate->slSubchannelStart +
                                                     itCandidate->slSubchannelLength - 1)
                                                 << "] erased. Its rsrp : "
                                                 << itSlotSensingDataProjection.slRsrp
                                                 << " Threshold : " << rsrpThreshold);
                                    erased = true; // Used to break out of outer for loop of sensed
                                                   // data projections
                                    break; // Stop further evaluation because candidate is erased
                                }
                            }
                        }
                    }
                }
                if (erased)
                {
                    break; // break for itSensingDataProjections
                }
            }
            if (!erased)
            {
                // Only need to increment if not erased above; if erased, the erase()
                // action will point itCandidate to the next item
                itCandidate++;
            }
        }
        // step 7. If the following while will not break, start over do-while
        // loop with rsrpThreshold increased by 3dB
        rsrpThreshold += 3;
        if (rsrpThreshold > 0)
        {
            // 0 dBm is the maximum RSRP threshold level so if we reach
            // it, that means all the available slots are overlapping
            // in time and frequency with the sensed slots, and the
            // RSRP of the sensed slots is very high.
            NS_LOG_DEBUG("Reached maximum RSRP threshold, unable to select resources");
            candidateResources.erase(candidateResources.begin(), candidateResources.end());
            break; // break do while
        }
    } while (candidateResources.size() < (GetResourcePercentage() / 100.0) * mTotal);

    NS_LOG_DEBUG(candidateResources.size()
                 << " slots selected after sensing resource selection from " << mTotal << " slots");

    report.m_finalRsrpThreshold = (rsrpThreshold - 3); // undo the last increment
    m_tracedSensingAlgorithm(report, candidateResources, updatedSensingData, updatedHistory);
    return candidateResources;
}

std::list<NrSlSlotInfo>
NrSlUeMac::GetNrSlCandidateResourcesFromSlots(
    const SfnSf& sfn,
    uint16_t lSubCh,
    uint16_t totalSubCh,
    std::list<NrSlCommResourcePool::SlotInfo> slotInfo) const
{
    NS_LOG_FUNCTION(this << sfn.Normalize() << lSubCh << totalSubCh << slotInfo.size());

    std::list<NrSlSlotInfo> nrResourceList;
    for (const auto& it : slotInfo)
    {
        for (uint16_t i = 0; i + lSubCh <= totalSubCh; i++)
        {
            std::set<uint8_t> emptySet;
            NrSlSlotInfo info(it.numSlPscchRbs,
                              it.slPscchSymStart,
                              it.slPscchSymLength,
                              it.slPsschSymStart,
                              it.slPsschSymLength,
                              it.slSubchannelSize,
                              it.slMaxNumPerReserve,
                              sfn.GetFutureSfnSf(it.slotOffset),
                              i,
                              lSubCh,
                              emptySet);
            nrResourceList.emplace_back(info);
        }
    }

    return nrResourceList;
}

void
NrSlUeMac::ExcludeResourcesBasedOnHistory(
    const SfnSf& sfn,
    const std::list<SfnSf>& transmitHistory,
    std::list<NrSlSlotInfo>& candidateList,
    const std::list<uint16_t>& slResourceReservePeriodList) const
{
    NS_LOG_FUNCTION(this << sfn.Normalize() << transmitHistory.size() << candidateList.size()
                         << slResourceReservePeriodList.size());

    std::set<uint64_t> sfnToExclude; // SFN slot numbers (normalized) to exclude
    uint64_t firstSfnNorm =
        candidateList.front().sfn.Normalize(); // lowest candidate SFN slot number
    uint64_t lastSfnNorm =
        candidateList.back().sfn.Normalize(); // highest candidate SFN slot number
    NS_LOG_DEBUG("Excluding resources between normalized SFNs (" << firstSfnNorm << ":"
                                                                 << lastSfnNorm << ")");

    // Iterate the resource reserve period list and the transmit history to
    // find all slot numbers such that multiples of the reserve period, when
    // added to the history's slot number, are within the candidate resource
    // slots lowest and highest numbers
    for (auto listIt : slResourceReservePeriodList)
    {
        if (listIt == 0)
        {
            continue; // 0ms value is ignored
        }
        listIt *= (1 << sfn.GetNumerology()); // Convert from ms to slots
        for (const auto& historyIt : transmitHistory)
        {
            uint16_t i = 1;
            uint64_t sfnToCheck = (historyIt).Normalize() + (listIt);
            while (sfnToCheck <= lastSfnNorm)
            {
                if (sfnToCheck >= firstSfnNorm)
                {
                    sfnToExclude.insert(sfnToCheck);
                }
                i++;
                sfnToCheck = (historyIt).Normalize() + (i) * (listIt);
            }
        }
    }
    // sfnToExclude is a set of SFN normalized slot numbers for which we need
    // to exclude (erase) any candidate resources that match
    for (auto i : sfnToExclude)
    {
        auto itCand = candidateList.begin();
        while (itCand != candidateList.end())
        {
            if ((*itCand).sfn.Normalize() == i)
            {
                NS_LOG_DEBUG("Erasing candidate resource at " << i);
                itCand = candidateList.erase(itCand);
            }
            else
            {
                itCand++;
            }
        }
    }
}

// Calculates parameters including Q for step 6(c) of sensing algorithm
std::list<SlotSensingData>
NrSlUeMac::GetFutSlotsBasedOnSens(SensingData sensedData,
                                  Time slotPeriod,
                                  uint16_t resvPeriodSlots) const
{
    NS_LOG_FUNCTION(this << sensedData.sfn.Normalize() << slotPeriod << resvPeriodSlots);
    std::list<SlotSensingData> listFutureSensTx;

    double slotLenMiSec = slotPeriod.GetSeconds() * 1000.0;
    NS_ABORT_MSG_IF(slotLenMiSec > 1, "Slot length can not exceed 1 ms");
    uint16_t selecWindLen = (m_t2 - m_t1) + 1; // selection window length in physical slots
    double tScalMilSec = selecWindLen * slotLenMiSec;
    double pRsvpRxMilSec = static_cast<double>(sensedData.rsvp);
    uint16_t q = 0;
    if (sensedData.rsvp != 0)
    {
        // I am aware that two double variable are compared. I don't expect these two
        // numbers to be big floating-point numbers.
        if (pRsvpRxMilSec < tScalMilSec)
        {
            q = static_cast<uint16_t>(std::ceil(tScalMilSec / pRsvpRxMilSec));
        }
        else
        {
            q = 1;
        }
        NS_LOG_DEBUG("tScalMilSec: " << tScalMilSec << " pRsvpRxMilSec: " << pRsvpRxMilSec);
    }
    uint16_t pPrimeRsvpRx = resvPeriodSlots;

    for (uint16_t i = 0; i <= q; i++)
    {
        SlotSensingData sensedSlotData(sensedData.sfn,
                                       sensedData.rsvp,
                                       sensedData.sbChLength,
                                       sensedData.sbChStart,
                                       sensedData.prio,
                                       sensedData.slRsrp);
        sensedSlotData.sfn.Add(i * pPrimeRsvpRx);
        listFutureSensTx.emplace_back(sensedSlotData);

        if (sensedData.gapReTx1 != std::numeric_limits<uint8_t>::max())
        {
            auto reTx1Slot = sensedSlotData;
            reTx1Slot.sfn = sensedSlotData.sfn.GetFutureSfnSf(sensedData.gapReTx1);
            reTx1Slot.sbChLength = sensedData.sbChLength;
            reTx1Slot.sbChStart = sensedData.sbChStartReTx1;
            listFutureSensTx.emplace_back(reTx1Slot);
        }
        if (sensedData.gapReTx2 != std::numeric_limits<uint8_t>::max())
        {
            auto reTx2Slot = sensedSlotData;
            reTx2Slot.sfn = sensedSlotData.sfn.GetFutureSfnSf(sensedData.gapReTx2);
            reTx2Slot.sbChLength = sensedData.sbChLength;
            reTx2Slot.sbChStart = sensedData.sbChStartReTx2;
            listFutureSensTx.emplace_back(reTx2Slot);
        }
    }
    NS_LOG_DEBUG("q: " << q << " Size of listFutureSensTx: " << listFutureSensTx.size());

    return listFutureSensTx;
}

void
NrSlUeMac::RemoveOldSensingData(const SfnSf& sfn,
                                uint16_t sensingWindow,
                                std::list<SensingData>& sensingData,
                                [[maybe_unused]] uint64_t imsi)
{
    NS_LOG_FUNCTION(this << sfn << sensingWindow << sensingData.size() << imsi);
    // oldest sensing data is on the top of the list
    auto it = sensingData.cbegin();
    while (it != sensingData.cend())
    {
        if (it->sfn.Normalize() < sfn.Normalize() - sensingWindow)
        {
            NS_LOG_DEBUG("IMSI " << imsi << " erasing SCI at sfn " << sfn << " received at "
                                 << it->sfn);
            it = sensingData.erase(it);
        }
        else
        {
            // once we reached the sensing data, which lies in the
            // sensing window, we break. If the last entry lies in the sensing
            // window rest of the entries as well.
            break;
        }
        ++it;
    }
}

void
NrSlUeMac::RemoveOldTransmitHistory(const SfnSf& sfn,
                                    uint16_t sensingWindow,
                                    std::list<SfnSf>& history,
                                    [[maybe_unused]] uint64_t imsi)
{
    NS_LOG_FUNCTION(this << sfn << sensingWindow << history.size() << imsi);

    auto it = history.cbegin();
    while (it != history.cend())
    {
        if (it->Normalize() < sfn.Normalize() - sensingWindow)
        {
            NS_LOG_DEBUG("IMSI " << imsi << " erasing SFN history at sfn " << sfn << " sent at "
                                 << *it);
            it = history.erase(it);
        }
        else
        {
            // break upon reaching the edge of the sensing window
            break;
        }
        ++it;
    }
}

bool
NrSlUeMac::OverlappedResource(uint8_t firstStart,
                              uint8_t firstLength,
                              uint8_t secondStart,
                              uint8_t secondLength) const
{
    NS_ASSERT_MSG(firstLength && secondLength, "Length should not be zero");
    return (std::max(firstStart, secondStart) <
            std::min(firstStart + firstLength, secondStart + secondLength));
}

void
NrSlUeMac::DoReceiveSensingData(SensingData sensingData)
{
    NS_LOG_FUNCTION(this);

    if (m_enableSensing)
    {
        // oldest data will be at the front of the queue
        m_sensingData.push_back(sensingData);
    }
}

void
NrSlUeMac::DoReceivePsschPhyPdu(Ptr<PacketBurst> pdu)
{
    NS_LOG_FUNCTION(this << "Received Sidelink PDU from PHY");

    LteRadioBearerTag tag;
    NrSlSciF2aHeader sciF2a;
    // Separate SCI stage 2 packet from data packets
    std::list<Ptr<Packet>> dataPkts;
    bool foundSci2 = false;
    for (auto packet : pdu->GetPackets())
    {
        LteRadioBearerTag tag;
        if (!packet->PeekPacketTag(tag))
        {
            // SCI stage 2 is the only packet in the packet burst, which does
            // not have the tag
            packet->RemoveHeader(sciF2a);
            foundSci2 = true;
        }
        else
        {
            dataPkts.push_back(packet);
        }
    }

    NS_ABORT_MSG_IF(foundSci2 == false, "Did not find SCI stage 2 in PSSCH packet burst");
    NS_ASSERT_MSG(!dataPkts.empty(), "Received PHY PDU with not data packets");

    // Perform L2 filtering.
    // Remember, all the packets in the packet burst are for the same
    // destination, therefore it is safe to do the following.
    auto it = m_sidelinkRxDestinations.find(sciF2a.GetDstId());
    if (it == m_sidelinkRxDestinations.end())
    {
        // if we hit this assert that means SCI 1 reception code in NrUePhy
        // is not filtering the SCI 1 correctly.
        NS_FATAL_ERROR("Received PHY PDU with unknown destination " << sciF2a.GetDstId());
    }

    for (auto& pktIt : dataPkts)
    {
        pktIt->RemovePacketTag(tag);
        // Even though all the packets in the packet burst are for the same
        // destination, they can belong to different Logical Channels (LC),
        // therefore, we have to build the identifier and find the LC of the
        // packet.
        SidelinkLcIdentifier identifier;
        identifier.lcId = tag.GetLcid();
        identifier.srcL2Id = sciF2a.GetSrcId();
        identifier.dstL2Id = sciF2a.GetDstId();

        std::map<SidelinkLcIdentifier, SlLcInfoUeMac>::iterator it =
            m_nrSlLcInfoMap.find(identifier);
        if (it == m_nrSlLcInfoMap.end())
        {
            // notify RRC to setup bearer
            m_nrSlUeCmacSapUser->NotifySidelinkReception(tag.GetLcid(),
                                                         identifier.srcL2Id,
                                                         identifier.dstL2Id);

            // should be setup now
            it = m_nrSlLcInfoMap.find(identifier);
            if (it == m_nrSlLcInfoMap.end())
            {
                NS_FATAL_ERROR("Failure to setup Sidelink radio bearer for reception");
            }
        }
        NrSlMacSapUser::NrSlReceiveRlcPduParameters rxPduParams(pktIt,
                                                                GetRnti(),
                                                                tag.GetLcid(),
                                                                identifier.srcL2Id,
                                                                identifier.dstL2Id);

        FireTraceSlRlcRxPduWithTxRnti(pktIt->Copy(), tag.GetLcid());

        it->second.macSapUser->ReceiveNrSlRlcPdu(rxPduParams);
    }
}

void
NrSlUeMac::DoNrSlSlotIndication(const SfnSf& sfn)
{
    NS_LOG_FUNCTION(this << " Frame " << sfn.GetFrame() << " Subframe " << +sfn.GetSubframe()
                         << " slot " << sfn.GetSlot() << " Normalized slot number "
                         << sfn.Normalize());

    bool atLeastOneTransmissionInSlot = false;
    if (m_slTxPool->GetNrSlSchedulingType() == NrSlCommResourcePool::SCHEDULED)
    {
    }
    else if (m_slTxPool->GetNrSlSchedulingType() == NrSlCommResourcePool::UE_SELECTED)
    {
        // Do not ask for resources if no HARQ/Sidelink process is available
        if (m_nrSlHarq->GetNumAvailableHarqIds() > 0)
        {
            for (const auto& itDst : m_sidelinkTxDestinations)
            {
                const auto itGrantInfo = m_grantInfo.find(itDst.first);
                bool foundDest = itGrantInfo != m_grantInfo.end();
                if (foundDest)
                {
                    // If the re-selection counter of the found destination is not zero,
                    // it means it already have resources assigned to it via semi-persistent
                    // scheduling, thus, we go to the next destination
                    if (itGrantInfo->second.slResoReselCounter != 0)
                    {
                        NS_LOG_INFO("Destination " << itDst.first
                                                   << " already have the allocation, scheduling "
                                                      "the next destination, if any");
                        continue;
                    }

                    double randProb = m_ueSelectedUniformVariable->GetValue(0, 1);
                    if (itGrantInfo->second.cReselCounter > 0 &&
                        !itGrantInfo->second.slotAllocations.empty() &&
                        m_slProbResourceKeep > randProb)
                    {
                        NS_LOG_INFO("IMSI " << GetImsi() << " keeping the resource for "
                                            << itDst.first);
                        NS_ASSERT_MSG(itGrantInfo->second.slResoReselCounter == 0,
                                      "Sidelink resource re-selection counter must be zero before "
                                      "continuing with the same grant for dst "
                                          << itDst.first);
                        // keeping the resource, reassign the same sidelink resource re-selection
                        // counter we chose while creating the fresh grant
                        itGrantInfo->second.slResoReselCounter =
                            itGrantInfo->second.prevSlResoReselCounter;
                        continue;
                    }
                    else
                    {
                        // we need to choose new resource so erase the previous allocations
                        NS_LOG_DEBUG("Choosing new resources : ResoReselCounter "
                                     << +itGrantInfo->second.slResoReselCounter << " cResel "
                                     << itGrantInfo->second.cReselCounter << " remaining alloc "
                                     << itGrantInfo->second.slotAllocations.size()
                                     << " slProbResourceKeep " << +m_slProbResourceKeep
                                     << " random prob " << randProb);
                        itGrantInfo->second.slotAllocations.erase(
                            itGrantInfo->second.slotAllocations.begin(),
                            itGrantInfo->second.slotAllocations.end());
                    }
                }

                m_reselCounter = GetRndmReselectionCounter();
                m_cResel = m_reselCounter * 10;
                NS_LOG_DEBUG("Resel Counter " << +m_reselCounter << " cResel " << m_cResel);

                // LC priority is not known here; use a provisional value of zero until this
                // code is moved to the scheduler.
                // Packet Delay Budget is not yet supported in NR SL; use a provisional value of
                // 20ms
                // Also, in future revisions to the scheduler, the scheduler will determine the
                // number of subchannels to request, based on the TB size and MCS.  Here, just
                // request a single subchannel, and the outcome will be converted below
                // to the old style representation of resources that the current simple
                // scheduler expects.
                NrSlTransmissionParams params{0,
                                              MilliSeconds(20),
                                              1, // one subchannel
                                              GetReservationPeriod(),
                                              m_cResel};
                std::list<NrSlSlotInfo> availableResources = GetNrSlAvailableResources(sfn, params);
                if (!availableResources.empty())
                {
                    // we ask the scheduler for resources only if the filtered list is not empty.
                    NS_LOG_INFO("IMSI " << GetImsi() << " scheduling the destination "
                                        << itDst.first);
                    // This (temporary) method exists to convert the new style
                    // list of resources to the old style list of slots with
                    // occupiedSbCh set of available subchannel indices.  It
                    // will be removed upon scheduler upgrade.
                    std::list<NrSlSlotInfo> resourcesForSimpleScheduler =
                        ConvertResources(availableResources);
                    m_nrSlUeMacScheduler->SchedNrSlTriggerReq(itDst.first,
                                                              resourcesForSimpleScheduler);
                    m_reselCounter = 0;
                    m_cResel = 0;
                }
                else
                {
                    NS_LOG_DEBUG(
                        "Do not have enough slots to allocate. Not calling the scheduler for dst "
                        << itDst.first);
                    m_reselCounter = 0;
                    m_cResel = 0;
                }
            }
        }
    }
    else
    {
        NS_FATAL_ERROR("Scheduling type " << m_slTxPool->GetNrSlSchedulingType()
                                          << " for NR Sidelink pools is unknown");
    }

    // check if we need to transmit PSCCH + PSSCH
    // We are starting with the transmission of data packets because if the buffer
    // at the RLC would be empty we just erase the grant of the current slot
    // without transmitting SCI 1 and SCI 2 message, and data. Therefore,
    // even we had the grant we will not put anything in the queues at the PHY.
    for (auto& itGrantInfo : m_grantInfo)
    {
        if (itGrantInfo.second.slResoReselCounter != 0 &&
            itGrantInfo.second.slotAllocations.begin()->sfn == sfn)
        {
            auto grantIt = itGrantInfo.second.slotAllocations.begin();
            NrSlSlotAlloc currentGrant = *grantIt;
            // remove the allocation since we already used it
            itGrantInfo.second.slotAllocations.erase(grantIt);
            NS_LOG_INFO("Grant at : Frame = " << currentGrant.sfn.GetFrame()
                                              << " SF = " << +currentGrant.sfn.GetSubframe()
                                              << " slot = " << currentGrant.sfn.GetSlot());

            uint32_t tbSize = 0;
            // sum all the assigned bytes to each LC of this destination
            for (const auto& it : currentGrant.slRlcPduInfo)
            {
                NS_LOG_DEBUG("LC " << static_cast<uint16_t>(it.lcid) << " was assigned " << it.size
                                   << "bytes");
                tbSize += it.size;
            }

            if (currentGrant.ndi)
            {
                itGrantInfo.second.tbTxCounter = 1;
                for (const auto& itLcRlcPduInfo : currentGrant.slRlcPduInfo)
                {
                    SidelinkLcIdentifier slLcId;
                    slLcId.lcId = itLcRlcPduInfo.lcid;
                    slLcId.srcL2Id = m_srcL2Id;
                    slLcId.dstL2Id = currentGrant.dstL2Id;
                    const auto& itLc = m_nrSlLcInfoMap.find(slLcId);
                    NS_ASSERT_MSG(itLc != m_nrSlLcInfoMap.end(),
                                  "No LC with id " << +itLcRlcPduInfo.lcid
                                                   << " found for destination "
                                                   << currentGrant.dstL2Id);
                    // Assign HARQ id and store it in the grant
                    // Side effect, if RLC buffer would be empty, the assigned
                    // HARQ id will be occupied until all the grants for this slot
                    // and it ReTxs are exhausted.
                    uint8_t nrSlHarqId{std::numeric_limits<uint8_t>::max()};
                    nrSlHarqId = m_nrSlHarq->AssignNrSlHarqProcessId(currentGrant.dstL2Id);
                    itGrantInfo.second.nrSlHarqId = nrSlHarqId;
                    NS_ASSERT_MSG(
                        itGrantInfo.second.nrSlHarqId != std::numeric_limits<uint8_t>::max(),
                        "HARQ id was not assigned for destination " << currentGrant.dstL2Id);
                    NS_LOG_DEBUG("Notifying NR SL RLC of TX opportunity for LC id "
                                 << +itLcRlcPduInfo.lcid << " for TB size " << itLcRlcPduInfo.size);
                    itLc->second.macSapUser->NotifyNrSlTxOpportunity(
                        NrSlMacSapUser::NrSlTxOpportunityParameters(itLcRlcPduInfo.size,
                                                                    GetRnti(),
                                                                    itLcRlcPduInfo.lcid,
                                                                    0,
                                                                    itGrantInfo.second.nrSlHarqId,
                                                                    GetBwpId(),
                                                                    m_srcL2Id,
                                                                    currentGrant.dstL2Id));
                }
                if (itGrantInfo.second.tbTxCounter == itGrantInfo.second.nSelected)
                {
                    // 38.321 5.22.1.3.1a says: if this transmission corresponds
                    // to the last transmission of the MAC PDU, decrement
                    // SL_RESOURCE_RESELECTION_COUNTER by 1, if available.
                    --itGrantInfo.second.slResoReselCounter;
                    --itGrantInfo.second.cReselCounter;
                    // Clear the HARQ buffer since we assign the HARQ id
                    // and put the TB in HARQ buffer (if RLC buffer was not empty)
                    // even if the retxs are not configured.
                    m_nrSlHarq->RecvNrSlHarqFeedback(currentGrant.dstL2Id,
                                                     itGrantInfo.second.nrSlHarqId);
                }
            }
            else
            {
                // retx from MAC HARQ buffer
                // we might want to match the LC ids in currentGrant.slRlcPduInfo and
                // the LC ids whose packets are in the packet burst in the HARQ
                // buffer. I am not doing it at the moment as it might slow down
                // the simulation.
                itGrantInfo.second.tbTxCounter++;
                Ptr<PacketBurst> pb = CreateObject<PacketBurst>();
                pb =
                    m_nrSlHarq->GetPacketBurst(currentGrant.dstL2Id, itGrantInfo.second.nrSlHarqId);
                if (m_enableBlindReTx)
                {
                    if (pb->GetNPackets() > 0)
                    {
                        m_nrSlMacPduTxed = true;
                        NS_ASSERT_MSG(pb->GetNPackets() > 0,
                                      "Packet burst for HARQ id " << +itGrantInfo.second.nrSlHarqId
                                                                  << " is empty");
                        for (const auto& itPkt : pb->GetPackets())
                        {
                            m_nrSlUePhySapProvider->SendPsschMacPdu(itPkt);
                        }
                    }
                    else
                    {
                        NS_LOG_DEBUG("Wasted Retx grant");
                    }
                    if (itGrantInfo.second.tbTxCounter == itGrantInfo.second.nSelected)
                    {
                        // 38.321 5.22.1.3.1a says: if this transmission corresponds
                        // to the last transmission of the MAC PDU, decrement
                        // SL_RESOURCE_RESELECTION_COUNTER by 1, if available.
                        --itGrantInfo.second.slResoReselCounter;
                        --itGrantInfo.second.cReselCounter;
                        itGrantInfo.second.tbTxCounter = 0;
                        // generate fake feedback. It is important to clear the
                        // HARQ buffer, which make the HARQ id available again
                        // since we assign the HARQ id even in the end
                        // RLC buffer is empty. See the for loop above to trigger RLC.
                        NS_LOG_INFO("sending fake HARQ feedback for HARQ id "
                                    << +itGrantInfo.second.nrSlHarqId);
                        m_nrSlHarq->RecvNrSlHarqFeedback(currentGrant.dstL2Id,
                                                         itGrantInfo.second.nrSlHarqId);
                    }
                }
                else
                {
                    // we need to have a feedback to do the retx when blind retx
                    // are not enabled.
                    NS_FATAL_ERROR("Feedback based retransmissions are not supported");
                }
            }

            if (!m_nrSlMacPduTxed)
            {
                // NR SL MAC PDU was not txed. It can happen if RLC buffer was empty
                NS_LOG_DEBUG("Grant wasted at : Frame = "
                             << currentGrant.sfn.GetFrame()
                             << " SF = " << +currentGrant.sfn.GetSubframe()
                             << " slot = " << currentGrant.sfn.GetSlot());
                continue;
            }
            atLeastOneTransmissionInSlot = true;
            // prepare and send SCI format 2A message
            NrSlSciF2aHeader sciF2a;
            sciF2a.SetHarqId(itGrantInfo.second.nrSlHarqId);
            sciF2a.SetNdi(currentGrant.ndi);
            sciF2a.SetRv(currentGrant.rv);
            sciF2a.SetSrcId(m_srcL2Id);
            sciF2a.SetDstId(currentGrant.dstL2Id);
            // fields which are not used yet that is why we set them to 0
            sciF2a.SetCsiReq(0);
            sciF2a.SetHarqFbIndicator(0);
            sciF2a.SetCastType(NrSlSciF2aHeader::Broadcast);
            Ptr<Packet> pktSciF02 = Create<Packet>();
            pktSciF02->AddHeader(sciF2a);
            // put SCI stage 2 in PSSCH queue
            m_nrSlUePhySapProvider->SendPsschMacPdu(pktSciF02);

            // set the VarTti allocation info for PSSCH
            NrSlVarTtiAllocInfo dataVarTtiInfo;
            dataVarTtiInfo.SlVarTtiType = NrSlVarTtiAllocInfo::DATA;
            dataVarTtiInfo.symStart = currentGrant.slPsschSymStart;
            dataVarTtiInfo.symLength = currentGrant.slPsschSymLength;
            dataVarTtiInfo.rbStart =
                currentGrant.slPsschSubChStart * m_slTxPool->GetNrSlSubChSize(GetBwpId(), m_poolId);
            dataVarTtiInfo.rbLength = currentGrant.slPsschSubChLength *
                                      m_slTxPool->GetNrSlSubChSize(GetBwpId(), m_poolId);
            m_nrSlUePhySapProvider->SetNrSlVarTtiAllocInfo(sfn, dataVarTtiInfo);

            // Collect statistics for NR SL PSCCH UE MAC scheduling trace
            SlPsschUeMacStatParameters psschStatsParams;
            psschStatsParams.timeMs = Simulator::Now().GetSeconds() * 1000.0;
            psschStatsParams.imsi = GetImsi();
            psschStatsParams.rnti = GetRnti();
            psschStatsParams.frameNum = currentGrant.sfn.GetFrame();
            psschStatsParams.subframeNum = currentGrant.sfn.GetSubframe();
            psschStatsParams.slotNum = currentGrant.sfn.GetSlot();
            psschStatsParams.symStart = currentGrant.slPsschSymStart;
            psschStatsParams.symLength = currentGrant.slPsschSymLength;
            psschStatsParams.rbStart =
                currentGrant.slPsschSubChStart * m_slTxPool->GetNrSlSubChSize(GetBwpId(), m_poolId);
            psschStatsParams.subChannelSize = m_slTxPool->GetNrSlSubChSize(GetBwpId(), m_poolId);
            psschStatsParams.rbLength = currentGrant.slPsschSubChLength *
                                        m_slTxPool->GetNrSlSubChSize(GetBwpId(), m_poolId);
            psschStatsParams.harqId = itGrantInfo.second.nrSlHarqId;
            psschStatsParams.ndi = currentGrant.ndi;
            psschStatsParams.rv = currentGrant.rv;
            psschStatsParams.srcL2Id = m_srcL2Id;
            psschStatsParams.dstL2Id = currentGrant.dstL2Id;
            psschStatsParams.csiReq = sciF2a.GetCsiReq();
            psschStatsParams.castType = sciF2a.GetCastType();
            psschStatsParams.resoReselCounter = itGrantInfo.second.slResoReselCounter;
            psschStatsParams.cReselCounter = itGrantInfo.second.cReselCounter;
            m_slPsschScheduling(psschStatsParams); // Trace

            if (currentGrant.txSci1A)
            {
                // prepare and send SCI format 1A message
                NrSlSciF1aHeader sciF1a;
                sciF1a.SetPriority(currentGrant.priority);
                sciF1a.SetMcs(currentGrant.mcs);
                sciF1a.SetSciStage2Format(NrSlSciF1aHeader::SciFormat2A);
                sciF1a.SetSlResourceReservePeriod(
                    static_cast<uint16_t>(m_pRsvpTx.GetMilliSeconds()));
                sciF1a.SetTotalSubChannels(GetTotalSubCh());
                sciF1a.SetIndexStartSubChannel(currentGrant.slPsschSubChStart);
                sciF1a.SetLengthSubChannel(currentGrant.slPsschSubChLength);
                sciF1a.SetSlMaxNumPerReserve(currentGrant.maxNumPerReserve);
                if (currentGrant.slotNumInd > 1)
                {
                    // itGrantInfo.second.slotAllocations.cbegin () points to
                    // the next slot allocation this slot has to indicate
                    std::vector<uint8_t> gaps =
                        ComputeGaps(currentGrant.sfn,
                                    itGrantInfo.second.slotAllocations.cbegin(),
                                    currentGrant.slotNumInd);
                    std::vector<uint8_t> sbChIndex =
                        GetStartSbChOfReTx(itGrantInfo.second.slotAllocations.cbegin(),
                                           currentGrant.slotNumInd);
                    sciF1a.SetGapReTx1(gaps.at(0));
                    sciF1a.SetIndexStartSbChReTx1(sbChIndex.at(0));
                    if (gaps.size() > 1)
                    {
                        sciF1a.SetGapReTx2(gaps.at(1));
                        NS_ASSERT_MSG(gaps.at(0) < gaps.at(1),
                                      "Incorrect computation of ReTx slot gaps");
                        sciF1a.SetIndexStartSbChReTx2(sbChIndex.at(1));
                    }
                }

                Ptr<Packet> pktSciF1a = Create<Packet>();
                pktSciF1a->AddHeader(sciF1a);
                NrSlMacPduTag tag(GetRnti(),
                                  currentGrant.sfn,
                                  currentGrant.slPsschSymStart,
                                  currentGrant.slPsschSymLength,
                                  tbSize,
                                  currentGrant.dstL2Id);
                pktSciF1a->AddPacketTag(tag);

                m_nrSlUePhySapProvider->SendPscchMacPdu(pktSciF1a);

                // set the VarTti allocation info for PSCCH
                NrSlVarTtiAllocInfo ctrlVarTtiInfo;
                ctrlVarTtiInfo.SlVarTtiType = NrSlVarTtiAllocInfo::CTRL;
                ctrlVarTtiInfo.symStart = currentGrant.slPscchSymStart;
                ctrlVarTtiInfo.symLength = currentGrant.slPscchSymLength;
                ctrlVarTtiInfo.rbStart = currentGrant.slPsschSubChStart *
                                         m_slTxPool->GetNrSlSubChSize(GetBwpId(), m_poolId);
                ctrlVarTtiInfo.rbLength = currentGrant.numSlPscchRbs;
                m_nrSlUePhySapProvider->SetNrSlVarTtiAllocInfo(sfn, ctrlVarTtiInfo);

                // Collect statistics for NR SL PSCCH UE MAC scheduling trace
                SlPscchUeMacStatParameters pscchStatsParams;
                pscchStatsParams.timeMs = Simulator::Now().GetSeconds() * 1000.0;
                pscchStatsParams.imsi = GetImsi();
                pscchStatsParams.rnti = GetRnti();
                pscchStatsParams.frameNum = currentGrant.sfn.GetFrame();
                pscchStatsParams.subframeNum = currentGrant.sfn.GetSubframe();
                pscchStatsParams.slotNum = currentGrant.sfn.GetSlot();
                pscchStatsParams.symStart = currentGrant.slPscchSymStart;
                pscchStatsParams.symLength = currentGrant.slPscchSymLength;
                pscchStatsParams.rbStart = currentGrant.slPsschSubChStart *
                                           m_slTxPool->GetNrSlSubChSize(GetBwpId(), m_poolId);
                pscchStatsParams.rbLength = currentGrant.numSlPscchRbs;
                pscchStatsParams.priority = currentGrant.priority;
                pscchStatsParams.mcs = currentGrant.mcs;
                pscchStatsParams.tbSize = tbSize;
                pscchStatsParams.slResourceReservePeriod =
                    static_cast<uint16_t>(m_pRsvpTx.GetMilliSeconds());
                pscchStatsParams.totalSubChannels = GetTotalSubCh();
                pscchStatsParams.slPsschSubChStart = currentGrant.slPsschSubChStart;
                pscchStatsParams.slPsschSubChLength = currentGrant.slPsschSubChLength;
                pscchStatsParams.slMaxNumPerReserve = currentGrant.maxNumPerReserve;
                pscchStatsParams.gapReTx1 = sciF1a.GetGapReTx1();
                pscchStatsParams.gapReTx2 = sciF1a.GetGapReTx2();
                m_slPscchScheduling(pscchStatsParams); // Trace
            }
        }
        else
        {
            // When there are no resources it may happen that the re-selection
            // counter of already existing destination remains zero. In this case,
            // we just go the next destination, if any.
        }
        // make this false before processing the grant for next destination
        m_nrSlMacPduTxed = false;
    }
    if (atLeastOneTransmissionInSlot)
    {
        NS_LOG_DEBUG("IMSI " << GetImsi() << " adding SFN history at sfn " << sfn);
        m_transmitHistory.push_back(sfn);
    }
}

std::vector<uint8_t>
NrSlUeMac::ComputeGaps(const SfnSf& sfn,
                       std::set<NrSlSlotAlloc>::const_iterator it,
                       uint8_t slotNumInd)
{
    NS_LOG_FUNCTION(this);
    std::vector<uint8_t> gaps;
    // slotNumInd is the number including the first TX. Gaps are computed only for
    // the ReTxs
    for (uint8_t i = 0; i < slotNumInd - 1; i++)
    {
        std::advance(it, i);
        gaps.push_back(static_cast<uint8_t>(it->sfn.Normalize() - sfn.Normalize()));
    }

    return gaps;
}

// Temporary converter method to be removed upon scheduler upgrade
std::list<NrSlSlotInfo>
NrSlUeMac::ConvertResources(const std::list<NrSlSlotInfo>& availableResources) const
{
    std::list<NrSlSlotInfo> listForScheduler;
    if (availableResources.empty())
    {
        return listForScheduler;
    }
    NrSlSlotInfo currentSlot{availableResources.front()};
    for (const auto& it : availableResources)
    {
        if (currentSlot < it)
        {
            listForScheduler.push_back(currentSlot);
            currentSlot = it;
            currentSlot.occupiedSbCh.insert(it.slSubchannelStart);
        }
        else
        {
            currentSlot.occupiedSbCh.insert(it.slSubchannelStart);
        }
    }
    listForScheduler.push_back(currentSlot);
    return listForScheduler;
}

std::vector<uint8_t>
NrSlUeMac::GetStartSbChOfReTx(std::set<NrSlSlotAlloc>::const_iterator it, uint8_t slotNumInd)
{
    NS_LOG_FUNCTION(this);
    std::vector<uint8_t> startSbChIndex;
    // slotNumInd is the number including the first TX. Start sub-channel index or
    // indices are retrieved only for the ReTxs
    for (uint8_t i = 0; i < slotNumInd - 1; i++)
    {
        std::advance(it, i);
        startSbChIndex.push_back(it->slPsschSubChStart);
    }

    return startSbChIndex;
}

NrSlUeMac::NrSlGrantInfo
NrSlUeMac::CreateGrantInfo(const std::set<NrSlSlotAlloc>& slotAllocList)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG((m_reselCounter != 0), "Can not create grants with 0 Resource selection counter");
    NS_ASSERT_MSG((m_cResel != 0), "Can not create grants with 0 cResel counter");

    NS_LOG_DEBUG("Creating grants with Resel Counter " << +m_reselCounter << " and cResel "
                                                       << m_cResel);

    uint16_t resPeriodSlots =
        m_slTxPool->GetResvPeriodInSlots(GetBwpId(),
                                         m_poolId,
                                         m_pRsvpTx,
                                         m_nrSlUePhySapProvider->GetSlotPeriod());
    NrSlGrantInfo grant;

    grant.cReselCounter = m_cResel;
    // save reselCounter to be used if probability of keeping the resource would
    // be higher than the configured one
    grant.prevSlResoReselCounter = m_reselCounter;
    grant.slResoReselCounter = m_reselCounter;

    grant.nSelected = static_cast<uint8_t>(slotAllocList.size());
    NS_LOG_DEBUG("nSelected = " << +grant.nSelected);

    for (uint16_t i = 0; i < m_cResel; i++)
    {
        for (const auto& it : slotAllocList)
        {
            auto slAlloc = it;
            slAlloc.sfn.Add(i * resPeriodSlots);
            NS_LOG_DEBUG("First tx at : Frame = " << slAlloc.sfn.GetFrame()
                                                  << " SF = " << +slAlloc.sfn.GetSubframe()
                                                  << " slot = " << slAlloc.sfn.GetSlot());
            bool insertStatus = grant.slotAllocations.emplace(slAlloc).second;
            NS_ASSERT_MSG(insertStatus, "slot allocation already exist");
        }
    }

    return grant;
}

NrSlMacSapProvider*
NrSlUeMac::GetNrSlMacSapProvider()
{
    NS_LOG_FUNCTION(this);
    return m_nrSlMacSapProvider;
}

void
NrSlUeMac::SetNrSlMacSapUser(NrSlMacSapUser* s)
{
    NS_LOG_FUNCTION(this);
    m_nrSlMacSapUser = s;
}

NrSlUeCmacSapProvider*
NrSlUeMac::GetNrSlUeCmacSapProvider()
{
    NS_LOG_FUNCTION(this);
    return m_nrSlUeCmacSapProvider;
}

void
NrSlUeMac::SetNrSlUeCmacSapUser(NrSlUeCmacSapUser* s)
{
    NS_LOG_FUNCTION(this);
    m_nrSlUeCmacSapUser = s;
}

NrSlUePhySapUser*
NrSlUeMac::GetNrSlUePhySapUser()
{
    NS_LOG_FUNCTION(this);
    return m_nrSlUePhySapUser;
}

void
NrSlUeMac::SetNrSlUePhySapProvider(NrSlUePhySapProvider* s)
{
    NS_LOG_FUNCTION(this);
    m_nrSlUePhySapProvider = s;
}

void
NrSlUeMac::DoTransmitNrSlRlcPdu(const NrSlMacSapProvider::NrSlRlcPduParameters& params)
{
    NS_LOG_FUNCTION(this << +params.lcid << +params.harqProcessId);
    LteRadioBearerTag bearerTag(params.rnti, params.lcid, 0);
    params.pdu->AddPacketTag(bearerTag);
    NS_LOG_DEBUG("Adding packet in HARQ buffer for HARQ id "
                 << +params.harqProcessId << " pkt size " << params.pdu->GetSize());
    m_nrSlHarq->AddPacket(params.dstL2Id, params.lcid, params.harqProcessId, params.pdu);
    m_nrSlUePhySapProvider->SendPsschMacPdu(params.pdu);
    m_nrSlMacPduTxed = true;
}

void
NrSlUeMac::DoReportNrSlBufferStatus(
    const NrSlMacSapProvider::NrSlReportBufferStatusParameters& params)
{
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("Reporting for Sidelink. Tx Queue size = " << params.txQueueSize);
    // Sidelink BSR
    std::map<SidelinkLcIdentifier, NrSlMacSapProvider::NrSlReportBufferStatusParameters>::iterator
        it;

    SidelinkLcIdentifier slLcId;
    slLcId.lcId = params.lcid;
    slLcId.srcL2Id = params.srcL2Id;
    slLcId.dstL2Id = params.dstL2Id;

    it = m_nrSlBsrReceived.find(slLcId);
    if (it != m_nrSlBsrReceived.end())
    {
        // update entry
        (*it).second = params;
    }
    else
    {
        m_nrSlBsrReceived.insert(std::make_pair(slLcId, params));
    }

    m_nrSlUeMacScheduler->SchedNrSlRlcBufferReq(params);
}

void
NrSlUeMac::DoAddNrSlLc(const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& slLcInfo,
                       NrSlMacSapUser* msu)
{
    NS_LOG_FUNCTION(this << +slLcInfo.lcId << slLcInfo.srcL2Id << slLcInfo.dstL2Id);
    SidelinkLcIdentifier slLcIdentifier;
    slLcIdentifier.lcId = slLcInfo.lcId;
    slLcIdentifier.srcL2Id = slLcInfo.srcL2Id;
    slLcIdentifier.dstL2Id = slLcInfo.dstL2Id;

    NS_ASSERT_MSG(m_nrSlLcInfoMap.find(slLcIdentifier) == m_nrSlLcInfoMap.end(),
                  "cannot add LCID " << +slLcInfo.lcId << ", srcL2Id " << slLcInfo.srcL2Id
                                     << ", dstL2Id " << slLcInfo.dstL2Id << " is already present");

    SlLcInfoUeMac slLcInfoUeMac;
    slLcInfoUeMac.lcInfo = slLcInfo;
    slLcInfoUeMac.macSapUser = msu;
    m_nrSlLcInfoMap.insert(std::make_pair(slLcIdentifier, slLcInfoUeMac));

    // Following if is needed because this method is called for both
    // TX and RX LCs addition into m_nrSlLcInfoMap. In case of RX LC, the
    // destination is this UE MAC.
    if (slLcInfo.srcL2Id == m_srcL2Id)
    {
        NS_LOG_DEBUG("UE MAC with src id " << m_srcL2Id << " giving info of LC to the scheduler");
        m_nrSlUeMacScheduler->CschedNrSlLcConfigReq(slLcInfo);
        AddNrSlDstL2Id(slLcInfo.dstL2Id, slLcInfo.priority);
    }
}

void
NrSlUeMac::DoRemoveNrSlLc(uint8_t slLcId, uint32_t srcL2Id, uint32_t dstL2Id)
{
    NS_LOG_FUNCTION(this << +slLcId << srcL2Id << dstL2Id);
    NS_ASSERT_MSG(slLcId > 3, "Hey! I can delete only the LC for data radio bearers.");
    SidelinkLcIdentifier slLcIdentifier;
    slLcIdentifier.lcId = slLcId;
    slLcIdentifier.srcL2Id = srcL2Id;
    slLcIdentifier.dstL2Id = dstL2Id;
    NS_ASSERT_MSG(m_nrSlLcInfoMap.find(slLcIdentifier) != m_nrSlLcInfoMap.end(),
                  "could not find Sidelink LCID " << slLcId);
    m_nrSlLcInfoMap.erase(slLcIdentifier);
}

void
NrSlUeMac::DoResetNrSlLcMap()
{
    NS_LOG_FUNCTION(this);

    auto it = m_nrSlLcInfoMap.begin();

    while (it != m_nrSlLcInfoMap.end())
    {
        if (it->first.lcId > 3) // SL DRB LC starts from 4
        {
            m_nrSlLcInfoMap.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void
NrSlUeMac::AddNrSlDstL2Id(uint32_t dstL2Id, uint8_t lcPriority)
{
    NS_LOG_FUNCTION(this << dstL2Id << lcPriority);
    bool foundDst = false;
    for (auto& it : m_sidelinkTxDestinations)
    {
        if (it.first == dstL2Id)
        {
            foundDst = true;
            if (lcPriority < it.second)
            {
                it.second = lcPriority;
            }
            break;
        }
    }

    if (!foundDst)
    {
        m_sidelinkTxDestinations.emplace_back(dstL2Id, lcPriority);
    }

    std::sort(m_sidelinkTxDestinations.begin(), m_sidelinkTxDestinations.end(), CompareSecond);
}

bool
NrSlUeMac::CompareSecond(std::pair<uint32_t, uint8_t>& a, std::pair<uint32_t, uint8_t>& b)
{
    return a.second < b.second;
}

void
NrSlUeMac::DoAddNrSlCommTxPool(Ptr<const NrSlCommResourcePool> txPool)
{
    NS_LOG_FUNCTION(this << txPool);
    m_slTxPool = txPool;
    m_slTxPool->ValidateResvPeriod(GetBwpId(),
                                   m_poolId,
                                   m_pRsvpTx,
                                   m_nrSlUePhySapProvider->GetSlotPeriod());
}

void
NrSlUeMac::DoAddNrSlCommRxPool(Ptr<const NrSlCommResourcePool> rxPool)
{
    NS_LOG_FUNCTION(this);
    m_slRxPool = rxPool;
}

void
NrSlUeMac::DoSetSlProbResoKeep(double prob)
{
    NS_LOG_FUNCTION(this << prob);
    NS_ASSERT_MSG(prob <= 1.0, "Probability value must be between 0 and 1");
    m_slProbResourceKeep = prob;
}

void
NrSlUeMac::DoSetSlMaxTxTransNumPssch(uint8_t maxTxPssch)
{
    NS_LOG_FUNCTION(this << +maxTxPssch);
    NS_ASSERT_MSG(maxTxPssch <= 32, "Number of PSSCH transmissions can not exceed 32");
    m_slMaxTxTransNumPssch = maxTxPssch;
}

void
NrSlUeMac::DoSetSourceL2Id(uint32_t srcL2Id)
{
    NS_LOG_FUNCTION(this << srcL2Id);
    m_srcL2Id = srcL2Id;
}

void
NrSlUeMac::DoAddNrSlRxDstL2Id(uint32_t dstL2Id)
{
    NS_LOG_FUNCTION(this << dstL2Id);
    m_sidelinkRxDestinations.insert(dstL2Id);
}

uint8_t
NrSlUeMac::DoGetSlActiveTxPoolId() const
{
    return GetSlActivePoolId();
}

std::vector<std::pair<uint32_t, uint8_t>>
NrSlUeMac::DoGetSlTxDestinations()
{
    return m_sidelinkTxDestinations;
}

std::unordered_set<uint32_t>
NrSlUeMac::DoGetSlRxDestinations()
{
    return m_sidelinkRxDestinations;
}

uint8_t
NrSlUeMac::GetSlMaxTxTransNumPssch() const
{
    return m_slMaxTxTransNumPssch;
}

void
NrSlUeMac::CschedNrSlLcConfigCnf(uint8_t lcg, uint8_t lcId)
{
    NS_LOG_FUNCTION(this << +lcg << +lcId);
    NS_LOG_INFO("SL UE scheduler successfully added LCG " << +lcg << " LC id " << +lcId);
}

NrSlUeMac::NrSlTransmissionParams::NrSlTransmissionParams(uint8_t prio,
                                                          Time pdb,
                                                          uint16_t lSubch,
                                                          Time pRsvpTx,
                                                          uint16_t cResel)
    : m_priority(prio),
      m_packetDelayBudget(pdb),
      m_lSubch(lSubch),
      m_pRsvpTx(pRsvpTx),
      m_cResel(cResel)
{
}

void
NrSlUeMac::EnableSensing(bool enableSensing)
{
    NS_LOG_FUNCTION(this << enableSensing);
    NS_ASSERT_MSG(m_enableSensing == false,
                  " Once the sensing is enabled, it can not be enabled or disabled again");
    m_enableSensing = enableSensing;
}

void
NrSlUeMac::EnableBlindReTx(bool enableBlindReTx)
{
    NS_LOG_FUNCTION(this << enableBlindReTx);
    NS_ASSERT_MSG(
        m_enableBlindReTx == false,
        " Once the blind re-transmission is enabled, it can not be enabled or disabled again");
    m_enableBlindReTx = enableBlindReTx;
}

void
NrSlUeMac::SetTproc0(uint8_t tproc0)
{
    NS_LOG_FUNCTION(this << +tproc0);
    m_tproc0 = tproc0;
}

uint8_t
NrSlUeMac::GetTproc0() const
{
    return m_tproc0;
}

uint8_t
NrSlUeMac::GetT1() const
{
    return m_t1;
}

void
NrSlUeMac::SetT1(uint8_t t1)
{
    NS_LOG_FUNCTION(this << +t1);
    m_t1 = t1;
}

uint16_t
NrSlUeMac::GetT2() const
{
    return m_t2;
}

void
NrSlUeMac::SetT2(uint16_t t2)
{
    NS_LOG_FUNCTION(this << t2);
    m_t2 = t2;
}

uint16_t
NrSlUeMac::GetSlActivePoolId() const
{
    return m_poolId;
}

void
NrSlUeMac::SetSlActivePoolId(uint16_t poolId)
{
    m_poolId = poolId;
}

uint8_t
NrSlUeMac::GetTotalSubCh() const
{
    uint16_t subChSize = m_slTxPool->GetNrSlSubChSize(static_cast<uint8_t>(GetBwpId()), m_poolId);

    uint8_t totalSubChannels =
        static_cast<uint8_t>(std::floor(m_nrSlUePhySapProvider->GetBwInRbs() / subChSize));

    return totalSubChannels;
}

void
NrSlUeMac::SetReservationPeriod(const Time& rsvp)
{
    NS_LOG_FUNCTION(this << rsvp);
    m_pRsvpTx = rsvp;
}

Time
NrSlUeMac::GetReservationPeriod() const
{
    return m_pRsvpTx;
}

uint8_t
NrSlUeMac::GetRndmReselectionCounter() const
{
    uint8_t min;
    uint8_t max;
    uint16_t periodInt = static_cast<uint16_t>(m_pRsvpTx.GetMilliSeconds());

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
            min = GetLoBoundReselCounter(periodInt);
            max = GetUpBoundReselCounter(periodInt);
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
NrSlUeMac::GetLoBoundReselCounter(uint16_t pRsrv) const
{
    NS_LOG_FUNCTION(this << pRsrv);
    NS_ASSERT_MSG(pRsrv < 100, "Resource reservation must be less than 100 ms");
    uint8_t lBound = (5 * std::ceil(100 / (std::max(static_cast<uint16_t>(20), pRsrv))));
    return lBound;
}

uint8_t
NrSlUeMac::GetUpBoundReselCounter(uint16_t pRsrv) const
{
    NS_LOG_FUNCTION(this << pRsrv);
    NS_ASSERT_MSG(pRsrv < 100, "Resource reservation must be less than 100 ms");
    uint8_t uBound = (15 * std::ceil(100 / (std::max(static_cast<uint16_t>(20), pRsrv))));
    return uBound;
}

void
NrSlUeMac::SetNumSidelinkProcess(uint8_t numSidelinkProcess)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(
        m_grantInfo.empty(),
        "Can not reset the number of Sidelink processes. Scheduler already assigned grants");
    m_numSidelinkProcess = numSidelinkProcess;
    m_nrSlHarq->InitHarqBuffer(m_numSidelinkProcess);
}

uint8_t
NrSlUeMac::GetNumSidelinkProcess() const
{
    NS_LOG_FUNCTION(this);
    return m_numSidelinkProcess;
}

void
NrSlUeMac::SetSlThresPsschRsrp(int thresRsrp)
{
    NS_LOG_FUNCTION(this);
    m_thresRsrp = thresRsrp;
}

int
NrSlUeMac::GetSlThresPsschRsrp() const
{
    NS_LOG_FUNCTION(this);
    return m_thresRsrp;
}

void
NrSlUeMac::SetResourcePercentage(uint8_t percentage)
{
    NS_LOG_FUNCTION(this);
    m_resPercentage = percentage;
}

uint8_t
NrSlUeMac::GetResourcePercentage() const
{
    NS_LOG_FUNCTION(this);
    return m_resPercentage;
}

void
NrSlUeMac::FireTraceSlRlcRxPduWithTxRnti(const Ptr<Packet> p, uint8_t lcid)
{
    NS_LOG_FUNCTION(this);
    // Receiver timestamp
    RlcTag rlcTag;
    Time delay;

    bool ret = p->FindFirstMatchingByteTag(rlcTag);
    NS_ASSERT_MSG(ret, "RlcTag is missing for NR SL");

    delay = Simulator::Now() - rlcTag.GetSenderTimestamp();
    m_rxRlcPduWithTxRnti(GetImsi(),
                         GetRnti(),
                         rlcTag.GetTxRnti(),
                         lcid,
                         p->GetSize(),
                         delay.GetSeconds());
}

std::ostream&
operator<<(std::ostream& os, const NrSlUeMac::NrSlTransmissionParams& p)
{
    os << "Prio: " << +p.m_priority << ", PDB: " << p.m_packetDelayBudget.As(Time::MS)
       << ", subchannels: " << p.m_lSubch << ", RRI: " << p.m_pRsvpTx.As(Time::MS)
       << ", Cresel: " << p.m_cResel;
    return os;
}

std::ostream&
operator<<(std::ostream& os, const struct NrSlUeMac::SensingTraceReport& report)
{
    os << "Sfn (" << report.m_sfn.GetFrame() << ":" << +report.m_sfn.GetSubframe() << ":"
       << +report.m_sfn.GetSlot() << "):" << report.m_sfn.Normalize() << " T0 " << report.m_t0
       << " T_proc0 " << +report.m_tProc0 << " T1 " << +report.m_t1 << " T2 " << report.m_t2
       << " poolSubch " << report.m_subchannels << " lSubch " << report.m_lSubch << " resoPct "
       << +report.m_resourcePercentage << " initCandSlots " << report.m_initialCandidateSlotsSize
       << " initCandReso " << report.m_initialCandidateResourcesSize << " candResoAfterStep5 "
       << report.m_candidateResourcesSizeAfterStep5 << " initRsrp " << report.m_initialRsrpThreshold
       << " finalRsrp " << report.m_finalRsrpThreshold;
    return os;
}

} // namespace ns3
