/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Biljana Bojovic <bbojovic@cttc.es>
 */

#include <ns3/uinteger.h>
#include <ns3/boolean.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/mmwave-enb-phy.h>
#include <ns3/pointer.h>
#include <ns3/mmwave-enb-mac.h>
#include <ns3/lte-ffr-algorithm.h>
#include <ns3/ff-mac-scheduler.h>
#include "component-carrier-gnb.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ComponentCarrierGnb");
NS_OBJECT_ENSURE_REGISTERED (ComponentCarrierGnb);

TypeId ComponentCarrierGnb::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::ComponentCarrierGnb")
    .SetParent<ComponentCarrierBaseStation> ()
    .AddConstructor<ComponentCarrierGnb> ()
    .AddAttribute ("MmWaveEnbPhy",
                   "The PHY associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&ComponentCarrierGnb::m_phy),
                   MakePointerChecker <MmWaveEnbPhy> ())
    .AddAttribute ("MmWaveEnbMac",
                   "The MAC associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&ComponentCarrierGnb::m_mac),
                   MakePointerChecker <MmWaveEnbMac> ())
    .AddAttribute ("FfMacScheduler",
                   "The scheduler associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&ComponentCarrierGnb::m_scheduler),
                   MakePointerChecker <FfMacScheduler> ())
  ;
  return tid;
}
ComponentCarrierGnb::ComponentCarrierGnb ()
  : ComponentCarrierBaseStation ()
{
  NS_LOG_FUNCTION (this);
  m_phy = nullptr;
}

ComponentCarrierGnb::~ComponentCarrierGnb (void)
{
  NS_LOG_FUNCTION (this);
}

void
ComponentCarrierGnb::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_phy)
    {
      m_phy->Dispose ();
      m_phy = 0;
    }
  if (m_mac)
    {
      m_mac->Dispose ();
      m_mac = 0;
    }
  if (m_scheduler)
    {
      m_scheduler->Dispose ();
      m_scheduler = 0;
    }
  Object::DoDispose ();
}


void
ComponentCarrierGnb::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  m_phy->Initialize ();
  m_mac->Initialize ();
  m_scheduler->Initialize ();
  ComponentCarrierBaseStation::DoInitialize ();
}

Ptr<MmWaveEnbPhy>
ComponentCarrierGnb::GetPhy ()
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void
ComponentCarrierGnb::SetPhy (Ptr<MmWaveEnbPhy> s)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_phy != nullptr);
  m_phy = s;
}

Ptr<MmWaveEnbMac>
ComponentCarrierGnb::GetMac ()
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}
void
ComponentCarrierGnb::SetMac (Ptr<MmWaveEnbMac> s)
{
  NS_LOG_FUNCTION (this);
  m_mac = s;
}


Ptr<MmWaveMacScheduler>
ComponentCarrierGnb::GetMmWaveMacScheduler ()
{
  NS_LOG_FUNCTION (this);
  return m_scheduler;
}

void
ComponentCarrierGnb::SetMmWaveMacScheduler (Ptr<MmWaveMacScheduler> s)
{
  NS_LOG_FUNCTION (this);
  m_scheduler = s;
}

/*void
ComponentCarrierGnb::SetFrequencyInHz (double centerFrequency)
{
  NS_LOG_FUNCTION (this);
  m_centerFrequencyInHz = centerFrequency;
}

void
ComponentCarrierGnb::SetNumerology (uint32_t numerology)
{
  NS_LOG_FUNCTION (this);
  m_numerology = numerology;
}*/



} // namespace ns3


