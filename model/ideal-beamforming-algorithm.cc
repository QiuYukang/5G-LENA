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

#include "ideal-beamforming-algorithm.h"
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/node.h>
#include "nr-spectrum-phy.h"
#include "nr-ue-phy.h"
#include "nr-gnb-phy.h"
#include "nr-gnb-net-device.h"
#include "nr-ue-net-device.h"
#include "beam-manager.h"
#include <ns3/double.h>
#include <ns3/angles.h>
#include <ns3/uinteger.h>
#include "nr-spectrum-value-helper.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("IdealBeamformingAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (CellScanBeamforming);
NS_OBJECT_ENSURE_REGISTERED (DirectPathBeamforming);
NS_OBJECT_ENSURE_REGISTERED (QuasiOmniDirectPathBeamforming);
NS_OBJECT_ENSURE_REGISTERED (OptimalCovMatrixBeamforming);

IdealBeamformingAlgorithm::IdealBeamformingAlgorithm ()
{

}

IdealBeamformingAlgorithm::~IdealBeamformingAlgorithm()
{

}

TypeId
IdealBeamformingAlgorithm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IdealBeamformingAlgorithm")
                      .SetParent<Object> ()
  ;

  return tid;
}

void
IdealBeamformingAlgorithm::GetBeamformingVectors(const Ptr<const NrGnbNetDevice>& gnbDev,
                                                 const Ptr<const NrUeNetDevice>& ueDev,
                                                 BeamformingVector* gnbBfv,
                                                 BeamformingVector* ueBfv,
                                                 uint16_t ccId) const
{
  DoGetBeamformingVectors (gnbDev, ueDev, gnbBfv, ueBfv, ccId);
}

TypeId
CellScanBeamforming::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CellScanBeamforming")
                     .SetParent<IdealBeamformingAlgorithm> ()
                     .AddConstructor<CellScanBeamforming>()
                     .AddAttribute ("BeamSearchAngleStep",
                                    "Angle step when searching for the best beam",
                                    DoubleValue (30),
                                    MakeDoubleAccessor (&CellScanBeamforming::SetBeamSearchAngleStep,
                                                        &CellScanBeamforming::GetBeamSearchAngleStep),
                                    MakeDoubleChecker<double> ());

  return tid;
}

void
CellScanBeamforming::SetBeamSearchAngleStep (double beamSearchAngleStep)
{
  m_beamSearchAngleStep = beamSearchAngleStep;
}

double
CellScanBeamforming::GetBeamSearchAngleStep () const
{
  return m_beamSearchAngleStep;
}

void
CellScanBeamforming::DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                              const Ptr<const NrUeNetDevice>& ueDev,
                                              BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const
{
  NS_ABORT_MSG_IF (gnbDev == nullptr || ueDev == nullptr, "Something went wrong, gnb or UE device does not exist.");

  NS_ABORT_MSG_IF (gnbDev->GetNode()->GetObject<MobilityModel>()->GetDistanceFrom( ueDev->GetNode()->GetObject<MobilityModel>()) == 0,
                   "Beamforming method cannot be performed between two devices that are placed in the same position.");

  // TODO check if this is correct: assuming the ccId of gNB PHY and corresponding UE PHY are the equal
  Ptr<const NrGnbPhy> txPhy = gnbDev->GetPhy(ccId);
  Ptr<const NrUePhy> rxPhy = ueDev->GetPhy(ccId);

  Ptr<const NrSpectrumPhy> txSpectrumPhy = txPhy->GetSpectrumPhy();
  Ptr<const NrSpectrumPhy> rxSpectrumPhy = rxPhy->GetSpectrumPhy();

  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel(); // SpectrumChannel should be const.. but need to change ns-3-dev
  Ptr<SpectrumChannel> rxSpectrumChannel = rxSpectrumPhy->GetSpectrumChannel();

  Ptr<const SpectrumPropagationLossModel> txThreeGppSpectrumPropModel = txSpectrumChannel->GetSpectrumPropagationLossModel();
  Ptr<const SpectrumPropagationLossModel> rxThreeGppSpectrumPropModel = rxSpectrumChannel->GetSpectrumPropagationLossModel();

  NS_ASSERT_MSG (txThreeGppSpectrumPropModel == rxThreeGppSpectrumPropModel, "Devices should be connected on the same spectrum channel");

  Ptr<const SpectrumValue> fakePsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity (0.0, txSpectrumPhy->GetRxSpectrumModel());

  double max = 0, maxTxTheta = 0, maxRxTheta = 0;
  uint16_t maxTxSector = 0, maxRxSector = 0;
  complexVector_t  maxLongTerm, maxTxW, maxRxW;

  UintegerValue uintValue;
  txPhy->GetAntennaArray()->GetAttribute("NumRows", uintValue);
  uint32_t txNumRows = static_cast<uint32_t> (uintValue.Get());
  rxPhy->GetAntennaArray()->GetAttribute("NumRows", uintValue);
  uint32_t rxNumRows = static_cast<uint32_t> (uintValue.Get());

  for (double txTheta = 60; txTheta < 121; txTheta = txTheta + m_beamSearchAngleStep)
    {
      for (uint16_t txSector = 0; txSector <= txNumRows; txSector++)
        {
          NS_ASSERT(txSector < UINT16_MAX);

          txPhy->GetBeamManager()->SetSector (txSector, txTheta);
          complexVector_t txW = txPhy->GetBeamManager()->GetCurrentBeamformingVector();

          for (double rxTheta = 60; rxTheta < 121; rxTheta = static_cast<uint16_t> (rxTheta + m_beamSearchAngleStep))
            {
              for (uint16_t rxSector = 0; rxSector <= rxNumRows; rxSector++)
                {
                  NS_ASSERT(rxSector < UINT16_MAX);

                  rxPhy->GetBeamManager()->SetSector (rxSector, rxTheta);
                  complexVector_t rxW = rxPhy->GetBeamManager()->GetCurrentBeamformingVector();

                  NS_ABORT_MSG_IF (txW.size()==0 || rxW.size()==0, "Beamforming vectors must be initialized in order to calculate the long term matrix.");

                  Ptr<SpectrumValue> rxPsd = txThreeGppSpectrumPropModel->CalcRxPowerSpectralDensity (fakePsd, gnbDev->GetNode()->GetObject<MobilityModel>(), ueDev->GetNode()->GetObject<MobilityModel>());

                  size_t nbands = rxPsd->GetSpectrumModel ()->GetNumBands ();
                  double power = Sum (*rxPsd) / nbands;
                  
                  NS_LOG_LOGIC (" Rx power: "<< power << "txTheta " << txTheta << " rxTheta " << rxTheta << " tx sector " <<
                                (M_PI *  static_cast<double> (txSector) / static_cast<double>(txNumRows) - 0.5 * M_PI) / (M_PI) * 180 << " rx sector " <<
                                (M_PI * static_cast<double> (rxSector) / static_cast<double> (rxNumRows) - 0.5 * M_PI) / (M_PI) * 180);

                  if (max < power)
                    {
                      max = power;
                      maxTxSector = txSector;
                      maxRxSector = rxSector;
                      maxTxTheta = txTheta;
                      maxRxTheta = rxTheta;
                      maxTxW = txW;
                      maxRxW = rxW;
                    }
                }
            }
        }
    }

  *gnbBfv = BeamformingVector (std::make_pair(maxTxW, BeamId (maxTxSector, maxTxTheta)));
  *ueBfv = BeamformingVector (std::make_pair (maxRxW, BeamId (maxRxSector, maxRxTheta)));

  NS_LOG_DEBUG ("Beamforming vectors for gNB with node id: "<< gnbDev->GetNode()->GetId ()<<
                " and UE with node id: " << ueDev->GetNode()->GetId ()<<
                " are txTheta " << maxTxTheta << " rxTheta " << maxRxTheta <<
                " tx sector " << (M_PI * static_cast<double> (maxTxSector) / static_cast<double> (txNumRows) - 0.5 * M_PI) / (M_PI) * 180 <<
                " rx sector " << (M_PI * static_cast<double> (maxRxSector) / static_cast<double> (rxNumRows) - 0.5 * M_PI) / (M_PI) * 180);
}


TypeId
DirectPathBeamforming::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DirectPathBeamforming")
                     .SetParent<IdealBeamformingAlgorithm> ()
                     .AddConstructor<DirectPathBeamforming>()
  ;
  return tid;
}


void
DirectPathBeamforming::DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice> &gnbDev,
                                                const Ptr<const NrUeNetDevice> &ueDev,
                                                BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const
{
  NS_LOG_FUNCTION (this);

  Ptr<MobilityModel> gnbMob = gnbDev->GetNode()->GetObject<MobilityModel>();
  Ptr<MobilityModel> ueMob = ueDev->GetNode()->GetObject<MobilityModel>();
  Ptr<const ThreeGppAntennaArrayModel> gnbAntenna = gnbDev->GetPhy(ccId)->GetAntennaArray();
  Ptr<const ThreeGppAntennaArrayModel> ueAntenna = ueDev->GetPhy(ccId)->GetAntennaArray();

  DoGetDirectPathBeamformingVector (gnbMob, ueMob, gnbAntenna, gnbBfv, ccId);
  DoGetDirectPathBeamformingVector (ueMob, gnbMob, ueAntenna, ueBfv, ccId);

}

void
DirectPathBeamforming::DoGetDirectPathBeamformingVector(const Ptr<MobilityModel>& a,
                                                        const Ptr<MobilityModel>& b,
                                                        const Ptr<const ThreeGppAntennaArrayModel>& aAntenna,
                                                        BeamformingVector* bfv, uint16_t ccId) const
{
  complexVector_t antennaWeights;

  // retrieve the position of the two devices
  Vector aPos = a->GetPosition ();
  Vector bPos = b->GetPosition ();

  // compute the azimuth and the elevation angles
  Angles completeAngle (bPos,aPos);

  double posX = bPos.x - aPos.x;
  double phiAngle = atan ((bPos.y - aPos.y) / posX);

  if (posX < 0)
    {
      phiAngle = phiAngle + M_PI;
    }
  if (phiAngle < 0)
    {
      phiAngle = phiAngle + 2 * M_PI;
    }

  double hAngleRadian = fmod ((phiAngle + M_PI),2 * M_PI - M_PI); // the azimuth angle
  double vAngleRadian = completeAngle.theta; // the elevation angle

  // retrieve the number of antenna elements
  int totNoArrayElements = aAntenna->GetNumberOfElements ();

  // the total power is divided equally among the antenna elements
  double power = 1 / sqrt (totNoArrayElements);

  // compute the antenna weights
  for (int ind = 0; ind < totNoArrayElements; ind++)
    {
      Vector loc = aAntenna->GetElementLocation (ind);
      double phase = -2 * M_PI * (sin (vAngleRadian) * cos (hAngleRadian) * loc.x
                                  + sin (vAngleRadian) * sin (hAngleRadian) * loc.y
                                  + cos (vAngleRadian) * loc.z);
      antennaWeights.push_back (exp (std::complex<double> (0, phase)) * power);
    }
  // store the antenna weights
  *bfv = BeamformingVector (std::make_pair(antennaWeights, BeamId (0, 0)));
}


TypeId
QuasiOmniDirectPathBeamforming::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuasiOmniDirectPathBeamforming")
                      .SetParent<DirectPathBeamforming> ()
                      .AddConstructor<QuasiOmniDirectPathBeamforming>();
  return tid;
}


void
QuasiOmniDirectPathBeamforming::DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice> &gnbDev,
                                                         const Ptr<const NrUeNetDevice> &ueDev,
                                                         BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const
{
  NS_LOG_FUNCTION (this);

  Ptr<MobilityModel> gnbMob = gnbDev->GetNode()->GetObject<MobilityModel>();
  Ptr<MobilityModel> ueMob = ueDev->GetNode()->GetObject<MobilityModel>();
  Ptr<const ThreeGppAntennaArrayModel> gnbAntenna = gnbDev->GetPhy(ccId)->GetAntennaArray();
  Ptr<const ThreeGppAntennaArrayModel> ueAntenna = ueDev->GetPhy(ccId)->GetAntennaArray();

  // configure gNb beamforming vector to be quasi omni
  UintegerValue numRows, numColumns;
  gnbAntenna->GetAttribute ("NumRows", numRows);
  gnbAntenna->GetAttribute ("NumColumns", numColumns);
  *gnbBfv = std::make_pair (CreateQuasiOmniBfv (numRows.Get(), numColumns.Get()), OMNI_BEAM_ID);

  //configure UE beamforming vector to be directed towards gNB
  DirectPathBeamforming::DoGetDirectPathBeamformingVector (ueMob, gnbMob, ueAntenna, ueBfv, ccId);

}


TypeId
OptimalCovMatrixBeamforming::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OptimalCovMatrixBeamforming")
    .SetParent<IdealBeamformingAlgorithm> ()
    .AddConstructor<OptimalCovMatrixBeamforming>()
  ;

  return tid;
}

void
OptimalCovMatrixBeamforming::DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                                      const Ptr<const NrUeNetDevice>& ueDev,
                                                      BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const
{
  NS_UNUSED (gnbDev);
  NS_UNUSED (ueDev);
  NS_UNUSED (gnbBfv);
  NS_UNUSED (ueBfv);
}

} // end of ns3 namespace
