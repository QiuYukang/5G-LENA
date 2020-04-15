/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


/**
 * \ingroup examples
 * \file lte-s3-scenario.cc
 * \brief original Run original LTE to compare performance with LTE in NR
 *
 * This example configures an LTE scenario using original LTE protocol stack and
 * other classes from LTE module.
 *
 *
 * In this example, each sector operates in a separate band.
 *
 * Each cell of three-sector eNb will have the following spectrum division:
 *
 *  Sector 1     Sector 2    Sector 3
 * |---Band1---|---Band2---|---Band3---|
 * |----CC1----|----CC2----|----CC3----|
 * |----BWP1---|----BWP2---|----BWP3---|
 *
 *
 *
 */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/radio-environment-map-helper.h>
#include <ns3/nr-module.h>
#include <ns3/grid-scenario-helper.h>
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include <iomanip>
#include <string>
#include <vector>

using namespace ns3;
using std::vector;

int
main (int argc, char *argv[])
{
  CommandLine cmd;

  uint16_t numOuterRings = 0;
  uint16_t ueNumPerEnb = 1;

  bool logging = false;

  std::string scenario = "UMi";
  double txPower = 46; //dBm
  double uetxPower = 20; //dBm
  uint32_t bandwidthBandDl = 100; // 18MHz
  uint32_t bandwidthBandUl = 100; //

  uint32_t simTimeMs = 1400;
  uint32_t udpAppStartTimeMs = 400;
  std::string direction = "UL";

  std::string pathlossModel = "";

  // Traffic parameters (that we will use inside this script:)
  uint32_t udpPacketSize = 1252;
  uint32_t lambda = 10000;

  // Where we will store the output files.
  std::string simTag = "default";
  std::string outputDir = "./";


  cmd.AddValue ("scenario",
                "The urban scenario string (UMa or UMi)",
                 scenario);
  // We scenario UMa or Umi so that we can use HexagonalGridScenarioHelper for positions, etc,
  // but, we want to be able to use not only Uma/Umi that are defined in the HexagonalGridScenarioHelper, so we add this additional parameter
  cmd.AddValue ("numRings",
                "The number of rings around the central site",
                 numOuterRings);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per cell or gNB in multiple-ue topology",
                 ueNumPerEnb);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.AddValue ("packetSize",
                "packet size in bytes to be used by UE traffic",
                udpPacketSize);
  cmd.AddValue ("lambda",
                "Number of UDP packets generated in one second per UE",
                lambda);
  cmd.AddValue ("direction",
                "The flow direction (DL or UL)",
                 direction);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);
  cmd.Parse (argc, argv);

  /*
  *  An example of how the spectrum is being used.
  *
  *                              centralEarfcnFrequencyBand = 300
  *                                     |
  *         100 RB                    100 RB                 100RB
  * |-----------------------|-----------------------|-----------------------|
  *
  *      50RB      50RB         50RB        50RB        50RB       50RB
  * |-----------|-----------|-----------|-----------|-----------|-----------|
  *       DL          UL          DL         UL           DL         UL
  *
  * |-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
  *     fc_dl       fc_ul       fc_dl       fc_ul        fc_dl      fc_ul
  */

  uint32_t centralFrequencyBand1Dl = 100;
  uint32_t centralFrequencyBand1Ul = 200;
  uint32_t centralFrequencyBand2Dl = 300;
  uint32_t centralFrequencyBand2Ul = 400;
  uint32_t centralFrequencyBand3Dl = 500;
  uint32_t centralFrequencyBand3Ul = 600;

  if (scenario == "UMa")
    {
      txPower = 49;
      pathlossModel = "ns3::ThreeGppUmaPropagationLossModel";
    }
  else if (scenario == "UMi")
    {
      txPower = 44;
      pathlossModel = "ns3::ThreeGppUmiStreetCanyonPropagationLossModel";
    }
/*  else
    {
      txPower = 46;
      pathlossModel = "ns3::FriisPropagationLossModel";
    }*/

  if (logging)
    {
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
      LogComponentEnable ("LteSpectrumValueHelper", LOG_LEVEL_INFO);
    }

  HexagonalGridScenarioHelper gridScenario;
  gridScenario.SetNumRings (numOuterRings);
  gridScenario.SetScenarioParamenters (scenario);
  gridScenario.SetNumCells ();  // Note that the call takes no arguments since the number is obtained from the parameters in SetUMaParameters or SetUMiParameters
  uint16_t gNbNum = gridScenario.GetNumCells ();
  uint32_t ueNum = ueNumPerEnb * gNbNum;
  gridScenario.SetUtNumber (ueNum);
  gridScenario.CreateScenario ();  //!< Creates and plots the network deployment

  const uint16_t ffr = 3; // Fractional Frequency Reuse scheme to mitigate intra-site inter-sector interferences

  NodeContainer enbSector1Container, enbSector2Container, enbSector3Container;
  for (uint32_t j = 0; j < gridScenario.GetBaseStations ().GetN (); ++j)
    {
      Ptr<Node> enb = gridScenario.GetBaseStations ().Get (j);
      switch (j % ffr)
      {
        case 0:
          enbSector1Container.Add (enb);
          break;
        case 1:
          enbSector2Container.Add (enb);
          break;
        case 2:
          enbSector3Container.Add (enb);
          break;
        default:
          NS_ABORT_MSG("ffr param cannot be larger than 3");
          break;
      }
    }

  /*
    * Create two different NodeContainer for the different traffic type.
    * In ueLowLat we will put the UEs that will receive low-latency traffic.
    */
   NodeContainer ueSector1Container, ueSector2Container, ueSector3Container;

   for (uint32_t j = 0; j < gridScenario.GetUserTerminals ().GetN (); ++j)
     {
       Ptr<Node> ue = gridScenario.GetUserTerminals ().Get (j);
       switch (j % ffr)
       {
         case 0:
           ueSector1Container.Add (ue);
           break;
         case 1:
           ueSector2Container.Add (ue);
           break;
         case 2:
           ueSector3Container.Add (ue);
           break;
         default:
           NS_ABORT_MSG("ffr param cannot be larger than 3");
           break;
       }
     }

  Ptr < LteHelper > lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  // ALL SECTORS AND BANDS configuration
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (txPower));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (uetxPower));
  lteHelper->SetAttribute ("PathlossModel", StringValue (pathlossModel)); // for each band the same pathloss model
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
  lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
  lteHelper->SetEnbAntennaModelAttribute ("Beamwidth", DoubleValue (120));
  lteHelper->SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidthBandDl));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidthBandUl));

  //SECTOR 1 eNB configuration
  double orientationDegrees = gridScenario.GetAntennaOrientationDegrees (0, gridScenario.GetNumSectorsPerSite ());
  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (orientationDegrees));
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (centralFrequencyBand1Dl));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (centralFrequencyBand1Ul));
  NetDeviceContainer enbSector1NetDev = lteHelper->InstallEnbDevice (enbSector1Container);

  //SECTOR 2 eNB configuration
  orientationDegrees = gridScenario.GetAntennaOrientationDegrees (1, gridScenario.GetNumSectorsPerSite ());
  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (orientationDegrees));
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (centralFrequencyBand2Dl));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (centralFrequencyBand2Ul));
  NetDeviceContainer enbSector2NetDev = lteHelper->InstallEnbDevice (enbSector2Container);

  //SECTOR 3 eNB configuration
  orientationDegrees = gridScenario.GetAntennaOrientationDegrees (2, gridScenario.GetNumSectorsPerSite ());
  lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (orientationDegrees));
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (centralFrequencyBand3Dl));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (centralFrequencyBand3Ul));
  NetDeviceContainer enbSector3NetDev = lteHelper->InstallEnbDevice (enbSector3Container);

  NetDeviceContainer ueSector1NetDev = lteHelper->InstallUeDevice(ueSector1Container);
  NetDeviceContainer ueSector2NetDev = lteHelper->InstallUeDevice(ueSector2Container);
  NetDeviceContainer ueSector3NetDev = lteHelper->InstallUeDevice(ueSector3Container);


  // copy- pasted from s3-scenario -> TODO unify this

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

  Ipv4InterfaceContainer ueSector1IpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueSector1NetDev));
  Ipv4InterfaceContainer ueSector2IpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueSector2NetDev));
  Ipv4InterfaceContainer ueSector3IpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueSector3NetDev));

  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < gridScenario.GetUserTerminals ().GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (gridScenario.GetUserTerminals ().Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
   }

  // attach UEs to their gNB. Try to attach them per cellId order
  for (uint32_t u = 0; u < ueNum; ++u)
   {
     uint32_t sector = u % ffr;
     uint32_t i = u / ffr;
        if (sector == 0)
          {
            Ptr<NetDevice> enbNetDev = enbSector1NetDev.Get (i % gridScenario.GetNumSites ());
            Ptr<NetDevice> ueNetDev = ueSector1NetDev.Get (i);
            lteHelper->Attach (ueNetDev, enbNetDev);
            if (logging == true)
              {
                Vector enbpos = enbNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
                Vector uepos = ueNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
                double distance = CalculateDistance (enbpos, uepos);
                std::cout << "Distance = " << distance << " meters" << std::endl;
              }
          }
        else if (sector == 1)
          {
            Ptr<NetDevice> enbNetDev = enbSector2NetDev.Get (i % gridScenario.GetNumSites ());
            Ptr<NetDevice> ueNetDev = ueSector2NetDev.Get (i);
            lteHelper->Attach (ueNetDev, enbNetDev);
            if (logging == true)
              {
                Vector enbpos = enbNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
                Vector uepos = ueNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
                double distance = CalculateDistance (enbpos, uepos);
                std::cout << "Distance = " << distance << " meters" << std::endl;
              }
          }
        else if (sector == 2)
          {
            Ptr<NetDevice> enbNetDev = enbSector3NetDev.Get (i % gridScenario.GetNumSites ());
            Ptr<NetDevice> ueNetDev = ueSector3NetDev.Get (i);
            lteHelper->Attach (ueNetDev, enbNetDev);
            if (logging == true)
              {
                Vector enbpos = enbNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
                Vector uepos = ueNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
                double distance = CalculateDistance (enbpos, uepos);
                std::cout << "Distance = " << distance << " meters" << std::endl;
              }
          }
        else
          {
            NS_ABORT_MSG("Number of sector cannot be larger than 3");
          }
      }


  /*
    * Traffic part. Install two kind of traffic: low-latency and voice, each
    * identified by a particular source port.
    */
   uint16_t dlPortLowLat = 1234;

   ApplicationContainer serverApps;

   // The sink will always listen to the specified ports
   UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);

   // The server, that is the application which is listening, is installed in the UE
   if (direction == "DL")
     {
       serverApps.Add (dlPacketSinkLowLat.Install ({ueSector1Container,ueSector2Container,ueSector3Container}));
     }
   else
     {
       serverApps.Add (dlPacketSinkLowLat.Install (remoteHost));
     }

   /*
    * Configure attributes for the different generators, using user-provided
    * parameters for generating a CBR traffic
    *
    * Low-Latency configuration and object creation:
    */
   UdpClientHelper dlClientLowLat;
   dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
   dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
   dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSize));
   dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0/lambda)));

   // The bearer that will carry low latency traffic
   EpsBearer lowLatBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);

   // The filter for the low-latency traffic
   Ptr<EpcTft> lowLatTft = Create<EpcTft> ();
   EpcTft::PacketFilter dlpfLowLat;
   if (direction == "DL")
     {
       dlpfLowLat.localPortStart = dlPortLowLat;
       dlpfLowLat.localPortEnd = dlPortLowLat;
       dlpfLowLat.direction = EpcTft::DOWNLINK;
     }
   else
     {
       dlpfLowLat.remotePortStart = dlPortLowLat;
       dlpfLowLat.remotePortEnd = dlPortLowLat;
       dlpfLowLat.direction = EpcTft::UPLINK;
       }
   lowLatTft->Add (dlpfLowLat);

   /*
    * Let's install the applications!
    */
   ApplicationContainer clientApps;

   for (uint32_t i = 0; i < ueSector1Container.GetN (); ++i)
     {
       Ptr<Node> ue = ueSector1Container.Get (i);
       Ptr<NetDevice> ueDevice = ueSector1NetDev.Get(i);
       Address ueAddress = ueSector1IpIface.GetAddress (i);

       // The client, who is transmitting, is installed in the remote host,
       // with destination address set to the address of the UE
       if (direction == "DL")
         {
           dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
           clientApps.Add (dlClientLowLat.Install (remoteHost));
         }
       else
         {
           dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (remoteHostAddr));
           clientApps.Add (dlClientLowLat.Install (ue));
         }
       // Activate a dedicated bearer for the traffic type
       lteHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
     }

   for (uint32_t i = 0; i < ueSector2Container.GetN (); ++i)
     {
       Ptr<Node> ue = ueSector2Container.Get (i);
       Ptr<NetDevice> ueDevice = ueSector2NetDev.Get(i);
       Address ueAddress = ueSector2IpIface.GetAddress (i);

       // The client, who is transmitting, is installed in the remote host,
       // with destination address set to the address of the UE
       if (direction == "DL")
         {
           dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
           clientApps.Add (dlClientLowLat.Install (remoteHost));
         }
       else
         {
           dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (remoteHostAddr));
           clientApps.Add (dlClientLowLat.Install (ue));
         }
       // Activate a dedicated bearer for the traffic type
       lteHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
     }

   for (uint32_t i = 0; i < ueSector3Container.GetN (); ++i)
     {
       Ptr<Node> ue = ueSector3Container.Get (i);
       Ptr<NetDevice> ueDevice = ueSector3NetDev.Get(i);
       Address ueAddress = ueSector3IpIface.GetAddress (i);

       // The client, who is transmitting, is installed in the remote host,
       // with destination address set to the address of the UE
       if (direction == "DL")
         {
           dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
           clientApps.Add (dlClientLowLat.Install (remoteHost));
         }
       else
         {
           dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (remoteHostAddr));
           clientApps.Add (dlClientLowLat.Install (ue));
         }
       // Activate a dedicated bearer for the traffic type
       lteHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
     }

   // start UDP server and client apps
   serverApps.Start(MilliSeconds(udpAppStartTimeMs));
   clientApps.Start(MilliSeconds(udpAppStartTimeMs));
   serverApps.Stop(MilliSeconds(simTimeMs));
   clientApps.Stop(MilliSeconds(simTimeMs));

   // enable the traces provided by the mmWave module
   lteHelper->EnableTraces ();


   FlowMonitorHelper flowmonHelper;
   NodeContainer endpointNodes;
   endpointNodes.Add (remoteHost);
   endpointNodes.Add (gridScenario.GetUserTerminals ());

   Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
   monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
   monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
   monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));


   /*  Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
     remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
     remHelper->SetAttribute ("OutputFile", StringValue ("rem.out"));
     remHelper->SetAttribute ("XMin", DoubleValue (-2000.0));
     remHelper->SetAttribute ("XMax", DoubleValue (+2000.0));
     remHelper->SetAttribute ("YMin", DoubleValue (-500.0));
     remHelper->SetAttribute ("YMax", DoubleValue (+3500.0));
     remHelper->SetAttribute ("Z", DoubleValue (1.5));
     remHelper->Install ();*/

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
