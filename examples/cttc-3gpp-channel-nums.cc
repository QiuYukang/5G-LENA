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
 *
 *   Author:  Biljana Bojovic <bbojovic@cttc.es>

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
#include "ns3/log.h"


/**
 * \file cttc-3gpp-channel-nums.cc
 * \ingroup examples
 * \brief Simple topology numerologies example.
 *
 * This example allows users to configure the numerology and test the end-to-end
 * performance for different numerologies. In the following figure we illustrate the simulation setup.
 *
 * For example, UDP interval can be configured by setting
 * "--udpInterval=0.001". The numerology can be toggled by the argument,
 * e.g. "--numerology=1". Additionally, in this example two arguments
 * are added "bandwidth" and "frequency". The modulation scheme of
 * this example is in test mode, and it is fixed to 28.
 *
 * By default, the program uses the 3GPP channel model, without shadowing and with
 * line of sight ('l') option. The program runs for 0.4 seconds and one single packet
 * is to be transmitted. The packet size can be configured by using  the
 * following parameter: "--packetSize=1000".
 *
 * This simulation prints the output to the terminal and also to the file which
 * is named by default "cttc-3gpp-channel-nums-fdm-output" and which is by
 * default placed in the root directory of the project.
 *
 * To run the simulation with the default configuration one shall run the
 * following in the command line:
 *
 * ./waf --run cttc-3gpp-channel-nums
 *
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("3gppChannelNumerologiesExample");

static ns3::GlobalValue g_frequency("frequency",
                                    "The system frequency",
                                     ns3::DoubleValue(28e9),
                                     ns3::MakeDoubleChecker<double>(6e9,100e9));//!< Global variable used to configure the frequency. It is accessible as "--frequency" from CommandLine.

static ns3::GlobalValue g_bandwidth("bandwidth",
                                    "The system bandwidth",
                                     ns3::DoubleValue(200e6),
                                     ns3::MakeDoubleChecker<double>()); //!< Global variable used to configure the bandwidth. It is accessible as "--bandwidth" from CommandLine.

static ns3::GlobalValue g_numerology ("numerology",
                                      "The default 3GPP NR numerology to be used",
                                      ns3::UintegerValue (0),
                                      ns3::MakeUintegerChecker<uint32_t>());//!< Global variable used to configure the numerology. It is accessible as "--numerology" from CommandLine.

static ns3::GlobalValue g_udpInterval ("udpInterval",
                                      "Udp interval for UDP application packet arrival, in seconds",
                                      ns3::DoubleValue (0.001),
                                      ns3::MakeDoubleChecker<double>());//!< Global variable used to configure the UDP packet interval. It is accessible as "--udpInterval" from CommandLine.

static ns3::GlobalValue g_udpPacketSize ("udpPacketSize",
                                         "Udp packet size in bytes",
                                         ns3::UintegerValue (1000),
                                         ns3::MakeUintegerChecker<uint32_t>()); //!< Global variable used to configure the UDP packet size. It is accessible as "--udpPacketSize" from CommandLine.

static ns3::GlobalValue g_udpRate ("udpFullBuffer",
                                   "Whether to set the full buffer traffic; if this parameter is set then the udpInterval parameter"
                                   "will be neglected.",
                                   ns3::BooleanValue (true),
                                   ns3::MakeBooleanChecker()); //!< Global variable used to configure whether the traffic is the full buffer traffic. It is accessible as "--udpFullBuffer" from CommandLine.

static ns3::GlobalValue g_singleUeTopology ("singleUeTopology",
                                            "When true the example uses a single UE topology, when false use topology with variable number of UEs"
                                            "will be neglected.",
                                            ns3::BooleanValue (true),
                                            ns3::MakeBooleanChecker()); //!< Global variable used to configure whether topology is with single of various number of UEs. It is accessible as "--singleUeTopology" from CommandLine.

static ns3::GlobalValue g_useFixedMcs ("useFixedMcs",
                                       "Whether to use fixed mcs, normally used for testing purposes",
                                        ns3::BooleanValue (true),
                                        ns3::MakeBooleanChecker()); //!< Global variable used to configure whether to use fixed MCS. It is accessible as "--useFixedMcs" from CommandLine.

static ns3::GlobalValue g_fixedMcs ("fixedMcs",
                                    "The MCS that will be used in this example",
                                    ns3::UintegerValue (28),
                                    ns3::MakeUintegerChecker<uint32_t>()); //!< Global variable used to configure fixed MCS. It is accessible as "--fixedMcs" from CommandLine.

static ns3::GlobalValue g_gNbNum ("gNbNum",
                                  "The number of gNbs in multiple-ue topology",
                                   ns3::UintegerValue (1),
                                   ns3::MakeUintegerChecker<uint32_t>());//!< Global variable used to configure the number of gNbs in multi-UE topology. It is accessible as "--gNbNum" from CommandLine.

static ns3::GlobalValue g_ueNum ("ueNumPergNb",
                                  "The number of UE per gNb in multiple-ue topology",
                                  ns3::UintegerValue (1),
                                  ns3::MakeUintegerChecker<uint32_t>()); //!< Global variable used to configure the number of UEs in multi-UE topology. It is accessible as "--ueNumPergNb" from CommandLine.

static ns3::GlobalValue g_cellScan ("cellScan",
                                    "Use beam search method to determine beamforming vector, the default is long-term covariance matrix method"
                                    "true to use cell scanning method, false to use the default power method.",
                                    ns3::BooleanValue (false),
                                    ns3::MakeBooleanChecker()); //!< Global variable used to configure whether to use Beam Search of Long-Term Cov. matrix for beamforming. It is accessible as "--cellScan" from CommandLine.

static ns3::GlobalValue g_beamSearchAngleStep ("beamSearchAngleStep",
                                               "Beam search angle step for beam search method",
                                               ns3::DoubleValue (10),
                                               ns3::MakeDoubleChecker<double>()); //!< Global variable used to configure beam search angle step in the case that beam search method is used. It is accessible as "--beamSearchAngleStep" from CommandLine.

static ns3::GlobalValue g_txPower ("txPower",
                                   "Tx power",
                                    ns3::DoubleValue (1),
                                    ns3::MakeDoubleChecker<double>()); //!< Global variable used to configure gNb TX power. It is accessible as "--txPower" from CommandLine.

static ns3::GlobalValue g_simTag ("simTag",
                                  "tag to be appended to output filenames to distinguish simulation campaigns",
                                  ns3::StringValue ("default"),
                                  ns3::MakeStringChecker ()); //!< Global variable used to configure simulation output tag that helps distinguishing different simulation campaigns. It is accessible as "--simTag" from CommandLine.

static ns3::GlobalValue g_outputDir ("outputDir",
                                     "directory where to store simulation results",
                                     ns3::StringValue ("./"),
                                     ns3::MakeStringChecker ());  //!< Global variable used to configure simulation output folder. It is accessible as "--outputDir" from CommandLine.

int 
main (int argc, char *argv[])
{

    CommandLine cmd;
    cmd.Parse (argc, argv);
    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults ();
    // parse again so you can override input file default values via command line
    cmd.Parse (argc, argv);

  // enable logging or not
  bool logging = false;
  if(logging)
    {
      LogComponentEnable ("MmWave3gppPropagationLossModel", LOG_LEVEL_ALL);
      LogComponentEnable ("MmWave3gppBuildingsPropagationLossModel", LOG_LEVEL_ALL);
      LogComponentEnable ("MmWave3gppChannel", LOG_LEVEL_ALL);
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);

    }

  // set simulation time and mobility
  double simTime = 1; // seconds
  double udpAppStartTime = 0.4; //seconds
  //double speed = 1; // 1 m/s for walking UT.

  // parse the command line options
  BooleanValue booleanValue;
  StringValue stringValue;
  IntegerValue integerValue;
  UintegerValue uintegerValue;
  DoubleValue doubleValue;
  GlobalValue::GetValueByName("numerology", uintegerValue); // use optional NLOS equation
  uint16_t numerology = uintegerValue.Get();
  GlobalValue::GetValueByName("fixedMcs", uintegerValue); // use optional NLOS equation
  uint16_t fixedMcs = uintegerValue.Get();
  GlobalValue::GetValueByName("gNbNum", uintegerValue); // use optional NLOS equation
  uint16_t gNbNum = uintegerValue.Get();
  GlobalValue::GetValueByName("ueNumPergNb", uintegerValue); // use optional NLOS equation
  uint16_t ueNumPergNb = uintegerValue.Get();
  GlobalValue::GetValueByName("udpInterval", doubleValue); // use optional NLOS equation
  double udpInterval = doubleValue.Get();
  GlobalValue::GetValueByName("udpPacketSize", uintegerValue); // use optional NLOS equation
  uint32_t udpPacketSize = uintegerValue.Get();
  GlobalValue::GetValueByName("frequency", doubleValue); //
  double frequency = doubleValue.Get();
  GlobalValue::GetValueByName("udpFullBuffer", booleanValue); //
  bool udpFullBuffer = booleanValue.Get();
  GlobalValue::GetValueByName("singleUeTopology", booleanValue); //
  bool singleUeTopology = booleanValue.Get();
  GlobalValue::GetValueByName("bandwidth", doubleValue); //
  double bandwidth = doubleValue.Get();
  GlobalValue::GetValueByName("cellScan", booleanValue); //
  bool cellScan = booleanValue.Get();
  GlobalValue::GetValueByName("useFixedMcs", booleanValue); //
  bool useFixedMcs = booleanValue.Get();
  GlobalValue::GetValueByName("beamSearchAngleStep", doubleValue); // use optional NLOS equation
  double beamSearchAngleStep = doubleValue.Get();
  GlobalValue::GetValueByName("txPower", doubleValue); // use optional NLOS equation
  double txPower = doubleValue.Get();
  GlobalValue::GetValueByName ("simTag", stringValue);
  std::string simTag = stringValue.Get ();
  GlobalValue::GetValueByName ("outputDir", stringValue);
  std::string outputDir = stringValue.Get ();

  // attributes that can be set for this channel model
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Frequency", DoubleValue(frequency));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("l"));

  if (singleUeTopology)
    {
    //Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon"));
      Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("RMa"));
    }
  else
    {
      Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("InH-OfficeOpen"));
    }

  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(false));

  Config::SetDefault ("ns3::MmWave3gppChannel::CellScan", BooleanValue(cellScan));
  Config::SetDefault ("ns3::MmWave3gppChannel::BeamSearchAngleStep", DoubleValue(beamSearchAngleStep));

  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue(frequency));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::Bandwidth", DoubleValue(bandwidth));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::Numerology", UintegerValue(numerology));

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::FixedMcsDl", BooleanValue (useFixedMcs));
  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::McsDefaultDl", UintegerValue (fixedMcs));

  //Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNum", UintegerValue (4));
  //Config::SetDefault("ns3::MmWaveEnbNetDevice::AntennaNum", UintegerValue (16));

  Config::SetDefault("ns3::MmWaveEnbPhy::TxPower", DoubleValue (txPower));

  // setup the mmWave simulation
  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> (); 
  mmWaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
  mmWaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));

  Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmWaveHelper->SetEpcHelper (epcHelper);
  mmWaveHelper->Initialize();

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10;
  double ueHeight = 1.5;

  if (singleUeTopology)
    {
      gNbNodes.Create (1);
      ueNodes.Create (1);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (gNbNodes);
      mobility.Install (ueNodes);
      gNbNodes.Get(0)->GetObject<MobilityModel>()->SetPosition (Vector (0.0, 0.0, gNbHeight));
      ueNodes.Get(0)->GetObject<MobilityModel> ()->SetPosition (Vector (0.0, 30.0 , ueHeight));
    }
  else
    {
      gNbNodes.Create (gNbNum);
      ueNodes.Create (ueNumPergNb * gNbNum);

      MobilityHelper mobility;
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
                  staPositionAlloc->Add (Vector (xValue, 1, ueHeight));
                }
              else
                {
                  staPositionAlloc->Add (Vector (xValue, -1, ueHeight));
                }
            }
        }

      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.SetPositionAllocator (apPositionAlloc);
      mobility.Install (gNbNodes);

      mobility.SetPositionAllocator (staPositionAlloc);
      mobility.Install (ueNodes);
    }

  // install mmWave net devices
  NetDeviceContainer enbNetDev = mmWaveHelper->InstallEnbDevice (gNbNodes);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNodes);

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

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(0)));

  for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
    {
      UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
      dlClient.SetAttribute("PacketSize", UintegerValue(udpPacketSize));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      //dlClient.SetAttribute ("MaxPackets", UintegerValue(1000));

      if (udpFullBuffer)
        {
          double bitRate = 75000000; // 75 Mb/s will saturate the system of 20 MHz

          if (bandwidth > 20e6)
            {
              bitRate *=  bandwidth / 20e6;
            }
          udpInterval = static_cast<double> (udpPacketSize * 8) / bitRate ;
        }
      dlClient.SetAttribute ("Interval", TimeValue (Seconds(udpInterval)));
      clientApps.Add (dlClient.Install (remoteHost));
      
      Ptr<EpcTft> tft = Create<EpcTft> ();
      EpcTft::PacketFilter dlpf;
      dlpf.localPortStart = dlPort;
      dlpf.localPortEnd = dlPort;
      dlPort++;
      tft->Add (dlpf);

      enum EpsBearer::Qci q;
      q = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearer (q);
      mmWaveHelper->ActivateDedicatedEpsBearer(ueNetDev.Get(j), bearer, tft);
    }

  // start server and client apps
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



  // Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  double averageFlowThroughput = 0.0;
  double averageFlowDelay = 0.0;

  std::ofstream outFile;
  std::string filename = outputDir + "/" + simTag;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::app);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
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
          double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();

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
          outFile << "  Mean upt:  0  Mbps \n";
          outFile << "  Mean jitter: 0 ms\n";
        }
      outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
    }

  outFile << "\n\n  Mean flow throughput: " << averageFlowThroughput / stats.size() << "\n";
  outFile << "  Mean flow delay: " << averageFlowDelay / stats.size () << "\n";
  outFile.close ();

  Ptr<UdpClient> clientApp = clientApps.Get(0)->GetObject<UdpClient>();
  Ptr<UdpServer> serverApp = serverApps.Get(0)->GetObject<UdpServer>();
  std::cout<<"\n Total UDP throughput (bps):"<<(serverApp->GetReceived()*udpPacketSize*8)/(simTime-udpAppStartTime)<<std::endl;

  Simulator::Destroy ();
  return 0;
}


