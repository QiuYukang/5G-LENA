// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-fh-control.h"

#include <ns3/core-module.h>

#include <unordered_set>

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
                          MakeUintegerChecker<uint16_t>(0, 50000))
            .AddAttribute("OverheadDyn",
                          "The overhead for dynamic adaptation (in bits)",
                          UintegerValue(32),
                          MakeUintegerAccessor(&NrFhControl::SetOverheadDyn),
                          MakeUintegerChecker<uint8_t>(0, 100))
            .AddAttribute("ErrorModelType",
                          "The ErrorModelType based on which the MCS Table (1 or 2) will be set."
                          "Select among: ns3::NrEesmIrT1 and ns3::NrEesmCcT1 for MCS Table 1"
                          "and among ns3::NrEesmIrT2 and ns3::NrEesmCcT2 for MCS Table 2.",
                          StringValue("ns3::NrEesmIrT1"),
                          MakeStringAccessor(&NrFhControl::SetErrorModelType),
                          MakeStringChecker());
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
}

void
NrFhControl::SetNrFhPhySapUser(uint16_t bwpId, NrFhPhySapUser* s)
{
    NS_LOG_FUNCTION(this << s);
    std::map<uint16_t, NrFhPhySapUser*>::iterator it = m_fhPhySapUser.find(bwpId);

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

    std::map<uint16_t, NrFhSchedSapUser*>::iterator it = m_fhSchedSapUser.find(bwpId);

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
    NS_LOG_DEBUG("Set the Fh Control Limit Model to: " << model);
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
    NS_LOG_FUNCTION(this);
    return m_fhControlMethod;
}

void
NrFhControl::SetCellFhCapacity(uint16_t capacity)
{
    NS_LOG_FUNCTION(this);
    m_fhCapacity = capacity;
}

void
NrFhControl::ConfigureFhCapacityPerBwp(uint32_t numberOfActiveBwps)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(numberOfActiveBwps > 0, "Active BWPs cannot be 0");
    m_fhCapacity = static_cast<uint16_t>(m_fhCapacity / numberOfActiveBwps);
    NS_LOG_DEBUG("Active BWPs set by nrHelper: " << numberOfActiveBwps
                                                 << " FH capacity per BWP: " << m_fhCapacity);
}

void
NrFhControl::SetOverheadDyn(uint8_t overhead)
{
    NS_LOG_FUNCTION(this);
    m_overheadDyn = overhead;
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
        NS_ABORT_MSG("Wrong error model type. Please select among: ns3::NrEesmIrT1,"
                     "ns3::NrEesmCcT1 for MCS Table 1 and ns3::NrEesmIrT2 and ns3::NrEesmCcT2"
                     "for MCS Table 2");
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
NrFhControl::SetNumerology(uint16_t bwpId, uint16_t num)
{
    if (m_numerologyPerBwp.find(bwpId) == m_numerologyPerBwp.end()) // bwpId not in the map
    {
        m_numerologyPerBwp.insert(std::make_pair(bwpId, num));
        SfnSf waitingSlot = {0, 0, 0, static_cast<uint8_t>(num)};
        m_waitingSlotPerBwp.insert(std::make_pair(bwpId, waitingSlot));
    }
    else
    {
        NS_ABORT_MSG("Configure NrFhControl should be called only once");
    }
}

void
NrFhControl::DoSetActiveUe(uint16_t bwpId, uint16_t rnti, uint32_t bytes)
{
    uint32_t c1 = Cantor(bwpId, rnti);
    if (m_activeUesPerBwp.find(rnti) == m_activeUesPerBwp.end()) // UE not in the map
    {
        NS_LOG_DEBUG("Creating pair for rnti: " << rnti << " and bwpId: " << bwpId);
        m_activeUesPerBwp.insert(std::make_pair(rnti, bwpId));
    }

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
NrFhControl::DoUpdateActiveUesMap(uint16_t bwpId, const std::deque<VarTtiAllocInfo>& allocation)
{
    NS_LOG_INFO("Cell: " << m_physicalCellId << " We got called for reset for bwpId: " << bwpId);

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

        NS_LOG_DEBUG("Get num of RBs per RBG from sched: "
                     << m_fhSchedSapUser.at(bwpId)->GetNumRbPerRbgFromSched()
                     << " numRbs = " << numRbs << " MCS Table: " << +m_mcsTable << " UE: " << rnti);

        uint16_t numerology = m_fhPhySapUser.at(bwpId)->GetNumerology();
        NS_ASSERT_MSG(numerology == m_numerologyPerBwp.at(bwpId),
                      " Numerology has not been configured properly for bwpId: " << +bwpId);

        // update FH trace and AI trace
        m_reqFhDlThrTracedValue += GetFhThr(bwpId,
                                            static_cast<uint32_t>(alloc.m_dci->m_mcs),
                                            static_cast<uint32_t>(alloc.m_dci->m_numSym) * numRbs);
        NS_LOG_DEBUG("bwpId: " << bwpId << " rnti: " << rnti
                               << " m_reqFhDlThrTracedValue: " << m_reqFhDlThrTracedValue);

        if (m_rbsAirTracedValue.find(bwpId) == m_rbsAirTracedValue.end()) // bwp not in the map
        {
            NS_LOG_DEBUG("Make new m_rbsAirTracedValue pair for bwpId: "
                         << bwpId << " (rnti: " << rnti << ")"
                         << " and numRbs: " << numRbs);
            m_rbsAirTracedValue.insert(std::make_pair(bwpId, numRbs));
        }
        else // bwpId already in the map: increase traced value
        {
            m_rbsAirTracedValue[bwpId] += numRbs;
            NS_LOG_DEBUG("Update m_rbsAirTracedValue for bwpId: "
                         << bwpId << " (rnti: " << rnti << ")"
                         << " increase traced value to: " << m_rbsAirTracedValue[bwpId] << " RBs");
        }
        if (alloc.m_dci->m_ndi == 0)  // retx
        {
            continue; // retx are not considered in the optimization (focuses on DL new data), but
                      // stored for the trace
        }

        // update stored maps
        if (m_rntiQueueSize.size() == 0)
        {
            NS_LOG_DEBUG("empty MAP");
            NS_ABORT_MSG_IF(m_activeUesPerBwp.size() > 0,
                            "No UE in map, but something in activeUes map");
            continue;
        }

        NS_LOG_DEBUG("Looking for key " << c1 << " map size " << m_rntiQueueSize.size());

        if (m_rntiQueueSize.at(c1) > (alloc.m_dci->m_tbSize - 3)) // 3 bytes MAC subPDU header
        {
            m_rntiQueueSize.at(c1) = m_rntiQueueSize.at(c1) - (alloc.m_dci->m_tbSize - 3);
            NS_LOG_DEBUG("Updating queue size for cell: " << m_physicalCellId << " bwpId: " << bwpId
                                                          << " RNTI: " << rnti << " to "
                                                          << m_rntiQueueSize.at(c1) << " bytes");
        }
        else
        {
            NS_LOG_DEBUG("Removing UE because we served it. RLC queue size: "
                         << m_rntiQueueSize.at(c1)
                         << " and allocation of: " << (alloc.m_dci->m_tbSize - 3));
            m_rntiQueueSize.erase(c1);
            m_activeUesPerBwp.erase(rnti);
        }
    }
}

uint16_t
NrFhControl::GetNumberActiveUes(uint16_t bwpId) const
{
    uint16_t numActiveUes = 0;
    for (auto& it : m_activeUesPerBwp)
    {
        if (it.second == bwpId)
        {
            numActiveUes++;
        }
    }
    NS_LOG_DEBUG("active Ues Per bwp: " << numActiveUes);
    return numActiveUes;
}

bool
NrFhControl::DoGetDoesAllocationFit(uint16_t bwpId, uint32_t mcs, uint32_t nRegs)
{
    return 1;
    /*NS_LOG_INFO("NrFhControl::DoGetDoesAllocationFit for cell: " << m_physicalCellId << " bwpId: "
                                                                 << bwpId << " mcs: " << mcs
                                                                 << " nRegs: " << nRegs);
    uint16_t numBwps =
        GetNumberActiveBwps(); // considers only active cells as BWPs with new data in queue
    if (numBwps == 0) // if we are at this point with numBWPs=0, it means there are BWPs with just
                      // remaining HARQ allocations
    {
        numBwps++;
    }
    uint64_t thr =
        GetFhThr(mcs, nRegs * static_cast<uint32_t>(m_fhSchedSapUser->GetNumRbPerRbgFromSched()));

    NS_LOG_INFO("Throughput: " << thr);
    if (m_allocCellThrPerBwp.find(bwpId) == m_allocCellThrPerBwp.end()) // bwpId not in the map
    {
        NS_LOG_INFO("BWP not in the map ");
        if (thr < (m_fhCapacity * 1e6 / numBwps) &&
            (m_allocFhThroughput + thr < m_fhCapacity * 1e6)) // divide with active BWPs?
        {
            m_allocCellThrPerBwp.insert(std::make_pair(bwpId, thr));
            m_allocFhThroughput += thr;
            NS_LOG_INFO("Return True");
            return 1;
        }
        else
        {
            NS_LOG_INFO("Return False");
            return 0;
        }
    }
    else if (((m_allocCellThrPerBwp[bwpId] + thr) < (m_fhCapacity * 1e6 / numBwps)) &&
             (m_allocFhThroughput + thr <
              m_fhCapacity * 1e6)) // cell in the map & we can store the allocation
    {
        NS_LOG_INFO("BWP exists in the map ");
        m_allocCellThrPerBwp[bwpId] += thr;
        m_allocFhThroughput += thr; // double check
        NS_LOG_INFO("Return True");
        return 1;
    }
    else // we cannot include the allocation
    {
        NS_LOG_INFO("Allocation cannot be included");
        return 0;
    }*/
}

uint8_t
NrFhControl::DoGetMaxMcsAssignable(uint16_t bwpId, uint32_t reg, uint32_t rnti)
{
    uint16_t numActiveUes = GetNumberActiveUes(bwpId);
    NS_LOG_INFO("BwpId: " << bwpId << " Number of Active UEs: " << numActiveUes);
    uint16_t Kp = numActiveUes;

    Time slotLength = MicroSeconds(
        static_cast<uint16_t>(1000 / std::pow(2, m_numerologyPerBwp.at(bwpId)))); // slot length
    uint32_t overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 /
        std::pow(2, m_numerologyPerBwp.at(bwpId))); // bits (10e6 (bps) x slot length (in s))

    if (m_fhCapacity * 1e6 * slotLength.GetSeconds() <=
        numActiveUes * m_overheadDyn + numActiveUes * overheadMac + numActiveUes * 12 * 2 * 10)
    {
        while (m_fhCapacity * 1e6 * slotLength.GetSeconds() <=
               Kp * m_overheadDyn + Kp * overheadMac + Kp * 12 * 2 * 10)
        {
            NS_LOG_DEBUG("Inside if while loop - reduce Kp");
            Kp--;
        }
    }

    NS_LOG_DEBUG("Kp: " << Kp);
    NS_ABORT_MSG_IF(m_fhCapacity * 1e6 * slotLength.GetSeconds() <=
                        Kp * m_overheadDyn + Kp * overheadMac + Kp * 12 * 2 * 10,
                    "Not enough fronthaul capacity to send intra-PHY split overhead");

    uint32_t num = static_cast<uint32_t>(m_fhCapacity * 1e6 * slotLength.GetSeconds() -
                                         Kp * m_overheadDyn - Kp * overheadMac - Kp * 12 * 2 * 10);
    if (Kp == 0)
    {
        return 0;
    }
    uint16_t modOrderMax =
        num / (12 * Kp * reg) /
        static_cast<uint32_t>(
            m_fhSchedSapUser.at(bwpId)
                ->GetNumRbPerRbgFromSched());            // REGs, otherwise, should divide by nSymb
    uint8_t mcsMax = GetMaxMcs(m_mcsTable, modOrderMax); // MCS max

    NS_ABORT_MSG_IF(mcsMax == 0, "could not compute correctly the maxMCS");

    return mcsMax;
}

uint32_t
NrFhControl::DoGetMaxRegAssignable(uint16_t bwpId, uint32_t mcs, uint32_t rnti)
{
    uint32_t modulationOrder =
        m_mcsTable == 1 ? GetModulationOrderTable1(mcs) : GetModulationOrderTable2(mcs);

    uint16_t numActiveUes = GetNumberActiveUes(bwpId);
    NS_LOG_INFO("BwpId: " << bwpId << " Number of Active UEs: " << numActiveUes);
    uint16_t Kp = numActiveUes;

    Time slotLength = MicroSeconds(
        static_cast<uint16_t>(1000 / std::pow(2, m_numerologyPerBwp.at(bwpId)))); // slot length
    uint32_t overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 /
        std::pow(2, m_numerologyPerBwp.at(bwpId))); // bits (10e6 (bps) x slot length (in s))

    if (m_fhCapacity * 1e6 * slotLength.GetSeconds() <=
        numActiveUes * m_overheadDyn + numActiveUes * overheadMac + numActiveUes * 12 * 2 * 10)
    {
        while (m_fhCapacity * 1e6 * slotLength.GetSeconds() <=
               Kp * m_overheadDyn + Kp * overheadMac + Kp * 12 * 2 * 10)
        {
            Kp--;
        }
    }

    NS_ABORT_MSG_IF(m_fhCapacity * 1e6 * slotLength.GetSeconds() <=
                        Kp * m_overheadDyn + Kp * overheadMac + Kp * 12 * 2 * 10,
                    "Not enough fronthaul capacity to send intra-PHY split overhead");

    uint32_t num = static_cast<uint32_t>(m_fhCapacity * 1e6 * slotLength.GetSeconds() -
                                         Kp * m_overheadDyn - Kp * overheadMac - Kp * 12 * 2 * 10);
    uint32_t nMax =
        num / (12 * Kp * modulationOrder) /
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
                                              uint32_t nSymb)
{
    // in Dropping, the trace is computed from PHY layer
    uint32_t numRbs =
        nRbgs * static_cast<uint32_t>(m_fhSchedSapUser.at(bwpId)->GetNumRbPerRbgFromSched());

    // update FH trace and AI trace
    m_reqFhDlThrTracedValue += GetFhThr(bwpId, mcs, numRbs * nSymb);
    NS_LOG_DEBUG("Update Traces based on Dropped Data - m_reqFhDlThrTracedValue: "
                 << m_reqFhDlThrTracedValue);
    if (m_rbsAirTracedValue.find(bwpId) == m_rbsAirTracedValue.end()) // bwpId not in the map
    {
        m_rbsAirTracedValue.insert(std::make_pair(bwpId, numRbs));
        NS_LOG_DEBUG("BwpId not in the map m_reqFhDlThrTracedValue - Create pair: "
                     << bwpId << " numRbs: " << numRbs);
    }
    else // bwpId already in the map: increase traced value
    {
        m_rbsAirTracedValue[bwpId] += numRbs;
        NS_LOG_DEBUG("Update BwpId: " << bwpId << " new numRbs (m_rbsAirTracedValue): "
                                      << m_rbsAirTracedValue[bwpId]);
    }
}

void
NrFhControl::DoNotifyEndSlot(uint16_t bwpId, SfnSf currentSlot)
{
    // we only want to save it once (per slot) in the trace. note that every cell that calls EndSlot
    if (currentSlot == m_waitingSlotPerBwp.at(bwpId))
    {
        NS_LOG_INFO(currentSlot);
        m_reqFhDlThrTrace(currentSlot,
                          m_reqFhDlThrTracedValue); // store SfnSf and required FH thr (in DL)
        NS_LOG_INFO("Req FH DL thr at end slot for bwpId:"
                    << bwpId << " (m_reqFhDlThrTracedValue): " << m_reqFhDlThrTracedValue);
        uint32_t rbSum = 0;
        if (m_rbsAirTracedValue.size() == 0)
        {
            m_rbsAirTrace(currentSlot, rbSum); // store SfnSf and 0 used RBs
            NS_LOG_DEBUG("Size 0, Average RBs used at the end of slot: " << rbSum);
        }
        else
        {
            for (std::pair<uint16_t, uint32_t> element : m_rbsAirTracedValue)
            {
                rbSum += element.second;
            }
            rbSum = rbSum / m_rbsAirTracedValue.size();
            m_rbsAirTrace(currentSlot, rbSum); // store SfnSf and AVERAGE used RBs
            NS_LOG_DEBUG("Average RBs used at the end of slot: " << rbSum);
        }
        NS_LOG_DEBUG("Reset traces");
        m_reqFhDlThrTracedValue = 0;      // reset it, for next slot
        m_rbsAirTracedValue.erase(bwpId); // reset it, for next slot
        m_allocFhThroughput = 0;          // reset it, for next slot
        m_allocCellThrPerBwp.clear();     // reset it, for next slot
        m_waitingSlotPerBwp.at(bwpId).Add(1);
    }
}

uint64_t
NrFhControl::GetFhThr(uint16_t bwpId, uint32_t mcs, uint32_t Nres) const
{
    uint64_t thr;
    uint32_t modulationOrder =
        m_mcsTable == 1 ? GetModulationOrderTable1(mcs) : GetModulationOrderTable2(mcs);

    uint16_t numerology = m_fhPhySapUser.at(bwpId)->GetNumerology();
    Time slotLength =
        MicroSeconds(static_cast<uint16_t>(1000 / std::pow(2, numerology))); // slot length
    uint32_t overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 / std::pow(2, numerology)); // bits (10e6 (bps) x slot length (in s))

    thr = (12 * modulationOrder * Nres + m_overheadDyn + overheadMac + 12 * 2 * 10) /
          slotLength.GetSeconds(); // added 10 RBs of DCI overhead over 1 symbol, encoded with QPSK

    return thr;
}

uint8_t
NrFhControl::GetMaxMcs(uint8_t mcsTable, uint16_t modOrder)
{
    uint8_t mcsMax = 0;
    if (mcsTable == 1)
    {
        if (modOrder < 4)
        {
            mcsMax = GetMcsTable1(0);
        }
        else if (modOrder >= 4 && modOrder < 6)
        {
            mcsMax = GetMcsTable1(1);
        }
        else if (modOrder >= 6)
        {
            mcsMax = GetMcsTable1(2);
        }
    }
    else if (m_mcsTable == 2)
    {
        if (modOrder < 4)
        {
            mcsMax = GetMcsTable2(0);
        }
        else if (modOrder >= 4 && modOrder < 6)
        {
            mcsMax = GetMcsTable2(1);
        }
        else if (modOrder >= 6 && modOrder < 8)
        {
            mcsMax = GetMcsTable2(2);
        }
        else if (modOrder >= 8)
        {
            mcsMax = GetMcsTable2(3);
        }
    }
    return mcsMax;
}

uint32_t
NrFhControl::GetModulationOrderTable1(const uint32_t mcs) const
{
    std::vector<uint8_t> McsMTable1 = {// QPSK (modulationOrder = 2)
                                       2,
                                       2,
                                       2,
                                       2,
                                       2,
                                       2,
                                       2,
                                       2,
                                       2,
                                       2,
                                       // 16QAM (modulationOrder = 4)
                                       4,
                                       4,
                                       4,
                                       4,
                                       4,
                                       4,
                                       4,
                                       // 64QAM (modulationOrder = 6)
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6};
    return McsMTable1.at(mcs);
}

uint32_t
NrFhControl::GetModulationOrderTable2(const uint32_t mcs) const
{
    NS_ASSERT_MSG(mcs <= 27, "MCS must be up to 27");
    std::vector<uint8_t> McsMTable2 = {// QPSK (modulationOrder = 2)
                                       2,
                                       2,
                                       2,
                                       2,
                                       2,
                                       // 16QAM (modulationOrder = 4)
                                       4,
                                       4,
                                       4,
                                       4,
                                       4,
                                       4,
                                       // 64QAM (modulationOrder = 6)
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       6,
                                       // 256QAM (modulationOrder = 8)
                                       8,
                                       8,
                                       8,
                                       8,
                                       8,
                                       8,
                                       8,
                                       8};
    return McsMTable2.at(mcs);
}

uint8_t
NrFhControl::GetMcsTable1(const uint8_t modOrd) const
{
    std::vector<uint8_t> McsTable1 = {// QPSK (modulationOrder = 2)
                                      9,
                                      // 16QAM (modulationOrder = 4)
                                      16,
                                      // 64QAM (modulationOrder = 6)
                                      28};
    return McsTable1.at(modOrd);
}

uint8_t
NrFhControl::GetMcsTable2(const uint8_t modOrd) const
{
    std::vector<uint8_t> McsTable2 = {// QPSK (modulationOrder = 2)
                                      4,
                                      // 16QAM (modulationOrder = 4)
                                      10,
                                      // 64QAM (modulationOrder = 6)
                                      19,
                                      // 256QAM (modulationOrder = 8)
                                      27};
    return McsTable2.at(modOrd);
}

} // namespace ns3
