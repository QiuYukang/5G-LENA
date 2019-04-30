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
      if (m_phyMacConfig)                                                \
        {                                                                \
          std::clog << " [ CellId " << m_cellId << ", ccId "             \
                    << +m_phyMacConfig->GetCcId ()                       \
                    << ", RNTI " << m_rnti << "] ";                      \
        }                                                                \
    }                                                                    \
  while (false);

#include "mmwave-ue-phy.h"
#include "mmwave-ue-net-device.h"
#include "mmwave-spectrum-value-helper.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/node.h>
#include <ns3/double.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveUePhy");
NS_OBJECT_ENSURE_REGISTERED (MmWaveUePhy);

MmWaveUePhy::MmWaveUePhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

MmWaveUePhy::MmWaveUePhy (Ptr<MmWaveSpectrumPhy> dlPhy, Ptr<MmWaveSpectrumPhy> ulPhy,
                          const Ptr<Node> &n)
  : MmWavePhy (dlPhy, ulPhy)
{
  NS_LOG_FUNCTION (this);
  m_wbCqiLast = Simulator::Now ();
  m_ueCphySapProvider = new MemberLteUeCphySapProvider<MmWaveUePhy> (this);
  Simulator::ScheduleWithContext (n->GetId (), MilliSeconds (0),
                                  &MmWaveUePhy::StartSlot, this, 0, 0, 0);
}

MmWaveUePhy::~MmWaveUePhy ()
{
  NS_LOG_FUNCTION (this);
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
    .AddAttribute ("DlSpectrumPhy",
                   "The downlink MmWaveSpectrumPhy associated to this MmWavePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&MmWaveUePhy::GetDlSpectrumPhy),
                   MakePointerChecker <MmWaveSpectrumPhy> ())
    .AddAttribute ("AntennaArrayType",
                    "AntennaArray of this UE phy. There are two types of antenna array available: "
                    "a) AntennaArrayModel which is using isotropic antenna elements, and "
                    "b) AntennaArray3gppModel which is using directional 3gpp antenna elements."
                    "Another important parameters to specify is the number of antenna elements by "
                    "dimension.",
                    TypeIdValue(ns3::AntennaArrayModel::GetTypeId()),
                    MakeTypeIdAccessor (&MmWavePhy::SetAntennaArrayType,
                                        &MmWavePhy::GetAntennaArrayType),
                    MakeTypeIdChecker())
    .AddAttribute ("AntennaNumDim1",
                   "Size of the first dimension of the antenna sector/panel expressed in number of antenna elements",
                   UintegerValue (2),
                   MakeUintegerAccessor (&MmWavePhy::SetAntennaNumDim1,
                                         &MmWavePhy::GetAntennaNumDim1),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("AntennaNumDim2",
                   "Size of the second dimension of the antenna sector/panel expressed in number of antenna elements",
                   UintegerValue (4),
                   MakeUintegerAccessor (&MmWavePhy::SetAntennaNumDim2,
                                         &MmWavePhy::GetAntennaNumDim2),
                   MakeUintegerChecker<uint8_t> ())
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
      ;
  return tid;
}

void
MmWaveUePhy::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  MmWavePhy::DoInitialize ();
}

void
MmWaveUePhy::DoDispose (void)
{
  delete m_ueCphySapProvider;
  MmWavePhy::DoDispose ();
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
  m_downlinkSpectrumPhy->SetTxPowerSpectralDensity (txPsd);
}

void
MmWaveUePhy::DoSendControlMessage (Ptr<MmWaveControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  EnqueueCtrlMessage (msg);
}

void
MmWaveUePhy::RegisterToEnb (uint16_t cellId, Ptr<MmWavePhyMacCommon> config)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phyMacConfig == nullptr); // Otherwise we probably have to change things..

  m_cellId = cellId;
  m_phyMacConfig = config;

  InitializeMessageList ();

  MmWavePhy::InstallAntenna ();
  NS_ASSERT_MSG (GetAntennaArray(), "Error in initialization of the AntennaModel object");
  Ptr<AntennaArray3gppModel> antenna3gpp = DynamicCast<AntennaArray3gppModel> (GetAntennaArray());

  if (antenna3gpp)
    {
      antenna3gpp->SetIsUe(true);
    }


  m_downlinkSpectrumPhy->SetComponentCarrierId (m_phyMacConfig->GetCcId ());
  m_uplinkSpectrumPhy->SetComponentCarrierId (m_phyMacConfig->GetCcId ());

  m_downlinkSpectrumPhy->SetAntenna (GetAntennaArray());
  m_uplinkSpectrumPhy->SetAntenna (GetAntennaArray());

  Ptr<SpectrumValue> noisePsd = GetNoisePowerSpectralDensity ();
  m_downlinkSpectrumPhy->SetNoisePowerSpectralDensity (noisePsd);
  m_downlinkSpectrumPhy->GetSpectrumChannel ()->AddRx (m_downlinkSpectrumPhy);
  m_downlinkSpectrumPhy->SetCellId (m_cellId);

  GetAntennaArray()->SetSpectrumModel (m_downlinkSpectrumPhy->GetRxSpectrumModel());

  m_amc = CreateObject <NrAmc> (m_phyMacConfig);
}

Ptr<MmWaveSpectrumPhy>
MmWaveUePhy::GetDlSpectrumPhy () const
{
  return m_downlinkSpectrumPhy;
}

void
MmWaveUePhy::PhyCtrlMessagesReceived (const std::list<Ptr<MmWaveControlMessage>> &msgList)
{
  NS_LOG_FUNCTION (this);
  bool dlUpdated = false;
  bool ulUpdated = false;

  SfnSf ulSfnSf = m_currSlotAllocInfo.m_sfnSf.CalculateUplinkSlot (m_phyMacConfig->GetUlSchedDelay (),
                                                                   m_phyMacConfig->GetSlotsPerSubframe (),
                                                                   m_phyMacConfig->GetSubframesPerFrame ());

  for (const auto &msg : msgList)
    {
      if (msg->GetMessageType () == MmWaveControlMessage::DCI_TDMA)
        {
          NS_ASSERT_MSG (m_varTtiNum == 0, "UE" << m_rnti << " got DCI on slot != 0");
          Ptr<MmWaveTdmaDciMessage> dciMsg = DynamicCast<MmWaveTdmaDciMessage> (msg);
          auto dciInfoElem = dciMsg->GetDciInfoElement ();
          SfnSf dciSfn = dciMsg->GetSfnSf ();

          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  m_rnti, m_phyMacConfig->GetCcId (), msg);

          if (dciSfn.m_frameNum != m_frameNum || dciSfn.m_subframeNum != m_subframeNum)
            {
              NS_FATAL_ERROR ("DCI intended for different subframe (dci= "
                              << dciSfn.m_frameNum << " "
                              << dciSfn.m_subframeNum << ", actual= "
                              << m_frameNum << " " << m_subframeNum);
            }

          if (dciInfoElem->m_rnti != m_rnti)
            {
              continue;   // DCI not for me
            }

          if (dciInfoElem->m_format == DciInfoElementTdma::DL)   // set downlink slot schedule for current slot
            {
              NS_LOG_DEBUG ("UE" << m_rnti << " DL-DCI received for slot " << dciSfn <<
                            " symStart " << static_cast<uint32_t> (dciInfoElem->m_symStart) <<
                            " numSym " << static_cast<uint32_t> (dciInfoElem->m_numSym) <<
                            " tbs " << dciInfoElem->m_tbSize <<
                            " harqId " << static_cast<uint32_t> (dciInfoElem->m_harqProcess));

              VarTtiAllocInfo varTtiInfo (dciInfoElem);
              m_currSlotAllocInfo.m_varTtiAllocInfo.push_back (varTtiInfo);
              dlUpdated = true;
            }
          else if (dciInfoElem->m_format == DciInfoElementTdma::UL)   // set downlink slot schedule for t+Tul_sched slot
            {

              NS_LOG_DEBUG ("UE" << m_rnti <<
                            " UL-DCI received for slot " << ulSfnSf <<
                            " symStart " << static_cast<uint32_t> (dciInfoElem->m_symStart) <<
                            " numSym " << static_cast<uint32_t> (dciInfoElem->m_numSym) <<
                            " tbs " << dciInfoElem->m_tbSize <<
                            " harqId " << static_cast<uint32_t> (dciInfoElem->m_harqProcess));

              VarTtiAllocInfo varTtiInfo (dciInfoElem);

              if (m_phyMacConfig->GetUlSchedDelay () == 0)
                {
                  m_currSlotAllocInfo.m_varTtiAllocInfo.push_back (varTtiInfo);
                  ulUpdated = true;
                }
              else
                {
                  if (SlotAllocInfoExists (ulSfnSf))
                    {
                      auto & ulSlot = PeekSlotAllocInfo (ulSfnSf);
                      ulSlot.m_varTtiAllocInfo.push_back (varTtiInfo);
                    }
                  else
                    {
                      SlotAllocInfo slotAllocInfo = SlotAllocInfo (ulSfnSf);
                      slotAllocInfo.m_varTtiAllocInfo.push_back (varTtiInfo);
                      PushBackSlotAllocInfo (slotAllocInfo);
                    }
                  ulUpdated = true;
                }
            }

          m_phySapUser->ReceiveControlMessage (msg);
        }
      else if (msg->GetMessageType () == MmWaveControlMessage::MIB)
        {
          NS_LOG_INFO ("received MIB");
          NS_ASSERT (m_cellId > 0);
          Ptr<MmWaveMibMessage> msg2 = DynamicCast<MmWaveMibMessage> (msg);
          m_ueCphySapUser->RecvMasterInformationBlock (m_cellId, msg2->GetMib ());
          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  m_rnti, m_phyMacConfig->GetCcId (), msg);
        }
      else if (msg->GetMessageType () == MmWaveControlMessage::SIB1)
        {
          NS_ASSERT (m_cellId > 0);
          Ptr<MmWaveSib1Message> msg2 = DynamicCast<MmWaveSib1Message> (msg);
          m_ueCphySapUser->RecvSystemInformationBlockType1 (m_cellId, msg2->GetSib1 ());
          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  m_rnti, m_phyMacConfig->GetCcId (), msg);
        }
      else if (msg->GetMessageType () == MmWaveControlMessage::RAR)
        {
          NS_ASSERT (m_cellId > 0);
          NS_LOG_INFO ("Received RAR in slot " << SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum));
          Ptr<MmWaveRarMessage> rarMsg = DynamicCast<MmWaveRarMessage> (msg);
          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  m_rnti, m_phyMacConfig->GetCcId (), msg);

          // As the TbDecodeLatency includes only the PHY delay, and considering
          // that the RAR message is then forwarded immediately by the MAC to the
          // RRC, we have to put 2 here to respect the TDD timings.
          Simulator::Schedule (2 * MicroSeconds(m_phyMacConfig->GetTbDecodeLatency()),
                               &MmWaveUePhy::DoReceiveRar, this, rarMsg);
        }
      else
        {
          m_phySapUser->ReceiveControlMessage (msg);
          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  m_rnti, m_phyMacConfig->GetCcId (), msg);
        }
    }

  if (dlUpdated)
    {
      std::sort (m_currSlotAllocInfo.m_varTtiAllocInfo.begin (), m_currSlotAllocInfo.m_varTtiAllocInfo.end ());
    }

  if (ulUpdated)
    {
      if (m_phyMacConfig->GetUlSchedDelay () == 0)
        {
          std::sort (m_currSlotAllocInfo.m_varTtiAllocInfo.begin (), m_currSlotAllocInfo.m_varTtiAllocInfo.end ());
        }
      else if (m_phyMacConfig->GetUlSchedDelay () > 0 && SlotAllocInfoExists (ulSfnSf))
        {
          auto & ulSlot = PeekSlotAllocInfo (ulSfnSf);
          std::sort (ulSlot.m_varTtiAllocInfo.begin (), ulSlot.m_varTtiAllocInfo.end ());
        }
    }
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
MmWaveUePhy::StartSlot (uint16_t frameNum, uint8_t sfNum, uint16_t slotNum)
{
  NS_LOG_FUNCTION (this);
  m_frameNum = frameNum;
  m_subframeNum = sfNum;
  m_slotNum = static_cast<uint8_t> (slotNum);
  m_lastSlotStart = Simulator::Now ();
  m_varTtiNum = 0;

  // Call MAC before doing anything in PHY
  m_phySapUser->SlotIndication (SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0));   // trigger mac

  // update the current slot object, and insert DL/UL CTRL allocations.
  // That will not be true anymore when true TDD pattern will be used.
  if (SlotAllocInfoExists (SfnSf (frameNum, sfNum, slotNum, m_varTtiNum)))
    {
      m_currSlotAllocInfo = RetrieveSlotAllocInfo (SfnSf (frameNum, sfNum, slotNum, m_varTtiNum));
    }
  else
    {
      m_currSlotAllocInfo = SlotAllocInfo (SfnSf (frameNum, sfNum, slotNum, m_varTtiNum));
    }

  std::vector<uint8_t> rbgBitmask (m_phyMacConfig->GetBandwidthInRbg (), 1);
  VarTtiAllocInfo dlCtrlSlot (std::make_shared<DciInfoElementTdma> (0, 1, DciInfoElementTdma::DL, DciInfoElementTdma::CTRL, rbgBitmask));
  VarTtiAllocInfo ulCtrlSlot (std::make_shared<DciInfoElementTdma> (m_phyMacConfig->GetSymbolsPerSlot () - 1, 1, DciInfoElementTdma::UL, DciInfoElementTdma::CTRL, rbgBitmask));
  m_currSlotAllocInfo.m_varTtiAllocInfo.push_front (dlCtrlSlot);
  m_currSlotAllocInfo.m_varTtiAllocInfo.push_back (ulCtrlSlot);

  if (m_currSlotAllocInfo.m_varTtiAllocInfo.size () == 0)
    {
      NS_ASSERT (m_currSlotAllocInfo.m_numSymAlloc == 0);
      NS_LOG_INFO ("No allocation in this slot, directly go to the end of the slot");
      Simulator::Schedule (m_phyMacConfig->GetSlotPeriod (), &MmWaveUePhy::EndVarTti, this);
      return;
    }

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
  auto nextVarTtiStart = m_phyMacConfig->GetSymbolPeriod () * currentDci->m_symStart;

  Simulator::Schedule (nextVarTtiStart, &MmWaveUePhy::StartVarTti, this);
}


Time
MmWaveUePhy::DlCtrl(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  Time varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * m_phyMacConfig->GetDlCtrlSymbols ();

  NS_LOG_DEBUG ("UE" << m_rnti <<
                " RXing DL CTRL frame for"
                " symbols "  << +dci->m_symStart <<
                "-" << +(dci->m_symStart + dci->m_numSym - 1) <<
                "\t start " << Simulator::Now () <<
                " end " << (Simulator::Now () + varTtiPeriod));
  return varTtiPeriod;
}

Time
MmWaveUePhy::UlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  std::vector<int> channelRbs;
  for (uint32_t i = 0; i < m_phyMacConfig->GetBandwidthInRbs (); i++)
    {
      channelRbs.push_back (static_cast<int> (i));
    }

  SetSubChannelsForTransmission (channelRbs);
  Time varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * m_phyMacConfig->GetUlCtrlSymbols ();

  std::list<Ptr<MmWaveControlMessage> > ctrlMsg = GetControlMessages ();

  for (auto ctrlIt = ctrlMsg.begin (); ctrlIt != ctrlMsg.end (); ++ctrlIt)
    {
      Ptr<MmWaveControlMessage> msg = (*ctrlIt);
      m_phyTxedCtrlMsgsTrace (SfnSf(m_frameNum, m_subframeNum, m_slotNum, dci->m_symStart),
                              dci->m_rnti, m_phyMacConfig->GetCcId (), msg);
    }

  NS_LOG_DEBUG ("UE" << m_rnti << " TXing UL CTRL frame for symbols " <<
                +dci->m_symStart << "-" <<
                +(dci->m_symStart + dci->m_numSym - 1) <<
                "\t start " << Simulator::Now () << " end " <<
                (Simulator::Now () + varTtiPeriod - NanoSeconds (1.0)));
  SendCtrlChannels (ctrlMsg, varTtiPeriod - NanoSeconds (1.0));
  return varTtiPeriod;
}

Time
MmWaveUePhy::DlData (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  m_receptionEnabled = true;
  Time varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * dci->m_numSym;

  m_downlinkSpectrumPhy->AddExpectedTb (dci->m_rnti, dci->m_ndi, dci->m_tbSize, dci->m_mcs,
                                        FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask),
                                        dci->m_harqProcess, dci->m_rv, true,
                                        dci->m_symStart, dci->m_numSym);
  m_reportDlTbSize (GetDevice ()->GetObject <MmWaveUeNetDevice> ()->GetImsi (), dci->m_tbSize);
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
  Time varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * dci->m_numSym;
  std::list<Ptr<MmWaveControlMessage> > ctrlMsg = GetControlMessages ();
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
  m_reportUlTbSize (GetDevice ()->GetObject <MmWaveUeNetDevice> ()->GetImsi (), dci->m_tbSize);

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
  if (m_varTtiNum == m_currSlotAllocInfo.m_varTtiAllocInfo.size () - 1)
    {
      // end of slot
      SfnSf retVal = SfnSf (m_frameNum, m_subframeNum, m_slotNum,0).IncreaseNoOfSlots (m_phyMacConfig->GetSlotsPerSubframe (),
                                                                                       m_phyMacConfig->GetSubframesPerFrame ());

      Simulator::Schedule (m_lastSlotStart + m_phyMacConfig->GetSlotPeriod () -
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
      Time nextVarTtiStart = m_phyMacConfig->GetSymbolPeriod () *
                             m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum].m_dci->m_symStart;

      Simulator::Schedule (nextVarTtiStart + m_lastSlotStart - Simulator::Now (), &MmWaveUePhy::StartVarTti, this);
    }

  m_receptionEnabled = false;
}

void
MmWaveUePhy::PhyDataPacketReceived (const Ptr<Packet> &p)
{
  Simulator::ScheduleWithContext (m_netDevice->GetNode ()->GetId (),
                                  MicroSeconds (m_phyMacConfig->GetTbDecodeLatency ()),
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

  m_downlinkSpectrumPhy->StartTxDataFrames (pb, ctrlMsg, duration, slotInd);
}

void
MmWaveUePhy::SendCtrlChannels (std::list<Ptr<MmWaveControlMessage> > ctrlMsg, Time prd)
{
  m_downlinkSpectrumPhy->StartTxDlControlFrames (ctrlMsg,prd);
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
          Ptr<MmWaveUeNetDevice> UeRx = DynamicCast<MmWaveUeNetDevice> (GetDevice ());
          m_reportCurrentCellRsrpSinrTrace (UeRx->GetImsi (), newSinr, newSinr);
        }
    }
}

void
MmWaveUePhy::ReceiveLteDlHarqFeedback (const DlHarqInfo &m)
{
  NS_LOG_FUNCTION (this);
  // generate feedback to eNB and send it through ideal PUCCH
  Ptr<MmWaveDlHarqFeedbackMessage> msg = Create<MmWaveDlHarqFeedbackMessage> ();
  msg->SetDlHarqFeedback (m);
  Simulator::Schedule (MicroSeconds (m_phyMacConfig->GetTbDecodeLatency ()), &MmWaveUePhy::DoSendControlMessage, this, msg);
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
  if (cellId == 0)
    {
      NS_FATAL_ERROR ("Cell ID shall not be zero");
    }

  m_cellId = cellId;

  Ptr<SpectrumValue> noisePsd = GetNoisePowerSpectralDensity ();
  m_downlinkSpectrumPhy->SetNoisePowerSpectralDensity (noisePsd);
  m_downlinkSpectrumPhy->GetSpectrumChannel ()->AddRx (m_downlinkSpectrumPhy);
  m_downlinkSpectrumPhy->SetCellId (m_cellId);
}

AntennaArrayBasicModel::BeamId
MmWaveUePhy::GetBeamId (uint16_t rnti) const
{
  NS_LOG_FUNCTION (this);
  // That's a bad specification: the UE PHY doesn't know anything about its beam id.
  NS_UNUSED (rnti);
  NS_FATAL_ERROR ("ERROR");
}

void
MmWaveUePhy::DoSetDlBandwidth (uint8_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << +dlBandwidth);
}


void
MmWaveUePhy::DoConfigureUplink (uint16_t ulEarfcn, uint8_t ulBandwidth)
{
  NS_LOG_FUNCTION (this << ulEarfcn << +ulBandwidth);
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

}


