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

#include "mmwave-spectrum-phy.h"

#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <ns3/boolean.h>
#include <cmath>
#include <ns3/trace-source-accessor.h>
#include <ns3/antenna-model.h>
#include "mmwave-phy-mac-common.h"
#include <ns3/mmwave-enb-net-device.h>
#include <ns3/mmwave-ue-net-device.h>
#include <ns3/mmwave-ue-phy.h>
#include "mmwave-radio-bearer-tag.h"
#include <stdio.h>
#include <ns3/double.h>
#include "mmwave-mac-pdu-tag.h"
#include "nr-lte-mi-error-model.h"
#include <functional>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveSpectrumPhy");
NS_OBJECT_ENSURE_REGISTERED (MmWaveSpectrumPhy);

MmWaveSpectrumPhy::MmWaveSpectrumPhy ()
  : SpectrumPhy (),
    m_cellId (0),
  m_state (IDLE)
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

  Ptr<MmWaveEnbNetDevice> enbNetDev =
    DynamicCast<MmWaveEnbNetDevice> (GetDevice ());

  if (enbNetDev != 0)
    {
      m_isEnb = true;
    }
  else
    {
      m_isEnb = false;
    }
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
  return m_antenna;
}

void
MmWaveSpectrumPhy::SetAntenna (Ptr<AntennaModel> a)
{
  NS_ABORT_IF (m_antenna != nullptr);
  m_antenna = a;
}

void
MmWaveSpectrumPhy::SetState (State newState)
{
  ChangeState (newState);
}

void
MmWaveSpectrumPhy::ChangeState (State newState)
{
  NS_LOG_LOGIC (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
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
MmWaveSpectrumPhy::SetPhyRxCtrlEndOkCallback (MmWavePhyRxCtrlEndOkCallback c)
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

  Ptr<MmWaveEnbNetDevice> EnbTx =
    DynamicCast<MmWaveEnbNetDevice> (params->txPhy->GetDevice ());
  Ptr<MmWaveEnbNetDevice> enbRx =
    DynamicCast<MmWaveEnbNetDevice> (GetDevice ());
  if ((EnbTx != 0 && enbRx != 0) || (EnbTx == 0 && enbRx == 0))
    {
      NS_LOG_INFO ("BS to BS or UE to UE transmission neglected.");
      return;
    }

  Ptr<MmwaveSpectrumSignalParametersDataFrame> mmwaveDataRxParams =
    DynamicCast<MmwaveSpectrumSignalParametersDataFrame> (params);

  Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame> DlCtrlRxParams =
    DynamicCast<MmWaveSpectrumSignalParametersDlCtrlFrame> (params);

  if (mmwaveDataRxParams != 0)
    {
      Ptr<MmWaveUeNetDevice> ueRx = 0;
      ueRx = DynamicCast<MmWaveUeNetDevice> (GetDevice ());

      /****************** the following code does not work with 2 bandwidth parts ************************/
      // seems to be some issue with concurrent execution of independent events ...
      // TODO @CTTC: needs to be revisited
      /*
      bool isAllocated = true;

      if ((ueRx!=0) && (ueRx->GetPhy ()->IsReceptionEnabled () == false))
        {
          isAllocated = false;
        }

       if (isAllocated)
        {*/
      m_interferenceData->AddSignal (mmwaveDataRxParams->psd, mmwaveDataRxParams->duration);
      NS_LOG_INFO ("Start Rxing a signal: " << mmwaveDataRxParams->psd <<
                   " duration= " << mmwaveDataRxParams->duration << " cellId " <<
                   mmwaveDataRxParams->cellId << " this cellId: " << m_cellId);
      if (mmwaveDataRxParams->cellId == m_cellId)
        {
          //m_interferenceData->AddSignal (mmwaveDataRxParams->psd, mmwaveDataRxParams->duration);
          StartRxData (mmwaveDataRxParams);
        }

      /*  TODO @CTTC:
       *  double check why this code is not used, not clear how the interference calculated.
       else
       {
         if (ueRx != 0)
           {
             m_interferenceData->AddSignal (mmwaveDataRxParams->psd, mmwaveDataRxParams->duration);
           }
       }
       */
      //}

    }
  else
    {
      Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame> DlCtrlRxParams =
        DynamicCast<MmWaveSpectrumSignalParametersDlCtrlFrame> (params);
      if (DlCtrlRxParams != 0)
        {
          if (DlCtrlRxParams->cellId == m_cellId)
            {
              StartRxCtrl (params);
            }
          else
            {
              // Do nothing
            }
        }
    }
}

void
MmWaveSpectrumPhy::StartRxData (Ptr<MmwaveSpectrumSignalParametersDataFrame> params)
{
  NS_LOG_FUNCTION (this);

  m_interferenceData->StartRx (params->psd);

  Ptr<MmWaveEnbNetDevice> enbRx =
    DynamicCast<MmWaveEnbNetDevice> (GetDevice ());
  Ptr<MmWaveUeNetDevice> ueRx =
    DynamicCast<MmWaveUeNetDevice> (GetDevice ());
  switch (m_state)
    {
    case TX:
      NS_FATAL_ERROR ("Cannot receive while transmitting");
      break;
    case RX_CTRL:
      NS_FATAL_ERROR ("Cannot receive control in data period");
      break;
    case RX_DATA:
    case IDLE:
      {
        if (params->cellId == m_cellId)
          {
            if (m_rxPacketBurstList.empty ())
              {
                NS_ASSERT (m_state == IDLE);
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

            ChangeState (RX_DATA);
            if (params->packetBurst && !params->packetBurst->GetPackets ().empty ())
              {
                m_rxPacketBurstList.push_back (params->packetBurst);
              }
            //NS_LOG_DEBUG (this << " insert msgs " << params->ctrlMsgList.size ());
            m_rxControlMessageList.insert (m_rxControlMessageList.end (), params->ctrlMsgList.begin (), params->ctrlMsgList.end ());

            NS_LOG_LOGIC (this << " numSimultaneousRxEvents = " << m_rxPacketBurstList.size ());
          }
        else
          {
            NS_LOG_LOGIC (this << " not in sync with this signal (cellId="
                               << params->cellId  << ", m_cellId=" << m_cellId << ")");
          }
      }
      break;
    default:
      NS_FATAL_ERROR ("Programming Error: Unknown State");
    }
}

void
MmWaveSpectrumPhy::StartRxCtrl (Ptr<SpectrumSignalParameters> params)
{
  NS_LOG_FUNCTION (this);
  // RDF: method currently supports Downlink control only!
  switch (m_state)
    {
    case TX:
      NS_FATAL_ERROR ("Cannot RX while TX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
      break;
    case RX_DATA:
      NS_FATAL_ERROR ("Cannot RX data while receiving control");
      break;
    case RX_CTRL:
    case IDLE:
      {
        // the behavior is similar when we're IDLE or RX because we can receive more signals
        // simultaneously (e.g., at the eNB).
        Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame> dlCtrlRxParams = \
          DynamicCast<MmWaveSpectrumSignalParametersDlCtrlFrame> (params);
        // To check if we're synchronized to this signal, we check for the CellId
        uint16_t cellId = 0;
        if (dlCtrlRxParams != 0)
          {
            cellId = dlCtrlRxParams->cellId;
          }
        else
          {
            NS_LOG_ERROR ("SpectrumSignalParameters type not supported");
          }
        // check presence of PSS for UE measuerements
        /*if (dlCtrlRxParams->pss == true)
                      {
                              SpectrumValue pssPsd = *params->psd;
                              if (!m_phyRxPssCallback.IsNull ())
                              {
                                      m_phyRxPssCallback (cellId, params->psd);
                              }
                      }*/
        if (cellId  == m_cellId)
          {
            if (m_state == RX_CTRL)
              {
                Ptr<MmWaveUeNetDevice> ueRx =
                  DynamicCast<MmWaveUeNetDevice> (GetDevice ());
                if (ueRx)
                  {
                    NS_FATAL_ERROR ("UE already receiving control data from serving cell");
                  }
                NS_ASSERT ((m_firstRxStart == Simulator::Now ())
                           && (m_firstRxDuration == params->duration));
              }
            NS_LOG_LOGIC (this << " synchronized with this signal (cellId=" << cellId << ")");
            if (m_state == IDLE)
              {
                // first transmission, i.e., we're IDLE and we start RX
                NS_ASSERT (m_rxControlMessageList.empty ());
                m_firstRxStart = Simulator::Now ();
                m_firstRxDuration = params->duration;
                NS_LOG_LOGIC (this << " scheduling EndRx with delay " << params->duration);
                // store the DCIs
                m_rxControlMessageList = dlCtrlRxParams->ctrlMsgList;
                Simulator::Schedule (params->duration, &MmWaveSpectrumPhy::EndRxCtrl, this);
                ChangeState (RX_CTRL);
              }
            else
              {
                m_rxControlMessageList.insert (m_rxControlMessageList.end (), dlCtrlRxParams->ctrlMsgList.begin (), dlCtrlRxParams->ctrlMsgList.end ());
              }

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

  double sinrAvg = Sum (m_sinrPerceived) / (m_sinrPerceived.GetSpectrumModel ()->GetNumBands ());
  double sinrMin = 99999999999;
  for (Values::const_iterator it = m_sinrPerceived.ConstValuesBegin (); it != m_sinrPerceived.ConstValuesEnd (); it++)
    {
      if (*it < sinrMin)
        {
          sinrMin = *it;
        }
    }

  NS_LOG_INFO ("Finishing RX, sinrAvg=" << sinrAvg << " sinrMin=" << sinrMin);

  Ptr<MmWaveEnbNetDevice> enbRx = DynamicCast<MmWaveEnbNetDevice> (GetDevice ());
  Ptr<MmWaveUeNetDevice> ueRx = DynamicCast<MmWaveUeNetDevice> (GetDevice ());

  NS_ASSERT (m_state = RX_DATA);

  GetSecond GetTBInfo;
  GetFirst GetRnti;

  for (auto &tbIt : m_transportBlocks)
    {
      if ((!m_dataErrorModelEnabled) || (m_rxPacketBurstList.empty ()))
        {
          continue;
        }

      uint32_t rv = 0;

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

      if (harqInfoList.size () > 0)
        {
          rv = static_cast<uint32_t> (harqInfoList.size ());
        }
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
                       GetTBInfo (tbIt).m_expected.m_rbBitmap.size () << " rv " << rv <<
                       " TBLER " << GetTBInfo(tbIt).m_outputOfEM->m_tbler << " corrupted " <<
                       GetTBInfo (tbIt).m_isCorrupted);
        }

    }

  std::map <uint16_t, DlHarqInfo> harqDlInfoMap;

  for (auto packetBurst : m_rxPacketBurstList)
    {
      uint16_t rnti = 0;
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

          if (rnti != 0)
            {
              NS_ASSERT (bearerTag.GetRnti () == rnti);
            }

          rnti = bearerTag.GetRnti ();

          auto itTb = m_transportBlocks.find (rnti);

          if (itTb == m_transportBlocks.end ())
            {
              // Packet for other device... I really don't understand why?!
              // TODO this must be removed.
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
          traceParams.m_sinr = sinrAvg;
          traceParams.m_sinrMin = sinrMin;
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
                  UlHarqInfo harqUlInfo;
                  harqUlInfo.m_rnti = rnti;
                  harqUlInfo.m_tpc = 0;
                  harqUlInfo.m_harqProcessId = GetTBInfo(*itTb).m_expected.m_harqProcessId;
                  harqUlInfo.m_numRetx = GetTBInfo(*itTb).m_expected.m_rv;
                  if (GetTBInfo(*itTb).m_isCorrupted)
                    {
                      harqUlInfo.m_receptionStatus = UlHarqInfo::NotOk;
                      m_harqPhyModule->UpdateUlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId,
                                                                  GetTBInfo(*itTb).m_outputOfEM);
                    }
                  else
                    {
                      harqUlInfo.m_receptionStatus = UlHarqInfo::Ok;
                      m_harqPhyModule->ResetUlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId);
                    }
                  if (!m_phyUlHarqFeedbackCallback.IsNull ())
                    {
                      m_phyUlHarqFeedbackCallback (harqUlInfo);
                    }
                }
              else
                {
                  auto itHarq = harqDlInfoMap.find (rnti);
                  if (itHarq == harqDlInfoMap.end ())
                    {
                      DlHarqInfo harqDlInfo;
                      harqDlInfo.m_harqStatus = DlHarqInfo::NACK;
                      harqDlInfo.m_rnti = rnti;
                      harqDlInfo.m_harqProcessId = GetTBInfo(*itTb).m_expected.m_harqProcessId;
                      harqDlInfo.m_numRetx = GetTBInfo(*itTb).m_expected.m_rv;
                      if (GetTBInfo(*itTb).m_isCorrupted)
                        {
                          harqDlInfo.m_harqStatus = DlHarqInfo::NACK;
                          m_harqPhyModule->UpdateDlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId,
                                                                      GetTBInfo(*itTb).m_outputOfEM);
                        }
                      else
                        {
                          harqDlInfo.m_harqStatus = DlHarqInfo::ACK;
                          m_harqPhyModule->ResetDlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId);
                        }
                      harqDlInfoMap.insert (std::pair <uint16_t, DlHarqInfo> (rnti, harqDlInfo));
                    }
                  else
                    {
                      if (GetTBInfo(*itTb).m_isCorrupted)
                        {
                          (*itHarq).second.m_harqStatus = DlHarqInfo::NACK;
                          m_harqPhyModule->UpdateDlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId,
                                                                      GetTBInfo(*itTb).m_outputOfEM);
                        }
                      else
                        {
                          (*itHarq).second.m_harqStatus = DlHarqInfo::ACK;
                          m_harqPhyModule->ResetDlHarqProcessStatus (rnti, GetTBInfo(*itTb).m_expected.m_harqProcessId);
                        }
                    }
                }   // end if (itTb->second.downlink) HARQ
            }   // end if (!itTb->second.harqFeedbackSent)
        }
    }

  // send DL HARQ feedback to LtePhy
  std::map <uint16_t, DlHarqInfo>::iterator itHarq;
  for (itHarq = harqDlInfoMap.begin (); itHarq != harqDlInfoMap.end (); itHarq++)
    {
      if (!m_phyDlHarqFeedbackCallback.IsNull ())
        {
          m_phyDlHarqFeedbackCallback ((*itHarq).second);
        }
    }
  // forward control messages of this frame to MmWavePhy

  if (!m_rxControlMessageList.empty () && !m_phyRxCtrlEndOkCallback.IsNull ())
    {
      m_phyRxCtrlEndOkCallback (m_rxControlMessageList);
    }

  m_state = IDLE;
  m_rxPacketBurstList.clear ();
  m_transportBlocks.clear ();
  m_rxControlMessageList.clear ();
}

void
MmWaveSpectrumPhy::EndRxCtrl ()
{
  NS_ASSERT (m_state = RX_CTRL);

  // control error model not supported
  // forward control messages of this frame to LtePhy
  if (!m_rxControlMessageList.empty ())
    {
      if (!m_phyRxCtrlEndOkCallback.IsNull ())
        {
          m_phyRxCtrlEndOkCallback (m_rxControlMessageList);
        }
    }

  m_state = IDLE;
  m_rxControlMessageList.clear ();
}

bool
MmWaveSpectrumPhy::StartTxDataFrames (Ptr<PacketBurst> pb, std::list<Ptr<MmWaveControlMessage> > ctrlMsgList, Time duration, uint8_t slotInd)
{
  switch (m_state)
    {
    case RX_DATA:
    case RX_CTRL:
      NS_FATAL_ERROR ("cannot TX while RX: Cannot transmit while receiving");
      break;
    case TX:
      NS_FATAL_ERROR ("cannot TX while already Tx: Cannot transmit while a transmission is still on");
      break;
    case IDLE:
      {
        NS_ASSERT (m_txPsd);

        m_state = TX;
        Ptr<MmwaveSpectrumSignalParametersDataFrame> txParams = Create<MmwaveSpectrumSignalParametersDataFrame> ();
        txParams->duration = duration;
        txParams->txPhy = this->GetObject<SpectrumPhy> ();
        txParams->psd = m_txPsd;
        txParams->packetBurst = pb;
        txParams->cellId = m_cellId;
        txParams->ctrlMsgList = ctrlMsgList;
        txParams->slotInd = slotInd;
        txParams->txAntenna = m_antenna;



        //NS_LOG_DEBUG ("ctrlMsgList.size () == " << txParams->ctrlMsgList.size ());

        /* This section is used for trace */
        Ptr<MmWaveEnbNetDevice> enbTx =
          DynamicCast<MmWaveEnbNetDevice> (GetDevice ());
        Ptr<MmWaveUeNetDevice> ueTx =
          DynamicCast<MmWaveUeNetDevice> (GetDevice ());

        if (enbTx)
          {
            EnbPhyPacketCountParameter traceParam;
            traceParam.m_noBytes = (txParams->packetBurst) ? txParams->packetBurst->GetSize () : 0;
            traceParam.m_cellId = txParams->cellId;
            traceParam.m_isTx = true;
            traceParam.m_subframeno = 0;   // TODO extend this

            m_txPacketTraceEnb (traceParam);
          }

        //		if (enbTx)
        //		{
        //			EnbPhyPacketCountParameter traceParam;
        //			traceParam.m_noBytes = (txParams->packetBurst)?txParams->packetBurst->GetSize ():0;
        //			traceParam.m_cellId = txParams->cellId;
        //			traceParam.m_isTx = true;
        //			traceParam.m_subframeno = enbTx->GetPhy ()->GetAbsoluteSubframeNo ();
        //			m_reportEnbPacketCount (traceParam);
        //		}
        //		else if (ueTx)
        //		{
        //			UePhyPacketCountParameter traceParam;
        //			traceParam.m_noBytes = (txParams->packetBurst)?txParams->packetBurst->GetSize ():0;
        //			traceParam.m_imsi = ueTx->GetImsi ();
        //			traceParam.m_isTx = true;
        //			traceParam.m_subframeno = ueTx->GetPhy ()->GetAbsoluteSubframeNo ();
        //			m_reportUePacketCount (traceParam);
        //		}

        m_channel->StartTx (txParams);

        Simulator::Schedule (duration, &MmWaveSpectrumPhy::EndTx, this);
      }
      break;
    default:
      NS_LOG_FUNCTION (this << "Programming Error. Code should not reach this point");
    }
  return true;
}

bool
MmWaveSpectrumPhy::StartTxDlControlFrames (std::list<Ptr<MmWaveControlMessage> > ctrlMsgList, Time duration)
{
  NS_LOG_LOGIC (this << " state: " << m_state);

  switch (m_state)
    {
    case RX_DATA:
    case RX_CTRL:
      NS_FATAL_ERROR (Simulator::Now() << "cannot TX while RX: Cannot transmit while receiving.");
      break;
    case TX:
      NS_FATAL_ERROR ("cannot TX while already Tx: Cannot transmit while a transmission is still on");
      break;
    case IDLE:
      {
        NS_ASSERT (m_txPsd);

        m_state = TX;

        Ptr<MmWaveSpectrumSignalParametersDlCtrlFrame> txParams = Create<MmWaveSpectrumSignalParametersDlCtrlFrame> ();
        txParams->duration = duration;
        txParams->txPhy = GetObject<SpectrumPhy> ();
        txParams->psd = m_txPsd;
        txParams->cellId = m_cellId;
        txParams->pss = true;
        txParams->ctrlMsgList = ctrlMsgList;
        txParams->txAntenna = m_antenna;
        m_channel->StartTx (txParams);
        Simulator::Schedule (duration, &MmWaveSpectrumPhy::EndTx, this);
      }
    }
  return false;
}

void
MmWaveSpectrumPhy::EndTx ()
{
  NS_ASSERT (m_state == TX);

  m_state = IDLE;
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

Ptr<mmWaveInterference>
MmWaveSpectrumPhy::GetMmWaveInterference (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interferenceData;
}


}
