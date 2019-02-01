/*
 -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*-

 *   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *
 *   Author:  Biljana Bojovic <bbojovic@cttc.es>
 *
 */


#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/mmwave-mac-scheduler-tdma-rr.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("3gppChannelFdmBandwidthPartsExample");

int 
main (int argc, char *argv[])
{
  bool udpFullBuffer = false;
  int32_t fixedMcs = -1;
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 2;
  bool cellScan = false;
  double beamSearchAngleStep = 10.0;
  uint16_t numerologyBwp1 = 4;
  double frequencyBwp1 = 28e9;
  double bandwidthBwp1 = 100e6;
  uint16_t numerologyBwp2 = 2;
  double frequencyBwp2 = 28e9;
  double bandwidthBwp2 = 100e6;
  uint32_t udpPacketSizeUll = 100;
  uint32_t udpPacketSizeBe = 1252;
  uint32_t lambdaUll = 10000;
  uint32_t lambdaBe = 1000;
  bool singleBwp = false;
  std::string simTag = "default";
  std::string outputDir = "./";
  double totalTxPower = 4;
  bool logging = false;

  double simTime = 1; // seconds
  double udpAppStartTime = 0.4; //seconds

  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("udpFullBuffer",
                "Whether to set the full buffer traffic; if this parameter is "
                "set then the udpInterval parameter will be neglected.",
                udpFullBuffer);
  cmd.AddValue ("fixedMcs",
                "The MCS that will be used in this example, -1 for auto",
                fixedMcs);
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
  cmd.AddValue ("numerologyBwp1",
                "The numerology to be used in bandwidth part 1",
                numerologyBwp1);
  cmd.AddValue ("frequencyBwp1",
                "The system frequency to be used in bandwidth part 1",
                frequencyBwp1);
  cmd.AddValue ("bandwidthBwp1",
                "The system bandwidth to be used in bandwidth part 1",
                bandwidthBwp1);
  cmd.AddValue ("numerologyBwp2",
                "The numerology to be used in bandwidth part 2",
                numerologyBwp2);
  cmd.AddValue ("frequencyBwp2",
                "The system frequency to be used in bandwidth part 2",
                frequencyBwp2);
  cmd.AddValue ("bandwidthBwp2",
                "The system bandwidth to be used in bandwidth part 2",
                bandwidthBwp2);
  cmd.AddValue ("packetSizeUll",
                "packet size in bytes to be used by ultra low latency traffic",
                udpPacketSizeUll);
  cmd.AddValue ("packetSizeBe",
                "packet size in bytes to be used by best effort traffic",
                udpPacketSizeBe);
  cmd.AddValue ("lambdaUll",
                "Number of UDP packets in one second for ultra low latency traffic",
                lambdaUll);
  cmd.AddValue ("lambdaBe",
                "Number of UDP packets in one second for best effor traffic",
                lambdaBe);
  cmd.AddValue ("singleBwp",
                "Simulate with single BWP, BWP1 configuration will be used",
                singleBwp);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);
  cmd.AddValue ("totalTxPower",
                "total tx power that will be proportionally assigned to"
                " bandwidth parts depending on each BWP bandwidth ",
                totalTxPower);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);


  cmd.Parse (argc, argv);
  NS_ABORT_IF (frequencyBwp1 < 6e9 || frequencyBwp1 > 100e9);
  NS_ABORT_IF (frequencyBwp2 < 6e9 || frequencyBwp2 > 100e9);

  //ConfigStore inputConfig;
  //inputConfig.ConfigureDefaults ();

  // enable logging or not
  if(logging)
    {
      LogComponentEnable ("MmWave3gppPropagationLossModel", LOG_LEVEL_ALL);
      LogComponentEnable ("MmWave3gppBuildingsPropagationLossModel", LOG_LEVEL_ALL);
      LogComponentEnable ("MmWave3gppChannel", LOG_LEVEL_ALL);
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
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

  Config::SetDefault("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds(0)));
  Config::SetDefault("ns3::MmWavePointToPointEpcHelper::X2LinkDelay", TimeValue (MilliSeconds(0)));

  //Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNum", UintegerValue (4));
  //Config::SetDefault("ns3::MmWaveEnbNetDevice::AntennaNum", UintegerValue (16));
  //Config::SetDefault("ns3::MmWaveEnbPhy::TxPower", DoubleValue (txPower));


  if(singleBwp)
    {
      Config::SetDefault ("ns3::MmWaveHelper::NumberOfComponentCarriers", UintegerValue (1));
    }
  else
    {
      Config::SetDefault ("ns3::MmWaveHelper::NumberOfComponentCarriers", UintegerValue (2));
    }

  Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::NGBR_LOW_LAT_EMBB", UintegerValue (0));
  Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::GBR_CONV_VOICE", UintegerValue (1));

  Config::SetDefault ("ns3::MmWaveHelper::EnbComponentCarrierManager", StringValue ("ns3::BwpManagerGnb"));

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10;
  double ueHeight = 1.5;

  gNbNodes.Create (gNbNum);
  ueNodes.Create (ueNumPergNb * gNbNum);

  Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();
  int32_t yValue = 0.0;

  for (uint32_t i = 1; i <= gNbNodes.GetN(); ++i)
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

      apPositionAlloc->Add (Vector (0.0, yValue, gNbHeight));


      // 1.0, -1.0, 3.0, -3.0, 5.0, -5.0, ...
      double xValue = 0.0;
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
              staPositionAlloc->Add (Vector (xValue, 10, ueHeight));
            }
          else
            {
              staPositionAlloc->Add (Vector (xValue, -10, ueHeight));
            }
        }
    }

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


  // if not single BWP simulation add second BWP configuration
  if (!singleBwp)
    {
      Ptr<MmWavePhyMacCommon> phyMacCommonBwp2 = CreateObject<MmWavePhyMacCommon>();
      phyMacCommonBwp2->SetCentreFrequency(frequencyBwp2);
      phyMacCommonBwp2->SetBandwidth (bandwidthBwp2);
      phyMacCommonBwp2->SetNumerology(numerologyBwp2);
      phyMacCommonBwp2->SetCcId(1);
      bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp2);
    }

  mmWaveHelper->SetBandwidthPartMap (bwpConf);


  Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmWaveHelper->SetEpcHelper (epcHelper);
  mmWaveHelper->Initialize();

  // install mmWave net devices
  NetDeviceContainer enbNetDev = mmWaveHelper->InstallEnbDevice (gNbNodes);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNodes);

  double x = pow(10, totalTxPower/10);

  double totalBandwidth = 0;

  if (singleBwp)
    {
      totalBandwidth = bandwidthBwp1;
    }
  else
    {
      totalBandwidth = bandwidthBwp1 + bandwidthBwp2;
    }


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
              std::cout<<"\n txPower1 = "<<10*log10((bandwidthBwp1/totalBandwidth)*x)<<std::endl;
            }
          else if (i==1)
            {
              bandwidthPart->GetPhy()->SetTxPower(10*log10((bandwidthBwp2/totalBandwidth)*x));
              std::cout<<"\n txPower2 = "<<10*log10((bandwidthBwp2/totalBandwidth)*x)<<std::endl;
            }
          else
            {
              std::cout<<"\n Please extend power assignment for additional bandwidht parts...";
            }
        }
      //std::map<uint8_t, Ptr<ComponentCarrierGnb> > ccMap = objectMapValue.GetN()
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
      dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      //dlClient.SetAttribute ("MaxPackets", UintegerValue(10));

      if (udpFullBuffer)
        {
          double bitRate = 75000000; // 75 Mb/s will saturate the system of 20 MHz

          if (bandwidthBwp1 > 20e6)
            {
              bitRate *=  bandwidthBwp1 / 20e6;
            }
          lambdaUll = 1.0 / ((udpPacketSizeUll * 8) / bitRate);


          bitRate = 75000000; // 75 Mb/s will saturate the system of 20 MHz

          if (bandwidthBwp2 > 20e6)
            {
              bitRate *=  bandwidthBwp2 / 20e6;
            }
          lambdaUll = 1.0 / ((udpPacketSizeBe * 8) / bitRate);
        }

      if (j % 2 == 0)
        {
          dlClient.SetAttribute("PacketSize", UintegerValue(udpPacketSizeUll));
          dlClient.SetAttribute ("Interval", TimeValue (Seconds(1.0/lambdaUll)));
        }
      else
        {
          dlClient.SetAttribute("PacketSize", UintegerValue(udpPacketSizeBe));
          dlClient.SetAttribute ("Interval", TimeValue (Seconds(1.0/lambdaBe)));
        }

      clientApps.Add (dlClient.Install (remoteHost));


      Ptr<EpcTft> tft = Create<EpcTft> ();
      EpcTft::PacketFilter dlpf;
      dlpf.localPortStart = dlPort;
      dlpf.localPortEnd = dlPort;
      dlPort++;
      tft->Add (dlpf);

      enum EpsBearer::Qci q;

      if (j % 2 == 0)
        {
          q = EpsBearer::NGBR_LOW_LAT_EMBB;
        }
      else
        {
          q = EpsBearer::GBR_CONV_VOICE;
        }

      //      q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;

      EpsBearer bearer (q);
      mmWaveHelper->ActivateDedicatedEpsBearer(ueNetDev.Get(j), bearer, tft);
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


  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (ueNodes);

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  /*
   * To check what was installed in the memory, i.e., BWPs of eNb Device, and its configuration.
   * Example is: Node 1 -> Device 0 -> ComponentCarrierMap -> {0,1} BWPs -> MmWaveEnbPhy -> MmWavePhyMacCommong-> Numerology, Bandwidth, ...
  GtkConfigStore config;
  config.ConfigureAttributes ();
  */

  // Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  double averageFlowThroughput = 0.0;
  double averageFlowDelay = 0.0;

  std::ofstream outFile;
  std::string filename = outputDir + "/" + simTag;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }

  outFile.setf (std::ios_base::fixed);

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::stringstream protoStream;
      protoStream << (uint16_t) t.protocol;
      if (t.protocol == 6)
        {
          protoStream.str ("TCP");
        }
      if (t.protocol == 17)
        {
          protoStream.str ("UDP");
        }
      outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") proto " << protoStream.str () << "\n";
      outFile << "  Tx Packets: " << i->second.txPackets << "\n";
      outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
      outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / (simTime - udpAppStartTime) / 1000 / 1000  << " Mbps\n";
      outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          //double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
          double rxDuration = (simTime - udpAppStartTime);

          averageFlowThroughput += i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;
          averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

          outFile << "  Throughput: " << i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000  << " Mbps\n";
          outFile << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
          //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
          outFile << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
        }
      else
        {
          outFile << "  Throughput:  0 Mbps\n";
          outFile << "  Mean delay:  0 ms\n";
          outFile << "  Mean jitter: 0 ms\n";
        }
      outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
    }

  outFile << "\n\n  Mean flow throughput: " << averageFlowThroughput / stats.size() << "\n";
  outFile << "  Mean flow delay: " << averageFlowDelay / stats.size () << "\n";

  outFile.close ();

  std::ifstream f (filename.c_str ());

  if (f.is_open())
    {
      std::cout << f.rdbuf();
    }

  Simulator::Destroy ();
  return 0;
}


