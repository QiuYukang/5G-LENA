/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *                Sourjya Dutta <sdutta@nyu.edu>
 *                Russell Ford <russell.ford@nyu.edu>
 *                Menglei Zhang <menglei@nyu.edu>
 *                Biljana Bojovic <bbojovic@cttc.es>
 */

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      if (m_phyMacConfig)                                                \
        {                                                                \
          std::clog << " [ccId "                                         \
                    << static_cast<uint32_t> (m_phyMacConfig->GetCcId ())\
                    << "] ";                                             \
        }                                                                \
    }                                                                    \
  while (false);
#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/log.h>
#include "mmwave-phy.h"
#include "mmwave-phy-sap.h"
#include "mmwave-mac-pdu-tag.h"
#include "mmwave-mac-pdu-header.h"
#include "mmwave-net-device.h"
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWavePhy");

NS_OBJECT_ENSURE_REGISTERED ( MmWavePhy);

/*   SAP   */
class MmWaveMemberPhySapProvider : public MmWavePhySapProvider
{
public:
  MmWaveMemberPhySapProvider (MmWavePhy* phy);

  virtual void SendMacPdu (Ptr<Packet> p ) override;

  virtual void SendControlMessage (Ptr<MmWaveControlMessage> msg) override;

  virtual void SendRachPreamble (uint8_t PreambleId, uint8_t Rnti) override;

  virtual void SetSlotAllocInfo (SlotAllocInfo slotAllocInfo) override;

  virtual AntennaArrayModel::BeamId GetBeamId (uint8_t rnti) const override;

private:
  MmWavePhy* m_phy;
};

MmWaveMemberPhySapProvider::MmWaveMemberPhySapProvider (MmWavePhy* phy)
  : m_phy (phy)
{
  //  Nothing more to do
}

void
MmWaveMemberPhySapProvider::SendMacPdu (Ptr<Packet> p)
{
  m_phy->SetMacPdu (p);
}

void
MmWaveMemberPhySapProvider::SendControlMessage (Ptr<MmWaveControlMessage> msg)
{
  m_phy->SetControlMessage (msg);  //May need to change
}

void
MmWaveMemberPhySapProvider::SendRachPreamble (uint8_t PreambleId, uint8_t Rnti)
{
  m_phy->SendRachPreamble (PreambleId, Rnti);
}

void
MmWaveMemberPhySapProvider::SetSlotAllocInfo (SlotAllocInfo slotAllocInfo)
{
  m_phy->SetSlotAllocInfo (slotAllocInfo);
}

AntennaArrayModel::BeamId
MmWaveMemberPhySapProvider::GetBeamId (uint8_t rnti) const
{
  return m_phy->GetBeamId (rnti);
}

/* ======= */

TypeId
MmWavePhy::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::MmWavePhy")
    .SetParent<Object> ()
  ;

  return tid;
}

std::vector<int>
MmWavePhy::FromRBGBitmaskToRBAssignment (const std::vector<uint8_t> rbgBitmask) const
{
  NS_ASSERT (rbgBitmask.size () == m_phyMacConfig->GetBandwidthInRbg ());
  std::vector<int> ret;

  for (uint32_t i = 0; i < rbgBitmask.size (); ++i)
    {
      if (rbgBitmask.at (i) == 1)
        {
          for (uint32_t k = 0; k < m_phyMacConfig->GetNumRbPerRbg (); ++k)
            {
              ret.push_back ((i * m_phyMacConfig->GetNumRbPerRbg ()) + k);
            }
        }
    }

  NS_ASSERT (static_cast<uint32_t> (std::count (rbgBitmask.begin (), rbgBitmask.end (), 1) * m_phyMacConfig->GetNumRbPerRbg ())
             == ret.size ());
  return ret;
}

MmWavePhy::MmWavePhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

MmWavePhy::MmWavePhy (Ptr<MmWaveSpectrumPhy> dlChannelPhy, Ptr<MmWaveSpectrumPhy> ulChannelPhy)
  : m_downlinkSpectrumPhy (dlChannelPhy),
  m_uplinkSpectrumPhy (ulChannelPhy),
  m_cellId (0),
  m_frameNum (0),
  m_subframeNum (0),
  m_slotNum (0),
  m_varTtiNum (0),
  m_slotAllocInfoUpdated (false)
{
  NS_LOG_FUNCTION (this);
  m_phySapProvider = new MmWaveMemberPhySapProvider (this);
}

MmWavePhy::~MmWavePhy ()
{
  NS_LOG_FUNCTION (this);
  m_slotAllocInfo.clear ();
}

void
MmWavePhy::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
}

void
MmWavePhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_controlMessageQueue.clear ();
  delete m_phySapProvider;
  Object::DoDispose ();
}

void
MmWavePhy::SetDevice (Ptr<MmWaveNetDevice> d)
{
  NS_LOG_FUNCTION (this);
  m_netDevice = d;
}

Ptr<MmWaveNetDevice>
MmWavePhy::GetDevice ()
{
  NS_LOG_FUNCTION (this);
  return m_netDevice;
}

void
MmWavePhy::SetChannel (Ptr<SpectrumChannel> c)
{
  NS_LOG_FUNCTION (this);
}

void
MmWavePhy::DoSetCellId (uint16_t cellId)
{
  NS_LOG_FUNCTION (this);
  m_cellId = cellId;
  m_downlinkSpectrumPhy->SetCellId (cellId);
  m_uplinkSpectrumPhy->SetCellId (cellId);
}


void
MmWavePhy::SetNoiseFigure (double nf)
{
  NS_LOG_FUNCTION (this);
  m_noiseFigure = nf;
}

double
MmWavePhy::GetNoiseFigure (void) const
{
  NS_LOG_FUNCTION (this);
  return m_noiseFigure;
}

void
MmWavePhy::SendRachPreamble (uint32_t PreambleId, uint32_t Rnti)
{
  NS_LOG_FUNCTION (this);
  m_raPreambleId = PreambleId;
  Ptr<MmWaveRachPreambleMessage> msg = Create<MmWaveRachPreambleMessage> ();
  msg->SetRapId (PreambleId);
  SetControlMessage (msg);
}

void
MmWavePhy::SetMacPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  MmWaveMacPduTag tag;
  if (p->PeekPacketTag (tag))
    {
      NS_ASSERT ((tag.GetSfn ().m_slotNum >= 0) && (tag.GetSfn ().m_varTtiNum < m_phyMacConfig->GetSymbolsPerSlot ()));

      std::map<uint64_t, Ptr<PacketBurst> >::iterator it = m_packetBurstMap.find (tag.GetSfn ().Encode ());

      if (it == m_packetBurstMap.end ())
        {
          it = m_packetBurstMap.insert (std::pair<uint64_t, Ptr<PacketBurst> > (tag.GetSfn ().Encode (), CreateObject<PacketBurst> ())).first;
        }
      it->second->AddPacket (p);
    }
  else
    {
      NS_FATAL_ERROR ("No MAC packet PDU header available");
    }
}

Ptr<PacketBurst>
MmWavePhy::GetPacketBurst (SfnSf sfn)
{
  NS_LOG_FUNCTION (this);
  Ptr<PacketBurst> pburst;
  std::map<uint64_t, Ptr<PacketBurst> >::iterator it = m_packetBurstMap.find (sfn.Encode ());

  if (it == m_packetBurstMap.end ())
    {
      NS_LOG_ERROR ("GetPacketBurst(): Packet burst not found for subframe " << (unsigned)sfn.m_subframeNum << " slot" << (unsigned) sfn.m_slotNum << " tti start "  << (unsigned)sfn.m_varTtiNum);
      return pburst;
    }
  else
    {
      pburst = it->second;
      m_packetBurstMap.erase (it);
    }
  return pburst;
}

uint32_t
MmWavePhy::GetCcId() const
{
  if (m_phyMacConfig != nullptr)
    {
      return m_phyMacConfig->GetCcId ();
    }
  return 777;
}

void
MmWavePhy::SetControlMessage (Ptr<MmWaveControlMessage> m)
{
  NS_LOG_FUNCTION (this);
  if (m_controlMessageQueue.empty ())
    {
      std::list<Ptr<MmWaveControlMessage> > l;
      l.push_back (m);
      m_controlMessageQueue.push_back (l);
    }
  else
    {
      m_controlMessageQueue.at (m_controlMessageQueue.size () - 1).push_back (m);
    }
}

std::list<Ptr<MmWaveControlMessage> >
MmWavePhy::GetControlMessages (void)
{
  NS_LOG_FUNCTION (this);
  if (m_controlMessageQueue.empty ())
    {
      std::list<Ptr<MmWaveControlMessage> > emptylist;
      return (emptylist);
    }

  if (m_controlMessageQueue.at (0).size () > 0)
    {
      std::list<Ptr<MmWaveControlMessage> > ret = m_controlMessageQueue.front ();
      m_controlMessageQueue.erase (m_controlMessageQueue.begin ());
      std::list<Ptr<MmWaveControlMessage> > newlist;
      m_controlMessageQueue.push_back (newlist);
      return (ret);
    }
  else
    {
      m_controlMessageQueue.erase (m_controlMessageQueue.begin ());
      std::list<Ptr<MmWaveControlMessage> > newlist;
      m_controlMessageQueue.push_back (newlist);
      std::list<Ptr<MmWaveControlMessage> > emptylist;
      return (emptylist);
    }
}

void
MmWavePhy::SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig)
{
  NS_LOG_FUNCTION (this);
  m_phyMacConfig = ptrConfig;
}

Ptr<MmWavePhyMacCommon>
MmWavePhy::GetConfigurationParameters (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyMacConfig;
}


MmWavePhySapProvider*
MmWavePhy::GetPhySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_phySapProvider;
}

void
MmWavePhy::SetSlotAllocInfo (const SlotAllocInfo &slotAllocInfo)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("ccId:" << static_cast<uint32_t> (m_componentCarrierId) <<
               " frameNum:" << static_cast<uint32_t> (slotAllocInfo.m_sfnSf.m_frameNum) <<
               " subframe:" << static_cast<uint32_t> (slotAllocInfo.m_sfnSf.m_subframeNum) <<
               " slot:" << static_cast<uint32_t> (slotAllocInfo.m_sfnSf.m_slotNum));

  SfnSf sf = slotAllocInfo.m_sfnSf;

  if (m_slotAllocInfo.find (sf) == m_slotAllocInfo.end ())
    {
      m_slotAllocInfo [sf] = slotAllocInfo;
    }
  else
    {
      m_slotAllocInfo [sf].Merge (slotAllocInfo);
    }
}


bool
MmWavePhy::SlotExists (const SfnSf &retVal) const
{
  NS_LOG_FUNCTION (this);
  return m_slotAllocInfo.find (retVal) != m_slotAllocInfo.end ();
}


SlotAllocInfo
MmWavePhy::GetSlotAllocInfo (const SfnSf &sfnsf)
{
  NS_LOG_FUNCTION (this << " at:" << Simulator::Now ().GetSeconds () << "ccId:" << (unsigned)m_componentCarrierId << "frameNum:" << sfnsf.m_frameNum <<
                   "subframe:" << (unsigned)sfnsf.m_subframeNum << "slot:" << (unsigned)sfnsf.m_slotNum);

  NS_ASSERT_MSG (m_slotAllocInfo.find (sfnsf) != m_slotAllocInfo.end (), "Trying to fetch a non existing slot allocation info.");
  SlotAllocInfo slot = m_slotAllocInfo[sfnsf];
  m_slotAllocInfo.erase (m_slotAllocInfo.find (sfnsf));
  return slot;
}

SlotAllocInfo &
MmWavePhy::PeekSlotAllocInfo (const SfnSf &sfnsf)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_slotAllocInfo.find (sfnsf) != m_slotAllocInfo.end (),
                 "Trying to fetch a non existing slot allocation info.");
  return m_slotAllocInfo[sfnsf];
}

void
MmWavePhy::SetComponentCarrierId (uint8_t index)
{
  NS_LOG_FUNCTION (this);
  m_componentCarrierId = index;
  m_downlinkSpectrumPhy->SetComponentCarrierId (index);
  m_uplinkSpectrumPhy->SetComponentCarrierId (index);
}

uint8_t
MmWavePhy::GetComponentCarrierId ()
{
  NS_LOG_FUNCTION (this);
  return m_componentCarrierId;
}

}
