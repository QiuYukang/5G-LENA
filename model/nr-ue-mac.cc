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
                << GetBwpId () << ", rnti " << m_rnti << "] ";           \
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
#include "nr-sl-ue-mac-csched-sap.h"
#include "nr-sl-ue-mac-harq.h"
#include "nr-sl-sci-f01-header.h"
#include "nr-sl-sci-f02-header.h"
#include "nr-sl-mac-pdu-tag.h"
#include <algorithm>
#include <bitset>

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


class MemberNrSlUeMacSchedSapUser : public NrSlUeMacSchedSapUser
{

public:
  MemberNrSlUeMacSchedSapUser (NrUeMac* mac);

  virtual void SchedUeNrSlConfigInd (const NrSlSlotAlloc& params);
  virtual uint8_t GetTotalSubCh () const;

private:
  NrUeMac* m_mac;
};

MemberNrSlUeMacSchedSapUser::MemberNrSlUeMacSchedSapUser (NrUeMac* mac)
:m_mac (mac)
{
}
void
MemberNrSlUeMacSchedSapUser::SchedUeNrSlConfigInd (const NrSlSlotAlloc& params)
{
  m_mac->DoSchedUeNrSlConfigInd (params);
}

uint8_t
MemberNrSlUeMacSchedSapUser::GetTotalSubCh () const
{
  return m_mac->DoGetTotalSubCh ();
}

class MemberNrSlUeMacCschedSapUser : public NrSlUeMacCschedSapUser
{

public:
  MemberNrSlUeMacCschedSapUser (NrUeMac* mac);


private:
  NrUeMac* m_mac;
};

MemberNrSlUeMacCschedSapUser::MemberNrSlUeMacCschedSapUser (NrUeMac* mac)
:m_mac (mac)
{
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
    .AddAttribute ("EnableSensing",
                   "Flag to enable NR Sidelink resource selection based on sensing; otherwise, use random selection",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrUeMac::EnableSensing),
                   MakeBooleanChecker ())
    .AddAttribute ("Tproc0",
                   "t_proc0 in slots",
                   UintegerValue (1),
                   MakeUintegerAccessor (&NrUeMac::SetTproc0,
                                         &NrUeMac::GetTproc0),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("T1",
                   "The offset in number of slots between the slot in which the"
                   "resource selection is triggered and the start of the selection window",
                   UintegerValue (2),
                   MakeUintegerAccessor (&NrUeMac::SetT1,
                                         &NrUeMac::GetT1),
                                         MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("T2",
                   "The offset in number of slots between the slot in which the"
                   "resource selection is triggered and the end of the selection window",
                   UintegerValue (32),
                   MakeUintegerAccessor (&NrUeMac::SetT2,
                                         &NrUeMac::GetT2),
                                         MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ActivePoolId",
                   "The pool id of the active pool used for TX and RX",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrUeMac::SetSlActivePoolId,
                                         &NrUeMac::GetSlActivePoolId),
                                         MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ReservationPeriod",
                   "Resource Reservation Interval for NR Sidelink in ms"
                   "Must be among the values included in LteRrcSap::SlResourceReservePeriod",
                   TimeValue(MilliSeconds(100)),
                   MakeTimeAccessor (&NrUeMac::SetReservationPeriod,
                                     &NrUeMac::GetReservationPeriod),
                                     MakeTimeChecker ())
     .AddAttribute ("NumSidelinkProcess",
                    "Number of concurrent stop-and-wait Sidelink processes per destination",
                    UintegerValue (4),
                    MakeUintegerAccessor (&NrUeMac::SetNumSidelinkProcess,
                                          &NrUeMac::GetNumSidelinkProcess),
                    MakeUintegerChecker<uint8_t> ())
     .AddAttribute ("EnableBlindReTx",
                    "Flag to enable NR Sidelink blind retransmissions",
                    BooleanValue (true),
                    MakeBooleanAccessor (&NrUeMac::EnableBlindReTx),
                    MakeBooleanChecker ())
     .AddTraceSource ("SlPscchScheduling",
                      "Information regarding NR SL PSCCH UE scheduling",
                      MakeTraceSourceAccessor (&NrUeMac::m_slPscchScheduling),
                      "ns3::SlUeMacStatParameters::TracedCallback")
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
  //NR SL
  m_nrSlMacSapProvider = new MemberNrSlMacSapProvider <NrUeMac> (this);
  m_nrSlUeCmacSapProvider = new MemberNrSlUeCmacSapProvider<NrUeMac> (this);
  m_nrSlUePhySapUser = new MemberNrSlUePhySapUser<NrUeMac> (this);
  m_nrSlUeMacCschedSapUser = new MemberNrSlUeMacCschedSapUser (this);
  m_nrSlUeMacSchedSapUser = new MemberNrSlUeMacSchedSapUser (this);
  m_ueSelectedUniformVariable = CreateObject<UniformRandomVariable> ();
  m_nrSlHarq = CreateObject<NrSlUeMacHarq> ();
}

NrUeMac::~NrUeMac (void)
{
}

void
NrUeMac::DoDispose ()
{
  m_miUlHarqProcessesPacket.clear ();
  m_miUlHarqProcessesPacketTimer.clear ();
  m_ulBsrReceived.clear ();
  m_lcInfoMap.clear ();
  m_raPreambleUniformVariable = nullptr;
  m_ueSelectedUniformVariable = nullptr;
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_phySapUser;
  delete m_nrSlMacSapProvider;
  delete m_nrSlUeCmacSapProvider;
  delete m_nrSlUePhySapUser;
  delete m_nrSlUeMacCschedSapUser;
  delete m_nrSlUeMacSchedSapUser;
  m_nrSlHarq->Dispose ();
  m_nrSlHarq = nullptr;
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
  NS_ASSERT (m_ulDci->m_harqProcess == params.harqProcessId);

  m_miUlHarqProcessesPacket.at (params.harqProcessId).m_lcidList.push_back (params.lcid);

  NrMacHeaderVs header;
  header.SetLcId (params.lcid);
  header.SetSize (params.pdu->GetSize ());

  params.pdu->AddHeader (header);

  LteRadioBearerTag bearerTag (params.rnti, params.lcid, 0);
  params.pdu->AddPacketTag (bearerTag);

  m_miUlHarqProcessesPacket.at (params.harqProcessId).m_pktBurst->AddPacket (params.pdu);
  m_miUlHarqProcessesPacketTimer.at (params.harqProcessId) = GetNumHarqProcess();

  m_ulDciTotalUsed += params.pdu->GetSize ();

  NS_ASSERT_MSG (m_ulDciTotalUsed <= m_ulDci->m_tbSize, "We used more data than the DCI allowed us.");

  m_phySapProvider->SendMacPdu (params.pdu, m_ulDciSfnsf, m_ulDci->m_symStart);
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

  // Please note that the levels are defined from the standard. In this case,
  // we have 5 bit available, so use such standard levels. In the future,
  // when LONG BSR will be implemented, this have to change.
  NrMacShortBsrCe header;
  header.m_bufferSizeLevel_0 = NrMacShortBsrCe::FromBytesToLevel (queue.at (0));
  header.m_bufferSizeLevel_1 = NrMacShortBsrCe::FromBytesToLevel (queue.at (1));
  header.m_bufferSizeLevel_2 = NrMacShortBsrCe::FromBytesToLevel (queue.at (2));
  header.m_bufferSizeLevel_3 = NrMacShortBsrCe::FromBytesToLevel (queue.at (3));

  p->AddHeader (header);

  LteRadioBearerTag bearerTag (m_rnti, NrMacHeaderFsUl::SHORT_BSR, 0);
  p->AddPacketTag (bearerTag);

  m_ulDciTotalUsed += p->GetSize ();
  NS_ASSERT_MSG (m_ulDciTotalUsed <= m_ulDci->m_tbSize, "We used more data than the DCI allowed us.");

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
      if (m_miUlHarqProcessesPacketTimer.at (i) == 0 && m_miUlHarqProcessesPacket.at (i).m_pktBurst)
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

  if (m_nrSlUeCmacSapUser != nullptr)
    {
      std::vector <std::bitset<1>> phyPool = m_slTxPool->GetNrSlPhyPool (GetBwpId (), m_poolId);
      uint64_t absSlotIndex = sfn.Normalize ();
      uint16_t absPoolIndex = absSlotIndex % phyPool.size ();
      //trigger SL only when it is a SL slot
      if (phyPool [absPoolIndex] == 1)
        {
          DoNrSlSlotIndication (sfn);
        }
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

  // p can be empty. Well, right now no, but when someone will add CE in downlink,
  // then p can be empty.
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

void
NrUeMac::ProcessUlDci (const Ptr<NrUlDciMessage> &dciMsg)
{
  NS_LOG_FUNCTION (this);

  SfnSf dataSfn = m_currentSlot;
  dataSfn.Add (dciMsg->GetKDelay ());

  // Saving the data we need in DoTransmitPdu
  m_ulDciSfnsf = dataSfn;
  m_ulDciTotalUsed = 0;
  m_ulDci = dciMsg->GetDciInfoElement ();

  m_macRxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), dciMsg);

  NS_LOG_INFO ("UL DCI received, transmit data in slot " << dataSfn <<
               " Harq Process " << +m_ulDci->m_harqProcess <<
               " TBS " << m_ulDci->m_tbSize << " total queue " << GetTotalBufSize ());

  if (m_ulDci->m_ndi == 0)
    {
      // This method will retransmit the data saved in the harq buffer
      TransmitRetx ();

      // This method will transmit a new BSR.
      SendReportBufferStatus (dataSfn, m_ulDci->m_symStart);
    }
  else if (m_ulDci->m_ndi == 1)
    {
      SendNewData ();

      NS_LOG_INFO ("After sending NewData, bufSize " << GetTotalBufSize ());

      // Send a new BSR. SendNewData() already took into account the size of
      // the BSR.
      SendReportBufferStatus (dataSfn, m_ulDci->m_symStart);

      NS_LOG_INFO ("UL DCI processing done, sent to PHY a total of " << m_ulDciTotalUsed <<
                   " B out of " << m_ulDci->m_tbSize << " allocated bytes ");

      if (GetTotalBufSize () == 0)
        {
          m_srState = INACTIVE;
          NS_LOG_INFO ("ACTIVE -> INACTIVE, bufSize " << GetTotalBufSize ());

          // the UE may have been scheduled, but we didn't use a single byte
          // of the allocation. So send an empty PDU. This happens because the
          // byte reporting in the BSR is not accurate, due to RLC and/or
          // BSR quantization.
          if (m_ulDciTotalUsed == 0)
            {
              NS_LOG_WARN ("No byte used for this UL-DCI, sending empty PDU");

              LteMacSapProvider::TransmitPduParameters txParams;

              txParams.pdu = Create<Packet> ();
              txParams.lcid = 3;
              txParams.rnti = m_rnti;
              txParams.layer = 0;
              txParams.harqProcessId = m_ulDci->m_harqProcess;
              txParams.componentCarrierId = GetBwpId ();

              DoTransmitPdu (txParams);
            }
        }
    }
}

void
NrUeMac::TransmitRetx ()
{
  NS_LOG_FUNCTION (this);

  Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_pktBurst;

  if (pb == nullptr)
    {
      NS_LOG_WARN ("The previous transmission did not contain any new data; "
                   "probably it was BSR only. To not send an old BSR to the scheduler, "
                   "we don't send anything back in this allocation. Eventually, "
                   "the Harq timer at gnb will expire, and soon this allocation will be forgotten.");
      return;
    }

  NS_LOG_DEBUG ("UE MAC RETX HARQ " << + m_ulDci->m_harqProcess);

  NS_ASSERT (pb->GetNPackets() > 0);

  for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
    {
      Ptr<Packet> pkt = (*j)->Copy ();
      LteRadioBearerTag bearerTag;
      if (!pkt->PeekPacketTag (bearerTag))
        {
          NS_FATAL_ERROR ("No radio bearer tag");
        }
      m_phySapProvider->SendMacPdu (pkt, m_ulDciSfnsf, m_ulDci->m_symStart);
    }

  m_miUlHarqProcessesPacketTimer.at (m_ulDci->m_harqProcess) = GetNumHarqProcess();
}

void
NrUeMac::SendRetxData (uint32_t usefulTbs, uint32_t activeLcsRetx)
{
  NS_LOG_FUNCTION (this);

  if (activeLcsRetx == 0)
    {
      return;
    }

  uint32_t bytesPerLcId = usefulTbs / activeLcsRetx;

  for (auto & itBsr : m_ulBsrReceived)
    {
      auto &bsr = itBsr.second;

      // Check if we have room to transmit the retxData
      uint32_t assignedBytes = std::min (bytesPerLcId, bsr.retxQueueSize);
      if (assignedBytes > 0 && m_ulDciTotalUsed + assignedBytes <= usefulTbs)
        {
          LteMacSapUser::TxOpportunityParameters txParams;
          txParams.lcid = bsr.lcid;
          txParams.rnti = m_rnti;
          txParams.bytes = assignedBytes;
          txParams.layer = 0;
          txParams.harqId = m_ulDci->m_harqProcess;
          txParams.componentCarrierId = GetBwpId ();

          NS_LOG_INFO ("Notifying RLC of LCID " << +bsr.lcid << " of a TxOpp "
                       "of " << assignedBytes << " B for a RETX PDU");

          m_lcInfoMap.at (bsr.lcid).macSapUser->NotifyTxOpportunity (txParams);
          // After this call, m_ulDciTotalUsed has been updated with the
          // correct amount of bytes... but it is up to us in updating the BSR
          // value, substracting the amount of bytes transmitted
          bsr.retxQueueSize -= assignedBytes;
        }
      else
        {
          NS_LOG_DEBUG ("Something wrong with the calculation of overhead."
                        "Active LCS Retx: " << activeLcsRetx << " assigned to this: " <<
                        assignedBytes << ", with TBS of " << m_ulDci->m_tbSize <<
                        " usefulTbs " << usefulTbs << " and total used " << m_ulDciTotalUsed);
        }
    }
}

void
NrUeMac::SendTxData(uint32_t usefulTbs, uint32_t activeTx)
{
  NS_LOG_FUNCTION (this);

  if (activeTx == 0)
    {
      return;
    }

  uint32_t bytesPerLcId = usefulTbs / activeTx;

  for (auto & itBsr : m_ulBsrReceived)
    {
      auto &bsr = itBsr.second;

      // Check if we have room to transmit the retxData
      uint32_t assignedBytes = std::min (bytesPerLcId, bsr.txQueueSize);
      if (assignedBytes > 0 && m_ulDciTotalUsed + assignedBytes <= usefulTbs)
        {
          LteMacSapUser::TxOpportunityParameters txParams;
          txParams.lcid = bsr.lcid;
          txParams.rnti = m_rnti;
          txParams.bytes = assignedBytes;
          txParams.layer = 0;
          txParams.harqId = m_ulDci->m_harqProcess;
          txParams.componentCarrierId = GetBwpId ();

          NS_LOG_INFO ("Notifying RLC of LCID " << +bsr.lcid << " of a TxOpp "
                       "of " << assignedBytes << " B for a TX PDU");

          m_lcInfoMap.at (bsr.lcid).macSapUser->NotifyTxOpportunity (txParams);
          // After this call, m_ulDciTotalUsed has been updated with the
          // correct amount of bytes... but it is up to us in updating the BSR
          // value, substracting the amount of bytes transmitted
          bsr.txQueueSize -= assignedBytes;
        }
      else
        {
          NS_LOG_DEBUG ("Something wrong with the calculation of overhead."
                        "Active LCS Retx: " << activeTx << " assigned to this: " <<
                        assignedBytes << ", with TBS of " << m_ulDci->m_tbSize <<
                        " usefulTbs " << usefulTbs << " and total used " << m_ulDciTotalUsed);
        }
    }
}

void
NrUeMac::SendNewData ()
{
  NS_LOG_FUNCTION (this);
  // New transmission -> empty pkt buffer queue (for deleting eventual pkts not acked )
  Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
  m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_pktBurst = pb;
  m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_lcidList.clear ();
  NS_LOG_INFO ("Reset HARQP " << +m_ulDci->m_harqProcess);

  // Sending the status data has no boundary: let's try to send the ACK as
  // soon as possible, filling the TBS, if necessary.
  SendNewStatusData ();

  // Let's count how many LC we have, that are waiting with some data
  uint16_t activeLcsRetx = 0;
  uint16_t activeLcsTx = 0;
  uint32_t totRetx = 0;
  uint32_t totTx = 0;
  for (const auto & itBsr : m_ulBsrReceived)
    {
      totRetx += itBsr.second.retxQueueSize;
      totTx += itBsr.second.txQueueSize;

      if (itBsr.second.retxQueueSize > 0)
        {
          activeLcsRetx++;
        }
      if (itBsr.second.txQueueSize > 0)
        {
          activeLcsTx++;
        }
    }

  // Of the TBS we received in the DCI, one part is gone for the status pdu,
  // where we didn't check much as it is the most important data, that has to go
  // out. For the rest that we have left, we can use only a part of it because of
  // the overhead of the SHORT_BSR, which is 5 bytes.
  NS_ASSERT_MSG (m_ulDciTotalUsed + 5 <= m_ulDci->m_tbSize,
                 "The StatusPDU used " << m_ulDciTotalUsed << " B, we don't have any for the SHORT_BSR.");
  uint32_t usefulTbs = m_ulDci->m_tbSize - m_ulDciTotalUsed - 5;

  // Now, we have 3 bytes of overhead for each subPDU. Let's try to serve all
  // the queues with some RETX data.
  if (activeLcsRetx * 3 > usefulTbs)
    {
      NS_LOG_DEBUG ("The overhead for transmitting retx data is greater than the space for transmitting it."
                    "Ignore the TBS of " << usefulTbs << " B.");
    }
  else
    {
      usefulTbs -= activeLcsRetx * 3;
      SendRetxData (usefulTbs, activeLcsRetx);
    }

  // Now we have to update our useful TBS for the next transmission.
  // Remember that m_ulDciTotalUsed keep count of data and overhead that we
  // used till now.
  NS_ASSERT_MSG (m_ulDciTotalUsed + 5 <= m_ulDci->m_tbSize,
                 "The StatusPDU sending required all space, we don't have any for the SHORT_BSR.");
  usefulTbs = m_ulDci->m_tbSize - m_ulDciTotalUsed - 5; // Update the usefulTbs.

  // The last part is for the queues with some non-RETX data. If there is no space left,
  // then nothing.
  if (activeLcsTx * 3 > usefulTbs)
    {
      NS_LOG_DEBUG ("The overhead for transmitting new data is greater than the space for transmitting it."
                    "Ignore the TBS of " << usefulTbs << " B.");
    }
  else
    {
      usefulTbs -= activeLcsTx * 3;
      SendTxData (usefulTbs, activeLcsTx);
    }

  // If we did not used the packet burst, explicitly signal it to the HARQ
  // retx, if any.
  if (m_ulDciTotalUsed == 0)
    {
      m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_pktBurst = nullptr;
      m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_lcidList.clear ();
    }
}


void
NrUeMac::SendNewStatusData()
{
  NS_LOG_FUNCTION (this);

  bool hasStatusPdu = false;
  bool sentOneStatusPdu = false;

  for (auto & bsrIt : m_ulBsrReceived)
    {
      auto & bsr = bsrIt.second;

      if (bsr.statusPduSize > 0)
        {
          hasStatusPdu = true;

          // Check if we have room to transmit the statusPdu
          if (m_ulDciTotalUsed + bsr.statusPduSize <= m_ulDci->m_tbSize)
            {
              LteMacSapUser::TxOpportunityParameters txParams;
              txParams.lcid = bsr.lcid;
              txParams.rnti = m_rnti;
              txParams.bytes = bsr.statusPduSize;
              txParams.layer = 0;
              txParams.harqId = m_ulDci->m_harqProcess;
              txParams.componentCarrierId = GetBwpId ();

              NS_LOG_INFO ("Notifying RLC of LCID " << +bsr.lcid << " of a TxOpp "
                           "of " << bsr.statusPduSize << " B for a status PDU");

              m_lcInfoMap.at (bsr.lcid).macSapUser->NotifyTxOpportunity (txParams);
              // After this call, m_ulDciTotalUsed has been updated with the
              // correct amount of bytes... but it is up to us in updating the BSR
              // value, substracting the amount of bytes transmitted
              bsr.statusPduSize = 0;
              sentOneStatusPdu = true;
            }
          else
            {
              NS_LOG_INFO ("Cannot send StatusPdu of " << bsr.statusPduSize <<
                           " B, we already used all the TBS");
            }
        }
    }

  NS_ABORT_MSG_IF (hasStatusPdu && !sentOneStatusPdu,
                   "The TBS of size " << m_ulDci->m_tbSize << " doesn't allow us "
                   "to send one status PDU...");
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

int64_t
NrUeMac::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_raPreambleUniformVariable ->SetStream (stream);
  m_ueSelectedUniformVariable->SetStream (stream + 1);
  return 2;
}
//NR SL

std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrUeMac::GetNrSlTxOpportunities (const SfnSf& sfn, uint16_t poolId)
{
  NS_LOG_FUNCTION (this << sfn.GetFrame() << +sfn.GetSubframe() << sfn.GetSlot () << poolId);

  std::list <NrSlCommResourcePool::SlotInfo> candSsResoA, candSsResoB;// S_A and S_B as per TS 38.214
  uint64_t absSlotIndex = sfn.Normalize ();
  uint8_t bwpId = GetBwpId ();
  uint16_t numerology = sfn.GetNumerology ();

  if (m_enableSensing)
    {

    }
  else
    {
      //no sensing
      candSsResoA = m_slTxPool->GetNrSlCommOpportunities (absSlotIndex, bwpId, numerology, poolId, m_t1, m_t2);
      candSsResoB = candSsResoA;
    }

  return GetNrSupportedList (sfn, candSsResoB);
}

std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrUeMac::GetNrSupportedList (const SfnSf& sfn, std::list <NrSlCommResourcePool::SlotInfo> slotInfo)
{
  NS_LOG_FUNCTION (this);
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> nrSupportedList;
  for (const auto& it:slotInfo)
    {
      NrSlUeMacSchedSapProvider::NrSlSlotInfo info (it.numSlPscchRbs, it.slPscchSymStart,
                                                  it.slPscchSymLength, it.slPsschSymStart,
                                                  it.slPsschSymLength, it.slSubchannelSize,
                                                  it.slMaxNumPerReserve,
                                                  sfn.GetFutureSfnSf (it.slotOffset));
      nrSupportedList.emplace_back (info);
    }

  return nrSupportedList;
}

void
NrUeMac::DoNrSlSlotIndication (const SfnSf& sfn)
{
  NS_LOG_FUNCTION (this << " Frame " << sfn.GetFrame() << " Subframe " << +sfn.GetSubframe()
                        << " slot " << sfn.GetSlot () << " Normalized slot number " << sfn.Normalize ());

  if (m_slTxPool->GetNrSlSchedulingType () == NrSlCommResourcePool::SCHEDULED)
    {

    }
  else if (m_slTxPool->GetNrSlSchedulingType () == NrSlCommResourcePool::UE_SELECTED)
    {
      std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> availbleReso = GetNrSlTxOpportunities (sfn, m_poolId);
      for (const auto &itDst : m_sidelinkDestinations)
        {
          const auto itGrantInfo = m_grantInfo.find (itDst.first);
          //If the re-selection counter of the found destination is not zero,
          //it means it already have resources assigned to it via semi-persistent
          //scheduling, thus, we go to the next destination.
          bool foundDest = itGrantInfo != m_grantInfo.end ();
          if (foundDest && itGrantInfo->second.slResoReselCounter != 0)
            {
              NS_LOG_INFO ("Destination " << itDst.first << " already have the allocation, scheduling the next destination, if any");
              continue;
            }
          uint32_t randProb = m_ueSelectedUniformVariable->GetInteger (0, 1);
          if (foundDest && itGrantInfo->second.cReselCounter > 0 &&
              itGrantInfo->second.slotAllocations.size () > 0 && randProb > m_slProbResourceKeep)
            {
              NS_ASSERT_MSG (itGrantInfo->second.slResoReselCounter == 0, "Sidelink resource re-selection counter must be zero before continuing with the same grant for dst " << itDst.first);
              //keeping the resource, reassign the same sidelink resource re-selection
              //counter we chose while creating the fresh grant
              itGrantInfo->second.slResoReselCounter = itGrantInfo->second.prevSlResoReselCounter;
            }
          else
            {
              auto filteredReso = FilterTxOpportunities (availbleReso);
              if (!filteredReso.empty () && filteredReso.size () >= filteredReso.begin ()->slMaxNumPerReserve)
                {
                  //we ask the scheduler for resources only if:
                  //1. The filtered list is not empty.
                  //2. The filtered list have enough slots to be allocated for all
                  //   the possible transmissions.
                  NS_LOG_INFO ("scheduling the destination " << itDst.first);
                  m_nrSlUeMacSchedSapProvider->SchedUeNrSlTriggerReq (itDst.first, filteredReso);
                }
              else
                {
                  NS_LOG_DEBUG ("Do not have enough slots to allocate. Clearing the remaining allocations if any, and not calling the scheduler for dst " << itDst.first);
                  //Also clear the previous remaining allocations
                  itGrantInfo->second.cReselCounter = 0;
                  itGrantInfo->second.prevSlResoReselCounter = 0;
                  itGrantInfo->second.slotAllocations.erase (itGrantInfo->second.slotAllocations.begin (), itGrantInfo->second.slotAllocations.end ());
                }
            }
        }
    }
  else
    {
      NS_FATAL_ERROR ("Scheduling type " << m_slTxPool->GetNrSlSchedulingType () << " for NR Sidelink pools is unknown");
    }

  //check if we need to transmit PSCCH + PSSCH
  for (auto & itGrantInfo : m_grantInfo)
    {
      if (itGrantInfo.second.slResoReselCounter != 0 && itGrantInfo.second.slotAllocations.begin ()->sfn == sfn)
        {
          //first thing first, set this allocation info in PHY
          m_nrSlUePhySapProvider->SetNrSlAllocInfo (*(itGrantInfo.second.slotAllocations.begin ()));

          auto grant = itGrantInfo.second.slotAllocations.begin ();

          //prepare and send SCI format 01 message
          NrSlSciF01Header sciF01;
          sciF01.SetPriority (grant->priority);
          sciF01.SetMcs (grant->mcs);
          sciF01.SetSlResourceReservePeriod (static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ()));
          sciF01.SetTotalSubChannels (GetTotalSubCh (m_poolId));
          sciF01.SetIndexStartSubChannel (grant->slPsschSubChStart);
          sciF01.SetLengthSubChannel (grant->slPsschSubChLength);
          sciF01.SetSlMaxNumPerReserve (grant->maxNumPerReserve);
          sciF01.SetGapReTx1 (grant->gapReTx1);
          sciF01.SetGapReTx2 (grant->gapReTx2);

          //sum all the assigned bytes to each LC of this destination
          uint32_t tbs = 0;
          for (const auto & it : grant->slRlcPduInfo)
            {
              NS_LOG_DEBUG ("LC " << static_cast <uint16_t> (it.lcid) << " was assigned " << it.size << "bytes");
              tbs += it.size;
            }
          Ptr<Packet> pktSciF01 = Create<Packet> ();
          pktSciF01->AddHeader (sciF01);
          NrSlMacPduTag tag (m_rnti, grant->sfn, grant->slPsschSymStart, grant->slPsschSymLength, tbs, grant->dstL2Id);
          pktSciF01->AddPacketTag (tag);

          m_nrSlUePhySapProvider->SendPscchMacPdu (pktSciF01);

          // Collect statistics for NR SL PSCCH UE MAC scheduling trace
          SlPscchUeMacStatParameters pscchStatsParams;
          pscchStatsParams.timestamp = Simulator::Now ().GetMilliSeconds ();
          pscchStatsParams.imsi = m_imsi;
          pscchStatsParams.rnti = m_rnti;
          pscchStatsParams.frameNum = grant->sfn.GetFrame ();
          pscchStatsParams.subframeNum = grant->sfn.GetSubframe ();
          pscchStatsParams.slotNum = grant->sfn.GetSlot ();
          pscchStatsParams.priority = grant->priority;
          pscchStatsParams.mcs = grant->mcs;
          pscchStatsParams.tbSize = tbs;
          pscchStatsParams.slResourceReservePeriod = static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ());
          pscchStatsParams.gapReTx1 = grant->gapReTx1;
          pscchStatsParams.gapReTx2 = grant->gapReTx2;
          m_slPscchScheduling (pscchStatsParams); //Trace

          //prepare and send SCI format 02 message
          NrSlSciF02Header sciF02;
          if (grant->ndi)
            {
              --itGrantInfo.second.slResoReselCounter;
              --itGrantInfo.second.cReselCounter;
              uint8_t nrSlHarqId {std::numeric_limits <uint8_t>::max ()};
              nrSlHarqId = m_nrSlHarq->AssignNrSlHarqProcessId (grant->dstL2Id);
              sciF02.SetHarqId (nrSlHarqId);
              itGrantInfo.second.nrSlHarqId = nrSlHarqId;
            }
          else
            {
              sciF02.SetHarqId (itGrantInfo.second.nrSlHarqId);
            }
          sciF02.SetNdi (grant->ndi);
          sciF02.SetRv (grant->rv);
          sciF02.SetSrcId (m_srcL2Id);
          sciF02.SetDstId (grant->dstL2Id);
          //fields which are not used yet that is why we set them to 0
          sciF02.SetCsiReq (0);
          sciF02.SetZoneId (0);
          sciF02.SetCommRange (0);
          Ptr<Packet> pktSciF02 = Create<Packet> ();
          pktSciF02->AddHeader (sciF02);
          //put SCI stage 2 in PSSCH queue
          m_nrSlUePhySapProvider->SendPsschMacPdu (pktSciF02);

          if (grant->ndi)
            {
              for (const auto & itLcRlcPduInfo : grant->slRlcPduInfo)
                {
                  SidelinkLcIdentifier slLcId;
                  slLcId.lcId = itLcRlcPduInfo.lcid;
                  slLcId.srcL2Id = m_srcL2Id;
                  slLcId.dstL2Id = grant->dstL2Id;
                  const auto & itLc = m_nrSlLcInfoMap.find (slLcId);
                  NS_ASSERT_MSG (itLc != m_nrSlLcInfoMap.end (), "No LC with id " << +itLcRlcPduInfo.lcid << " found for destination " << grant->dstL2Id);
                  NS_LOG_DEBUG ("Notifying NR SL RLC of TX opportunity for LC id " << +itLcRlcPduInfo.lcid << " for TB size " << itLcRlcPduInfo.size);
                  NS_ASSERT_MSG (itGrantInfo.second.nrSlHarqId != std::numeric_limits <uint8_t>::max (), "HARQ id was not assigned for destination " << grant->dstL2Id);
                  itLc->second.macSapUser->NotifyNrSlTxOpportunity (NrSlMacSapUser::NrSlTxOpportunityParameters (itLcRlcPduInfo.size, m_rnti, itLcRlcPduInfo.lcid,
                                                                                                                 0, itGrantInfo.second.nrSlHarqId, GetBwpId (),
                                                                                                                 m_srcL2Id, grant->dstL2Id));
                }
            }
          else
            {
              //retx from MAC HARQ buffer
              //we might want to match the LC ids in grant->slRlcPduInfo and
              //the LC ids whose packets are in the packet bust in the HARQ
              //buffer. I am not doing it at the moment as it might slow down
              //the simulation.
              Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
              if (m_enableBlindReTx)
                {
                  pb = m_nrSlHarq->GetPacketBurst (grant->dstL2Id, itGrantInfo.second.nrSlHarqId);
                  NS_ASSERT_MSG (pb->GetNPackets () > 0, "Packet burst for HARQ id " << +itGrantInfo.second.nrSlHarqId << " is empty");
                  for (const auto & itPkt : pb->GetPackets ())
                    {
                      m_nrSlUePhySapProvider->SendPsschMacPdu (itPkt);
                    }
                  if (grant->rv == grant->maxNumPerReserve - 1)
                    {
                      //generate fake feedback
                      m_nrSlHarq->RecvNrSlHarqFeedback (grant->dstL2Id, itGrantInfo.second.nrSlHarqId);
                    }
                }
              else
                {
                  //we need to have a feedback to do the retx when blind retx
                  //are not enabled.
                  NS_FATAL_ERROR ("Feedback based retransmissions are not supported");
                }
            }
        }
      else
        {
          //When there are no resources it may happen that the re-selection
          //counter of already existing destination remains zero. In this case,
          //we just go the next destination, if any.
          continue;

        }

    }

}

std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrUeMac::FilterTxOpportunities (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOppr)
{
  NS_LOG_FUNCTION (this);

  NrSlSlotAlloc dummyAlloc;

  for (const auto & itDst : m_grantInfo)
    {
      auto itTxOppr = txOppr.begin ();
      while (itTxOppr != txOppr.end ())
        {
          dummyAlloc.sfn = itTxOppr->sfn;
          auto itAlloc = itDst.second.slotAllocations.find (dummyAlloc);
          if (itAlloc != itDst.second.slotAllocations.end ())
            {
              itTxOppr = txOppr.erase (itTxOppr);
            }
          else
            {
              ++itTxOppr;
            }
        }
    }

  return txOppr;
}

void
NrUeMac::DoSchedUeNrSlConfigInd (const NrSlSlotAlloc& params)
{
  NS_LOG_FUNCTION (this);
  const auto itGrantInfo = m_grantInfo.find (params.dstL2Id);

  if (itGrantInfo == m_grantInfo.end ())
    {
      NrSlGrantInfo grant = CreateGrantInfo (params);
      m_grantInfo.emplace (std::make_pair (params.dstL2Id, grant));
    }
  else
    {
      NS_ASSERT_MSG (itGrantInfo->second.slResoReselCounter == 0, "Sidelink resource counter must be zero before assigning new grant for dst " << params.dstL2Id);
      NrSlGrantInfo grant = CreateGrantInfo (params);
      itGrantInfo->second = grant;
    }
}

NrUeMac::NrSlGrantInfo
NrUeMac::CreateGrantInfo (NrSlSlotAlloc params)
{
  NS_LOG_FUNCTION (this);
  uint8_t reselCounter = GetRndmReselectionCounter ();
  uint8_t cResel = reselCounter * 10;

  uint16_t resPeriodSlots = m_slTxPool->GetResvPeriodInSlots (GetBwpId (), m_poolId, m_pRsvpTx, m_nrSlUePhySapProvider->GetSlotPeriod ());
  NrSlGrantInfo grant;

  grant.cReselCounter = cResel;
  //save reselCounter to be used if probability of keeping the resource would
  //be higher than the configured one
  grant.prevSlResoReselCounter = reselCounter;
  grant.slResoReselCounter = reselCounter;

  for (uint8_t i = 0; i < cResel; i++)
    {
      for (uint16_t tx = 0; tx < params.maxNumPerReserve; tx++)
        {
          if (tx == 0)
            {
              NS_ASSERT_MSG (params.ndi == 1, "Scheduler forgot to set ndi flag");
              NS_ASSERT_MSG (params.rv == 0, "Scheduler forgot to set redundancy version");
              params.sfn.Add (i * resPeriodSlots);
              grant.slotAllocations.emplace (params);
            }
          if (tx == 1)
            {
              params.sfn.Add (i * resPeriodSlots + params.gapReTx1);
              params.ndi = 0;
              params.rv = 1;
              grant.slotAllocations.emplace (params);
            }
          if (tx == 2)
            {
              params.sfn.Add (i * resPeriodSlots + params.gapReTx2);
              params.ndi = 0;
              params.rv = 2;
              grant.slotAllocations.emplace (params);
            }
        }
    }

  return grant;
}



NrSlMacSapProvider*
NrUeMac::GetNrSlMacSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlMacSapProvider;
}

void
NrUeMac::SetNrSlMacSapUser (NrSlMacSapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_nrSlMacSapUser = s;
}

NrSlUeCmacSapProvider*
NrUeMac::GetNrSlUeCmacSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlUeCmacSapProvider;
}

 void
 NrUeMac::SetNrSlUeCmacSapUser (NrSlUeCmacSapUser* s)
 {
   NS_LOG_FUNCTION (this);
   m_nrSlUeCmacSapUser = s;
 }

 NrSlUePhySapUser*
 NrUeMac::GetNrSlUePhySapUser ()
 {
   NS_LOG_FUNCTION (this);
   return m_nrSlUePhySapUser;
 }

 void
 NrUeMac::SetNrSlUePhySapProvider (NrSlUePhySapProvider* s)
 {
   NS_LOG_FUNCTION (this);
   m_nrSlUePhySapProvider = s;
 }

 void
 NrUeMac::SetNrSlUeMacSchedSapProvider (NrSlUeMacSchedSapProvider* s)
 {
   NS_LOG_FUNCTION (this);
   m_nrSlUeMacSchedSapProvider = s;
 }

 NrSlUeMacSchedSapUser*
 NrUeMac::GetNrSlUeMacSchedSapUser ()
 {
   NS_LOG_FUNCTION (this);
   return m_nrSlUeMacSchedSapUser;
 }

 void
 NrUeMac::SetNrSlUeMacCschedSapProvider (NrSlUeMacCschedSapProvider* s)
 {
   NS_LOG_FUNCTION (this);
   m_nrSlUeMacCschedSapProvider = s;
 }

 NrSlUeMacCschedSapUser*
 NrUeMac::GetNrSlUeMacCschedSapUser ()
 {
   NS_LOG_FUNCTION (this);
   return m_nrSlUeMacCschedSapUser;
 }

void
NrUeMac::DoTransmitNrSlRlcPdu (const NrSlMacSapProvider::NrSlRlcPduParameters &params)
{
  NS_LOG_FUNCTION (this << +params.lcid << +params.harqProcessId);
  LteRadioBearerTag bearerTag (params.rnti, params.lcid, 0);
  params.pdu->AddPacketTag (bearerTag);
  m_nrSlHarq->AddPacket (params.dstL2Id, params.lcid, params.harqProcessId, params.pdu);
  m_nrSlUePhySapProvider->SendPsschMacPdu (params.pdu);
}

void
NrUeMac::DoReportNrSlBufferStatus (const NrSlMacSapProvider::NrSlReportBufferStatusParameters &params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Reporting for Sidelink. Tx Queue size = " << params.txQueueSize);
  //Sidelink BSR
  std::map <SidelinkLcIdentifier, NrSlMacSapProvider::NrSlReportBufferStatusParameters>::iterator it;

  SidelinkLcIdentifier slLcId;
  slLcId.lcId = params.lcid;
  slLcId.srcL2Id = params.srcL2Id;
  slLcId.dstL2Id = params.dstL2Id;

  it = m_nrSlBsrReceived.find (slLcId);
  if (it != m_nrSlBsrReceived.end ())
    {
      // update entry
      (*it).second = params;
    }
  else
    {
      m_nrSlBsrReceived.insert (std::make_pair (slLcId, params));
    }

  auto report = NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams (params.rnti, params.lcid,
                                                                                params.txQueueSize, params.txQueueHolDelay,
                                                                                params.retxQueueSize, params.retxQueueHolDelay,
                                                                                params.statusPduSize, params.srcL2Id, params.dstL2Id);

  m_nrSlUeMacSchedSapProvider->SchedUeNrSlRlcBufferReq (report);
}

void
NrUeMac::DoAddNrSlLc (const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo &slLcInfo, NrSlMacSapUser* msu)
{
  NS_LOG_FUNCTION (this << +slLcInfo.lcId << slLcInfo.srcL2Id << slLcInfo.dstL2Id);
  SidelinkLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = slLcInfo.lcId;
  slLcIdentifier.srcL2Id = slLcInfo.srcL2Id;
  slLcIdentifier.dstL2Id = slLcInfo.dstL2Id;

  NS_ASSERT_MSG (m_nrSlLcInfoMap.find (slLcIdentifier) == m_nrSlLcInfoMap.end (), "cannot add LCID " << +slLcInfo.lcId
                                                                    << ", srcL2Id " << slLcInfo.srcL2Id << ", dstL2Id " << slLcInfo.dstL2Id << " is already present");

  SlLcInfoUeMac slLcInfoUeMac;
  slLcInfoUeMac.lcInfo = slLcInfo;
  slLcInfoUeMac.macSapUser = msu;
  m_nrSlLcInfoMap.insert (std::make_pair (slLcIdentifier, slLcInfoUeMac));

  auto lcInfo = NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo (slLcInfo.dstL2Id, slLcInfo.lcId,
                                                                        slLcInfo.lcGroup, slLcInfo.pqi,
                                                                        slLcInfo.priority, slLcInfo.isGbr,
                                                                        slLcInfo.mbr, slLcInfo.gbr);

  //Following if is needed because this method is called for both
  //TX and RX LCs addition into m_nrSlLcInfoMap. In case of RX LC, the
  //destination is this UE MAC.
  if (slLcInfo.srcL2Id == m_srcL2Id)
    {
      m_nrSlUeMacCschedSapProvider->CschedUeNrSlLcConfigReq (lcInfo);
      AddNrSlDstL2Id (slLcInfo.dstL2Id, slLcInfo.priority);
    }
}

void
NrUeMac::DoRemoveNrSlLc (uint8_t slLcId, uint32_t srcL2Id, uint32_t dstL2Id)
{
  NS_LOG_FUNCTION (this << +slLcId << srcL2Id << dstL2Id);
  NS_ASSERT_MSG (slLcId > 3, "Hey! I can delete only the LC for data radio bearers.");
  SidelinkLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = slLcId;
  slLcIdentifier.srcL2Id = srcL2Id;
  slLcIdentifier.dstL2Id = dstL2Id;
  NS_ASSERT_MSG (m_nrSlLcInfoMap.find (slLcIdentifier) != m_nrSlLcInfoMap.end (), "could not find Sidelink LCID " << slLcId);
  m_nrSlLcInfoMap.erase (slLcIdentifier);
}

void
NrUeMac::DoResetNrSlLcMap ()
{
  NS_LOG_FUNCTION (this);

  auto it = m_nrSlLcInfoMap.begin ();

  while (it != m_nrSlLcInfoMap.end ())
    {
      if (it->first.lcId > 3) //SL DRB LC starts from 4
        {
          m_nrSlLcInfoMap.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
NrUeMac::AddNrSlDstL2Id (uint32_t dstL2Id, uint8_t lcPriority)
{
  NS_LOG_FUNCTION (this << dstL2Id << lcPriority);
  bool foundDst = false;
  for (auto& it : m_sidelinkDestinations)
    {
      if (it.first == dstL2Id)
        {
          foundDst = true;
          if (lcPriority < it.second)
            {
              it.second = lcPriority;
            }
          break;
        }
    }

  if (!foundDst)
    {
      m_sidelinkDestinations.push_back (std::make_pair (dstL2Id, lcPriority));
      m_nrSlHarq->AddDst (dstL2Id, m_numSidelinkProcess);
    }

  std::sort (m_sidelinkDestinations.begin (), m_sidelinkDestinations.end (), CompareSecond);
}

bool
NrUeMac::CompareSecond (std::pair<uint32_t, uint8_t>& a, std::pair<uint32_t, uint8_t>& b)
{
  return a.second < b.second;
}

void
NrUeMac::DoAddNrSlCommTxPool (Ptr<const NrSlCommResourcePool> txPool)
{
  NS_LOG_FUNCTION (this << txPool);
  m_slTxPool = txPool;
  m_slTxPool->ValidateResvPeriod (GetBwpId (), m_poolId, m_pRsvpTx);
}

void
NrUeMac::DoAddNrSlCommRxPool (Ptr<const NrSlCommResourcePool> rxPool)
{
  NS_LOG_FUNCTION (this);
  m_slRxPool = rxPool;
}

void
NrUeMac::DoSetSlProbResoKeep (uint8_t prob)
{
  NS_LOG_FUNCTION (this << +prob);
  NS_ASSERT_MSG (prob <= 1, "Probability value must be less than 1");
  m_slProbResourceKeep = prob;
}

void
NrUeMac::DoSetSourceL2Id (uint32_t srcL2Id)
{
  NS_LOG_FUNCTION (this << srcL2Id);
  m_srcL2Id = srcL2Id;
}

uint8_t
NrUeMac::DoGetSlActiveTxPoolId ()
{
  return GetSlActivePoolId ();
}

uint8_t
NrUeMac::DoGetTotalSubCh () const
{
  NS_LOG_FUNCTION (this);
  return this->GetTotalSubCh (m_poolId);
}

void
NrUeMac::EnableSensing (bool enableSensing)
{
  NS_LOG_FUNCTION (this << enableSensing);
  NS_ASSERT_MSG (m_enableSensing == false, " Once the sensing is enabled, it can not be enabled or disabled again");
  m_enableSensing = enableSensing;
}

void
NrUeMac::EnableBlindReTx (bool enableBlindReTx)
{
  NS_LOG_FUNCTION (this << enableBlindReTx);
  NS_ASSERT_MSG (m_enableBlindReTx == false, " Once the blind re-transmission is enabled, it can not be enabled or disabled again");
  m_enableBlindReTx = enableBlindReTx;
}

void
NrUeMac::SetTproc0 (uint8_t tproc0)
{
  NS_LOG_FUNCTION (this << +tproc0);
  m_tproc0 = tproc0;
}

uint8_t
NrUeMac::GetTproc0 () const
{
  return m_tproc0;
}

uint8_t
NrUeMac::GetT1 () const
{
  return m_t1;
}

void
NrUeMac::SetT1 (uint8_t t1)
{
  NS_LOG_FUNCTION (this << +t1);
  m_t1 = t1;
}

uint16_t
NrUeMac::GetT2 () const
{
  return m_t2;
}

void
NrUeMac::SetT2 (uint16_t t2)
{
  NS_LOG_FUNCTION (this << t2);
  m_t2 = t2;
}

uint16_t
NrUeMac::GetSlActivePoolId () const
{
  return m_poolId;
}

void
NrUeMac::SetSlActivePoolId (uint16_t poolId)
{
  m_poolId =  poolId;
}

uint8_t
NrUeMac::GetTotalSubCh (uint16_t poolId) const
{
  NS_LOG_FUNCTION (this << poolId);

  uint16_t subChSize = m_slTxPool->GetNrSlSubChSize (static_cast <uint8_t> (GetBwpId ()), poolId);

  uint8_t totalSubChanels = static_cast <uint8_t> (std::floor (m_nrSlUePhySapProvider->GetBwInRbs () / subChSize));

  return totalSubChanels;
}

void
NrUeMac::SetReservationPeriod (const Time &rsvp)
{
  NS_LOG_FUNCTION (this << rsvp);
  m_pRsvpTx = rsvp;
}

Time
NrUeMac::GetReservationPeriod () const
{
  return m_pRsvpTx;
}

uint8_t
NrUeMac::GetRndmReselectionCounter () const
{
  uint8_t min, max;
  uint16_t periodInt = static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ());

  switch(periodInt)
  {
    case 50:
      min = GetLoBoundReselCounter (periodInt);
      max = GetUpBoundReselCounter (periodInt);
      break;
    case 100:
    case 150:
    case 200:
    case 250:
    case 300:
    case 350:
    case 400:
    case 450:
    case 500:
    case 550:
    case 600:
    case 700:
    case 750:
    case 800:
    case 850:
    case 900:
    case 950:
    case 1000:
      min = 5;
      max = 15;
      break;
    default:
      NS_FATAL_ERROR ("VALUE NOT SUPPORTED!");
      break;
  }
  return m_ueSelectedUniformVariable->GetInteger (min, max);
}

uint8_t
NrUeMac::GetLoBoundReselCounter (uint16_t pRsrv) const
{
  NS_LOG_FUNCTION (this << pRsrv);
  NS_ASSERT_MSG (pRsrv < 100, "Resource reservation must be less than 100 ms");
  uint8_t lBound = (5 * std::ceil (100 / (std::max (static_cast <uint16_t> (20), pRsrv))));
  return lBound;
}

uint8_t
NrUeMac::GetUpBoundReselCounter (uint16_t pRsrv) const
{
  NS_LOG_FUNCTION (this << pRsrv);
  NS_ASSERT_MSG (pRsrv < 100, "Resource reservation must be less than 100 ms");
  uint8_t uBound = (15 * std::ceil (100 / (std::max (static_cast <uint16_t> (20), pRsrv))));
  return uBound;
}

void
NrUeMac::SetNumSidelinkProcess (uint8_t numSidelinkProcess)
{
  NS_LOG_FUNCTION (this);
  m_numSidelinkProcess = numSidelinkProcess;
}

uint8_t
NrUeMac::GetNumSidelinkProcess () const
{
  NS_LOG_FUNCTION (this);
  return m_numSidelinkProcess;
}

//////////////////////////////////////////////

}
