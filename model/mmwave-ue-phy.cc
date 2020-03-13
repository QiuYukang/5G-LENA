/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015 NYU WIRELESS, Tandon School of Engineering, New York University
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

#include "mmwave-ue-phy.h"
#include "mmwave-ue-net-device.h"
#include "mmwave-spectrum-value-helper.h"
#include "nr-ch-access-manager.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/node.h>
#include <ns3/double.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <algorithm>
#include <ns3/boolean.h>
#include <ns3/pointer.h>
#include "beam-manager.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveUePhy");
NS_OBJECT_ENSURE_REGISTERED (MmWaveUePhy);

MmWaveUePhy::MmWaveUePhy ()
{
  NS_LOG_FUNCTION (this);
  m_wbCqiLast = Simulator::Now ();
  m_ueCphySapProvider = new MemberLteUeCphySapProvider<MmWaveUePhy> (this);
}

MmWaveUePhy::~MmWaveUePhy ()
{
  NS_LOG_FUNCTION (this);
  delete m_ueCphySapProvider;
}

TypeId
MmWaveUePhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveUePhy")
    .SetParent<MmWavePhy> ()
    .AddConstructor<MmWaveUePhy> ()
    .AddAttribute ("TxPower",
                   "Transmission power in dBm",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&MmWaveUePhy::m_txPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0.\" "
                  "In this model, we consider T0 = 290K.",
                   DoubleValue (5.0), // mmwave code from NYU and UniPd assumed in the code the value of 5dB, thats why we configure the default value to that
                   MakeDoubleAccessor (&MmWaveUePhy::m_noiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SpectrumPhy",
                   "The SpectrumPhy associated to this MmWavePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&MmWavePhy::GetSpectrumPhy),
                   MakePointerChecker <MmWaveSpectrumPhy> ())
    .AddAttribute ("LBTThresholdForCtrl",
                   "After a DL/UL transmission, if we have less than this value to send the UL CTRL, we consider the channel as granted",
                   TimeValue (MicroSeconds (25)),
                   MakeTimeAccessor (&MmWaveUePhy::m_lbtThresholdForCtrl),
                   MakeTimeChecker ())
    .AddAttribute ("TbDecodeLatency",
                   "Transport block decode latency",
                   TimeValue (MicroSeconds (100)),
                   MakeTimeAccessor (&MmWavePhy::SetTbDecodeLatency,
                   &MmWavePhy::GetTbDecodeLatency),
                   MakeTimeChecker ())
    .AddTraceSource ("ReportCurrentCellRsrpSinr",
                     "RSRP and SINR statistics.",
                     MakeTraceSourceAccessor (&MmWaveUePhy::m_reportCurrentCellRsrpSinrTrace),
                     "ns3::CurrentCellRsrpSinr::TracedCallback")
    .AddTraceSource ("ReportUplinkTbSize",
                     "Report allocated uplink TB size for trace.",
                     MakeTraceSourceAccessor (&MmWaveUePhy::m_reportUlTbSize),
                     "ns3::UlTbSize::TracedCallback")
    .AddTraceSource ("ReportDownlinkTbSize",
                     "Report allocated downlink TB size for trace.",
                     MakeTraceSourceAccessor (&MmWaveUePhy::m_reportDlTbSize),
                     "ns3::DlTbSize::TracedCallback")
    .AddTraceSource ("UePhyRxedCtrlMsgsTrace",
                     "Ue PHY Control Messages Traces.",
                     MakeTraceSourceAccessor (&MmWaveUePhy::m_phyRxedCtrlMsgsTrace),
                     "ns3::MmWavePhyRxTrace::RxedUePhyCtrlMsgsTracedCallback")
    .AddTraceSource ("UePhyTxedCtrlMsgsTrace",
                     "Ue PHY Control Messages Traces.",
                     MakeTraceSourceAccessor (&MmWaveUePhy::m_phyTxedCtrlMsgsTrace),
                     "ns3::MmWavePhyRxTrace::TxedUePhyCtrlMsgsTracedCallback")
    .AddTraceSource ("UePhyRxedDlDciTrace",
                     "Ue PHY DL DCI Traces.",
                     MakeTraceSourceAccessor (&MmWaveUePhy::m_phyUeRxedDlDciTrace),
                     "ns3::MmWavePhyRxTrace::RxedUePhyDlDciTracedCallback")
    .AddTraceSource ("UePhyTxedHarqFeedbackTrace",
                     "Ue PHY DL HARQ Feedback Traces.",
                     MakeTraceSourceAccessor (&MmWaveUePhy::m_phyUeTxedHarqFeedbackTrace),
                     "ns3::MmWavePhyRxTrace::TxedUePhyHarqFeedbackTracedCallback")
      ;
  return tid;
}

void
MmWaveUePhy::ChannelAccessGranted (const Time &time)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (time);
  // That will be granted only till the end of the slot
  m_channelStatus = GRANTED;
}

void
MmWaveUePhy::ChannelAccessDenied ()
{
  NS_LOG_FUNCTION (this);
  m_channelStatus = NONE;
}

void
MmWaveUePhy::SetUeCphySapUser (LteUeCphySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_ueCphySapUser = s;
}

LteUeCphySapProvider*
MmWaveUePhy::GetUeCphySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return (m_ueCphySapProvider);
}

void
MmWaveUePhy::SetTxPower (double pow)
{
  m_txPower = pow;
}
double
MmWaveUePhy::GetTxPower () const
{
  return m_txPower;
}

void
MmWaveUePhy::SetSubChannelsForTransmission (std::vector <int> mask)
{
  Ptr<SpectrumValue> txPsd = GetTxPowerSpectralDensity (mask);
  NS_ASSERT (txPsd);
  m_spectrumPhy->SetTxPowerSpectralDensity (txPsd);
}

void
MmWaveUePhy::DoSendControlMessage (Ptr<MmWaveControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  EnqueueCtrlMessage (msg);
}

void
MmWaveUePhy::DoSendControlMessageNow (Ptr<MmWaveControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  EnqueueCtrlMsgNow (msg);
}

void
MmWaveUePhy::RegisterToEnb (uint16_t bwpId, Ptr<MmWavePhyMacCommon> config)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phyMacConfig == nullptr); // Otherwise we probably have to change things..

  m_phyMacConfig = config;

  InitializeMessageList ();

  Ptr<SpectrumValue> noisePsd = GetNoisePowerSpectralDensity ();
  m_spectrumPhy->SetNoisePowerSpectralDensity (noisePsd);

  m_spectrumPhy->GetHarqPhyModule ()->SetHarqNum (m_phySapUser->GetNumHarqProcess ());

  m_amc = CreateObject <NrAmc> (m_phyMacConfig);
  DoSetCellId (bwpId);
}

void
MmWaveUePhy::SetNumRbPerRbg (uint32_t numRbPerRbg)
{
  m_numRbPerRbg = numRbPerRbg;
}

uint32_t
MmWaveUePhy::GetNumRbPerRbg () const
{
  return m_numRbPerRbg;
}

uint32_t
MmWaveUePhy::GetChannelBandwidth() const
{
  // m_channelBandwidth is in kHz * 100
  return m_channelBandwidth * 1000 * 100;
}

void
MmWaveUePhy::InsertAllocation (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  VarTtiAllocInfo varTtiInfo (dci);
  m_currSlotAllocInfo.m_varTtiAllocInfo.push_back (varTtiInfo);
  std::sort (m_currSlotAllocInfo.m_varTtiAllocInfo.begin (), m_currSlotAllocInfo.m_varTtiAllocInfo.end ());
}

void
MmWaveUePhy::InsertFutureAllocation (const SfnSf &sfnSf,
                                     const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  VarTtiAllocInfo varTtiInfo (dci);
  if (SlotAllocInfoExists (sfnSf))
    {
      auto & ulSlot = PeekSlotAllocInfo (sfnSf);
      ulSlot.m_varTtiAllocInfo.push_back (varTtiInfo);
      std::sort (ulSlot.m_varTtiAllocInfo.begin (), ulSlot.m_varTtiAllocInfo.end ());
    }
  else
    {
      SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);
      slotAllocInfo.m_varTtiAllocInfo.push_back (varTtiInfo);
      PushBackSlotAllocInfo (slotAllocInfo);
    }
}

void
MmWaveUePhy::PhyCtrlMessagesReceived (const Ptr<MmWaveControlMessage> &msg)
{
  NS_LOG_FUNCTION (this);

  if (msg->GetMessageType () == MmWaveControlMessage::DCI_TDMA)
    {
      NS_ASSERT_MSG (m_varTtiNum == 0, "UE" << m_rnti << " got DCI on slot != 0");
      Ptr<MmWaveTdmaDciMessage> dciMsg = DynamicCast<MmWaveTdmaDciMessage> (msg);

      auto dciInfoElem = dciMsg->GetDciInfoElement ();

      m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                              m_rnti, GetBwpId (), msg);

      if (dciInfoElem->m_rnti != 0 && dciInfoElem->m_rnti != m_rnti)
        {
          return;   // DCI not for me
        }

      if (dciInfoElem->m_format == DciInfoElementTdma::DL
          && dciInfoElem->m_type == DciInfoElementTdma::DATA)
        {
          SfnSf dciSfn = SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum);
          uint32_t k0Delay = dciMsg->GetKDelay ();
          dciSfn.Add (k0Delay, m_phyMacConfig->GetSlotsPerSubframe ());

          NS_LOG_DEBUG ("UE" << m_rnti << " DL-DCI received for slot " << dciSfn <<
                        " symStart " << static_cast<uint32_t> (dciInfoElem->m_symStart) <<
                        " numSym " << static_cast<uint32_t> (dciInfoElem->m_numSym) <<
                        " tbs " << dciInfoElem->m_tbSize <<
                        " harqId " << static_cast<uint32_t> (dciInfoElem->m_harqProcess));

          /* BIG ASSUMPTION: We assume that K0 is always 0 */

          auto it = m_harqIdToK1Map.find (dciInfoElem->m_harqProcess);
          if (it!=m_harqIdToK1Map.end ())
            {
              m_harqIdToK1Map.erase (m_harqIdToK1Map.find (dciInfoElem->m_harqProcess));
            }

          m_harqIdToK1Map.insert (std::make_pair (dciInfoElem->m_harqProcess, dciMsg->GetK1Delay ()));

          m_phyUeRxedDlDciTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                 m_rnti, GetBwpId (), dciInfoElem->m_harqProcess, dciMsg->GetK1Delay ());

          InsertAllocation (dciInfoElem);
        }
      else if (dciInfoElem->m_format == DciInfoElementTdma::UL
               && dciInfoElem->m_type == DciInfoElementTdma::DATA)   // set downlink slot schedule for t+Tul_sched slot
        {
          SfnSf ulSfnSf = SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum);
          uint32_t k2Delay = dciMsg->GetKDelay ();
          ulSfnSf.Add (k2Delay, m_phyMacConfig->GetSlotsPerSubframe ());

          NS_LOG_DEBUG ("UE" << m_rnti <<
                        " UL-DCI received for slot " << ulSfnSf <<
                        " symStart " << static_cast<uint32_t> (dciInfoElem->m_symStart) <<
                        " numSym " << static_cast<uint32_t> (dciInfoElem->m_numSym) <<
                        " tbs " << dciInfoElem->m_tbSize <<
                        " harqId " << static_cast<uint32_t> (dciInfoElem->m_harqProcess));

          if (ulSfnSf == SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0))
            {
              InsertAllocation (dciInfoElem);
            }
          else
            {
              InsertFutureAllocation (ulSfnSf, dciInfoElem);
            }
        }

      NS_LOG_INFO ("Received DCI_TDMA for RNTI: " << m_rnti << " in slot " <<
                   SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum) <<
                   ", scheduling MAC ReceiveControlMessage after the decode latency");
      Simulator::Schedule (GetTbDecodeLatency(), &MmWaveUePhySapUser::ReceiveControlMessage, m_phySapUser, msg);
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::MIB)
    {
      NS_LOG_INFO ("received MIB");
      Ptr<MmWaveMibMessage> msg2 = DynamicCast<MmWaveMibMessage> (msg);
      m_ueCphySapUser->RecvMasterInformationBlock (GetBwpId (), msg2->GetMib ());
      m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                              m_rnti, GetBwpId (), msg);
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::SIB1)
    {
      Ptr<MmWaveSib1Message> msg2 = DynamicCast<MmWaveSib1Message> (msg);
      m_ueCphySapUser->RecvSystemInformationBlockType1 (GetBwpId (), msg2->GetSib1 ()); 
      m_tddPattern = msg2->GetTddPattern ();
      m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                              m_rnti, GetBwpId (), msg);
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::RAR)
    {
      NS_LOG_INFO ("Received RAR in slot " << SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum));
      Ptr<MmWaveRarMessage> rarMsg = DynamicCast<MmWaveRarMessage> (msg);
      m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                              m_rnti, GetBwpId (), msg);

      Simulator::Schedule (GetTbDecodeLatency(), &MmWaveUePhy::DoReceiveRar, this, rarMsg);
    }
  else
    {
      NS_LOG_INFO ("Message type not recognized " << msg->GetMessageType ());
      m_phySapUser->ReceiveControlMessage (msg);
      m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                              m_rnti, GetBwpId (), msg);
    }
}

void
MmWaveUePhy::TryToPerformLbt ()
{
  uint8_t ulCtrlSymStart = 0;
  uint8_t ulCtrlNumSym = 0;

  for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL && alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
          ulCtrlSymStart = alloc.m_dci->m_symStart;
          ulCtrlNumSym = alloc.m_dci->m_numSym;
          break;
        }
    }

  if (ulCtrlNumSym != 0)
    {
      // We have an UL CTRL symbol scheduled and we have to transmit CTRLs..
      // .. so we check that we have at least 25 us between the latest DCI,
      // or we have to schedule an LBT event.

      Time limit = m_lastSlotStart + GetSlotPeriod () -
          ((GetSymbolsPerSlot () - ulCtrlSymStart) * GetSymbolPeriod ()) -
          m_lbtThresholdForCtrl;

      for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
        {
          int64_t symbolPeriod = GetSymbolPeriod ().GetMicroSeconds ();
          int64_t dciEndsAt = m_lastSlotStart.GetMicroSeconds () +
              ((alloc.m_dci->m_numSym + alloc.m_dci->m_symStart) * symbolPeriod);

          if (alloc.m_dci->m_type != DciInfoElementTdma::DATA)
            {
              continue;
            }

          if (limit.GetMicroSeconds () < dciEndsAt)
            {
              NS_LOG_INFO ("This data DCI ends at " << MicroSeconds (dciEndsAt) <<
                           " which is inside the LBT shared COT (the limit is " <<
                           limit << "). No need for LBT");
              m_lbtEvent.Cancel (); // Forget any LBT we previously set, because of the new
                                    // DCI information
              m_channelStatus = GRANTED;
            }
          else
            {
              NS_LOG_INFO ("This data DCI starts at " << +alloc.m_dci->m_symStart << " for " <<
                           +alloc.m_dci->m_numSym << " ends at " << MicroSeconds (dciEndsAt) <<
                           " which is outside the LBT shared COT (the limit is " <<
                           limit << ").");
            }
        }
      if (m_channelStatus != GRANTED)
        {
          Time sched = m_lastSlotStart - Simulator::Now () +
              (GetSymbolPeriod () * ulCtrlSymStart) - MicroSeconds (25);
          NS_LOG_INFO ("Scheduling an LBT for sending the UL CTRL at " <<
                       Simulator::Now () + sched);
          m_lbtEvent.Cancel ();
          m_lbtEvent = Simulator::Schedule (sched, &MmWaveUePhy::RequestAccess, this);
        }
    }
}

void
MmWaveUePhy::RequestAccess ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Request access at " << Simulator::Now () << " because we have to transmit UL CTRL");
  m_cam->RequestAccess (); // This will put the m_channelStatus to granted when
                           // the channel will be granted.
}

void
MmWaveUePhy::DoReceiveRar (Ptr<MmWaveRarMessage> rarMsg)
{
  NS_LOG_FUNCTION (this);

  for (auto it = rarMsg->RarListBegin (); it != rarMsg->RarListEnd (); ++it)
    {
      if (it->rapId == m_raPreambleId)
        {
          m_phySapUser->ReceiveControlMessage (rarMsg);
        }
    }
}

void
MmWaveUePhy::PushCtrlAllocations (const SfnSf currentSfnSf)
{
  NS_LOG_FUNCTION (this);

  // The UE does not know anything from the GNB yet, so listen on the default
  // bandwidth.
  std::vector<uint8_t> rbgBitmask (GetRbNum (), 1);

  // The UE still doesn't know the TDD pattern, so just add a DL CTRL
  if (m_tddPattern.size () == 0)
    {
      NS_LOG_INFO ("TDD Pattern unknown, insert DL CTRL at the beginning of the slot");
      VarTtiAllocInfo dlCtrlSlot (std::make_shared<DciInfoElementTdma> (0, 1, DciInfoElementTdma::DL, DciInfoElementTdma::CTRL, rbgBitmask));
      m_currSlotAllocInfo.m_varTtiAllocInfo.push_front (dlCtrlSlot);
      return;
    }

  uint64_t currentSlotN = currentSfnSf.Normalize (m_phyMacConfig->GetSlotsPerSubframe ()) % m_tddPattern.size ();

  if (m_tddPattern[currentSlotN] < LteNrTddSlotType::UL)
    {
      NS_LOG_INFO ("The current TDD pattern indicates that we are in a " <<
                   m_tddPattern[currentSlotN] <<
                   " slot, so insert DL CTRL at the beginning of the slot");
      VarTtiAllocInfo dlCtrlSlot (std::make_shared<DciInfoElementTdma> (0, 1, DciInfoElementTdma::DL, DciInfoElementTdma::CTRL, rbgBitmask));
      m_currSlotAllocInfo.m_varTtiAllocInfo.push_front (dlCtrlSlot);
    }
  if (m_tddPattern[currentSlotN] > LteNrTddSlotType::DL)
    {
      NS_LOG_INFO ("The current TDD pattern indicates that we are in a " <<
                   m_tddPattern[currentSlotN] <<
                   " slot, so insert UL CTRL at the end of the slot");
      VarTtiAllocInfo ulCtrlSlot (std::make_shared<DciInfoElementTdma> (GetSymbolsPerSlot () - 1, 1, DciInfoElementTdma::UL, DciInfoElementTdma::CTRL, rbgBitmask));
      m_currSlotAllocInfo.m_varTtiAllocInfo.push_back (ulCtrlSlot);
    }
}

void
MmWaveUePhy::StartSlot (uint16_t frameNum, uint8_t sfNum, uint16_t slotNum)
{
  NS_LOG_FUNCTION (this);
  m_frameNum = frameNum;
  m_subframeNum = sfNum;
  m_slotNum = static_cast<uint8_t> (slotNum);
  m_lastSlotStart = Simulator::Now ();
  m_varTtiNum = 0;

  auto currentSfnSf = SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0);

  // Call MAC before doing anything in PHY
  m_phySapUser->SlotIndication (currentSfnSf);   // trigger mac

  // update the current slot object, and insert DL/UL CTRL allocations depending on the TDD pattern
  if (SlotAllocInfoExists (SfnSf (frameNum, sfNum, slotNum, m_varTtiNum)))
    {
      m_currSlotAllocInfo = RetrieveSlotAllocInfo (SfnSf (frameNum, sfNum, slotNum, m_varTtiNum));
    }
  else
    {
      m_currSlotAllocInfo = SlotAllocInfo (SfnSf (frameNum, sfNum, slotNum, m_varTtiNum));
    }

  PushCtrlAllocations (currentSfnSf);

  NS_ASSERT ((m_currSlotAllocInfo.m_sfnSf.m_frameNum == m_frameNum)
             && (m_currSlotAllocInfo.m_sfnSf.m_subframeNum == m_subframeNum
                 && m_currSlotAllocInfo.m_sfnSf.m_slotNum == m_slotNum));

  NS_LOG_INFO ("UE " << m_rnti << " start slot " << m_currSlotAllocInfo.m_sfnSf <<
               " composed by the following allocations, total " << m_currSlotAllocInfo.m_varTtiAllocInfo.size ());
  for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      std::string direction, type;
      if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL)
        {
          type = "CTRL";
        }
      else if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL_DATA)
        {
          type = "CTRL_DATA";
        }
      else
        {
          type = "DATA";
        }

      if (alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
          direction = "UL";
        }
      else
        {
          direction = "DL";
        }
      NS_LOG_INFO ("Allocation from sym " << static_cast<uint32_t> (alloc.m_dci->m_symStart) <<
                   " to sym " << static_cast<uint32_t> (alloc.m_dci->m_numSym + alloc.m_dci->m_symStart) <<
                   " direction " << direction << " type " << type);
    }

  auto currentDci = m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum].m_dci;
  auto nextVarTtiStart = GetSymbolPeriod () * currentDci->m_symStart;

  TryToPerformLbt ();

  auto ctrlMsgs = PopCurrentSlotCtrlMsgs ();
  if (m_netDevice)
    {
      DynamicCast<MmWaveUeNetDevice> (m_netDevice)->RouteOutgoingCtrlMsgs (ctrlMsgs, GetBwpId ());
    }
  else
    {
      // No netDevice (that could happen in tests) so just redirect them to us
      for (const auto & msg : ctrlMsgs)
        {
          EncodeCtrlMsg (msg);
        }

    }


  Simulator::Schedule (nextVarTtiStart, &MmWaveUePhy::StartVarTti, this);
}


Time
MmWaveUePhy::DlCtrl(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  NS_LOG_DEBUG ("UE" << m_rnti <<
                " RXing DL CTRL frame for"
                " symbols "  << +dci->m_symStart <<
                "-" << +(dci->m_symStart + dci->m_numSym - 1) <<
                "\t start " << Simulator::Now () <<
                " end " << (Simulator::Now () + varTtiPeriod));

  m_tryToPerformLbt = true;

  return varTtiPeriod;
}

Time
MmWaveUePhy::UlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  if (m_ctrlMsgs.size () == 0)
    {
      NS_LOG_INFO   ("UE" << m_rnti << " reserved space for UL CTRL frame for symbols " <<
                    +dci->m_symStart << "-" <<
                    +(dci->m_symStart + dci->m_numSym - 1) <<
                    "\t start " << Simulator::Now () << " end " <<
                    (Simulator::Now () + varTtiPeriod - NanoSeconds (1.0)) <<
                    " but no data to transmit");
      m_cam->Cancel ();
      return varTtiPeriod;
    }
  else if (m_channelStatus != GRANTED)
    {
      NS_LOG_INFO ("UE" << m_rnti << " has to transmit CTRL but channel not granted");
      m_cam->Cancel ();
      return varTtiPeriod;
    }

  for (const auto & msg : m_ctrlMsgs)
    {
      m_phyTxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, dci->m_symStart),
                              dci->m_rnti, GetBwpId (), msg);

      if (msg->GetMessageType () == MmWaveControlMessage::DL_HARQ)
        {
          Ptr<MmWaveDlHarqFeedbackMessage> harqMsg = DynamicCast<MmWaveDlHarqFeedbackMessage> (msg);
          uint8_t harqId = harqMsg->GetDlHarqFeedback ().m_harqProcessId;

          auto it = m_harqIdToK1Map.find (harqId);
          if (it!=m_harqIdToK1Map.end ())
            {
              m_phyUeTxedHarqFeedbackTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                      m_rnti, GetBwpId (),
                                      static_cast<uint32_t> (harqId), it->second);
            }
        }
    }

  std::vector<int> channelRbs;
  for (uint32_t i = 0; i < GetRbNum (); i++)
    {
      channelRbs.push_back (static_cast<int> (i));
    }

  SetSubChannelsForTransmission (channelRbs);

  NS_LOG_DEBUG ("UE" << m_rnti << " TXing UL CTRL frame for symbols " <<
                +dci->m_symStart << "-" <<
                +(dci->m_symStart + dci->m_numSym - 1) <<
                "\t start " << Simulator::Now () << " end " <<
                (Simulator::Now () + varTtiPeriod - NanoSeconds (1.0)));

  SendCtrlChannels (varTtiPeriod - NanoSeconds (1.0));

  ChannelAccessDenied (); // Reset the channel status
  return varTtiPeriod;
}

Time
MmWaveUePhy::DlData (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  m_receptionEnabled = true;
  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  m_spectrumPhy->AddExpectedTb (dci->m_rnti, dci->m_ndi, dci->m_tbSize, dci->m_mcs,
                                        FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask),
                                        dci->m_harqProcess, dci->m_rv, true,
                                        dci->m_symStart, dci->m_numSym);
  m_reportDlTbSize (m_netDevice->GetObject <MmWaveUeNetDevice> ()->GetImsi (), dci->m_tbSize);
  NS_LOG_DEBUG ("UE" << m_rnti <<
                " RXing DL DATA frame for"
                " symbols "  << +dci->m_symStart <<
                "-" << +(dci->m_symStart + dci->m_numSym - 1) <<
                " num of rbg assigned: " << FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask).size () <<
                "\t start " << Simulator::Now () <<
                " end " << (Simulator::Now () + varTtiPeriod));

  return varTtiPeriod;
}

Time
MmWaveUePhy::UlData(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  SetSubChannelsForTransmission (FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask));
  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;
  std::list<Ptr<MmWaveControlMessage> > ctrlMsg;
  Ptr<PacketBurst> pktBurst = GetPacketBurst (SfnSf (m_frameNum, m_subframeNum, m_slotNum, dci->m_symStart));
  if (pktBurst && pktBurst->GetNPackets () > 0)
    {
      std::list< Ptr<Packet> > pkts = pktBurst->GetPackets ();
      MmWaveMacPduTag tag;
      pkts.front ()->PeekPacketTag (tag);
      NS_ASSERT ((tag.GetSfn ().m_subframeNum == m_subframeNum) && (tag.GetSfn ().m_varTtiNum == dci->m_symStart));

      LteRadioBearerTag bearerTag;
      if (!pkts.front ()->PeekPacketTag (bearerTag))
        {
          NS_FATAL_ERROR ("No radio bearer tag");
        }
    }
  else
    {
      NS_LOG_WARN ("Send an empty PDU .... ");
      // sometimes the UE will be scheduled when no data is queued
      // in this case, send an empty PDU
      MmWaveMacPduTag tag (SfnSf (m_frameNum, m_subframeNum, m_slotNum, dci->m_symStart));
      Ptr<Packet> emptyPdu = Create <Packet> ();
      MmWaveMacPduHeader header;
      MacSubheader subheader (3, 0);    // lcid = 3, size = 0
      header.AddSubheader (subheader);
      emptyPdu->AddHeader (header);
      emptyPdu->AddPacketTag (tag);
      LteRadioBearerTag bearerTag (m_rnti, 3, 0);
      emptyPdu->AddPacketTag (bearerTag);
      pktBurst = CreateObject<PacketBurst> ();
      pktBurst->AddPacket (emptyPdu);
    }
  m_reportUlTbSize (m_netDevice->GetObject <MmWaveUeNetDevice> ()->GetImsi (), dci->m_tbSize);

  NS_LOG_DEBUG ("UE" << m_rnti <<
                " TXing UL DATA frame for" <<
                " symbols "  << +dci->m_symStart <<
                "-" << +(dci->m_symStart + dci->m_numSym - 1)
                     << "\t start " << Simulator::Now () <<
                " end " << (Simulator::Now () + varTtiPeriod));

  Simulator::Schedule (NanoSeconds (1.0), &MmWaveUePhy::SendDataChannels, this,
                       pktBurst, ctrlMsg, varTtiPeriod - NanoSeconds (2.0), m_varTtiNum);
  return varTtiPeriod;
}

void
MmWaveUePhy::StartVarTti ()
{
  NS_LOG_FUNCTION (this);
  Time varTtiPeriod;
  const VarTtiAllocInfo & currSlot = m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum];

  m_currTbs = currSlot.m_dci->m_tbSize;
  m_receptionEnabled = false;

  NS_LOG_DEBUG ("UE " << m_rnti << " frame " << static_cast<uint32_t> (m_frameNum) <<
                " subframe " << static_cast<uint32_t> (m_subframeNum) << " slot " <<
                static_cast<uint32_t> (m_slotNum) << " sym " <<
                static_cast<uint32_t> (currSlot.m_dci->m_symStart));

  if (currSlot.m_dci->m_type == DciInfoElementTdma::CTRL && currSlot.m_dci->m_format == DciInfoElementTdma::DL)
    {
      varTtiPeriod = DlCtrl (currSlot.m_dci);
    }
  else if (currSlot.m_dci->m_type == DciInfoElementTdma::CTRL && currSlot.m_dci->m_format == DciInfoElementTdma::UL)
    {
      varTtiPeriod = UlCtrl (currSlot.m_dci);
    }
  else if (currSlot.m_dci->m_type == DciInfoElementTdma::DATA && currSlot.m_dci->m_format == DciInfoElementTdma::DL)
    {
      varTtiPeriod = DlData (currSlot.m_dci);
    }
  else if (currSlot.m_dci->m_type == DciInfoElementTdma::DATA && currSlot.m_dci->m_format == DciInfoElementTdma::UL)
    {
      varTtiPeriod = UlData (currSlot.m_dci);
    }

  Simulator::Schedule (varTtiPeriod, &MmWaveUePhy::EndVarTti, this);
}


void
MmWaveUePhy::EndVarTti ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Executed varTti " << (+m_varTtiNum) + 1 << " of " << m_currSlotAllocInfo.m_varTtiAllocInfo.size ());

  if (m_tryToPerformLbt)
    {
      TryToPerformLbt ();
      m_tryToPerformLbt = false;
    }

  if (m_varTtiNum == m_currSlotAllocInfo.m_varTtiAllocInfo.size () - 1)
    {
      // end of slot
      SfnSf retVal = SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0).IncreaseNoOfSlots (m_phyMacConfig->GetSlotsPerSubframe ());

      Simulator::Schedule (m_lastSlotStart + GetSlotPeriod () -
                           Simulator::Now (),
                           &MmWaveUePhy::StartSlot,
                           this,
                           retVal.m_frameNum,
                           retVal.m_subframeNum,
                           retVal.m_slotNum);
    }
  else
    {
      m_varTtiNum++;
      Time nextVarTtiStart = GetSymbolPeriod () *
                             m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum].m_dci->m_symStart;

      Simulator::Schedule (nextVarTtiStart + m_lastSlotStart - Simulator::Now (), &MmWaveUePhy::StartVarTti, this);
    }

  m_receptionEnabled = false;
}

void
MmWaveUePhy::PhyDataPacketReceived (const Ptr<Packet> &p)
{
  Simulator::ScheduleWithContext (m_netDevice->GetNode ()->GetId (),
                                  GetTbDecodeLatency (),
                                  &MmWaveUePhySapUser::ReceivePhyPdu,
                                  m_phySapUser,
                                  p);
  // m_phySapUser->ReceivePhyPdu (p);
}

void
MmWaveUePhy::SendDataChannels (Ptr<PacketBurst> pb, std::list<Ptr<MmWaveControlMessage> > ctrlMsg, Time duration, uint8_t slotInd)
{
  if (pb->GetNPackets () > 0)
    {
      LteRadioBearerTag tag;
      if (!pb->GetPackets ().front ()->PeekPacketTag (tag))
        {
          NS_FATAL_ERROR ("No radio bearer tag");
        }
    }

  m_spectrumPhy->StartTxDataFrames (pb, ctrlMsg, duration, slotInd);
}

void
MmWaveUePhy::SendCtrlChannels (Time prd)
{
  m_spectrumPhy->StartTxUlControlFrames (m_ctrlMsgs, prd);
  m_ctrlMsgs.clear ();
}

Ptr<MmWaveDlCqiMessage>
MmWaveUePhy::CreateDlCqiFeedbackMessage (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);
  SpectrumValue newSinr = sinr;
  // CREATE DlCqiLteControlMessage
  Ptr<MmWaveDlCqiMessage> msg = Create<MmWaveDlCqiMessage> ();
  DlCqiInfo dlcqi;

  dlcqi.m_rnti = m_rnti;
  dlcqi.m_cqiType = DlCqiInfo::WB;

  std::vector<int> cqi;

  uint8_t mcs;
  dlcqi.m_wbCqi = m_amc->CreateCqiFeedbackWbTdma (newSinr, m_currTbs, mcs);

  msg->SetDlCqi (dlcqi);
  return msg;
}

void
MmWaveUePhy::GenerateDlCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);
  // Not totally sure what this is about. We have to check.
  if (m_ulConfigured && (m_rnti > 0) && m_receptionEnabled)
    {
      if (Simulator::Now () > m_wbCqiLast + m_wbCqiPeriod)
        {
          SpectrumValue newSinr = sinr;
          Ptr<MmWaveDlCqiMessage> msg = CreateDlCqiFeedbackMessage (newSinr);

          if (msg)
            {
              DoSendControlMessage (msg);
            }
          Ptr<MmWaveUeNetDevice> UeRx = DynamicCast<MmWaveUeNetDevice> (m_netDevice);
          m_reportCurrentCellRsrpSinrTrace (UeRx->GetImsi (), newSinr, newSinr);
        }
    }
}

void
MmWaveUePhy::EnqueueDlHarqFeedback (const DlHarqInfo &m)
{
  NS_LOG_FUNCTION (this);
  // get the feedback from MmWaveSpectrumPhy and send it through ideal PUCCH to eNB
  Ptr<MmWaveDlHarqFeedbackMessage> msg = Create<MmWaveDlHarqFeedbackMessage> ();
  msg->SetDlHarqFeedback (m);

  auto k1It = m_harqIdToK1Map.find (m.m_harqProcessId);

  NS_LOG_DEBUG ("ReceiveLteDlHarqFeedback" << " Harq Process " <<
                static_cast<uint32_t> (k1It->first) <<
                " K1: " << k1It->second << " Frame " <<
                SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum));

  Simulator::Schedule (((GetSlotPeriod () * k1It->second) - (Simulator::Now () - m_lastSlotStart)),
                       &MmWaveUePhy::DoSendControlMessageNow, this, msg);
}

void
MmWaveUePhy::SetCam(const Ptr<NrChAccessManager> &cam)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (cam != nullptr);
  m_cam = cam;
  m_cam->SetAccessGrantedCallback (std::bind (&MmWaveUePhy::ChannelAccessGranted, this,
                                              std::placeholders::_1));
  m_cam->SetAccessDeniedCallback (std::bind (&MmWaveUePhy::ChannelAccessDenied, this));
}

uint16_t
MmWaveUePhy::GetRnti ()
{
  return m_rnti;
}

void
MmWaveUePhy::DoReset ()
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveUePhy::DoStartCellSearch (uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << dlEarfcn);
}

void
MmWaveUePhy::DoSynchronizeWithEnb (uint16_t cellId, uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << cellId << dlEarfcn);
  DoSynchronizeWithEnb (cellId);
}

void
MmWaveUePhy::DoSetPa (double pa)
{
  NS_LOG_FUNCTION (this << pa);
}

void
MmWaveUePhy::DoSetRsrpFilterCoefficient (uint8_t rsrpFilterCoefficient)
{
  NS_LOG_FUNCTION (this << +rsrpFilterCoefficient);
}

void
MmWaveUePhy::DoSynchronizeWithEnb (uint16_t cellId)
{
  NS_LOG_FUNCTION (this << cellId);
  NS_UNUSED (cellId);

  m_spectrumPhy->SetNoisePowerSpectralDensity (GetNoisePowerSpectralDensity ());
}

BeamId
MmWaveUePhy::GetBeamId (uint16_t rnti) const
{
  NS_LOG_FUNCTION (this);
  // That's a bad specification: the UE PHY doesn't know anything about its beam id.
  NS_UNUSED (rnti);
  NS_FATAL_ERROR ("ERROR");
}

void
MmWaveUePhy::ScheduleStartEventLoop (uint32_t nodeId, const SfnSf &startSlot)
{
  NS_LOG_FUNCTION (this);
  Simulator::ScheduleWithContext (nodeId, MilliSeconds (0),
                                  &MmWaveUePhy::StartSlot, this,
                                  startSlot.m_frameNum, startSlot.m_subframeNum,
                                  startSlot.m_slotNum);
}

void
MmWaveUePhy::DoSetDlBandwidth (uint16_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << +dlBandwidth);
  m_channelBandwidth = dlBandwidth;
  UpdateRbNum ();
}


void
MmWaveUePhy::DoConfigureUplink (uint16_t ulEarfcn, uint8_t ulBandwidth)
{
  NS_LOG_FUNCTION (this << ulEarfcn << +ulBandwidth);
  // Ignore this; should be equal to dlBandwidth
  m_ulConfigured = true;
}

void
MmWaveUePhy::DoConfigureReferenceSignalPower (int8_t referenceSignalPower)
{
  NS_LOG_FUNCTION (this << referenceSignalPower);
}

void
MmWaveUePhy::DoSetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  m_rnti = rnti;
}

void
MmWaveUePhy::DoSetTransmissionMode (uint8_t txMode)
{
  NS_LOG_FUNCTION (this << +txMode);
}

void
MmWaveUePhy::DoSetSrsConfigurationIndex (uint16_t srcCi)
{
  NS_LOG_FUNCTION (this << srcCi);
}

void
MmWaveUePhy::SetPhySapUser (MmWaveUePhySapUser* ptr)
{
  m_phySapUser = ptr;
}

void
MmWaveUePhy::DoResetPhyAfterRlf ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("MmWaveUePhy does not have RLF functionality yet");
}

void
MmWaveUePhy::DoResetRlfParams ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("MmWaveUePhy does not have RLF functionality yet");
}

void
MmWaveUePhy::DoStartInSnycDetection ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("MmWaveUePhy does not have RLF functionality yet");
}

void
MmWaveUePhy::DoSetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this);
  m_imsi = imsi;
}


}


