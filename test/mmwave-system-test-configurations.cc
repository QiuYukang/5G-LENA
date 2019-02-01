/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2017 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *   Author: Biljana Bojovic <bbojovic@cttc.es>
 */


// Include a header file from your module to test.
#include "ns3/mmwave-helper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include "ns3/log.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"



// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;

/**
 * \file mmwave-system-test-configurations.cc
 * \ingroup test
 * \brief Test the configuration for 5G-LENA
 */

// This is an example TestCase.
class MmwaveSystemTestConfigurationsTestCase1 : public TestCase
{
public:
  MmwaveSystemTestConfigurationsTestCase1 (std::string name, uint32_t numerology,
                                           std::string scheduler);
  virtual ~MmwaveSystemTestConfigurationsTestCase1 ();

private:
  virtual void DoRun (void);

  uint32_t m_numerology;
  std::string m_scheduler;
};


MmwaveSystemTestConfigurationsTestCase1::MmwaveSystemTestConfigurationsTestCase1 (std::string name, uint32_t numerology,
                                                                                  std::string scheduler)
: TestCase (name)
{

  m_numerology = numerology;
  m_scheduler = scheduler;
}


MmwaveSystemTestConfigurationsTestCase1::~MmwaveSystemTestConfigurationsTestCase1 ()
{
}

void
MmwaveSystemTestConfigurationsTestCase1::DoRun (void)
{

  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("a"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::OptionalNlos", BooleanValue(false));
  // important to set frequency into the 3gpp model
  Config::SetDefault ("ns3::AntennaArrayModel::AntennaHorizontalSpacing", DoubleValue(0.5));
  Config::SetDefault ("ns3::AntennaArrayModel::AntennaVerticalSpacing", DoubleValue(0.5));
  Config::SetDefault ("ns3::MmWave3gppChannel::UpdatePeriod", TimeValue(MilliSeconds(100))); // interval after which the channel for a moving user is updated,
  // with spatial consistency procedure. If 0, spatial consistency is not used
  Config::SetDefault ("ns3::MmWave3gppChannel::CellScan", BooleanValue(false)); // Set true to use cell scanning method, false to use the default power method.
  Config::SetDefault ("ns3::MmWave3gppChannel::Blockage", BooleanValue(true)); // use blockage or not
  Config::SetDefault ("ns3::MmWave3gppChannel::PortraitMode", BooleanValue(true)); // use blockage model with UT in portrait mode
  Config::SetDefault ("ns3::MmWave3gppChannel::NumNonselfBlocking", IntegerValue(4)); // number of non-self blocking obstacles
  Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));

  // default 28e9
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Frequency", DoubleValue(28e9));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue(28e9)); // check MmWavePhyMacCommon for other PHY layer parameters

  // changing parameters in frequency domain
  Config::SetDefault("ns3::MmWavePhyMacCommon::Numerology", UintegerValue(m_numerology));// set numerology, and set full frame structure just with this single parameter

  // set mobile device and base station antenna heights in meters, according to the chosen scenario
  double hBS; //base station antenna height in meters;
  double hUT; //user antenna height in meters;

  hBS = 35;
  hUT = 1.5;

  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();
  mmWaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
  mmWaveHelper->SetSchedulerType (m_scheduler);

  mmWaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));
  Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmWaveHelper->SetEpcHelper (epcHelper);
  mmWaveHelper->Initialize();

  // create base stations and mobile terminals
  NodeContainer enbNode;
  NodeContainer ueNode;
  enbNode.Create (1);
  ueNode.Create (1);

  // position the base stations
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, hBS));

  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator(enbPositionAlloc);
  enbmobility.Install (enbNode);

  // position the mobile terminals and enable the mobility
  MobilityHelper uemobility;
  uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  uemobility.Install (ueNode);

  ueNode.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, 10, hUT));

  // install mmWave net devices
  NetDeviceContainer enbNetDev = mmWaveHelper->InstallEnbDevice (enbNode);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNode);

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
  internet.Install (ueNode);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  Ptr<Node> ue = ueNode.Get (0);
  // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverApps.Add (dlPacketSinkHelper.Install (ueNode.Get(0)));

  UdpClientHelper dlClient (ueIpIface.GetAddress (0), dlPort);
  dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds(10000)));
  dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
  clientApps.Add (dlClient.Install (remoteHost));

  // start server and client apps
  serverApps.Start(Seconds(0.4));
  clientApps.Start(Seconds(0.4));
  serverApps.Stop(Seconds(1));
  clientApps.Stop(Seconds(1));

  // attach UEs to the closest eNB
  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  Simulator::Stop (Seconds (1));
  Simulator::Run ();
  Simulator::Destroy ();

  // A wide variety of test macros are available in src/core/test.h
  NS_TEST_ASSERT_MSG_EQ (true, true, "true doesn't equal true for some reason");
  // Use this one for floating point comparisons
  NS_TEST_ASSERT_MSG_EQ_TOL (0.01, 0.01, 0.001, "Numbers are not equal within tolerance");
}


class MmwaveSystemTestConfigurationsTestSuite : public TestSuite
{
public:
  MmwaveSystemTestConfigurationsTestSuite ();

};

MmwaveSystemTestConfigurationsTestSuite::MmwaveSystemTestConfigurationsTestSuite ()
: TestSuite ("mmwave-system-test-configurations", SYSTEM)
{
  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=0, scheduler=rr", 0, "ns3::MmWaveMacSchedulerTdmaRR"), QUICK);
  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=2, scheduler=rr", 2, "ns3::MmWaveMacSchedulerTdmaRR"), QUICK);
  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=4, scheduler=rr", 4, "ns3::MmWaveMacSchedulerTdmaRR"), QUICK);

  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=0, scheduler=pf", 0, "ns3::MmWaveMacSchedulerTdmaPF"), QUICK);
  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=2, scheduler=pf", 2, "ns3::MmWaveMacSchedulerTdmaPF"), QUICK);
  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=4, scheduler=pf", 4, "ns3::MmWaveMacSchedulerTdmaPF"), QUICK);

  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=0, scheduler=mr", 0, "ns3::MmWaveMacSchedulerTdmaMR"), QUICK);
  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=2, scheduler=mr", 2, "ns3::MmWaveMacSchedulerTdmaMR"), QUICK);
  AddTestCase (new MmwaveSystemTestConfigurationsTestCase1 ("num=4, scheduler=mr", 4, "ns3::MmWaveMacSchedulerTdmaMR"), QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static MmwaveSystemTestConfigurationsTestSuite mmwaveTestSuite;

