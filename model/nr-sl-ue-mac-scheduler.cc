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
    NS_LOG_FUNCTION(this);
}

NrSlUeMacScheduler::~NrSlUeMacScheduler()
{
    NS_LOG_FUNCTION(this);
}

void
NrSlUeMacScheduler::DoDispose()
{
    m_nrSlUeMac = nullptr;
    Object::DoDispose();
}

void
NrSlUeMacScheduler::SchedNrSlTriggerReq(uint32_t dstL2Id, const std::list<NrSlSlotInfo>& params)
{
    DoSchedNrSlTriggerReq(dstL2Id, params);
}

void
NrSlUeMacScheduler::SchedNrSlRlcBufferReq(
    const struct NrSlMacSapProvider::NrSlReportBufferStatusParameters& params)
{
    DoSchedNrSlRlcBufferReq(params);
}

void
NrSlUeMacScheduler::CschedNrSlLcConfigReq(
    const struct NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params)
{
    DoCschedNrSlLcConfigReq(params);
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

} // namespace ns3
