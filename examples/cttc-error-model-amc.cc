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
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nr-module.h"

#include <chrono>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcErrorModelExample");


static Ptr<ListPositionAllocator>
GetGnbPositions(uint32_t gnbNum, double gNbHeight = 10.0)
{
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator> ();
  pos->Add (Vector (0.0, 0.0, gNbHeight));
  return pos;
}

static Ptr<ListPositionAllocator>
GetUePositions(double ueY, double ueHeight = 1.5)
{
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator> ();
  pos->Add (Vector (0.0, ueY, ueHeight));

  return pos;
}

static std::vector<uint64_t> packetsTime;

static void
PrintRxPkt (std::string context, Ptr<const Packet> pkt)
{
  NS_UNUSED(context);
  // ASSUMING ONE UE!!!!

  SeqTsHeader seqTs;
  pkt->PeekHeader (seqTs);
  packetsTime.push_back ((Simulator::Now () - seqTs.GetTs()).GetMicroSeconds ());
}

int
main (int argc, char *argv[])
{
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 1;
  bool cellScan = false;
  double beamSearchAngleStep = 10.0;
  double totalTxPower = 4;
  uint16_t numerologyBwp1 = 4;
  double frequencyBwp1 = 28e9;
  double bandwidthBwp1 = 100e6;
  double ueY = 30.0;

  double simTime = 5.0; // 50.0; // seconds
  uint32_t pktSize = 500;
  Time udpAppStartTime = MilliSeconds (1000);
  Time packetInterval = MilliSeconds (200);
  Time updateChannelInterval = MilliSeconds(0); //MilliSeconds (150);
  uint32_t packets = (simTime - udpAppStartTime.GetSeconds ()) / packetInterval.GetSeconds ();
  NS_ABORT_IF (packets == 0);


  std::string errorModel = "ns3::NrEesmErrorModel";
  uint32_t eesmTable = 1;
  std::string harqMethod = "HarqCc";

  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
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
  cmd.AddValue("errorModelType",
               "Error model type: ns3::NrEesmErrorModel , ns3::NrLteMiErrorModel",
               errorModel);
  cmd.AddValue("eesmTable",
               "Table to use when error model is Eesm (1 for McsTable1 or 2 for McsTable2)",
               eesmTable);
  cmd.AddValue("harqMethod",
               "The HARQ method to be used in case of Eesm (NrEesmErrorModel::HarqCc or NrEesmErrorModel::HarqIr)",
               harqMethod);
  cmd.AddValue("ueY",
               "Y position of any UE",
               ueY);
  cmd.AddValue("pktSize",
               "Packet Size",
               pktSize);

  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition",
                      StringValue("a"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario",
                      StringValue("UMi-StreetCanyon")); // with antenna height of 10 m
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing",
                      BooleanValue(false));

  Config::SetDefault ("ns3::MmWave3gppChannel::CellScan",
                      BooleanValue(cellScan));
  Config::SetDefault ("ns3::MmWave3gppChannel::BeamSearchAngleStep",
                      DoubleValue(beamSearchAngleStep));
  Config::SetDefault ("ns3::MmWave3gppChannel::UpdatePeriod",
                      TimeValue (updateChannelInterval));

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize",
                      UintegerValue(999999999));
  Config::SetDefault ("ns3::MmWaveHelper::NumberOfComponentCarriers", UintegerValue (1));

  Config::SetDefault("ns3::PointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds(0)));
  Config::SetDefault("ns3::PointToPointEpcHelper::X2LinkDelay", TimeValue (MilliSeconds(0)));

  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::FixedMcsDl", BooleanValue(false));
  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::FixedMcsUl", BooleanValue(false));

  if (harqMethod == "HarqCc")
    {
      Config::SetDefault("ns3::NrEesmErrorModel::HarqMethod", EnumValue (NrEesmErrorModel::HarqCc));
    }
  else if (harqMethod == "HarqIr")
    {
      Config::SetDefault("ns3::NrEesmErrorModel::HarqMethod", EnumValue (NrEesmErrorModel::HarqIr));
    }
  else
    {
      NS_FATAL_ERROR ("HARQ method not valid, you set " << harqMethod);
    }
  //Config::SetDefault("ns3::NrEesmErrorModel::HarqMethod", EnumValue (NrEesmErrorModel::HarqCc));

  if (eesmTable == 1)
    {
      Config::SetDefault("ns3::NrEesmErrorModel::McsTable", EnumValue (NrEesmErrorModel::McsTable1));
    }
  else if (eesmTable == 2)
    {
      Config::SetDefault("ns3::NrEesmErrorModel::McsTable", EnumValue (NrEesmErrorModel::McsTable2));
    }
  else
    {
      NS_FATAL_ERROR ("Valid tables are 1 or 2, you set " << eesmTable);
    }

  Config::SetDefault("ns3::NrAmc::ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));
  Config::SetDefault("ns3::NrAmc::AmcModel", EnumValue (NrAmc::ErrorModel));  // NrAmc::PiroEW2010 or NrAmc::ErrorModel

  Config::SetDefault("ns3::MmWaveSpectrumPhy::ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));


  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10;
  double ueHeight = 1.5;

  gNbNodes.Create (gNbNum);
  ueNodes.Create (ueNumPergNb * gNbNum);

  Ptr<ListPositionAllocator> apPositionAlloc = GetGnbPositions(gNbNum, gNbHeight);
  Ptr<ListPositionAllocator> staPositionAlloc = GetUePositions(ueY, ueHeight);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (apPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (staPositionAlloc);
  mobility.Install (ueNodes);

  // setup the mmWave simulation
  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();
  mmWaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
  mmWaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));

  Ptr<MmWavePhyMacCommon> phyMacCommonBwp1 = CreateObject<MmWavePhyMacCommon>();
  phyMacCommonBwp1->SetCentreFrequency(frequencyBwp1);
  phyMacCommonBwp1->SetBandwidth (bandwidthBwp1);
  phyMacCommonBwp1->SetNumerology(numerologyBwp1);
  phyMacCommonBwp1->SetAttribute ("MacSchedulerType", TypeIdValue (MmWaveMacSchedulerTdmaRR::GetTypeId ()));
  phyMacCommonBwp1->SetCcId(0);

  BandwidthPartRepresentation repr1 (0, phyMacCommonBwp1, nullptr, nullptr, nullptr);
  mmWaveHelper->AddBandwidthPart(0, repr1);

  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
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
      dlClient.SetAttribute ("MaxPackets", UintegerValue(packets));
      dlClient.SetAttribute("PacketSize", UintegerValue(pktSize));
      dlClient.SetAttribute ("Interval", TimeValue (packetInterval));

      clientApps.Add (dlClient.Install (remoteHost));
    }

  for (uint32_t j = 0; j < serverApps.GetN (); ++j)
    {
      Ptr<UdpServer> client = DynamicCast<UdpServer> (serverApps.Get (j));
      NS_ASSERT(client != nullptr);
      std::stringstream ss;
      ss << j;
      client->TraceConnect("Rx", ss.str(), MakeCallback (&PrintRxPkt));
    }

  // start UDP server and client apps
  serverApps.Start(udpAppStartTime);
  clientApps.Start(udpAppStartTime);
  serverApps.Stop(Seconds(simTime));
  clientApps.Stop(Seconds(simTime));

  // attach UEs to the closest eNB
  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // enable the traces provided by the mmWave module
  //mmWaveHelper->EnableTraces();

  Simulator::Stop (Seconds (simTime));

  auto start = std::chrono::steady_clock::now();

  Simulator::Run ();

  auto end = std::chrono::steady_clock::now();


  uint64_t sum = 0;
  uint32_t cont = 0;
  for (auto & v : packetsTime)
    {
      if ( v < 100000 )
        {
          sum += v;
          cont++;
          //std::cerr << "Packet latency: " << v << std::endl;
        }
    }
  std::cerr << "Packets received: " << packetsTime.size () << std::endl;
  std::cerr << "Counter: " << +cont << std::endl;

  if (packetsTime.size () > 0)
    {
      //std::cerr << "Average e2e latency: " << sum / packetsTime.size () << " us" << std::endl;
      std::cerr << "Average e2e latency: " << sum / cont << " us" << std::endl;

    }
  else
    {
      std::cerr << "Average e2e latency: Not Available" << std::endl;
    }


  for (auto it = serverApps.Begin(); it != serverApps.End(); ++it)
    {
      uint64_t recv = DynamicCast<UdpServer> (*it)->GetReceived ();
      std::cerr << "Sent: " << packets << " Recv: " << recv << " Lost: "
                << packets - recv << " pkts, ( "
                << (static_cast<double> (packets - recv) / packets) * 100.0
                << " % )" << std::endl;
    }


  Simulator::Destroy ();

  std::cerr << "Running time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
            << " s." << std::endl;
  return 0;
}


