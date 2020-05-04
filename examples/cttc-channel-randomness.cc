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

/**
 * \ingroup examples
 * \file cttc-nr-demo.cc
 * \brief A cozy, simple, NR demo (in a tutorial style)
 *
 * This example is intended to test the randmness of the channel in order to see
 * if we can reproduce the same channel realization within the same simulation run.
 *
 * This example is needed for the RemHelper generation task in order to decide
 * how to handle the randomness and how to calculate different RemPoints without
 * having these calculations correlate.
 *
 * \code{.unparsed}
$ ./waf --run "cttc-channel-randomness --Help"
    \endcode
 *
 */


#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/nr-module.h"
#include "ns3/config-store.h"
#include "ns3/config-store-module.h"
#include "ns3/simple-net-device.h"
#include <ns3/spectrum-model.h>
#include "ns3/three-gpp-channel-model.h"
#include "ns3/mmwave-spectrum-value-helper.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/log.h"


using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("CttcChannelRandomness");


/**
 * Perform the beamforming using the DFT beamforming method
 * \param thisDevice the device performing the beamforming
 * \param thisAntenna the antenna object associated to thisDevice
 * \param otherDevice the device towards which point the beam
 */
static void
DoBeamforming (Ptr<NetDevice> thisDevice, Ptr<ThreeGppAntennaArrayModel> thisAntenna, Ptr<NetDevice> otherDevice)
{
  ThreeGppAntennaArrayModel::ComplexVector antennaWeights;

  // retrieve the position of the two devices
  Vector aPos = thisDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  Vector bPos = otherDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();

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
  int totNoArrayElements = thisAntenna->GetNumberOfElements ();

  // the total power is divided equally among the antenna elements
  double power = 1 / sqrt (totNoArrayElements);

  // compute the antenna weights
  for (int ind = 0; ind < totNoArrayElements; ind++)
    {
      Vector loc = thisAntenna->GetElementLocation (ind);
      double phase = -2 * M_PI * (sin (vAngleRadian) * cos (hAngleRadian) * loc.x
                                  + sin (vAngleRadian) * sin (hAngleRadian) * loc.y
                                  + cos (vAngleRadian) * loc.z);
      antennaWeights.push_back (exp (std::complex<double> (0, phase)) * power);
    }

  // store the antenna weights
  thisAntenna->SetBeamformingVector (antennaWeights);
}


int 
main (int argc, char *argv[])
{
  double frequency = 2125.0e6;
  double bandwidth = 100e6;
  uint8_t numerology = 0;
  double txPower = 40;
  double distance = 10.0;
  std::string scenario = "UMa"; // 3GPP propagation scenario

  uint32_t simTimeMs = 1000;
  bool logging = false;

  CommandLine cmd;
  cmd.AddValue ("frequency",
                "The operating frequency in Hz (2125.0e6 corresponds to EARFCN 2100)",
                frequency);
  cmd.AddValue ("bandwidth",
                "The system BW",
                bandwidth);
  cmd.AddValue ("numerology",
                "The numerology",
                numerology);
  cmd.AddValue ("txPower",
                "The transmission power in dBm",
                txPower);
  cmd.AddValue ("distance",
                "The distance between tx and rx nodes in meters",
                distance);
  cmd.AddValue ("scenario",
                "The 3GPP propagation scenario for the simulation."
                "Choose among 'UMa'and 'UMi-StreetCanyon'",
                scenario);
  cmd.AddValue ("simTimeMs",
                "Simulation time in ms",
                simTimeMs);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.Parse (argc, argv);


  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds(0))); // update the channel at each iteration
  Config::SetDefault ("ns3::ThreeGppChannelConditionModel::UpdatePeriod", TimeValue(MilliSeconds (0.0))); // do not update the channel condition


  // create the tx and rx nodes
  NodeContainer nodes;
  nodes.Create (2);

  // create the tx and rx devices
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();

  // associate the nodes and the devices
  nodes.Get (0)->AddDevice (txDev);
  txDev->SetNode (nodes.Get (0));
  nodes.Get (1)->AddDevice (rxDev);
  rxDev->SetNode (nodes.Get (1));

  // create the tx and rx mobility models, set the positions
  Ptr<MobilityModel> txMob = CreateObject<ConstantPositionMobilityModel> ();
  txMob->SetPosition (Vector (0.0,0.0,10.0));
  Ptr<MobilityModel> rxMob = CreateObject<ConstantPositionMobilityModel> ();
  rxMob->SetPosition (Vector (distance,0.0,1.6));

  // assign the mobility models to the nodes
  nodes.Get (0)->AggregateObject (txMob);
  nodes.Get (1)->AggregateObject (rxMob);


  RngSeedManager::SetSeed(1);
  RngSeedManager::SetRun(1);


  Ptr<ThreeGppPropagationLossModel> m_propagationLossModel; //!< the PropagationLossModel object
  Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLossModel; //!< the SpectrumPropagationLossModel object

  // create and configure the factories for the channel condition and propagation loss models
  ObjectFactory propagationLossModelFactory;
  ObjectFactory channelConditionModelFactory;

  if (scenario == "UMa")
  {
    propagationLossModelFactory.SetTypeId (ThreeGppUmaPropagationLossModel::GetTypeId ());
    channelConditionModelFactory.SetTypeId (ThreeGppUmaChannelConditionModel::GetTypeId ());
  }
  else if (scenario == "UMi-StreetCanyon")
  {
    propagationLossModelFactory.SetTypeId (ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId ());
    channelConditionModelFactory.SetTypeId (ThreeGppUmiStreetCanyonChannelConditionModel::GetTypeId ());
  }
  else
  {
      NS_FATAL_ERROR ("The scenario can be 'UMa'or 'UMi-StreetCanyon'");
  }

  // create the propagation loss model
  m_propagationLossModel = propagationLossModelFactory.Create<ThreeGppPropagationLossModel> ();
  m_propagationLossModel->SetAttribute ("Frequency", DoubleValue (frequency));
  m_propagationLossModel->SetAttribute ("ShadowingEnabled", BooleanValue (false));

  // create the spectrum propagation loss model
  m_spectrumLossModel = CreateObject<ThreeGppSpectrumPropagationLossModel> ();
  m_spectrumLossModel->SetChannelModelAttribute ("Frequency", DoubleValue (frequency));
  m_spectrumLossModel->SetChannelModelAttribute ("Scenario", StringValue (scenario));

  // create the channel condition model and associate it with the spectrum and
  // propagation loss model
  Ptr<ChannelConditionModel> condModel = channelConditionModelFactory.Create<ThreeGppChannelConditionModel> ();
  m_spectrumLossModel->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (condModel));
  m_propagationLossModel->SetChannelConditionModel (condModel);

  // create the antenna objects and set their dimensions
  Ptr<ThreeGppAntennaArrayModel> txAntenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));
  Ptr<ThreeGppAntennaArrayModel> rxAntenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));

  // initialize the devices in the ThreeGppSpectrumPropagationLossModel
  m_spectrumLossModel->AddDevice (txDev, txAntenna);
  m_spectrumLossModel->AddDevice (rxDev, rxAntenna);


  // set the beamforming vectors
  DoBeamforming (txDev, txAntenna, rxDev);
  DoBeamforming (rxDev, rxAntenna, txDev);


  Ptr<const SpectrumModel> sm =  MmWaveSpectrumValueHelper::GetSpectrumModel (bandwidth, frequency, numerology);
  Ptr<const SpectrumValue> txPsd = MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity (txPower, sm);
  NS_LOG_UNCOND ("Average tx power " << 10*log10(Sum (*txPsd) * 180e3) << " dB");
  Ptr<SpectrumValue> rxPsd = m_spectrumLossModel->DoCalcRxPowerSpectralDensity (txPsd, txMob, rxMob);
  NS_LOG_UNCOND ("Average rx power " << 10*log10 (Sum (*rxPsd) * 180e3) << " dB");


  Simulator::Stop (MilliSeconds (simTimeMs));
  Simulator::Run ();


  Simulator::Destroy ();
  return 0;
}

