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
 *             Sourjya Dutta <sdutta@nyu.edu>
 *             Russell Ford <russell.ford@nyu.edu>
 *            Menglei Zhang <menglei@nyu.edu>
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
#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <cfloat>
#include <cmath>
#include <ns3/simulator.h>
#include <ns3/attribute-accessor-helper.h>
#include <ns3/double.h>
#include <algorithm>

#include "mmwave-enb-phy.h"
#include "mmwave-ue-phy.h"
#include "mmwave-net-device.h"
#include "mmwave-ue-net-device.h"
#include "mmwave-spectrum-value-helper.h"
#include "mmwave-radio-bearer-tag.h"

#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveEnbPhy");

NS_OBJECT_ENSURE_REGISTERED (MmWaveEnbPhy);

MmWaveEnbPhy::MmWaveEnbPhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

MmWaveEnbPhy::MmWaveEnbPhy (Ptr<MmWaveSpectrumPhy> dlPhy, Ptr<MmWaveSpectrumPhy> ulPhy,
                            const Ptr<Node> &n)
  : MmWavePhy (dlPhy, ulPhy),
  m_prevVarTti (0),
  m_prevVarTtiDir (VarTtiAllocInfo::NA),
  m_currSymStart (0)
{
  m_enbCphySapProvider = new MemberLteEnbCphySapProvider<MmWaveEnbPhy> (this);

  m_phyMacConfig = nullptr;

  Simulator::ScheduleWithContext (n->GetId (), MilliSeconds (0), &MmWaveEnbPhy::StartSlot, this);
}

MmWaveEnbPhy::~MmWaveEnbPhy ()
{

}

TypeId
MmWaveEnbPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveEnbPhy")
    .SetParent<MmWavePhy> ()
    .AddConstructor<MmWaveEnbPhy> ()
    .AddAttribute ("TxPower",
                   "Transmission power in dBm",
                   DoubleValue (30.0),
                   MakeDoubleAccessor (&MmWaveEnbPhy::SetTxPower,
                                       &MmWaveEnbPhy::GetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0.\" "
                   "In this model, we consider T0 = 290K.",
                   DoubleValue (5.0),
                   MakeDoubleAccessor (&MmWavePhy::SetNoiseFigure,
                                       &MmWavePhy::GetNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DlSpectrumPhy",
                   "The downlink MmWaveSpectrumPhy associated to this MmWavePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&MmWaveEnbPhy::GetDlSpectrumPhy),
                   MakePointerChecker <MmWaveSpectrumPhy> ())
    .AddAttribute ("UlSpectrumPhy",
                   "The uplink MmWaveSpectrumPhy associated to this MmWavePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&MmWaveEnbPhy::GetUlSpectrumPhy),
                   MakePointerChecker <MmWaveSpectrumPhy> ())
    .AddTraceSource ("UlSinrTrace",
                     "UL SINR statistics.",
                     MakeTraceSourceAccessor (&MmWaveEnbPhy::m_ulSinrTrace),
                     "ns3::UlSinr::TracedCallback")
    .AddAttribute ("MmWavePhyMacCommon", "The associated MmWavePhyMacCommon",
                   PointerValue (), MakePointerAccessor (&MmWaveEnbPhy::m_phyMacConfig),
                   MakePointerChecker<MmWaveEnbPhy> ())
  ;
  return tid;

}

void
MmWaveEnbPhy::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  NS_ABORT_IF (m_phyMacConfig == nullptr);

  Ptr<SpectrumValue> noisePsd = MmWaveSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_phyMacConfig, m_noiseFigure);
  m_downlinkSpectrumPhy->SetNoisePowerSpectralDensity (noisePsd);

  for (unsigned i = 0; i < m_phyMacConfig->GetL1L2CtrlLatency (); i++)
    {   // push elements onto queue for initial scheduling delay
      m_controlMessageQueue.push_back (std::list<Ptr<MmWaveControlMessage> > ());
    }
  //m_slotAllocInfoUpdated = true;

  SfnSf sfnSf = SfnSf (m_frameNum, m_subframeNum, 0, 0);
  std::vector<uint8_t> rbgBitmask (m_phyMacConfig->GetBandwidthInRbg (), 1);

  for (unsigned i = 0; i < m_phyMacConfig->GetL1L2DataLatency (); i++)
    {
      SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);
      auto dciDl = std::make_shared<DciInfoElementTdma> (0, 1, rbgBitmask);
      auto dciUl = std::make_shared<DciInfoElementTdma> (m_phyMacConfig->GetSymbolsPerSlot () - 1, 1, rbgBitmask);

      VarTtiAllocInfo dlCtrlVarTti (VarTtiAllocInfo::DL, VarTtiAllocInfo::CTRL, dciDl);
      VarTtiAllocInfo ulCtrlVarTti (VarTtiAllocInfo::UL, VarTtiAllocInfo::CTRL, dciUl);

      slotAllocInfo.m_varTtiAllocInfo.emplace_back (dlCtrlVarTti);
      slotAllocInfo.m_varTtiAllocInfo.emplace_back (ulCtrlVarTti);

      SetSlotAllocInfo (slotAllocInfo);
      NS_LOG_INFO ("Pushing DL/UL CTRL symbol allocation for " << sfnSf);
      sfnSf = sfnSf.IncreaseNoOfSlots (m_phyMacConfig->GetSlotsPerSubframe (),
                                       m_phyMacConfig->GetSubframesPerFrame ());
    }

  MmWavePhy::DoInitialize ();
}
void
MmWaveEnbPhy::DoDispose (void)
{
  delete m_enbCphySapProvider;
  MmWavePhy::DoDispose ();
}

void
MmWaveEnbPhy::SetmmWaveEnbCphySapUser (LteEnbCphySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_enbCphySapUser = s;
}

LteEnbCphySapProvider*
MmWaveEnbPhy::GetmmWaveEnbCphySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_enbCphySapProvider;
}

AntennaArrayModel::BeamId MmWaveEnbPhy::GetBeamId (uint8_t rnti) const
{
  for (uint8_t i = 0; i < m_deviceMap.size (); i++)
    {
      Ptr<MmWaveUeNetDevice> ueDev = DynamicCast < MmWaveUeNetDevice > (m_deviceMap.at (i));
      uint64_t ueRnti = ueDev->GetPhy (0)->GetRnti ();

      if (ueRnti == rnti)
        {
          Ptr<AntennaArrayModel> antennaArray = DynamicCast<AntennaArrayModel> (GetDlSpectrumPhy ()->GetRxAntenna ());
          return AntennaArrayModel::GetBeamId (antennaArray->GetBeamformingVector (m_deviceMap.at (i)));
        }
    }
  return AntennaArrayModel::BeamId (std::make_pair (0,0));
}

void
MmWaveEnbPhy::SetTxPower (double pow)
{
  m_txPower = pow;
}
double
MmWaveEnbPhy::GetTxPower () const
{
  return m_txPower;
}

void
MmWaveEnbPhy::SetNoiseFigure (double nf)
{
  m_noiseFigure = nf;
}
double
MmWaveEnbPhy::GetNoiseFigure () const
{
  return m_noiseFigure;
}

void
MmWaveEnbPhy::CalcChannelQualityForUe (std::vector <double> sinr, Ptr<MmWaveSpectrumPhy> ue)
{

}

Ptr<SpectrumValue>
MmWaveEnbPhy::CreateTxPowerSpectralDensity (const std::vector<int> &rbIndexVector) const
{
  return MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity (m_phyMacConfig, m_txPower, rbIndexVector);
}

void
MmWaveEnbPhy::SetSubChannels (const std::vector<int> &rbIndexVector)
{
  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensity (rbIndexVector);
  NS_ASSERT (txPsd);
  m_downlinkSpectrumPhy->SetTxPowerSpectralDensity (txPsd);
}

Ptr<MmWaveSpectrumPhy>
MmWaveEnbPhy::GetDlSpectrumPhy () const
{
  return m_downlinkSpectrumPhy;
}

Ptr<MmWaveSpectrumPhy>
MmWaveEnbPhy::GetUlSpectrumPhy () const
{
  return m_uplinkSpectrumPhy;
}

void
MmWaveEnbPhy::StartSlot (void)
{
  NS_LOG_FUNCTION (this);

  m_lastSlotStart = Simulator::Now ();
  m_currSlotAllocInfo = GetSlotAllocInfo (SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0));
  m_currSfNumVarTtis = m_currSlotAllocInfo.m_varTtiAllocInfo.size ();

  NS_ASSERT ((m_currSlotAllocInfo.m_sfnSf.m_frameNum == m_frameNum)
             && (m_currSlotAllocInfo.m_sfnSf.m_subframeNum == m_subframeNum)
             && (m_currSlotAllocInfo.m_sfnSf.m_slotNum == m_slotNum ));

  NS_LOG_INFO ("gNB start slot " << m_currSlotAllocInfo.m_sfnSf << " composed by the following allocations:");
  for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      std::string direction, type;
      if (alloc.m_varTtiType == VarTtiAllocInfo::CTRL)
        {
          type = "CTRL";
        }
      else if (alloc.m_varTtiType == VarTtiAllocInfo::CTRL_DATA)
        {
          type = "CTRL_DATA";
        }
      else
        {
          type = "DATA";
        }

      if (alloc.m_tddMode == VarTtiAllocInfo::UL)
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

  if (m_slotNum == 0)
    {
      if (m_subframeNum == 0)   // send MIB at the beginning of each frame
        {
          LteRrcSap::MasterInformationBlock mib;
          mib.dlBandwidth = (uint8_t)4;
          mib.systemFrameNumber = 1;
          Ptr<MmWaveMibMessage> mibMsg = Create<MmWaveMibMessage> ();
          mibMsg->SetMib (mib);
          if (m_controlMessageQueue.empty ())
            {
              std::list<Ptr<MmWaveControlMessage> > l;
              m_controlMessageQueue.push_back (l);
            }
          m_controlMessageQueue.at (0).push_back (mibMsg);
        }
      else if (m_subframeNum == 5)   // send SIB at beginning of second half-frame
        {
          Ptr<MmWaveSib1Message> msg = Create<MmWaveSib1Message> ();
          msg->SetSib1 (m_sib1);
          m_controlMessageQueue.at (0).push_back (msg);
        }
    }

  StartVarTti ();
}


void
MmWaveEnbPhy::StoreRBGAllocation (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  auto itAlloc = m_rbgAllocationPerSym.find (dci->m_symStart);
  if (itAlloc == m_rbgAllocationPerSym.end ())
    {
      itAlloc = m_rbgAllocationPerSym.insert (std::make_pair (dci->m_symStart, dci->m_rbgBitmask)).first;
    }
  else
    {
      auto & existingRBGBitmask = itAlloc->second;
      NS_ASSERT (existingRBGBitmask.size () == m_phyMacConfig->GetBandwidthInRbg ());
      NS_ASSERT (dci->m_rbgBitmask.size () == m_phyMacConfig->GetBandwidthInRbg ());
      for (uint32_t i = 0; i < m_phyMacConfig->GetBandwidthInRbg (); ++i)
        {
          existingRBGBitmask.at (i) = existingRBGBitmask.at (i) | dci->m_rbgBitmask.at (i);
        }
    }
}

std::list <Ptr<MmWaveControlMessage> >
MmWaveEnbPhy::RetrieveMsgsFromDCIs (const SfnSf &sfn)
{
  std::list <Ptr<MmWaveControlMessage> > ctrlMsgs;

  // find all DL DCI elements in the current slot and create the DL RBG bitmask
  uint8_t lastSymbolDl = 0, lastSymbolUl = 0;

  NS_LOG_INFO ("Retrieving DL allocation for slot " << m_currSlotAllocInfo.m_sfnSf <<
               " with a total of " << m_currSlotAllocInfo.m_varTtiAllocInfo.size () <<
               " allocations");
  for (const auto & dlAlloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      if (dlAlloc.m_varTtiType != VarTtiAllocInfo::CTRL
          && dlAlloc.m_tddMode == VarTtiAllocInfo::DL)
        {
          auto dciElem = dlAlloc.m_dci;
          NS_ASSERT (dciElem->m_format == DciInfoElementTdma::DL);
          NS_ASSERT (dciElem->m_tbSize > 0);
          NS_ASSERT (dciElem->m_symStart >= lastSymbolDl);
          NS_ASSERT_MSG (dciElem->m_symStart + dciElem->m_numSym <= m_phyMacConfig->GetSymbolsPerSlot (),
                         "symStart: " << static_cast<uint32_t> (dciElem->m_symStart) <<
                         " numSym: " << static_cast<uint32_t> (dciElem->m_numSym) <<
                         " symPerSlot: " << static_cast<uint32_t> (m_phyMacConfig->GetSymbolsPerSlot ()));
          lastSymbolDl = dciElem->m_symStart;

          StoreRBGAllocation (dciElem);

          Ptr<MmWaveTdmaDciMessage> dciMsg = Create<MmWaveTdmaDciMessage> (dciElem);
          dciMsg->SetSfnSf (sfn);

          ctrlMsgs.push_back (dciMsg);
          NS_LOG_INFO ("To send, DL DCI for UE " << dciElem->m_rnti);
        }
    }

  // TODO: REDUCE AMOUNT OF DUPLICATE CODE
  // Get all the DCI for UL. We retrieve that DCIs from a future slot
  // if UlSchedDelay > 0, or this slot if == 0.
  const SfnSf ulSfn = sfn.CalculateUplinkSlot (m_phyMacConfig->GetUlSchedDelay (),
                                               m_phyMacConfig->GetSlotsPerSubframe (),
                                               m_phyMacConfig->GetSubframesPerFrame ());
  if (m_phyMacConfig->GetUlSchedDelay () > 0)
    {
      if (SlotExists (ulSfn))
        {
          SlotAllocInfo & ulSlot = PeekSlotAllocInfo (ulSfn);
          NS_LOG_INFO ("Retrieving UL allocation for slot " << ulSlot.m_sfnSf <<
                       " with a total of " << ulSlot.m_varTtiAllocInfo.size () <<
                       " allocations");
          for (const auto & ulAlloc : ulSlot.m_varTtiAllocInfo)
            {
              if (ulAlloc.m_varTtiType != VarTtiAllocInfo::CTRL
                  && ulAlloc.m_tddMode == VarTtiAllocInfo::UL)
                {
                  auto dciElem = ulAlloc.m_dci;

                  NS_ASSERT (dciElem->m_format == DciInfoElementTdma::UL);
                  NS_ASSERT (dciElem->m_tbSize > 0);
                  NS_ASSERT_MSG (dciElem->m_symStart >= lastSymbolUl,
                                 "symStart: " << static_cast<uint32_t> (dciElem->m_symStart) <<
                                 " lastSymbolUl " << static_cast<uint32_t> (lastSymbolUl));

                  NS_ASSERT (dciElem->m_symStart + dciElem->m_numSym <= m_phyMacConfig->GetSymbolsPerSlot ());
                  lastSymbolUl = dciElem->m_symStart;

                  Ptr<MmWaveTdmaDciMessage> dciMsg = Create<MmWaveTdmaDciMessage> (dciElem);
                  dciMsg->SetSfnSf (sfn);
                  ctrlMsgs.push_back (dciMsg);

                  NS_LOG_INFO ("To send, UL DCI for UE " << dciElem->m_rnti);
                }
            }
        }
    }
  else
    {
      for (const auto & ulAlloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
        {
          if (ulAlloc.m_varTtiType != VarTtiAllocInfo::CTRL
              && ulAlloc.m_tddMode == VarTtiAllocInfo::UL)
            {
              auto dciElem = ulAlloc.m_dci;

              NS_ASSERT (dciElem->m_format == DciInfoElementTdma::UL);
              NS_ASSERT (dciElem->m_tbSize > 0);
              NS_ASSERT_MSG (dciElem->m_symStart >= lastSymbolUl,
                             "symStart: " << static_cast<uint32_t> (dciElem->m_symStart) <<
                             " lastSymbolUl " << static_cast<uint32_t> (lastSymbolUl));

              NS_ASSERT (dciElem->m_symStart + dciElem->m_numSym <= m_phyMacConfig->GetSymbolsPerSlot ());
              lastSymbolUl = dciElem->m_symStart;

              Ptr<MmWaveTdmaDciMessage> dciMsg = Create<MmWaveTdmaDciMessage> (dciElem);
              dciMsg->SetSfnSf (sfn);
              ctrlMsgs.push_back (dciMsg);

              NS_LOG_INFO ("To send, UL DCI for UE " << dciElem->m_rnti);
            }
        }
    }

  return ctrlMsgs;
}

void
MmWaveEnbPhy::StartVarTti (void)
{
  NS_LOG_FUNCTION (this);

  //assume the control signal is omni
  Ptr<AntennaArrayModel> antennaArray = DynamicCast<AntennaArrayModel> (GetDlSpectrumPhy ()->GetRxAntenna ());
  antennaArray->ChangeToOmniTx ();

  VarTtiAllocInfo & currVarTti = m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum];
  m_currSymStart = currVarTti.m_dci->m_symStart;
  SfnSf sfn = SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum);
  NS_LOG_INFO ("Starting VarTti on the AIR " << sfn);

  Time varTtiPeriod;

  if (m_varTtiNum == 0)  // DL control var tti
    {
      // Start with a clean RBG allocation bitmask
      m_rbgAllocationPerSym.clear ();

      // create control messages to be transmitted in DL-Control period
      std::list <Ptr<MmWaveControlMessage> > ctrlMsgs = GetControlMessages ();
      ctrlMsgs.merge (RetrieveMsgsFromDCIs (sfn));

      // TX control period
      varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * m_phyMacConfig->GetDlCtrlSymbols ();

      NS_LOG_DEBUG ("ENB TXing DL CTRL frame " << m_frameNum <<
                    " subframe " << static_cast<uint32_t> (m_subframeNum) <<
                    " slot " << static_cast<uint32_t> (m_slotNum) <<
                    " symbols "  << static_cast<uint32_t> (currVarTti.m_dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (currVarTti.m_dci->m_symStart + currVarTti.m_dci->m_numSym - 1) <<
                    " start " << Simulator::Now () <<
                    " end " << Simulator::Now () + varTtiPeriod - NanoSeconds (1.0));

      SendCtrlChannels (ctrlMsgs, varTtiPeriod - NanoSeconds (1.0)); // -1 ns ensures control ends before data period
    }
  else if (m_varTtiNum == m_currSfNumVarTtis - 1)   // UL control var tti
    {
      varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * m_phyMacConfig->GetUlCtrlSymbols ();

      NS_LOG_DEBUG ("ENB RXng UL CTRL frame " << m_frameNum <<
                    " subframe " << static_cast<uint32_t> (m_subframeNum) <<
                    " slot " << static_cast<uint32_t> (m_slotNum) <<
                    " symbols "  << static_cast<uint32_t> (currVarTti.m_dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (currVarTti.m_dci->m_symStart + currVarTti.m_dci->m_numSym - 1) <<
                    " start " << Simulator::Now () <<
                    " end " << Simulator::Now () + varTtiPeriod);
    }
  else if (currVarTti.m_tddMode == VarTtiAllocInfo::DL)      // transmit DL var tti
    {
      varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * currVarTti.m_dci->m_numSym;
      NS_ASSERT (currVarTti.m_tddMode == VarTtiAllocInfo::DL);

      Ptr<PacketBurst> pktBurst = GetPacketBurst (SfnSf (m_frameNum, m_subframeNum, m_slotNum, currVarTti.m_dci->m_symStart));
      if (pktBurst && pktBurst->GetNPackets () > 0)
        {
          std::list< Ptr<Packet> > pkts = pktBurst->GetPackets ();
          MmWaveMacPduTag macTag;
          pkts.front ()->PeekPacketTag (macTag);
          NS_ASSERT ((macTag.GetSfn ().m_slotNum == m_slotNum) && (macTag.GetSfn ().m_varTtiNum == currVarTti.m_dci->m_symStart));
        }
      else
        {
          // sometimes the UE will be scheduled when no data is queued
          // in this case, send an empty PDU
          MmWaveMacPduTag tag (SfnSf (m_frameNum, m_subframeNum, m_slotNum, currVarTti.m_dci->m_symStart));
          Ptr<Packet> emptyPdu = Create <Packet> ();
          MmWaveMacPduHeader header;
          MacSubheader subheader (3, 0);    // lcid = 3, size = 0
          header.AddSubheader (subheader);
          emptyPdu->AddHeader (header);
          emptyPdu->AddPacketTag (tag);
          LteRadioBearerTag bearerTag (currVarTti.m_dci->m_rnti, 3, 0);
          emptyPdu->AddPacketTag (bearerTag);
          pktBurst = CreateObject<PacketBurst> ();
          pktBurst->AddPacket (emptyPdu);
        }

      NS_LOG_DEBUG ("ENB TXing DL DATA frame " << m_frameNum <<
                    " subframe " << static_cast<uint32_t> (m_subframeNum) <<
                    " slot " << static_cast<uint32_t> (m_slotNum) <<
                    " symbols "  << static_cast<uint32_t> (currVarTti.m_dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (currVarTti.m_dci->m_symStart + currVarTti.m_dci->m_numSym - 1) <<
                    " start " << Simulator::Now () + NanoSeconds (1) <<
                    " end " << Simulator::Now () + varTtiPeriod - NanoSeconds (2.0));

      Simulator::Schedule (NanoSeconds (1.0), &MmWaveEnbPhy::SendDataChannels, this, pktBurst, varTtiPeriod - NanoSeconds (2.0), currVarTti);
    }
  else if (currVarTti.m_tddMode == VarTtiAllocInfo::UL)    // receive UL var tti
    {
      // Assert: we expect TDMA in UL
      NS_ASSERT (currVarTti.m_dci->m_rbgBitmask.size () == m_phyMacConfig->GetBandwidthInRbg ());
      NS_ASSERT (static_cast<uint32_t> (std::count (currVarTti.m_dci->m_rbgBitmask.begin (),
                                                    currVarTti.m_dci->m_rbgBitmask.end (), 1U)) ==
                 currVarTti.m_dci->m_rbgBitmask.size ());

      varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * currVarTti.m_dci->m_numSym;

      //NS_LOG_DEBUG ("Var tti " << (uint8_t)m_varTtiNum << " scheduled for Uplink");
      m_downlinkSpectrumPhy->AddExpectedTb (currVarTti.m_dci->m_rnti, currVarTti.m_dci->m_ndi,
                                            currVarTti.m_dci->m_tbSize, currVarTti.m_dci->m_mcs,
                                            FromRBGBitmaskToRBAssignment (currVarTti.m_dci->m_rbgBitmask),
                                            currVarTti.m_dci->m_harqProcess, currVarTti.m_dci->m_rv, false,
                                            currVarTti.m_dci->m_symStart, currVarTti.m_dci->m_numSym);

      bool found = false;
      for (uint8_t i = 0; i < m_deviceMap.size (); i++)
        {
          Ptr<MmWaveUeNetDevice> ueDev = DynamicCast < MmWaveUeNetDevice > (m_deviceMap.at (i));
          uint64_t ueRnti = ueDev->GetPhy (0)->GetRnti ();
          if (currVarTti.m_dci->m_rnti == ueRnti)
            {
              Ptr<AntennaArrayModel> antennaArray = DynamicCast<AntennaArrayModel> (GetDlSpectrumPhy ()->GetRxAntenna ());
              antennaArray->ChangeBeamformingVector (m_deviceMap.at (i));
              found = true;
              break;
            }
        }
      NS_ASSERT (found);

      NS_LOG_DEBUG ("ENB RXing UL DATA frame " << m_frameNum <<
                    " subframe " << static_cast<uint32_t> (m_subframeNum) <<
                    " slot " << static_cast<uint32_t> (m_slotNum) <<
                    " symbols "  << static_cast<uint32_t> (currVarTti.m_dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (currVarTti.m_dci->m_symStart + currVarTti.m_dci->m_numSym - 1) <<
                    " start " << Simulator::Now () <<
                    " end " << Simulator::Now () + varTtiPeriod);
    }

  m_prevVarTtiDir = currVarTti.m_tddMode;

  NS_LOG_DEBUG ("Asking MAC for SlotIndication for frame" << SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum));
  m_phySapUser->SlotIndication (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum));    // trigger MAC

  Simulator::Schedule (varTtiPeriod, &MmWaveEnbPhy::EndVarTti, this);
}

void
MmWaveEnbPhy::EndVarTti (void)
{
  NS_LOG_FUNCTION (this << Simulator::Now ().GetSeconds ());
  auto lastDci = m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum].m_dci;
  NS_LOG_INFO ("DCI started at symbol " << static_cast<uint32_t> (lastDci->m_symStart) <<
               " which lasted for " << static_cast<uint32_t> (lastDci->m_numSym) <<
               " symbols finished");

  Ptr<AntennaArrayModel> antennaArray = DynamicCast<AntennaArrayModel> (GetDlSpectrumPhy ()->GetRxAntenna ());

  antennaArray->ChangeToOmniTx ();

  if (m_varTtiNum == m_currSfNumVarTtis - 1)
    {
      m_varTtiNum = 0;
      EndSlot ();
    }
  else
    {
      m_varTtiNum++;
      auto currentDci = m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum].m_dci;

      if (lastDci->m_symStart == currentDci->m_symStart)
        {
          NS_LOG_INFO ("DCI " << static_cast <uint32_t> (m_varTtiNum) <<
                       " of " << m_currSlotAllocInfo.m_varTtiAllocInfo.size () - 1 <<
                       " for UE " << currentDci->m_rnti << " starts from symbol " <<
                       static_cast<uint32_t> (currentDci->m_symStart) << " ignoring at PHY");
          EndVarTti ();
        }
      else
        {
          auto nextVarTtiStart = m_phyMacConfig->GetSymbolPeriod () * currentDci->m_symStart;

          NS_LOG_INFO ("DCI " << static_cast <uint32_t> (m_varTtiNum) <<
                       " of " << m_currSlotAllocInfo.m_varTtiAllocInfo.size () - 1 <<
                       " for UE " << currentDci->m_rnti << " starts from symbol " <<
                       static_cast<uint32_t> (currentDci->m_symStart) << " scheduling at PHY, at " <<
                       nextVarTtiStart + m_lastSlotStart << " where last slot start = " <<
                       m_lastSlotStart << " nextVarTti " << nextVarTtiStart);

          Simulator::Schedule (nextVarTtiStart + m_lastSlotStart - Simulator::Now (),
                               &MmWaveEnbPhy::StartVarTti, this);
        }
      // Do not put any code here (tail recursion)
    }
  // Do not put any code here (tail recursion)
}

void
MmWaveEnbPhy::EndSlot (void)
{
  NS_LOG_FUNCTION (this << Simulator::Now ().GetSeconds ());

  Time slotStart = m_lastSlotStart + m_phyMacConfig->GetSlotPeriod () - Simulator::Now ();

  if (slotStart < Seconds (0))
    {
      NS_FATAL_ERROR ("lastStart=" << m_lastSlotStart + m_phyMacConfig->GetSlotPeriod () <<
                      " now " <<  Simulator::Now () << " slotStart value" << slotStart);
    }

  m_varTtiNum = 0;

  SfnSf sfnf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum);

  SfnSf retVal = sfnf.IncreaseNoOfSlots (m_phyMacConfig->GetSlotsPerSubframe (),m_phyMacConfig->GetSubframesPerFrame ());

  m_frameNum = retVal.m_frameNum;
  m_subframeNum = retVal.m_subframeNum;
  m_slotNum = retVal.m_slotNum;

  Simulator::Schedule (slotStart, &MmWaveEnbPhy::StartSlot, this);
}

void
MmWaveEnbPhy::SendDataChannels (Ptr<PacketBurst> pb, Time varTtiPeriod, VarTtiAllocInfo& varTtiInfo)
{
  if (varTtiInfo.m_isOmni)
    {
      Ptr<AntennaArrayModel> antennaArray = DynamicCast<AntennaArrayModel> (GetDlSpectrumPhy ()->GetRxAntenna ());
      antennaArray->ChangeToOmniTx ();
    }
  else
    {   // update beamforming vectors (currently supports 1 user only)
        //std::map<uint16_t, std::vector<unsigned> >::iterator ueRbIt = varTtiInfo.m_ueRbMap.begin();
        //uint16_t rnti = ueRbIt->first;
      bool found = false;
      for (uint8_t i = 0; i < m_deviceMap.size (); i++)
        {
          Ptr<MmWaveUeNetDevice> ueDev = DynamicCast<MmWaveUeNetDevice> (m_deviceMap.at (i));
          uint64_t ueRnti = ueDev->GetPhy (0)->GetRnti ();
          //NS_LOG_UNCOND ("Scheduled rnti:"<<rnti <<" ue rnti:"<< ueRnti);
          if (varTtiInfo.m_dci->m_rnti == ueRnti)
            {
              //NS_LOG_UNCOND ("Change Beamforming Vector");
              Ptr<AntennaArrayModel> antennaArray = DynamicCast<AntennaArrayModel> (GetDlSpectrumPhy ()->GetRxAntenna ());
              antennaArray->ChangeBeamformingVector (m_deviceMap.at (i));
              found = true;
              break;
            }
        }
      NS_ABORT_IF (!found);
    }

  // in the map we stored the RBG allocated by the MAC for this symbol.
  // If the transmission last n symbol (n > 1 && n < 12) the SetSubChannels
  // doesn't need to be called again. In fact, SendDataChannels will be
  // invoked only when the symStart changes.
  SetSubChannels (FromRBGBitmaskToRBAssignment (m_rbgAllocationPerSym.at (varTtiInfo.m_dci->m_symStart)));

  std::list<Ptr<MmWaveControlMessage> > ctrlMsgs;
  m_downlinkSpectrumPhy->StartTxDataFrames (pb, ctrlMsgs, varTtiPeriod, varTtiInfo.m_dci->m_symStart);
}

void
MmWaveEnbPhy::SendCtrlChannels (std::list<Ptr<MmWaveControlMessage> > ctrlMsgs, Time varTtiPeriod)
{
  NS_LOG_FUNCTION (this << "Send Ctrl");

  std::vector <int> fullBwRb (m_phyMacConfig->GetBandwidthInRbs ());
  // The first time set the right values for the phy
  for (uint32_t i = 0; i < m_phyMacConfig->GetBandwidthInRbs (); ++i)
    {
      fullBwRb.at (i) = static_cast<int> (i);
    }

  SetSubChannels (fullBwRb);

  m_downlinkSpectrumPhy->StartTxDlControlFrames (ctrlMsgs, varTtiPeriod);
}

bool
MmWaveEnbPhy::AddUePhy (uint64_t imsi, Ptr<NetDevice> ueDevice)
{
  NS_LOG_FUNCTION (this << imsi);
  std::set <uint64_t>::iterator it;
  it = m_ueAttached.find (imsi);

  if (it == m_ueAttached.end ())
    {
      m_ueAttached.insert (imsi);
      m_deviceMap.push_back (ueDevice);
      return (true);
    }
  else
    {
      NS_LOG_ERROR ("Programming error...UE already attached");
      return (false);
    }
}

void
MmWaveEnbPhy::PhyDataPacketReceived (Ptr<Packet> p)
{
  Simulator::ScheduleWithContext (m_netDevice->GetNode ()->GetId (),
                                  MicroSeconds (m_phyMacConfig->GetTbDecodeLatency ()),
                                  &MmWaveEnbPhySapUser::ReceivePhyPdu,
                                  m_phySapUser,
                                  p);
  //  m_phySapUser->ReceivePhyPdu(p);
}

void
MmWaveEnbPhy::GenerateDataCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr);

  Values::const_iterator it;
  MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi;
  ulcqi.m_ulCqi.m_type = UlCqiInfo::PUSCH;
  int i = 0;
  for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
    {
      //   double sinrdb = 10 * std::log10 ((*it));
      //       NS_LOG_DEBUG ("ULCQI RB " << i << " value " << sinrdb);
      // convert from double to fixed point notaltion Sxxxxxxxxxxx.xxx
      //   int16_t sinrFp = LteFfConverter::double2fpS11dot3 (sinrdb);
      ulcqi.m_ulCqi.m_sinr.push_back (*it);
      i++;
    }

  // here we use the start symbol index of the var tti in place of the var tti index because the absolute UL var tti index is
  // not known to the scheduler when m_allocationMap gets populated
  ulcqi.m_sfnSf = SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_currSymStart);
  SpectrumValue newSinr = sinr;
  m_ulSinrTrace (0, newSinr, newSinr);
  m_phySapUser->UlCqiReport (ulcqi);
}


void
MmWaveEnbPhy::PhyCtrlMessagesReceived (std::list<Ptr<MmWaveControlMessage> > msgList)
{
  NS_LOG_FUNCTION (this);

  std::list<Ptr<MmWaveControlMessage> >::iterator ctrlIt = msgList.begin ();

  while (ctrlIt != msgList.end ())
    {
      Ptr<MmWaveControlMessage> msg = (*ctrlIt);

      if (msg->GetMessageType () == MmWaveControlMessage::DL_CQI)
        {
          NS_LOG_INFO ("received CQI");
          m_phySapUser->ReceiveControlMessage (msg);
        }
      else if (msg->GetMessageType () == MmWaveControlMessage::BSR)
        {
          NS_LOG_INFO ("received BSR");
          m_phySapUser->ReceiveControlMessage (msg);
        }
      else if (msg->GetMessageType () == MmWaveControlMessage::RACH_PREAMBLE)
        {
          NS_LOG_INFO ("received RACH_PREAMBLE");
          NS_ASSERT (m_cellId > 0);

          Ptr<MmWaveRachPreambleMessage> rachPreamble = DynamicCast<MmWaveRachPreambleMessage> (msg);
          m_phySapUser->ReceiveRachPreamble (rachPreamble->GetRapId ());
        }
      else if (msg->GetMessageType () == MmWaveControlMessage::DL_HARQ)
        {

          Ptr<MmWaveDlHarqFeedbackMessage> dlharqMsg = DynamicCast<MmWaveDlHarqFeedbackMessage> (msg);
          DlHarqInfo dlharq = dlharqMsg->GetDlHarqFeedback ();

          NS_LOG_INFO ("cellId:" << m_cellId << Simulator::Now () <<
                       " received DL_HARQ from: " << dlharq.m_rnti);
          // check whether the UE is connected

          if (m_ueAttachedRnti.find (dlharq.m_rnti) != m_ueAttachedRnti.end ())
            {
              m_phySapUser->ReceiveControlMessage (msg);
            }
        }
      else
        {
          m_phySapUser->ReceiveControlMessage (msg);
        }

      ctrlIt++;
    }

}


////////////////////////////////////////////////////////////
/////////                     sap                 /////////
///////////////////////////////////////////////////////////

void
MmWaveEnbPhy::DoSetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << (uint32_t) ulBandwidth << (uint32_t) dlBandwidth);
}

void
MmWaveEnbPhy::DoSetEarfcn (uint16_t ulEarfcn, uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << ulEarfcn << dlEarfcn);
}


void
MmWaveEnbPhy::DoAddUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  bool success = AddUePhy (rnti);
  NS_ASSERT_MSG (success, "AddUePhy() failed");

}

bool
MmWaveEnbPhy::AddUePhy (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::set <uint16_t>::iterator it;
  it = m_ueAttachedRnti.find (rnti);
  if (it == m_ueAttachedRnti.end ())
    {
      m_ueAttachedRnti.insert (rnti);
      return (true);
    }
  else
    {
      NS_LOG_ERROR ("UE already attached");
      return (false);
    }
}

void
MmWaveEnbPhy::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);

  std::set <uint16_t>::iterator it = m_ueAttachedRnti.find (rnti);
  if (it != m_ueAttachedRnti.end ())
    {
      m_ueAttachedRnti.erase (it);
    }
  else
    {
      NS_FATAL_ERROR ("Impossible to remove UE, not attached!");
    }
}

void
MmWaveEnbPhy::DoSetPa (uint16_t rnti, double pa)
{
  NS_LOG_FUNCTION (this << rnti);
}

void
MmWaveEnbPhy::DoSetTransmissionMode (uint16_t  rnti, uint8_t txMode)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t)txMode);
  // UL supports only SISO MODE
}

void
MmWaveEnbPhy::DoSetSrsConfigurationIndex (uint16_t  rnti, uint16_t srcCi)
{
  NS_LOG_FUNCTION (this);
}


void
MmWaveEnbPhy::DoSetMasterInformationBlock (LteRrcSap::MasterInformationBlock mib)
{
  NS_LOG_FUNCTION (this);
  //m_mib = mib;
}


void
MmWaveEnbPhy::DoSetSystemInformationBlockType1 (LteRrcSap::SystemInformationBlockType1 sib1)
{
  NS_LOG_FUNCTION (this);
  m_sib1 = sib1;
}

int8_t
MmWaveEnbPhy::DoGetReferenceSignalPower () const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
}

void
MmWaveEnbPhy::SetPhySapUser (MmWaveEnbPhySapUser* ptr)
{
  m_phySapUser = ptr;
}

void
MmWaveEnbPhy::SetHarqPhyModule (Ptr<MmWaveHarqPhy> harq)
{
  m_harqPhyModule = harq;
}

void
MmWaveEnbPhy::ReceiveUlHarqFeedback (UlHarqInfo mes)
{
  NS_LOG_FUNCTION (this);
  // forward to scheduler
  if (m_ueAttachedRnti.find (mes.m_rnti) != m_ueAttachedRnti.end ())
    {
      m_phySapUser->UlHarqFeedback (mes);
    }
}

}
