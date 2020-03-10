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

#include "mmwave-spectrum-phy.h"
#include "mmwave-enb-net-device.h"
#include "mmwave-ue-net-device.h"
#include "mmwave-mac-pdu-tag.h"
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <functional>
#include <ns3/three-gpp-antenna-array-model.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/trace-source-accessor.h>
#include "nr-lte-mi-error-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveSpectrumPhy");
NS_OBJECT_ENSURE_REGISTERED (MmWaveSpectrumPhy);

std::string ToString (enum MmWaveSpectrumPhy::State state)
{
  switch (state)
  {
    case MmWaveSpectrumPhy::TX:
      return "TX";
      break;
    case MmWaveSpectrumPhy::RX_DL_CTRL:
      return "RX_DL_CTRL";
      break;
    case MmWaveSpectrumPhy::RX_UL_CTRL:
      return "RX_UL_CTRL";
      break;
    case MmWaveSpectrumPhy::CCA_BUSY:
      return "CCA_BUSY";
      break;
    case MmWaveSpectrumPhy::RX_DATA:
      return "RX_DATA";
      break;
    case MmWaveSpectrumPhy::IDLE:
      return "IDLE";
      break;
    default:
      NS_ABORT_MSG ("Unknown state.");
  }
}

MmWaveSpectrumPhy::MmWaveSpectrumPhy ()
  : SpectrumPhy (),
    m_cellId (0),
  m_state (IDLE),
  m_unlicensedMode (false),
  m_busyTimeEnds (Seconds (0))
{
  m_interferenceData = CreateObject<mmWaveInterference> ();
  m_random = CreateObject<UniformRandomVariable> ();
  m_random->SetAttribute ("Min", DoubleValue (0.0));
  m_random->SetAttribute ("Max", DoubleValue (1.0));
  m_antenna = nullptr;
}
MmWaveSpectrumPhy::~MmWaveSpectrumPhy ()
{

}

TypeId
MmWaveSpectrumPhy::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::MmWaveSpectrumPhy")
    .SetParent<NetDevice> ()
    .AddConstructor<MmWaveSpectrumPhy> ()
    .AddAttribute ("UnlicensedMode",
                   "Activate/Deactivate unlicensed mode in which energy detection is performed" 
                   " and PHY state machine has an additional state CCA_BUSY.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&MmWaveSpectrumPhy::m_unlicensedMode),
                   MakeBooleanChecker ())
    .AddAttribute ("CcaMode1Threshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to declare CCA BUSY state.",
                   DoubleValue (-62.0),
                   MakeDoubleAccessor (&MmWaveSpectrumPhy::SetCcaMode1Threshold,
                                       &MmWaveSpectrumPhy::GetCcaMode1Threshold),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("RxPacketTraceEnb",
                     "The no. of packets received and transmitted by the Base Station",
                     MakeTraceSourceAccessor (&MmWaveSpectrumPhy::m_rxPacketTraceEnb),
                     "ns3::EnbTxRxPacketCount::TracedCallback")
    .AddTraceSource ("TxPacketTraceEnb",
                     "Traces when the packet is being transmitted by the Base Station",
                     MakeTraceSourceAccessor (&MmWaveSpectrumPhy::m_txPacketTraceEnb),
                     "ns3::StartTxPacketEnb::TracedCallback")
    .AddTraceSource ("RxPacketTraceUe",
                     "The no. of packets received and transmitted by the User Device",
                     MakeTraceSourceAccessor (&MmWaveSpectrumPhy::m_rxPacketTraceUe),
                     "ns3::UeTxRxPacketCount::TracedCallback")
    .AddAttribute ("DataErrorModelEnabled",
                   "Activate/Deactivate the error model of data (TBs of PDSCH and PUSCH) [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&MmWaveSpectrumPhy::m_dataErrorModelEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("ErrorModelType",
                   "Type of the Error Model to apply to TBs of PDSCH and PUSCH",
                   TypeIdValue (NrLteMiErrorModel::GetTypeId ()),
                   MakeTypeIdAccessor (&MmWaveSpectrumPhy::m_errorModelType),
                   MakeTypeIdChecker ())
    .AddTraceSource ("ChannelOccupied",
                     "This traced callback is triggered every time that the channel is occupied",
                     MakeTraceSourceAccessor (&MmWaveSpectrumPhy::m_channelOccupied),
                     "ns3::MmWaveSpectrumPhy::ChannelOccupiedTracedCallback")
    .AddTraceSource ("TxDataTrace",
                     "Indicates when the channel is being occupied by a data transmission",
                     MakeTraceSourceAccessor (&MmWaveSpectrumPhy::m_txDataTrace),
                     "ns3::MmWaveSpectrumPhy::ChannelOccupiedTracedCallback")
    .AddTraceSource ("TxCtrlTrace",
                     "Indicates when the channel is being occupied by a ctrl transmission",
                     MakeTraceSourceAccessor (&MmWaveSpectrumPhy::m_txCtrlTrace),
                     "ns3::MmWaveSpectrumPhy::ChannelOccupiedTracedCallback")
    .AddAttribute ("EnableAllInterferences",
                   "If true, enables gNB-gNB and UE-UE interferences,"
                   "if false, gNB-gNB and UE-UE are not taken into account. "
                   "This parameter is true, requires generation of pathloss "
                   "and channels in between gNB-gNB and UE-UE, through "
                   "EnableAllChannels attribute in 3gpp-channel",
                    BooleanValue (false),
                    MakeBooleanAccessor (&MmWaveSpectrumPhy::m_enableAllInterferences),
                    MakeBooleanChecker ())
  ;

  return tid;
}
void
MmWaveSpectrumPhy::DoDispose ()
{

}

void
MmWaveSpectrumPhy::SetDevice (Ptr<NetDevice> d)
{
  m_device = d;
}

void
MmWaveSpectrumPhy::SetCcaMode1Threshold (double thresholdDBm)
{
  NS_LOG_FUNCTION (this << thresholdDBm);
  // convert dBm to Watt
  m_ccaMode1ThresholdW = (std::pow (10.0, thresholdDBm / 10.0)) / 1000.0;
}

double
MmWaveSpectrumPhy::GetCcaMode1Threshold (void) const
{
  // convert Watt to dBm
  return 10.0 * std::log10 (m_ccaMode1ThresholdW * 1000.0);
}

Ptr<NetDevice>
MmWaveSpectrumPhy::GetDevice () const
{
  return m_device;
}

void
MmWaveSpectrumPhy::SetMobility (Ptr<MobilityModel> m)
{
  m_mobility = m;
}

Ptr<MobilityModel>
MmWaveSpectrumPhy::GetMobility ()
{
  return m_mobility;
}

void
MmWaveSpectrumPhy::SetChannel (Ptr<SpectrumChannel> c)
{
  m_channel = c;
}

Ptr<const SpectrumModel>
MmWaveSpectrumPhy::GetRxSpectrumModel () const
{
  return m_rxSpectrumModel;
}

Ptr<AntennaModel>
MmWaveSpectrumPhy::GetRxAntenna ()
{
   NS_LOG_WARN("In NR module can be used only ThreeGppAntennaArrayModel antenna type.");
   return nullptr;
}


Ptr<ThreeGppAntennaArrayModel>
MmWaveSpectrumPhy::GetAntennaArray ()
{
   return m_antenna;
}

void
MmWaveSpectrumPhy::SetAntennaArray (Ptr<ThreeGppAntennaArrayModel> a)
{
  NS_ABORT_IF (m_antenna != nullptr);
  m_antenna = a;
}

void
MmWaveSpectrumPhy::ChangeState (State newState, Time duration)
{
  NS_LOG_LOGIC (this << " change state: " << ToString (m_state) << " -> " << ToString (newState));
  m_state = newState;

  if (newState == RX_DATA || newState == RX_DL_CTRL || newState == RX_UL_CTRL || newState == TX || newState == CCA_BUSY)
    {
      m_channelOccupied (duration);
    }
}


void
MmWaveSpectrumPhy::SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd)
{
  NS_LOG_FUNCTION (this << noisePsd);
  NS_ASSERT (noisePsd);
  m_rxSpectrumModel = noisePsd->GetSpectrumModel ();
  m_interferenceData->SetNoisePowerSpectralDensity (noisePsd);

}

void
MmWaveSpectrumPhy::SetTxPowerSpectralDensity (Ptr<SpectrumValue> TxPsd)
{
  m_txPsd = TxPsd;
}

void
MmWaveSpectrumPhy::SetPhyRxDataEndOkCallback (MmWavePhyRxDataEndOkCallback c)
{
  m_phyRxDataEndOkCallback = c;
}


void
MmWaveSpectrumPhy::SetPhyRxCtrlEndOkCallback (const MmWavePhyRxCtrlEndOkCallback &c)
{
  m_phyRxCtrlEndOkCallback = c;
}

void
MmWaveSpectrumPhy::AddExpectedTb (uint16_t rnti, uint8_t ndi, uint32_t size, uint8_t mcs,
                                  const std::vector<int> &rbMap, uint8_t harqId, uint8_t rv, bool downlink,
                                  uint8_t symStart, uint8_t numSym)
{
  NS_LOG_FUNCTION (this);
  auto it = m_transportBlocks.find (rnti);
  if (it != m_transportBlocks.end ())
    {
      // migth be a TB of an unreceived packet (due to high progpalosses)
      m_transportBlocks.erase (it);
    }

  m_transportBlocks.emplace (std::make_pair(rnti, TransportBlockInfo(ExpectedTb (ndi, size, mcs,
                                                                                rbMap, harqId, rv,
                                                                                downlink, symStart,
                                                                                numSym))));
  NS_LOG_INFO ("Add expected TB for rnti " << rnti << " size=" << size <<
               " mcs=" << static_cast<uint32_t> (mcs) << " symstart=" <<
               static_cast<uint32_t> (symStart) << " numSym=" <<
               static_cast<uint32_t> (numSym));
}

bool
MmWaveSpectrumPhy::IsEnb () const
{
  return (DynamicCast<MmWaveEnbNetDevice> (GetDevice ()) != nullptr);
}

void
MmWaveSpectrumPhy::SetPhyDlHarqFeedbackCallback (MmWavePhyDlHarqFeedbackCallback c)
{
  NS_LOG_FUNCTION (this);
  m_phyDlHarqFeedbackCallback = c;
}

void
MmWaveSpectrumPhy::SetPhyUlHarqFeedbackCallback (MmWavePhyUlHarqFeedbackCallback c)
{
  NS_LOG_FUNCTION (this);
  m_phyUlHarqFeedbackCallback = c;
}

void
MmWaveSpectrumPhy::StartRx (Ptr<SpectrumSignalParameters> params)
{
  NS_LOG_FUNCTION (this);
  Ptr <const SpectrumValue> rxPsd = params->psd;
  Time duration = params->duration;
  NS_LOG_INFO ("Start receiving signal: " << rxPsd <<" duration= " << duration);

  if (m_enableAllInterferences == false)
    {
      // if false, ignore gNB-to-gNB and UE-to-UE interferences
      Ptr<MmWaveEnbNetDevice> enbTx =
          DynamicCast<MmWaveEnbNetDevice> (params->txPhy->GetDevice ());
      Ptr<MmWaveEnbNetDevice> enbRx =
          DynamicCast<MmWaveEnbNetDevice> (GetDevice ());

      Ptr<MmWaveUeNetDevice> ueTx =
          DynamicCast<MmWaveUeNetDevice> (params->txPhy->GetDevice ());
      Ptr<MmWaveUeNetDevice> ueRx =
          DynamicCast<MmWaveUeNetDevice> (GetDevice ());

      if ((enbTx != 0 && enbRx != 0) || (ueTx != 0 && ueRx != 0))
        {
          NS_LOG_INFO ("BS to BS or UE to UE transmission neglected.");
          return;
        }
    }

  // pass it to interference calculations regardless of the type (mmwave or non-mmwave)
  m_interferenceData->AddSignal (rxPsd, duration);

  Ptr<MmwaveSpectrumSignalParametersDataFrame> mmwaveDataRxParams =
    DynamicCast<MmwaveSpectrumSignalParametersDataFrame> (params);

  Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame> dlCtrlRxParams =
    DynamicCast<MmWaveSpectrumSignalParametersDlCtrlFrame> (params);

  Ptr<MmWaveSpectrumSignalParametersUlCtrlFrame> ulCtrlRxParams =
    DynamicCast<MmWaveSpectrumSignalParametersUlCtrlFrame> (params);

  if (mmwaveDataRxParams != nullptr)
    {
      if (mmwaveDataRxParams->cellId == m_cellId)
        {
          StartRxData (mmwaveDataRxParams);
        }
      else
        {
          NS_LOG_INFO (" Received DATA not in sync with this signal (cellId=" <<
                       mmwaveDataRxParams->cellId  << ", m_cellId=" << m_cellId << ")");
        }
    }
  else if (dlCtrlRxParams != nullptr)
    {
      if (!IsEnb ())
        {
          if (dlCtrlRxParams->cellId == m_cellId)
            {
              StartRxDlCtrl (dlCtrlRxParams);
            }
          else
            {
              NS_LOG_INFO ("Received DL CTRL, but not in sync with this signal (cellId=" <<
                       dlCtrlRxParams->cellId  << ", m_cellId=" << m_cellId << ")");
            }
        }
      else
        {
          NS_LOG_DEBUG ("DL CTRL ignored at gNB");
        }
    }
  else if (ulCtrlRxParams != nullptr)
    {
      if (IsEnb ()) // only gNBs should enter into reception of UL CTRL signals
        {
          if (ulCtrlRxParams->cellId == m_cellId)
            {
              StartRxUlCtrl (ulCtrlRxParams);
            }
          else
            {
              NS_LOG_INFO ("Received UL CTRL, but not in sync with this signal (cellId=" <<
                           ulCtrlRxParams->cellId  << ", m_cellId=" << m_cellId << ")");
            }
        }
      else
        {
           NS_LOG_DEBUG ("UL CTRL ignored at UE device");
        }
    }
  else
    {
      NS_LOG_INFO ("Received non-mmwave signal of duration:" << duration);
    }

  // If in RX or TX state, do not change to CCA_BUSY until is finished
  // RX or TX state. If in IDLE state, then ok, move to CCA_BUSY if the
  // channel is found busy.
  if (m_unlicensedMode && m_state == IDLE)
    {
      MaybeCcaBusy ();
    }
}

void
MmWaveSpectrumPhy::StartRxData (Ptr<MmwaveSpectrumSignalParametersDataFrame> params)
{
  NS_LOG_FUNCTION (this);

  switch (m_state)
    {
    case TX:
      NS_FATAL_ERROR ("Cannot RX while TX.");
      break;
    case RX_DL_CTRL:
      /* no break */
    case RX_UL_CTRL:
      NS_FATAL_ERROR ("Cannot receive DATA while receiving CTRL.");
      break;
    case CCA_BUSY:
      NS_LOG_INFO ("Start receiving DATA while in CCA_BUSY state.");
      /* no break */
    case RX_DATA: // RX_DATA while RX_DATA is possible with OFDMA, i.e. gNB receives from multiple UEs at the same time
      /* no break */
    case IDLE:
      {
        m_interferenceData->StartRx (params->psd);

        if (m_rxPacketBurstList.empty ())
          {
            NS_ASSERT (m_state == IDLE || m_state == CCA_BUSY);
            // first transmission, i.e., we're IDLE and we start RX
            m_firstRxStart = Simulator::Now ();
            m_firstRxDuration = params->duration;
            NS_LOG_LOGIC (this << " scheduling EndRx with delay " << params->duration.GetSeconds () << "s");

            Simulator::Schedule (params->duration, &MmWaveSpectrumPhy::EndRxData, this);
          }
        else
          {
            NS_ASSERT (m_state == RX_DATA);
            // sanity check: if there are multiple RX events, they
            // should occur at the same time and have the same
            // duration, otherwise the interference calculation
            // won't be correct
            NS_ASSERT ((m_firstRxStart == Simulator::Now ()) && (m_firstRxDuration == params->duration));
          }

        ChangeState (RX_DATA, params->duration);

        if (params->packetBurst && !params->packetBurst->GetPackets ().empty ())
          {
            m_rxPacketBurstList.push_back (params->packetBurst);
          }
        //NS_LOG_DEBUG (this << " insert msgs " << params->ctrlMsgList.size ());
        m_rxControlMessageList.insert (m_rxControlMessageList.end (), params->ctrlMsgList.begin (), params->ctrlMsgList.end ());

        NS_LOG_LOGIC (this << " numSimultaneousRxEvents = " << m_rxPacketBurstList.size ());
      }
      break;
    default:
      NS_FATAL_ERROR ("Programming Error: Unknown State");
    }
}

void
MmWaveSpectrumPhy::StartRxDlCtrl (Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame> params)
{
  // The current code of this function assumes:
  // that this function is called only when cellId = m_cellId, which means
  // that UE can start to receive DL CTRL only from its own cellId,
  // and CTRL from other cellIds will be ignored
  NS_LOG_FUNCTION (this);
  NS_ASSERT (params->cellId == m_cellId && !IsEnb ());
  // RDF: method currently supports Downlink control only!
  switch (m_state)
    {
    case TX:
      NS_FATAL_ERROR ("Cannot RX while TX.");
      break;
    case RX_DATA:
      NS_FATAL_ERROR ("Cannot RX CTRL while receiving DATA.");
      break;
    case RX_DL_CTRL:
      NS_FATAL_ERROR ("Cannot RX DL CTRL while already receiving DL CTRL.");
      break;
    case RX_UL_CTRL:
      NS_FATAL_ERROR ("UE should never be in RX_UL_CTRL state.");
      break;
    case CCA_BUSY:
      NS_LOG_INFO ("Start receiving CTRL while channel in CCA_BUSY state.");
      /* no break */
    case IDLE:
      {
        NS_ASSERT (m_rxControlMessageList.empty ());
        NS_LOG_LOGIC (this << "receiving DL CTRL from cellId:"<<params->cellId<< "and scheduling EndRx with delay " << params->duration);
        // store the DCIs
        m_rxControlMessageList = params->ctrlMsgList;
        Simulator::Schedule (params->duration, &MmWaveSpectrumPhy::EndRxCtrl, this);
        ChangeState (RX_DL_CTRL, params->duration);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unknown state.");
        break;
      }
    }
}

void
MmWaveSpectrumPhy::StartRxUlCtrl (Ptr<MmWaveSpectrumSignalParametersUlCtrlFrame> params)
{
  // The current code of this function assumes:
  // 1) that this function is called only when cellId = m_cellId
  // 2) this function should be only called for gNB, only gNB should enter into reception of UL CTRL signals
  // 3) gNB can receive simultaneously signals from various UEs
  NS_LOG_FUNCTION (this);
  NS_ASSERT (params->cellId == m_cellId && IsEnb ());
  // RDF: method currently supports Uplink control only!
  switch (m_state)
    {
    case TX:
      NS_FATAL_ERROR ("Cannot RX UL CTRL while TX.");
      break;
    case RX_DATA:
      NS_FATAL_ERROR ("Cannot RX UL CTRL while receiving DATA.");
      break;
    case RX_DL_CTRL:
      NS_FATAL_ERROR ("gNB should not be in RX_DL_CTRL state.");
      break;
    case CCA_BUSY:
      NS_LOG_INFO ("Start receiving UL CTRL while channel in CCA_BUSY state.");
      /* no break */
    case RX_UL_CTRL:
      /* no break */
    case IDLE:
      {
        // at the gNB we can receive more UL CTRL signals simultaneously
        if (m_state == IDLE || m_state == CCA_BUSY)
          {
            // first transmission, i.e., we're IDLE and we start RX
            NS_ASSERT (m_rxControlMessageList.empty ());
            m_firstRxStart = Simulator::Now ();
            m_firstRxDuration = params->duration;
            NS_LOG_LOGIC (this << " scheduling EndRx with delay " << params->duration);
            // store the DCIs
            m_rxControlMessageList = params->ctrlMsgList;
            Simulator::Schedule (params->duration, &MmWaveSpectrumPhy::EndRxCtrl, this);
            ChangeState (RX_UL_CTRL, params->duration);
          }
        else // already in RX_UL_CTRL state, just add new CTRL messages from other UE
          {
            NS_ASSERT ((m_firstRxStart == Simulator::Now ()) && (m_firstRxDuration == params->duration));
            m_rxControlMessageList.insert (m_rxControlMessageList.end (), params->ctrlMsgList.begin (), params->ctrlMsgList.end ());
          }
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("unknown state");
        break;
      }
    }
}

void
MmWaveSpectrumPhy::EndRxData ()
{
  NS_LOG_FUNCTION (this);
  m_interferenceData->EndRx ();

  Ptr<MmWaveEnbNetDevice> enbRx = DynamicCast<MmWaveEnbNetDevice> (GetDevice ());
  Ptr<MmWaveUeNetDevice> ueRx = DynamicCast<MmWaveUeNetDevice> (GetDevice ());

  NS_ASSERT (m_state == RX_DATA);

  GetSecond GetTBInfo;
  GetFirst GetRnti;

  for (auto &tbIt : m_transportBlocks)
    {
      GetTBInfo(tbIt).m_sinrAvg = 0.0;
      GetTBInfo(tbIt).m_sinrMin = 99999999999;
      for (const auto & rbIndex : GetTBInfo(tbIt).m_expected.m_rbBitmap)
        {
          GetTBInfo(tbIt).m_sinrAvg += m_sinrPerceived.ValuesAt (rbIndex);
          if (m_sinrPerceived.ValuesAt (rbIndex) < GetTBInfo(tbIt).m_sinrMin)
            {
              GetTBInfo(tbIt).m_sinrMin = m_sinrPerceived.ValuesAt (rbIndex);
            }
        }

      GetTBInfo(tbIt).m_sinrAvg = GetTBInfo(tbIt).m_sinrAvg / GetTBInfo(tbIt).m_expected.m_rbBitmap.size ();

      NS_LOG_INFO ("Finishing RX, sinrAvg=" << GetTBInfo(tbIt).m_sinrAvg <<
                   " sinrMin=" << GetTBInfo(tbIt).m_sinrMin <<
                   " SinrAvg (dB) " << 10 * log (GetTBInfo(tbIt).m_sinrAvg) / log (10));

      if ((!m_dataErrorModelEnabled) || (m_rxPacketBurstList.empty ()))
        {
          continue;
        }

      std::function < const NrErrorModel::NrErrorModelHistory & (uint16_t, uint8_t) > RetrieveHistory;

      if (GetTBInfo (tbIt).m_expected.m_isDownlink)
        {
          RetrieveHistory = std::bind (&MmWaveHarqPhy::GetHarqProcessInfoDl, m_harqPhyModule,
                                       std::placeholders::_1, std::placeholders::_2);
        }
      else
        {
          RetrieveHistory = std::bind (&MmWaveHarqPhy::GetHarqProcessInfoUl, m_harqPhyModule,
                                       std::placeholders::_1, std::placeholders::_2);
        }

      const NrErrorModel::NrErrorModelHistory & harqInfoList = RetrieveHistory (GetRnti (tbIt),
                                                                                GetTBInfo (tbIt).m_expected.m_harqProcessId);

      NS_ABORT_MSG_IF (!m_errorModelType.IsChildOf(NrErrorModel::GetTypeId()),
                       "The error model must be a child of NrErrorModel");

      ObjectFactory emFactory;
      emFactory.SetTypeId (m_errorModelType);
      Ptr<NrErrorModel> em = DynamicCast<NrErrorModel> (emFactory.Create ());
      NS_ABORT_IF (em == nullptr);

      // Output is the output of the error model. From the TBLER we decide
      // if the entire TB is corrupted or not

      GetTBInfo(tbIt).m_outputOfEM = em->GetTbDecodificationStats (m_sinrPerceived,
                                                                   GetTBInfo(tbIt).m_expected.m_rbBitmap,
                                                                   GetTBInfo(tbIt).m_expected.m_tbSize,
                                                                   GetTBInfo(tbIt).m_expected.m_mcs,
                                                                   harqInfoList);
      GetTBInfo (tbIt).m_isCorrupted = m_random->GetValue () > GetTBInfo(tbIt).m_outputOfEM->m_tbler ? false : true;

      if (GetTBInfo (tbIt).m_isCorrupted)
        {
          NS_LOG_INFO (" RNTI " << GetRnti (tbIt) << " size " <<
                       GetTBInfo (tbIt).m_expected.m_tbSize << " mcs " <<
                       (uint32_t)GetTBInfo (tbIt).m_expected.m_mcs << " bitmap " <<
                       GetTBInfo (tbIt).m_expected.m_rbBitmap.size () << " rv from MAC: " <<
                       GetTBInfo (tbIt).m_expected.m_rv << " elements in the history: " <<
                       harqInfoList.size () << " TBLER " <<
                       GetTBInfo(tbIt).m_outputOfEM->m_tbler << " corrupted " <<
                       GetTBInfo (tbIt).m_isCorrupted);
        }
    }

  std::map <uint16_t, DlHarqInfo> harqDlInfoMap;
  for (auto packetBurst : m_rxPacketBurstList)
    {
      for (auto packet : packetBurst->GetPackets ())
        {
          if (packet->GetSize () == 0)
            {
              continue;
            }

          LteRadioBearerTag bearerTag;
          if (packet->PeekPacketTag (bearerTag) == false)
            {
              NS_FATAL_ERROR ("No radio bearer tag found");
            }

          uint16_t rnti = bearerTag.GetRnti ();

          auto itTb = m_transportBlocks.find (rnti);

          if (itTb == m_transportBlocks.end ())
            {
              // Packet for other device...
              continue;
            }

          if (! GetTBInfo (*itTb).m_isCorrupted)
            {
              m_phyRxDataEndOkCallback (packet);
            }
          else
            {
              NS_LOG_INFO ("TB failed");
            }

          MmWaveMacPduTag pduTag;
          if (packet->PeekPacketTag (pduTag) == false)
            {
              NS_FATAL_ERROR ("No radio bearer tag found");
            }

          RxPacketTraceParams traceParams;
          traceParams.m_tbSize = GetTBInfo(*itTb).m_expected.m_tbSize;
          traceParams.m_frameNum = pduTag.GetSfn ().m_frameNum;
          traceParams.m_subframeNum = pduTag.GetSfn ().m_subframeNum;
          traceParams.m_slotNum = pduTag.GetSfn ().m_slotNum;
          traceParams.m_varTtiNum = pduTag.GetSfn ().m_varTtiNum;
          traceParams.m_rnti = rnti;
          traceParams.m_mcs = GetTBInfo(*itTb).m_expected.m_mcs;
          traceParams.m_rv = GetTBInfo(*itTb).m_expected.m_rv;
          traceParams.m_sinr = GetTBInfo(*itTb).m_sinrAvg;
          traceParams.m_sinrMin = GetTBInfo(*itTb).m_sinrMin;
          traceParams.m_tbler = GetTBInfo(*itTb).m_outputOfEM->m_tbler;
          traceParams.m_corrupt = GetTBInfo(*itTb).m_isCorrupted;
          traceParams.m_symStart = GetTBInfo(*itTb).m_expected.m_symStart;
          traceParams.m_numSym = GetTBInfo(*itTb).m_expected.m_numSym;
          traceParams.m_ccId = m_componentCarrierId;
          traceParams.m_rbAssignedNum = static_cast<uint32_t> (GetTBInfo(*itTb).m_expected.m_rbBitmap.size ());

          if (enbRx)
            {
              traceParams.m_cellId = enbRx->GetCellId ();
              m_rxPacketTraceEnb (traceParams);
            }
          else if (ueRx)
            {
              traceParams.m_cellId = ueRx->GetTargetEnb ()->GetCellId ();
              m_rxPacketTraceUe (traceParams);
            }


          // send HARQ feedback (if not already done for this TB)
          if (! GetTBInfo(*itTb).m_harqFeedbackSent)
            {
              GetTBInfo(*itTb).m_harqFeedbackSent = true;
              if (! GetTBInfo(*itTb).m_expected.m_isDownlink)    // UPLINK TB
                {
                  // Generate the feedback
                  UlHarqInfo harqUlInfo;
                  harqUlInfo.m_rnti = rnti;
                  harqUlInfo.m_tpc = 0;
                  harqUlInfo.m_harqProcessId = GetTBInfo(*itTb).m_expected.m_harqProcessId;
                  harqUlInfo.m_numRetx = GetTBInfo(*itTb).m_expected.m_rv;
                  if (GetTBInfo(*itTb).m_isCorrupted)
                    {
                      harqUlInfo.m_receptionStatus = UlHarqInfo::NotOk;
                    }
                  else
                    {
                      harqUlInfo.m_receptionStatus = UlHarqInfo::Ok;
                    }

                  // Send the feedback
                  if (!m_phyUlHarqFeedbackCallback.IsNull ())
                    {
                      m_phyUlHarqFeedbackCallback (harqUlInfo);
                    }

                  // Arrange the history
                  if (! GetTBInfo(*itTb).m_isCorrupted || GetTBInfo(*itTb).m_expected.m_rv == 3)
                    {
                      m_harqPhyModule->ResetUlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId);
                    }
                  else
                    {
                      m_harqPhyModule->UpdateUlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId,
                                                                  GetTBInfo(*itTb).m_outputOfEM);
                    }
                }
              else
                {
                  // Generate the feedback
                  DlHarqInfo harqDlInfo;
                  harqDlInfo.m_rnti = rnti;
                  harqDlInfo.m_harqProcessId = GetTBInfo(*itTb).m_expected.m_harqProcessId;
                  harqDlInfo.m_numRetx = GetTBInfo(*itTb).m_expected.m_rv;
                  harqDlInfo.m_bwpIndex = m_componentCarrierId;
                  if (GetTBInfo(*itTb).m_isCorrupted)
                    {
                      harqDlInfo.m_harqStatus = DlHarqInfo::NACK;
                    }
                  else
                    {
                      harqDlInfo.m_harqStatus = DlHarqInfo::ACK;
                    }

                  NS_ASSERT (harqDlInfoMap.find(rnti) == harqDlInfoMap.end());
                  harqDlInfoMap.insert(std::make_pair (rnti, harqDlInfo));

                  // Send the feedback
                  if (!m_phyDlHarqFeedbackCallback.IsNull ())
                    {
                      m_phyDlHarqFeedbackCallback (harqDlInfo);
                    }

                  // Arrange the history
                  if (! GetTBInfo(*itTb).m_isCorrupted || GetTBInfo(*itTb).m_expected.m_rv == 3)
                    {
                      m_harqPhyModule->ResetDlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId);
                    }
                  else
                    {
                      m_harqPhyModule->UpdateDlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId,
                                                                  GetTBInfo(*itTb).m_outputOfEM);
                    }
                }   // end if (itTb->second.downlink) HARQ
            }   // end if (!itTb->second.harqFeedbackSent)
        }
    }

  // forward control messages of this frame to MmWavePhy

  if (!m_rxControlMessageList.empty () && m_phyRxCtrlEndOkCallback)
    {
      m_phyRxCtrlEndOkCallback (m_rxControlMessageList, m_componentCarrierId);
    }

  // if in unlicensed mode check after reception if the state should be 
  // changed to IDLE or CCA_BUSY
  if (m_unlicensedMode)
    {
      MaybeCcaBusy ();
    }
  else
    {
      ChangeState (IDLE, Seconds (0));
    }

  m_rxPacketBurstList.clear ();
  m_transportBlocks.clear ();
  m_rxControlMessageList.clear ();
}

void
MmWaveSpectrumPhy::CheckIfStillBusy ()
{
  NS_ABORT_MSG_IF ( m_state == IDLE, "This function should not be called when in IDLE state." );

  // If in state of RX/TX do not switch to CCA_BUSY until RX/TX is finished. 
  // When RX/TX finishes, check if the channel is still busy.
  
  if (m_state == CCA_BUSY)
    {
      MaybeCcaBusy();
    }
  else // RX_DL_CTRL, RX_UL_CTRL, RX_DATA, TX
    {
      Time delayUntilCcaEnd = m_interferenceData->GetEnergyDuration (m_ccaMode1ThresholdW);

      if (delayUntilCcaEnd.IsZero())
        {
          NS_LOG_INFO (" Channel found IDLE as expected.");
        }
      else
        {
          NS_LOG_INFO (" Wait while channel BUSY for: "<<delayUntilCcaEnd<<" ns.");
        }
    }
}

void
MmWaveSpectrumPhy::MaybeCcaBusy ()
{
  Time delayUntilCcaEnd = m_interferenceData->GetEnergyDuration (m_ccaMode1ThresholdW);
  if (!delayUntilCcaEnd.IsZero ())
    {
      NS_LOG_DEBUG ("Channel detected BUSY for:" << delayUntilCcaEnd << " ns.");

      ChangeState (CCA_BUSY, delayUntilCcaEnd);

      // check if with the new energy the channel will be for longer time in CCA_BUSY
      if ( m_busyTimeEnds < Simulator::Now() + delayUntilCcaEnd)
        {
          m_busyTimeEnds = Simulator::Now () + delayUntilCcaEnd;

          if (m_checkIfIsIdleEvent.IsRunning())
            {
              m_checkIfIsIdleEvent.Cancel();
            }

          NS_LOG_DEBUG ("Check if still BUSY in:" << delayUntilCcaEnd << " us, and that is at "
              " time:"<<Simulator::Now() + delayUntilCcaEnd<<" and current time is:"<<Simulator::Now());

          m_checkIfIsIdleEvent = Simulator::Schedule (delayUntilCcaEnd, &MmWaveSpectrumPhy::CheckIfStillBusy, this);
        }
    }
  else
    {
      NS_ABORT_MSG_IF (m_checkIfIsIdleEvent.IsRunning(), "Unexpected state: returning to IDLE while there is an event "
                       "running that should switch from CCA_BUSY to IDLE ?!");
      NS_LOG_DEBUG ("Channel detected IDLE after being in: " << ToString (m_state) << " state.");
      ChangeState (IDLE, Seconds (0));
    }
}

void
MmWaveSpectrumPhy::EndRxCtrl ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_state == RX_DL_CTRL || m_state == RX_UL_CTRL);

  // control error model not supported
  // forward control messages of this frame to LtePhy
  if (!m_rxControlMessageList.empty ())
    {
      if (m_phyRxCtrlEndOkCallback)
        {
          m_phyRxCtrlEndOkCallback (m_rxControlMessageList, m_componentCarrierId);
        }
    }

  // if in unlicensed mode check after reception if we are in IDLE or CCA_BUSY mode
  if (m_unlicensedMode)
    {
      MaybeCcaBusy ();
    }
  else
    {
      ChangeState (IDLE, Seconds (0));
    }

  m_rxControlMessageList.clear ();
}

bool
MmWaveSpectrumPhy::StartTxDataFrames (Ptr<PacketBurst> pb, std::list<Ptr<MmWaveControlMessage> > ctrlMsgList,
                                      Time duration, uint8_t slotInd)
{
  NS_LOG_FUNCTION (this);
  switch (m_state)
    {
    case RX_DATA:
      /* no break */
    case RX_DL_CTRL:
      /* no break */
    case RX_UL_CTRL:
      NS_FATAL_ERROR ("Cannot TX while RX.");
      break;
    case TX:
      NS_FATAL_ERROR ("Cannot TX while already TX.");
      break;
    case CCA_BUSY:
      NS_LOG_WARN ("Start transmitting DATA while in CCA_BUSY state.");
      /* no break */
    case IDLE:
      {
        NS_ASSERT (m_txPsd);

        ChangeState (TX, duration);

        Ptr<MmwaveSpectrumSignalParametersDataFrame> txParams = Create<MmwaveSpectrumSignalParametersDataFrame> ();
        txParams->duration = duration;
        txParams->txPhy = this->GetObject<SpectrumPhy> ();
        txParams->psd = m_txPsd;
        txParams->packetBurst = pb;
        txParams->cellId = m_cellId;
        txParams->ctrlMsgList = ctrlMsgList;
        txParams->slotInd = slotInd;

        /* This section is used for trace */
        if (IsEnb ())
          {
            EnbPhyPacketCountParameter traceParam;
            traceParam.m_noBytes = (txParams->packetBurst) ? txParams->packetBurst->GetSize () : 0;
            traceParam.m_cellId = txParams->cellId;
            traceParam.m_isTx = true;
            traceParam.m_subframeno = 0;   // TODO extend this

            m_txPacketTraceEnb (traceParam);
          }

        m_txDataTrace (duration);

        if (m_channel)
          {
            m_channel->StartTx (txParams);
          }
        else
          {
            NS_LOG_WARN ("Working without channel (i.e., under test)");
          }


        Simulator::Schedule (duration, &MmWaveSpectrumPhy::EndTx, this);
      }
      break;
    default:
      NS_LOG_FUNCTION (this << "Programming Error. Code should not reach this point");
    }
  return true;
}

bool
MmWaveSpectrumPhy::StartTxDlControlFrames (const std::list<Ptr<MmWaveControlMessage> > &ctrlMsgList,
                                           const Time &duration)
{
  NS_LOG_LOGIC (this << " state: " << ToString (m_state));

  switch (m_state)
    {
    case RX_DATA:
      /* no break */
    case RX_DL_CTRL:
      /* no break */
    case RX_UL_CTRL:
      NS_FATAL_ERROR ("Cannot TX while RX.");
      break;
    case TX:
      NS_FATAL_ERROR ("Cannot TX while already TX.");
      break;
    case CCA_BUSY:
      NS_LOG_WARN ("Start transmitting DL CTRL while in CCA_BUSY state.");
      /* no break */
    case IDLE:
      {
        NS_ASSERT (m_txPsd);
        ChangeState (TX, duration);
        Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame> txParams = Create<MmWaveSpectrumSignalParametersDlCtrlFrame> ();
        txParams->duration = duration;
        txParams->txPhy = GetObject<SpectrumPhy> ();
        txParams->psd = m_txPsd;
        txParams->cellId = m_cellId;
        txParams->pss = true;
        txParams->ctrlMsgList = ctrlMsgList;

        m_txCtrlTrace (duration);
        if (m_channel)
          {
            m_channel->StartTx (txParams);
          }
        else
          {
            NS_LOG_WARN ("Working without channel (i.e., under test)");
          }

        Simulator::Schedule (duration, &MmWaveSpectrumPhy::EndTx, this);
      }
    }
  return false;
}


bool
MmWaveSpectrumPhy::StartTxUlControlFrames (const std::list<Ptr<MmWaveControlMessage> > &ctrlMsgList,
                                           const Time &duration)
{
  NS_LOG_LOGIC (this << " state: " << ToString (m_state));

  switch (m_state)
    {
    case RX_DATA:
      /* no break */
    case RX_DL_CTRL:
      /* no break */ 
    case RX_UL_CTRL:
      NS_FATAL_ERROR ("Cannot TX while RX.");
      break;
    case TX:
      NS_FATAL_ERROR ("Cannot TX while already TX.");
      break;
    case CCA_BUSY:
      NS_LOG_WARN ("Start transmitting UL CTRL while in CCA_BUSY state");
      /* no break */
    case IDLE:
      {
        NS_ASSERT (m_txPsd);
        ChangeState (TX, duration);
        Ptr<MmWaveSpectrumSignalParametersUlCtrlFrame> txParams = Create<MmWaveSpectrumSignalParametersUlCtrlFrame> ();
        txParams->duration = duration;
        txParams->txPhy = GetObject<SpectrumPhy> ();
        txParams->psd = m_txPsd;
        txParams->cellId = m_cellId;
        txParams->ctrlMsgList = ctrlMsgList;

        m_txCtrlTrace (duration);
        if (m_channel)
          {
            m_channel->StartTx (txParams);
          }
        else
          {
            NS_LOG_WARN ("Working without channel (i.e., under test)");
          }
        Simulator::Schedule (duration, &MmWaveSpectrumPhy::EndTx, this);
      }
    }
  return false;
}

void
MmWaveSpectrumPhy::EndTx ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_state == TX);

  // if in unlicensed mode check after transmission if we are in IDLE or CCA_BUSY mode
  if (m_unlicensedMode)
    {
      MaybeCcaBusy ();
    }
  else
    {
      ChangeState (IDLE, Seconds (0));
    }
}

Ptr<SpectrumChannel>
MmWaveSpectrumPhy::GetSpectrumChannel ()
{
  return m_channel;
}

void
MmWaveSpectrumPhy::SetCellId (uint16_t cellId)
{
  m_cellId = cellId;
}

void
MmWaveSpectrumPhy::SetComponentCarrierId (uint8_t componentCarrierId)
{
  m_componentCarrierId = componentCarrierId;
}

void
MmWaveSpectrumPhy::AddDataPowerChunkProcessor (Ptr<mmWaveChunkProcessor> p)
{
  m_interferenceData->AddPowerChunkProcessor (p);
}

void
MmWaveSpectrumPhy::AddDataSinrChunkProcessor (Ptr<mmWaveChunkProcessor> p)
{
  m_interferenceData->AddSinrChunkProcessor (p);
}

void
MmWaveSpectrumPhy::UpdateSinrPerceived (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr);
  NS_LOG_INFO ("Update SINR perceived with this value: " << sinr);
  m_sinrPerceived = sinr;
}

void
MmWaveSpectrumPhy::SetHarqPhyModule (Ptr<MmWaveHarqPhy> harq)
{
  m_harqPhyModule = harq;
}

Ptr<MmWaveHarqPhy>
MmWaveSpectrumPhy::GetHarqPhyModule() const
{
  return m_harqPhyModule;
}

Ptr<mmWaveInterference>
MmWaveSpectrumPhy::GetMmWaveInterference (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interferenceData;
}


}
