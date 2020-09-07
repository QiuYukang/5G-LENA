/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "nr-sl-ue-mac-harq.h"
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacHarq");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacHarq);

TypeId
NrSlUeMacHarq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacHarq")
    .SetParent<Object> ()
    .AddConstructor <NrSlUeMacHarq> ()
    .SetGroupName ("nr")
  ;

  return tid;
}

NrSlUeMacHarq::NrSlUeMacHarq ()
{
}

NrSlUeMacHarq::~NrSlUeMacHarq ()
{
}

void
NrSlUeMacHarq::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_nrSlProcessesPackets.clear ();
}

void
NrSlUeMacHarq::AddDst (uint32_t dstL2Id, uint8_t maxSidelinkProcess)
{
  NS_LOG_FUNCTION (this << dstL2Id << +maxSidelinkProcess);
  std::map <uint32_t, NrSlProcessesBuffer_t>::iterator it;
  it = m_nrSlProcessesPackets.find (dstL2Id);
  NS_ABORT_MSG_IF (it != m_nrSlProcessesPackets.end (), "the destination " << dstL2Id << " already exist");
  // Create SL transmission HARQ buffers for the destination
  NrSlProcessesBuffer_t buf;
  buf.resize (maxSidelinkProcess);
  for (uint8_t i = 0; i < maxSidelinkProcess; i++)
    {
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      buf.at (i).pktBurst = pb;
    }
  m_nrSlProcessesPackets.insert (std::make_pair (dstL2Id, buf));
}

uint8_t
NrSlUeMacHarq::AssignNrSlHarqProcessId (uint32_t dstL2Id)
{
  NS_LOG_FUNCTION (this << dstL2Id);
  std::map <uint32_t, NrSlProcessesBuffer_t>::iterator it;
  it = m_nrSlProcessesPackets.find (dstL2Id);
  NS_ABORT_MSG_IF (it == m_nrSlProcessesPackets.end (), "the destination " << dstL2Id << " does not exist");
  uint8_t availableHarqId = 255;
  for (uint8_t i = 0; i < it->second.size (); i++)
    {
      if (it->second.at (i).slProcessStatus == NrSlProcessInfo::IDLE)
        {
          //associate Sidelink process id with HARQ id
          availableHarqId = i;
          it->second.at (i).slProcessStatus = NrSlProcessInfo::BUSY;
          break;
        }
    }

  NS_ABORT_MSG_IF (availableHarqId == 255, "All the Sidelink processes are busy for " << dstL2Id);

  return availableHarqId;
}

void
NrSlUeMacHarq::AddPacket (uint32_t dstL2Id, uint8_t lcId, uint8_t harqId, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << dstL2Id << +lcId << +harqId);
  std::map <uint32_t, NrSlProcessesBuffer_t>::iterator it;
  it = m_nrSlProcessesPackets.find (dstL2Id);
  NS_ABORT_MSG_IF (it == m_nrSlProcessesPackets.end (), "the destination " << dstL2Id << " does not exist");
  if (it->second.at (harqId).slProcessStatus == NrSlProcessInfo::BUSY)
    {
      it->second.at (harqId).lcidList.insert (lcId);
      it->second.at (harqId).pktBurst->AddPacket (pkt);
    }
  //Each LC have one MAC PDU in a TB. Packet burst here, imitates a TB, therefore,
  //the number of LCs inside lcidList and the packets inside the packet burst
  //must be equal.
  NS_ABORT_MSG_IF (it->second.at (harqId).lcidList.size () == it->second.at (harqId).pktBurst->GetNPackets (),
                   "Mismatch in number of LCIDs and the number of packets for SL HARQ ID " << +harqId << " dest " << dstL2Id);
}

void
NrSlUeMacHarq::RecvSlHarqFeedback (uint32_t dstL2Id, uint8_t harqProcessId)
{
  NS_LOG_FUNCTION (this << dstL2Id << +harqProcessId);
  std::map <uint32_t, NrSlProcessesBuffer_t>::iterator it;
  it = m_nrSlProcessesPackets.find (dstL2Id);
  NS_ABORT_MSG_IF (it == m_nrSlProcessesPackets.end (), "the destination " << dstL2Id << " does not exist");
  NS_ABORT_MSG_IF (it->second.at (harqProcessId).slProcessStatus == NrSlProcessInfo::BUSY,
                   "Can not refresh HARQ buffer of already available SL process " << +harqProcessId << " of the destination " << dstL2Id);
  //refresh Sidelink process info
  it->second.at (harqProcessId).slProcessStatus = NrSlProcessInfo::IDLE;
  Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
  it->second.at (harqProcessId).pktBurst = pb;
  it->second.at (harqProcessId).lcidList.clear ();
}



} // namespace ns3


