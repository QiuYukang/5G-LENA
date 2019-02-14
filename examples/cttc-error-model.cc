/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nr-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcErrorModelExample");


static Ptr<ListPositionAllocator>
GetGnbPositions(uint32_t gnbNum, double gNbHeight = 10.0)
{
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator> ();
  double yValue = 0.0;

  for (uint32_t i = 1; i <= gnbNum; ++i)
    {
      // 2.0, -2.0, 6.0, -6.0, 10.0, -10.0, ....
      if (i % 2 != 0)
        {
          yValue = static_cast<int>(i) * 30;
        }
      else
        {
          yValue = -yValue;
        }

      pos->Add (Vector (0.0, yValue, gNbHeight));
    }

  return pos;
}

static Ptr<ListPositionAllocator>
GetUePositions(uint32_t gnbNum, uint32_t ueNumPergNb,
               double ueHeight = 1.5)
{
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator> ();
  // 1.0, -1.0, 3.0, -3.0, 5.0, -5.0, ...
  double xValue = 0.0;
  double yValue = 0.0;
  for (uint32_t i = 1; i <= gnbNum; ++i)
    {
      if (i % 2 != 0)
        {
          yValue = static_cast<int>(i) * 30;
        }
      else
        {
          yValue = -yValue;
        }

      for (uint32_t j = 1; j <= ueNumPergNb; ++j)
        {
          if (j % 2 != 0)
            {
              xValue = j;
            }
          else
            {
              xValue = -xValue;
            }

          if (yValue > 0)
            {
              pos->Add (Vector (xValue, 10, ueHeight));
            }
          else
            {
              pos->Add (Vector (xValue, -10, ueHeight));
            }
        }
    }

  return pos;
}

int
main (int argc, char *argv[])
{
  enum Mode
  {
    DELAY,
    THROUGHPUT
  };

  uint32_t mcs = 13;
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 1;
  bool cellScan = false;
  double beamSearchAngleStep = 10.0;
  double totalTxPower = 4;
  uint16_t numerologyBwp1 = 4;
  double frequencyBwp1 = 28e9;
  double bandwidthBwp1 = 100e6;

  double simTime = 10; // seconds
  double udpAppStartTime = 1.0; //seconds

  uint32_t mode = DELAY;

  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("mcs",
                "The MCS that will be used in this example",
                mcs);
  cmd.AddValue ("gNbNum",
                "The number of gNbs in multiple-ue topology",
                gNbNum);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per gNb in multiple-ue topology",
                ueNumPergNb);
  cmd.AddValue ("cellScan",
                "Use beam search method to determine beamforming vector,"
                " the default is long-term covariance matrix method"
                " true to use cell scanning method, false to use the default"
                " power method.",
                cellScan);
  cmd.AddValue ("beamSearchAngleStep",
                "Beam search angle step for beam search method",
                beamSearchAngleStep);
  cmd.AddValue ("totalTxPower",
                "total tx power that will be proportionally assigned to"
                " bandwidth parts depending on each BWP bandwidth ",
                totalTxPower);
  cmd.AddValue("mode",
               "Mode: 0 for DELAY, 1 for THROUGHPUT",
               mode);

  cmd.Parse (argc, argv);

  if (mode == DELAY)
    {
      LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
    }

  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition",
                      StringValue("l"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario",
                      StringValue("UMi-StreetCanyon")); // with antenna height of 10 m
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing",
                      BooleanValue(false));

  Config::SetDefault ("ns3::MmWave3gppChannel::CellScan",
                      BooleanValue(cellScan));
  Config::SetDefault ("ns3::MmWave3gppChannel::BeamSearchAngleStep",
                      DoubleValue(beamSearchAngleStep));

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize",
                      UintegerValue(999999999));
  Config::SetDefault ("ns3::MmWaveHelper::NumberOfComponentCarriers", UintegerValue (1));

  Config::SetDefault("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds(0)));
  Config::SetDefault("ns3::MmWavePointToPointEpcHelper::X2LinkDelay", TimeValue (MilliSeconds(0)));

  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::FixedMcsDl", BooleanValue(true));
  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::FixedMcsUl", BooleanValue(true));
  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::StartingMcsDl", UintegerValue (mcs));
  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::StartingMcsUl", UintegerValue (mcs));

  Config::SetDefault("ns3::NrAmc::ErrorModelType", TypeIdValue (NrEesmErrorModel::GetTypeId()));
  Config::SetDefault("ns3::NrAmc::AmcModel", EnumValue (NrAmc::PiroEW2010));

  Config::SetDefault("ns3::MmWaveSpectrumPhy::ErrorModelType", TypeIdValue (NrEesmErrorModel::GetTypeId()));

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10;
  double ueHeight = 1.5;

  gNbNodes.Create (gNbNum);
  ueNodes.Create (ueNumPergNb * gNbNum);

  Ptr<ListPositionAllocator> apPositionAlloc = GetGnbPositions(gNbNum, gNbHeight);
  Ptr<ListPositionAllocator> staPositionAlloc = GetUePositions(gNbNum, ueNumPergNb,
                                                               ueHeight);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (apPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (staPositionAlloc);
  mobility.Install (ueNodes);

  // setup the mmWave simulation
  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();
  mmWaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
  mmWaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));

  Ptr<BandwidthPartsPhyMacConf> bwpConf = CreateObject <BandwidthPartsPhyMacConf> ();

  Ptr<MmWavePhyMacCommon> phyMacCommonBwp1 = CreateObject<MmWavePhyMacCommon>();
  phyMacCommonBwp1->SetCentreFrequency(frequencyBwp1);
  phyMacCommonBwp1->SetBandwidth (bandwidthBwp1);
  phyMacCommonBwp1->SetNumerology(numerologyBwp1);
  phyMacCommonBwp1->SetAttribute ("MacSchedulerType", TypeIdValue (MmWaveMacSchedulerTdmaRR::GetTypeId ()));
  phyMacCommonBwp1->SetCcId(0);

  bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp1);

  mmWaveHelper->SetBandwidthPartMap (bwpConf);

  Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmWaveHelper->SetEpcHelper (epcHelper);
  mmWaveHelper->Initialize();

  // install mmWave net devices
  NetDeviceContainer enbNetDev = mmWaveHelper->InstallEnbDevice (gNbNodes);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNodes);

  double x = pow(10, totalTxPower/10);

  double totalBandwidth = bandwidthBwp1;

  for (uint32_t j = 0; j < enbNetDev.GetN(); ++j)
    {
      ObjectMapValue objectMapValue;
      Ptr<MmWaveEnbNetDevice> netDevice = DynamicCast<MmWaveEnbNetDevice>(enbNetDev.Get(j));
      netDevice->GetAttribute("ComponentCarrierMap", objectMapValue);
      for (uint32_t i = 0; i < objectMapValue.GetN(); i++)
        {
          Ptr<ComponentCarrierGnb> bandwidthPart = DynamicCast<ComponentCarrierGnb>(objectMapValue.Get(i));
          if (i==0)
            {
              bandwidthPart->GetPhy()->SetTxPower(10*log10((bandwidthBwp1/totalBandwidth)*x));
            }

          else
            {
              NS_FATAL_ERROR ("\n Please extend power assignment for additional bandwidht parts...");
            }
        }
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
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps, serverApps;

  ApplicationContainer clientAppsEmbb, serverAppsEmbb;

  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverApps.Add (dlPacketSinkHelper.Install (ueNodes));

  // configure here UDP traffic
  for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
    {
      UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
      if (mode == DELAY)
        {
          dlClient.SetAttribute ("MaxPackets", UintegerValue(10));
          dlClient.SetAttribute("PacketSize", UintegerValue(500));
          dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(200)));
        }
      else
        {
          dlClient.SetAttribute ("MaxPackets", UintegerValue(8000));
          dlClient.SetAttribute("PacketSize", UintegerValue(1000));
          dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(1)));
        }

      clientApps.Add (dlClient.Install (remoteHost));
    }

  // start UDP server and client apps
  serverApps.Start(Seconds(udpAppStartTime));
  clientApps.Start(Seconds(udpAppStartTime));
  serverApps.Stop(Seconds(simTime));
  clientApps.Stop(Seconds(simTime));

  // attach UEs to the closest eNB
  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // enable the traces provided by the mmWave module
  //mmWaveHelper->EnableTraces();

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  for (auto it = serverApps.Begin(); it != serverApps.End(); ++it)
    {
      NS_LOG_UNCOND ("Lost: " << DynamicCast<UdpServer> (*it)->GetLost ());
      uint64_t recv = DynamicCast<UdpServer> (*it)->GetReceived ();
      double throughput = recv * 1000 / 9.0;
      NS_LOG_UNCOND ("Throughput: " << throughput / 1e6 << " Mbps");
    }


  Simulator::Destroy ();
  return 0;
}


