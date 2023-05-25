// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-fh-control.h"

#include <ns3/core-module.h>

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
                "allocated during the scheduling process) and asks CI for the max MCS. It"
                "assigns the min among the allocated one and the max MCS."
                "d) Optimize RBs. When tdma/ofdma are allocating the RBs/symbols to a UE,"
                "it calls the CI to provide the max RBs that can be assigned.",
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
                          "The available fronthaul capacity (in MHz)",
                          UintegerValue(1000),
                          MakeUintegerAccessor(&NrFhControl::SetFhCapacity),
                          MakeUintegerChecker<uint16_t>(0, 50000))
            .AddAttribute("OverheadDyn",
                          "The overhead for dynamic adaptation (in bits)",
                          UintegerValue(32),
                          MakeUintegerAccessor(&NrFhControl::SetOverheadDyn),
                          MakeUintegerChecker<uint8_t>(0, 100))
            .AddAttribute("ErrorModelType",
                          "The ErrorModelType based on which the MCS Table (1 or 2) will be set."
                          "Select among: ns3::NrEesmIrT1 and ns3::NrEesmCcT1 for MCS Table 1 "
                          "and among ns3::NrEesmIrT2 and ns3::NrEesmCcT2 for MCS Table 2.",
                          StringValue("ns3::NrEesmIrT1"),
                          MakeStringAccessor(&NrFhControl::SetErrorModelType),
                          MakeStringChecker());
    return tid;
}

NrFhControl::NrFhControl()
    : m_physicalCellId(0),
      m_fhPhySapUser(0),
      m_fhSchedSapUser(0)
{
    NS_LOG_FUNCTION(this);
    m_fhPhySapProvider = new MemberNrFhPhySapProvider<NrFhControl>(this);
    m_fhSchedSapProvider = new MemberNrFhSchedSapProvider<NrFhControl>(this);
}

NrFhControl::~NrFhControl()
{
}

void
NrFhControl::SetNrFhPhySapUser(NrFhPhySapUser* s)
{
    NS_LOG_FUNCTION(this << s);
    m_fhPhySapUser = s;
}

NrFhPhySapProvider*
NrFhControl::GetNrFhPhySapProvider()
{
    NS_LOG_FUNCTION(this);

    return m_fhPhySapProvider;
}

void
NrFhControl::SetNrFhSchedSapUser(NrFhSchedSapUser* s)
{
    NS_LOG_FUNCTION(this << s);
    m_fhSchedSapUser = s;
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
NrFhControl::SetFhCapacity(uint16_t capacity)
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
NrFhControl::DoSetActiveUe(uint16_t bwpId, uint16_t rnti, uint32_t bytes)
{
    uint32_t c1 = Cantor(bwpId, rnti);
    if (m_activeUes.find(bwpId) == m_activeUes.end()) // bwpId not in the map
    {
        m_activeUes.insert(std::make_pair(bwpId, rnti));
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
    NS_LOG_INFO("Cell: " << m_physicalCellId << " We got called for reset for " << bwpId);

    for (const auto& alloc : allocation)
    {
        if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL ||
            alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
            continue;
        }

        uint16_t rnti = alloc.m_dci->m_rnti;
        uint32_t c1 = Cantor(bwpId, rnti);
        uint32_t numRbs =
            static_cast<uint32_t>(
                std::count(alloc.m_dci->m_rbgBitmask.begin(), alloc.m_dci->m_rbgBitmask.end(), 1)) *
            static_cast<uint32_t>(m_fhSchedSapUser->GetNumRbPerRbgFromSched());

        NS_LOG_INFO("Get num of RBs per RBG from sched: "
                    << m_fhSchedSapUser->GetNumRbPerRbgFromSched() << " numRbs = " << numRbs
                    << " MCS Table: " << +m_mcsTable);

        uint16_t num = m_fhPhySapUser->GetNumerology();
        NS_LOG_INFO("Numerology: " << num); // TODO: Add traces and remove this

        // update stored maps
        if (m_rntiQueueSize.size() == 0)
        {
            NS_LOG_INFO("empty MAP");
            NS_ABORT_MSG_IF(m_activeUes.size() > 0, "No UE in map, but something in activeUes map");
            continue;
        }

        NS_LOG_DEBUG("Looking for key " << c1 << " map size " << m_rntiQueueSize.size());

        if (m_rntiQueueSize.at(c1) > (alloc.m_dci->m_tbSize - 3)) // 3 bytes MAC subPDU header
        {
            m_rntiQueueSize.at(c1) = m_rntiQueueSize.at(c1) - (alloc.m_dci->m_tbSize - 3);
            NS_LOG_DEBUG("Updating queue size for cell: " << m_physicalCellId << " bwpId: " << bwpId
                                                          << " RNTI: " << rnti << " to "
                                                          << m_rntiQueueSize.at(c1));
        }
        else
        {
            NS_LOG_DEBUG("Removing UE because we served it. RLC queue size: "
                         << m_rntiQueueSize.at(c1)
                         << " and allocation of: " << (alloc.m_dci->m_tbSize - 3));
            m_rntiQueueSize.erase(c1);
            m_activeUes.erase(bwpId);
        }
    }
}

void
NrFhControl::DoGetDoesAllocationFit()
{
    // NS_LOG_UNCOND("NrFhControl::DoGetDoesAllocationFit for cell: " << m_physicalCellId);
}

uint8_t
NrFhControl::DoGetMaxMcsAssignable(uint16_t bwpId [[maybe_unused]], uint32_t reg, uint32_t rnti)
{
    uint16_t numActiveUes = static_cast<uint16_t>(m_rntiQueueSize.size()); // number of active UEs
    // This map is for all the BWPs. If we get all the active UEs of all the BWPs
    // it is not correct, no? Maybe we should keep track of the active UEs per BWP
    // in different map or vector.

    uint16_t Kp = numActiveUes;

    uint16_t numerology = m_fhPhySapUser->GetNumerology();
    Time slotLength =
        MicroSeconds(static_cast<uint16_t>(1000 / std::pow(2, numerology))); // slot length
    uint32_t overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 / std::pow(2, numerology)); // bits (10e6 (bps) x slot length (in s))

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

    uint16_t modOrderMax =
        num / (12 * Kp * reg) / m_numRbPerRbg; // REGs, otherwise, should divide by nSymb

    uint8_t mcsMax = GetMaxMcs(m_mcsTable, modOrderMax); // MCS max

    NS_ABORT_MSG_IF(mcsMax == 0, "could not compute correctly the maxMCS");

    return mcsMax;
}

uint32_t
NrFhControl::DoGetMaxRegAssignable(uint16_t bwpId [[maybe_unused]], uint32_t mcs, uint32_t rnti)
{
    uint32_t modulationOrder = m_mcsTable == 1 ? GetModulationOrderTable1(mcs) : GetModulationOrderTable2(mcs);

    uint16_t numActiveUes = static_cast<uint16_t>(m_rntiQueueSize.size()); // number of active UEs
    // This map is for all the BWPs. If we get all the active UEs of all the BWPs
    // it is not correct, no? Maybe we should keep track of the active UEs per BWP
    // in different map or vector.

    uint16_t Kp = numActiveUes;

    uint16_t numerology = m_fhPhySapUser->GetNumerology();
    Time slotLength =
        MicroSeconds(static_cast<uint16_t>(1000 / std::pow(2, numerology))); // slot length
    uint32_t overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 / std::pow(2, numerology)); // bits (10e6 (bps) x slot length (in s))

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
    uint32_t nMax = num / (12 * Kp * modulationOrder) /
                    m_numRbPerRbg; // in REGs, otherwise, should divide by nSymb

    NS_LOG_DEBUG("Scheduler GetMaxRegAssignable " << nMax << " for UE " << rnti << " with mcs "
                                                  << mcs);

    return nMax;
}

uint64_t
NrFhControl::GetFhThr (uint32_t mcs, uint32_t Nres) const
{
    uint64_t thr;
    uint32_t modulationOrder = m_mcsTable == 1 ? GetModulationOrderTable1(mcs) : GetModulationOrderTable2(mcs);

    uint16_t numerology = m_fhPhySapUser->GetNumerology();
    Time slotLength =
        MicroSeconds(static_cast<uint16_t>(1000 / std::pow(2, numerology))); // slot length
    uint32_t overheadMac = static_cast<uint32_t>(
        10e6 * 1e-3 / std::pow(2, numerology)); // bits (10e6 (bps) x slot length (in s))

    thr = (12 * modulationOrder * Nres + m_overheadDyn + overheadMac + 12 * 2 * 10) / slotLength.GetSeconds(); // added 10 RBs of DCI overhead over 1 symbol, encoded with QPSK

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
