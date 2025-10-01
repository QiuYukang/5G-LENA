// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-fh-control.h"

#include "ns3/core-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrFhControl");
NS_OBJECT_ENSURE_REGISTERED(NrFhControl);

static constexpr uint32_t
Cantor(uint16_t x1, uint16_t x2)
{
    return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
}

TypeId
NrFhControl::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrFhControl")
            .SetParent<Object>()
            .AddConstructor<NrFhControl>()
            .SetGroupName("Nr")
            .AddAttribute(
                "FhControlMethod",
                "The FH Control method defines the model that the fhControl will use"
                "to limit the capacity. There are four FH Control methods: "
                "a) Dropping. When CTRL channels are sent, PHY asks the FhControl whether"
                "the allocation fits. If not, it drops the DCI + data."
                "b) Postponing. When tdma/ofdma have allocated the RBs/symbols to all the"
                "UEs, it iterates through all the UEs and asks the FhControl whether the"
                "allocation fits. If not, it sets the assigned RBGs to 0 and therefore the"
                "sending of the data is postponed (DCI is not created – data stays in RLC queue)"
                "c) Optimize MCS. When tdma/ofdma have allocated the RBs/symbols to all the UEs,"
                "it iterates through all the UEs (with data in their queues and resources"
                "allocated during the scheduling process) and asks fhControl for the max MCS."
                "It assigns the min among the allocated one and the max MCS."
                "d) Optimize RBs. When tdma/ofdma are allocating the RBs/symbols to a UE,"
                "it calls the fhControl to provide the max RBs that can be assigned.",
                EnumValue(NrFhControl::Dropping),
                MakeEnumAccessor<FhControlMethod>(&NrFhControl::SetFhControlMethod,
                                                  &NrFhControl::GetFhControlMethod),
                MakeEnumChecker(NrFhControl::Dropping,
                                "Dropping",
                                NrFhControl::Postponing,
                                "Postponing",
                                NrFhControl::OptimizeMcs,
                                "OptimizeMcs",
                                NrFhControl::OptimizeRBs,
                                "OptimizeRBs"))
            .AddAttribute("FhCapacity",
                          "The available fronthaul capacity (in Mbps)."
                          "The capacity is shared among the active BWPs"
                          "of a cell.",
                          UintegerValue(1000),
                          MakeUintegerAccessor(&NrFhControl::SetCellFhCapacity),
                          MakeUintegerChecker<uint32_t>(0, 150000))
            .AddAttribute("OverheadDyn",
                          "The overhead for dynamic adaptation (in bits)",
                          UintegerValue(32),
                          MakeUintegerAccessor(&NrFhControl::SetOverheadDyn),
                          MakeUintegerChecker<uint8_t>(0, 100))
            .AddAttribute("EnableDynamicModComp",
                          "Enable dynamic modulation compression for split 7.2",
                          BooleanValue(true),
                          MakeBooleanAccessor(&NrFhControl::SetEnableModComp),
                          MakeBooleanChecker())
            .AddTraceSource(
                "RequiredFhDlThroughput",
                "Report required fronthaul throughput in DL per BWP (Sfnfn, bwpId, reqFhThr)",
                MakeTraceSourceAccessor(&NrFhControl::m_reqFhDlThrTrace),
                "ns3::ReqFhDlThr::TracedCallback")
            .AddTraceSource(
                "UsedAirRbs",
                "Report the employed RBs of the air interface in DL per BWP (Sfnfn, bwpId, rbsAir)",
                MakeTraceSourceAccessor(&NrFhControl::m_rbsAirTrace),
                "ns3::rbsAir::TracedCallback");
    return tid;
}

NrFhControl::NrFhControl()
    : m_physicalCellId(0),
      m_fhPhySapUser{},
      m_fhSchedSapUser{}
{
    NS_LOG_FUNCTION(this);
    m_fhPhySapProvider = new MemberNrFhPhySapProvider<NrFhControl>(this);
    m_fhSchedSapProvider = new MemberNrFhSchedSapProvider<NrFhControl>(this);
}

NrFhControl::~NrFhControl()
{
    delete m_fhPhySapProvider;
    delete m_fhSchedSapProvider;
}

void
NrFhControl::SetNrFhPhySapUser(uint16_t bwpId, NrFhPhySapUser* s)
{
    NS_LOG_FUNCTION(this << s);
    auto it = m_fhPhySapUser.find(bwpId);

    if (it != m_fhPhySapUser.end())
    {
        NS_FATAL_ERROR("Tried to allocated an existing bwpId");
    }
    if (it == m_fhPhySapUser.end())
    {
        m_fhPhySapUser.insert(std::pair<uint8_t, NrFhPhySapUser*>(bwpId, s));
    }
}

NrFhPhySapProvider*
NrFhControl::GetNrFhPhySapProvider()
{
    NS_LOG_FUNCTION(this);

    return m_fhPhySapProvider;
}

void
NrFhControl::SetNrFhSchedSapUser(uint16_t bwpId, NrFhSchedSapUser* s)
{
    NS_LOG_FUNCTION(this << s);

    auto it = m_fhSchedSapUser.find(bwpId);

    if (it != m_fhSchedSapUser.end())
    {
        NS_FATAL_ERROR("Tried to allocated an existing bwpId");
    }
    if (it == m_fhSchedSapUser.end())
    {
        m_fhSchedSapUser.insert(std::pair<uint8_t, NrFhSchedSapUser*>(bwpId, s));
    }
}

NrFhSchedSapProvider*
NrFhControl::GetNrFhSchedSapProvider()
{
    NS_LOG_FUNCTION(this);
    return m_fhSchedSapProvider;
}

void
NrFhControl::SetFhControlMethod(FhControlMethod model)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Set the Fh Control Method to: " << model);
    m_fhControlMethod = model;
}

NrFhControl::FhControlMethod
NrFhControl::GetFhControlMethod() const
{
    NS_LOG_FUNCTION(this);
    return m_fhControlMethod;
}

uint8_t
NrFhControl::DoGetFhControlMethod() const
{
    return m_fhControlMethod;
}

void
NrFhControl::SetCellFhCapacity(uint32_t capacity)
{
    NS_LOG_FUNCTION(this);
    m_fhCapacity = capacity;
}

void
NrFhControl::SetOverheadDyn(uint8_t overhead)
{
    NS_LOG_FUNCTION(this);
    m_overheadDyn = overhead;
}

void
NrFhControl::SetEnableModComp(bool v)
{
    NS_LOG_FUNCTION(this);
    m_enableModComp = v;
}

void
NrFhControl::SetErrorModelType(std::string errorModelType)
{
    m_errorModelType = errorModelType;

    if (m_errorModelType == "ns3::NrEesmIrT1" || m_errorModelType == "ns3::NrEesmCcT1")
    {
        m_mcsTable = 1;
    }
    else if (m_errorModelType == "ns3::NrEesmIrT2" || m_errorModelType == "ns3::NrEesmCcT2")
    {
        m_mcsTable = 2;
    }
    else
    {
        NS_ABORT_MSG(
            "Wrong error model type. To use NrFhControl, one of the Nr error models should be set."
            "Please select among: ns3::NrEesmIrT1, ns3::NrEesmCcT1 for MCS Table 1 and"
            "ns3::NrEesmIrT2 and ns3::NrEesmCcT2 for MCS Table 2");
    }
}

void
NrFhControl::SetPhysicalCellId(uint16_t physicalCellId)
{
    NS_LOG_FUNCTION(this);
    m_physicalCellId = physicalCellId;
    NS_LOG_DEBUG("NrFhControl initialized for cell Id: " << m_physicalCellId);
}

uint16_t
NrFhControl::DoGetPhysicalCellId() const
{
    return m_physicalCellId;
}

void
NrFhControl::SetFhNumerology(uint16_t bwpId, uint16_t num)
{
    if (m_numerologyPerBwp.find(bwpId) == m_numerologyPerBwp.end()) // bwpId not in the map
    {
        m_numerologyPerBwp.insert(std::make_pair(bwpId, num));
        SfnSf waitingSlot = {0, 0, 0, static_cast<uint8_t>(num)};
        m_waitingSlotPerBwp.insert(std::make_pair(bwpId, waitingSlot));
        NS_LOG_DEBUG("Cell: " << m_physicalCellId << " BWP: " << bwpId << " num: " << num);
    }
    else
    {
        NS_ABORT_MSG("Configure NrFhControl should be called only once");
    }
}

void
NrFhControl::DoSetActiveUe(uint16_t bwpId, uint16_t rnti, uint32_t bytes)
{
    if (m_activeUesPerBwp.find(bwpId) == m_activeUesPerBwp.end())
    {
        NS_LOG_DEBUG("Creating m_activeUesPerBwp entry for bwpId: " << bwpId);
        m_activeUesPerBwp[bwpId] = {};
    }
    NS_LOG_DEBUG("Creating m_activeUesPerBwp entry for bwpId: " << bwpId << " and rnti: " << rnti);
    m_activeUesPerBwp.at(bwpId).emplace(rnti);

    uint32_t c1 = Cantor(bwpId, rnti);
    if (m_rntiQueueSize.find(c1) == m_rntiQueueSize.end()) // UE not in the map
    {
        NS_LOG_DEBUG("Cell: " << m_physicalCellId << " Creating pair " << c1 << " for bwpId: "
                              << bwpId << " and rnti: " << rnti << " with bytes: " << bytes);

        m_rntiQueueSize.insert(std::make_pair(c1, bytes));
    }
    else
    {
        NS_LOG_DEBUG("Cell: " << m_physicalCellId << " Updating pair " << c1 << " for bwpId: "
                              << bwpId << " and rnti: " << rnti << " with bytes: " << bytes);
        m_rntiQueueSize.at(c1) = bytes;
    }
}

void
NrFhControl::DoSetActiveHarqUes(uint16_t bwpId, uint16_t rnti)
{
    if (m_activeHarqUesPerBwp.find(bwpId) == m_activeHarqUesPerBwp.end()) // UE not in the map
    {
        NS_LOG_DEBUG("Creating m_activeHarqUesPerBwp entry for bwpId: " << bwpId);
        m_activeHarqUesPerBwp[bwpId] = {};
    }
    NS_LOG_DEBUG("Creating m_activeHarqUesPerBwp entry for bwpId: " << bwpId
                                                                    << " and rnti: " << rnti);
    m_activeHarqUesPerBwp.at(bwpId).emplace(rnti);
}

void
NrFhControl::DoUpdateActiveUesMap(
    uint16_t bwpId,
    const std::deque<VarTtiAllocInfo>& allocation,
    const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& ueMap)
{
    for (const auto& alloc : allocation)
    {
        if (alloc.m_dci->m_type != DciInfoElementTdma::DATA ||
            alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
            continue;
        }

        uint16_t rnti = alloc.m_dci->m_rnti;
        uint32_t c1 = Cantor(bwpId, rnti);
        uint32_t numRbs =
            static_cast<uint32_t>(
                std::count(alloc.m_dci->m_rbgBitmask.begin(), alloc.m_dci->m_rbgBitmask.end(), 1)) *
            static_cast<uint32_t>(m_fhSchedSapUser.at(bwpId)->GetNumRbPerRbgFromSched());

        NS_LOG_INFO("Cell: " << m_physicalCellId << " We got called for Update for bwpId: " << bwpId
                             << " RNTI: " << rnti);

        // Create/Update FH DL Throughput per BWP
        uint64_t fhDlThr = GetFhThr(bwpId,
                                    static_cast<uint32_t>(alloc.m_dci->m_mcs),
                                    static_cast<uint32_t>(alloc.m_dci->m_numSym) * numRbs,
                                    alloc.m_dci->m_rank);
        if (m_reqFhDlThrTracedValuePerBwp.find(bwpId) == m_reqFhDlThrTracedValuePerBwp.end())
        {
            NS_LOG_DEBUG("Create pair for m_reqFhDlThrTracedValuePerBwp.at(" << bwpId
                                                                             << "): " << fhDlThr);
        }
        m_reqFhDlThrTracedValuePerBwp[bwpId] += fhDlThr;
        NS_LOG_DEBUG("Update m_reqFhDlThrTracedValuePerBwp.at("
                     << bwpId << "): " << m_reqFhDlThrTracedValuePerBwp.at(bwpId));

        // Create/Update used RBs of the air of a specific bwpId (AI Trace)
        if (m_rbsAirTracedValue.find(bwpId) == m_rbsAirTracedValue.end()) // bwp not in the map
        {
            NS_LOG_DEBUG("Create pair for m_rbsAirTracedValue.at(" << bwpId << "): " << numRbs
                                                                   << " RBs");
        }
        m_rbsAirTracedValue[bwpId] += numRbs;
        NS_LOG_DEBUG("Update m_rbsAirTracedValue.at(" << bwpId << ")" << m_rbsAirTracedValue[bwpId]
                                                      << " RBs");

        if (alloc.m_dci->m_ndi == 0) // retx
        {
            NS_LOG_DEBUG("Retransmission, update only m_activeHarqUesPerBwp");
            if (m_activeHarqUesPerBwp.find(rnti) != m_activeHarqUesPerBwp.end())
            {
                m_activeHarqUesPerBwp.at(bwpId).erase(rnti);
                NS_LOG_DEBUG("Update m_activeHarqBwps map for bwpId: "
                             << bwpId << " with: " << m_activeHarqUesPerBwp.at(bwpId).size()
                             << " UEs");
                if (m_activeHarqUesPerBwp.at(bwpId).empty())
                {
                    NS_LOG_DEBUG(
                        "Remove BWP from m_activeHarqBwps because we served all its HARQ UEs");
                    m_activeHarqUesPerBwp.erase(bwpId);
                }
            }
            continue;
        }

        // update stored maps
        if (m_rntiQueueSize.empty())
        {
            NS_LOG_DEBUG("empty MAP");
            NS_ABORT_MSG_IF(!m_activeUesPerBwp.at(bwpId).empty(),
                            "No UE in map, but something in activeUes map");
            continue;
        }

        if (ueMap.find(rnti) != ueMap.end())
        {
            uint32_t totBuffer = ueMap.at(rnti)->GetTotalDlBuffer();
            if (totBuffer > 0)
            {
                m_rntiQueueSize.at(c1) = totBuffer;
                NS_LOG_DEBUG("Updating queue size for bwpId: " << bwpId << " RNTI: " << rnti
                                                               << " to " << m_rntiQueueSize.at(c1)
                                                               << " bytes");
            }
            else
            {
                NS_LOG_INFO(
                    "Removing UE because we served it. RLC queue size: " << m_rntiQueueSize.at(c1));
                m_rntiQueueSize.erase(c1);
                m_activeUesPerBwp.at(bwpId).erase(rnti);
                NS_LOG_DEBUG("Update ActiveBwps map for bwpId: "
                             << bwpId << " with: " << m_activeUesPerBwp.at(bwpId).size() << " UEs");

                if (m_activeUesPerBwp.at(bwpId).empty())
                {
                    NS_LOG_DEBUG("Remove BWP from Active BWPs because we served all its UEs");
                    m_activeUesPerBwp.erase(bwpId);
                }
            }
        }
        else
        {
            NS_ABORT_MSG("UE not in the map, but has an allocation");
        }
    }
}

uint16_t
NrFhControl::GetNumberActiveUes(uint16_t bwpId) const
{
    return m_activeUesPerBwp.at(bwpId).size();
}

uint16_t
NrFhControl::GetNumberActiveBwps() const
{
    // BWPs with active UEs with new data
    uint16_t numActiveBwps = m_activeUesPerBwp.size();
    for (auto& it : m_activeHarqUesPerBwp)
    {
        // If there is an active BWP with active HARQ UE(s)
        // and it is not included in the active BWPs list
        if (m_activeUesPerBwp.find(it.first) == m_activeUesPerBwp.end())
        {
            numActiveBwps++; // increment the number of active BWPs
        }
    }
    NS_LOG_DEBUG("Number of active BWPs calculated: " << numActiveBwps);
    return numActiveBwps;
}

bool
NrFhControl::DoGetDoesAllocationFit(uint16_t bwpId, uint32_t mcs, uint32_t nRegs, uint8_t dlRank)
{
    NS_LOG_INFO("NrFhControl::DoGetDoesAllocationFit for cell: " << m_physicalCellId << " bwpId: "
                                                                 << bwpId << " mcs: " << mcs
                                                                 << " nRegs: " << nRegs);
    uint16_t numOfActiveBwps =
        GetNumberActiveBwps(); // considers only active BWPs with data in queue

    NS_LOG_DEBUG("Number of active BWPs in DoesAllocFit: " << numOfActiveBwps);

    if (numOfActiveBwps == 0) // if we are at this point with numBWPs=0, it means there are
                              // BWPs with just remaining HARQ allocations
    {
        numOfActiveBwps++;
    }
    uint64_t thr = GetFhThr(
        bwpId,
        mcs,
        nRegs * static_cast<uint32_t>(m_fhSchedSapUser.at(bwpId)->GetNumRbPerRbgFromSched()),
        dlRank);

    if (m_allocThrPerBwp.find(bwpId) == m_allocThrPerBwp.end()) // bwpId not in the map
    {
        if (thr < (m_fhCapacity / static_cast<uint32_t>(numOfActiveBwps) * 1e6))
        {
            m_allocThrPerBwp.insert(std::make_pair(bwpId, thr));
            NS_LOG_DEBUG("BWP not in the map, Allocation can be included. BWP Thr: "
                         << m_allocThrPerBwp.at(bwpId));
            return true;
        }
        NS_LOG_DEBUG("BWP not in the map, Allocation cannot be included");
        return false;
    } // bwp in the map & we can store the allocation
    if ((m_allocThrPerBwp[bwpId] + thr) <
        (m_fhCapacity / static_cast<uint32_t>(numOfActiveBwps) * 1e6))
    {
        m_allocThrPerBwp[bwpId] += thr;
        NS_LOG_DEBUG(
            "BWP in the map, Allocation can be included. BWP Thr: " << m_allocThrPerBwp.at(bwpId));
        return true;
    }
    NS_LOG_INFO("BWP in the map, Allocation cannot be included");
    return false;
}

uint8_t
NrFhControl::DoGetMaxMcsAssignable(uint16_t bwpId, uint32_t reg, uint32_t rnti, uint8_t dlRank)
{
    uint16_t numOfActiveBwps =
        GetNumberActiveBwps(); // considers only active BWPs with data in queue
    NS_ASSERT_MSG(numOfActiveBwps > 0, "No Active BWPs, sth is wrong");
    NS_ASSERT_MSG(m_enableModComp == true,
                  "DoGetMaxMcsAssignable has no sense without modulation compression enabled");
    uint32_t availableCapacity = m_fhCapacity / static_cast<uint32_t>(numOfActiveBwps);

    uint16_t numActiveUes = GetNumberActiveUes(bwpId);
    NS_LOG_INFO("BwpId: " << bwpId << " Number of Active UEs: " << numActiveUes);
    uint16_t Kp = numActiveUes;

    Time slotLength = MicroSeconds(
        static_cast<uint16_t>(1000 / std::pow(2, m_numerologyPerBwp.at(bwpId)))); // slot length
    auto overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 /
        std::pow(2, m_numerologyPerBwp.at(bwpId))); // bits (10e6 (bps) x slot length (in s))

    if (availableCapacity * 1e6 * slotLength.GetSeconds() <=
        numActiveUes * (m_overheadDyn + overheadMac + (12 * 2 * 10)))
    {
        while (availableCapacity * 1e6 * slotLength.GetSeconds() <=
               Kp * (m_overheadDyn + overheadMac + (12 * 2 * 10)))
        {
            Kp--;
        }
    }
    NS_ABORT_MSG_IF(availableCapacity * 1e6 * slotLength.GetSeconds() <=
                        Kp * (m_overheadDyn + overheadMac + (12 * 2 * 10)),
                    "Not enough fronthaul capacity to send intra-PHY split overhead");

    auto num = static_cast<uint32_t>(availableCapacity * 1e6 * slotLength.GetSeconds() -
                                     Kp * (m_overheadDyn - overheadMac - (12 * 2 * 10)));
    if (Kp == 0)
    {
        return 0;
    }
    uint16_t modOrderMax =
        num / (12 * Kp * reg * dlRank) /
        static_cast<uint32_t>(
            m_fhSchedSapUser.at(bwpId)
                ->GetNumRbPerRbgFromSched());            // REGs, otherwise, should divide by nSymb
    uint8_t mcsMax = GetMaxMcs(m_mcsTable, modOrderMax); // MCS max

    NS_ABORT_MSG_IF(mcsMax == 0, "could not compute correctly the maxMCS");

    return mcsMax;
}

uint32_t
NrFhControl::DoGetMaxRegAssignable(uint16_t bwpId, uint32_t mcs, uint32_t rnti, uint8_t dlRank)
{
    uint32_t modulationOrder =
        m_mcsTable == 1 ? nrEesmT1.m_mcsMTable->at(mcs) : nrEesmT2.m_mcsMTable->at(mcs);

    uint16_t numOfActiveBwps =
        GetNumberActiveBwps(); // considers only active BWPs with data in queue
    NS_ASSERT_MSG(numOfActiveBwps > 0, "No Active BWPs, sth is wrong");
    uint32_t availableCapacity = m_fhCapacity / static_cast<uint32_t>(numOfActiveBwps);

    uint16_t numActiveUes = GetNumberActiveUes(bwpId);
    NS_LOG_INFO("BwpId: " << bwpId << " Number of Active UEs: " << numActiveUes);
    uint16_t Kp = numActiveUes;

    uint8_t overheadDyn =
        (m_enableModComp ? m_overheadDyn : 0); // overhead of dynamic adaptations due to
                                               // dynamic modulation compression.
                                               // 0 if modulation compression is disabled.

    Time slotLength = MicroSeconds(
        static_cast<uint16_t>(1000 / std::pow(2, m_numerologyPerBwp.at(bwpId)))); // slot length
    auto overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 /
        std::pow(2, m_numerologyPerBwp.at(bwpId))); // bits (10e6 (bps) x slot length (in s))

    if (availableCapacity * 1e6 * slotLength.GetSeconds() <=
        numActiveUes * (overheadDyn + overheadMac + (12 * 2 * 10)))
    {
        while (availableCapacity * 1e6 * slotLength.GetSeconds() <=
               Kp * (overheadDyn + overheadMac + (12 * 2 * 10)))
        {
            Kp--;
        }
    }
    NS_ABORT_MSG_IF(availableCapacity * 1e6 * slotLength.GetSeconds() <=
                        Kp * (overheadDyn + overheadMac + (12 * 2 * 10)),
                    "Not enough fronthaul capacity to send intra-PHY split overhead");

    auto num = static_cast<uint32_t>(availableCapacity * 1e6 * slotLength.GetSeconds() -
                                     Kp * (overheadDyn - overheadMac - (12 * 2 * 10)));
    if (Kp == 0)
    {
        return 0;
    }

    uint32_t W = (m_enableModComp ? modulationOrder : 32); // bitwidth (number of IQ bits)
    uint32_t nMax =
        num / (12 * Kp * W * dlRank) /
        static_cast<uint32_t>(
            m_fhSchedSapUser.at(bwpId)
                ->GetNumRbPerRbgFromSched()); // in REGs, otherwise, should divide by nSymb

    NS_LOG_DEBUG("Scheduler GetMaxRegAssignable " << nMax << " for UE " << rnti << " with mcs "
                                                  << mcs);

    return nMax;
}

void
NrFhControl::DoUpdateTracesBasedOnDroppedData(uint16_t bwpId,
                                              uint32_t mcs,
                                              uint32_t nRbgs,
                                              uint32_t nSymb,
                                              uint8_t dlRank)
{
    // in Dropping, the trace is computed from PHY layer
    uint32_t numRbs =
        nRbgs * static_cast<uint32_t>(m_fhSchedSapUser.at(bwpId)->GetNumRbPerRbgFromSched());

    // update FH trace and AI trace
    NS_LOG_DEBUG("Update Traces based on Dropped Data");
    // bwpId not in the map
    if (m_reqFhDlThrTracedValuePerBwp.find(bwpId) == m_reqFhDlThrTracedValuePerBwp.end())
    {
        NS_LOG_DEBUG("Create pair for" << " m_reqFhDlThrTracedValuePerBwp.at(" << bwpId
                                       << "): " << GetFhThr(bwpId, mcs, (numRbs * nSymb), dlRank));
    }
    m_reqFhDlThrTracedValuePerBwp[bwpId] += GetFhThr(bwpId, mcs, (numRbs * nSymb), dlRank);
    NS_LOG_DEBUG("Update m_reqFhDlThrTracedValuePerBwp.at("
                 << bwpId << "): " << m_reqFhDlThrTracedValuePerBwp.at(bwpId));

    if (m_rbsAirTracedValue.find(bwpId) == m_rbsAirTracedValue.end()) // bwpId not in the map
    {
        NS_LOG_DEBUG("Create pair for m_rbsAirTracedValue.at(" << bwpId << "): " << numRbs
                                                               << " RBs");
    }
    m_rbsAirTracedValue[bwpId] += numRbs;
    NS_LOG_DEBUG("Update m_rbsAirTracedValue.at(" << bwpId << "): " << m_rbsAirTracedValue.at(bwpId)
                                                  << " RBs");
}

void
NrFhControl::DoNotifyEndSlot(uint16_t bwpId, SfnSf currentSlot)
{
    // we only want to save it once (per slot) in the trace. Note that every cell that calls EndSlot
    if (currentSlot == m_waitingSlotPerBwp.at(bwpId))
    {
        NS_LOG_INFO(currentSlot);

        // store SfnSf and required FH thr (in DL)
        if (m_reqFhDlThrTracedValuePerBwp.find(bwpId) == m_reqFhDlThrTracedValuePerBwp.end())
        {
            m_reqFhDlThrTrace(currentSlot, m_physicalCellId, bwpId, 0);
            NS_LOG_DEBUG("Size 0, bwpId: " << bwpId << " FH DL Throughput 0");
        }
        else
        {
            NS_LOG_INFO("Req FH DL thr at end slot for m_reqFhDlThrTracedValuePerBwp.at("
                        << bwpId << "): " << m_reqFhDlThrTracedValuePerBwp.at(bwpId));
            m_reqFhDlThrTrace(currentSlot,
                              m_physicalCellId,
                              bwpId,
                              m_reqFhDlThrTracedValuePerBwp.at(
                                  bwpId)); // store SfnSf, bwpId and required FH thr (in DL)
        }

        uint32_t rbSum = 0;
        if (m_rbsAirTracedValue.empty())
        {
            m_rbsAirTrace(currentSlot,
                          m_physicalCellId,
                          bwpId,
                          rbSum); // store SfnSf, bwpId and 0 used RBs
            NS_LOG_DEBUG("Size 0, bwpId: " << bwpId
                                           << " Average RBs used at the end of slot: " << rbSum);
        }
        else
        {
            for (std::pair<uint16_t, uint32_t> element : m_rbsAirTracedValue)
            {
                rbSum += element.second;
            }
            rbSum = rbSum / m_rbsAirTracedValue.size();
            m_rbsAirTrace(currentSlot,
                          m_physicalCellId,
                          bwpId,
                          rbSum); // store SfnSf, bwpId and AVERAGE used RBs
            NS_LOG_DEBUG("Average RBs used at the end of slot: " << rbSum);
        }

        NS_LOG_DEBUG("Reset traces for next slot");
        m_reqFhDlThrTracedValuePerBwp.erase(bwpId);
        m_rbsAirTracedValue.erase(bwpId);
        m_allocThrPerCell = 0;
        m_allocThrPerBwp.erase(bwpId);
        m_waitingSlotPerBwp.at(bwpId).Add(1);
    }
}

uint64_t
NrFhControl::GetFhThr(uint16_t bwpId, uint32_t mcs, uint32_t nRegs, uint8_t dlRank) const
{
    uint64_t thr;
    uint16_t numerology = m_fhPhySapUser.at(bwpId)->GetNumerology();
    NS_ASSERT_MSG(numerology == m_numerologyPerBwp.at(bwpId),
                  " Numerology has not been configured properly for bwpId: " << bwpId);
    Time slotLength =
        MicroSeconds(static_cast<uint16_t>(1000 / std::pow(2, numerology))); // slot length
    auto overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 / std::pow(2, numerology)); // bits (10e6 (bps) x slot length (in s))

    uint32_t effectiveModulationOrder =
        m_enableModComp
            ? (m_mcsTable == 1 ? nrEesmT1.m_mcsMTable->at(mcs) : nrEesmT2.m_mcsMTable->at(mcs))
            : 32;

    uint8_t overheadDyn = (m_enableModComp ? m_overheadDyn : 0);
    thr = ((12 * effectiveModulationOrder * nRegs * dlRank) + overheadDyn + overheadMac +
           (12 * 2 * 10)) /
          slotLength.GetSeconds();
    // added 10 RBs of DCI overhead over 1 symbol, encoded with QPSK

    return thr;
}

uint8_t
NrFhControl::GetMaxMcs(uint8_t mcsTable, uint16_t modOrder) const
{
    // If the calculated modOrder is higher than 6 or 8, limit its value to the maximum allowed in
    // the MCS table
    while (true)
    {
        if (modOrder < 4)
        {
            modOrder = 2;
            break;
        }

        if (modOrder < 6)
        {
            modOrder = 4;
            break;
        }

        if (modOrder < 8)
        {
            modOrder = 6;
            break;
        }

        if (m_mcsTable == 1)
        {
            NS_ABORT_MSG("Illegal modOrder for MCS Table 1");
        }
        modOrder = 8;
        break;
    }

    const std::vector<uint8_t>* mcsMTable =
        (mcsTable == 1) ? nrEesmT1.m_mcsMTable : nrEesmT2.m_mcsMTable;
    // Find the last position where the modulation order appears in the MCS table. This position
    // corresponds to the highest MCS that can be associated with that modulation order.
    auto it = std::find(mcsMTable->rbegin(), mcsMTable->rend(), modOrder);
    uint8_t mcsMax = std::distance(mcsMTable->begin(), it.base()) - 1;

    return mcsMax;
}

} // namespace ns3
