// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-fh-control.h"

#include <ns3/core-module.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrFhControl");
NS_OBJECT_ENSURE_REGISTERED(NrFhControl);

NrFhControl::NrFhControl()
    : m_fhPhySapUser(0),
      m_fhSchedSapUser(0)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_UNCOND("Initialze Fh Control");
    m_fhPhySapProvider = new MemberNrFhPhySapProvider<NrFhControl>(this);
    m_fhSchedSapProvider = new MemberNrFhSchedSapProvider<NrFhControl>(this);
}

NrFhControl::~NrFhControl()
{
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

        ;
    return tid;
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
NrFhControl::DoGetDoesAllocationFit()
{
    NS_LOG_UNCOND("Call the Fh Control DoGetDoesAllocationFit");
}

} // namespace ns3
