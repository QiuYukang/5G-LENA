/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "mmwave-ue-net-device.h"
#include "bandwidth-part-ue.h"
#include "mmwave-ue-mac.h"
#include "mmwave-ue-phy.h"
#include "mmwave-enb-net-device.h"
#include "bwp-manager-ue.h"
#include <ns3/lte-ue-rrc.h>
#include <ns3/epc-ue-nas.h>
#include <ns3/lte-ue-component-carrier-manager.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/object-map.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveUeNetDevice");

NS_OBJECT_ENSURE_REGISTERED (MmWaveUeNetDevice);

TypeId
MmWaveUeNetDevice::GetTypeId (void)
{
  static TypeId
    tid =  TypeId ("ns3::MmWaveUeNetDevice")
    .SetParent<MmWaveNetDevice> ()
    .AddConstructor<MmWaveUeNetDevice> ()
    .AddAttribute ("EpcUeNas",
                   "The NAS associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveUeNetDevice::m_nas),
                   MakePointerChecker <EpcUeNas> ())
    .AddAttribute ("mmWaveUeRrc",
                   "The RRC associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveUeNetDevice::m_rrc),
                   MakePointerChecker <LteUeRrc> ())
    .AddAttribute ("Imsi",
                   "International Mobile Subscriber Identity assigned to this UE",
                   UintegerValue (0),
                   MakeUintegerAccessor (&MmWaveUeNetDevice::m_imsi),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("LteUeRrc",
                   "The RRC layer associated with the ENB",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveUeNetDevice::m_rrc),
                   MakePointerChecker <LteUeRrc> ())
    .AddAttribute ("LteUeComponentCarrierManager",
                   "The ComponentCarrierManager associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveUeNetDevice::m_componentCarrierManager),
                   MakePointerChecker <LteUeComponentCarrierManager> ())
    .AddAttribute ("ComponentCarrierMapUe", "List of all component Carrier.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&MmWaveUeNetDevice::m_ccMap),
                   MakeObjectMapChecker<BandwidthPartUe> ())
  ;
  return tid;
}

MmWaveUeNetDevice::MmWaveUeNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}

MmWaveUeNetDevice::~MmWaveUeNetDevice (void)
{

}

void
MmWaveUeNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  m_rrc->Initialize ();
}
void
MmWaveUeNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_rrc->Dispose ();
}

std::map < uint8_t, Ptr<BandwidthPartUe> >
MmWaveUeNetDevice::GetCcMap ()
{
  NS_LOG_FUNCTION (this);
  return m_ccMap;
}

uint32_t
MmWaveUeNetDevice::GetCcMapSize() const
{
  NS_LOG_FUNCTION (this);
  return m_ccMap.size ();
}

void
MmWaveUeNetDevice::EnqueueDlHarqFeedback (const DlHarqInfo &m) const
{
  NS_LOG_FUNCTION (this);

  auto ccManager = DynamicCast<BwpManagerUe> (m_componentCarrierManager);
  NS_ASSERT (ccManager != nullptr);
  uint8_t index = ccManager->RouteDlHarqFeedback (m);
  m_ccMap.at (index)->GetPhy ()->EnqueueDlHarqFeedback (m);
}

void
MmWaveUeNetDevice::RouteIngoingCtrlMsgs (const std::list<Ptr<MmWaveControlMessage> > &msgList, uint8_t sourceBwpId)
{
  NS_LOG_FUNCTION (this);

  for (const auto & msg : msgList)
    {
      uint8_t bwpId = DynamicCast<BwpManagerUe> (m_componentCarrierManager)->RouteIngoingCtrlMsg (msg, sourceBwpId);
      m_ccMap.at (bwpId)->GetPhy ()->PhyCtrlMessagesReceived (msg);
    }
}

void
MmWaveUeNetDevice::RouteOutgoingCtrlMsgs (const std::list<Ptr<MmWaveControlMessage> > &msgList,
                                          uint8_t sourceBwpId)
{
  NS_LOG_FUNCTION (this);

  for (const auto & msg : msgList)
    {
      uint8_t bwpId = DynamicCast<BwpManagerUe> (m_componentCarrierManager)->RouteOutgoingCtrlMsg (msg, sourceBwpId);
      m_ccMap.at (bwpId)->GetPhy ()->EncodeCtrlMsg (msg);
    }
}

void
MmWaveUeNetDevice::SetCcMap (std::map< uint8_t, Ptr<BandwidthPartUe> > ccm)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_ccMap.size () > 0);
  m_ccMap = ccm;
}

uint32_t
MmWaveUeNetDevice::GetCsgId () const
{
  NS_LOG_FUNCTION (this);
  return m_csgId;
}

void
MmWaveUeNetDevice::SetCsgId (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << csgId);
  m_csgId = csgId;
  UpdateConfig (); // propagate the change down to NAS and RRC
}

void
MmWaveUeNetDevice::UpdateConfig (void)
{
  NS_LOG_FUNCTION (this);
  m_nas->SetImsi (m_imsi);
  m_rrc->SetImsi (m_imsi);
  m_nas->SetCsgId (m_csgId); // this also handles propagation to RRC
}

bool
MmWaveUeNetDevice::DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << dest << protocolNumber);
  if (protocolNumber != Ipv4L3Protocol::PROT_NUMBER)
    {
      NS_LOG_INFO ("unsupported protocol " << protocolNumber << ", only IPv4 is supported");
      return false;
    }
  return m_nas->Send (packet, protocolNumber);
}

Ptr<MmWaveUePhy>
MmWaveUeNetDevice::GetPhy (uint8_t index) const
{
  NS_LOG_FUNCTION (this);
  return m_ccMap.at (index)->GetPhy ();
}

Ptr<BwpManagerUe>
MmWaveUeNetDevice::GetBwpManager (void) const
{
  NS_LOG_FUNCTION (this);
  return DynamicCast<BwpManagerUe> (m_componentCarrierManager);
}

Ptr<EpcUeNas>
MmWaveUeNetDevice::GetNas (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nas;
}


Ptr<LteUeRrc>
MmWaveUeNetDevice::GetRrc (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rrc;
}

uint64_t
MmWaveUeNetDevice::GetImsi () const
{
  NS_LOG_FUNCTION (this);
  return m_imsi;
}

uint16_t
MmWaveUeNetDevice::GetEarfcn () const
{
  NS_LOG_FUNCTION (this);
  return m_earfcn;
}

uint16_t
MmWaveUeNetDevice::GetCellId () const
{
  auto gnb = GetTargetEnb ();
  if (gnb)
    {
      return GetTargetEnb ()->GetCellId ();
    }
  else
    {
      return UINT16_MAX;
    }
}

void
MmWaveUeNetDevice::SetEarfcn (uint16_t earfcn)
{
  NS_LOG_FUNCTION (this);
  m_earfcn = earfcn;
}

void
MmWaveUeNetDevice::SetTargetEnb (Ptr<MmWaveEnbNetDevice> enb)
{
  NS_LOG_FUNCTION (this);
  m_targetEnb = enb;
}

Ptr<const MmWaveEnbNetDevice>
MmWaveUeNetDevice::GetTargetEnb (void) const
{
  NS_LOG_FUNCTION (this);
  return m_targetEnb;
}

}
