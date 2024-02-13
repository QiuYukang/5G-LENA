/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-sl-ue-mac-scheduler.h"

#include "nr-sl-ue-mac.h"

#include <ns3/log.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrSlUeMacScheduler");
NS_OBJECT_ENSURE_REGISTERED(NrSlUeMacScheduler);

TypeId
NrSlUeMacScheduler::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::NrSlUeMacScheduler").SetParent<Object>().SetGroupName("nr");

    return tid;
}

NrSlUeMacScheduler::NrSlUeMacScheduler()
{
    NS_LOG_FUNCTION_NOARGS();
    m_nrSlUeMacSchedSapProvider = new NrSlUeMacGeneralSchedSapProvider(this);
    m_nrSlUeMacCschedSapProvider = new NrSlUeMacGeneralCschedSapProvider(this);
}

NrSlUeMacScheduler::~NrSlUeMacScheduler()
{
    NS_LOG_FUNCTION_NOARGS();
    delete m_nrSlUeMacSchedSapProvider;
    m_nrSlUeMacSchedSapProvider = nullptr;

    delete m_nrSlUeMacCschedSapProvider;
    m_nrSlUeMacCschedSapProvider = nullptr;
}

void
NrSlUeMacScheduler::DoDispose()
{
    m_nrSlUeMac = nullptr;
    Object::DoDispose();
}

void
NrSlUeMacScheduler::SetNrSlUeMac(Ptr<NrSlUeMac> nrSlUeMac)
{
    m_nrSlUeMac = nrSlUeMac;
}

Ptr<NrSlUeMac>
NrSlUeMacScheduler::GetNrSlUeMac() const
{
    return m_nrSlUeMac;
}

void
NrSlUeMacScheduler::SetNrSlUeMacSchedSapUser(NrSlUeMacSchedSapUser* sap)
{
    m_nrSlUeMacSchedSapUser = sap;
}

NrSlUeMacSchedSapProvider*
NrSlUeMacScheduler::GetNrSlUeMacSchedSapProvider()
{
    return m_nrSlUeMacSchedSapProvider;
}

void
NrSlUeMacScheduler::SetNrSlUeMacCschedSapUser(NrSlUeMacCschedSapUser* sap)
{
    m_nrSlUeMacCschedSapUser = sap;
}

NrSlUeMacCschedSapProvider*
NrSlUeMacScheduler::GetNrSlUeMacCschedSapProvider()
{
    return m_nrSlUeMacCschedSapProvider;
}

// CSCHED API primitives for NR Sidelink
NrSlUeMacGeneralCschedSapProvider::NrSlUeMacGeneralCschedSapProvider(NrSlUeMacScheduler* scheduler)
    : m_scheduler(scheduler)
{
}

void
NrSlUeMacGeneralCschedSapProvider::CschedUeNrSlLcConfigReq(
    const struct NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params)
{
    m_scheduler->DoCschedUeNrSlLcConfigReq(params);
}

// SCHED API primitives for NR Sidelink
NrSlUeMacGeneralSchedSapProvider::NrSlUeMacGeneralSchedSapProvider(NrSlUeMacScheduler* sched)
    : m_scheduler(sched)
{
}

void
NrSlUeMacGeneralSchedSapProvider::SchedUeNrSlRlcBufferReq(
    const struct NrSlReportBufferStatusParams& params)
{
    m_scheduler->DoSchedUeNrSlRlcBufferReq(params);
}

void
NrSlUeMacGeneralSchedSapProvider::SchedUeNrSlTriggerReq(uint32_t dstL2Id,
                                                        const std::list<NrSlSlotInfo>& params)
{
    m_scheduler->DoSchedUeNrSlTriggerReq(dstL2Id, params);
}

} // namespace ns3
