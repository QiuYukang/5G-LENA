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
#include "beam-manager.h"
#include "mmwave-spectrum-phy.h"
#include "mmwave-ue-phy.h"
#include "mmwave-enb-phy.h"
#include "mmwave-enb-net-device.h"
#include "mmwave-ue-net-device.h"


namespace ns3{

NS_LOG_COMPONENT_DEFINE ("IdealBeamformingAlgorithm");
NS_OBJECT_ENSURE_REGISTERED (CellScanBeamforming);
NS_OBJECT_ENSURE_REGISTERED (DirectPathBeamforming);
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
  static TypeId tid = TypeId ("IdealBeamformingAlgorithm")
    .SetParent<Object> ()
  ;

  return tid;
}


Ptr<const SpectrumValue>
IdealBeamformingAlgorithm::CreateFakeTxPowerSpectralDensity (double powerTx, Ptr<const SpectrumModel> txSm)
{
  Ptr<SpectrumValue> txPsd = Create <SpectrumValue> (txSm);
  double powerTxW = std::pow (10., (powerTx - 30) / 10);
  double bw =   (txPsd->GetSpectrumModel()->End()-1)->fh - txPsd->GetSpectrumModel()->Begin()->fl;
  double txPowerDensity = powerTxW / bw;

  std::vector<int> listOfSubchannels;
  for (size_t rbId = 0; rbId < txSm->GetNumBands (); rbId++)
    {
      (*txPsd)[rbId] = txPowerDensity;
    }
  NS_LOG_LOGIC (*txPsd);
  return txPsd;
}

void
IdealBeamformingAlgorithm::Run(Ptr<MmWaveEnbNetDevice> gnbDev, Ptr<MmWaveUeNetDevice> ueDev) const
{
  DoRun (gnbDev, ueDev);
}


void
IdealBeamformingAlgorithm::SetOwner (uint8_t ccId)
{
  m_ccId = ccId;
}

TypeId
CellScanBeamforming::GetTypeId (void)
{
  static TypeId tid = TypeId ("CellScanBeamforming")
    .SetParent<IdealBeamformingAlgorithm> ()
    .AddConstructor<CellScanBeamforming>()
  ;

  return tid;
}


void
CellScanBeamforming::DoRun (Ptr<MmWaveEnbNetDevice> gNbDev, Ptr<MmWaveUeNetDevice> ueDev) const
{
  NS_ABORT_MSG_IF (gNbDev == nullptr || ueDev == nullptr, "Something went wrong, gnb or UE device does not exist.");

  NS_ABORT_MSG_IF (gNbDev->GetNode()->GetObject<MobilityModel>()->GetDistanceFrom( ueDev->GetNode()->GetObject<MobilityModel>()) == 0,
                   "Beamforming method cannot be performed between two devices that are placed in the same position.");

  // TODO check if this is correct: assuming the ccId of gNB PHY and corresponding UE PHY are the equal
  Ptr<MmWaveEnbPhy> txPhy = gNbDev->GetPhy(m_ccId);
  Ptr<MmWaveUePhy> rxPhy = ueDev->GetPhy(m_ccId);

  Ptr<MmWaveSpectrumPhy> txSpectrumPhy = txPhy->GetSpectrumPhy();
  Ptr<MmWaveSpectrumPhy> rxSpectrumPhy = rxPhy->GetSpectrumPhy();

  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel();
  Ptr<SpectrumChannel> rxSpectrumChannel = rxSpectrumPhy->GetSpectrumChannel();

  Ptr<SpectrumPropagationLossModel> txThreeGppSpectrumPropModel = txSpectrumChannel->GetSpectrumPropagationLossModel();
  Ptr<SpectrumPropagationLossModel> rxThreeGppSpectrumPropModel = rxSpectrumChannel->GetSpectrumPropagationLossModel();

  NS_ASSERT_MSG (txThreeGppSpectrumPropModel == rxThreeGppSpectrumPropModel, "Devices should be connected on the same spectrum channel");

  Ptr<BeamManager> txBeamManager = txPhy->GetBeamManager();
  Ptr<BeamManager> rxBeamManager = rxPhy->GetBeamManager();

  NS_ABORT_MSG_IF (txBeamManager == nullptr || rxBeamManager == nullptr, "Tx or Rx antenna beam managers are not set");

  Ptr<const SpectrumValue> fakePsd = IdealBeamformingAlgorithm::CreateFakeTxPowerSpectralDensity (0.0, txSpectrumPhy->GetRxSpectrumModel());

  double max = 0, maxTxTheta = 0, maxRxTheta = 0;
  size_t maxTx = 0, maxRx = 0;
  complexVector_t  maxLongTerm, maxTxW, maxRxW;

  UintegerValue uintValue;
  txBeamManager->GetAntennaArray()->GetAttribute("NumRows", uintValue);
  uint32_t txNumRows = uintValue.Get();
  rxBeamManager->GetAntennaArray()->GetAttribute("NumRows", uintValue);
  uint32_t rxNumRows = uintValue.Get();

  for (uint16_t txTheta = 60; txTheta < 121; txTheta = txTheta + m_beamSearchAngleStep)
    {
      for (size_t tx = 0; tx <= txNumRows; tx++)
        {
          NS_ASSERT(tx < UINT8_MAX);
          SetSector (static_cast<uint8_t> (tx), txTheta, txBeamManager);

          complexVector_t txW = txBeamManager->GetCurrentBeamformingVector();

          for (uint16_t rxTheta = 60; rxTheta < 121; rxTheta = static_cast<uint16_t> (rxTheta + m_beamSearchAngleStep))
            {
              for (size_t rx = 0; rx <= rxNumRows; rx++)
                {
                  NS_ASSERT(rx < UINT8_MAX);

                  SetSector (static_cast<uint8_t> (rx), rxTheta, rxBeamManager);

                  complexVector_t rxW = rxBeamManager->GetCurrentBeamformingVector ();

                  NS_ABORT_MSG_IF (txW.size()==0 || rxW.size()==0, "Beamforming vectors must be initialized in order to caclulate the long term matrix.");

                  Ptr<SpectrumValue> rxPsd = txThreeGppSpectrumPropModel->CalcRxPowerSpectralDensity (fakePsd, gNbDev->GetNode()->GetObject<MobilityModel>(), ueDev->GetNode()->GetObject<MobilityModel>());

                  size_t nbands = rxPsd->GetSpectrumModel ()->GetNumBands ();
                  double power = Sum (*rxPsd) / nbands;
                  
                  NS_LOG_LOGIC (" Rx power: "<< power << "txTheta " << txTheta << " rxTheta " << rxTheta << " tx sector " <<
                                (M_PI *  static_cast<double> (tx) / static_cast<double>(txNumRows) - 0.5 * M_PI) / (M_PI) * 180 << " rx sector " <<
                                (M_PI * static_cast<double> (rx) / static_cast<double> (rxNumRows) - 0.5 * M_PI) / (M_PI) * 180);

                  if (max < power)
                    {
                      max = power;
                      maxTx = tx;
                      maxRx = rx;
                      maxTxTheta = txTheta;
                      maxRxTheta = rxTheta;
                      maxTxW = txW;
                      maxRxW = rxW;
                    }
                }
            }
        }
    }

  txBeamManager->SetBeamformingVector (maxTxW, BeamId (maxTx, maxTxTheta), ueDev);
  rxBeamManager->SetBeamformingVector (maxRxW, BeamId (maxRx, maxRxTheta), gNbDev);

  NS_LOG_DEBUG ("Beamforming vectors for gNB with node id: "<< gNbDev->GetNode()->GetId ()<<
                " and UE with node id: " << ueDev->GetNode()->GetId ()<<
                " are txTheta " << maxTxTheta << " rxTheta " << maxRxTheta << " tx sector " <<
                (M_PI * static_cast<double> (maxTx) / static_cast<double> (txNumRows) - 0.5 * M_PI) / (M_PI) * 180 << " rx sector " <<
                (M_PI * static_cast<double> (maxRx) / static_cast<double> (rxNumRows) - 0.5 * M_PI) / (M_PI) * 180);
}


void
CellScanBeamforming::SetSector (uint16_t sector, double elevation, Ptr<BeamManager> beamManager) const
{
  NS_LOG_INFO ("Set sector to :"<< (unsigned) sector<< ", and elevation to:"<< elevation);
  complexVector_t tempVector;

  UintegerValue uintValueNumRows;
  beamManager->GetAntennaArray()->GetAttribute("NumRows", uintValueNumRows);


  double hAngle_radian = M_PI * (static_cast<double>(sector) / static_cast<double>(uintValueNumRows.Get())) - 0.5 * M_PI;
  double vAngle_radian = elevation * M_PI / 180;
  uint16_t size = beamManager->GetAntennaArray()->GetNumberOfElements();
  double power = 1 / sqrt (size);
  for (auto ind = 0; ind < size; ind++)
    {
      Vector loc = beamManager->GetAntennaArray()->GetElementLocation(ind);
      double phase = -2 * M_PI * (sin (vAngle_radian) * cos (hAngle_radian) * loc.x
                                  + sin (vAngle_radian) * sin (hAngle_radian) * loc.y
                                  + cos (vAngle_radian) * loc.z);
      tempVector.push_back (exp (std::complex<double> (0, phase)) * power);
    }
   beamManager->GetAntennaArray()->SetBeamformingVector(tempVector);
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
DirectPathBeamforming::DoRun (Ptr<MmWaveEnbNetDevice> gNbDev, Ptr<MmWaveUeNetDevice> ueDev) const
{

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
OptimalCovMatrixBeamforming::DoRun (Ptr<MmWaveEnbNetDevice> gNbDev, Ptr<MmWaveUeNetDevice> ueDev) const
{

}

} // end of ns3 namespace
