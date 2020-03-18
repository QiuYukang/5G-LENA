/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

/**
 * \ingroup examples
 * \file cttc-nr-demo.cc
 * \brief A cozy, simple, NR demo (in a tutorial style)
 *
 * This example describes how to setup a simulation using the 3GPP channel model
 * from TR 38.900. This example consists of a simple grid topology, in which you
 * can choose the number of gNbs and UEs. Have a look at the possible parameters
 * to know what you can configure through the command line.
 *
 * The example will print on-screen the end-to-end result of one (or two) flows,
 * as well as writing them on a file.
 *
 * \code{.unparsed}
$ ./waf --run "cttc-nr-demo --Help"
    \endcode
 *
 */

/*
 * Include part. Often, you will have to include the headers for an entire module;
 * do that by including the name of the module you need with the suffix "-module.h".
 */

#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-module.h"
#include "ns3/config-store-module.h"

/*
 * To be able to use LOG_* functions.
 */
#include "ns3/log.h"

/*
 * Use, always, the namespace ns3. All the NR classes are inside such namespace.
 */
using namespace ns3;

/*
 * With this line, we will be able to see the logs of the file by enabling the
 * component "CttcNrDemo", in this way:
 *
 * $ export NS_LOG="CttcNrDemo=level_info|prefix_func|prefix_time"
 */
NS_LOG_COMPONENT_DEFINE ("CttcNrDemo");

int 
main (int argc, char *argv[])
{
  /*
   * Variables that represent the parameters we will accept as input by the
   * command line. Each of them is initialized with a default value.
   */
  // Scenario parameters (that we will use inside this script):
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 2;
  bool logging = false;
  bool singleBwp = false;

  // Traffic parameters (that we will use inside this script:)
  bool udpFullBuffer = false;
  uint32_t udpPacketSizeUll = 100;
  uint32_t udpPacketSizeBe = 1252;
  uint32_t lambdaUll = 10000;
  uint32_t lambdaBe = 10000;

  // Simulation parameters. Please don't use double to indicate seconds, use
  // milliseconds and integers to avoid representation errors.
  uint32_t simTimeMs = 1000;
  uint32_t udpAppStartTimeMs = 400;

  // NR parameters. We will take the input from the command line, and then we
  // will pass them inside the NR module.
  int32_t fixedMcs = -1;
  bool cellScan = false;
  double beamSearchAngleStep = 10.0;
  uint16_t numerologyBwp1 = 4;
  double frequencyBwp1 = 28e9;
  double bandwidthBwp1 = 100e6;
  uint16_t numerologyBwp2 = 2;
  double frequencyBwp2 = 28.2e9;
  double bandwidthBwp2 = 100e6;
  double totalTxPower = 4;

  // Where we will store the output files.
  std::string simTag = "default";
  std::string outputDir = "./";

  /*
   * From here, we instruct the ns3::CommandLine class of all the input parameters
   * that we may accept as input, as well as their description, and the storage
   * variable.
   */
  CommandLine cmd;

  cmd.AddValue ("simTimeMs",
                "Simulation time",
                simTimeMs);
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


  // Parse the command line
  cmd.Parse (argc, argv);

  /*
   * Check if the frequency is in the allowed range.
   * If you need to add other checks, here is the best position to put them.
   */
  NS_ABORT_IF (frequencyBwp1 < 6e9 || frequencyBwp1 > 100e9);
  NS_ABORT_IF (frequencyBwp2 < 6e9 || frequencyBwp2 > 100e9);

  /*
   * If the logging variable is set to true, enable the log of some components
   * through the code. The same effect can be obtained through the use
   * of the NS_LOG environment variable:
   *
   * export NS_LOG="UdpClient=level_info|prefix_time|prefix_func|prefix_node:UdpServer=..."
   *
   * Usually, the environment variable way is preferred, as it is more customizable,
   * and more expressive.
   */
  if (logging)
    {
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
    }

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  /*
   * Create the scenario. In our examples, we heavily use helpers that setup
   * the gnbs and ue following a pre-defined pattern. Please have a look at the
   * GridScenarioHelper documentation to see how the nodes will be distributed.
   */
  GridScenarioHelper gridScenario;
  gridScenario.SetRows (1);
  gridScenario.SetColumns (gNbNum);
  gridScenario.SetHorizontalBsDistance (5.0);
  gridScenario.SetBsHeight (10.0);
  gridScenario.SetUtHeight (1.5);
  gridScenario.SetBsNumber (gNbNum);
  gridScenario.SetUtNumber (ueNumPergNb * gNbNum);
  gridScenario.SetScenarioHeight (3); // Create a 3x3 scenario where the UE will
  gridScenario.SetScenarioLength (3); // be distribuited.
  gridScenario.CreateScenario ();

  /*
   * Setup the NR module. We create the various helpers needed for the
   * NR simulation:
   * - EpcHelper, which will setup the core network
   * - IdealBeamformingHelper, which takes care of the beamforming part
   * - MmWaveHelper, which takes care of creating and connecting the various
   * part of the NR stack
   */
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();

  // Put the pointers inside mmWaveHelper
  mmWaveHelper->SetIdealBeamformingHelper (idealBeamformingHelper);
  mmWaveHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum division. We create two operational bands, each of them containing
   * one bandwidth part centered at the frequency specified by the input parameters.
   * Each spectrum part length is, as well, specified by the input parameters.
   * Both operational bands will use the StreetCanyon channel modeling.
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;

  // Create the configuration for the CcBwpHelper
  CcBwpCreator::SimpleOperationBandConf bandConf1 (frequencyBwp1, bandwidthBwp1, 1, BandwidthPartInfo::UMi_StreetCanyon);
  CcBwpCreator::SimpleOperationBandConf bandConf2 (frequencyBwp2, bandwidthBwp2, 1, BandwidthPartInfo::UMi_StreetCanyon);

  // By using the configuration created, it is time to make the operation bands
  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);
  OperationBandInfo band2 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf2);

  /*
   * Attributes of ThreeGppChannelModel still cannot be set in our way.
   * TODO: Coordinate with Tommaso
   */
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds(0)));

  mmWaveHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  mmWaveHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  /*
   * Initialize channel and pathloss, plus other things inside band1. If needed,
   * the band configuration can be done manually, but we leave it for more
   * sophisticated examples. For the moment, this method will take care
   * of all the spectrum initialization needs.
   */
  mmWaveHelper->InitializeOperationBand (&band1);

  /*
   * Start to account for the bandwidth used by the example, as well as
   * the total power that has to be divided among the bwp.
   */
  double x = pow (10, totalTxPower/10);
  double totalBandwidth = bandwidthBwp1;

  /*
   * if not single BWP simulation, initialize and setup power in the second bwp
   */
  if (!singleBwp)
    {
      // Initialize channel and pathloss, plus other things inside band2
      mmWaveHelper->InitializeOperationBand (&band2);
      totalBandwidth += bandwidthBwp2;
      allBwps = CcBwpCreator::GetAllBwps ({band1, band2});
    }
  else
    {
      allBwps = CcBwpCreator::GetAllBwps ({band1});
    }

  /*
   * allBwps contains all the spectrum configuration needed for the mmWaveHelper.
   *
   * Now, we can setup the attributes. We can have three kind of attributes:
   * (i) parameters that are valid for all the bandwidth parts and applies to
   * all nodes, (ii) parameters that are valid for all the bandwidth parts
   * and applies to some node only, and (iii) parameters that are different for
   * every bandwidth parts. The approach is:
   *
   * - for (i): Configure the attribute through the helper, and then install;
   * - for (ii): Configure the attribute through the helper, and then install
   * for the first set of nodes. Then, change the attribute through the helper,
   * and install again;
   * - for (iii): Install, and then configure the attributes by retrieving
   * the pointer needed, and calling "SetAttribute" on top of such pointer.
   *
   */

  Packet::EnableChecking ();
  Packet::EnablePrinting ();

  /*
   *  Case (i): Attributes valid for all the nodes
   */
  // Beamforming method
  idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for all the UEs
  mmWaveHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  mmWaveHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  mmWaveHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (true));

  // Antennas for all the gNbs
  mmWaveHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  mmWaveHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  mmWaveHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (true));

  uint32_t bwpIdForLowLat = 0;
  uint32_t bwpIdForVoice = 0;
  if (! singleBwp)
    {
      bwpIdForVoice = 1;
      bwpIdForLowLat = 0;
    }

  // gNb routing between Bearer and bandwidh part
  mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));

  // Ue routing between Bearer and bandwidth part
  mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));


  /*
   * We miss many other parameters. By default, not configuring them is equivalent
   * to use the default values. Please, have a look at the documentation to see
   * what are the default values for all the attributes you are not seeing here.
   */

  /*
   * Case (ii): Attributes valid for a subset of the nodes
   */

  // NOT PRESENT IN THIS SIMPLE EXAMPLE

  /*
   * We have configured the attributes we needed. Now, install and get the pointers
   * to the NetDevices, which contains all the NR stack:
   */

  NetDeviceContainer enbNetDev = mmWaveHelper->InstallGnbDevice (gridScenario.GetBaseStations (), allBwps);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (gridScenario.GetUserTerminals (), allBwps);

  /*
   * Case (iii): Go node for node and change the attributes we have to setup
   * per-node.
   */

  // Get the first netdevice (enbNetDev.Get (0)) and the first bandwidth part (0)
  // and set the attribute.
  mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp1));
  mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue (10*log10 ((bandwidthBwp1/totalBandwidth) * x)));

  if (!singleBwp)
    {
      // Get the first netdevice (enbNetDev.Get (0)) and the second bandwidth part (1)
      // and set the attribute.
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (numerologyBwp2));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetTxPower (10*log10 ((bandwidthBwp2/totalBandwidth) * x));
    }

  // When all the configuration is done, explicitly call UpdateConfig ()

  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
    {
      DynamicCast<MmWaveEnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<MmWaveUeNetDevice> (*it)->UpdateConfig ();
    }

  // From here, it is standard NS3. In the future, we will create helpers
  // for this part as well.

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
  internet.Install (gridScenario.GetUserTerminals ());
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < gridScenario.GetUserTerminals ().GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (gridScenario.GetUserTerminals ().Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to the closest eNB
  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  uint16_t dlPortLowLat = 1234;
  uint16_t dlPortVoice = 1235;

  UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);
  UdpServerHelper dlPacketSinkVoice (dlPortVoice);

  ApplicationContainer clientApps, serverApps;

  // Install the sinks in all the UEs.
  serverApps.Add (dlPacketSinkLowLat.Install (gridScenario.GetUserTerminals ()));
  serverApps.Add (dlPacketSinkVoice.Install (gridScenario.GetUserTerminals ()));

  // configure here UDP traffic
  for (uint32_t j = 0; j < gridScenario.GetUserTerminals ().GetN(); ++j)
    {
      uint16_t dlPort = dlPortVoice;
      bool isLowLat = false;

      if (j % 2 == 0)
        {
          isLowLat = true;
          dlPort = dlPortLowLat;
        }

      UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
      dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      //dlClient.SetAttribute ("MaxPackets", UintegerValue(1));

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

      if (isLowLat)
        {
          dlClient.SetAttribute ("PacketSize", UintegerValue(udpPacketSizeUll));
          dlClient.SetAttribute ("Interval", TimeValue (Seconds(1.0/lambdaUll)));
        }
      else
        {
          dlClient.SetAttribute ("PacketSize", UintegerValue(udpPacketSizeBe));
          dlClient.SetAttribute ("Interval", TimeValue (Seconds(1.0/lambdaBe)));
        }

      clientApps.Add (dlClient.Install (remoteHost));


      Ptr<EpcTft> tft = Create<EpcTft> ();
      EpcTft::PacketFilter dlpf;
      dlpf.localPortStart = dlPort;
      dlpf.localPortEnd = dlPort;
      tft->Add (dlpf);

      enum EpsBearer::Qci q;

      if (isLowLat)
        {
          q = EpsBearer::NGBR_LOW_LAT_EMBB;
        }
      else
        {
          q = EpsBearer::GBR_CONV_VOICE;
        }

      EpsBearer bearer (q);
      mmWaveHelper->ActivateDedicatedEpsBearer(ueNetDev.Get(j), bearer, tft);
    }
  // start UDP server and client apps
  serverApps.Start(MilliSeconds(udpAppStartTimeMs));
  clientApps.Start(MilliSeconds(udpAppStartTimeMs));
  serverApps.Stop(MilliSeconds(simTimeMs));
  clientApps.Stop(MilliSeconds(simTimeMs));

  // enable the traces provided by the mmWave module
  //mmWaveHelper->EnableTraces();


  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (gridScenario.GetUserTerminals ());

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Simulator::Stop (MilliSeconds (simTimeMs));
  Simulator::Run ();

  /*
   * To check what was installed in the memory, i.e., BWPs of eNb Device, and its configuration.
   * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> MmWaveEnbPhy -> Numerology,
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
      outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / ((simTimeMs - udpAppStartTimeMs) / 1000.0) / 1000.0 / 1000.0  << " Mbps\n";
      outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          //double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
          double rxDuration = (simTimeMs - udpAppStartTimeMs) / 1000.0;

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


