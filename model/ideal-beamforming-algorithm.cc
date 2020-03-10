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
#include "mmwave-spectrum-phy.h"
#include "mmwave-ue-phy.h"
#include "mmwave-enb-phy.h"
#include "mmwave-enb-net-device.h"
#include "mmwave-ue-net-device.h"
#include "beam-manager.h"

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
IdealBeamformingAlgorithm::GetBeamformingVectors(const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const
{
  DoGetBeamformingVectors (gnbDev, ueDev, gnbBfv, ueBfv);
}


void
IdealBeamformingAlgorithm::SetOwner (uint8_t bwpId)
{
  m_bwpId = bwpId;
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
CellScanBeamforming::DoGetBeamformingVectors (const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const
{
  NS_ABORT_MSG_IF (gnbDev == nullptr || ueDev == nullptr, "Something went wrong, gnb or UE device does not exist.");

  NS_ABORT_MSG_IF (gnbDev->GetNode()->GetObject<MobilityModel>()->GetDistanceFrom( ueDev->GetNode()->GetObject<MobilityModel>()) == 0,
                   "Beamforming method cannot be performed between two devices that are placed in the same position.");

  // TODO check if this is correct: assuming the ccId of gNB PHY and corresponding UE PHY are the equal
  Ptr<MmWaveEnbPhy> txPhy = gnbDev->GetPhy(m_bwpId);
  Ptr<MmWaveUePhy> rxPhy = ueDev->GetPhy(m_bwpId);

  Ptr<MmWaveSpectrumPhy> txSpectrumPhy = txPhy->GetSpectrumPhy();
  Ptr<MmWaveSpectrumPhy> rxSpectrumPhy = rxPhy->GetSpectrumPhy();

  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel();
  Ptr<SpectrumChannel> rxSpectrumChannel = rxSpectrumPhy->GetSpectrumChannel();

  Ptr<SpectrumPropagationLossModel> txThreeGppSpectrumPropModel = txSpectrumChannel->GetSpectrumPropagationLossModel();
  Ptr<SpectrumPropagationLossModel> rxThreeGppSpectrumPropModel = rxSpectrumChannel->GetSpectrumPropagationLossModel();

  NS_ASSERT_MSG (txThreeGppSpectrumPropModel == rxThreeGppSpectrumPropModel, "Devices should be connected on the same spectrum channel");

  Ptr<const SpectrumValue> fakePsd = IdealBeamformingAlgorithm::CreateFakeTxPowerSpectralDensity (0.0, txSpectrumPhy->GetRxSpectrumModel());

  double max = 0, maxTxTheta = 0, maxRxTheta = 0;
  uint16_t maxTxSector = 0, maxRxSector = 0;
  complexVector_t  maxLongTerm, maxTxW, maxRxW;

  UintegerValue uintValue;
  txSpectrumPhy->GetAntennaArray()->GetAttribute("NumRows", uintValue);
  uint32_t txNumRows = uintValue.Get();
  rxSpectrumPhy->GetAntennaArray()->GetAttribute("NumRows", uintValue);
  uint32_t rxNumRows = uintValue.Get();

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
DirectPathBeamforming::DoGetBeamformingVectors (const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const
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
OptimalCovMatrixBeamforming::DoGetBeamformingVectors (const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const
{

}

} // end of ns3 namespace
