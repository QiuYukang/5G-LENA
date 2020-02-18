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
      if (m_phyMacConfig)                                                \
        {                                                                \
          std::clog << " [ CellId " << m_cellId << ", ccId "             \
                    << +m_phyMacConfig->GetCcId () << "] ";              \
        }                                                                \
    }                                                                    \
  while (false);

#include <ns3/log.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/node.h>
#include <algorithm>
#include <functional>

#include "mmwave-enb-phy.h"
#include "mmwave-ue-phy.h"
#include "mmwave-net-device.h"
#include "mmwave-ue-net-device.h"
#include "mmwave-radio-bearer-tag.h"
#include "nr-ch-access-manager.h"

#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/pointer.h>
#include <ns3/double.h>
#include "beam-manager.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveEnbPhy");

NS_OBJECT_ENSURE_REGISTERED (MmWaveEnbPhy);

MmWaveEnbPhy::MmWaveEnbPhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

MmWaveEnbPhy::MmWaveEnbPhy (Ptr<MmWaveSpectrumPhy> channelPhy,
                            const Ptr<Node> &n)
  : MmWavePhy (channelPhy)
{
  m_enbCphySapProvider = new MemberLteEnbCphySapProvider<MmWaveEnbPhy> (this);

  Simulator::ScheduleWithContext (n->GetId (), MilliSeconds (0),
                                  &MmWaveEnbPhy::StartSlot, this, 0, 0, 0);
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
                   MakeDoubleAccessor (&MmWaveEnbPhy::m_noiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SpectrumPhy",
                   "The downlink MmWaveSpectrumPhy associated to this MmWavePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&MmWaveEnbPhy::GetSpectrumPhy),
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
    .AddAttribute ("MmWavePhyMacCommon",
                   "The associated MmWavePhyMacCommon",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveEnbPhy::m_phyMacConfig),
                   MakePointerChecker<MmWaveEnbPhy> ())
	.AddAttribute ("IsotropicAntennaElements",
	               "Defines type of antenna elements to be used: "
	               "a) when true, isotropic, and "
				   "b) when false, 3gpp."
				   "Another important parameter to specify is the number of antenna elements by "
				   "dimension.",
				   BooleanValue(false),
				   MakeBooleanAccessor(&MmWaveEnbPhy::m_areIsotropicElements),
				   MakeBooleanChecker())
    .AddAttribute ("AntennaNumDim1",
                   "Size of the first dimension of the antenna sector/panel expressed in number of antenna elements",
                   UintegerValue (4),
				   MakeUintegerAccessor (&MmWaveEnbPhy::m_antennaNumDim1),
				   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("AntennaNumDim2",
                   "Size of the second dimension of the antenna sector/panel expressed in number of antenna elements",
                   UintegerValue (8),
				   MakeUintegerAccessor (&MmWaveEnbPhy::m_antennaNumDim2),
				   MakeUintegerChecker<uint32_t> ())
	.AddAttribute ("IdealBeamformingEnabled",
	               "If true, ideal beamforming will be performed between gNB and its ideally attached UE devices."
				   "If false, no ideal beamforming will be performed. By default is ideal, until "
				   "real beamforming methods are implemented.",
				   BooleanValue (true),
				   MakeBooleanAccessor (&MmWaveEnbPhy::m_idealBeamformingEnabled),
				   MakeBooleanChecker ())
	.AddAttribute ("IdealBeamformingAlgorithmType",
				   "Type of the ideal beamforming algorithm in the case that it is enabled, by default is \"cell scan\" method.",
				   TypeIdValue (CellScanBeamforming::GetTypeId ()),
				   MakeTypeIdAccessor (&MmWaveEnbPhy::m_idealBeamformingAlgorithmType),
				   MakeTypeIdChecker ())
    ;
  return tid;

}

void
MmWaveEnbPhy::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  m_spectrumPhy->SetNoisePowerSpectralDensity (GetNoisePowerSpectralDensity());

  MmWavePhy::InstallBeamManager();

  if (m_idealBeamformingEnabled)
    {
      ObjectFactory objectFactory;
      objectFactory.SetTypeId (m_idealBeamformingAlgorithmType);
      Ptr<IdealBeamformingAlgorithm> idealAlgorithm = objectFactory.Create<IdealBeamformingAlgorithm>();
      m_beamManager->SetIdeamBeamformingAlgorithm (idealAlgorithm);
    }

  MmWavePhy::DoInitialize ();
  NS_LOG_INFO ("eNb antenna array initialised:" << static_cast<uint32_t> (m_beamManager->GetAntennaArray()->GetNumberOfElements()));

}

void
MmWaveEnbPhy::DoDispose (void)
{
  delete m_enbCphySapProvider;
  MmWavePhy::DoDispose ();
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

  for (uint32_t i = 0; i < n; i++)
    {
      if (pattern[i] == LteNrTddSlotType::UL)
        {
          GenerateDciMaps (pattern, toSendUl, generateUl, i, n2, l1l2CtrlLatency);
        }
      else if (pattern[i] == LteNrTddSlotType::DL || pattern[i] == LteNrTddSlotType::S)
        {
          GenerateDciMaps (pattern, toSendDl, generateDl, i, n0, l1l2CtrlLatency);

          int32_t k1 = ReturnHarqSlot (pattern, i, n1);
          (*dlHarqfbPosition).insert (std::make_pair (i, k1));
        }
      else if (pattern[i] == LteNrTddSlotType::F)
        {
          GenerateDciMaps (pattern, toSendDl, generateDl, i, n0, l1l2CtrlLatency);
          GenerateDciMaps (pattern, toSendUl, generateUl, i, n2, l1l2CtrlLatency);

          int32_t k1 = ReturnHarqSlot (pattern, i, n1);
          (*dlHarqfbPosition).insert (std::make_pair (i, k1));
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
  NS_LOG_INFO ("Pushing DL CTRL symbol allocation for " << sfnSf);

  std::vector<uint8_t> rbgBitmask (m_phyMacConfig->GetBandwidthInRbg (), 1);
  SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);

  slotAllocInfo.m_numSymAlloc = 1;
  slotAllocInfo.m_type = SlotAllocInfo::BOTH;

  auto dciDl = std::make_shared<DciInfoElementTdma> (0, 1, DciInfoElementTdma::DL, DciInfoElementTdma::CTRL, rbgBitmask);
  VarTtiAllocInfo dlCtrlVarTti (dciDl);

  slotAllocInfo.m_varTtiAllocInfo.emplace_back (dlCtrlVarTti);
  m_phySapProvider->SetSlotAllocInfo (slotAllocInfo);
}

void
MmWaveEnbPhy::PushUlAllocation (const SfnSf &sfnSf) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Pushing UL CTRL symbol allocation for " << sfnSf);

  std::vector<uint8_t> rbgBitmask (m_phyMacConfig->GetBandwidthInRbg (), 1);
  SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);

  slotAllocInfo.m_numSymAlloc = 1;
  slotAllocInfo.m_type = SlotAllocInfo::BOTH;

  auto dciUl = std::make_shared<DciInfoElementTdma> (m_phyMacConfig->GetSymbolsPerSlot () - 1, 1, DciInfoElementTdma::UL, DciInfoElementTdma::CTRL, rbgBitmask);
  VarTtiAllocInfo ulCtrlVarTti (dciUl);

  slotAllocInfo.m_varTtiAllocInfo.emplace_back (ulCtrlVarTti);
  m_phySapProvider->SetSlotAllocInfo (slotAllocInfo);
}

void
MmWaveEnbPhy::SetTddPattern (const std::vector<LteNrTddSlotType> &pattern)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phyMacConfig != nullptr);
  m_tddPattern = pattern;

  m_generateDl.clear ();
  m_generateUl.clear ();
  m_toSendDl.clear ();
  m_toSendUl.clear ();
  m_dlHarqfbPosition.clear ();

  GenerateStructuresFromPattern (pattern, &m_toSendDl, &m_toSendUl,
                                 &m_generateDl, &m_generateUl,
                                 &m_dlHarqfbPosition, 0,
                                 static_cast<uint32_t> (m_phyMacConfig->GetN2Delay ()),
                                 m_phyMacConfig->GetN1Delay (),
                                 m_phyMacConfig->GetL1L2CtrlLatency ());

  // At the beginning of the simulation, fill the slot allocations until
  // the mac will generate one. It means from slot 0 up to slot indicated in
  // the structure (with a possible wrap-around)
  if (Simulator::GetContext() == Simulator::NO_CONTEXT)
    {
      NS_ASSERT (m_generateDl.begin() != m_generateDl.end());
      NS_ASSERT (m_generateUl.begin() != m_generateUl.end());

      SfnSf sfnSf = SfnSf (m_frameNum, m_subframeNum, 0, 0);

      uint32_t times = m_generateDl.begin()->first + 0 + m_phyMacConfig->GetL1L2CtrlLatency() - 1; // Missing K0

      for (uint32_t i = 0; i <= times; ++i)
        {
          auto index = modulo (i, m_tddPattern.size ());
          if (m_tddPattern[index] == DL || m_tddPattern[index] == S || m_tddPattern[index] == F)
            {
              PushDlAllocation (sfnSf);
            }

          sfnSf.Add (1, m_phyMacConfig->GetSlotsPerSubframe(), m_phyMacConfig->GetSubframesPerFrame());
        }

      sfnSf = SfnSf (m_frameNum, m_subframeNum, 0, 0);

      times = m_generateUl.begin()->first + m_phyMacConfig->GetN2Delay () + m_phyMacConfig->GetL1L2CtrlLatency() - 1;

      for (uint32_t i = 0; i <= times; ++i)
        {
          auto index = modulo (i, m_tddPattern.size ());
          if (m_tddPattern[index] == UL || m_tddPattern[index] == F)
            {
              PushUlAllocation (sfnSf);
            }

          sfnSf.Add (1, m_phyMacConfig->GetSlotsPerSubframe(), m_phyMacConfig->GetSubframesPerFrame());
        }
    }
  else
    {
      // Don't change dynamically the pattern. What would happen with the
      // already scheduled slots?
      NS_FATAL_ERROR ("Changing TDD pattern dynamically is disabled.");
    }
}

void
MmWaveEnbPhy::SetConfigurationParameters (const Ptr<MmWavePhyMacCommon> &phyMacCommon)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phyMacConfig == nullptr);
  m_phyMacConfig = phyMacCommon;

  InitializeMessageList ();

  SetTddPattern (m_tddPattern); // Initialize everything needed for the TDD patterns
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

Ptr<MmWaveSpectrumPhy> MmWaveEnbPhy::GetSpectrumPhy() const
{
  return m_spectrumPhy;
}

void
MmWaveEnbPhy::QueueMib ()
{
  NS_LOG_FUNCTION (this);
  LteRrcSap::MasterInformationBlock mib;
  mib.dlBandwidth = 4U;
  mib.systemFrameNumber = 1;
  Ptr<MmWaveMibMessage> mibMsg = Create<MmWaveMibMessage> ();
  mibMsg->SetMib (mib);
  EnqueueCtrlMsgNow (mibMsg);
}

void MmWaveEnbPhy::QueueSib ()
{
  NS_LOG_FUNCTION (this);
  Ptr<MmWaveSib1Message> msg = Create<MmWaveSib1Message> ();
  msg->SetSib1 (m_sib1);
  msg->SetTddPattern (m_tddPattern);
  EnqueueCtrlMsgNow (msg);
}

void
MmWaveEnbPhy::CallMacForSlotIndication (const SfnSf &currentSlot)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_generateDl.empty());
  NS_ASSERT (!m_generateUl.empty());

  m_phySapUser->SetCurrentSfn (currentSlot);

  uint64_t currentSlotN = currentSlot.Normalize (m_phyMacConfig->GetSlotsPerSubframe (),
                                                 m_phyMacConfig->GetSubframesPerFrame ()) % m_tddPattern.size ();

  NS_LOG_INFO ("Start Slot " << currentSlot << ". In position " <<
               currentSlotN << " there is a slot of type " <<
               m_tddPattern[currentSlotN]);

  for (const auto & k2WithLatency : m_generateUl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;
      targetSlot.Add (k2WithLatency,
                      m_phyMacConfig->GetSlotsPerSubframe (),
                      m_phyMacConfig->GetSubframesPerFrame ());

      uint64_t pos = targetSlot.Normalize (m_phyMacConfig->GetSlotsPerSubframe (),
                                           m_phyMacConfig->GetSubframesPerFrame ()) % m_tddPattern.size ();

      NS_LOG_INFO (" in slot " << currentSlot << " generate UL for " <<
                     targetSlot << " which is of type " << m_tddPattern[pos]);

      m_phySapUser->SlotUlIndication (targetSlot, m_tddPattern[pos]);
    }

  for (const auto & k0WithLatency : m_generateDl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;
      targetSlot.Add (k0WithLatency,
                      m_phyMacConfig->GetSlotsPerSubframe (),
                      m_phyMacConfig->GetSubframesPerFrame ());

      uint64_t pos = targetSlot.Normalize (m_phyMacConfig->GetSlotsPerSubframe (),
                                           m_phyMacConfig->GetSubframesPerFrame ()) % m_tddPattern.size ();

      NS_LOG_INFO (" in slot " << currentSlot << " generate DL for " <<
                     targetSlot << " which is of type " << m_tddPattern[pos]);

      m_phySapUser->SlotDlIndication (targetSlot, m_tddPattern[pos]);
    }
}

void
MmWaveEnbPhy::StartSlot (uint16_t frameNum, uint8_t sfNum, uint16_t slotNum)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_channelStatus != TO_LOSE);

  m_frameNum = frameNum;
  m_subframeNum = sfNum;
  m_slotNum = static_cast<uint8_t> (slotNum);
  m_lastSlotStart = Simulator::Now ();
  m_varTtiNum = 0;

  m_lastSlotStart = Simulator::Now ();
  m_currSlotAllocInfo = RetrieveSlotAllocInfo ();

  const SfnSf currentSlot = SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0);

  NS_ASSERT_MSG ((m_currSlotAllocInfo.m_sfnSf.m_frameNum == m_frameNum)
                 && (m_currSlotAllocInfo.m_sfnSf.m_subframeNum == m_subframeNum)
                 && (m_currSlotAllocInfo.m_sfnSf.m_slotNum == m_slotNum ),
                 "Retrieved slot " << m_currSlotAllocInfo.m_sfnSf << " but we are on " << currentSlot );

  if (m_currSlotAllocInfo.m_varTtiAllocInfo.size () == 0)
    {
      NS_LOG_INFO ("gNB start  empty slot " << m_currSlotAllocInfo.m_sfnSf <<
                   " scheduling directly the end of the slot");
      Simulator::Schedule (m_phyMacConfig->GetSlotPeriod (), &MmWaveEnbPhy::EndSlot, this);
      return;
    }

  NS_ASSERT_MSG ((m_currSlotAllocInfo.m_sfnSf.m_frameNum == m_frameNum)
                 && (m_currSlotAllocInfo.m_sfnSf.m_subframeNum == m_subframeNum)
                 && (m_currSlotAllocInfo.m_sfnSf.m_slotNum == m_slotNum ),
                 "Current slot " << SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0) <<
                 " but allocation for " << m_currSlotAllocInfo.m_sfnSf);


  if (m_slotNum == 0)
    {
      if (m_subframeNum == 0)   //send MIB at the beginning of each frame
        {
          QueueMib ();
        }
      else if (m_subframeNum == 5)   // send SIB at beginning of second half-frame
        {
          QueueSib ();
        }
    }

  if (m_channelStatus == GRANTED)
    {
      NS_LOG_DEBUG ("Channel granted; asking MAC for SlotIndication for the future and then start the slot");
      CallMacForSlotIndication (currentSlot);
      DoStartSlot ();
    }
  else
    {
      bool hasUlDci = false;
      const SfnSf ulSfn = currentSlot.CalculateUplinkSlot (m_phyMacConfig->GetN2Delay (),
                                                           m_phyMacConfig->GetSlotsPerSubframe (),
                                                           m_phyMacConfig->GetSubframesPerFrame ());
      if (m_phyMacConfig->GetN2Delay () > 0)
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
              NS_LOG_DEBUG ("Channel not granted, request the channel");
              m_channelStatus = REQUESTED; // This goes always before RequestAccess()
              m_cam->RequestAccess ();
              if (m_channelStatus == GRANTED)
                {
                  // Repetition but we can have a CAM that gives the channel
                  // instantaneously
                  NS_LOG_DEBUG ("Channel granted; asking MAC for SlotIndication for the future and then start the slot");
                  CallMacForSlotIndication (currentSlot);

                  DoStartSlot ();
                  return; // Exit without calling anything else
                }
            }
          // If the channel was not granted, queue back the allocation,
          // without calling the MAC for a new slot
          auto slotAllocCopy = m_currSlotAllocInfo;
          auto newSfnSf = slotAllocCopy.m_sfnSf.IncreaseNoOfSlots(m_phyMacConfig->GetSlotsPerSubframe(),
                                                                  m_phyMacConfig->GetSubframesPerFrame());
          NS_LOG_INFO ("Queueing allocation in front for " << SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0));
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
          NS_LOG_DEBUG ("Channel not granted; but asking MAC for SlotIndication for the future, maybe there will be data");
          CallMacForSlotIndication (currentSlot);
        }
      // If we have the UL CTRL, then schedule it (we are listening, so
      // we don't need the channel. Otherwise, just go at the end of the
      // slot
      Time end = m_phyMacConfig->GetSlotPeriod () - NanoSeconds (1);
      if (m_currSlotAllocInfo.m_varTtiAllocInfo.size() > 0)
        {
          for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
            {
              if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL && alloc.m_dci->m_format == DciInfoElementTdma::UL)
                {
                  Time start = m_phyMacConfig->GetSymbolPeriod () * alloc.m_dci->m_symStart;
                  NS_LOG_DEBUG ("Schedule UL CTRL at " << start);
                  Simulator::Schedule (start, &MmWaveEnbPhy::UlCtrl, this, alloc.m_dci);
                  end = m_phyMacConfig->GetSymbolPeriod () * alloc.m_dci->m_numSym;
                }
            }
        }
      Simulator::Schedule (end, &MmWaveEnbPhy::EndSlot, this);
      NS_LOG_DEBUG ("Schedule end of slot at " << Simulator::Now () + end);
    }

}

void MmWaveEnbPhy::DoStartSlot ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_ctrlMsgs.size () == 0);
  NS_LOG_INFO ("Gnb Start Slot: " << m_currSlotAllocInfo);

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

  Time lastDataTime = m_phyMacConfig->GetSymbolPeriod() * lastDlSymbol;

  if (m_phyMacConfig->GetSlotPeriod () - lastDataTime > MicroSeconds (25))
    {
      NS_LOG_INFO ("Last symbol of data: " << +lastDlSymbol << ", to the end of slot we still have " <<
                   (m_phyMacConfig->GetSlotPeriod () - lastDataTime).GetMicroSeconds() <<
                   " us, so we're going to lose the channel");
      m_channelStatus = TO_LOSE;
    }
  else
    {
      NS_LOG_INFO ("Last symbol of data: " << +lastDlSymbol << ", to the end of slot we still have " <<
                   (m_phyMacConfig->GetSlotPeriod () - lastDataTime).GetMicroSeconds() <<
                   " us, so we're NOT going to lose the channel");
    }

  auto currentDci = m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum].m_dci;
  auto nextVarTtiStart = m_phyMacConfig->GetSymbolPeriod () * currentDci->m_symStart;

  Simulator::Schedule (nextVarTtiStart, &MmWaveEnbPhy::StartVarTti, this);
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
          NS_ASSERT_MSG (dciElem->m_symStart + dciElem->m_numSym <= m_phyMacConfig->GetSymbolsPerSlot (),
                         "symStart: " << static_cast<uint32_t> (dciElem->m_symStart) <<
                         " numSym: " << static_cast<uint32_t> (dciElem->m_numSym) <<
                         " symPerSlot: " << static_cast<uint32_t> (m_phyMacConfig->GetSymbolsPerSlot ()));

          NS_LOG_INFO ("Send DCI to " << dciElem->m_rnti << " from sym " <<
                         +dciElem->m_symStart << " to " << +dciElem->m_symStart + dciElem->m_numSym);

          Ptr<MmWaveTdmaDciMessage> dciMsg = Create<MmWaveTdmaDciMessage> (dciElem);

          dciMsg->SetKDelay (kDelay);
          dciMsg->SetK1Delay (k1Delay);  //Set for both DL/UL however used only in DL (in UL UE ignors it)

          ctrlMsgs.push_back (dciMsg);
          NS_LOG_INFO ("To send, DCI for UE " << dciElem->m_rnti);
        }
    }

  return ctrlMsgs;
}

std::list <Ptr<MmWaveControlMessage> >
MmWaveEnbPhy::RetrieveMsgsFromDCIs (const SfnSf &currentSlot)
{
  std::list <Ptr<MmWaveControlMessage> > ctrlMsgs;
  uint64_t currentSlotN = currentSlot.Normalize (m_phyMacConfig->GetSlotsPerSubframe (),
                                                 m_phyMacConfig->GetSubframesPerFrame ()) % m_tddPattern.size ();

  uint32_t k1delay = m_dlHarqfbPosition[currentSlotN];

  // TODO: copy paste :(
  for (const auto & k0delay : m_toSendDl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;

      targetSlot.Add (k0delay,
                      m_phyMacConfig->GetSlotsPerSubframe (),
                      m_phyMacConfig->GetSubframesPerFrame ());

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

      targetSlot.Add (k2delay,
                      m_phyMacConfig->GetSlotsPerSubframe (),
                      m_phyMacConfig->GetSubframesPerFrame ());

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

  SfnSf currentSlot = SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum);

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
  Time varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * m_phyMacConfig->GetDlCtrlSymbols ();

  NS_ASSERT(dci->m_numSym == m_phyMacConfig->GetDlCtrlSymbols ());

  // create control messages to be transmitted in DL-Control period

  EnqueueCtrlMsgNow (RetrieveMsgsFromDCIs (currentSlot));
  std::list <Ptr<MmWaveControlMessage>> ctrlMsgs = PopCurrentSlotCtrlMsgs ();

  if (ctrlMsgs.size () > 0)
    {
      NS_LOG_DEBUG ("ENB TXing DL CTRL with " << ctrlMsgs.size () << " msgs, frame " << m_frameNum <<
                    " subframe " << static_cast<uint32_t> (m_subframeNum) <<
                    " slot " << static_cast<uint32_t> (m_slotNum) <<
                    " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                    " start " << Simulator::Now () <<
                    " end " << Simulator::Now () + varTtiPeriod - NanoSeconds (1.0));

      for (auto ctrlIt = ctrlMsgs.begin (); ctrlIt != ctrlMsgs.end (); ++ctrlIt)
        {
          Ptr<MmWaveControlMessage> msg = (*ctrlIt);
          m_phyTxedCtrlMsgsTrace (SfnSf(m_frameNum, m_subframeNum, m_slotNum, dci->m_symStart),
                                  dci->m_rnti, m_phyMacConfig->GetCcId (), msg);
        }

      SendCtrlChannels (&ctrlMsgs, varTtiPeriod - NanoSeconds (1.0)); // -1 ns ensures control ends before data period
    }
  else
    {
      NS_LOG_INFO ("Scheduled time for DL CTRL but no messages to send");
    }

  return varTtiPeriod;
}

Time
MmWaveEnbPhy::UlCtrl(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  Time varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * m_phyMacConfig->GetUlCtrlSymbols ();

  NS_LOG_DEBUG ("ENB RXng UL CTRL frame " << m_frameNum <<
                " subframe " << static_cast<uint32_t> (m_subframeNum) <<
                " slot " << static_cast<uint32_t> (m_slotNum) <<
                " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                " start " << Simulator::Now () <<
                " end " << Simulator::Now () + varTtiPeriod);
  return varTtiPeriod;
}

Time
MmWaveEnbPhy::DlData (const VarTtiAllocInfo& varTtiInfo)
{
  NS_LOG_FUNCTION (this);
  Time varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * varTtiInfo.m_dci->m_numSym;

  Ptr<PacketBurst> pktBurst = GetPacketBurst (SfnSf (m_frameNum, m_subframeNum, m_slotNum, varTtiInfo.m_dci->m_symStart));
  if (pktBurst && pktBurst->GetNPackets () > 0)
    {
      std::list< Ptr<Packet> > pkts = pktBurst->GetPackets ();
      MmWaveMacPduTag macTag;
      pkts.front ()->PeekPacketTag (macTag);
      NS_ASSERT ((macTag.GetSfn ().m_slotNum == m_slotNum) && (macTag.GetSfn ().m_varTtiNum == varTtiInfo.m_dci->m_symStart));
    }
  else
    {
      // sometimes the UE will be scheduled when no data is queued
      // in this case, send an empty PDU
      MmWaveMacPduTag tag (SfnSf (m_frameNum, m_subframeNum, m_slotNum, varTtiInfo.m_dci->m_symStart));
      Ptr<Packet> emptyPdu = Create <Packet> ();
      MmWaveMacPduHeader header;
      MacSubheader subheader (3, 0);    // lcid = 3, size = 0
      header.AddSubheader (subheader);
      emptyPdu->AddHeader (header);
      emptyPdu->AddPacketTag (tag);
      LteRadioBearerTag bearerTag (varTtiInfo.m_dci->m_rnti, 3, 0);
      emptyPdu->AddPacketTag (bearerTag);
      pktBurst = CreateObject<PacketBurst> ();
      pktBurst->AddPacket (emptyPdu);
    }

  NS_LOG_DEBUG ("ENB TXing DL DATA frame " << m_frameNum <<
                " subframe " << static_cast<uint32_t> (m_subframeNum) <<
                " slot " << static_cast<uint32_t> (m_slotNum) <<
                " symbols "  << static_cast<uint32_t> (varTtiInfo.m_dci->m_symStart) <<
                "-" << static_cast<uint32_t> (varTtiInfo.m_dci->m_symStart + varTtiInfo.m_dci->m_numSym - 1) <<
                " start " << Simulator::Now () + NanoSeconds (1) <<
                " end " << Simulator::Now () + varTtiPeriod - NanoSeconds (2.0));

  Simulator::Schedule (NanoSeconds (1.0), &MmWaveEnbPhy::SendDataChannels, this,
                       pktBurst, varTtiPeriod - NanoSeconds (2.0), varTtiInfo);

  return varTtiPeriod;
}

Time
MmWaveEnbPhy::UlData(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_INFO (this);

  // Assert: we expect TDMA in UL
  NS_ASSERT (dci->m_rbgBitmask.size () == m_phyMacConfig->GetBandwidthInRbg ());
  NS_ASSERT (static_cast<uint32_t> (std::count (dci->m_rbgBitmask.begin (),
                                                dci->m_rbgBitmask.end (), 1U)) == dci->m_rbgBitmask.size ());

  Time varTtiPeriod = m_phyMacConfig->GetSymbolPeriod () * dci->m_numSym;

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

  NS_LOG_DEBUG ("ENB RXing UL DATA frame " << m_frameNum <<
                " subframe " << static_cast<uint32_t> (m_subframeNum) <<
                " slot " << static_cast<uint32_t> (m_slotNum) <<
                " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                " start " << Simulator::Now () <<
                " end " << Simulator::Now () + varTtiPeriod);
  return varTtiPeriod;
}

void
MmWaveEnbPhy::StartVarTti (void)
{
  NS_LOG_FUNCTION (this);

  //assume the control signal is omni
  NS_ABORT_MSG_IF(m_beamManager == nullptr, "Beam manager not initialized");
  m_beamManager->ChangeToOmniTx(); //assume the control signal is omni

  VarTtiAllocInfo & currVarTti = m_currSlotAllocInfo.m_varTtiAllocInfo[m_varTtiNum];
  m_currSymStart = currVarTti.m_dci->m_symStart;
  SfnSf sfn = SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum);
  NS_LOG_INFO ("Starting VarTti on the AIR " << sfn);

  Time varTtiPeriod;

  NS_ASSERT (currVarTti.m_dci->m_type != DciInfoElementTdma::CTRL_DATA);

  if (currVarTti.m_dci->m_type == DciInfoElementTdma::CTRL)
    {
      if (currVarTti.m_dci->m_format == DciInfoElementTdma::DL)
        {
          varTtiPeriod = DlCtrl (currVarTti.m_dci);
        }
      else if (currVarTti.m_dci->m_format == DciInfoElementTdma::UL)
        {
          varTtiPeriod = UlCtrl (currVarTti.m_dci);
        }
    }
  else  if (currVarTti.m_dci->m_type == DciInfoElementTdma::DATA)
    {
      if (currVarTti.m_dci->m_format == DciInfoElementTdma::DL)
        {
          varTtiPeriod = DlData (currVarTti);
        }
      else if (currVarTti.m_dci->m_format == DciInfoElementTdma::UL)
        {
          varTtiPeriod = UlData (currVarTti.m_dci);
        }
    }

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

  NS_ABORT_MSG_IF(m_beamManager == nullptr, "Beam manager not initialized");
  m_beamManager->ChangeToOmniTx(); //assume the control signal is omni

  if (m_varTtiNum == m_currSlotAllocInfo.m_varTtiAllocInfo.size () - 1)
    {
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

  if (m_channelStatus == TO_LOSE)
    {
      NS_LOG_INFO ("Release the channel because we did not have any data to maintain the grant");
      m_channelStatus = NONE;
      m_channelLostTimer.Cancel ();
    }

  NS_ASSERT_MSG (slotStart > MilliSeconds (0),
                 "lastStart=" << m_lastSlotStart + m_phyMacConfig->GetSlotPeriod () <<
                 " now " <<  Simulator::Now () << " slotStart value" << slotStart);

  SfnSf sfnf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum);

  SfnSf retVal = sfnf.IncreaseNoOfSlots (m_phyMacConfig->GetSlotsPerSubframe (),
                                         m_phyMacConfig->GetSubframesPerFrame ());

  Simulator::Schedule (slotStart, &MmWaveEnbPhy::StartSlot, this,
                       retVal.m_frameNum, retVal.m_subframeNum,
                       static_cast<uint8_t> (retVal.m_slotNum));
}

void
MmWaveEnbPhy::SendDataChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod,
                                const VarTtiAllocInfo& varTtiInfo)
{
  if (varTtiInfo.m_isOmni)
    {
	  NS_ABORT_MSG_IF(m_beamManager == nullptr, "Beam manager not initialized");
	  m_beamManager->ChangeToOmniTx(); //assume the control signal is omni
    }
  else
    {   // update beamforming vectors (currently supports 1 user only)
        //std::map<uint16_t, std::vector<unsigned> >::iterator ueRbIt = varTtiInfo.m_ueRbMap.begin();
        //uint16_t rnti = ueRbIt->first;
      bool found = false;
      for (uint8_t i = 0; i < m_deviceMap.size (); i++)
        {
          Ptr<MmWaveUeNetDevice> ueDev = DynamicCast<MmWaveUeNetDevice> (m_deviceMap.at (i));
          uint64_t ueRnti = (DynamicCast<MmWaveUePhy>(ueDev->GetPhy (0)))->GetRnti ();
          //NS_LOG_INFO ("Scheduled rnti:"<<rnti <<" ue rnti:"<< ueRnti);
          if (varTtiInfo.m_dci->m_rnti == ueRnti)
            {
              NS_ABORT_MSG_IF(m_beamManager == nullptr, "Beam manager not initialized");
              m_beamManager->ChangeBeamformingVector(m_deviceMap.at (i));
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
  NS_ASSERT (varTtiInfo.m_dci != nullptr);
  NS_ASSERT (m_rbgAllocationPerSym.find(varTtiInfo.m_dci->m_symStart) != m_rbgAllocationPerSym.end ());
  SetSubChannels (FromRBGBitmaskToRBAssignment (m_rbgAllocationPerSym.at (varTtiInfo.m_dci->m_symStart)));

  std::list<Ptr<MmWaveControlMessage> > ctrlMsgs;
  m_spectrumPhy->StartTxDataFrames (pb, ctrlMsgs, varTtiPeriod, varTtiInfo.m_dci->m_symStart);
}

void
MmWaveEnbPhy::SendCtrlChannels (std::list<Ptr<MmWaveControlMessage> > *ctrlMsgs,
                                const Time &varTtiPeriod)
{
  NS_LOG_FUNCTION (this << "Send Ctrl");

  std::vector <int> fullBwRb (m_phyMacConfig->GetBandwidthInRbs ());
  // The first time set the right values for the phy
  for (uint32_t i = 0; i < m_phyMacConfig->GetBandwidthInRbs (); ++i)
    {
      fullBwRb.at (i) = static_cast<int> (i);
    }

  SetSubChannels (fullBwRb);

  m_spectrumPhy->StartTxDlControlFrames (*ctrlMsgs, varTtiPeriod);

  ctrlMsgs->clear ();
}

bool
MmWaveEnbPhy::RegisterUe (uint64_t imsi, const Ptr<NetDevice> &ueDevice)
{
  NS_LOG_FUNCTION (this << imsi);
  std::set <uint64_t>::iterator it;
  it = m_ueAttached.find (imsi);

  if (it == m_ueAttached.end ())
    {
      m_ueAttached.insert (imsi);
      m_deviceMap.push_back (ueDevice);
      if (m_idealBeamformingEnabled)
        {
          m_beamManager->GetIdealBeamformingAlgorithm ()->AddUeDevice (ueDevice);
        }
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
MmWaveEnbPhy::PhyCtrlMessagesReceived (const std::list<Ptr<MmWaveControlMessage> > &msgList)
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

  auto ctrlIt = msgList.begin ();

  while (ctrlIt != msgList.end ())
    {
      Ptr<MmWaveControlMessage> msg = (*ctrlIt);

      if (msg->GetMessageType () == MmWaveControlMessage::DL_CQI)
        {
          NS_LOG_INFO ("received CQI");

          Ptr<MmWaveDlCqiMessage> dlcqi = DynamicCast<MmWaveDlCqiMessage> (msg);
          DlCqiInfo dlcqiLE = dlcqi->GetDlCqi ();
          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  dlcqiLE.m_rnti, m_phyMacConfig->GetCcId (), msg);

          NS_LOG_INFO ("Received DL_CQI for RNTI: " << dlcqiLE.m_rnti << " in slot " <<
                       SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum) <<
                       ", scheduling MAC ReceiveControlMessage after the decode latency");
          Simulator::Schedule( MicroSeconds (m_phyMacConfig->GetTbDecodeLatency()),
                               &MmWaveEnbPhySapUser::ReceiveControlMessage, m_phySapUser, msg);
        }
      else if (msg->GetMessageType () == MmWaveControlMessage::BSR)
        {
          NS_LOG_INFO ("received BSR");

          Ptr<MmWaveBsrMessage> bsrmsg = DynamicCast<MmWaveBsrMessage> (msg);
          MacCeElement macCeEl = bsrmsg->GetBsr();
          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  macCeEl.m_rnti, m_phyMacConfig->GetCcId (), msg);

          NS_LOG_INFO ("Received BSR for RNTI: " << macCeEl.m_rnti << " in slot " <<
                       SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum) <<
                       ", scheduling MAC ReceiveControlMessage after the decode latency");
          Simulator::Schedule( MicroSeconds (m_phyMacConfig->GetTbDecodeLatency()),
                               &MmWaveEnbPhySapUser::ReceiveControlMessage, m_phySapUser, msg);
        }
      else if (msg->GetMessageType () == MmWaveControlMessage::RACH_PREAMBLE)
        {
          NS_LOG_INFO ("received RACH_PREAMBLE");
          NS_ASSERT (m_cellId > 0);

          Ptr<MmWaveRachPreambleMessage> rachPreamble = DynamicCast<MmWaveRachPreambleMessage> (msg);
//          m_phySapUser->ReceiveRachPreamble (rachPreamble->GetRapId ());
          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  0, m_phyMacConfig->GetCcId (), msg);
          NS_LOG_INFO ("Received RACH Preamble in slot " <<
                                 SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum) <<
                                 ", scheduling MAC ReceiveControlMessage after the decode latency");
          Simulator::Schedule( MicroSeconds (m_phyMacConfig->GetTbDecodeLatency()),
                               &MmWaveEnbPhySapUser::ReceiveRachPreamble, m_phySapUser,
                               rachPreamble->GetRapId ());

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
              m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                      dlharq.m_rnti, m_phyMacConfig->GetCcId (), msg);

              NS_LOG_INFO ("Received DL_HARQ for RNTI: " << dlharq.m_rnti << " in slot " <<
                           SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum) <<
                           ", scheduling MAC ReceiveControlMessage after the decode latency");
              Simulator::Schedule( MicroSeconds (m_phyMacConfig->GetTbDecodeLatency()),
                                   &MmWaveEnbPhySapUser::ReceiveControlMessage, m_phySapUser, msg);
            }
        }
      else
        {
          //m_phySapUser->ReceiveControlMessage (msg);

          m_phyRxedCtrlMsgsTrace (SfnSf (m_frameNum, m_subframeNum, m_slotNum, m_varTtiNum),
                                  0, m_phyMacConfig->GetCcId (), msg);

          Simulator::Schedule( MicroSeconds (m_phyMacConfig->GetTbDecodeLatency()),
                               &MmWaveEnbPhySapUser::ReceiveControlMessage, m_phySapUser, msg);
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
  NS_LOG_FUNCTION (this << +ulBandwidth << +dlBandwidth);
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
      m_phySapUser->UlHarqFeedback (mes);
    }
}

void
MmWaveEnbPhy::ChannelAccessGranted (const Time &time)
{
  NS_LOG_FUNCTION (this);

  if (time < m_phyMacConfig->GetSlotPeriod ())
    {
      NS_LOG_INFO ("Channel granted for less than the slot time. Ignoring the grant.");
      m_channelStatus = NONE;
      return;
    }

  m_channelStatus = GRANTED;

  Time toNextSlot = m_lastSlotStart + m_phyMacConfig->GetSlotPeriod () - Simulator::Now ();
  Time grant = time - toNextSlot;
  int64_t slotGranted = grant.GetNanoSeconds () / m_phyMacConfig->GetSlotPeriod().GetNanoSeconds ();

  NS_LOG_INFO ("Channel access granted for " << time.GetMilliSeconds () <<
               " ms, which corresponds to " << slotGranted << " slot in which each slot is " <<
               m_phyMacConfig->GetSlotPeriod() << " ms. We lost " <<
               toNextSlot.GetMilliSeconds() << " ms. ");
  NS_LOG_DEBUG ("Channel access granted for " << slotGranted << " slot");
  NS_ASSERT(! m_channelLostTimer.IsRunning ());

  slotGranted = std::max (1L, slotGranted);
  m_channelLostTimer = Simulator::Schedule (m_phyMacConfig->GetSlotPeriod () * slotGranted - NanoSeconds (1),
                                            &MmWaveEnbPhy::ChannelAccessLost, this);
}

void
MmWaveEnbPhy::ChannelAccessLost ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Channel access lost");
  m_channelStatus = NONE;
}

}
