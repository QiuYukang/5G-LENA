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
    static TypeId tid = TypeId("ns3::NrSlUeMacScheduler")
                            .SetParent<Object>()
                            .SetGroupName("nr")
                            .AddAttribute("NrSlAmc",
                                          "The NR SL AMC of this scheduler",
                                          PointerValue(),
                                          MakePointerAccessor(&NrSlUeMacScheduler::m_nrSlAmc),
                                          MakePointerChecker<NrAmc>());
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
    m_nrSlAmc = nullptr;
    Object::DoDispose();
}

void
NrSlUeMacScheduler::SchedNrSlTriggerReq(const SfnSf& sfn, const std::deque<uint8_t>& ids)
{
    DoSchedNrSlTriggerReq(sfn, ids);
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
NrSlUeMacScheduler::InstallNrSlAmc(const Ptr<NrAmc>& nrSlAmc)
{
    NS_LOG_FUNCTION(this);
    m_nrSlAmc = nrSlAmc;
    // In NR it does not have any impact
    m_nrSlAmc->SetUlMode();
}

Ptr<const NrAmc>
NrSlUeMacScheduler::GetNrSlAmc() const
{
    NS_LOG_FUNCTION(this);
    return m_nrSlAmc;
}

} // namespace ns3
