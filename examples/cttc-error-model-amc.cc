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

/**
 * \file cttc-error-model-amc.cc
 * \ingroup examples
 * \brief Error model example with adaptive modulation and coding: 1 gNB and 1 UE, multiple packets with non-varying fading conditions.
 *
 * This example allows the user to test the end-to-end performance with the new
 * NR PHY abstraction model for error modeling by using adaptive modulation and coding (AMC).
 * It allows the user to set the gNB-UE distance, the MCS table, the error model type, and the HARQ method.
 *
 * For example, the HARQ method can be configured by setting e.g.
 * "--harqMethod=HarqIr" for HARQ-IR. The MCS table can be toggled by the argument,
 * e.g. "--eesmTable=1" for MCS Table1.
 * The NR error model can be set as "--errorModel=ns3::NrEesmErrorModel",
 * while "--errorModel=ns3::NrLteMiErrorModel" configures the LTE error model.
 * The AMC model defaults to the Error-model based AMC, but can
 * be changed through to use the Shannon-based model, through the AmcModel attribute, manually.
 *
 * The scenario consists of a single gNB and a single UE, placed at positions (0.0, 0.0, 10), and
 * (0.0, ueY, 1.5), respectively. ueY can be configured by the user, e.g. "ueY=20", and defaults
 * to 30 m.
 *
 * By default, the program uses the 3GPP channel model, Urban Micro scenario, without shadowing and with
 * probabilistic line of sight / non-line of sight option. The program runs for
 * 5 seconds and one packet is transmitted every 200 ms from gNB to UE (donwlink direction).
 * The packet size can be configured by using the following parameter: "--packetSize=1000".
 * There are no channel updates (the channel update period is 0 ms), so that we allow
 * for proper MCS adaptation.
 *
 * This simulation prints the output to the terminal. The output statistics are
 * averaged among all the transmitted packets.
 *
 * To run the simulation with the default configuration one shall run the
 * following in the command line:
 *
 * ./waf --run cttc-error-model-amc
 *
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcErrorModelAmcExample");


static Ptr<ListPositionAllocator>
GetGnbPositions(double gNbHeight = 10.0)
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
  // ASSUMING ONE UE

  SeqTsHeader seqTs;
  pkt->PeekHeader (seqTs);
  packetsTime.push_back ((Simulator::Now () - seqTs.GetTs()).GetMicroSeconds ());
}

int
main (int argc, char *argv[])
{
  const uint8_t gNbNum = 1;
  const uint8_t ueNum = 1;
  double totalTxPower = 4;
  uint16_t numerologyBwp = 4;
  double centralFrequencyBand = 28e9;
  double bandwidthBand = 100e6;
  double ueY = 30.0;

  double simTime = 5.0; // 5 seconds: to give time AMC to stabilize
  uint32_t pktSize = 500;
  Time udpAppStartTime = MilliSeconds (1000);
  Time packetInterval = MilliSeconds (200);
  Time updateChannelInterval = MilliSeconds (0);  // no channel updates to test AMC
  uint32_t packets = (simTime - udpAppStartTime.GetSeconds ()) / packetInterval.GetSeconds ();
  NS_ABORT_IF (packets == 0);


  std::string errorModel = "ns3::NrEesmErrorModel";
  uint32_t eesmTable = 1;
  std::string harqMethod = "HarqIr";

  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue("errorModelType",
               "Error model type: ns3::NrEesmErrorModel , ns3::NrLteMiErrorModel",
               errorModel);
  cmd.AddValue("eesmTable",
               "Table to use when error model is Eesm (1 for McsTable1 or 2 for McsTable2)",
               eesmTable);
  cmd.AddValue("ueY",
               "Y position of any UE",
               ueY);
  cmd.AddValue("pktSize",
               "Packet Size",
               pktSize);
  cmd.AddValue("harqMethod",
               "The HARQ method to be used in case of Eesm (HarqCc or HarqIr)",
               harqMethod);

  cmd.Parse (argc, argv);

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize",
                      UintegerValue(999999999));

  /*
   * TODO: remove all the instances of SetDefault, NrEesmErrorModel, NrAmc
   */
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
  Config::SetDefault("ns3::NrAmc::AmcModel", EnumValue (NrAmc::ErrorModel));  // NrAmc::ShannonModel or NrAmc::ErrorModel

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10.0;
  double ueHeight = 1.5;

  gNbNodes.Create (gNbNum);
  ueNodes.Create (ueNum);

  Ptr<ListPositionAllocator> gNbPositionAlloc = GetGnbPositions(gNbHeight);
  Ptr<ListPositionAllocator> uePositionAlloc = GetUePositions(ueY, ueHeight);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gNbPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (uePositionAlloc);
  mobility.Install (ueNodes);

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
   * Spectrum division. We create one operational band, with one CC, and the CC with a single bandwidth part.
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;

  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequencyBand, bandwidthBand, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  /*
   * Attributes of ThreeGppChannelModel still cannot be set in our way.
   * TODO: Coordinate with Tommaso
   */
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (updateChannelInterval));
  mmWaveHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  mmWaveHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  /*
   * Initialize channel and pathloss, plus other things inside band.
   */
  mmWaveHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});

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

  // Scheduler
  mmWaveHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue(false));
  mmWaveHelper->SetSchedulerAttribute ("FixedMcsUl", BooleanValue(false));

  // Error Model
  mmWaveHelper->SetGnbSpectrumAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));
  mmWaveHelper->SetUeSpectrumAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));


  uint32_t bwpId = 0;

  // gNb routing between Bearer and bandwidh part
  mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpId));

  // Ue routing between Bearer and bandwidth part
  mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpId));

  NetDeviceContainer gnbNetDev = mmWaveHelper->InstallGnbDevice (gNbNodes, allBwps);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNodes, allBwps);

  /*
   * Case (iii): Go node for node and change the attributes we have to setup
   * per-node.
   */

  // Get the first netdevice (enbNetDev.Get (0)) and the first bandwidth part (0)
  // and set the attribute.
  mmWaveHelper->GetEnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp));
  mmWaveHelper->GetEnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue (totalTxPower));

  // When all the configuration is done, explicitly call UpdateConfig ()

  for (auto it = gnbNetDev.Begin (); it != gnbNetDev.End (); ++it)
    {
      DynamicCast<MmWaveEnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<MmWaveUeNetDevice> (*it)->UpdateConfig ();
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
  mmWaveHelper->AttachToClosestEnb (ueNetDev, gnbNetDev);

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
        }
    }
  std::cerr << "Packets received: " << packetsTime.size () << std::endl;
  std::cerr << "Counter (packets not affected by reordering): " << +cont << std::endl;

  if (packetsTime.size () > 0 && cont > 0)
    {
      std::cerr << "Average e2e latency (over all received packets): " << sum / packetsTime.size () << " us" << std::endl;
      std::cerr << "Average e2e latency (over counter): " << sum / cont << " us" << std::endl;
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
