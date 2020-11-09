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
#include "nr-spectrum-value-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("RealisticBeamformingAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (RealisticBeamformingAlgorithm);

RealisticBeamformingAlgorithm::RealisticBeamformingAlgorithm ()
{

}

RealisticBeamformingAlgorithm::~RealisticBeamformingAlgorithm()
{

}

TypeId
RealisticBeamformingAlgorithm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RealisticBeamformingAlgorithm")
                     .SetParent<Object> ()
                     .AddConstructor<RealisticBeamformingAlgorithm> ()
                     .AddAttribute ("BeamSearchAngleStep",
                                    "Angle step when searching for the best beam",
                                    DoubleValue (30),
                                    MakeDoubleAccessor (&CellScanBeamforming::SetBeamSearchAngleStep,
                                                        &CellScanBeamforming::GetBeamSearchAngleStep),
                                    MakeDoubleChecker<double> ());

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
RealisticBeamformingAlgorithm::GetBeamformingVectors(const Ptr<const NrGnbNetDevice>& gnbDev,
                                                 const Ptr<const NrUeNetDevice>& ueDev,
                                                 BeamformingVector* gnbBfv,
                                                 BeamformingVector* ueBfv,
                                                 uint16_t ccId) const
{
  DoGetBeamformingVectors (gnbDev, ueDev, gnbBfv, ueBfv, ccId);
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
  complexVector_t  maxTxW, maxRxW;

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

  double varError = 1 / (m_lastRerportedSrsSinr); // SINR the SINR from UL SRS reception

  uint8_t numCluster = static_cast<uint8_t> (channelMatrix->m_channel[0][0].size ());

  for (uint8_t cIndex = 0; cIndex < numCluster; cIndex++)
    {
      std::complex<double> txSum (0,0);
      for (uint16_t sIndex = 0; sIndex < sAntenna; sIndex++)
        {
          std::complex<double> rxSum (0,0);
          for (uint16_t uIndex = 0; uIndex < uAntenna; uIndex++)
            {
              Ptr<NormalRandomVariable> normalRandomVariable = CreateObject<NormalRandomVariable> ();
              //error is generated from the normal random variable with mean 0 and  variance varError*sqrt(1/2) for real/imaginary parts
              std::complex<double> error = std::complex <double> (normalRandomVariable->GetValue (0, sqrt (0.5) * varError),
                                                                  normalRandomVariable->GetValue (0, sqrt (0.5) * varError)) ;

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
