/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "realistic-beamforming-algorithm.h"
#include <ns3/double.h>
#include <ns3/angles.h>
#include <ns3/uinteger.h>
#include <ns3/node.h>
#include "nr-ue-phy.h"
#include "nr-gnb-phy.h"
#include "nr-gnb-net-device.h"
#include "nr-ue-net-device.h"
#include "beam-manager.h"
#include <ns3/nr-spectrum-value-helper.h>
#include "ns3/random-variable-stream.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("RealisticBeamformingAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (RealisticBeamformingAlgorithm);

RealisticBeamformingAlgorithm::RealisticBeamformingAlgorithm ()
{
  NS_UNUSED(this);
}

/**
 * \brief constructor
 */
RealisticBeamformingAlgorithm:: RealisticBeamformingAlgorithm (Ptr<NrGnbNetDevice>& gNbDevice, Ptr<NrUeNetDevice>& ueDevice, uint8_t)
{
  m_normalRandomVariable = CreateObject<NormalRandomVariable> ();
}

int64_t
RealisticBeamformingAlgorithm::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_normalRandomVariable->SetStream (stream);
  return 1;
}

RealisticBeamformingAlgorithm::~RealisticBeamformingAlgorithm()
{
}

TypeId
RealisticBeamformingAlgorithm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RealisticBeamformingAlgorithm")
                     .SetParent<BeamformingAlgorithm> ()
                     .AddConstructor<RealisticBeamformingAlgorithm> ()
                     .AddAttribute ("BeamSearchAngleStep",
                                    "Angle step when searching for the best beam",
                                    DoubleValue (30),
                                    MakeDoubleAccessor (&CellScanBeamforming::SetBeamSearchAngleStep,
                                                        &CellScanBeamforming::GetBeamSearchAngleStep),
                                    MakeDoubleChecker<double> ())
                     .AddAttribute ("TriggerEvent",
                                    "Defines a beamforming trigger event",
                                    EnumValue (RealisticBeamformingAlgorithm::SRS_COUNT),
                                    MakeEnumAccessor (&RealisticBeamformingAlgorithm::SetTriggerEvent,
                                                      &RealisticBeamformingAlgorithm::GetTriggerEvent),
                                    MakeEnumChecker (RealisticBeamformingAlgorithm::SRS_COUNT, "SrsCount",
                                                     RealisticBeamformingAlgorithm::DELAYED_UPDATE, "DelayedUpdate"))
                      .AddAttribute ("SrsCountPeriodicity",
                                     "Interval between consecutive beamforming update method executions expressed in the number of SRS SINR reports"
                                     "to wait before triggering the next beamforming update method execution.",
                                     UintegerValue (1),
                                     MakeUintegerAccessor (&RealisticBeamformingAlgorithm::SetSrsCountPeriodicity,
                                                           &RealisticBeamformingAlgorithm::GetSrsCountPeriodicity),
                                     MakeUintegerChecker <uint16_t>())
                      .AddAttribute ("SrsToBeamformingDelay",
                                     "Delay between SRS SINR report and the beamforming vectors update. ",
                                     TimeValue (MilliSeconds (10)),
                                     MakeTimeAccessor (&RealisticBeamformingAlgorithm::SetSrsToBeamformingDelay,
                                                       &RealisticBeamformingAlgorithm::GetSrsToBeamformingDelay),
                                     MakeTimeChecker());

  return tid;
}

void
RealisticBeamformingAlgorithm::SetBeamSearchAngleStep (double beamSearchAngleStep)
{
  m_beamSearchAngleStep = beamSearchAngleStep;
}

double
RealisticBeamformingAlgorithm::GetBeamSearchAngleStep () const
{
  return m_beamSearchAngleStep;
}

void
RealisticBeamformingAlgorithm::SetTriggerEvent (RealisticBeamformingAlgorithm::TriggerEvent triggerEvent)
{
  m_triggerEvent = triggerEvent;
}

RealisticBeamformingAlgorithm::TriggerEvent
RealisticBeamformingAlgorithm::GetTriggerEvent () const
{
  return m_triggerEvent;
}

void
RealisticBeamformingAlgorithm::SetSrsCountPeriodicity (uint16_t periodicity)
{
  m_srsSinrPeriodicity = periodicity;
}

uint16_t
RealisticBeamformingAlgorithm::GetSrsCountPeriodicity () const
{
  return m_srsSinrPeriodicity;
}

void
RealisticBeamformingAlgorithm::SetSrsToBeamformingDelay (Time delay)
{
  m_srsToBeamformingDelay = delay;
}

Time
RealisticBeamformingAlgorithm::GetSrsToBeamformingDelay () const
{
  return m_srsToBeamformingDelay;
}

void
RealisticBeamformingAlgorithm::GetBeamformingVectors(const Ptr<const NrGnbNetDevice>& gnbDev,
                                                 const Ptr<const NrUeNetDevice>& ueDev,
                                                 BeamformingVector* gnbBfv,
                                                 BeamformingVector* ueBfv,
                                                 uint16_t ccId) const
{
  DoGetBeamformingVectors (gnbDev, ueDev, gnbBfv, ueBfv, ccId);
}


void
RealisticBeamformingAlgorithm::SaveSrsSinrReport (uint16_t cellId, uint16_t rnti, double srsSinr)
{
  NS_LOG_FUNCTION (this);

  //before anything check if RNTI corresponds to RNTI of UE of this algorithm instance

  if (m_ueDevice->GetRrc ()->GetRnti () != rnti)
    {
      NS_LOG_INFO ("Ignoring SRS report. Not for me. Report for RNTI:"<< rnti << ", and my RNTI is:"<< m_ueDevice->GetRrc ()->GetRnti ());
      return;
    }

  m_lastTimeUpdated = Simulator::Now ();
  m_maxSrsSinrPerSlot = srsSinr;
  m_srsSymbolsPerSlotCounter++;

  NS_ABORT_MSG_IF ( m_srsSymbolsPerSlot == 0, "SRS symbols per slot not set! Aborting.");

  if (m_srsSymbolsPerSlotCounter == m_srsSymbolsPerSlot)
    {
      // reset symbols per slot counter
      m_srsSymbolsPerSlotCounter = 0;
      m_maxSrsSinrPerSlot = 0;

      if (m_triggerEvent == SRS_COUNT)
        {
          // increase or reset SRS periodicity counter

          if (m_srsPeriodicityCounter == m_srsSinrPeriodicity )
            {
              m_srsPeriodicityCounter = 0;
              // it is time to trigger helpers callback
              m_helperCallback (m_gNbDevice, m_ueDevice, m_ccId);
            }
          else
            {
              m_srsPeriodicityCounter ++;
            }

        }
      else if (m_triggerEvent ==  DELAYED_UPDATE)
        {
          // schedule delayed update
          Simulator::Schedule (GetSrsToBeamformingDelay (),
                               &m_helperCallback, this,
                               m_gNbDevice, m_ueDevice, m_ccId);
        }
      else
        {
          NS_FATAL_ERROR ("Unknown trigger event type.");
        }
    }
  else
    {
      // reset symbols per slot counter
      m_srsSymbolsPerSlotCounter ++;
      m_maxSrsSinrPerSlot = std::max (srsSinr, m_maxSrsSinrPerSlot);
    }
}

void
RealisticBeamformingAlgorithm::SetTriggerCallback (RealisticBfHelperCallback callback)
{
  m_helperCallback = callback;
}

void
RealisticBeamformingAlgorithm::DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                              const Ptr<const NrUeNetDevice>& ueDev,
                                              BeamformingVector* gnbBfv,
                                              BeamformingVector* ueBfv,
                                              uint16_t ccId) const
{
  NS_ABORT_MSG_IF (gnbDev == nullptr || ueDev == nullptr, "Something went wrong, gnb or UE device does not exist.");
  double distance = gnbDev->GetNode ()->GetObject<MobilityModel> ()->GetDistanceFrom (ueDev->GetNode ()->GetObject<MobilityModel> ());
  NS_ABORT_MSG_IF (distance == 0, "Beamforming method cannot be performed between two devices that are placed in the same position.");

  // TODO check if this is correct: assuming the ccId of gNB PHY and corresponding UE PHY are the equal
  Ptr<const NrGnbPhy> gnbPhy = gnbDev->GetPhy (ccId);
  Ptr<const NrUePhy> uePhy = ueDev->GetPhy (ccId);

  Ptr<const NrSpectrumPhy> gnbSpectrumPhy = gnbPhy->GetSpectrumPhy ();
  Ptr<const NrSpectrumPhy> ueSpectrumPhy = uePhy->GetSpectrumPhy ();

  Ptr<SpectrumChannel> gnbSpectrumChannel = gnbSpectrumPhy->GetSpectrumChannel (); // SpectrumChannel should be const.. but need to change ns-3-dev
  Ptr<SpectrumChannel> ueSpectrumChannel = ueSpectrumPhy->GetSpectrumChannel ();

  Ptr<SpectrumPropagationLossModel> gnbThreeGppSpectrumPropModel = gnbSpectrumChannel->GetSpectrumPropagationLossModel ();
  Ptr<SpectrumPropagationLossModel> ueThreeGppSpectrumPropModel = ueSpectrumChannel->GetSpectrumPropagationLossModel ();

  NS_ASSERT_MSG (gnbThreeGppSpectrumPropModel == ueThreeGppSpectrumPropModel,
                 "Devices should be connected to the same spectrum channel");

  double max = 0, maxTxTheta = 0, maxRxTheta = 0;
  uint16_t maxTxSector = 0, maxRxSector = 0;
  complexVector_t maxTxW, maxRxW;

  UintegerValue uintValue;
  gnbPhy->GetAntennaArray ()->GetAttribute ("NumRows", uintValue);
  uint32_t gnbNumRows = static_cast<uint32_t> (uintValue.Get ());
  uePhy->GetAntennaArray ()->GetAttribute ("NumRows", uintValue);
  uint32_t ueNumRows = static_cast<uint32_t> (uintValue.Get ());

  for (double gnbTheta = 60; gnbTheta < 121; gnbTheta = gnbTheta + m_beamSearchAngleStep)
    {
      for (uint16_t gnbSector = 0; gnbSector <= gnbNumRows; gnbSector++)
        {
          NS_ASSERT(gnbSector < UINT16_MAX);
          gnbPhy->GetBeamManager()->SetSector (gnbSector, gnbTheta);
          complexVector_t gnbW = gnbPhy->GetBeamManager ()->GetCurrentBeamformingVector ();

          for (double ueTheta = 60; ueTheta < 121; ueTheta = static_cast<uint16_t> (ueTheta + m_beamSearchAngleStep))
            {
              for (uint16_t ueSector = 0; ueSector <= ueNumRows; ueSector++)
                {
                  NS_ASSERT(ueSector < UINT16_MAX);
                  uePhy->GetBeamManager ()->SetSector (ueSector, ueTheta);
                  complexVector_t ueW = uePhy->GetBeamManager ()->GetCurrentBeamformingVector ();

                  NS_ABORT_MSG_IF (gnbW.size()==0 || ueW.size()==0,
                                   "Beamforming vectors must be initialized in order to calculate the long term matrix.");

                  Ptr<ThreeGppSpectrumPropagationLossModel> threeGppSplm = DynamicCast<ThreeGppSpectrumPropagationLossModel>(gnbThreeGppSpectrumPropModel);
                  Ptr<MatrixBasedChannelModel> matrixBasedChannelModel = threeGppSplm -> GetChannelModel();
                  Ptr<ThreeGppChannelModel> channelModel = DynamicCast<ThreeGppChannelModel>(matrixBasedChannelModel);
                  NS_ASSERT (channelModel!=nullptr);

                  Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix = channelModel -> GetChannel (gnbDev->GetNode ()->GetObject<MobilityModel>(),
                                                                                                                ueDev->GetNode()->GetObject<MobilityModel>(),
                                                                                                                gnbPhy->GetAntennaArray (),
                                                                                                                uePhy->GetAntennaArray ());

                  const ThreeGppAntennaArrayModel::ComplexVector estimatedLongTermComponent = GetEstimatedLongTermComponent (channelMatrix, gnbW, ueW);

                  double estimatedLongTermMetric = CalculateTheEstimatedLongTermMetric (estimatedLongTermComponent);

                  NS_LOG_LOGIC (" Estimated long term metric value: "<< estimatedLongTermMetric <<
                                " gnb theta " << gnbTheta <<
                                " ue theta " << ueTheta <<
                                " gnb sector " << (M_PI *  static_cast<double> (gnbSector) / static_cast<double> (gnbNumRows) - 0.5 * M_PI) / (M_PI) * 180 <<
                                " ue sector " << (M_PI * static_cast<double> (ueSector) / static_cast<double> (ueNumRows) - 0.5 * M_PI) / (M_PI) * 180);

                  if (max < estimatedLongTermMetric)
                    {
                      max = estimatedLongTermMetric;
                      maxTxSector = gnbSector;
                      maxRxSector = ueSector;
                      maxTxTheta = gnbTheta;
                      maxRxTheta = ueTheta;
                      maxTxW = gnbW;
                      maxRxW = ueW;
                    }
                }
            }
        }
    }

  *gnbBfv = BeamformingVector (std::make_pair(maxTxW, BeamId (maxTxSector, maxTxTheta)));
  *ueBfv = BeamformingVector (std::make_pair (maxRxW, BeamId (maxRxSector, maxRxTheta)));

  NS_LOG_DEBUG ("Beamforming vectors for gNB with node id: "<< gnbDev->GetNode()->GetId () <<
                " and UE with node id: " << ueDev->GetNode()->GetId () <<
                " txTheta " << maxTxTheta <<
                " rxTheta " << maxRxTheta <<
                " tx sector " << (M_PI * static_cast<double> (maxTxSector) / static_cast<double> (gnbNumRows) - 0.5 * M_PI) / (M_PI) * 180 <<
                " rx sector " << (M_PI * static_cast<double> (maxRxSector) / static_cast<double> (ueNumRows) - 0.5 * M_PI) / (M_PI) * 180);
}

double
RealisticBeamformingAlgorithm::CalculateTheEstimatedLongTermMetric (const ThreeGppAntennaArrayModel::ComplexVector& longTermComponent) const
{
  NS_LOG_FUNCTION (this);

  double totalSum = 0;
  for (std::complex<double> c:longTermComponent)
    {
      totalSum += c.imag () * c.imag () + c.real () * c.real ();
    }
  return totalSum;
}


ThreeGppAntennaArrayModel::ComplexVector
RealisticBeamformingAlgorithm::GetEstimatedLongTermComponent (const Ptr<const MatrixBasedChannelModel::ChannelMatrix>& channelMatrix,
                                                              const ThreeGppAntennaArrayModel::ComplexVector &sW,
                                                              const ThreeGppAntennaArrayModel::ComplexVector &uW) const
{
  NS_LOG_FUNCTION (this);

  uint16_t sAntenna = static_cast<uint16_t> (sW.size ());
  uint16_t uAntenna = static_cast<uint16_t> (uW.size ());

  NS_LOG_DEBUG ("Calculate the estimation of the long term component with sAntenna: " << sAntenna << " uAntenna: " << uAntenna);
  ThreeGppAntennaArrayModel::ComplexVector estimatedlongTerm;

  NS_ABORT_IF (m_lastSrsSinrPerSlot == 0);

  double varError = 1 / (m_lastSrsSinrPerSlot); // SINR the SINR from UL SRS reception
  uint8_t numCluster = static_cast<uint8_t> (channelMatrix->m_channel[0][0].size ());

  for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
    {
      std::complex<double> txSum (0,0);
      for (uint16_t sIndex = 0; sIndex < sAntenna; sIndex++)
        {
          std::complex<double> rxSum (0,0);
          for (uint16_t uIndex = 0; uIndex < uAntenna; uIndex++)
            {
              //error is generated from the normal random variable with mean 0 and  variance varError*sqrt(1/2) for real/imaginary parts
              std::complex<double> error = std::complex <double> (m_normalRandomVariable->GetValue (0, sqrt (0.5) * varError),
                                                                  m_normalRandomVariable->GetValue (0, sqrt (0.5) * varError)) ;
              std::complex<double> hEstimate = channelMatrix->m_channel [uIndex][sIndex][cIndex] + error;
              rxSum += uW[uIndex] * (hEstimate);
            }
          txSum = txSum + sW[sIndex] * rxSum;
        }
      estimatedlongTerm.push_back (txSum);
    }
  return estimatedlongTerm;
}


void
RealisticBeamformingAlgorithm::SetSrsSinr (double sinrSrs)
{
  m_lastRerportedSrsSinr = sinrSrs;
}


} // end of namespace ns-3
