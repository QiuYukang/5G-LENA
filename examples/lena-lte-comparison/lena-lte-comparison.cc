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

#include "lena-lte-comparison.h"

#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/lte-module.h"
#include <ns3/radio-environment-map-helper.h>
#include "ns3/config-store-module.h"
#include <ns3/sqlite-output.h>
#include "sinr-output-stats.h"
#include "flow-monitor-output-stats.h"
#include "power-output-stats.h"
#include "slot-output-stats.h"
#include "rb-output-stats.h"
#include "lena-v1-utils.h"
#include "lena-v2-utils.h"

#include <iomanip>

/*
 * To be able to use LOG_* functions.
 */
#include "ns3/log.h"

/*
 * With this line, we will be able to see the logs of the file by enabling the
 * component "LenaLteComparison", in this way:
 *
 * $ export NS_LOG="LenaLteComparison=level_info|prefix_func|prefix_time"
 */
NS_LOG_COMPONENT_DEFINE ("LenaLteComparison");

namespace ns3 {
  
static std::pair<ApplicationContainer, double>
InstallApps (const Ptr<Node> &ue, const Ptr<NetDevice> &ueDevice,
             const Address &ueAddress, const std::string &direction,
             UdpClientHelper *dlClientLowLat, const Ptr<Node> &remoteHost,
             const Ipv4Address &remoteHostAddr, uint32_t udpAppStartTimeMs,
             uint16_t dlPortLowLat, const Ptr<UniformRandomVariable> &x,
             uint32_t appGenerationTimeMs,
             const Ptr<LteHelper> &lteHelper, const Ptr<NrHelper> &nrHelper)
{
  ApplicationContainer app;

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

  // The client, who is transmitting, is installed in the remote host,
  // with destination address set to the address of the UE
  if (direction == "DL")
    {
      dlClientLowLat->SetAttribute ("RemoteAddress", AddressValue (ueAddress));
      app = dlClientLowLat->Install (remoteHost);
    }
  else
    {
      dlClientLowLat->SetAttribute ("RemoteAddress", AddressValue (remoteHostAddr));
      app = dlClientLowLat->Install (ue);
    }

  double startTime = x->GetValue (udpAppStartTimeMs, udpAppStartTimeMs + 10);
  app.Start (MilliSeconds (startTime));
  app.Stop (MilliSeconds (startTime + appGenerationTimeMs));

  std::cout << "\tStarts at time " << MilliSeconds (startTime).GetMilliSeconds () << " ms and ends at "
            << (MilliSeconds (startTime + appGenerationTimeMs)).GetMilliSeconds () << " ms" << std::endl;

  // Activate a dedicated bearer for the traffic type
  if (lteHelper != nullptr)
    {
      lteHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
    }
  else if (nrHelper != nullptr)
    {
      nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
    }
  else
    {
      NS_ABORT_MSG ("Programming error");
    }

  return std::make_pair(app, startTime);
}


bool
Parameters::Validate (void) const
{
  
  NS_ABORT_MSG_IF (bandwidthMHz != 20 && bandwidthMHz != 10 && bandwidthMHz != 5,
                   "Valid bandwidth values are 20, 10, 5, you set " << bandwidthMHz);

  NS_ABORT_MSG_IF (trafficScenario > 2,
                   "Traffic scenario " << trafficScenario << " not valid. Valid values are 0 1 2");

  NS_ABORT_MSG_IF (numerologyBwp > 4,
                   "At most 4 bandwidth parts supported.");
  
  NS_ABORT_MSG_IF (direction != "DL" && direction != "UL",
                   "Flow direction can only be DL or UL");
  NS_ABORT_MSG_IF (operationMode != "TDD" && operationMode != "FDD",
                   "Operation mode can only be TDD or FDD");
  NS_ABORT_MSG_IF (radioNetwork != "LTE" && radioNetwork != "NR",
                   "Unrecognized radio network technology");
  NS_ABORT_MSG_IF (simulator != "LENA" && simulator != "5GLENA",
                   "Unrecognized simulator");
  NS_ABORT_MSG_IF (scheduler != "PF" && scheduler != "RR",
                   "Unrecognized scheduler");
  
  NS_ABORT_MSG_IF (trafficScenario > 2,
                   "Traffic scenario " << trafficScenario << " not valid. Valid values are 0 1 2");

  if (dlRem || ulRem)
    {
      NS_ABORT_MSG_IF (simulator != "5GLENA",
                   "Cannot do the REM with the simulator " << simulator);
      NS_ABORT_MSG_IF (dlRem && ulRem,
                       "You selected both DL and UL REM, that is not supported");
      NS_ABORT_MSG_IF (remSector > 3,
                       "Only three sectors supported for REM");
      
      NS_ABORT_MSG_IF (remSector == 0 && freqScenario != 1,
                       "RemSector == 0 makes sense only in a OVERLAPPING scenario");
    }
  
  
  return true;
}

  
void
LenaLteComparison (const Parameters &params)
{
  params.Validate ();
      
  // Traffic parameters (that we will use inside this script:)
  uint32_t udpPacketSize = 1000;
  uint32_t lambda;
  uint32_t packetCount;

  std::cout << "\n----------------------------------------\n"
            << "Configuring scenario"
            << std::endl;

  std::cout << "  traffic parameters\n";
  switch (params.trafficScenario)
    {
    case 0: // let's put 80 Mbps with 20 MHz of bandwidth. Everything else is scaled
      packetCount = 0xFFFFFFFF;
      switch (params.bandwidthMHz)
        {
        case 20:
          udpPacketSize = 1000;
          break;
        case 10:
          udpPacketSize = 500;
          break;
        case 5:
          udpPacketSize = 250;
          break;
        default:
          udpPacketSize = 1000;
        }
      lambda = 10000 / params.ueNumPergNb;
      break;
    case 1:
      packetCount = 1;
      udpPacketSize = 12;
      lambda = 1;
      break;
    case 2: // 1 Mbps == 0.125 MB/s in case of 20 MHz, everything else is scaled
      packetCount = 0xFFFFFFFF;
      switch (params.bandwidthMHz)
        {
        case 20:
          udpPacketSize = 125;
          break;
        case 10:
          udpPacketSize = 63;
          break;
        case 5:
          udpPacketSize = 32;
          break;
        default:
          udpPacketSize = 125;
        }
      lambda = 1000 / params.ueNumPergNb;
      break;
    default:
      NS_FATAL_ERROR ("Traffic scenario " << params.trafficScenario << " not valid. Valid values are 0 1 2");
    }

  std::cout << "  statistics\n";
  SQLiteOutput db (params.outputDir + "/" + params.simTag + ".db", "lena-lte-comparison");
  SinrOutputStats sinrStats;
  PowerOutputStats ueTxPowerStats;
  PowerOutputStats gnbRxPowerStats;
  SlotOutputStats slotStats;
  RbOutputStats rbStats;

  sinrStats.SetDb (&db);
  ueTxPowerStats.SetDb (&db, "ueTxPower");
  slotStats.SetDb (&db);
  rbStats.SetDb (&db);
  gnbRxPowerStats.SetDb (&db, "gnbRxPower");

  /*
   * Check if the frequency and numerology are in the allowed range.
   * If you need to add other checks, here is the best position to put them.
   */
  std::cout << "  checking frequency and numerology\n";
  
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
  std::cout << "  logging\n";
  if (params.logging)
    {
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
//      LogComponentEnable ("NrMacSchedulerOfdma", LOG_LEVEL_ALL);
    }

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  std::cout << "  max tx buffer size\n";  
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  /*
   * Create the scenario. In our examples, we heavily use helpers that setup
   * the gnbs and ue following a pre-defined pattern. Please have a look at the
   * HexagonalGridScenarioHelper documentation to see how the nodes will be distributed.
   */
  std::cout << "  hexagonal grid\n";  
  HexagonalGridScenarioHelper gridScenario;
  gridScenario.SetNumRings (params.numOuterRings);
  gridScenario.SetSectorization (HexagonalGridScenarioHelper::TRIPLE);
  gridScenario.SetScenarioParamenters (params.scenario);
  uint16_t gNbNum = gridScenario.GetNumCells ();
  uint32_t ueNum = params.ueNumPergNb * gNbNum;
  gridScenario.SetUtNumber (ueNum);
  gridScenario.CreateScenario ();  //!< Creates and plots the network deployment
  const uint16_t ffr = 3; // Fractional Frequency Reuse scheme to mitigate intra-site inter-sector interferences

  /*
   * Create different gNB NodeContainer for the different sectors.
   */
  NodeContainer gnbSector1Container, gnbSector2Container, gnbSector3Container;
  for (uint32_t j = 0; j < gridScenario.GetBaseStations ().GetN (); ++j)
    {
      Ptr<Node> gnb = gridScenario.GetBaseStations ().Get (j);
      switch (j % ffr)
      {
        case 0:
          gnbSector1Container.Add (gnb);
          break;
        case 1:
          gnbSector2Container.Add (gnb);
          break;
        case 2:
          gnbSector3Container.Add (gnb);
          break;
        default:
          NS_ABORT_MSG("ffr param cannot be larger than 3");
          break;
      }
    }

  /*
   * Create different UE NodeContainer for the different sectors.
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

  /*
   * Setup the LTE or NR module. We create the various helpers needed inside
   * their respective configuration functions
   */
  std::cout << "  helpers\n";  
  Ptr<PointToPointEpcHelper> epcHelper;

  NetDeviceContainer gnbSector1NetDev, gnbSector2NetDev, gnbSector3NetDev;
  NetDeviceContainer ueSector1NetDev, ueSector2NetDev,ueSector3NetDev;

  Ptr <LteHelper> lteHelper = nullptr;
  Ptr <NrHelper> nrHelper = nullptr;

  if (params.simulator == "LENA")
    {
      epcHelper = CreateObject<PointToPointEpcHelper> ();
      LenaV1Utils::SetLenaV1SimulatorParameters (gridScenario,
                                  params.scenario,
                                  gnbSector1Container,
                                  gnbSector2Container,
                                  gnbSector3Container,
                                  ueSector1Container,
                                  ueSector2Container,
                                  ueSector3Container,
                                  epcHelper,
                                  lteHelper,
                                  gnbSector1NetDev,
                                  gnbSector2NetDev,
                                  gnbSector3NetDev,
                                  ueSector1NetDev,
                                  ueSector2NetDev,
                                  ueSector3NetDev,
                                  params.calibration,
                                  &sinrStats,
                                  &ueTxPowerStats,
                                  params.scheduler,
                                  params.bandwidthMHz,
                                  params.freqScenario);
    }
  else if (params.simulator == "5GLENA")
    {
      epcHelper = CreateObject<NrPointToPointEpcHelper> ();
      LenaV2Utils::SetLenaV2SimulatorParameters (gridScenario,
                                    params.scenario,
                                    params.radioNetwork,
                                    params.errorModel,
                                    params.operationMode,
                                    params.direction,
                                    params.numerologyBwp,
                                    params.pattern,
                                    gnbSector1Container,
                                    gnbSector2Container,
                                    gnbSector3Container,
                                    ueSector1Container,
                                    ueSector2Container,
                                    ueSector3Container,
                                    epcHelper,
                                    nrHelper,
                                    gnbSector1NetDev,
                                    gnbSector2NetDev,
                                    gnbSector3NetDev,
                                    ueSector1NetDev,
                                    ueSector2NetDev,
                                    ueSector3NetDev,
                                    params.calibration,
                                    &sinrStats,
                                    &ueTxPowerStats,
                                    &gnbRxPowerStats,
                                    &slotStats,
                                    &rbStats,
                                    params.scheduler,
                                    params.bandwidthMHz,
                                    params.freqScenario);
    }
  else
    {
      NS_ABORT_MSG ("Unrecognized cellular simulator");
    }

  // From here, it is standard NS3. In the future, we will create helpers
  // for this part as well.

  // create the internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost
  std::cout << "  pgw and internet\n";  
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
  std::cout << "  default gateway\n";  
  for (uint32_t j = 0; j < gridScenario.GetUserTerminals ().GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (gridScenario.GetUserTerminals ().Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to their gNB. Try to attach them per cellId order
  std::cout << "  attach UEs to gNBs\n";  
  for (uint32_t u = 0; u < ueNum; ++u)
    {
      uint32_t sector = u % ffr;
      uint32_t i = u / ffr;
      if (sector == 0)
        {
          Ptr<NetDevice> gnbNetDev = gnbSector1NetDev.Get (i % gridScenario.GetNumSites ());
          Ptr<NetDevice> ueNetDev = ueSector1NetDev.Get (i);
          if (lteHelper != nullptr)
            {
              lteHelper->Attach (ueNetDev, gnbNetDev);
            }
          else if (nrHelper != nullptr)
            {
              nrHelper->AttachToEnb (ueNetDev, gnbNetDev);
            }
          else
            {
              NS_ABORT_MSG ("Programming error");
            }
          if (params.logging == true)
            {
              Vector gnbpos = gnbNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
              Vector uepos = ueNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
              double distance = CalculateDistance (gnbpos, uepos);
              std::cout << "Distance = " << distance << " meters" << std::endl;
            }
        }
      else if (sector == 1)
        {
          Ptr<NetDevice> gnbNetDev = gnbSector2NetDev.Get (i % gridScenario.GetNumSites ());
          Ptr<NetDevice> ueNetDev = ueSector2NetDev.Get (i);
          if (lteHelper != nullptr)
            {
              lteHelper->Attach (ueNetDev, gnbNetDev);
            }
          else if (nrHelper != nullptr)
            {
              nrHelper->AttachToEnb (ueNetDev, gnbNetDev);
            }
          else
            {
              NS_ABORT_MSG ("Programming error");
            }
          if (params.logging == true)
            {
              Vector gnbpos = gnbNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
              Vector uepos = ueNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
              double distance = CalculateDistance (gnbpos, uepos);
              std::cout << "Distance = " << distance << " meters" << std::endl;
            }
        }
      else if (sector == 2)
        {
          Ptr<NetDevice> gnbNetDev = gnbSector3NetDev.Get (i % gridScenario.GetNumSites ());
          Ptr<NetDevice> ueNetDev = ueSector3NetDev.Get (i);
          if (lteHelper != nullptr)
            {
              lteHelper->Attach (ueNetDev, gnbNetDev);
            }
          else if (nrHelper != nullptr)
            {
              nrHelper->AttachToEnb (ueNetDev, gnbNetDev);
            }
          else
            {
              NS_ABORT_MSG ("Programming error");
            }
          if (params.logging == true)
            {
              Vector gnbpos = gnbNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
              Vector uepos = ueNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
              double distance = CalculateDistance (gnbpos, uepos);
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
  std::cout << "  server factory\n";  
  uint16_t dlPortLowLat = 1234;

  ApplicationContainer serverApps;

  // The sink will always listen to the specified ports
  UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);

  // The server, that is the application which is listening, is installed in the UE
  if (params.direction == "DL")
    {
      serverApps.Add (dlPacketSinkLowLat.Install ({ueSector1Container,ueSector2Container,ueSector3Container}));
    }
  else
    {
      serverApps.Add (dlPacketSinkLowLat.Install (remoteHost));
    }

  // start UDP server
  serverApps.Start (MilliSeconds (params.udpAppStartTimeMs));

  /*
   * Configure attributes for the different generators, using user-provided
   * parameters for generating a CBR traffic
   *
   * Low-Latency configuration and object creation:
   */
  std::cout << "  client factory\n";  
  UdpClientHelper dlClientLowLat;
  dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
  dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (packetCount));
  dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSize));
  dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0/lambda)));

  /*
   * Let's install the applications!
   */
  std::cout << "  applications\n";  
  ApplicationContainer clientApps;
  std::vector<NodeContainer*> nodes = { &ueSector1Container, &ueSector2Container, &ueSector3Container };
  std::vector<NetDeviceContainer*> devices = { &ueSector1NetDev, &ueSector2NetDev, &ueSector3NetDev };
  std::vector<Ipv4InterfaceContainer*> ips = { &ueSector1IpIface, &ueSector2IpIface, &ueSector3IpIface };

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetStream (RngSeedManager::GetRun());
  double maxStartTime = 0.0;

  for (uint32_t userId = 0; userId < gridScenario.GetUserTerminals().GetN (); ++userId)
    {
      for (uint32_t j = 0; j < 3; ++j)
        {
          if (nodes.at (j)->GetN () <= userId)
            {
              continue;
            }
          Ptr<Node> n = nodes.at(j)->Get (userId);
          Ptr<NetDevice> d = devices.at(j)->Get(userId);
          Address a = ips.at(j)->GetAddress(userId);

          std::cout << "app for ue " << userId << " in sector " << j+1
                    << " position " << n->GetObject<MobilityModel>()->GetPosition ()
                    << ":" << std::endl;

          auto app = InstallApps (n, d, a, params.direction, &dlClientLowLat, remoteHost,
                                  remoteHostAddr, params.udpAppStartTimeMs, dlPortLowLat,
                                  x, params.appGenerationTimeMs, lteHelper, nrHelper);
          maxStartTime = std::max (app.second, maxStartTime);
          clientApps.Add (app.first);
        }
    }

  // enable the traces provided by the nr module
  std::cout << "  tracing\n";  
  if (params.traces == true)
    {
      if (lteHelper != nullptr)
        {
          lteHelper->EnableTraces ();
        }
      else if (nrHelper != nullptr)
        {
          nrHelper->EnableTraces ();
        }
    }


  std::cout << "  flowmon\n";  
  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (gridScenario.GetUserTerminals ());

  Ptr<FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  std::string tableName = "e2e";


  std::cout << "  rem helper\n";  
  Ptr<NrRadioEnvironmentMapHelper> remHelper; // Must be placed outside of block "if (generateRem)" because otherwise it gets destroyed,
                                              // and when simulation starts the object does not exist anymore, but the scheduled REM events do (exist).
                                              // So, REM events would be called with invalid pointer to remHelper ...

  if (params.dlRem || params.ulRem)
    {
      NS_ABORT_MSG_IF (params.simulator != "5GLENA",
                       "Cannot do the REM with the simulator " << params.simulator);
      NS_ABORT_MSG_IF (params.dlRem && params.ulRem,
                       "You selected both DL and UL REM, that is not supported");
      NS_ABORT_MSG_IF (params.remSector == 0 && params.freqScenario != 1,
                       "RemSector == 0 makes sense only in a OVERLAPPING scenario");

      NetDeviceContainer gnbContainerRem;
      Ptr<NetDevice> ueRemDevice;
      uint16_t remPhyIndex = 0;
      if (params.operationMode == "FDD" && params.direction == "UL")
      {
        remPhyIndex = 1;
      }

      NetDeviceContainer ueContainerRem;
      Ptr<NrGnbNetDevice> gnbRemDevice;

      if (params.ulRem)
        {
          if (params.remSector == 0)
            {
              ueContainerRem.Add (ueSector1NetDev);
              ueContainerRem.Add (ueSector2NetDev);
              ueContainerRem.Add (ueSector3NetDev);
              gnbRemDevice = DynamicCast<NrGnbNetDevice> (gnbSector1NetDev.Get (0));
              Ptr<ThreeGppAntennaArrayModel> antenna = ConstCast<ThreeGppAntennaArrayModel> (gnbRemDevice->GetPhy(0)->GetSpectrumPhy ()->GetAntennaArray ());
              antenna->SetAttribute ("IsotropicElements", BooleanValue (true));
            }
          else if (params.remSector == 1)
            {
              ueContainerRem = ueSector1NetDev;
              gnbRemDevice = DynamicCast<NrGnbNetDevice> (gnbSector1NetDev.Get(0));
              Ptr<ThreeGppAntennaArrayModel> antenna = ConstCast<ThreeGppAntennaArrayModel> (gnbRemDevice->GetPhy(0)->GetSpectrumPhy ()->GetAntennaArray ());
              antenna->SetAttribute ("IsotropicElements", BooleanValue (true));
            }
          else if (params.remSector == 2)
            {
              ueContainerRem = ueSector2NetDev;
              gnbRemDevice = DynamicCast<NrGnbNetDevice> (gnbSector2NetDev.Get(0));
              Ptr<ThreeGppAntennaArrayModel> antenna = ConstCast<ThreeGppAntennaArrayModel> (gnbRemDevice->GetPhy(0)->GetSpectrumPhy ()->GetAntennaArray ());
              antenna->SetAttribute ("IsotropicElements", BooleanValue (true));
            }
          else if (params.remSector == 3)
            {
              ueContainerRem = ueSector3NetDev;
              gnbRemDevice = DynamicCast<NrGnbNetDevice> (gnbSector3NetDev.Get(0));
              Ptr<ThreeGppAntennaArrayModel> antenna = ConstCast<ThreeGppAntennaArrayModel> (gnbRemDevice->GetPhy(0)->GetSpectrumPhy ()->GetAntennaArray ());
              antenna->SetAttribute ("IsotropicElements", BooleanValue (true));
            }
        }
      else
        {
          if (params.remSector == 0)
            {
              gnbContainerRem.Add (gnbSector1NetDev);
              gnbContainerRem.Add (gnbSector2NetDev);
              gnbContainerRem.Add (gnbSector3NetDev);
              ueRemDevice = ueSector1NetDev.Get (0);
            }
          else if (params.remSector == 1)
            {
              gnbContainerRem = gnbSector1NetDev;
              ueRemDevice = ueSector1NetDev.Get(0);
            }
          else if (params.remSector == 2)
            {
              gnbContainerRem = gnbSector2NetDev;
              ueRemDevice = ueSector2NetDev.Get(0);
            }
          else if (params.remSector == 3)
            {
              gnbContainerRem = gnbSector3NetDev;
              ueRemDevice = ueSector3NetDev.Get(0);
            }
          else
            {
              NS_FATAL_ERROR ("Sector does not exist");
            }
        }

      //Radio Environment Map Generation for ccId 0
      remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
      remHelper->SetMinX (params.xMinRem);
      remHelper->SetMaxX (params.xMaxRem);
      remHelper->SetResX (params.xResRem);
      remHelper->SetMinY (params.yMinRem);
      remHelper->SetMaxY (params.yMaxRem);
      remHelper->SetResY (params.yResRem);
      remHelper->SetZ (params.zRem);

      //save beamforming vectors
      for (uint32_t j = 0; j < gridScenario.GetNumSites (); ++j)
        {
          switch (params.remSector)
            {
            case 0:
              gnbSector1NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector1NetDev.Get(j));
              gnbSector2NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector2NetDev.Get(j));
              gnbSector3NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector3NetDev.Get(j));
              break;
            case 1:
              gnbSector1NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector1NetDev.Get(j));
              break;
            case 2:
              gnbSector2NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector2NetDev.Get(j));
              break;
            case 3:
              gnbSector3NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector3NetDev.Get(j));
              break;
            default:
              NS_ABORT_MSG("sector cannot be larger than 3");
              break;
            }
        }

      if (params.ulRem)
        {
          remHelper->CreateRem (ueContainerRem, gnbRemDevice, remPhyIndex);
        }
      else
        {
          remHelper->CreateRem (gnbContainerRem, ueRemDevice, remPhyIndex);
        }
    }

  std::cout << "\n----------------------------------------\n"
            << "Start simulation"
            << std::endl;
  Simulator::Stop (MilliSeconds (params.appGenerationTimeMs + maxStartTime));
  Simulator::Run ();

  sinrStats.EmptyCache ();
  ueTxPowerStats.EmptyCache ();
  gnbRxPowerStats.EmptyCache ();
  slotStats.EmptyCache ();
  rbStats.EmptyCache ();

  /*
   * To check what was installed in the memory, i.e., BWPs of eNb Device, and its configuration.
   * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> NrGnbPhy -> Numerology,
  GtkConfigStore config;
  config.ConfigureAttributes ();
  */

  FlowMonitorOutputStats flowMonStats;
  flowMonStats.SetDb (&db, tableName);
  flowMonStats.Save (monitor, flowmonHelper, params.outputDir + "/" + params.simTag);

  Simulator::Destroy ();
}

std::ostream &
operator << (std::ostream & os, const Parameters & parameters)
{
  // Use p as shorthand for arg parametersx
  auto p {parameters};

#define MSG(m) \
    os << "\n" << m << std::left \
       << std::setw (40 - strlen (m)) << (strlen (m) > 0 ? ":" : "")

  
  MSG ("LENA LTE Scenario Parameters");
  MSG ("");
  MSG ("Model version")
    << p.simulator << (p.simulator == "LENA" ? " (v1)" : " (v2)");
  if (p.simulator == "5GLENA")
    {
      MSG ("LTE Standard")
        << p.radioNetwork << (p.radioNetwork == "LTE" ? " (4G)" : " (5G NR)");
      MSG ("4G-NR calibration mode")    << (p.calibration ? "ON" : "off");
      MSG ("Operation mode")            << p.operationMode;
      if (p.operationMode == "TDD")
        {
          MSG ("Numerology")            << p.numerologyBwp;
          MSG ("TDD pattern")           << p.pattern;
        }
      if (p.errorModel != "")
        {
          MSG ("Error model")           << p.errorModel;
        }
      else if (p.radioNetwork == "LTE")
        {
          MSG ("Error model")           << "ns3::LenaErrorModel";
        }
      else if (p.radioNetwork == "NR")
        {
          MSG ("Error model")           << "ns3::NrEesmCcT2";
        }
        
    }
  else
    {
      // LENA v1
      p.operationMode = "FDD";
      MSG ("LTE Standard")              << "4G";
      MSG ("Calibration mode")          << (p.calibration ? "ON" : "off");
      MSG ("Operation mode")            << p.operationMode;
    }

  MSG ("");
  MSG ("Channel bandwidth")             << p.bandwidthMHz << " MHz";
  MSG ("Spectrum configuration")
    <<    (p.freqScenario == 0 ? "non-" : "") << "overlapping";
  MSG ("LTE Scheduler")                 << p.scheduler;

  MSG ("");
  MSG ("Basic scenario")                << p.scenario;
  if (p.scenario == "UMa")
    {
      os << "\n  (ISD: 1.7 km, BS: 30 m, UE: 1.5 m, UE-BS min: 30.2 m)";
    }
  else if (p.scenario == "UMi")
    {
      os << "\n  (ISD: 0.5 km, BS: 10 m, UE: 1.5 m, UE-BS min: 10 m)";
    }
  else if (p.scenario == "RMa")
    {
      os << "\n  (ISD: 7.0 km, BS: 45 m, UE: 1.5 m, UE-BS min: 44.6 m)";
    }
  else 
    {
      os << "\n  (unknown configuration)";
    }
  MSG ("Number of outer rings")         << p.numOuterRings;
  MSG ("Number of UEs per sector")      << p.ueNumPergNb;

  MSG ("");
  MSG ("Network loading")               << p.trafficScenario;
  switch (p.trafficScenario)
    {
    case 0:
      MSG ("  Max loading (80 Mbps/20 MHz)");
      MSG ("  Number of packets")       << "infinite";
      MSG ("  Packet size");
      switch (p.bandwidthMHz)
        {
        case 20:  os << "1000 bytes";    break;
        case 10:  os << "500 bytes";     break;
        case 5:   os << "250 bytes";     break;
        default:  os << "1000 bytes";
        }
      MSG ("  Inter-packet interval (per UE)") << 10000 / p.ueNumPergNb << " s";
      break ;
      
    case 1:
      MSG ("  Latency");
      MSG ("  Number of packets")              << 1;
      MSG ("  Packet size")                    << "12 bytes";
      MSG ("  Inter-packet interval (per UE)") << "1 s"; 
      break ;

    case 2:
      MSG ("  Moderate loading");
      MSG ("  Number of packets")              << "infinite";
      MSG ("  Packet size");
      switch (p.bandwidthMHz)
        {
        case 20:  os << "125 bytes";    break;
        case 10:  os << "63 bytes";     break;
        case 5:   os << "32 bytes";     break;
        default:  os << "125 bytes";
        }
      MSG ("  Inter-packet interval (per UE)") << 1000 / p.ueNumPergNb << " s";
  
      break ;

    default:
      os << "\n  (Unknown configuration)";
    }

  MSG ("Application start window")      << p.udpAppStartTimeMs << " + 10 ms";
  MSG ("Application on duration")       << p.appGenerationTimeMs << " ms";
  MSG ("Traffic direction")             << p.direction;

  MSG ("");
  MSG ("Output file name")              << p.simTag;
  MSG ("Output directory")              << p.outputDir;
  MSG ("Logging")                       << (p.logging ? "ON" : "off");
  MSG ("Trace file generation")         << (p.traces ? "ON" : "off");
  MSG ("");
  MSG ("Radio environment map")         << (p.dlRem ? "DL" : (p.ulRem ? "UL" : "off"));
  if (p.dlRem || p.ulRem)
    {
      MSG ("  Sector to sample");
      if (p.remSector == 0)
        {
          os << "all";
        }
      else
        {
          os << p.remSector;
        }
      MSG ("  X range") << p.xMinRem << " - " << p.xMaxRem << ", in " << p.xResRem << " m steps";
      MSG ("  Y range") << p.yMinRem << " - " << p.yMaxRem << ", in " << p.yResRem << " m steps";
      MSG ("  Altitude (Z)")             << p.zRem << " m";
    }

  os << std::endl;
  return os;
}
  

} // namespace ns3

