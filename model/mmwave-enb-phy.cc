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
 */

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);

#include <ns3/log.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/node.h>
#include <algorithm>
#include <functional>
#include <string>

#include "mmwave-enb-phy.h"
#include "mmwave-ue-phy.h"
#include "mmwave-net-device.h"
#include "mmwave-ue-net-device.h"
#include "mmwave-enb-net-device.h"
#include "mmwave-radio-bearer-tag.h"
#include "nr-ch-access-manager.h"

#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/pointer.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include "beam-manager.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveEnbPhy");

NS_OBJECT_ENSURE_REGISTERED (MmWaveEnbPhy);

MmWaveEnbPhy::MmWaveEnbPhy ():
    m_n0Delay (0),
    m_n1Delay (4)
{
  NS_LOG_FUNCTION (this);
  m_enbCphySapProvider = new MemberLteEnbCphySapProvider<MmWaveEnbPhy> (this);
}

MmWaveEnbPhy::~MmWaveEnbPhy ()
{
  delete m_enbCphySapProvider;
}

TypeId
MmWaveEnbPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveEnbPhy")
    .SetParent<MmWavePhy> ()
    .AddConstructor<MmWaveEnbPhy> ()
    .AddAttribute ("TxPower",
                   "Transmission power in dBm",
                   DoubleValue (4.0),
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
    .AddAttribute ("SpectrumPhy",
                   "The downlink MmWaveSpectrumPhy associated to this MmWavePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&MmWavePhy::GetSpectrumPhy),
                   MakePointerChecker <MmWaveSpectrumPhy> ())
    .AddTraceSource ("UlSinrTrace",
                     "UL SINR statistics.",
                     MakeTraceSourceAccessor (&MmWaveEnbPhy::m_ulSinrTrace),
                     "ns3::UlSinr::TracedCallback")
    .AddTraceSource ("EnbPhyRxedCtrlMsgsTrace",
                     "Enb PHY Rxed Control Messages Traces.",
                     MakeTraceSourceAccessor (&MmWaveEnbPhy::m_phyRxedCtrlMsgsTrace),
                     "ns3::MmWavePhyRxTrace::RxedEnbPhyCtrlMsgsTracedCallback")
    .AddTraceSource ("EnbPhyTxedCtrlMsgsTrace",
                     "Enb PHY Txed Control Messages Traces.",
                     MakeTraceSourceAccessor (&MmWaveEnbPhy::m_phyTxedCtrlMsgsTrace),
                     "ns3::MmWavePhyRxTrace::TxedEnbPhyCtrlMsgsTracedCallback")
    .AddAttribute ("N0Delay",
                   "Minimum processing delay needed to decode DL DCI and decode DL data",
                    UintegerValue (0),
                    MakeUintegerAccessor (&MmWaveEnbPhy::SetN0Delay,
                                          &MmWaveEnbPhy::GetN0Delay),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("N1Delay",
                   "Minimum processing delay (UE side) from the end of DL Data reception to "
                   "the earliest possible start of the corresponding ACK/NACK transmission",
                    UintegerValue (4),
                    MakeUintegerAccessor (&MmWaveEnbPhy::SetN1Delay,
                                          &MmWaveEnbPhy::GetN1Delay),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("N2Delay",
                   "Minimum processing delay needed to decode UL DCI and prepare UL data",
                   UintegerValue (2),
                   MakeUintegerAccessor (&MmWaveEnbPhy::SetN2Delay,
                                         &MmWaveEnbPhy::GetN2Delay),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("TbDecodeLatency",
                   "Transport block decode latency",
                    TimeValue (MicroSeconds (100)),
                    MakeTimeAccessor (&MmWavePhy::SetTbDecodeLatency,
                                      &MmWavePhy::GetTbDecodeLatency),
                    MakeTimeChecker ())
    .AddAttribute ("Numerology",
                   "The 3GPP numerology to be used",
                   UintegerValue (0),
                   MakeUintegerAccessor (&MmWavePhy::SetNumerology,
                                         &MmWavePhy::GetNumerology),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("SymbolsPerSlot",
                   "Number of symbols in one slot",
                   UintegerValue (14),
                   MakeUintegerAccessor (&MmWavePhy::SetSymbolsPerSlot,
                                         &MmWavePhy::GetSymbolsPerSlot),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Pattern",
                   "The slot pattern",
                   StringValue ("F|F|F|F|F|F|F|F|F|F|"),
                   MakeStringAccessor (&MmWaveEnbPhy::SetPattern,
                                       &MmWaveEnbPhy::GetPattern),
                   MakeStringChecker ())
    ;
  return tid;

}

uint32_t
MmWaveEnbPhy::GetNumRbPerRbg () const
{
  return m_phySapUser->GetNumRbPerRbg();
}

uint32_t
MmWaveEnbPhy::GetChannelBandwidth () const
{
  // m_channelBandwidth is in kHz * 100
  return m_channelBandwidth * 1000 * 100;
}

/**
 * \brief An intelligent way to calculate the modulo
 * \param n Number
 * \param m Modulo
 * \return n+=m until n < 0
 */
static uint32_t modulo (int n, uint32_t m)
{
  if (n >= 0)
    {
      return static_cast<uint32_t> (n) % m;
    }
  else
    {
      while (n < 0) {
          n += m;
        }
      return static_cast<uint32_t> (n);
    }
}

/**
 * \brief Return the slot in which the DL HARQ Feedback should be sent, according to the parameter N1
 * \param pattern The TDD pattern
 * \param pos The position of the data inside the pattern for which we want to find where the feedback should be sent
 * \param n1 The N1 parameter
 * \return k1 (after how many slots the DL HARQ Feedback should be sent)
 *
 * Please note that for the LTE TDD case, although the calculation follows the
 * logic of Table 10.1-1 of TS 36.213, some configurations are simplified in order
 * to avoid having a table from where we take the K1 values. In particular, for
 * configurations 3, 4 and 6 (starting form 0), the specification splits the
 * HARQ feedbacks among all UL subframes in an equal (as much as possible) manner.
 * This tactic is ommitted in this implementation.
 */
static int32_t
ReturnHarqSlot (const std::vector<LteNrTddSlotType> &pattern, uint32_t pos, uint32_t n1)
{
  int32_t k1 = static_cast<int32_t> (n1);

  uint32_t index = modulo (static_cast<int> (pos) + k1, static_cast<uint32_t> (pattern.size ()));

  while (pattern[index] < LteNrTddSlotType::F)
    {
      k1++;
      index = modulo (static_cast<int> (pos) + k1, static_cast<uint32_t> (pattern.size ()));
      NS_ASSERT (index < pattern.size ());
    }

  return k1;
}

struct DciKPair
{
  uint32_t indexDci {0};
  uint32_t k {0};
};

/**
 * \brief Return the slot in which the DCI should be send, according to the parameter n,
 * along with the number of slots required to add to the current slot to get the slot of DCI (k0/k2)
 * \param pattern The TDD pattern
 * \param pos The position inside the pattern for which we want to check where the DCI should be sent
 * \param n The N parameter (equal to N0 or N2, depending if it is DL or UL)
 * \return The slot position in which the DCI for the position specified should be sent and the k0/k2
 */
static DciKPair
ReturnDciSlot (const std::vector<LteNrTddSlotType> &pattern, uint32_t pos, uint32_t n)
{
  DciKPair ret;
  ret.k = n;
  ret.indexDci = modulo (static_cast<int> (pos) - static_cast<int> (ret.k),
                           static_cast<uint32_t> (pattern.size ()));

  while (pattern[ret.indexDci] > LteNrTddSlotType::F)
    {
      ret.k++;
      ret.indexDci = modulo (static_cast<int> (pos) - static_cast<int> (ret.k),
                      static_cast<uint32_t> (pattern.size ()));
      NS_ASSERT (ret.indexDci < pattern.size ());
    }

  return ret;
}

/**
 * \brief Generates the map tosendDl/Ul that holds the information of the DCI Slot and the
 * corresponding k0/k2 value, and the generateDl/Ul that includes the L1L2CtrlLatency.
 * \param pattern The TDD pattern
 * \param pattern The pattern to analyze
 * \param toSend The structure toSendDl/tosendUl to fill
 * \param generate The structure generateDl/generateUl to fill
 * \param pos The position inside the pattern for which we want to check where the DCI should be sent
 * \param n The N parameter (equal to N0 or N2, depending if it is DL or UL)
 * \param l1l2CtrlLatency L1L2CtrlLatency of the system
 */
static void GenerateDciMaps (const std::vector<LteNrTddSlotType> &pattern,
                             std::map<uint32_t, std::vector<uint32_t>> *toSend,
                             std::map<uint32_t, std::vector<uint32_t>> *generate,
                             uint32_t pos, uint32_t n, uint32_t l1l2CtrlLatency)
{
  auto dciSlot = ReturnDciSlot (pattern, pos, n);
  uint32_t indexGen = modulo (static_cast<int>(dciSlot.indexDci) - static_cast<int> (l1l2CtrlLatency),
                              static_cast<uint32_t> (pattern.size ()));
  uint32_t kWithCtrlLatency = static_cast<uint32_t> (dciSlot.k) + l1l2CtrlLatency;

  (*toSend)[dciSlot.indexDci].push_back(static_cast<uint32_t> (dciSlot.k));
  (*generate)[indexGen].push_back (kWithCtrlLatency);
}

void
MmWaveEnbPhy::GenerateStructuresFromPattern (const std::vector<LteNrTddSlotType> &pattern,
                                             std::map<uint32_t, std::vector<uint32_t>> *toSendDl,
                                             std::map<uint32_t, std::vector<uint32_t>> *toSendUl,
                                             std::map<uint32_t, std::vector<uint32_t>> *generateDl,
                                             std::map<uint32_t, std::vector<uint32_t>> *generateUl,
                                             std::map<uint32_t, uint32_t> *dlHarqfbPosition,
                                             uint32_t n0, uint32_t n2, uint32_t n1, uint32_t l1l2CtrlLatency)
{
  const uint32_t n = static_cast<uint32_t> (pattern.size ());

  // Create a pattern that is all F.
  std::vector<LteNrTddSlotType> fddGenerationPattern;
  fddGenerationPattern.resize (pattern.size (), LteNrTddSlotType::F);

  /* if we have to generate structs for a TDD pattern, then use the input pattern.
   * Otherwise, pass to the gen functions a pattern which is all F (therefore, the
   * the function will think that they will be able to transmit or
   * receive things following n0, n1, n2, that is what happen in FDD, just in
   * another band..
   */

  const std::vector<LteNrTddSlotType> *generationPattern;

  if (IsTdd (pattern))
    {
      generationPattern = &pattern;
    }
  else
    {
      generationPattern = &fddGenerationPattern;
    }

  for (uint32_t i = 0; i < n; i++)
    {
      if ((*generationPattern)[i] == LteNrTddSlotType::UL)
        {
          GenerateDciMaps (*generationPattern, toSendUl, generateUl, i, n2, l1l2CtrlLatency);
        }
      else if ((*generationPattern)[i] == LteNrTddSlotType::DL || pattern[i] == LteNrTddSlotType::S)
        {
          GenerateDciMaps (*generationPattern, toSendDl, generateDl, i, n0, l1l2CtrlLatency);

          int32_t k1 = ReturnHarqSlot (*generationPattern, i, n1);
          (*dlHarqfbPosition).insert (std::make_pair (i, k1));
        }
      else if ((*generationPattern)[i] == LteNrTddSlotType::F)
        {
          GenerateDciMaps (*generationPattern, toSendDl, generateDl, i, n0, l1l2CtrlLatency);
          GenerateDciMaps (*generationPattern, toSendUl, generateUl, i, n2, l1l2CtrlLatency);

          int32_t k1 = ReturnHarqSlot (*generationPattern, i, n1);
          (*dlHarqfbPosition).insert (std::make_pair (i, k1));
        }
    }

  /*
   * Now, if the input pattern is for FDD, remove the elements in the
   * opposite generate* structures: in the end, we don't want to generate DL
   * for a FDD-UL band, right?
   *
   * But.. maintain the toSend structures, as they will be used to send
   * feedback or other messages, like DCI.
   */

  if (! IsTdd (pattern))
    {
      if (HasUlSlot (pattern))
        {
          generateDl->clear ();
        }
      else
        {
          generateUl->clear();
        }
    }

  for (auto & list : (*generateUl))
    {
      std::sort (list.second.begin (), list.second.end ());
    }

  for (auto & list : (*generateDl))
    {
      std::sort (list.second.begin (), list.second.end ());
    }
}

void
MmWaveEnbPhy::PushDlAllocation (const SfnSf &sfnSf) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phySapUser);

  auto dci = m_phySapUser->GetDlCtrlDci ();
  VarTtiAllocInfo dlCtrlVarTti (dci);

  SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);

  slotAllocInfo.m_numSymAlloc = dlCtrlVarTti.m_dci->m_numSym;
  slotAllocInfo.m_type = SlotAllocInfo::DL;
  slotAllocInfo.m_varTtiAllocInfo.emplace_back (dlCtrlVarTti);

  m_phySapProvider->SetSlotAllocInfo (slotAllocInfo);
}

void
MmWaveEnbPhy::PushUlAllocation (const SfnSf &sfnSf) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phySapUser);

  auto dci = m_phySapUser->GetUlCtrlDci ();
  VarTtiAllocInfo ulCtrlVarTti (dci);

  SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);

  slotAllocInfo.m_numSymAlloc = ulCtrlVarTti.m_dci->m_numSym;
  slotAllocInfo.m_type = SlotAllocInfo::UL;
  slotAllocInfo.m_varTtiAllocInfo.emplace_back (ulCtrlVarTti);

  m_phySapProvider->SetSlotAllocInfo (slotAllocInfo);
}

void
MmWaveEnbPhy::SetTddPattern (const std::vector<LteNrTddSlotType> &pattern)
{
  NS_LOG_FUNCTION (this);

  std::stringstream ss;

  for (const auto & v : pattern)
    {
      ss << v << "|";
    }
  NS_LOG_INFO ("Set pattern : " << ss.str ());

  m_tddPattern = pattern;

  m_generateDl.clear ();
  m_generateUl.clear ();
  m_toSendDl.clear ();
  m_toSendUl.clear ();
  m_dlHarqfbPosition.clear ();

  GenerateStructuresFromPattern (pattern, &m_toSendDl, &m_toSendUl,
                                 &m_generateDl, &m_generateUl,
                                 &m_dlHarqfbPosition, 0,
                                 GetN2Delay (), GetN1Delay (),
                                 GetL1L2CtrlLatency ());
}

void
MmWaveEnbPhy::ScheduleStartEventLoop (uint32_t nodeId, uint16_t frame, uint8_t subframe, uint16_t slot)
{
  NS_LOG_FUNCTION (this);
  Simulator::ScheduleWithContext (nodeId, MilliSeconds (0),
                                  &MmWaveEnbPhy::StartEventLoop, this, frame, subframe, slot);
}

void
MmWaveEnbPhy::StartEventLoop (uint16_t frame, uint8_t subframe, uint16_t slot)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("PHY starting. Configuration: "  << std::endl <<
                "\t TxPower: " << m_txPower << " dB" << std::endl <<
                "\t NoiseFigure: " << m_noiseFigure << std::endl <<
                "\t N0: " << m_n0Delay << std::endl <<
                "\t N1: " << m_n1Delay << std::endl <<
                "\t N2: " << m_n2Delay << std::endl <<
                "\t TbDecodeLatency: " << GetTbDecodeLatency ().GetMicroSeconds () << " us " << std::endl <<
                "\t Numerology: " << GetNumerology () << std::endl <<
                "\t SymbolsPerSlot: " << GetSymbolsPerSlot () << std::endl <<
                "\t Pattern: " << GetPattern () << std::endl <<
                "Attached to physical channel: " << std::endl <<
                "\t Channel bandwidth: " << GetChannelBandwidth () << " Hz" << std::endl <<
                "\t Num. RB: " << GetRbNum ());
  SfnSf startSlot (frame, subframe, slot, GetNumerology ());
  InitializeMessageList ();
  StartSlot (startSlot);
}

void
MmWaveEnbPhy::SetEnbCphySapUser (LteEnbCphySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_enbCphySapUser = s;
}

LteEnbCphySapProvider*
MmWaveEnbPhy::GetEnbCphySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_enbCphySapProvider;
}

uint32_t
MmWaveEnbPhy::GetN0Delay (void) const
{
  return m_n0Delay;
}

uint32_t
MmWaveEnbPhy::GetN1Delay (void) const
{
  return m_n1Delay;
}

uint32_t
MmWaveEnbPhy::GetN2Delay () const
{
  return m_n2Delay;
}

void
MmWaveEnbPhy::SetN0Delay (uint32_t delay)
{
  m_n0Delay = delay;
  SetTddPattern (m_tddPattern); // Update the generate/send structures
}

void
MmWaveEnbPhy::SetN1Delay (uint32_t delay)
{
  m_n1Delay = delay;
  SetTddPattern (m_tddPattern); // Update the generate/send structures
}

void
MmWaveEnbPhy::SetN2Delay (uint32_t delay)
{
  m_n2Delay = delay;
  SetTddPattern (m_tddPattern); // Update the generate/send structures
}

BeamId MmWaveEnbPhy::GetBeamId (uint16_t rnti) const
{
  for (uint8_t i = 0; i < m_deviceMap.size (); i++)
    {
      Ptr<MmWaveUeNetDevice> ueDev = DynamicCast < MmWaveUeNetDevice > (m_deviceMap.at (i));
      uint64_t ueRnti = (DynamicCast<MmWaveUePhy>(ueDev->GetPhy (0)))->GetRnti ();

      if (ueRnti == rnti)
        {
          return m_beamManager->GetBeamId (m_deviceMap.at(i));
        }
    }
  return BeamId (0,0);
}

void
MmWaveEnbPhy::SetCam (const Ptr<NrChAccessManager> &cam)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (cam != nullptr);
  m_cam = cam;
  m_cam->SetAccessGrantedCallback (std::bind (&MmWaveEnbPhy::ChannelAccessGranted, this,
                                              std::placeholders::_1));
  m_cam->SetAccessDeniedCallback (std::bind (&MmWaveEnbPhy::ChannelAccessLost, this));
}

Ptr<NrChAccessManager>
MmWaveEnbPhy::GetCam() const
{
  NS_LOG_FUNCTION (this);
  return m_cam;
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
MmWaveEnbPhy::SetSubChannels (const std::vector<int> &rbIndexVector)
{
  Ptr<SpectrumValue> txPsd = GetTxPowerSpectralDensity (rbIndexVector);
  NS_ASSERT (txPsd);
  m_spectrumPhy->SetTxPowerSpectralDensity (txPsd);
}

void
MmWaveEnbPhy::QueueMib ()
{
  NS_LOG_FUNCTION (this);
  LteRrcSap::MasterInformationBlock mib;
  mib.dlBandwidth = m_channelBandwidth;
  mib.systemFrameNumber = 1;
  Ptr<MmWaveMibMessage> mibMsg = Create<MmWaveMibMessage> ();
  mibMsg->SetSourceBwp (GetBwpId ());
  mibMsg->SetMib (mib);
  EnqueueCtrlMsgNow (mibMsg);
}

void MmWaveEnbPhy::QueueSib ()
{
  NS_LOG_FUNCTION (this);
  Ptr<MmWaveSib1Message> msg = Create<MmWaveSib1Message> ();
  msg->SetSib1 (m_sib1);
  msg->SetSourceBwp (GetBwpId ());
  EnqueueCtrlMsgNow (msg);
}

void
MmWaveEnbPhy::CallMacForSlotIndication (const SfnSf &currentSlot)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_generateDl.empty() || !m_generateUl.empty());

  m_phySapUser->SetCurrentSfn (currentSlot);

  uint64_t currentSlotN = currentSlot.Normalize () % m_tddPattern.size ();

  NS_LOG_INFO ("Start Slot " << currentSlot << ". In position " <<
               currentSlotN << " there is a slot of type " <<
               m_tddPattern[currentSlotN]);

  for (const auto & k2WithLatency : m_generateUl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;
      targetSlot.Add (k2WithLatency);

      uint64_t pos = targetSlot.Normalize ();

      NS_LOG_INFO (" in slot " << currentSlot << " generate UL for " <<
                     targetSlot << " which is of type " << m_tddPattern[pos]);

      m_phySapUser->SlotUlIndication (targetSlot, m_tddPattern[pos]);
    }

  for (const auto & k0WithLatency : m_generateDl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;
      targetSlot.Add (k0WithLatency);

      uint64_t pos = targetSlot.Normalize ();

      NS_LOG_INFO (" in slot " << currentSlot << " generate DL for " <<
                     targetSlot << " which is of type " << m_tddPattern[pos]);

      m_phySapUser->SlotDlIndication (targetSlot, m_tddPattern[pos]);
    }
}

void
MmWaveEnbPhy::StartSlot (const SfnSf &startSlot)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_channelStatus != TO_LOSE);

  m_currentSlot = startSlot;
  m_lastSlotStart = Simulator::Now ();

  Simulator::Schedule (GetSlotPeriod (), &MmWaveEnbPhy::EndSlot, this);

  // update the current slot allocation; if empty (e.g., at the beginning of simu)
  // then insert a dummy allocation, without anything.
  if (SlotAllocInfoExists (m_currentSlot))
    {
      m_currSlotAllocInfo = RetrieveSlotAllocInfo (m_currentSlot);
    }
  else
    {
      NS_LOG_WARN ("No allocation for the current slot. Using an empty one");
      m_currSlotAllocInfo = SlotAllocInfo (m_currentSlot);
    }

  if (m_isPrimary)
    {
      if (m_currentSlot.GetSlot () == 0)
        {
          if (m_currentSlot.GetSubframe () == 0)   //send MIB at the beginning of each frame
            {
              QueueMib ();
            }
          else if (m_currentSlot.GetSubframe () == 5)   // send SIB at beginning of second half-frame
            {
              QueueSib ();
            }
        }
    }

  if (m_channelStatus == GRANTED)
    {
      NS_LOG_INFO ("Channel granted");
      CallMacForSlotIndication (m_currentSlot);
      DoStartSlot ();
    }
  else
    {
      bool hasUlDci = false;
      SfnSf ulSfn = m_currentSlot;
      ulSfn.Add (GetN2Delay ());

      if (GetN2Delay () > 0)
        {
          if (SlotAllocInfoExists (ulSfn))
            {
              SlotAllocInfo & ulSlot = PeekSlotAllocInfo (ulSfn);
              hasUlDci = ulSlot.ContainsDataAllocation ();
            }
        }
      if (m_currSlotAllocInfo.ContainsDataAllocation () || ! IsCtrlMsgListEmpty () || hasUlDci)
        {
          // Request the channel access
          if (m_channelStatus == NONE)
            {
              NS_LOG_INFO ("Channel not granted, request the channel");
              m_channelStatus = REQUESTED; // This goes always before RequestAccess()
              m_cam->RequestAccess ();
              if (m_channelStatus == GRANTED)
                {
                  // Repetition but we can have a CAM that gives the channel
                  // instantaneously
                  NS_LOG_INFO ("Channel granted; asking MAC for SlotIndication for the future and then start the slot");
                  CallMacForSlotIndication (m_currentSlot);

                  DoStartSlot ();
                  return; // Exit without calling anything else
                }
            }
          // If the channel was not granted, queue back the allocation,
          // without calling the MAC for a new slot
          auto slotAllocCopy = m_currSlotAllocInfo;
          auto newSfnSf = slotAllocCopy.m_sfnSf;
          newSfnSf.Add (1);
          NS_LOG_INFO ("Queueing allocation in front for " << newSfnSf );
          if (m_currSlotAllocInfo.ContainsDataAllocation ())
            {
              NS_LOG_INFO ("Reason: Current slot allocation has data");
            }
          else
            {
              NS_LOG_INFO ("Reason: CTRL message list is not empty");
            }

          PushFrontSlotAllocInfo (newSfnSf, slotAllocCopy);
        }
      else
        {
          // It's an empty slot; ask the MAC for a new one (maybe a new data will arrive..)
          // and just let the current one go away
          NS_LOG_INFO ("Empty slot, but asking MAC for SlotIndication for the future, maybe there will be data");
          CallMacForSlotIndication (m_currentSlot);
        }
      // If we have the UL CTRL, then schedule it (we are listening, so
      // we don't need the channel.

      if (m_currSlotAllocInfo.m_varTtiAllocInfo.size() > 0)
        {
          for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
            {
              if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL && alloc.m_dci->m_format == DciInfoElementTdma::UL)
                {
                  Time start = GetSymbolPeriod () * alloc.m_dci->m_symStart;
                  NS_LOG_INFO ("Schedule UL CTRL at " << start);
                  Simulator::Schedule (start, &MmWaveEnbPhy::UlCtrl, this, alloc.m_dci);
                }
            }
        }
    }
}

void MmWaveEnbPhy::DoStartSlot ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_ctrlMsgs.size () == 0);

  uint64_t currentSlotN = m_currentSlot.Normalize ();

  NS_LOG_DEBUG ("Start Slot " << m_currentSlot << " of type " << m_tddPattern[currentSlotN]);
  NS_LOG_INFO ("Allocations of the current slot: " << std::endl << m_currSlotAllocInfo);

  if (m_currSlotAllocInfo.m_varTtiAllocInfo.size () == 0)
    {
      return;
    }

  NS_ASSERT (m_channelStatus == GRANTED);
  // The channel is granted, we have to check if we maintain it for the next
  // slot or we have to release it.

  // Assuming the scheduler assign contiguos symbol
  uint8_t lastDlSymbol = 0;
  for (auto & dci : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      if (dci.m_dci->m_type == DciInfoElementTdma::DATA && dci.m_dci->m_format == DciInfoElementTdma::DL)
        {
          lastDlSymbol = std::max (lastDlSymbol,
                                   static_cast<uint8_t> (dci.m_dci->m_symStart + dci.m_dci->m_numSym));
        }
    }

  Time lastDataTime = GetSymbolPeriod () * lastDlSymbol;

  if (GetSlotPeriod () - lastDataTime > MicroSeconds (25))
    {
      NS_LOG_LOGIC ("Last symbol of data: " << +lastDlSymbol << ", to the end of slot we still have " <<
                   (GetSlotPeriod () - lastDataTime).GetMicroSeconds() <<
                   " us, so we're going to lose the channel");
      m_channelStatus = TO_LOSE;
    }
  else
    {
      NS_LOG_LOGIC ("Last symbol of data: " << +lastDlSymbol << ", to the end of slot we still have " <<
                   (GetSlotPeriod () - lastDataTime).GetMicroSeconds() <<
                   " us, so we're NOT going to lose the channel");
    }

  VarTtiAllocInfo allocation = m_currSlotAllocInfo.m_varTtiAllocInfo.front ();
  m_currSlotAllocInfo.m_varTtiAllocInfo.pop_front ();

  auto nextVarTtiStart = GetSymbolPeriod () * allocation.m_dci->m_symStart;

  auto ctrlMsgs = PopCurrentSlotCtrlMsgs ();
  ctrlMsgs.merge (RetrieveMsgsFromDCIs (m_currentSlot));

  if (m_netDevice != nullptr)
    {
      DynamicCast<MmWaveEnbNetDevice> (m_netDevice)->RouteOutgoingCtrlMsgs (ctrlMsgs, GetBwpId ());
    }
  else
    {
      // No netDevice (that could happen in tests) so just redirect them to us
      for (const auto & msg : ctrlMsgs)
        {
          EncodeCtrlMsg (msg);
        }
    }

  Simulator::Schedule (nextVarTtiStart, &MmWaveEnbPhy::StartVarTti, this, allocation.m_dci);
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
      NS_ASSERT (existingRBGBitmask.size () == dci->m_rbgBitmask.size ());
      for (uint32_t i = 0; i < existingRBGBitmask.size (); ++i)
        {
          existingRBGBitmask.at (i) = existingRBGBitmask.at (i) | dci->m_rbgBitmask.at (i);
        }
    }
}

std::list <Ptr<MmWaveControlMessage>>
MmWaveEnbPhy::RetrieveDciFromAllocation (const SlotAllocInfo &alloc,
                                         const DciInfoElementTdma::DciFormat &format,
                                         uint32_t kDelay, uint32_t k1Delay)
{
  NS_LOG_FUNCTION(this);
  std::list <Ptr<MmWaveControlMessage>> ctrlMsgs;

  for (const auto & dlAlloc : alloc.m_varTtiAllocInfo)
    {
      if (dlAlloc.m_dci->m_type != DciInfoElementTdma::CTRL && dlAlloc.m_dci->m_format == format)
        {
          auto & dciElem = dlAlloc.m_dci;
          NS_ASSERT (dciElem->m_format == format);
          NS_ASSERT (dciElem->m_tbSize > 0);
          NS_ASSERT_MSG (dciElem->m_symStart + dciElem->m_numSym <= GetSymbolsPerSlot (),
                         "symStart: " << static_cast<uint32_t> (dciElem->m_symStart) <<
                         " numSym: " << static_cast<uint32_t> (dciElem->m_numSym) <<
                         " symPerSlot: " << static_cast<uint32_t> (GetSymbolsPerSlot ()));

          NS_LOG_INFO ("Send DCI to " << dciElem->m_rnti << " from sym " <<
                         +dciElem->m_symStart << " to " << +dciElem->m_symStart + dciElem->m_numSym);

          Ptr<MmWaveTdmaDciMessage> dciMsg = Create<MmWaveTdmaDciMessage> (dciElem);

          dciMsg->SetSourceBwp (GetBwpId ());
          dciMsg->SetKDelay (kDelay);
          dciMsg->SetK1Delay (k1Delay);  //Set for both DL/UL however used only in DL (in UL UE ignors it)

          ctrlMsgs.push_back (dciMsg);
        }
    }

  return ctrlMsgs;
}

std::list <Ptr<MmWaveControlMessage> >
MmWaveEnbPhy::RetrieveMsgsFromDCIs (const SfnSf &currentSlot)
{
  std::list <Ptr<MmWaveControlMessage> > ctrlMsgs;
  uint64_t currentSlotN = currentSlot.Normalize () % m_tddPattern.size ();

  uint32_t k1delay = m_dlHarqfbPosition[currentSlotN];

  // TODO: copy paste :(
  for (const auto & k0delay : m_toSendDl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;

      targetSlot.Add (k0delay);

      if (targetSlot == currentSlot)
        {
          NS_LOG_INFO (" in slot " << currentSlot << " send DL DCI for the same slot");

          ctrlMsgs.merge (RetrieveDciFromAllocation (m_currSlotAllocInfo,
                                                     DciInfoElementTdma::DL, k0delay, k1delay));
        }
      else if (SlotAllocInfoExists (targetSlot))
        {
          NS_LOG_INFO (" in slot " << currentSlot << " send DL DCI for " <<
                         targetSlot);

          ctrlMsgs.merge (RetrieveDciFromAllocation (PeekSlotAllocInfo(targetSlot),
                                                     DciInfoElementTdma::DL, k0delay, k1delay));
        }
      else
        {
          NS_LOG_INFO ("No allocation found for slot " << targetSlot);
        }
    }

  for (const auto & k2delay : m_toSendUl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;

      targetSlot.Add (k2delay);

      if (targetSlot == currentSlot)
        {
          NS_LOG_INFO (" in slot " << currentSlot << " send UL DCI for the same slot");

          ctrlMsgs.merge (RetrieveDciFromAllocation (m_currSlotAllocInfo,
                                                     DciInfoElementTdma::UL, k2delay, k1delay));
        }
      else if (SlotAllocInfoExists (targetSlot))
        {
          NS_LOG_INFO (" in slot " << currentSlot << " send UL DCI for " <<
                         targetSlot);

          ctrlMsgs.merge (RetrieveDciFromAllocation (PeekSlotAllocInfo(targetSlot),
                                                     DciInfoElementTdma::UL, k2delay, k1delay));
        }
      else
        {
          NS_LOG_INFO ("No allocation found for slot " << targetSlot);
        }
    }

  return ctrlMsgs;
}

Time
MmWaveEnbPhy::DlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Starting DL CTRL TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  // Start with a clean RBG allocation bitmask
  m_rbgAllocationPerSym.clear ();

  // Create RBG map to know where to put power in DL
  for (const auto & dlAlloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      if (dlAlloc.m_dci->m_type != DciInfoElementTdma::CTRL
          && dlAlloc.m_dci->m_format == DciInfoElementTdma::DL)
        {
          StoreRBGAllocation (dlAlloc.m_dci);
        }
    }

  // TX control period
  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  // The function that is filling m_ctrlMsgs is MmWavePhy::encodeCtrlMsgs
  if (m_ctrlMsgs.size () > 0)
    {
      NS_LOG_INFO ("ENB TXing DL CTRL with " << m_ctrlMsgs.size () << " msgs, frame " << m_currentSlot <<
                    " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                    " start " << Simulator::Now () <<
                    " end " << Simulator::Now () + varTtiPeriod - NanoSeconds (1.0));

      for (auto ctrlIt = m_ctrlMsgs.begin (); ctrlIt != m_ctrlMsgs.end (); ++ctrlIt)
        {
          Ptr<MmWaveControlMessage> msg = (*ctrlIt);
          m_phyTxedCtrlMsgsTrace (m_currentSlot, dci->m_rnti, GetBwpId (), msg);
        }

      SendCtrlChannels (varTtiPeriod - NanoSeconds (1.0)); // -1 ns ensures control ends before data period
    }
  else
    {
      NS_LOG_INFO ("No messages to send, skipping");
    }

  return varTtiPeriod;
}

Time
MmWaveEnbPhy::UlCtrl(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Starting UL CTRL TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  NS_LOG_INFO ("ENB RXng UL CTRL frame " << m_currentSlot <<
                " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                " start " << Simulator::Now () <<
                " end " << Simulator::Now () + varTtiPeriod);
  return varTtiPeriod;
}

Time
MmWaveEnbPhy::DlData (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Starting DL DATA TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  Ptr<PacketBurst> pktBurst = GetPacketBurst (m_currentSlot, dci->m_symStart);
  if (pktBurst && pktBurst->GetNPackets () > 0)
    {
      std::list< Ptr<Packet> > pkts = pktBurst->GetPackets ();
      MmWaveMacPduTag macTag;
      pkts.front ()->PeekPacketTag (macTag);
    }
  else
    {
      // put an error, as something is wrong. The UE should not be scheduled
      // if there is no data for him...
      NS_FATAL_ERROR ("The UE " << dci->m_rnti << " has been scheduled without data");
      MmWaveMacPduTag tag (m_currentSlot, dci->m_symStart, dci->m_numSym);
      Ptr<Packet> emptyPdu = Create <Packet> ();
      MmWaveMacPduHeader header;
      MacSubheader subheader (3, 0);    // lcid = 3, size = 0
      header.AddSubheader (subheader);
      emptyPdu->AddHeader (header);
      emptyPdu->AddPacketTag (tag);
      LteRadioBearerTag bearerTag (dci->m_rnti, 3, 0);
      emptyPdu->AddPacketTag (bearerTag);
      pktBurst = CreateObject<PacketBurst> ();
      pktBurst->AddPacket (emptyPdu);
    }

  NS_LOG_INFO ("ENB TXing DL DATA frame " << m_currentSlot <<
                " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                " start " << Simulator::Now () + NanoSeconds (1) <<
                " end " << Simulator::Now () + varTtiPeriod - NanoSeconds (2.0));

  Simulator::Schedule (NanoSeconds (1.0), &MmWaveEnbPhy::SendDataChannels, this,
                       pktBurst, varTtiPeriod - NanoSeconds (2.0), dci);

  return varTtiPeriod;
}

Time
MmWaveEnbPhy::UlData(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_INFO (this);

  NS_LOG_DEBUG ("Starting UL DATA TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  // Assert: we expect TDMA in UL
  NS_ASSERT (static_cast<uint32_t> (std::count (dci->m_rbgBitmask.begin (),
                                                dci->m_rbgBitmask.end (), 1U)) == dci->m_rbgBitmask.size ());

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  m_spectrumPhy->AddExpectedTb (dci->m_rnti, dci->m_ndi,
                                        dci->m_tbSize, dci->m_mcs,
                                        FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask),
                                        dci->m_harqProcess, dci->m_rv, false,
                                        dci->m_symStart, dci->m_numSym);

  bool found = false;
  for (uint8_t i = 0; i < m_deviceMap.size (); i++)
    {
      Ptr<MmWaveUeNetDevice> ueDev = DynamicCast < MmWaveUeNetDevice > (m_deviceMap.at (i));
      uint64_t ueRnti = (DynamicCast<MmWaveUePhy>(ueDev->GetPhy (0)))->GetRnti ();
      if (dci->m_rnti == ueRnti)
        {
          NS_ABORT_MSG_IF(m_beamManager == nullptr, "Beam manager not initialized");
          m_beamManager->ChangeBeamformingVector (m_deviceMap.at (i)); //assume the control signal is omni
          found = true;
          break;
        }
    }
  NS_ASSERT (found);

  NS_LOG_INFO ("ENB RXing UL DATA frame " << m_currentSlot <<
                " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                " start " << Simulator::Now () <<
                " end " << Simulator::Now () + varTtiPeriod);
  return varTtiPeriod;
}

void
MmWaveEnbPhy::StartVarTti (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  NS_ABORT_MSG_IF(m_beamManager == nullptr, "Beam manager not initialized");
  m_beamManager->ChangeToOmniTx(); //assume the control signal is omni
  m_currSymStart = dci->m_symStart;

  Time varTtiPeriod;

  NS_ASSERT (dci->m_type != DciInfoElementTdma::CTRL_DATA);

  if (dci->m_type == DciInfoElementTdma::CTRL)
    {
      if (dci->m_format == DciInfoElementTdma::DL)
        {
          varTtiPeriod = DlCtrl (dci);
        }
      else if (dci->m_format == DciInfoElementTdma::UL)
        {
          varTtiPeriod = UlCtrl (dci);
        }
    }
  else  if (dci->m_type == DciInfoElementTdma::DATA)
    {
      if (dci->m_format == DciInfoElementTdma::DL)
        {
          varTtiPeriod = DlData (dci);
        }
      else if (dci->m_format == DciInfoElementTdma::UL)
        {
          varTtiPeriod = UlData (dci);
        }
    }

  Simulator::Schedule (varTtiPeriod, &MmWaveEnbPhy::EndVarTti, this, dci);
}

void
MmWaveEnbPhy::EndVarTti (const std::shared_ptr<DciInfoElementTdma> &lastDci)
{
  NS_LOG_FUNCTION (this << Simulator::Now ().GetSeconds ());

  NS_LOG_DEBUG ("DCI started at symbol " << static_cast<uint32_t> (lastDci->m_symStart) <<
                " which lasted for " << static_cast<uint32_t> (lastDci->m_numSym) <<
                " symbols finished");

  NS_ABORT_MSG_IF(m_beamManager == nullptr, "Beam manager not initialized");
  m_beamManager->ChangeToOmniTx(); //assume the control signal is omni

  if (m_currSlotAllocInfo.m_varTtiAllocInfo.size () > 0)
    {
      VarTtiAllocInfo allocation = m_currSlotAllocInfo.m_varTtiAllocInfo.front ();
      m_currSlotAllocInfo.m_varTtiAllocInfo.pop_front ();

      if (lastDci->m_symStart == allocation.m_dci->m_symStart)
        {
          NS_LOG_INFO ("DCI for UE " << allocation.m_dci->m_rnti << " starts from symbol " <<
                       static_cast<uint32_t> (allocation.m_dci->m_symStart) << " ignoring at PHY");
          EndVarTti (allocation.m_dci);
        }
      else
        {
          auto nextVarTtiStart = GetSymbolPeriod () * allocation.m_dci->m_symStart;

          NS_LOG_INFO ("DCI for UE " << allocation.m_dci->m_rnti << " starts from symbol " <<
                       static_cast<uint32_t> (allocation.m_dci->m_symStart) << " scheduling at PHY, at " <<
                       nextVarTtiStart + m_lastSlotStart << " where last slot start = " <<
                       m_lastSlotStart << " nextVarTti " << nextVarTtiStart);

          Simulator::Schedule (nextVarTtiStart + m_lastSlotStart - Simulator::Now (),
                               &MmWaveEnbPhy::StartVarTti, this, allocation.m_dci);
        }
      // Do not put any code here (tail recursion)
    }
  // Do not put any code here (tail recursion)
}

void
MmWaveEnbPhy::EndSlot (void)
{
  NS_LOG_FUNCTION (this);

  Time slotStart = m_lastSlotStart + GetSlotPeriod () - Simulator::Now ();

  if (m_channelStatus == TO_LOSE)
    {
      NS_LOG_INFO ("Release the channel because we did not have any data to maintain the grant");
      m_channelStatus = NONE;
      m_channelLostTimer.Cancel ();
    }

  NS_LOG_DEBUG ("Slot started at " << m_lastSlotStart << " ended");
  m_currentSlot.Add (1);
  Simulator::Schedule (slotStart, &MmWaveEnbPhy::StartSlot, this, m_currentSlot);
}

void
MmWaveEnbPhy::SendDataChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod,
                                const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  // update beamforming vectors (currently supports 1 user only)
  bool found = false;
  for (uint8_t i = 0; i < m_deviceMap.size (); i++)
    {
      Ptr<MmWaveUeNetDevice> ueDev = DynamicCast<MmWaveUeNetDevice> (m_deviceMap.at (i));
      uint64_t ueRnti = (DynamicCast<MmWaveUePhy>(ueDev->GetPhy (0)))->GetRnti ();
      //NS_LOG_INFO ("Scheduled rnti:"<<rnti <<" ue rnti:"<< ueRnti);
      if (dci->m_rnti == ueRnti)
        {
          NS_ABORT_MSG_IF(m_beamManager == nullptr, "Beam manager not initialized");
          m_beamManager->ChangeBeamformingVector(m_deviceMap.at (i));
          found = true;
          break;
        }
    }
  NS_ABORT_IF (!found);


  // in the map we stored the RBG allocated by the MAC for this symbol.
  // If the transmission last n symbol (n > 1 && n < 12) the SetSubChannels
  // doesn't need to be called again. In fact, SendDataChannels will be
  // invoked only when the symStart changes.
  NS_ASSERT (m_rbgAllocationPerSym.find(dci->m_symStart) != m_rbgAllocationPerSym.end ());
  SetSubChannels (FromRBGBitmaskToRBAssignment (m_rbgAllocationPerSym.at (dci->m_symStart)));

  std::list<Ptr<MmWaveControlMessage> > ctrlMsgs;
  m_spectrumPhy->StartTxDataFrames (pb, ctrlMsgs, varTtiPeriod, dci->m_symStart);
}

void
MmWaveEnbPhy::SendCtrlChannels (const Time &varTtiPeriod)
{
  NS_LOG_FUNCTION (this << "Send Ctrl");

  std::vector <int> fullBwRb (GetRbNum ());
  // The first time set the right values for the phy
  for (uint32_t i = 0; i < fullBwRb.size (); ++i)
    {
      fullBwRb[i] = static_cast<int> (i);
    }

  SetSubChannels (fullBwRb);

  m_spectrumPhy->StartTxDlControlFrames (m_ctrlMsgs, varTtiPeriod);
  m_ctrlMsgs.clear ();
}

bool
MmWaveEnbPhy::RegisterUe (uint64_t imsi, const Ptr<MmWaveUeNetDevice> &ueDevice)
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
MmWaveEnbPhy::PhyDataPacketReceived (const Ptr<Packet> &p)
{
  Simulator::ScheduleWithContext (m_netDevice->GetNode ()->GetId (),
                                  GetTbDecodeLatency (),
                                  &MmWaveEnbPhySapUser::ReceivePhyPdu,
                                  m_phySapUser,
                                  p);
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
      //       NS_LOG_INFO ("ULCQI RB " << i << " value " << sinrdb);
      // convert from double to fixed point notaltion Sxxxxxxxxxxx.xxx
      //   int16_t sinrFp = LteFfConverter::double2fpS11dot3 (sinrdb);
      ulcqi.m_ulCqi.m_sinr.push_back (*it);
      i++;
    }

  // here we use the start symbol index of the var tti in place of the var tti index because the absolute UL var tti index is
  // not known to the scheduler when m_allocationMap gets populated
  ulcqi.m_sfnSf = m_currentSlot;
  ulcqi.m_symStart = m_currSymStart;
  SpectrumValue newSinr = sinr;
  m_ulSinrTrace (0, newSinr, newSinr);
  m_phySapUser->UlCqiReport (ulcqi);
}


void
MmWaveEnbPhy::PhyCtrlMessagesReceived (const Ptr<MmWaveControlMessage> &msg)
{
  NS_LOG_FUNCTION (this);

  // If I have received UL CTRL messages, and I was about to lose the channel,
  // I can reuse through the cot gained by the UE.
  // We maintain the total timer in m_channelLostTimer: when our first calculated
  // MCOT expires, we release the channel anyway.
  //if (msgList.size () > 0 && m_channelStatus == TO_LOSE)
   // {
   //   NS_LOG_INFO ("Received " << msgList.size() << " CTRL msgs, channel gained again");
  //    m_channelStatus = GRANTED;
   // }

  if (msg->GetMessageType () == MmWaveControlMessage::DL_CQI)
    {
      NS_LOG_INFO ("received CQI");

      Ptr<MmWaveDlCqiMessage> dlcqi = DynamicCast<MmWaveDlCqiMessage> (msg);
      DlCqiInfo dlcqiLE = dlcqi->GetDlCqi ();
      m_phyRxedCtrlMsgsTrace (m_currentSlot, dlcqiLE.m_rnti, GetBwpId (), msg);

      NS_LOG_INFO ("Received DL_CQI for RNTI: " << dlcqiLE.m_rnti << " in slot " <<
                   m_currentSlot << ", scheduling MAC ReceiveControlMessage after the decode latency");
      Simulator::Schedule (GetTbDecodeLatency(), &MmWaveEnbPhySapUser::ReceiveControlMessage, m_phySapUser, msg);
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::BSR)
    {
      NS_LOG_INFO ("received BSR");

      Ptr<MmWaveBsrMessage> bsrmsg = DynamicCast<MmWaveBsrMessage> (msg);
      MacCeElement macCeEl = bsrmsg->GetBsr();
      m_phyRxedCtrlMsgsTrace (m_currentSlot, macCeEl.m_rnti, GetBwpId (), msg);

      NS_LOG_INFO ("Received BSR for RNTI: " << macCeEl.m_rnti << " in slot " <<
                   m_currentSlot << ", scheduling MAC ReceiveControlMessage after the decode latency");
      Simulator::Schedule (GetTbDecodeLatency(), &MmWaveEnbPhySapUser::ReceiveControlMessage, m_phySapUser, msg);
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::RACH_PREAMBLE)
    {
      NS_LOG_INFO ("received RACH_PREAMBLE");

      Ptr<MmWaveRachPreambleMessage> rachPreamble = DynamicCast<MmWaveRachPreambleMessage> (msg);
      //          m_phySapUser->ReceiveRachPreamble (rachPreamble->GetRapId ());
      m_phyRxedCtrlMsgsTrace (m_currentSlot, 0, GetBwpId (), msg);
      NS_LOG_INFO ("Received RACH Preamble in slot " << m_currentSlot <<
                   ", scheduling MAC ReceiveControlMessage after the decode latency");
      Simulator::Schedule (GetTbDecodeLatency(), &MmWaveEnbPhySapUser::ReceiveRachPreamble, m_phySapUser,
                           rachPreamble->GetRapId ());

    }
  else if (msg->GetMessageType () == MmWaveControlMessage::DL_HARQ)
    {

      Ptr<MmWaveDlHarqFeedbackMessage> dlharqMsg = DynamicCast<MmWaveDlHarqFeedbackMessage> (msg);
      DlHarqInfo dlharq = dlharqMsg->GetDlHarqFeedback ();

      NS_LOG_INFO (" received DL_HARQ: " << dlharq);
      // check whether the UE is connected

      if (m_ueAttachedRnti.find (dlharq.m_rnti) != m_ueAttachedRnti.end ())
        {
          m_phyRxedCtrlMsgsTrace (m_currentSlot, dlharq.m_rnti, GetBwpId (), msg);

          NS_LOG_INFO ("Received DL_HARQ for RNTI: " << dlharq.m_rnti << " in slot " <<
                       m_currentSlot << ", scheduling MAC ReceiveControlMessage after the decode latency");
          Simulator::Schedule (GetTbDecodeLatency(), &MmWaveEnbPhySapUser::ReceiveControlMessage, m_phySapUser, msg);
        }
    }
  else
    {
      //m_phySapUser->ReceiveControlMessage (msg);

      m_phyRxedCtrlMsgsTrace (m_currentSlot, 0, GetBwpId (), msg);

      Simulator::Schedule (GetTbDecodeLatency(), &MmWaveEnbPhySapUser::ReceiveControlMessage, m_phySapUser, msg);
    }

}


////////////////////////////////////////////////////////////
/////////                     sap                 /////////
///////////////////////////////////////////////////////////

void
MmWaveEnbPhy::DoSetBandwidth (uint16_t ulBandwidth, uint16_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << +ulBandwidth << +dlBandwidth);
  NS_ASSERT (ulBandwidth == dlBandwidth);
  m_channelBandwidth = dlBandwidth;
  UpdateRbNum ();
}

void
MmWaveEnbPhy::DoSetEarfcn (uint16_t ulEarfcn, uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << ulEarfcn << dlEarfcn);
}


void
MmWaveEnbPhy::DoAddUe (uint16_t rnti)
{
  NS_UNUSED (rnti);
  NS_LOG_FUNCTION (this << rnti);
  std::set <uint16_t>::iterator it;
  it = m_ueAttachedRnti.find (rnti);
  if (it == m_ueAttachedRnti.end ())
    {
      m_ueAttachedRnti.insert (rnti);
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
  NS_LOG_FUNCTION (this << rnti << pa);
}

void
MmWaveEnbPhy::DoSetTransmissionMode (uint16_t  rnti, uint8_t txMode)
{
  NS_LOG_FUNCTION (this << rnti << +txMode);
  // UL supports only SISO MODE
}

void
MmWaveEnbPhy::DoSetSrsConfigurationIndex (uint16_t  rnti, uint16_t srcCi)
{
  NS_LOG_FUNCTION (this << rnti << srcCi);
}

void
MmWaveEnbPhy::DoSetMasterInformationBlock (LteRrcSap::MasterInformationBlock mib)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (mib);
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
  return static_cast<int8_t> (m_txPower);
}

void
MmWaveEnbPhy::SetPhySapUser (MmWaveEnbPhySapUser* ptr)
{
  m_phySapUser = ptr;
}

void
MmWaveEnbPhy::ReportUlHarqFeedback (const UlHarqInfo &mes)
{
  NS_LOG_FUNCTION (this);
  // forward to scheduler
  if (m_ueAttachedRnti.find (mes.m_rnti) != m_ueAttachedRnti.end ())
    {
      NS_LOG_INFO ("Received UL HARQ feedback " << mes.IsReceivedOk() <<
                   " and forwarding to the scheduler");
      m_phySapUser->UlHarqFeedback (mes);
    }
}

void
MmWaveEnbPhy::SetPattern (const std::string &pattern)
{
  NS_LOG_FUNCTION (this);

  static std::unordered_map<std::string, LteNrTddSlotType> lookupTable =
  {
    { "DL", LteNrTddSlotType::DL },
    { "UL", LteNrTddSlotType::UL },
    { "S",  LteNrTddSlotType::S },
    { "F",  LteNrTddSlotType::F },
  };

  std::vector<LteNrTddSlotType> vector;
  std::stringstream ss (pattern);
  std::string token;
  std::vector<std::string> extracted;

   while (std::getline(ss, token, '|'))
     {
       extracted.push_back(token);
     }

   for (const auto & v : extracted)
     {
       vector.push_back (lookupTable[v]);
     }

   SetTddPattern (vector);
}

std::string
MmWaveEnbPhy::GetPattern () const
{
  static std::unordered_map<LteNrTddSlotType, std::string> lookupTable =
  {
    { LteNrTddSlotType::DL, "DL"},
    { LteNrTddSlotType::UL, "UL"},
    { LteNrTddSlotType::S,  "S"},
    { LteNrTddSlotType::F,  "F"}
  };

  std::stringstream ss;

  for (const auto & v : m_tddPattern)
    {
      ss << lookupTable[v] << "|";
    }

  return ss.str ();
}

void
MmWaveEnbPhy::SetPrimary ()
{
  NS_LOG_FUNCTION (this);
  m_isPrimary = true;
}

void
MmWaveEnbPhy::ChannelAccessGranted (const Time &time)
{
  NS_LOG_FUNCTION (this);

  if (time < GetSlotPeriod ())
    {
      NS_LOG_INFO ("Channel granted for less than the slot time. Ignoring the grant.");
      m_channelStatus = NONE;
      return;
    }

  m_channelStatus = GRANTED;

  Time toNextSlot = m_lastSlotStart + GetSlotPeriod () - Simulator::Now ();
  Time grant = time - toNextSlot;
  int64_t slotGranted = grant.GetNanoSeconds () / GetSlotPeriod().GetNanoSeconds ();

  NS_LOG_INFO ("Channel access granted for " << time.GetMilliSeconds () <<
               " ms, which corresponds to " << slotGranted << " slot in which each slot is " <<
               GetSlotPeriod() << " ms. We lost " <<
               toNextSlot.GetMilliSeconds() << " ms. ");
  NS_ASSERT(! m_channelLostTimer.IsRunning ());

  slotGranted = std::max (1L, slotGranted);
  m_channelLostTimer = Simulator::Schedule (GetSlotPeriod () * slotGranted - NanoSeconds (1),
                                            &MmWaveEnbPhy::ChannelAccessLost, this);
}

void
MmWaveEnbPhy::ChannelAccessLost ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Channel access lost");
  m_channelStatus = NONE;
}

}
