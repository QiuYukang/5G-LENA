/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-fh-control.h"

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
    static TypeId tid = TypeId("ns3::NrFhControl")
                            .SetParent<Object>()
                            .AddConstructor<NrFhControl>()
                            .SetGroupName("Nr");
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
NrFhControl::SetLimitModel()
{
    NS_LOG_UNCOND("Set the Fh Control");
}

void
NrFhControl::DoGetDoesAllocationFit()
{
    NS_LOG_UNCOND("Call the Fh Control DoGetDoesAllocationFit");
}

} // namespace ns3
