/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only and NIST-Software

#include "nr-sl-ue-mac-scheduler.h"

#include "nr-sl-ue-mac.h"

#include <ns3/log.h>
#include <ns3/pointer.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrSlUeMacScheduler");
NS_OBJECT_ENSURE_REGISTERED(NrSlUeMacScheduler);

TypeId
NrSlUeMacScheduler::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrSlUeMacScheduler")
            .SetParent<Object>()
            .SetGroupName("nr")
            .AddAttribute("NrSlAmc",
                          "The AMC used by this scheduler",
                          PointerValue(),
                          MakePointerAccessor(&NrSlUeMacScheduler::m_amc),
                          MakePointerChecker<NrAmc>())
            .AddTraceSource("GrantCreated",
                            "Trace the creation of a grant",
                            MakeTraceSourceAccessor(&NrSlUeMacScheduler::m_grantCreatedTrace),
                            "ns3::NrSlUeMacScheduler::GrantCreatedCallback")
            .AddTraceSource("GrantPublished",
                            "Trace the publishing of a grant to the NrSlUeMac",
                            MakeTraceSourceAccessor(&NrSlUeMacScheduler::m_grantPublishedTrace),
                            "ns3::NrSlUeMacScheduler::GrantPublishedCallback");
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
    m_ueMac = nullptr;
    m_amc = nullptr;
    Object::DoDispose();
}

void
NrSlUeMacScheduler::SchedNrSlTriggerReq(const SfnSf& sfn)
{
    DoSchedNrSlTriggerReq(sfn);
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
NrSlUeMacScheduler::RemoveNrSlLcConfigReq(uint8_t lcid, uint32_t dstL2Id)
{
    DoRemoveNrSlLcConfigReq(lcid, dstL2Id);
}

void
NrSlUeMacScheduler::NotifyNrSlRlcPduDequeue(uint32_t dstL2Id, uint8_t lcId, uint32_t size)
{
    DoNotifyNrSlRlcPduDequeue(dstL2Id, lcId, size);
}

void
NrSlUeMacScheduler::SetNrSlUeMac(Ptr<NrSlUeMac> ueMac)
{
    m_ueMac = ueMac;
}

Ptr<NrSlUeMac>
NrSlUeMacScheduler::GetMac() const
{
    return m_ueMac;
}

void
NrSlUeMacScheduler::InstallAmc(const Ptr<NrAmc>& amc)
{
    NS_LOG_FUNCTION(this);
    m_amc = amc;
    // In NR it does not have any impact
    m_amc->SetUlMode();
}

Ptr<const NrAmc>
NrSlUeMacScheduler::GetAmc() const
{
    NS_LOG_FUNCTION(this);
    return m_amc;
}

void
NrSlUeMacScheduler::NotifyGrantCreated(const struct GrantInfo& grant) const
{
    m_grantCreatedTrace(grant, m_ueMac->GetPsfchPeriod());
}

void
NrSlUeMacScheduler::NotifyGrantPublished(const struct NrSlUeMac::NrSlGrant& grant) const
{
    m_grantPublishedTrace(grant, m_ueMac->GetPsfchPeriod());
}

} // namespace ns3
