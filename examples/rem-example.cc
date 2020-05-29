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
 * \file rem-example.cc
 * \ingroup examples
 * \brief REM Creation Example
 *
 * This example describes how to setup a simulation using NrRadioEnvironmentMapHelper.
 *
 * We provide a number of simulation parameters that can be configured in the
 * command line, such as the number of UEs per cell or the number of rows and
 * columns of the gNB and Ue antennas.
 * Please have a look at the possible parameters to know what you can configure
 * through the command line.
 *
 * The user can also specify the type of REM map (BeamShape or CovrageArea) he
 * wishes to generate with the following command:
 * ./waf --run "rem-example --ns3::NrRadioEnvironmentMapHelper::RemMode=BeamShape"
 *
 * The output of the REM includes a map with the SNR values and a map with the
 * SINR. In case there is only one gNB configured, these maps will be the same.
 *
 * The output of this example are REM csv files from which can be generated REM
 * figures with the following command:
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/nr-helper.h"
#include "ns3/log.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-module.h"
#include <ns3/buildings-module.h>

using namespace ns3;

int 
main (int argc, char *argv[])
{
  std::string scenario = "UMa"; //scenario
  enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::UMa;

  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 1;

  double frequency = 28e9;
  double bandwidth = 100e6;
  uint16_t numerology = 0;
  double txPower = 40;

  //Antenna Parameters
  double hBS;   //Depend on the scenario (no input parameters)
  double hUT;
  uint32_t numRowsUe = 2;
  uint32_t numColumnsUe = 2;
  uint32_t numRowsGnb = 4;
  uint32_t numColumnsGnb = 4;
  bool isoUe = true;
  bool isoGnb = false;
  bool enableQuasiOmni = false;

  double mobility = false; //whether to enable mobility
  double speed = 1; // in m/s for walking UT.

  double simTime = 1; // in seconds
  bool logging = true;

  //building parameters in case of buildings addition
  bool enableBuildings; //Depends on the scenario (no input parameter)
  uint32_t numOfBuildings = 1;
  uint32_t apartmentsX = 2;
  uint32_t nFloors = 1;

  CommandLine cmd;
  cmd.AddValue ("scenario",
                "The scenario for the simulation. Choose among 'RMa', 'UMa', "
                "'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen'"
                "'UMa-Buildings', 'UMi-Buildings'.",
                scenario);
  cmd.AddValue ("gNbNum",
                "The number of gNbs in multiple-ue topology",
                gNbNum);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per gNb in multiple-ue topology",
                ueNumPergNb);
  cmd.AddValue ("frequency",
                "The central carrier frequency in Hz.",
                frequency);
  cmd.AddValue ("bandwidth",
                "The system bandwidth to be used",
                bandwidth);
  cmd.AddValue ("numerology",
                "The numerology to be used",
                numerology);
  cmd.AddValue ("txPower",
                "total tx power that will be proportionally assigned to"
                " bands, CCs and bandwidth parts depending on each BWP bandwidth ",
                txPower);
  cmd.AddValue ("numRowsUe",
                "Number of rows for the UE antenna",
                numRowsUe);
  cmd.AddValue ("numColumnsUe",
                "Number of columns for the UE antenna",
                numColumnsUe);
  cmd.AddValue ("isoUe",
                "If true (set to 1), use an isotropic radiation pattern in the Ue ",
                isoUe);
  cmd.AddValue ("numRowsGnb",
                "Number of rows for the gNB antenna",
                numRowsGnb);
  cmd.AddValue ("numColumnsGnb",
                "Number of columns for the gNB antenna",
                numColumnsGnb);
  cmd.AddValue ("isoGnb",
                "If true (set to 1), use an isotropic radiation pattern in the gNB ",
                isoGnb);
  cmd.AddValue ("mobility",
                "If set to 1 UEs will be mobile, when set to 0 UE will be static. "
                "By default, they are mobile.",
                mobility);
  cmd.AddValue ("numOfBuildings",
                "The number of Buildings to deploy in the scenario",
                numOfBuildings);
  cmd.AddValue ("apartmentsX",
                "The number of apartments inside a building",
                apartmentsX);
  cmd.AddValue ("nFloors",
                "The number of floors of a building",
                nFloors);
  cmd.AddValue ("enableQuasiOmni",
                "If true (set to 1) enable QuasiOmni DirectPath Beamforming,"
                "DirectPath Beamforming otherwise",
                enableQuasiOmni);
  cmd.AddValue ("logging",
                "Enable logging"
                "another option is by exporting the NS_LOG environment variable",
                logging);

  cmd.Parse (argc, argv);

  // enable logging
  if(logging)
    {
      //LogComponentEnable ("ThreeGppSpectrumPropagationLossModel", LOG_LEVEL_ALL);
      LogComponentEnable ("ThreeGppPropagationLossModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("ThreeGppChannelModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("ChannelConditionModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      //LogComponentEnable ("LteRlcUm", LOG_LEVEL_LOGIC);
      //LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
    }

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  // set mobile device and base station antenna heights in meters, according to the chosen scenario
  if(scenario.compare("RMa") == 0)
    {
      hBS = 35;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::RMa;
    }
  else if(scenario.compare("UMa") == 0)
    {
      hBS = 25;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMa;
    }
  else if(scenario.compare("UMa-Buildings") == 0)
    {
      hBS = 25;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMa_Buildings;
      enableBuildings = true;
    }
  else if (scenario.compare("UMi-StreetCanyon") == 0)
    {
      hBS = 10;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMi_StreetCanyon;
    }
  else if (scenario.compare("UMi-Buildings") == 0)
    {
      hBS = 10;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMi_Buildings;
      enableBuildings = true;
    }
  else if (scenario.compare("InH-OfficeMixed") == 0)
    {
      hBS = 3;
      hUT = 1;
      scenarioEnum = BandwidthPartInfo::InH_OfficeMixed;
    }
  else if (scenario.compare("InH-OfficeOpen") == 0)
    {
      hBS = 3;
      hUT = 1;
      scenarioEnum = BandwidthPartInfo::InH_OfficeOpen;
    }
  else
    {
      NS_ABORT_MSG("Scenario not supported. Choose among 'RMa', 'UMa', "
                   "'UMi-StreetCanyon', 'InH-OfficeMixed', and 'InH-OfficeOpen'.");
    }

  // create base stations and mobile terminals
  NodeContainer gnbNodes;
  NodeContainer ueNodes;
  gnbNodes.Create (2);
  ueNodes.Create (2);

  // position the base stations
  Ptr<ListPositionAllocator> gnbPositionAlloc = CreateObject<ListPositionAllocator> ();
  gnbPositionAlloc->Add (Vector (0.0, 0.0, hBS));
  gnbPositionAlloc->Add (Vector (0.0, 80.0, hBS));
  MobilityHelper gnbmobility;
  gnbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  gnbmobility.SetPositionAllocator(gnbPositionAlloc);
  gnbmobility.Install (gnbNodes);

  // position the mobile terminals and enable the mobility
  MobilityHelper uemobility;
  uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  uemobility.Install (ueNodes);

  if(mobility)
    {
      ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (90, 15, hUT)); // (x, y, z) in m
      ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, speed, 0)); // move UE1 along the y axis

      ueNodes.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (30, 50.0, hUT)); // (x, y, z) in m
      ueNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (-speed, 0, 0)); // move UE2 along the x axis
    }
  else
    {
      ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (90, 15, hUT));
      ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));

      ueNodes.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (30, 50.0, hUT));
      ueNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));
    }

  if (enableBuildings)
    {
      Ptr<GridBuildingAllocator>  gridBuildingAllocator;
      gridBuildingAllocator = CreateObject<GridBuildingAllocator> ();
      gridBuildingAllocator->SetAttribute ("GridWidth", UintegerValue (numOfBuildings));
      gridBuildingAllocator->SetAttribute ("LengthX", DoubleValue (2 * apartmentsX));
      gridBuildingAllocator->SetAttribute ("LengthY", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("DeltaX", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("DeltaY", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("Height", DoubleValue (3 * nFloors));
      gridBuildingAllocator->SetBuildingAttribute ("NRoomsX", UintegerValue (apartmentsX));
      gridBuildingAllocator->SetBuildingAttribute ("NRoomsY", UintegerValue (2));
      gridBuildingAllocator->SetBuildingAttribute ("NFloors", UintegerValue (nFloors));
      gridBuildingAllocator->SetAttribute ("MinX", DoubleValue (3));
      gridBuildingAllocator->SetAttribute ("MinY", DoubleValue (-3));
      gridBuildingAllocator->Create (numOfBuildings);

      BuildingsHelper::Install (gnbNodes);
      BuildingsHelper::Install (ueNodes);
    }

  /*
   * Create NR simulation helpers
   */
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject <IdealBeamformingHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  nrHelper->SetIdealBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum configuration:
   * We create a single operational band with 1 CC and 1 BWP.
   *
   * |---------------Band---------------|
   * |---------------CC-----------------|
   * |---------------BWP----------------|
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;


  CcBwpCreator::SimpleOperationBandConf bandConf (frequency, bandwidth, numCcPerBand, scenarioEnum);
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
  //Initialize channel and pathloss, plus other things inside band.
  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});

  // Configure beamforming method
  idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  if (enableQuasiOmni)
  {
    idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (QuasiOmniDirectPathBeamforming::GetTypeId ()));
  }

  // Configure scheduler
  nrHelper->SetSchedulerTypeId (NrMacSchedulerTdmaRR::GetTypeId ());

  // Antennas for the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (numRowsUe));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (numColumnsUe));
  nrHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (isoUe));

  // Antennas for the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (numRowsGnb));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (numColumnsGnb));
  nrHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (isoGnb));

  // install nr net devices
  NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice(gnbNodes, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetTxPower (txPower);
  nrHelper->GetGnbPhy (gnbNetDev.Get (1), 0)->SetTxPower (txPower);

  // When all the configuration is done, explicitly call UpdateConfig ()
  for (auto it = gnbNetDev.Begin (); it != gnbNetDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  // create the internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost 
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // connect a remoteHost to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);

  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      UdpServerHelper dlPacketSinkHelper (dlPort);
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(1)));
      //dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(10));
      dlClient.SetAttribute ("PacketSize", UintegerValue (1500));
      clientApps.Add (dlClient.Install (remoteHost));
    }

  // attach UEs to the closest gNB
  nrHelper->AttachToClosestEnb (ueNetDev, gnbNetDev);

  // start server and client apps
  serverApps.Start(Seconds(0.4));
  clientApps.Start(Seconds(0.4));
  serverApps.Stop(Seconds(simTime));
  clientApps.Stop(Seconds(simTime-0.2));

  // enable the traces provided by the nr module
  nrHelper->EnableTraces();

  //Let us create the REM for this user:
  Ptr<NetDevice> ueRemDevice = ueNetDev.Get(0);
  //Radio Environment Map Generation for ccId 0
  Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> (bandwidth, frequency, numerology);
  remHelper->SetMinX (-20.0);
  remHelper->SetMaxX (20.0);
  remHelper->SetResX (50);
  remHelper->SetMinY (-20.0);
  remHelper->SetMaxY (20.0);
  remHelper->SetResY (50);
  remHelper->SetZ (1.5);
  remHelper->CreateRem (gnbNetDev, ueRemDevice, 0);  //bwpId 0



  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


