/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2017 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "bandwidth-part-gnb.h"

#include "nr-gnb-mac.h"
#include "nr-gnb-phy.h"

#include <ns3/abort.h>
#include <ns3/boolean.h>
#include <ns3/ff-mac-scheduler.h>
#include <ns3/log.h>
#include <ns3/pointer.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("BandwidthPartGnb");
NS_OBJECT_ENSURE_REGISTERED(BandwidthPartGnb);

TypeId
BandwidthPartGnb::GetTypeId()
{
    static TypeId tid = TypeId("ns3::BandwidthPartGnb")
                            .SetParent<ComponentCarrierBaseStation>()
                            .AddConstructor<BandwidthPartGnb>()
                            .AddAttribute("NrGnbPhy",
                                          "The PHY associated to this EnbNetDevice",
                                          PointerValue(),
                                          MakePointerAccessor(&BandwidthPartGnb::m_phy),
                                          MakePointerChecker<NrGnbPhy>())
                            .AddAttribute("NrGnbMac",
                                          "The MAC associated to this EnbNetDevice",
                                          PointerValue(),
                                          MakePointerAccessor(&BandwidthPartGnb::m_mac),
                                          MakePointerChecker<NrGnbMac>())
                            .AddAttribute("FfMacScheduler",
                                          "The scheduler associated to this EnbNetDevice",
                                          PointerValue(),
                                          MakePointerAccessor(&BandwidthPartGnb::m_scheduler),
                                          MakePointerChecker<FfMacScheduler>());
    return tid;
}

BandwidthPartGnb::BandwidthPartGnb()
    : ComponentCarrierBaseStation()
{
    NS_LOG_FUNCTION(this);
    m_phy = nullptr;
}

BandwidthPartGnb::~BandwidthPartGnb()
{
    NS_LOG_FUNCTION(this);
}

void
BandwidthPartGnb::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_phy->Dispose();
    m_phy = nullptr;
    m_mac->Dispose();
    m_mac = nullptr;
    m_scheduler = nullptr;
    Object::DoDispose();
}

Ptr<NrGnbPhy>
BandwidthPartGnb::GetPhy()
{
    NS_LOG_FUNCTION(this);
    return m_phy;
}

void
BandwidthPartGnb::SetPhy(Ptr<NrGnbPhy> s)
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_IF(m_phy != nullptr);
    m_phy = s;
}

Ptr<NrGnbMac>
BandwidthPartGnb::GetMac()
{
    NS_LOG_FUNCTION(this);
    return m_mac;
}

void
BandwidthPartGnb::SetMac(Ptr<NrGnbMac> s)
{
    NS_LOG_FUNCTION(this);
    m_mac = s;
}

Ptr<NrMacScheduler>
BandwidthPartGnb::GetScheduler()
{
    NS_LOG_FUNCTION(this);
    return m_scheduler;
}

void
BandwidthPartGnb::SetNrMacScheduler(Ptr<NrMacScheduler> s)
{
    NS_LOG_FUNCTION(this);
    m_scheduler = s;
}

void
BandwidthPartGnb::SetAsPrimary(bool primaryCarrier)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_phy != nullptr);
    if (primaryCarrier)
    {
        m_phy->SetPrimary();
    }
}

} // namespace ns3
