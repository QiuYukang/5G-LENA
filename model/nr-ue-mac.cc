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
#include "nr-sl-ue-mac-csched-sap.h"
#include "nr-sl-ue-mac-harq.h"
#include "nr-sl-sci-f01-header.h"
#include "nr-sl-mac-pdu-tag.h"
#include <algorithm>

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

  virtual void SchedUeNrSlConfigInd (const NrSlUeMacSchedSapUser::NrSlSlotAlloc& params);
  virtual uint8_t GetTotalSubCh () const;

private:
  NrUeMac* m_mac;
};

MemberNrSlUeMacSchedSapUser::MemberNrSlUeMacSchedSapUser (NrUeMac* mac)
:m_mac (mac)
{
}
void
MemberNrSlUeMacSchedSapUser::SchedUeNrSlConfigInd (const NrSlUeMacSchedSapUser::NrSlSlotAlloc& params)
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
                                         MakeUintegerChecker<uint8_t> ())
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
  // TB UID passed back along with RLC data as HARQ process ID
  std::map<uint32_t, struct MacPduInfo>::iterator it = m_macPduMap.find (params.harqProcessId);
  if (it == m_macPduMap.end ())
    {
      NS_FATAL_ERROR ("No MAC PDU storage element found for this TB UID/RNTI");
    }
  else
    {
      NrMacPduTag tag;
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

          NrMacPduHeader headerTst;
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
NrUeMac::SendReportBufferStatus (void)
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

  // create the feedback to gNB
  Ptr<NrBsrMessage> msg = Create<NrBsrMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetBsr (bsr);

  m_macTxedCtrlMsgsTrace (m_currentSlot, GetCellId (), bsr.m_rnti, GetBwpId (), msg);
  m_phySapProvider->SendControlMessage (msg);
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
NrUeMac::DoSlotIndication (SfnSf sfn)
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

  DoNrSlSlotIndication (sfn);

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
  NrMacPduHeader macHeader;
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
NrUeMac::RecvRaResponse (BuildRarListElement_s raResponse)
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = false;
  m_rnti = raResponse.m_rnti;
  m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
  m_cmacSapUser->NotifyRandomAccessSuccessful ();
}

std::map<uint32_t, struct MacPduInfo>::iterator
NrUeMac::AddToMacPduMap (const std::shared_ptr<DciInfoElementTdma> &dci,
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

          NrMacPduTag tag (dataSfn, dciInfoElem->m_symStart, dciInfoElem->m_numSym);
          Ptr<Packet> emptyPdu = Create <Packet> ();
          NrMacPduHeader header;
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
          NrMacPduTag tag;
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
          --itGrantInfo.second.slResoReselCounter;
          --itGrantInfo.second.cReselCounter;
          auto grant = itGrantInfo.second.slotAllocations.begin ();
          //prepare and send SCI format 01 message
          NrSlSciF01Header sciF01;
          sciF01.SetPriority (grant->priority);
          sciF01.SetMcs (grant->mcs);
          sciF01.SetSlResourceReservePeriod (static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ()));
          sciF01.SetTotalSubChannels (GetTotalSubCh (m_poolId));
          sciF01.SetIndexStartSubChannel (grant->indexSubchannelStart);
          sciF01.SetLengthSubChannel (grant->subchannelLength);
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
          Ptr<Packet> p = Create<Packet> ();
          p->AddHeader (sciF01);
          NrSlMacPduTag tag (m_rnti, grant->sfn, grant->indexSymStart, grant->SymLength, tbs, grant->dstL2Id);
          p->AddPacketTag (tag);

          m_nrSlUePhySapProvider->SendPscchMacPdu (p);

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

  NrSlUeMacSchedSapUser::NrSlSlotAlloc dummyAlloc;

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
NrUeMac::DoSchedUeNrSlConfigInd (const NrSlUeMacSchedSapUser::NrSlSlotAlloc& params)
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
NrUeMac::CreateGrantInfo (NrSlUeMacSchedSapUser::NrSlSlotAlloc params)
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
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("Yet to be implemented");
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

  m_nrSlUeMacCschedSapProvider->CschedUeNrSlLcConfigReq (lcInfo);
  AddNrSlDstL2Id (slLcInfo.dstL2Id, slLcInfo.priority);
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

uint8_t
NrUeMac::GetSlActivePoolId () const
{
  return m_poolId;
}

void
NrUeMac::SetSlActivePoolId (uint8_t poolId)
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
