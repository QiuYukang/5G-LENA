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
 * \ingroup examples
 * \file s3-scenario.cc
 * \brief A multi-cell network deployment with site sectorization
 *
 * This example describes how to setup a simulation using the 3GPP channel model
 * from TR 38.900. This example consists of an hexagonal grid deployment
 * consisting on a central site and a number of outer rings of sites around this
 * central site. Each site is sectorized, meaning that a number of three antenna
 * arrays or panels are deployed per gNB. These three antennas are pointing to
 * 30º, 150º and 270º w.r.t. the horizontal axis. We allocate a band to each
 * sector of a site, and the bands are contiguous in frequency.
 *
 * We provide a number of simulation parameters that can be configured in the
 * command line, such as the number of UEs per cell or the number of outer rings.
 * Please have a look at the possible parameters to know what you can configure
 * through the command line.
 *
 * With the default configuration, the example will create one DL flow per UE.
 * The example will print on-screen the end-to-end result of each flow,
 * as well as writing them on a file.
 *
 * \code{.unparsed}
$ ./waf --run "lena-lte-comparison --Help"
    \endcode
 *
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

#include "ns3/lte-module.h"
#include <ns3/radio-environment-map-helper.h>
#include "ns3/config-store-module.h"
#include <ns3/sqlite-output.h>
#include "radio-network-parameters-helper.h"
#include "sinr-output-stats.h"
#include "flow-monitor-output-stats.h"
#include "power-output-stats.h"
#include "slot-output-stats.h"
#include "lena-v1-utils.h"
#include "lena-v2-utils.h"

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
 * $ export NS_LOG="LenaLteComparison=level_info|prefix_func|prefix_time"
 */
NS_LOG_COMPONENT_DEFINE ("LenaLteComparison");

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

int 
main (int argc, char *argv[])
{
  /*
   * Variables that represent the parameters we will accept as input by the
   * command line. Each of them is initialized with a default value.
   */
  // Scenario parameters (that we will use inside this script):
  uint16_t numOuterRings = 3;
  uint16_t ueNumPergNb = 2;
  bool logging = false;
  bool traces = true;
  std::string simulator = "";
  std::string scenario = "UMa";
  std::string radioNetwork = "NR";  // LTE or NR
  std::string operationMode = "TDD";  // TDD or FDD

  // Simulation parameters. Please don't use double to indicate seconds, use
  // milliseconds and integers to avoid representation errors.
  uint32_t appGenerationTimeMs = 1000;
  uint32_t udpAppStartTimeMs = 400;
  std::string direction = "DL";

  // Spectrum parameters. We will take the input from the command line, and then
  //  we will pass them inside the NR module.
  uint16_t numerologyBwp = 0;
  std::string pattern = "F|F|F|F|F|F|F|F|F|F|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"
  uint32_t bandwidthMHz = 20;

  // Where we will store the output files.
  std::string simTag = "default";
  std::string outputDir = "./";

  // Error models
  std::string errorModel = "";

  bool calibration = true;

  uint32_t trafficScenario = 0;

  std::string scheduler = "PF";

  // Rem parameters: Modify them by hand, don't use the CommandLine for
  // the moment
  double xMinRem = -2000.0;
  double xMaxRem = 2000.0;
  uint16_t xResRem = 100;
  double yMinRem = -2000.0;
  double yMaxRem = 2000.0;
  uint16_t yResRem = 100;
  double zRem = 1.5;
  bool generateRem = false;
  uint32_t remSector = 1;


  /*
   * From here, we instruct the ns3::CommandLine class of all the input parameters
   * that we may accept as input, as well as their description, and the storage
   * variable.
   */
  CommandLine cmd;

  cmd.AddValue ("scenario",
                "The urban scenario string (UMa,UMi,RMa)",
                scenario);
  cmd.AddValue ("numRings",
                "The number of rings around the central site",
                numOuterRings);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per cell or gNB in multiple-ue topology",
                ueNumPergNb);
  cmd.AddValue ("appGenerationTimeMs",
                "Simulation time",
                appGenerationTimeMs);
  cmd.AddValue ("numerologyBwp",
                "The numerology to be used (NR only)",
                numerologyBwp);
  cmd.AddValue ("pattern",
                "The TDD pattern to use",
                pattern);
  cmd.AddValue ("direction",
                "The flow direction (DL or UL)",
                direction);
  cmd.AddValue ("simulator",
                "The cellular network simulator to use: LENA or 5GLENA",
                simulator);
  cmd.AddValue ("technology",
                "The radio access network technology",
                radioNetwork);
  cmd.AddValue ("operationMode",
                "The network operation mode can be TDD or FDD",
                operationMode);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);
  cmd.AddValue("errorModelType",
               "Error model type: ns3::NrEesmCcT1, ns3::NrEesmCcT2, ns3::NrEesmIrT1, ns3::NrEesmIrT2, ns3::NrLteMiErrorModel",
               errorModel);
  cmd.AddValue ("calibration",
                "disable a bunch of things to make LENA and NR_LTE comparable",
                calibration);
  cmd.AddValue ("trafficScenario",
                "0: saturation (110 Mbps/enb), 1: latency (1 pkt of 10 bytes), 2: low-load (20 Mbps)",
                trafficScenario);
  cmd.AddValue ("scheduler",
                "PF: Proportional Fair, RR: Round-Robin",
                scheduler);
  cmd.AddValue ("bandwidth",
                "BW in MHz for each BWP (integer value): valid values are 20, 10, 5",
                bandwidthMHz);

  // Parse the command line
  cmd.Parse (argc, argv);

  // Traffic parameters (that we will use inside this script:)
  uint32_t udpPacketSize = 1000;
  uint32_t lambda;
  uint32_t packetCount;

  NS_ABORT_MSG_IF (bandwidthMHz != 20 && bandwidthMHz != 10 && bandwidthMHz != 5,
                   "Valid bandwidth values are 20, 10, 5, you set " << bandwidthMHz);

  switch (trafficScenario)
    {
    case 0: // let's put 80 Mbps with 20 MHz of bandwidth. Everything else is scaled
      packetCount = 0xFFFFFFFF;
      switch (bandwidthMHz)
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
      lambda = 10000 / ueNumPergNb;
      break;
    case 1:
      packetCount = 1;
      udpPacketSize = 12;
      lambda = 1;
      break;
    case 2: // 20 Mbps == 2.5 MB/s in case of 20 MHz, everything else is scaled
      packetCount = 0xFFFFFFFF;
      switch (bandwidthMHz)
        {
        case 20:
          udpPacketSize = 250;
          break;
        case 10:
          udpPacketSize = 125;
          break;
        case 5:
          udpPacketSize = 75;
          break;
        default:
          udpPacketSize = 250;
        }
      lambda = 10000 / ueNumPergNb;
      break;
    default:
      NS_FATAL_ERROR ("Traffic scenario " << trafficScenario << " not valid. Valid values are 0 1 2");
    }

  SQLiteOutput db (outputDir + "/" + simTag + ".db", "lena-lte-comparison");
  SinrOutputStats sinrStats;
  PowerOutputStats powerStats;
  SlotOutputStats slotStats;

  sinrStats.SetDb (&db);
  powerStats.SetDb (&db);
  slotStats.SetDb (&db);

  /*
   * Check if the frequency and numerology are in the allowed range.
   * If you need to add other checks, here is the best position to put them.
   */
//  NS_ABORT_IF (centralFrequencyBand > 100e9);
  NS_ABORT_IF (numerologyBwp > 4);
  NS_ABORT_MSG_IF (direction != "DL" && direction != "UL", "Flow direction can only be DL or UL");
  NS_ABORT_MSG_IF (operationMode != "TDD" && operationMode != "FDD", "Operation mode can only be TDD or FDD");
  NS_ABORT_MSG_IF (radioNetwork != "LTE" && radioNetwork != "NR", "Unrecognized radio network technology");
  NS_ABORT_MSG_IF (simulator != "LENA" && simulator != "5GLENA", "Unrecognized simulator");
  NS_ABORT_MSG_IF (scheduler != "PF" && scheduler != "RR", "Unrecognized scheduler");
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
//      LogComponentEnable ("NrMacSchedulerOfdma", LOG_LEVEL_ALL);
    }

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  /*
   * Create the scenario. In our examples, we heavily use helpers that setup
   * the gnbs and ue following a pre-defined pattern. Please have a look at the
   * HexagonalGridScenarioHelper documentation to see how the nodes will be distributed.
   */
  HexagonalGridScenarioHelper gridScenario;
  gridScenario.SetNumRings (numOuterRings);
  gridScenario.SetSectorization (HexagonalGridScenarioHelper::TRIPLE);
  gridScenario.SetScenarioParamenters (scenario);
  uint16_t gNbNum = gridScenario.GetNumCells ();
  uint32_t ueNum = ueNumPergNb * gNbNum;
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
  Ptr<PointToPointEpcHelper> epcHelper;

  NetDeviceContainer gnbSector1NetDev, gnbSector2NetDev, gnbSector3NetDev;
  NetDeviceContainer ueSector1NetDev, ueSector2NetDev,ueSector3NetDev;

  Ptr <LteHelper> lteHelper = nullptr;
  Ptr <NrHelper> nrHelper = nullptr;

  if (simulator == "LENA")
    {
      epcHelper = CreateObject<PointToPointEpcHelper> ();
      LenaV1Utils::SetLenaV1SimulatorParameters (gridScenario,
                                  scenario,
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
                                  calibration,
                                  &sinrStats,
                                  &powerStats,
                                  scheduler,
                                  bandwidthMHz);
    }
  else if (simulator == "5GLENA")
    {
      epcHelper = CreateObject<NrPointToPointEpcHelper> ();
      LenaV2Utils::SetLenaV2SimulatorParameters (gridScenario,
                                    scenario,
                                    radioNetwork,
                                    errorModel,
                                    operationMode,
                                    direction,
                                    numerologyBwp,
                                    pattern,
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
                                    calibration,
                                    &sinrStats,
                                    &powerStats,
                                    &slotStats,
                                    scheduler,
                                    bandwidthMHz);
    }
  else
    {
      NS_ABORT_MSG ("Unrecognized cellular simulator");
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
          if (logging == true)
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
          if (logging == true)
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
          if (logging == true)
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

  // start UDP server
  serverApps.Start (MilliSeconds (udpAppStartTimeMs));

  /*
   * Configure attributes for the different generators, using user-provided
   * parameters for generating a CBR traffic
   *
   * Low-Latency configuration and object creation:
   */
  UdpClientHelper dlClientLowLat;
  dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
  dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (packetCount));
  dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSize));
  dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0/lambda)));

  /*
   * Let's install the applications!
   */
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

          auto app = InstallApps (n, d, a, direction, &dlClientLowLat, remoteHost,
                                  remoteHostAddr, udpAppStartTimeMs, dlPortLowLat,
                                  x, appGenerationTimeMs, lteHelper, nrHelper);
          maxStartTime = std::max (app.second, maxStartTime);
          clientApps.Add (app.first);
        }
    }

  // enable the traces provided by the nr module
  if (traces == true)
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


  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (gridScenario.GetUserTerminals ());

  Ptr<FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  std::string tableName = "e2e";


  Ptr<NrRadioEnvironmentMapHelper> remHelper; // Must be placed outside of block "if (generateRem)" because otherwise it gets destroyed,
                                              // and when simulation starts the object does not exist anymore, but the scheduled REM events do (exist).
                                              // So, REM events would be called with invalid pointer to remHelper ...
  if (generateRem)
    {
      if (simulator == "5GLENA" && !calibration)
        {
          NetDeviceContainer gnbContainerRem;
          Ptr<NetDevice> ueRemDevice;
          uint16_t remPhyIndex = 0;

          if (remSector == 1)
            {
              gnbContainerRem = gnbSector1NetDev;
              ueRemDevice = ueSector1NetDev.Get(0);
            }
          else if (remSector == 2)
            {
              gnbContainerRem = gnbSector2NetDev;
              ueRemDevice = ueSector2NetDev.Get(0);
            }
          else if (remSector == 3)
            {
              gnbContainerRem = gnbSector3NetDev;
              ueRemDevice = ueSector3NetDev.Get(0);
            }
          else
            {
              NS_FATAL_ERROR ("Sector does not exist");
            }

          //Radio Environment Map Generation for ccId 0
          remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
          remHelper->SetMinX (xMinRem);
          remHelper->SetMaxX (xMaxRem);
          remHelper->SetResX (xResRem);
          remHelper->SetMinY (yMinRem);
          remHelper->SetMaxY (yMaxRem);
          remHelper->SetResY (yResRem);
          remHelper->SetZ (zRem);

          //save beamforming vectors
          for (uint32_t j = 0; j < gridScenario.GetNumSites (); ++j)
            {
              switch (remSector - 1)
              {
                case 0:
                  gnbSector1NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector1NetDev.Get(j));
                  break;
                case 1:
                  gnbSector2NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector2NetDev.Get(j));
                  break;
                case 2:
                  gnbSector3NetDev.Get(j)->GetObject<NrGnbNetDevice>()->GetPhy(remPhyIndex)->GetBeamManager()->ChangeBeamformingVector(ueSector3NetDev.Get(j));
                  break;
                default:
                  NS_ABORT_MSG("sector cannot be larger than 3");
                  break;
              }
            }

          remHelper->CreateRem (gnbContainerRem, ueRemDevice, remPhyIndex);  //bwpId 0
        }
    }


  Simulator::Stop (MilliSeconds (appGenerationTimeMs + maxStartTime));
  Simulator::Run ();

  sinrStats.EmptyCache ();
  powerStats.EmptyCache ();
  slotStats.EmptyCache ();

  /*
   * To check what was installed in the memory, i.e., BWPs of eNb Device, and its configuration.
   * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> NrGnbPhy -> Numerology,
  GtkConfigStore config;
  config.ConfigureAttributes ();
  */

  FlowMonitorOutputStats flowMonStats;
  flowMonStats.SetDb (&db, tableName);
  flowMonStats.Save (monitor, flowmonHelper, outputDir + "/" + simTag);

  Simulator::Destroy ();
  return 0;
}


