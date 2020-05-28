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

#include "nr-ue-mac.h"
#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/random-variable-stream.h>
#include "nr-phy-sap.h"
#include "nr-control-messages.h"
#include "nr-mac-header-vs.h"
#include "nr-mac-short-bsr-ce.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrUeMac");
NS_OBJECT_ENSURE_REGISTERED (NrUeMac);

uint8_t NrUeMac::g_raPreambleId = 0;

///////////////////////////////////////////////////////////
// SAP forwarders
///////////////////////////////////////////////////////////


class UeMemberNrUeCmacSapProvider : public LteUeCmacSapProvider
{
public:
  UeMemberNrUeCmacSapProvider (NrUeMac* mac);

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
  NrUeMac* m_mac;
};


UeMemberNrUeCmacSapProvider::UeMemberNrUeCmacSapProvider (NrUeMac* mac)
  : m_mac (mac)
{
}

void
UeMemberNrUeCmacSapProvider::ConfigureRach (RachConfig rc)
{
  m_mac->DoConfigureRach (rc);
}

void
UeMemberNrUeCmacSapProvider::StartContentionBasedRandomAccessProcedure ()
{
  m_mac->DoStartContentionBasedRandomAccessProcedure ();
}

void
UeMemberNrUeCmacSapProvider::StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask)
{
  m_mac->DoStartNonContentionBasedRandomAccessProcedure (rnti, preambleId, prachMask);
}


void
UeMemberNrUeCmacSapProvider::AddLc (uint8_t lcId, LogicalChannelConfig lcConfig, LteMacSapUser* msu)
{
  m_mac->AddLc (lcId, lcConfig, msu);
}

void
UeMemberNrUeCmacSapProvider::RemoveLc (uint8_t lcid)
{
  m_mac->DoRemoveLc (lcid);
}

void
UeMemberNrUeCmacSapProvider::Reset ()
{
  m_mac->DoReset ();
}

void
UeMemberNrUeCmacSapProvider::SetRnti (uint16_t rnti)
{
  m_mac->SetRnti (rnti);
}

void
UeMemberNrUeCmacSapProvider::NotifyConnectionSuccessful ()
{
  m_mac->DoNotifyConnectionSuccessful ();
}

void
UeMemberNrUeCmacSapProvider::SetImsi (uint64_t imsi)
 {
   m_mac->DoSetImsi (imsi);
 }

class UeMemberNrMacSapProvider : public LteMacSapProvider
{
public:
  UeMemberNrMacSapProvider (NrUeMac* mac);

  // inherited from LteMacSapProvider
  virtual void TransmitPdu (TransmitPduParameters params);
  virtual void ReportBufferStatus (ReportBufferStatusParameters params);

private:
  NrUeMac* m_mac;
};


UeMemberNrMacSapProvider::UeMemberNrMacSapProvider (NrUeMac* mac)
  : m_mac (mac)
{
}

void
UeMemberNrMacSapProvider::TransmitPdu (TransmitPduParameters params)
{
  m_mac->DoTransmitPdu (params);
}


void
UeMemberNrMacSapProvider::ReportBufferStatus (ReportBufferStatusParameters params)
{
  m_mac->DoReportBufferStatus (params);
}


class NrUePhySapUser;

class MacUeMemberPhySapUser : public NrUePhySapUser
{
public:
  MacUeMemberPhySapUser (NrUeMac* mac);

  virtual void ReceivePhyPdu (Ptr<Packet> p) override;

  virtual void ReceiveControlMessage (Ptr<NrControlMessage> msg) override;

  virtual void SlotIndication (SfnSf sfn) override;

  //virtual void NotifyHarqDeliveryFailure (uint8_t harqId);

  virtual uint8_t GetNumHarqProcess () const override;

private:
  NrUeMac* m_mac;
};

MacUeMemberPhySapUser::MacUeMemberPhySapUser (NrUeMac* mac)
  : m_mac (mac)
{

}
void
MacUeMemberPhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
MacUeMemberPhySapUser::ReceiveControlMessage (Ptr<NrControlMessage> msg)
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
NrUeMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUeMac")
    .SetParent<Object> ()
    .AddConstructor<NrUeMac> ()
    .AddAttribute ("NumHarqProcess",
                   "Number of concurrent stop-and-wait Hybrid ARQ processes per user",
                    UintegerValue (20),
                    MakeUintegerAccessor (&NrUeMac::SetNumHarqProcess,
                                          &NrUeMac::GetNumHarqProcess),
                    MakeUintegerChecker<uint8_t> ())
    .AddTraceSource ("UeMacRxedCtrlMsgsTrace",
                     "Ue MAC Control Messages Traces.",
                     MakeTraceSourceAccessor (&NrUeMac::m_macRxedCtrlMsgsTrace),
                     "ns3::NrMacRxTrace::RxedUeMacCtrlMsgsTracedCallback")
    .AddTraceSource ("UeMacTxedCtrlMsgsTrace",
                     "Ue MAC Control Messages Traces.",
                     MakeTraceSourceAccessor (&NrUeMac::m_macTxedCtrlMsgsTrace),
                     "ns3::NrMacRxTrace::TxedUeMacCtrlMsgsTracedCallback")
  ;
  return tid;
}

NrUeMac::NrUeMac (void) : Object ()
{
  NS_LOG_FUNCTION (this);
  m_cmacSapProvider = new UeMemberNrUeCmacSapProvider (this);
  m_macSapProvider = new UeMemberNrMacSapProvider (this);
  m_phySapUser = new MacUeMemberPhySapUser (this);
  m_raPreambleUniformVariable = CreateObject<UniformRandomVariable> ();
}

NrUeMac::~NrUeMac (void)
{
  NS_LOG_FUNCTION (this);
}

void
NrUeMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_miUlHarqProcessesPacket.clear ();
  m_miUlHarqProcessesPacketTimer.clear ();
  m_ulBsrReceived.clear ();
  m_lcInfoMap.clear ();
  m_macPduMap.clear ();
  m_raPreambleUniformVariable = nullptr;
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_phySapUser;
}

void
NrUeMac::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  m_rnti = rnti;
}

void
NrUeMac::DoNotifyConnectionSuccessful ()
{
  NS_LOG_FUNCTION (this);
  m_phySapProvider->NotifyConnectionSuccessful ();
}

void
NrUeMac::DoSetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this);
  m_imsi = imsi;
}

uint16_t
NrUeMac::GetBwpId () const
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
NrUeMac::GetCellId () const
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
NrUeMac::GetTotalBufSize () const
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
NrUeMac::SetNumHarqProcess (uint8_t numHarqProcess)
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
NrUeMac::GetNumHarqProcess () const
{
  return m_numHarqProcess;
}

// forwarded from MAC SAP
void
NrUeMac::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this);

  std::unordered_map<uint32_t, NrMacPduInfo>::iterator it = m_macPduMap.find (params.harqProcessId);
  GetSecond GetNrMacPduInfo;
  if (it == m_macPduMap.end ())
    {
      NS_FATAL_ERROR ("No MAC PDU storage element found for this TB UID/RNTI");
    }

  m_miUlHarqProcessesPacket.at (params.harqProcessId).m_lcidList.push_back (params.lcid);

  NrMacHeaderVs header;
  header.SetLcId (params.lcid);
  header.SetSize (params.pdu->GetSize ());

  uint32_t realTbSize = params.pdu->GetSize () + header.GetSerializedSize ();
  if (GetNrMacPduInfo (*it).m_dci->m_tbSize < realTbSize)
    {
      NS_FATAL_ERROR ("Maximum TB size exceeded");
    }

  params.pdu->AddHeader (header);

  LteRadioBearerTag bearerTag (params.rnti, params.lcid, 0);
  params.pdu->AddPacketTag (bearerTag);

  m_miUlHarqProcessesPacket.at (params.harqProcessId).m_pktBurst->AddPacket (params.pdu);
  m_miUlHarqProcessesPacketTimer.at (params.harqProcessId) = GetNumHarqProcess();


  m_phySapProvider->SendMacPdu (params.pdu, GetNrMacPduInfo (*it).m_sfnSf,
                                GetNrMacPduInfo (*it).m_dci->m_symStart);
  m_macPduMap.erase (it);
}

void
NrUeMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
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
NrUeMac::SendReportBufferStatus (const SfnSf &dataSfn, uint8_t symStart)
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
  std::unordered_map <uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  std::vector<uint32_t> queue (4, 0);   // one value per each of the 4 LCGs, initialized to 0
  for (it = m_ulBsrReceived.begin (); it != m_ulBsrReceived.end (); it++)
    {
      uint8_t lcid = it->first;
      std::unordered_map <uint8_t, LcInfo>::iterator lcInfoMapIt;
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
  bsr.m_macCeValue.m_bufferStatus.push_back (NrMacShortBsrCe::FromBytesToLevel (queue.at (0)));
  bsr.m_macCeValue.m_bufferStatus.push_back (NrMacShortBsrCe::FromBytesToLevel (queue.at (1)));
  bsr.m_macCeValue.m_bufferStatus.push_back (NrMacShortBsrCe::FromBytesToLevel (queue.at (2)));
  bsr.m_macCeValue.m_bufferStatus.push_back (NrMacShortBsrCe::FromBytesToLevel (queue.at (3)));

  // create the message. It is used only for tracing, but we don't send it...
  Ptr<NrBsrMessage> msg = Create<NrBsrMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetBsr (bsr);

  m_macTxedCtrlMsgsTrace (m_currentSlot, GetCellId (), bsr.m_rnti, GetBwpId (), msg);

  // Here we send the real SHORT_BSR, as a subpdu.
  Ptr<Packet> p = Create<Packet> ();

  NrMacShortBsrCe header;
  header.m_bufferSizeLevel_0 = NrMacShortBsrCe::FromBytesToLevel (queue.at (0));
  header.m_bufferSizeLevel_1 = NrMacShortBsrCe::FromBytesToLevel (queue.at (1));
  header.m_bufferSizeLevel_2 = NrMacShortBsrCe::FromBytesToLevel (queue.at (2));
  header.m_bufferSizeLevel_3 = NrMacShortBsrCe::FromBytesToLevel (queue.at (3));

  p->AddHeader (header);

  LteRadioBearerTag bearerTag (m_rnti, NrMacHeaderFsUl::SHORT_BSR, 0);
  p->AddPacketTag (bearerTag);

  m_phySapProvider->SendMacPdu (p, dataSfn, symStart);
}

void
NrUeMac::SetUeCmacSapUser (LteUeCmacSapUser* s)
{
  m_cmacSapUser = s;
}

LteUeCmacSapProvider*
NrUeMac::GetUeCmacSapProvider (void)
{
  return m_cmacSapProvider;
}

void
NrUeMac::RefreshHarqProcessesPacketBuffer (void)
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
NrUeMac::DoSlotIndication (const SfnSf &sfn)
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
NrUeMac::SendSR () const
{
  NS_LOG_FUNCTION (this);

  if (m_rnti == 0)
    {
      NS_LOG_INFO ("MAC not initialized, SR deferred");
      return;
    }

  // create the SR to send to the gNB
  Ptr<NrSRMessage> msg = Create<NrSRMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetRNTI (m_rnti);

  m_macTxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), msg);
  m_phySapProvider->SendControlMessage (msg);
}

void
NrUeMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);

  LteRadioBearerTag tag;
  p->RemovePacketTag (tag);

  if (tag.GetRnti() != m_rnti) // Packet is for another user
    {
      return;
    }

  NrMacHeaderVs header;
  p->RemoveHeader (header);

  LteMacSapUser::ReceivePduParameters rxParams;
  rxParams.p = p;
  rxParams.rnti = m_rnti;
  rxParams.lcid = header.GetLcId ();

  auto it = m_lcInfoMap.find (header.GetLcId());

  if (rxParams.p->GetSize () > 0)
    {
      it->second.macSapUser->ReceivePdu (rxParams);
    }
}

void
NrUeMac::RecvRaResponse (BuildRarListElement_s raResponse)
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = false;
  m_rnti = raResponse.m_rnti;
  m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
  m_cmacSapUser->NotifyRandomAccessSuccessful ();
}

std::unordered_map<uint32_t, struct NrMacPduInfo>::iterator
NrUeMac::AddToMacPduMap (const std::shared_ptr<DciInfoElementTdma> &dci,
                         const SfnSf &ulSfn)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Adding PDU at the position " << ulSfn);

  NrMacPduInfo macPduInfo (ulSfn, dci);
  std::unordered_map<uint32_t, struct NrMacPduInfo>::iterator it = m_macPduMap.find (dci->m_harqProcess);

  if (it != m_macPduMap.end ())
    {
      m_macPduMap.erase (it);
    }
  it = (m_macPduMap.insert (std::make_pair (dci->m_harqProcess, macPduInfo))).first;
  return it;
}

void
NrUeMac::ProcessUlDci (const Ptr<NrUlDciMessage> &dciMsg)
{
  SfnSf dataSfn = m_currentSlot;
  dataSfn.Add (dciMsg->GetKDelay ());

  auto dciInfoElem = dciMsg->GetDciInfoElement ();

  m_macRxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), dciMsg);

  NS_LOG_INFO ("UL DCI received, transmit data in slot " << dataSfn <<
               " TBS " << dciInfoElem->m_tbSize << " total queue " << GetTotalBufSize ());
  if (dciInfoElem->m_ndi == 1)
    {
      // New transmission -> empty pkt buffer queue (for deleting eventual pkts not acked )
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      m_miUlHarqProcessesPacket.at (dciInfoElem->m_harqProcess).m_pktBurst = pb;
      m_miUlHarqProcessesPacket.at (dciInfoElem->m_harqProcess).m_lcidList.clear ();
      // Retrieve data from RLC
      std::unordered_map <uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
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

      AddToMacPduMap (dciInfoElem, dataSfn);

      if (activeLcs == 0)
        {
          NS_LOG_WARN ("No active flows for this UL-DCI");
          // the UE may have been scheduled when it has no buffered data due to BSR quantization, send empty packet
          LteMacSapProvider::TransmitPduParameters txParams;

          txParams.pdu = Create<Packet> ();
          txParams.lcid = 3;
          txParams.rnti = m_rnti;
          txParams.layer = 0;
          txParams.harqProcessId = dciInfoElem->m_harqProcess;
          txParams.componentCarrierId = GetBwpId ();

          DoTransmitPdu (txParams);
          return;
        }

      std::unordered_map <uint8_t, LcInfo>::iterator lcIt;
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
                          (*lcIt).second.macSapUser->NotifyTxOpportunity ( LteMacSapUser::TxOpportunityParameters ((bytesForThisLc - 4 - 1), 0, dciInfoElem->m_harqProcess, GetBwpId (), m_rnti, (*lcIt).first)); // overhead: 5 bytes
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
                          (*lcIt).second.macSapUser->NotifyTxOpportunity (LteMacSapUser::TxOpportunityParameters ( (bytesForThisLc - 4 - 1), 0, dciInfoElem->m_harqProcess, GetBwpId (), m_rnti, (*lcIt).first)); // overhead: 5 bytes
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
          LteRadioBearerTag bearerTag;
          if (!pkt->PeekPacketTag (bearerTag))
            {
              NS_FATAL_ERROR ("No radio bearer tag");
            }
          m_phySapProvider->SendMacPdu (pkt, dataSfn, dciInfoElem->m_symStart);
        }
      m_miUlHarqProcessesPacketTimer.at (dciInfoElem->m_harqProcess) = GetNumHarqProcess();
    }

  // After a DCI UL, if I have data in the buffer, I can report a BSR
  if (GetTotalBufSize () > 0)
    {
      NS_LOG_INFO ("BSR_SENT, bufSize " << GetTotalBufSize ());
      SendReportBufferStatus (dataSfn, dciInfoElem->m_symStart);
    }
  else
    {
      m_srState = INACTIVE;
      NS_LOG_INFO ("ACTIVE -> INACTIVE, bufSize " << GetTotalBufSize ());
    }
}

void
NrUeMac::DoReceiveControlMessage  (Ptr<NrControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  switch (msg->GetMessageType ())
    {
    case (NrControlMessage::UL_DCI):
      {
        ProcessUlDci (DynamicCast<NrUlDciMessage> (msg));
        break;
      }
    case (NrControlMessage::RAR):
      {
        NS_LOG_INFO ("Received RAR in slot " << m_currentSlot);

        m_macRxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), msg);

        if (m_waitingForRaResponse == true)
          {
            Ptr<NrRarMessage> rarMsg = DynamicCast<NrRarMessage> (msg);
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

NrUePhySapUser*
NrUeMac::GetPhySapUser ()
{
  return m_phySapUser;
}

void
NrUeMac::SetPhySapProvider (NrPhySapProvider* ptr)
{
  m_phySapProvider = ptr;
}

void
NrUeMac::DoConfigureRach (LteUeCmacSapProvider::RachConfig rc)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (rc);
}

void
NrUeMac::DoStartContentionBasedRandomAccessProcedure ()
{
  NS_LOG_FUNCTION (this);
  RandomlySelectAndSendRaPreamble ();
}

void
NrUeMac::RandomlySelectAndSendRaPreamble ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (m_currentSlot << " Received System Information, send to PHY the RA preamble");
  SendRaPreamble (true);
}

void
NrUeMac::SendRaPreamble (bool contention)
{
  NS_LOG_INFO (this);
  NS_UNUSED (contention);
  //m_raPreambleId = m_raPreambleUniformVariable->GetInteger (0, 64 - 1);
  m_raPreambleId = g_raPreambleId++;
  /*raRnti should be subframeNo -1 */
  m_raRnti = 1;

  Ptr<NrRachPreambleMessage> rachMsg = Create<NrRachPreambleMessage> ();
  rachMsg->SetSourceBwp (GetBwpId ());
  m_macTxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), rachMsg);

  m_phySapProvider->SendRachPreamble (m_raPreambleId, m_raRnti);
}

void
NrUeMac::DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask)
{
  NS_LOG_FUNCTION (this << " rnti" << rnti);
  NS_UNUSED (preambleId);
  NS_ASSERT_MSG (prachMask == 0, "requested PRACH MASK = " << (uint32_t) prachMask << ", but only PRACH MASK = 0 is supported");
  m_rnti = rnti;
}

void
NrUeMac::AddLc (uint8_t lcId,  LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this << " lcId" << (uint32_t) lcId);
  NS_ASSERT_MSG (m_lcInfoMap.find (lcId) == m_lcInfoMap.end (), "cannot add channel because LCID " << lcId << " is already present");

  LcInfo lcInfo;
  lcInfo.lcConfig = lcConfig;
  lcInfo.macSapUser = msu;
  m_lcInfoMap[lcId] = lcInfo;
}

void
NrUeMac::DoRemoveLc (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << " lcId" << lcId);
}

LteMacSapProvider*
NrUeMac::GetUeMacSapProvider (void)
{
  return m_macSapProvider;
}

void
NrUeMac::DoReset ()
{
  NS_LOG_FUNCTION (this);
}
//////////////////////////////////////////////


}
