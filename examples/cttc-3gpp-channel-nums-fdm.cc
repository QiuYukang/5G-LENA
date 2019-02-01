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



/**
 *
 * \file cttc-3gpp-channel-nums-fdm.cc
 * \ingroup examples
 * \brief Frequency division multiplexing example.
 *
 * The simulation program allows the user to configure 2 UEs and 1 or 2 bandwidth parts (BWPs) and test the end-to-end performance.
 * This example is designed to expect the full configuration of each BWP. The configuration of BWP is composed of the following parameters:
 * central carrier frequency, bandwidth and numerology. There are 2 UEs, and each UE has one flow. One flow is of URLLC traffic type, while the another is eMBB.
 * URLLC is configured to be transmitted over the first BWP, and the eMBB over the second BWP.
 * Hence, in this example it is expected to configure the first BWP to use a higher numerology than the second BWP.
 * The simulation topology is as the one used in "cttc-3gpp-channel-nums.cc".
 * The user can run this example with UDP full buffer traffic or can specify the UDP packet interval and UDP packet size per type of traffic.
 * "--udpIntervalUll" and "--packetSizeUll" parameters are used to configure the UDP traffic of URLLC flow,
 * while "--udpIntervalBe" and "--packetSizeBe" parameters are used to configure the UDP traffic of eMBB flow.
 * If UDP full buffer traffic is configured, the packet interval for each flow is calculated based on approximated value of saturation rate for the bandwidth to
 * which the flow is mapped, and taking into account the packet size of the flow.
 * The total transmission power for each BWP depends on how the bandwidth is divided among BWP, and will be proportionally assigned to each BWP.
 * If the user configures only 1 BWP, then the configuration for the first BWP will be used.
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
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/mmwave-mac-scheduler-tdma-rr.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("3gppChannelFdmBandwidthPartsExample");

/**
 * \brief Global variable used to configure whether to configure a full buffer UDP traffic. It is accessible as "--udpFullBuffer" from CommandLine.
 */
static ns3::GlobalValue g_udpRate ("udpFullBuffer",
                                   "Whether to set the full buffer traffic; if this parameter is set then the udpInterval parameter"
                                   "will be neglected.",
                                   ns3::BooleanValue (false),
                                   ns3::MakeBooleanChecker());

/**
 * \brief Global variable used to configure whether to setup a single UE topology. It is accessible as "--singleUeTopology" from CommandLine.
 */
static ns3::GlobalValue g_singleUeTopology ("singleUeTopology",
                                            "When true the example uses a single UE topology, when false use topology with variable number of UEs"
                                            "will be neglected.",
                                            ns3::BooleanValue (false),
                                            ns3::MakeBooleanChecker());

/**
 * \brief Global variable used to configure whether to use the fixed MCS. It is accessible as "--useFixedMcs" from CommandLine.
 */
static ns3::GlobalValue g_useFixedMcs ("useFixedMcs",
                                       "Whether to use fixed mcs, normally used for testing purposes",
                                        ns3::BooleanValue (false),
                                        ns3::MakeBooleanChecker());
/**
 * \brief Global variable used to configure the value of fixed MCS in the case it is used. It is accessible as "--fixedMcs" from CommandLine.
 */
static ns3::GlobalValue g_fixedMcs ("fixedMcs",
                                    "The MCS that will be used in this example",
                                    ns3::UintegerValue (1),
                                    ns3::MakeUintegerChecker<uint32_t>());
/**
 * \brief Global variable used to configure the number of gNBs. It is accessible as "--gNbNum" from CommandLine.
 */
static ns3::GlobalValue g_gNbNum ("gNbNum",
                                  "The number of gNbs in multiple-ue topology",
                                   ns3::UintegerValue (1),
                                   ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the number of UEs per gNB. It is accessible as "--ueNumPergNb" from CommandLine.
 */
static ns3::GlobalValue g_ueNum ("ueNumPergNb",
                                  "The number of UE per gNb in multiple-ue topology",
                                  ns3::UintegerValue (2),
                                  ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the beamforming method. It is accessible as "--cellScan" from CommandLine.
 */
static ns3::GlobalValue g_cellScan ("cellScan",
                                    "Use beam search method to determine beamforming vector, the default is long-term covariance matrix method"
                                    "true to use cell scanning method, false to use the default power method.",
                                    ns3::BooleanValue (false),
                                    ns3::MakeBooleanChecker());
/**
 * \brief Global variable used to configure the beam searcg angle step. It is accessible as "--beamSearchAngleStep" from CommandLine.
 */
static ns3::GlobalValue g_beamSearchAngleStep ("beamSearchAngleStep",
                                      "Beam search angle step for beam search method",
                                      ns3::DoubleValue (10),
                                      ns3::MakeDoubleChecker<double>());

/******************************** FDM parameters ******************************************/

/**
 * \brief Global variable used to configure the numerology for BWP 1. It is accessible as "--numerologyBwp1" from CommandLine.
 */
static ns3::GlobalValue g_numerologyBwp1 ("numerologyBwp1",
                                          "The numerology to be used in bandwidth part 1",
                                           ns3::UintegerValue (4),
                                           ns3::MakeUintegerChecker<uint32_t>());
/**
 * \brief Global variable used to configure the central system frequency for BWP 1. It is accessible as "--frequencyBwp1" from CommandLine.
 */
static ns3::GlobalValue g_frequencyBwp1 ("frequencyBwp1",
                                         "The system frequency to be used in bandwidth part 1",
                                          ns3::DoubleValue(28e9),
                                          ns3::MakeDoubleChecker<double>(6e9,100e9));
/**
 * \brief Global variable used to configure the bandwidth for BWP 1. This value is expressed in Hz.It is accessible as "--bandwidthBwp1" from CommandLine.
 */
static ns3::GlobalValue g_bandwidthBwp1 ("bandwidthBwp1",
                                        "The system bandwidth to be used in bandwidth part 1",
                                         ns3::DoubleValue(100e6),
                                         ns3::MakeDoubleChecker<double>());

/**
 * \brief Global variable used to configure the numerology for BWP 2. It is accessible as "--numerologyBwp2" from CommandLine.
 */
static ns3::GlobalValue g_numerologyBwp2 ("numerologyBwp2",
                                          "The numerology to be used in bandwidth part 2",
                                           ns3::UintegerValue (2),
                                           ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the central system frequency for BWP 2. It is accessible as "--frequencyBwp2" from CommandLine.
 */
static ns3::GlobalValue g_frequencyBwp2 ("frequencyBwp2",
                                         "The system frequency to be used in bandwidth part 2",
                                          ns3::DoubleValue(28.2e9),
                                          ns3::MakeDoubleChecker<double>(6e9,100e9));

/**
 * \brief Global variable used to configure the bandwidth for BWP 2. This value is expressed in Hz.It is accessible as "--bandwidthBwp2" from CommandLine.
 */
static ns3::GlobalValue g_bandwidthBwp2 ("bandwidthBwp2",
                                         "The system bandwidth to be used in bandwidth part 2",
                                          ns3::DoubleValue(100e6),
                                          ns3::MakeDoubleChecker<double>());

/**
 * \brief Global variable used to configure the packet size for ULL  type of traffic . This value is expressed in bytes. It is accessible as "--packetSizeUll" from CommandLine.
 */
static ns3::GlobalValue g_udpPacketSizeUll ("packetSizeUll",
                                            "packet size in bytes to be used by ultra low latency traffic",
                                            ns3::UintegerValue (100),
                                            ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the packet size for BE  type of traffic . This value is expressed in bytes. It is accessible as "--packetSizeBe" from CommandLine.
 */
static ns3::GlobalValue g_udpPacketSizeBe ("packetSizeBe",
                                           "packet size in bytes to be used by best effort traffic",
                                           ns3::UintegerValue (1252),
                                           ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the lambda parameter for ULL type of traffic . This value is expressed in bytes. It is accessible as "--lambdaUll" from CommandLine.
 */
static ns3::GlobalValue g_udpIntervalUll ("lambdaUll",
                                          "Number of UDP packets in one second for ultra low latency traffic",
                                          ns3::UintegerValue (10),
                                          ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the lambda parameter for BE type of traffic . This value is expressed in bytes. It is accessible as "--lambdaBe" from CommandLine.
 */
static ns3::GlobalValue g_udpIntervalBe ("lambdaBe",
                                         "Number of UDP packets in one second for best effor traffic",
                                         ns3::UintegerValue (1),
                                         ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the simulation tag. This value is expressed in bytes. It is accessible as "--simTag" from CommandLine.
 */
static ns3::GlobalValue g_simTag ("simTag",
                                  "tag to be appended to output filenames to distinguish simulation campaigns",
                                  ns3::StringValue ("cttc-3gpp-channel-nums-fdm-output"),
                                  ns3::MakeStringChecker ());

/**
 * \brief Global variable used to configure the output results folder. This value is expressed in bytes. It is accessible as "--outputDir" from CommandLine.
 */
static ns3::GlobalValue g_outputDir ("outputDir",
                                     "directory where to store simulation results",
                                     ns3::StringValue ("./"),
                                     ns3::MakeStringChecker ());

/**
 * \brief Global variable used to configure the total TX power. This value is expressed in bytes. It is accessible as "--totalTxPower" from CommandLine.
 */
static ns3::GlobalValue g_totalTxPower ("totalTxPower",
                                       "total tx power that will be proportionally assigned to bandwidth parts depending on each BWP bandwidth ",
                                        ns3::DoubleValue (4),
                                        ns3::MakeDoubleChecker<double>());

/**
 * \brief Global variable used to configure whether to use on 1 BWP. This value is expressed in bytes. It is accessible as "--singleBwp" from CommandLine.
 */
static ns3::GlobalValue g_singleBwp ("singleBwp",
                                     "Simulate with single BWP, BWP1 configuration will be used",
                                     ns3::BooleanValue (true),
                                     ns3::MakeBooleanChecker());

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

  GlobalValue::GetValueByName("numerologyBwp1", uintegerValue); // use optional NLOS equation
  uint16_t numerologyBwp1 = uintegerValue.Get();
  GlobalValue::GetValueByName("frequencyBwp1", doubleValue); //
  double frequencyBwp1 = doubleValue.Get();
  GlobalValue::GetValueByName("bandwidthBwp1", doubleValue); //
  double bandwidthBwp1 = doubleValue.Get();
  GlobalValue::GetValueByName("numerologyBwp2", uintegerValue); // use optional NLOS equation
  uint16_t numerologyBwp2 = uintegerValue.Get();
  GlobalValue::GetValueByName("frequencyBwp2", doubleValue); //
  double frequencyBwp2 = doubleValue.Get();
  GlobalValue::GetValueByName("bandwidthBwp2", doubleValue); //
  double bandwidthBwp2 = doubleValue.Get();
  GlobalValue::GetValueByName("packetSizeUll", uintegerValue); // use optional NLOS equation
  uint32_t udpPacketSizeUll = uintegerValue.Get();
  GlobalValue::GetValueByName("packetSizeBe", uintegerValue); // use optional NLOS equation
  uint32_t udpPacketSizeBe = uintegerValue.Get();
  GlobalValue::GetValueByName("lambdaUll", uintegerValue); // use optional NLOS equation
  uint32_t lambdaUll = uintegerValue.Get();
  GlobalValue::GetValueByName("lambdaBe", uintegerValue); // use optional NLOS equation
  uint32_t lambdaBe = uintegerValue.Get();
  GlobalValue::GetValueByName ("simTag", stringValue);
  std::string simTag = stringValue.Get ();
  GlobalValue::GetValueByName ("outputDir", stringValue);
  std::string outputDir = stringValue.Get ();

  GlobalValue::GetValueByName("totalTxPower", doubleValue); // use optional NLOS equation
  double totalTxPower = doubleValue.Get();

  GlobalValue::GetValueByName("fixedMcs", uintegerValue); // use optional NLOS equation
  GlobalValue::GetValueByName("gNbNum", uintegerValue); // use optional NLOS equation
  uint16_t gNbNum = uintegerValue.Get();
  GlobalValue::GetValueByName("ueNumPergNb", uintegerValue); // use optional NLOS equation
  uint16_t ueNumPergNb = uintegerValue.Get();
  GlobalValue::GetValueByName("udpFullBuffer", booleanValue); //
  bool udpFullBuffer = booleanValue.Get();
  GlobalValue::GetValueByName("singleUeTopology", booleanValue); //
  bool singleUeTopology = booleanValue.Get();
  GlobalValue::GetValueByName("cellScan", booleanValue); //
  bool cellScan = booleanValue.Get();
  GlobalValue::GetValueByName("useFixedMcs", booleanValue); //
  GlobalValue::GetValueByName("beamSearchAngleStep", doubleValue); // use optional NLOS equation
  double beamSearchAngleStep = doubleValue.Get();
  GlobalValue::GetValueByName("singleBwp", booleanValue); //
  bool singleBwp = booleanValue.Get();

  // attributes that can be set for this channel model
  //Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Frequency", DoubleValue(frequencyBwp1));


  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("l"));

  if (singleUeTopology)
    {
    //Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon"));
      Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("RMa"));
    }
  else
    {
      //Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("InH-OfficeOpen")); // antenna height should be 1.5 m
      Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon")); // with antenna height of 10 m
    }

  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(false));

  Config::SetDefault ("ns3::MmWave3gppChannel::CellScan", BooleanValue(cellScan));
  Config::SetDefault ("ns3::MmWave3gppChannel::BeamSearchAngleStep", DoubleValue(beamSearchAngleStep));


  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  Config::SetDefault("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds(0)));
  Config::SetDefault("ns3::MmWavePointToPointEpcHelper::X2LinkDelay", TimeValue (MilliSeconds(0)));

  //Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNumDim1", UintegerValue (4));
  //Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNumDim2", UintegerValue (4));
  //Config::SetDefault("ns3::MmWaveEnbNetDevice::AntennaNumDim1", UintegerValue (16));
  //Config::SetDefault("ns3::MmWaveEnbNetDevice::AntennaNumDim2", UintegerValue (16));
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
  bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp1);

  Ptr<MmWavePhyMacCommon> phyMacCommonBwp2 = CreateObject<MmWavePhyMacCommon>();
  phyMacCommonBwp2->SetCentreFrequency(frequencyBwp2);
  phyMacCommonBwp2->SetBandwidth (bandwidthBwp2);
  phyMacCommonBwp2->SetNumerology(numerologyBwp2);

  // if not single BWP simulation add second BWP configuration
  if (!singleBwp)
    {
      bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp2);
    }

  mmWaveHelper->SetBandwidthPartMap (bwpConf);


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

  ApplicationContainer clientAppsEmbb;
  ApplicationContainer serverAppsEmbb;

  ObjectMapValue objectMapValue;

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

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }


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
          q = EpsBearer:: NGBR_LOW_LAT_EMBB;
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

  Ptr<FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
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


