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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);

#include "mmwave-ue-mac.h"
#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/random-variable-stream.h>
#include "mmwave-phy-sap.h"
#include "mmwave-control-messages.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveUeMac");
NS_OBJECT_ENSURE_REGISTERED (MmWaveUeMac);

uint8_t MmWaveUeMac::g_raPreambleId = 0;

///////////////////////////////////////////////////////////
// SAP forwarders
///////////////////////////////////////////////////////////


class UeMemberMmWaveUeCmacSapProvider : public LteUeCmacSapProvider
{
public:
  UeMemberMmWaveUeCmacSapProvider (MmWaveUeMac* mac);

  // inherited from LteUeCmacSapProvider
  virtual void ConfigureRach (RachConfig rc);
  virtual void StartContentionBasedRandomAccessProcedure ();
  virtual void StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask);
  virtual void AddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu);
  virtual void RemoveLc (uint8_t lcId);
  virtual void Reset ();
  virtual void SetRnti (uint16_t rnti);
  virtual void NotifyConnectionSuccessful ();
  virtual void SetImsi (uint64_t imsi);

private:
  MmWaveUeMac* m_mac;
};


UeMemberMmWaveUeCmacSapProvider::UeMemberMmWaveUeCmacSapProvider (MmWaveUeMac* mac)
  : m_mac (mac)
{
}

void
UeMemberMmWaveUeCmacSapProvider::ConfigureRach (RachConfig rc)
{
  m_mac->DoConfigureRach (rc);
}

void
UeMemberMmWaveUeCmacSapProvider::StartContentionBasedRandomAccessProcedure ()
{
  m_mac->DoStartContentionBasedRandomAccessProcedure ();
}

void
UeMemberMmWaveUeCmacSapProvider::StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask)
{
  m_mac->DoStartNonContentionBasedRandomAccessProcedure (rnti, preambleId, prachMask);
}


void
UeMemberMmWaveUeCmacSapProvider::AddLc (uint8_t lcId, LogicalChannelConfig lcConfig, LteMacSapUser* msu)
{
  m_mac->AddLc (lcId, lcConfig, msu);
}

void
UeMemberMmWaveUeCmacSapProvider::RemoveLc (uint8_t lcid)
{
  m_mac->DoRemoveLc (lcid);
}

void
UeMemberMmWaveUeCmacSapProvider::Reset ()
{
  m_mac->DoReset ();
}

void
UeMemberMmWaveUeCmacSapProvider::SetRnti (uint16_t rnti)
{
  m_mac->SetRnti (rnti);
}

void
UeMemberMmWaveUeCmacSapProvider::NotifyConnectionSuccessful ()
{
  m_mac->DoNotifyConnectionSuccessful ();
}

void
UeMemberMmWaveUeCmacSapProvider::SetImsi (uint64_t imsi)
 {
   m_mac->DoSetImsi (imsi);
 }

class UeMemberMmWaveMacSapProvider : public LteMacSapProvider
{
public:
  UeMemberMmWaveMacSapProvider (MmWaveUeMac* mac);

  // inherited from LteMacSapProvider
  virtual void TransmitPdu (TransmitPduParameters params);
  virtual void ReportBufferStatus (ReportBufferStatusParameters params);

private:
  MmWaveUeMac* m_mac;
};


UeMemberMmWaveMacSapProvider::UeMemberMmWaveMacSapProvider (MmWaveUeMac* mac)
  : m_mac (mac)
{
}

void
UeMemberMmWaveMacSapProvider::TransmitPdu (TransmitPduParameters params)
{
  m_mac->DoTransmitPdu (params);
}


void
UeMemberMmWaveMacSapProvider::ReportBufferStatus (ReportBufferStatusParameters params)
{
  m_mac->DoReportBufferStatus (params);
}


class MmWaveUePhySapUser;

class MacUeMemberPhySapUser : public MmWaveUePhySapUser
{
public:
  MacUeMemberPhySapUser (MmWaveUeMac* mac);

  virtual void ReceivePhyPdu (Ptr<Packet> p) override;

  virtual void ReceiveControlMessage (Ptr<MmWaveControlMessage> msg) override;

  virtual void SlotIndication (SfnSf sfn) override;

  //virtual void NotifyHarqDeliveryFailure (uint8_t harqId);

  virtual uint8_t GetNumHarqProcess () const override;

private:
  MmWaveUeMac* m_mac;
};

MacUeMemberPhySapUser::MacUeMemberPhySapUser (MmWaveUeMac* mac)
  : m_mac (mac)
{

}
void
MacUeMemberPhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
MacUeMemberPhySapUser::ReceiveControlMessage (Ptr<MmWaveControlMessage> msg)
{
  m_mac->DoReceiveControlMessage (msg);
}

void
MacUeMemberPhySapUser::SlotIndication (SfnSf sfn)
{
  m_mac->DoSlotIndication (sfn);
}

uint8_t
MacUeMemberPhySapUser::GetNumHarqProcess () const
{
  return m_mac->GetNumHarqProcess();
}

//-----------------------------------------------------------------------

TypeId
MmWaveUeMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveUeMac")
    .SetParent<Object> ()
    .AddConstructor<MmWaveUeMac> ()
    .AddAttribute ("NumHarqProcess",
                   "Number of concurrent stop-and-wait Hybrid ARQ processes per user",
                    UintegerValue (20),
                    MakeUintegerAccessor (&MmWaveUeMac::SetNumHarqProcess,
                                          &MmWaveUeMac::GetNumHarqProcess),
                    MakeUintegerChecker<uint8_t> ())
    .AddTraceSource ("UeMacRxedCtrlMsgsTrace",
                     "Ue MAC Control Messages Traces.",
                     MakeTraceSourceAccessor (&MmWaveUeMac::m_macRxedCtrlMsgsTrace),
                     "ns3::MmWaveMacRxTrace::RxedUeMacCtrlMsgsTracedCallback")
    .AddTraceSource ("UeMacTxedCtrlMsgsTrace",
                     "Ue MAC Control Messages Traces.",
                     MakeTraceSourceAccessor (&MmWaveUeMac::m_macTxedCtrlMsgsTrace),
                     "ns3::MmWaveMacRxTrace::TxedUeMacCtrlMsgsTracedCallback")
  ;
  return tid;
}

MmWaveUeMac::MmWaveUeMac (void) : Object ()
{
  NS_LOG_FUNCTION (this);
  m_cmacSapProvider = new UeMemberMmWaveUeCmacSapProvider (this);
  m_macSapProvider = new UeMemberMmWaveMacSapProvider (this);
  m_phySapUser = new MacUeMemberPhySapUser (this);
  m_raPreambleUniformVariable = CreateObject<UniformRandomVariable> ();
}

MmWaveUeMac::~MmWaveUeMac (void)
{
  NS_LOG_FUNCTION (this);
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_phySapUser;
  m_miUlHarqProcessesPacket.clear ();
}

void
MmWaveUeMac::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  m_rnti = rnti;
}

void
MmWaveUeMac::DoNotifyConnectionSuccessful ()
{
  NS_LOG_FUNCTION (this);
  m_phySapProvider->NotifyConnectionSuccessful ();
}

void
MmWaveUeMac::DoSetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this);
  m_imsi = imsi;
}

uint16_t
MmWaveUeMac::GetBwpId () const
{
  if (m_phySapProvider)
    {
      return m_phySapProvider->GetBwpId ();
    }
  else
    {
      return UINT16_MAX;
    }
}

uint16_t
MmWaveUeMac::GetCellId () const
{
  if (m_phySapProvider)
    {
      return m_phySapProvider->GetCellId ();
    }
  else
    {
      return UINT16_MAX;
    }
}

uint32_t
MmWaveUeMac::GetTotalBufSize () const
{
  NS_LOG_FUNCTION (this);
  uint32_t ret = 0;
  for (auto it = m_ulBsrReceived.cbegin (); it != m_ulBsrReceived.cend (); ++it)
    {
      ret += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
    }
  return ret;
}

/**
 * \brief Sets the number of HARQ processes
 * \param numHarqProcesses the maximum number of harq processes
 */
void
MmWaveUeMac::SetNumHarqProcess (uint8_t numHarqProcess)
{
  m_numHarqProcess = numHarqProcess;

  m_miUlHarqProcessesPacket.resize (GetNumHarqProcess ());
  for (uint8_t i = 0; i < m_miUlHarqProcessesPacket.size (); i++)
    {
      if (m_miUlHarqProcessesPacket.at (i).m_pktBurst == nullptr)
        {
          Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
          m_miUlHarqProcessesPacket.at (i).m_pktBurst = pb;
        }
    }
  m_miUlHarqProcessesPacketTimer.resize (GetNumHarqProcess (), 0);
}

/**
 * \return number of HARQ processes
 */
uint8_t
MmWaveUeMac::GetNumHarqProcess () const
{
  return m_numHarqProcess;
}

// forwarded from MAC SAP
void
MmWaveUeMac::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
{
  // TB UID passed back along with RLC data as HARQ process ID
  std::map<uint32_t, struct MacPduInfo>::iterator it = m_macPduMap.find (params.harqProcessId);
  if (it == m_macPduMap.end ())
    {
      NS_FATAL_ERROR ("No MAC PDU storage element found for this TB UID/RNTI");
    }
  else
    {
      MmWaveMacPduTag tag;
      it->second.m_pdu->PeekPacketTag (tag);

      /*if (tag.GetSfn ().m_frameNum < m_frameNum) // what is purpose of this?
        {
          return;
        }*/

      if (it->second.m_pdu == 0)
        {
          it->second.m_pdu = params.pdu;
        }
      else
        {

          it->second.m_pdu->AddAtEnd (params.pdu);   // append to MAC PDU
        }

      //it->second.m_pdu->AddAtEnd (params.pdu); // append to MAC PDU

      MacSubheader subheader (params.lcid, params.pdu->GetSize ());
      it->second.m_macHeader.AddSubheader (subheader);   // add RLC PDU sub-header into MAC header
      m_miUlHarqProcessesPacket.at (params.harqProcessId).m_lcidList.push_back (params.lcid);
      if (it->second.m_size <
          (params.pdu->GetSize () + it->second.m_macHeader.GetSerializedSize ()))
        {
          NS_FATAL_ERROR ("Maximum TB size exceeded");
        }

      if (it->second.m_numRlcPdu <= 1)
        {
          // wait for all RLC PDUs to be received
          it->second.m_pdu->AddHeader (it->second.m_macHeader);

          MmWaveMacPduHeader headerTst;
          it->second.m_pdu->PeekHeader (headerTst);
          LteRadioBearerTag bearerTag (params.rnti, 0, 0);
          it->second.m_pdu->AddPacketTag (bearerTag);
          m_miUlHarqProcessesPacket.at (params.harqProcessId).m_pktBurst->AddPacket (it->second.m_pdu);
          m_miUlHarqProcessesPacketTimer.at (params.harqProcessId) = GetNumHarqProcess();
          //m_harqProcessId = (m_harqProcessId + 1) % GetNumHarqProcess;
          m_phySapProvider->SendMacPdu (it->second.m_pdu);
          m_macPduMap.erase (it);    // delete map entry
        }
      else
        {
          it->second.m_numRlcPdu--;   // decrement count of remaining RLC requests
        }
    }
}

void
MmWaveUeMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (params.lcid));

  auto it = m_ulBsrReceived.find (params.lcid);

  NS_LOG_INFO ("Received BSR for LC Id" << static_cast<uint32_t>(params.lcid));

  if (it != m_ulBsrReceived.end ())
    {
      // update entry
      (*it).second = params;
    }
  else
    {
      it = m_ulBsrReceived.insert (std::make_pair (params.lcid, params)).first;
    }

  if (m_srState == INACTIVE)
    {
      NS_LOG_INFO ("INACTIVE -> TO_SEND, bufSize " << GetTotalBufSize ());
      m_srState = TO_SEND;
    }
}


void
MmWaveUeMac::SendReportBufferStatus (void)
{
  NS_LOG_FUNCTION (this);

  if (m_rnti == 0)
    {
      NS_LOG_INFO ("MAC not initialized, BSR deferred");
      return;
    }

  if (m_ulBsrReceived.size () == 0)
    {
      NS_LOG_INFO ("No BSR report to transmit");
      return;
    }
  MacCeElement bsr = MacCeElement ();
  bsr.m_rnti = m_rnti;
  bsr.m_macCeType = MacCeElement::BSR;

  // BSR is reported for each LCG
  std::map <uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  std::vector<uint32_t> queue (4, 0);   // one value per each of the 4 LCGs, initialized to 0
  for (it = m_ulBsrReceived.begin (); it != m_ulBsrReceived.end (); it++)
    {
      uint8_t lcid = it->first;
      std::map <uint8_t, LcInfo>::iterator lcInfoMapIt;
      lcInfoMapIt = m_lcInfoMap.find (lcid);
      NS_ASSERT (lcInfoMapIt !=  m_lcInfoMap.end ());
      NS_ASSERT_MSG ((lcid != 0) || (((*it).second.txQueueSize == 0)
                                     && ((*it).second.retxQueueSize == 0)
                                     && ((*it).second.statusPduSize == 0)),
                     "BSR should not be used for LCID 0");
      uint8_t lcg = lcInfoMapIt->second.lcConfig.logicalChannelGroup;
      queue.at (lcg) += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
    }

  NS_LOG_INFO ("Sending BSR with this info for the LCG: " << queue.at (0) << " " <<
               queue.at (1) << " " << queue.at(2) << " " << queue.at(3));
  // FF API says that all 4 LCGs are always present
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (0)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (1)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (2)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (3)));

  // create the feedback to eNB
  Ptr<MmWaveBsrMessage> msg = Create<MmWaveBsrMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetBsr (bsr);

  m_macTxedCtrlMsgsTrace (m_currentSlot, bsr.m_rnti, GetBwpId (), msg);
  m_phySapProvider->SendControlMessage (msg);
}

void
MmWaveUeMac::SetUeCmacSapUser (LteUeCmacSapUser* s)
{
  m_cmacSapUser = s;
}

LteUeCmacSapProvider*
MmWaveUeMac::GetUeCmacSapProvider (void)
{
  return m_cmacSapProvider;
}

void
MmWaveUeMac::RefreshHarqProcessesPacketBuffer (void)
{
  NS_LOG_FUNCTION (this);

  for (uint16_t i = 0; i < m_miUlHarqProcessesPacketTimer.size (); i++)
    {
      if (m_miUlHarqProcessesPacketTimer.at (i) == 0)
        {
          if (m_miUlHarqProcessesPacket.at (i).m_pktBurst->GetSize () > 0)
            {
              // timer expired: drop packets in buffer for this process
              NS_LOG_INFO ("HARQ Proc Id " << i << " packets buffer expired");
              Ptr<PacketBurst> emptyPb = CreateObject <PacketBurst> ();
              m_miUlHarqProcessesPacket.at (i).m_pktBurst = emptyPb;
              m_miUlHarqProcessesPacket.at (i).m_lcidList.clear ();
            }
        }
      else
        {
          //m_miUlHarqProcessesPacketTimer.at (i)--;  // ignore HARQ timeout
        }
    }
}

void
MmWaveUeMac::DoSlotIndication (SfnSf sfn)
{
  NS_LOG_FUNCTION (this);
  m_currentSlot = sfn;
  NS_LOG_INFO ("Slot " << m_currentSlot);

  RefreshHarqProcessesPacketBuffer ();

  if (m_srState == TO_SEND)
    {
      NS_LOG_INFO ("Sending SR to PHY in slot " << sfn);
      SendSR ();
      m_srState = ACTIVE;
    }

  // Feedback missing
}

void
MmWaveUeMac::SendSR () const
{
  NS_LOG_FUNCTION (this);

  if (m_rnti == 0)
    {
      NS_LOG_INFO ("MAC not initialized, SR deferred");
      return;
    }

  // create the SR to send to the gNB
  Ptr<MmWaveSRMessage> msg = Create<MmWaveSRMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetMessageType (MmWaveControlMessage::SR);
  msg->SetRNTI (m_rnti);

  m_macTxedCtrlMsgsTrace (m_currentSlot, m_rnti, GetBwpId (), msg);
  m_phySapProvider->SendControlMessage (msg);
}

void
MmWaveUeMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  LteRadioBearerTag tag;
  p->RemovePacketTag (tag);
  MmWaveMacPduHeader macHeader;
  p->RemoveHeader (macHeader);
  if (tag.GetRnti () == m_rnti)   // packet is for the current user
    {
      std::vector<MacSubheader> macSubheaders = macHeader.GetSubheaders ();
      uint32_t currPos = 0;
      for (unsigned ipdu = 0; ipdu < macSubheaders.size (); ipdu++)
        {
          if (macSubheaders[ipdu].m_size == 0)
            {
              continue;
            }

          std::map <uint8_t, LcInfo>::const_iterator it = m_lcInfoMap.find (macSubheaders[ipdu].m_lcid);
          NS_ASSERT_MSG (it != m_lcInfoMap.end (), "received packet with unknown lcid");
          Ptr<Packet> rlcPdu;
          if ((p->GetSize () - currPos) < (uint32_t)macSubheaders[ipdu].m_size)
            {
              NS_LOG_ERROR ("Packet size less than specified in MAC header (actual= " \
                            << p->GetSize () << " header= " << (uint32_t)macSubheaders[ipdu].m_size << ")" );
            }
          else if ((p->GetSize () - currPos) > (uint32_t)macSubheaders[ipdu].m_size)
            {
              NS_LOG_DEBUG ("Fragmenting MAC PDU (packet size greater than specified in MAC header (actual= " \
                            << p->GetSize () << " header= " << (uint32_t)macSubheaders[ipdu].m_size << ")" );
              rlcPdu = p->CreateFragment (currPos, (uint32_t)macSubheaders[ipdu].m_size);
              currPos += (uint32_t)macSubheaders[ipdu].m_size;
              it->second.macSapUser->ReceivePdu (LteMacSapUser::ReceivePduParameters (rlcPdu, m_rnti, macSubheaders[ipdu].m_lcid));
            }
          else
            {
              rlcPdu = p->CreateFragment (currPos, p->GetSize () - currPos);
              currPos = p->GetSize ();
              it->second.macSapUser->ReceivePdu (LteMacSapUser::ReceivePduParameters (rlcPdu, m_rnti, macSubheaders[ipdu].m_lcid));
            }
        }
    }
}

void
MmWaveUeMac::RecvRaResponse (BuildRarListElement_s raResponse)
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = false;
  m_rnti = raResponse.m_rnti;
  m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
  m_cmacSapUser->NotifyRandomAccessSuccessful ();
}

std::map<uint32_t, struct MacPduInfo>::iterator
MmWaveUeMac::AddToMacPduMap (const std::shared_ptr<DciInfoElementTdma> &dci,
                             unsigned activeLcs, const SfnSf &ulSfn)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Adding PDU at the position " << ulSfn);

  MacPduInfo macPduInfo (ulSfn, activeLcs, *dci);
  std::map<uint32_t, struct MacPduInfo>::iterator it = m_macPduMap.find (dci->m_harqProcess);

  if (it != m_macPduMap.end ())
    {
      m_macPduMap.erase (it);
    }
  it = (m_macPduMap.insert (std::make_pair (dci->m_harqProcess, macPduInfo))).first;
  return it;
}

void
MmWaveUeMac::ProcessUlDci (const Ptr<MmWaveUlDciMessage> &dciMsg)
{
  SfnSf dataSfn = m_currentSlot;
  dataSfn.Add (dciMsg->GetKDelay ());

  auto dciInfoElem = dciMsg->GetDciInfoElement ();

  m_macRxedCtrlMsgsTrace (m_currentSlot, m_rnti, GetBwpId (), dciMsg);

  NS_LOG_INFO ("UL DCI received, transmit data in slot " << dataSfn <<
               " TBS " << dciInfoElem->m_tbSize << " total queue " << GetTotalBufSize ());
  if (dciInfoElem->m_ndi == 1)
    {
      // New transmission -> empty pkt buffer queue (for deleting eventual pkts not acked )
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      m_miUlHarqProcessesPacket.at (dciInfoElem->m_harqProcess).m_pktBurst = pb;
      m_miUlHarqProcessesPacket.at (dciInfoElem->m_harqProcess).m_lcidList.clear ();
      // Retrieve data from RLC
      std::map <uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
      uint16_t activeLcs = 0;
      uint32_t statusPduMinSize = 0;
      for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
        {
          if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) || ((*itBsr).second.txQueueSize > 0))
            {
              activeLcs++;
              if (((*itBsr).second.statusPduSize != 0)&&((*itBsr).second.statusPduSize < statusPduMinSize))
                {
                  statusPduMinSize = (*itBsr).second.statusPduSize;
                }
              if (((*itBsr).second.statusPduSize != 0)&&(statusPduMinSize == 0))
                {
                  statusPduMinSize = (*itBsr).second.statusPduSize;
                }
            }
        }

      if (activeLcs == 0)
        {
          NS_LOG_WARN ("No active flows for this UL-DCI");
          // the UE may have been scheduled when it has no buffered data due to BSR quantization, send empty packet

          MmWaveMacPduTag tag (dataSfn, dciInfoElem->m_symStart, dciInfoElem->m_numSym);
          Ptr<Packet> emptyPdu = Create <Packet> ();
          MmWaveMacPduHeader header;
          MacSubheader subheader (3, 0);  // lcid = 3, size = 0
          header.AddSubheader (subheader);
          emptyPdu->AddHeader (header);
          emptyPdu->AddPacketTag (tag);
          LteRadioBearerTag bearerTag (dciInfoElem->m_rnti, 3, 0);
          emptyPdu->AddPacketTag (bearerTag);
          m_miUlHarqProcessesPacket.at (dciInfoElem->m_harqProcess).m_pktBurst->AddPacket (emptyPdu);
          m_miUlHarqProcessesPacketTimer.at (dciInfoElem->m_harqProcess) = GetNumHarqProcess ();
          //m_harqProcessId = (m_harqProcessId + 1) % GetNumHarqProcess;
          m_phySapProvider->SendMacPdu (emptyPdu);
          return;
        }

      std::map<uint32_t, struct MacPduInfo>::iterator macPduIt = AddToMacPduMap (dciInfoElem, activeLcs, dataSfn);
      std::map <uint8_t, LcInfo>::iterator lcIt;
      uint32_t bytesPerActiveLc = dciInfoElem->m_tbSize / activeLcs;
      bool statusPduPriority = false;
      if ((statusPduMinSize != 0)&&(bytesPerActiveLc < statusPduMinSize))
        {
          // send only the status PDU which has highest priority
          statusPduPriority = true;
          NS_LOG_DEBUG (this << " Reduced resource -> send only Status, bytes " << statusPduMinSize);
          if (dciInfoElem->m_tbSize < statusPduMinSize)
            {
              NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
            }
        }
      NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of " << dciInfoElem->m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC" << " statusPduMinSize " << statusPduMinSize);
      for (lcIt = m_lcInfoMap.begin (); lcIt != m_lcInfoMap.end (); lcIt++)
        {
          itBsr = m_ulBsrReceived.find ((*lcIt).first);
          NS_LOG_DEBUG (this << " Processing LC " << (uint32_t)(*lcIt).first << " bytesPerActiveLc " << bytesPerActiveLc);
          if ( (itBsr != m_ulBsrReceived.end ())
               && ( ((*itBsr).second.statusPduSize > 0)
                    || ((*itBsr).second.retxQueueSize > 0)
                    || ((*itBsr).second.txQueueSize > 0)) )
            {
              if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                {
                  MacSubheader subheader ((*lcIt).first,(*itBsr).second.statusPduSize);
                  (*lcIt).second.macSapUser->NotifyTxOpportunity (LteMacSapUser::TxOpportunityParameters (((*itBsr).second.statusPduSize), 0, dciInfoElem->m_harqProcess, GetBwpId (), m_rnti, (*lcIt).first));
                  NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  " << (*itBsr).second.statusPduSize << " status bytes to LC " << (uint32_t)(*lcIt).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                  (*itBsr).second.statusPduSize = 0;
                  break;
                }
              else
                {
                  uint32_t bytesForThisLc = bytesPerActiveLc;
                  NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC " << (uint32_t)(*lcIt).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                  if (((*itBsr).second.statusPduSize > 0) && (bytesForThisLc > (*itBsr).second.statusPduSize))
                    {
                      if ((*itBsr).second.txQueueSize > 0 || (*itBsr).second.retxQueueSize > 0)
                        {
                          macPduIt->second.m_numRlcPdu++;  // send status PDU + data PDU
                        }
                      //MacSubheader subheader((*lcIt).first,(*itBsr).second.statusPduSize);
                      (*lcIt).second.macSapUser->NotifyTxOpportunity (LteMacSapUser::TxOpportunityParameters (((*itBsr).second.statusPduSize), 0, dciInfoElem->m_harqProcess, GetBwpId (), m_rnti, (*lcIt).first));
                      bytesForThisLc -= (*itBsr).second.statusPduSize;
                      NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                      (*itBsr).second.statusPduSize = 0;
                    }
                  else
                    {
                      if ((*itBsr).second.statusPduSize > bytesForThisLc)
                        {
                          NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                        }
                    }

                  if ((bytesForThisLc > 7)    // 7 is the min TxOpportunity useful for Rlc
                      && (((*itBsr).second.retxQueueSize > 0)
                          || ((*itBsr).second.txQueueSize > 0)))
                    {
                      if ((*itBsr).second.retxQueueSize > 0)
                        {
                          NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                          MacSubheader subheader ((*lcIt).first, bytesForThisLc);
                          (*lcIt).second.macSapUser->NotifyTxOpportunity ( LteMacSapUser::TxOpportunityParameters ((bytesForThisLc - subheader.GetSize () - 1), 0, dciInfoElem->m_harqProcess, GetBwpId (), m_rnti, (*lcIt).first)); //zml add 1 byte overhead
                          if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                            {
                              (*itBsr).second.retxQueueSize -= bytesForThisLc;
                            }
                          else
                            {
                              (*itBsr).second.retxQueueSize = 0;
                            }
                        }
                      else if ((*itBsr).second.txQueueSize > 0)
                        {
                          uint16_t lcid = (*lcIt).first;
                          uint32_t rlcOverhead;
                          if (lcid == 1)
                            {
                              // for SRB1 (using RLC AM) it's better to
                              // overestimate RLC overhead rather than
                              // underestimate it and risk unneeded
                              // segmentation which increases delay
                              rlcOverhead = 4;
                            }
                          else
                            {
                              // minimum RLC overhead due to header
                              rlcOverhead = 2;
                            }
                          NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc << ", RLC overhead " << rlcOverhead);
                          MacSubheader subheader ((*lcIt).first, bytesForThisLc);
                          (*lcIt).second.macSapUser->NotifyTxOpportunity (LteMacSapUser::TxOpportunityParameters ( (bytesForThisLc - subheader.GetSize () - 1), 0, dciInfoElem->m_harqProcess, GetBwpId (), m_rnti, (*lcIt).first)); //zml add 1 byte overhead
                          if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                            {
                              (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                            }
                          else
                            {
                              (*itBsr).second.txQueueSize = 0;
                            }
                        }
                    }
                  else
                    {
                      NS_LOG_WARN ("TxOpportunity of " << bytesForThisLc << " ignored");
                    }
                  NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues " << (uint32_t)(*lcIt).first << " statusQueue " << (*itBsr).second.statusPduSize << " retxQueue" << (*itBsr).second.retxQueueSize << " txQueue" <<  (*itBsr).second.txQueueSize);
                }
            }
        }
    }
  else if (dciInfoElem->m_ndi == 0)
    {
      // HARQ retransmission -> retrieve data from HARQ buffer
      NS_LOG_DEBUG ("UE MAC RETX HARQ " << (unsigned)dciInfoElem->m_harqProcess);
      Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (dciInfoElem->m_harqProcess).m_pktBurst;
      for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
        {
          Ptr<Packet> pkt = (*j)->Copy ();
          // update packet tag
          MmWaveMacPduTag tag;
          if (!pkt->RemovePacketTag (tag))
            {
              NS_FATAL_ERROR ("No MAC PDU tag");
            }
          LteRadioBearerTag bearerTag;
          if (!pkt->PeekPacketTag (bearerTag))
            {
              NS_FATAL_ERROR ("No radio bearer tag");
            }

          tag.SetSfn (dataSfn);
          pkt->AddPacketTag (tag);
          m_phySapProvider->SendMacPdu (pkt);
        }
      m_miUlHarqProcessesPacketTimer.at (dciInfoElem->m_harqProcess) = GetNumHarqProcess();
    }

  // After a DCI UL, if I have data in the buffer, I can report a BSR
  if (GetTotalBufSize () > 0)
    {
      NS_LOG_INFO ("BSR_SENT, bufSize " << GetTotalBufSize ());
      SendReportBufferStatus ();
    }
  else
    {
      m_srState = INACTIVE;
      NS_LOG_INFO ("ACTIVE -> INACTIVE, bufSize " << GetTotalBufSize ());
    }
}

void
MmWaveUeMac::DoReceiveControlMessage  (Ptr<MmWaveControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  switch (msg->GetMessageType ())
    {
    case (MmWaveControlMessage::UL_DCI):
      {
        ProcessUlDci (DynamicCast<MmWaveUlDciMessage> (msg));
        break;
      }
    case (MmWaveControlMessage::RAR):
      {
        NS_LOG_INFO ("Received RAR in slot " << m_currentSlot);

        m_macRxedCtrlMsgsTrace (m_currentSlot, m_rnti, GetBwpId (), msg);

        if (m_waitingForRaResponse == true)
          {
            Ptr<MmWaveRarMessage> rarMsg = DynamicCast<MmWaveRarMessage> (msg);
            NS_LOG_LOGIC ("got RAR with RA-RNTI " << +rarMsg->GetRaRnti () <<
                          ", expecting " << +m_raRnti);
            for (auto it = rarMsg->RarListBegin (); it != rarMsg->RarListEnd (); ++it)
              {
                if (it->rapId == m_raPreambleId)
                  {
                    RecvRaResponse (it->rarPayload);
                  }
              }
          }
        break;
      }

    default:
      NS_LOG_LOGIC ("Control message not supported/expected");
    }
}

MmWaveUePhySapUser*
MmWaveUeMac::GetPhySapUser ()
{
  return m_phySapUser;
}

void
MmWaveUeMac::SetPhySapProvider (MmWavePhySapProvider* ptr)
{
  m_phySapProvider = ptr;
}

void
MmWaveUeMac::DoConfigureRach (LteUeCmacSapProvider::RachConfig rc)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (rc);
}

void
MmWaveUeMac::DoStartContentionBasedRandomAccessProcedure ()
{
  NS_LOG_FUNCTION (this);
  RandomlySelectAndSendRaPreamble ();
}

void
MmWaveUeMac::RandomlySelectAndSendRaPreamble ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (m_currentSlot << " Received System Information, send to PHY the RA preamble");
  SendRaPreamble (true);
}

void
MmWaveUeMac::SendRaPreamble (bool contention)
{
  NS_LOG_INFO (this);
  NS_UNUSED (contention);
  //m_raPreambleId = m_raPreambleUniformVariable->GetInteger (0, 64 - 1);
  m_raPreambleId = g_raPreambleId++;
  /*raRnti should be subframeNo -1 */
  m_raRnti = 1;

  Ptr<MmWaveRachPreambleMessage> rachMsg = Create<MmWaveRachPreambleMessage> ();
  rachMsg->SetMessageType (MmWaveControlMessage::RACH_PREAMBLE);
  rachMsg->SetSourceBwp (GetBwpId ());
  m_macTxedCtrlMsgsTrace (m_currentSlot, m_rnti, GetBwpId (), rachMsg);

  m_phySapProvider->SendRachPreamble (m_raPreambleId, m_raRnti);
}

void
MmWaveUeMac::DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask)
{
  NS_LOG_FUNCTION (this << " rnti" << rnti);
  NS_UNUSED (preambleId);
  NS_ASSERT_MSG (prachMask == 0, "requested PRACH MASK = " << (uint32_t) prachMask << ", but only PRACH MASK = 0 is supported");
  m_rnti = rnti;
}

void
MmWaveUeMac::AddLc (uint8_t lcId,  LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this << " lcId" << (uint32_t) lcId);
  NS_ASSERT_MSG (m_lcInfoMap.find (lcId) == m_lcInfoMap.end (), "cannot add channel because LCID " << lcId << " is already present");

  LcInfo lcInfo;
  lcInfo.lcConfig = lcConfig;
  lcInfo.macSapUser = msu;
  m_lcInfoMap[lcId] = lcInfo;
}

void
MmWaveUeMac::DoRemoveLc (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << " lcId" << lcId);
}

LteMacSapProvider*
MmWaveUeMac::GetUeMacSapProvider (void)
{
  return m_macSapProvider;
}

void
MmWaveUeMac::DoReset ()
{
  NS_LOG_FUNCTION (this);
}
//////////////////////////////////////////////


}
