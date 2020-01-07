/*
 -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*-
 *
 * A simple example of Carrier Aggregation (CA) configuration in LTE, where
 * three Component Carriers (CC) are allocated in two operation bands. CA can
 * aggregate contiguous and non-contiguous CCs. In this example, non-contiguous
 * CC are aggregated following the standard configuration CA-38A-40A-40A (Rel.14),
 * and each CC has 20 MHz bandwidth.
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
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/mmwave-mac-scheduler-tdma-rr.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("3gppChannelFdmLteComponentCarriersExample");

int 
main (int argc, char *argv[])
{
  bool udpFullBuffer = false;
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 1;
  uint16_t numFlowsUe = 2;
  bool cellScan = false;
  double beamSearchAngleStep = 10.0;
  uint32_t udpPacketSizeUll = 1000;
  uint32_t udpPacketSizeBe = 1252;
  uint32_t lambdaUll = 10000;
  uint32_t lambdaBe = 1000;
  uint8_t numBands = 2;
  bool contiguousCc = false;
  std::string simTag = "default";
  std::string outputDir = "./";
  double totalTxPower = 13;
  bool logging = false;
  uint16_t tddPattern = 15;
  bool disableDl = false;
  bool disableUl = false;

  double simTime = 1.4; // seconds
  double udpAppStartTime = 0.4; //seconds

  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("udpFullBuffer",
                "Whether to set the full buffer traffic; if this parameter is "
                "set then the udpInterval parameter will be neglected.",
                udpFullBuffer);
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
  cmd.AddValue ("numBands",
                "Number of operation bands. More than one implies non-contiguous CC",
                numBands);
  cmd.AddValue ("contiguousCc",
                "Simulate with contiguous CC or non-contiguous CC example",
                contiguousCc);
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
  cmd.AddValue ("tddPattern",
	        "LTE TDD pattern to use",
	        tddPattern);
  cmd.AddValue ("disableDl",
                "Disable DL flow",
                disableDl);
  cmd.AddValue ("disableUl",
                "Disable UL flow",
                disableUl);

  cmd.Parse (argc, argv);

  NS_ABORT_IF (numBands < 1);
  NS_ABORT_MSG_IF (disableDl==true && disableUl==true, "Enable one of the flows");

  //ConfigStore inputConfig;
  //inputConfig.ConfigureDefaults ();

  // enable logging or not
  if(logging)
    {
//      LogComponentEnable ("MmWave3gppPropagationLossModel", LOG_LEVEL_ALL);
//      LogComponentEnable ("MmWave3gppBuildingsPropagationLossModel", LOG_LEVEL_ALL);
//      LogComponentEnable ("MmWave3gppChannel", LOG_LEVEL_ALL);
//      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
//      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
//      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
//      LogComponentEnable ("BwpManagerGnb", LOG_LEVEL_INFO);
//      LogComponentEnable ("BwpManagerAlgorithm", LOG_LEVEL_INFO);
      LogComponentEnable ("MmWaveEnbPhy", LOG_LEVEL_INFO);
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

  Config::SetDefault("ns3::PointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds(0)));

  /*
   * In this example, each UE generates numFlows flows with non-repeating QCI.
   * Since Static CA Algorithm is used, each flow will be transmitted on a
   * dedicated component carrier. Therefore, the number of component carriers
   * matches the number of flows. Each carrier will multiplex flows from
   * different UEs but with the same CQI
   */
  uint8_t numCcs = numFlowsUe;

  // The maximum number of carriers in LTE Rel. 10 is 5; 32 in Rel. 15
  NS_ABORT_MSG_IF(numCcs > 5, "Please adjust the number of CCs");
  Config::SetDefault ("ns3::MmWaveHelper::NumberOfComponentCarriers",
                      UintegerValue (numCcs));

  // Define here which carrier ids will be mapped to a given QCI
  Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::NGBR_LOW_LAT_EMBB", UintegerValue (0));
  Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::GBR_CONV_VOICE", UintegerValue (1));
  Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::NGBR_VIDEO_TCP_PREMIUM", UintegerValue (2));
  Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::NGBR_VOICE_VIDEO_GAMING", UintegerValue (3));

//  Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNum", UintegerValue (4));
//  Config::SetDefault("ns3::MmWaveEnbNetDevice::AntennaNum", UintegerValue (16));
//  Config::SetDefault("ns3::MmWaveEnbPhy::TxPower", DoubleValue (txPower));

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

  /*
   * Setup the operation bands. In this example, two standard operation bands are deployed:
   * n1 and n3. n1 has a secondary carrier of 20 MHz and n3 has two non-contiguous carriers
   * of 20 MHz each, one primary carrier and another secondary carrier. Transmission of a
   * single DL UDP flow occurs on the primary carrier.
   */
  ComponentCarrierBandwidthPartCreator ccBwpManager(numBands);  //<! A first CA/BWP manager with numBands operation bands

  uint8_t ccId = 0;
  uint8_t bwpCount = 0;

  // Manually creates a non-contiguous CC configuration with 2 CCs. First CC has two BWPs and the second only one.

  //Lowerst frequency band
  OperationBandInfo band40;
  band40.m_bandId = 40;
  band40.m_centralFrequency  = 2350e6;
  band40.m_bandwidth = 100e6;
  band40.m_lowerFrequency = band40.m_centralFrequency - static_cast<double>(band40.m_bandwidth) / 2;
  band40.m_higherFrequency = band40.m_centralFrequency + static_cast<double>(band40.m_bandwidth) / 2;

  ComponentCarrierInfo cc0;
  cc0.m_ccId = 1;
  cc0.m_primaryCc = PRIMARY;
  cc0.m_centralFrequency = band40.m_lowerFrequency + 10e6;
  cc0.m_bandwidth = 18e6;
  cc0.m_lowerFrequency = cc0.m_centralFrequency - static_cast<double>(cc0.m_bandwidth) / 2;
  cc0.m_higherFrequency = cc0.m_centralFrequency + static_cast<double>(cc0.m_bandwidth) / 2;
  cc0.m_activeBwp = bwpCount;

  ComponentCarrierBandwidthPartElement bwp0;
  bwp0.m_bwpId = bwpCount;
  bwp0.m_numerology = 0;
  bwp0.m_centralFrequency = cc0.m_centralFrequency;
  bwp0.m_bandwidth = cc0.m_bandwidth;
  bwp0.m_lowerFrequency = cc0.m_lowerFrequency;
  bwp0.m_higherFrequency = cc0.m_higherFrequency;
  cc0.AddBwp(bwp0);
  ++bwpCount;

  band40.AddCc(cc0);//<! Adds the CC to the band


  ComponentCarrierInfo cc1;
  cc1.m_ccId = 2;
  cc1.m_primaryCc = SECONDARY;
  cc1.m_centralFrequency = band40.m_higherFrequency-10e6;
  cc1.m_bandwidth = 18e6;
  cc1.m_lowerFrequency = cc1.m_centralFrequency - (double)cc1.m_bandwidth/2;
  cc1.m_higherFrequency = cc1.m_centralFrequency + (double)cc1.m_bandwidth/2;
  cc1.m_activeBwp = bwpCount;

  ComponentCarrierBandwidthPartElement bwp1;
  bwp1.m_bwpId = bwpCount;
  bwp1.m_numerology = 0;
  bwp1.m_centralFrequency = cc1.m_centralFrequency;
  bwp1.m_bandwidth = cc1.m_bandwidth;
  bwp1.m_lowerFrequency = cc1.m_lowerFrequency;
  bwp1.m_higherFrequency = cc1.m_higherFrequency;
  cc1.AddBwp(bwp1);
  ++bwpCount;

  band40.AddCc(cc1); //<! Adds the CC to the band

  // Add the UE operation band to the CA/BWP manager
  ccBwpManager.AddOperationBand(band40);


  // Highest frequency band
  OperationBandInfo band38;
  band38.m_bandId = 38;
  band38.m_centralFrequency  = 2595e6;
  band38.m_bandwidth = 50e6;
  band38.m_lowerFrequency = band38.m_centralFrequency - static_cast<double> (band38.m_bandwidth) / 2;
  band38.m_higherFrequency = band38.m_centralFrequency + static_cast<double> (band38.m_bandwidth) / 2;

  // Component Carrier 1
  ComponentCarrierInfo cc2;
  cc2.m_ccId = 0;
  cc2.m_primaryCc = SECONDARY;
  cc2.m_centralFrequency = band38.m_centralFrequency;
  cc2.m_bandwidth = 18e6;
  cc2.m_lowerFrequency = cc2.m_centralFrequency - static_cast<double> (cc2.m_bandwidth) / 2;
  cc2.m_higherFrequency = cc2.m_centralFrequency + static_cast<double> (cc2.m_bandwidth) / 2;
  cc2.m_activeBwp = bwpCount;
  // The NR module works with BWPs, so yous should create it occupying the whole CC
  ComponentCarrierBandwidthPartElement bwp2;
  bwp2.m_bwpId = bwpCount;
  bwp2.m_numerology = 0;
  bwp2.m_centralFrequency = cc2.m_centralFrequency;
  bwp2.m_bandwidth = cc2.m_bandwidth;
  bwp2.m_lowerFrequency = cc2.m_lowerFrequency;
  bwp2.m_higherFrequency = cc2.m_higherFrequency;
  cc2.AddBwp (bwp2);
  ++bwpCount;

  // Add the CC to the band
  band38.AddCc (cc2);

  // Add the UE operation band to the CA/BWP manager
  ccBwpManager.AddOperationBand (band38);


  // Check that the CA/BWP configurations are correct
  ccBwpManager.ValidateCaBwpConfiguration ();

  // Plot operation bands
  ccBwpManager.PlotLteCaConfiguration ("TestLteBandPlot");


  // Create BandwidthPartRepresentations referred to the active BWP only of each CC
  Ptr<MmWavePhyMacCommon> phyMacCommonBwp0 = CreateObject<MmWavePhyMacCommon> ();
  phyMacCommonBwp0->SetNumRbPerRbg (4); //<! Just to force the RBG size for the given CC bandwidth
  ComponentCarrierBandwidthPartElement recBwp0 = bwp0;
  phyMacCommonBwp0->SetCentreFrequency (recBwp0.m_centralFrequency);
  phyMacCommonBwp0->SetBandwidth (recBwp0.m_bandwidth);
  phyMacCommonBwp0->SetNumerology ((uint32_t)recBwp0.m_numerology);
  phyMacCommonBwp0->SetAttribute ("MacSchedulerType", TypeIdValue (MmWaveMacSchedulerTdmaRR::GetTypeId ()));
  phyMacCommonBwp0->SetCcId (ccId);
  BandwidthPartRepresentation repr0 (ccId, phyMacCommonBwp0, nullptr, nullptr, nullptr);
  mmWaveHelper->AddBandwidthPart (ccId, repr0);
  ++ccId;
  std::cout << "CC0" << std::endl;
  std::cout << "  Central frequency (MHz): " << recBwp0.m_centralFrequency/1000/1000 << std::endl;
  std::cout << "  Bandwidth (MHz): " << recBwp0.m_bandwidth/1000/1000 << std::endl;

  Ptr<MmWavePhyMacCommon> phyMacCommonBwp1 = CreateObject<MmWavePhyMacCommon>();
  phyMacCommonBwp1->SetNumRbPerRbg (4); //<! Just to force the RBG size for the given CC bandwidth
  ComponentCarrierBandwidthPartElement recBwp1 = bwp1;
  phyMacCommonBwp1->SetCentreFrequency (recBwp1.m_centralFrequency);
  phyMacCommonBwp1->SetBandwidth (recBwp1.m_bandwidth);
  phyMacCommonBwp1->SetNumerology ((uint32_t)recBwp1.m_numerology);
  phyMacCommonBwp1->SetAttribute ("MacSchedulerType", TypeIdValue (MmWaveMacSchedulerTdmaRR::GetTypeId ()));
  phyMacCommonBwp1->SetCcId (ccId);
  BandwidthPartRepresentation repr1 (ccId, phyMacCommonBwp1, nullptr, nullptr, nullptr);
  mmWaveHelper->AddBandwidthPart (ccId, repr1);
  ++ccId;
  std::cout << "CC1" << std::endl;
  std::cout << "  Central frequency (MHz): " << recBwp1.m_centralFrequency/1000/1000 << std::endl;
  std::cout << "  Bandwidth (MHz): " << recBwp1.m_bandwidth/1000/1000 << std::endl;

  //Create the third CC and more if they are going to be used to convey data
//  Ptr<MmWavePhyMacCommon> phyMacCommonBwp2 = CreateObject<MmWavePhyMacCommon>();
//  phyMacCommonBwp2->SetNumRbPerRbg (4); //<! Just to force the RBG size for the given CC bandwidth
//  ComponentCarrierBandwidthPartElement recBwp2 = bwp2;
//  phyMacCommonBwp2->SetCentreFrequency(recBwp2.m_centralFrequency);
//  phyMacCommonBwp2->SetBandwidth (recBwp2.m_bandwidth);
//  phyMacCommonBwp2->SetNumerology((uint32_t)recBwp2.m_numerology);
//  phyMacCommonBwp2->SetAttribute ("MacSchedulerType", TypeIdValue (MmWaveMacSchedulerTdmaRR::GetTypeId ()));
//  phyMacCommonBwp2->SetCcId (ccId);
//  BandwidthPartRepresentation repr2 (ccId, phyMacCommonBwp2, nullptr, nullptr, nullptr);
//  mmWaveHelper->AddBandwidthPart (ccId, repr2);
//  ++ccId;
//  std::cout << "CC2" << std::endl;
//  std::cout << "  Central frequency (MHz): " << recBwp2.m_centralFrequency/1000/1000 << std::endl;
//  std::cout << "  Bandwidth (MHz): " << recBwp2.m_bandwidth/1000/1000 << std::endl;


  NS_ABORT_MSG_IF(ccId < 1,"No CC created");
  mmWaveHelper->SetAttribute("NumberOfComponentCarriers", UintegerValue(ccId));

  // Enable CA if there are more than one component carrier. Disabled by default
  if (ccId > 0)
    {
      mmWaveHelper->SetAttribute ("UseCa", BooleanValue (true));
    }

  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  mmWaveHelper->SetEpcHelper (epcHelper);
  mmWaveHelper->Initialize();

  // install mmWave net devices
  NetDeviceContainer enbNetDev = mmWaveHelper->InstallEnbDevice (gNbNodes);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNodes);

  // All LTE TDD patterns are defined here
  std::vector<LteNrTddSlotType> ltePattern0 {
    LteNrTddSlotType::DL, LteNrTddSlotType::S, LteNrTddSlotType::UL,
    LteNrTddSlotType::UL, LteNrTddSlotType::UL, LteNrTddSlotType::DL,
    LteNrTddSlotType::S, LteNrTddSlotType::UL, LteNrTddSlotType::UL,
    LteNrTddSlotType::UL};
  std::vector<LteNrTddSlotType> ltePattern1 {
    LteNrTddSlotType::DL, LteNrTddSlotType::S, LteNrTddSlotType::UL,
    LteNrTddSlotType::UL, LteNrTddSlotType::DL, LteNrTddSlotType::DL,
    LteNrTddSlotType::S, LteNrTddSlotType::UL, LteNrTddSlotType::UL,
    LteNrTddSlotType::DL};
  std::vector<LteNrTddSlotType> ltePattern2 {
    LteNrTddSlotType::DL, LteNrTddSlotType::S, LteNrTddSlotType::UL,
    LteNrTddSlotType::DL, LteNrTddSlotType::DL, LteNrTddSlotType::DL,
    LteNrTddSlotType::S, LteNrTddSlotType::UL, LteNrTddSlotType::DL,
    LteNrTddSlotType::DL};
  std::vector<LteNrTddSlotType> ltePattern3 {
    LteNrTddSlotType::DL, LteNrTddSlotType::S, LteNrTddSlotType::UL,
    LteNrTddSlotType::UL, LteNrTddSlotType::UL, LteNrTddSlotType::DL,
    LteNrTddSlotType::DL, LteNrTddSlotType::DL, LteNrTddSlotType::DL,
    LteNrTddSlotType::DL};
  std::vector<LteNrTddSlotType> ltePattern4 {
    LteNrTddSlotType::DL, LteNrTddSlotType::S, LteNrTddSlotType::UL,
    LteNrTddSlotType::UL, LteNrTddSlotType::DL, LteNrTddSlotType::DL,
    LteNrTddSlotType::DL, LteNrTddSlotType::DL, LteNrTddSlotType::DL,
    LteNrTddSlotType::DL};
  std::vector<LteNrTddSlotType> ltePattern5 {
    LteNrTddSlotType::DL, LteNrTddSlotType::S, LteNrTddSlotType::UL,
    LteNrTddSlotType::DL, LteNrTddSlotType::DL, LteNrTddSlotType::DL,
    LteNrTddSlotType::DL, LteNrTddSlotType::DL, LteNrTddSlotType::DL,
    LteNrTddSlotType::DL};
  std::vector<LteNrTddSlotType> ltePattern6 {
    LteNrTddSlotType::DL, LteNrTddSlotType::S, LteNrTddSlotType::UL,
    LteNrTddSlotType::UL, LteNrTddSlotType::UL, LteNrTddSlotType::DL,
    LteNrTddSlotType::S, LteNrTddSlotType::UL, LteNrTddSlotType::UL,
    LteNrTddSlotType::DL};
  std::vector<LteNrTddSlotType> ltePatternFs {
    LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F,
    LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F,
    LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F,
    LteNrTddSlotType::F};
  std::vector<LteNrTddSlotType> ltePattern;
  switch (tddPattern)
  {
    case 0:
      ltePattern = ltePattern0;
      break;
    case 1:
      ltePattern = ltePattern1;
      break;
    case 2:
      ltePattern = ltePattern2;
      break;
    case 3:
      ltePattern = ltePattern3;
      break;
    case 4:
      ltePattern = ltePattern4;
      break;
    case 5:
      ltePattern = ltePattern5;
      break;
    case 6:
      ltePattern = ltePattern6;
      break;
    case 15:	//<! All 'F's pattern to get kind of FDD operation
      ltePattern = ltePatternFs;
      break;
    default:
      NS_ABORT_MSG("Wrong LTE pattern id");
  }
  // Set the TDD patterns for all the component carriers you've created
  // Example: I want to set the ltePattern on bwp 0 of gnb 0
  mmWaveHelper->GetEnbPhy (enbNetDev.Get(0), 0)->SetTddPattern (ltePattern);
  mmWaveHelper->GetEnbPhy (enbNetDev.Get(0), 1)->SetTddPattern (ltePattern);
//  mmWaveHelper->GetEnbPhy (enbNetDev.Get(0), 2)->SetTddPattern (ltePattern);


  // Share the total transmission power among CCs proportionally with the BW
  double x = pow(10, totalTxPower/10);

  double totalBandwidth = ccBwpManager.GetAggregatedBandwidth();

  ObjectMapValue objectMapValue;
  // set tx power of gNBs
  for (uint32_t j = 0; j < enbNetDev.GetN(); ++j)
    {
      Ptr<MmWaveEnbNetDevice> netDevice = DynamicCast<MmWaveEnbNetDevice>(enbNetDev.Get(j));
      netDevice->GetAttribute("ComponentCarrierMap", objectMapValue);
      for (uint32_t i = 0; i < objectMapValue.GetN(); i++)
        {
          Ptr<ComponentCarrierGnb> bandwidthPart = DynamicCast<ComponentCarrierGnb>(objectMapValue.Get(i));
          uint32_t bwCc = ccBwpManager.GetCarrierBandwidth(i);
          bandwidthPart->GetPhy()->SetTxPower(10*log10((bwCc/totalBandwidth)*x));
          std::cout<<"\n txPower" << i <<" = "<<10*log10((bwCc/totalBandwidth)*x)<<std::endl;
        }
    }

//  // set tx power of UE devices
//  for (uint32_t j = 0; j < ueNetDev.GetN(); ++j)
//    {
//      Ptr<MmWaveUeNetDevice> netDevice = DynamicCast<MmWaveUeNetDevice>(ueNetDev.Get(j));
//      netDevice->GetAttribute("ComponentCarrierMapUe", objectMapValue);
//      for (uint32_t i = 0; i < objectMapValue.GetN(); i++)
//        {
//          Ptr<ComponentCarrierMmWaveUe> bandwidthPart = DynamicCast<ComponentCarrierMmWaveUe>(objectMapValue.Get(i));
//          uint32_t bwCc = ccBwpManager.GetCarrierBandwidth(i);
//          bandwidthPart->GetPhy()->SetTxPower(10*log10((bwCc/totalBandwidth)*x));
//        }
//    }

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

  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to the closest eNB
  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // install UDP applications
  uint16_t dlPort = 1234;
  uint16_t ulPort = dlPort + gNbNum * ueNumPergNb * numFlowsUe + 1;
  ApplicationContainer clientApps, serverApps;

  for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
    {
      for (uint16_t flow = 0; flow < numFlowsUe; ++flow)
        {
          if (!disableDl)
            {
              PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
              serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));

              UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
              dlClient.SetAttribute("PacketSize", UintegerValue(udpPacketSizeUll));
              dlClient.SetAttribute ("Interval", TimeValue (Seconds(1.0/lambdaUll)));
              dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
              clientApps.Add (dlClient.Install (remoteHost));

              Ptr<EpcTft> tft = Create<EpcTft> ();
              EpcTft::PacketFilter dlpf;
              dlpf.localPortStart = dlPort;
              dlpf.localPortEnd = dlPort;
              ++dlPort;
              tft->Add (dlpf);

              enum EpsBearer::Qci q;
              if (flow == 0)
                {
                  q = EpsBearer::NGBR_LOW_LAT_EMBB;
                }
              else if (flow == 1)
                {
                  q = EpsBearer::GBR_CONV_VOICE;
                }
              else if (flow == 2)
                {
                  q = EpsBearer::NGBR_VIDEO_TCP_PREMIUM;
                }
              else if (flow == 3)
                {
                  q = EpsBearer::NGBR_VOICE_VIDEO_GAMING;
                }
              else
                {
                  q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
                }
              EpsBearer bearer (q);
              mmWaveHelper->ActivateDedicatedEpsBearer(ueNetDev.Get(u), bearer, tft);
            }

          if (!disableUl)
            {
              PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
              serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

              UdpClientHelper ulClient (remoteHostAddr, ulPort);
              ulClient.SetAttribute("PacketSize", UintegerValue(udpPacketSizeUll));
              ulClient.SetAttribute ("Interval", TimeValue (Seconds(1.0/lambdaUll)));
              ulClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
              clientApps.Add (ulClient.Install (ueNodes.Get(u)));

              Ptr<EpcTft> tft = Create<EpcTft> ();
              EpcTft::PacketFilter ulpf;
              ulpf.localPortStart = ulPort;
              ulpf.localPortEnd = ulPort;
              ++ulPort;
              tft->Add (ulpf);

              enum EpsBearer::Qci q;
              if (flow == 0)
                {
                  q = EpsBearer::NGBR_LOW_LAT_EMBB;
                }
              else if (flow == 1)
                {
                  q = EpsBearer::GBR_CONV_VOICE;
                }
              else if (flow == 2)
                {
                  q = EpsBearer::NGBR_VIDEO_TCP_PREMIUM;
                }
              else if (flow == 3)
                {
                  q = EpsBearer::NGBR_VOICE_VIDEO_GAMING;
                }
              else
                {
                  q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
                }
              EpsBearer bearer (q);
              mmWaveHelper->ActivateDedicatedEpsBearer(ueNetDev.Get(u), bearer, tft);
            }

        }
    }

  // start UDP server and client apps
  serverApps.Start(Seconds(udpAppStartTime));
  clientApps.Start(Seconds(udpAppStartTime));
  serverApps.Stop(Seconds(simTime));
  clientApps.Stop(Seconds(simTime));

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

  outFile << "\n\n  Aggregated throughput: " << averageFlowThroughput << "\n";
  outFile << "  Mean flow throughput: " << averageFlowThroughput / stats.size() << "\n";
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


