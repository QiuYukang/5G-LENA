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
#include "ns3/bandwidth-part-gnb.h"
#include "ns3/nr-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("3gppChannelFdmLteComponentCarriersExample");


int 
main (int argc, char *argv[])
{
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 1;
  uint16_t numFlowsUe = 2;

  uint8_t numBands = 2;
  bool contiguousCc = false;
  double centralFrequencyBand40 = 2350e6;
  double bandwidthBand40 = 100e6;
  double centralFrequencyBand38 = 2595e6;
  double bandwidthBand38 = 50e6;

  //FDD CC parameters
  double bandwidthCc0 = 18e6;
  double bandwidthCc1 = 18e6;

  uint16_t numerologyBwp0 = 0;
  uint16_t numerologyBwp1 = 0;
  uint16_t numerologyBwp2 = 0;

  double totalTxPower = 13;
  std::string pattern = "F|F|F|F|F|F|F|F|F|F|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"
  std::string patternDL = "DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|";
  std::string patternUL = "UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|";
  std::string operationMode = "FDD";  // TDD or FDD

  bool cellScan = false;
  double beamSearchAngleStep = 10.0;

  bool udpFullBuffer = false;
  uint32_t udpPacketSizeUll = 1000;
  uint32_t udpPacketSizeBe = 1252;
  uint32_t lambdaUll = 10000;
  uint32_t lambdaBe = 1000;

  bool disableDl = false;
  bool disableUl = false;

  bool logging = false;

  std::string simTag = "default";
  std::string outputDir = "./";

  double simTime = 1.4; // seconds
  double udpAppStartTime = 0.4; //seconds



  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("gNbNum",
                "The number of gNbs in multiple-ue topology",
                gNbNum);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per gNb in multiple-ue topology",
                ueNumPergNb);
  cmd.AddValue ("numBands",
                "Number of operation bands. More than one implies non-contiguous CC",
                numBands);
  cmd.AddValue ("contiguousCc",
                "Simulate with contiguous CC or non-contiguous CC example",
                contiguousCc);
  cmd.AddValue ("bandwidthBand40",
                "The system bandwidth to be used in band 1",
                bandwidthBand40);
  cmd.AddValue ("bandwidthBand38",
                "The system bandwidth to be used in band 1",
                bandwidthBand38);
  cmd.AddValue ("bandwidthCc0",
                "The bandwidth to be used in CC 0",
                bandwidthCc0);
  cmd.AddValue ("bandwidthCc1",
                "The bandwidth to be used in CC 1",
                bandwidthCc1);
  cmd.AddValue ("numerologyBwp0",
                "The numerology to be used in bandwidth part 1",
                numerologyBwp0);
  cmd.AddValue ("numerologyBwp1",
                "The numerology to be used in bandwidth part 1",
                numerologyBwp1);
  cmd.AddValue ("numerologyBwp2",
                "The numerology to be used in bandwidth part 2",
                numerologyBwp2);
  cmd.AddValue ("totalTxPower",
                "total tx power that will be proportionally assigned to"
                " bandwidth parts depending on each BWP bandwidth ",
                totalTxPower);
  cmd.AddValue ("tddPattern",
                "LTE TDD pattern to use (e.g. --tddPattern=DL|S|UL|UL|UL|DL|S|UL|UL|UL|)",
                pattern);
  cmd.AddValue ("operationMode",
                "The network operation mode can be TDD or FDD",
                operationMode);
  cmd.AddValue ("cellScan",
                "Use beam search method to determine beamforming vector,"
                "true to use cell scanning method",
                cellScan);
  cmd.AddValue ("beamSearchAngleStep",
                "Beam search angle step for beam search method",
                beamSearchAngleStep);
  cmd.AddValue ("udpFullBuffer",
                "Whether to set the full buffer traffic; if this parameter is "
                "set then the udpInterval parameter will be neglected.",
                udpFullBuffer);
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
  cmd.AddValue ("disableDl",
                "Disable DL flow",
                disableDl);
  cmd.AddValue ("disableUl",
                "Disable UL flow",
                disableUl);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);

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
      LogComponentEnable ("MmWaveUePhy", LOG_LEVEL_INFO);
//      LogComponentEnable ("MmWaveEnbMac", LOG_LEVEL_INFO);
//      LogComponentEnable ("MmWaveUeMac", LOG_LEVEL_INFO);
    }

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
   Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();

  mmWaveHelper->SetIdealBeamformingHelper (idealBeamformingHelper);
  mmWaveHelper->SetEpcHelper (epcHelper);

  mmWaveHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));
  if (cellScan)
  {
    idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
    idealBeamformingHelper->SetIdealBeamFormingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (beamSearchAngleStep));
  }
  else
  {
    idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));
  }
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));

  /*
   * Setup the operation bands.
   * In this example, two standard operation bands are deployed:
   * Band 38 that has a component carrier (CC) of 20 MHz
   * Band 40 that has two non-contiguous CCs of 20 MHz each.
   * If TDD mode is defined, 1 BWP per CC is created.
   * If FDD mode is defined, Band 40 CC0 containes 2 BWPs (1 DL - 1 UL)
   *
   * This example manually creates a non-contiguous CC configuration with 2 CCs.
   * First CC has two BWPs and the second only one.
   *
   * The configured spectrum division for TDD mode is:
   * |------------- Band 40 -----------|   |------------- Band 38 ------------|
   * |----- CC0-----|   |----- CC1-----|        |--------- CC2 ----------|
   * |---- BWP0 ----|   |---- BWP1 ----|        |--------- BWP2 ---------|
   *
   * The configured spectrum division for FDD mode is:
   * |----------------- Band 40 ----------------|   |------------- Band 38 ------------|
   * |-------- CC0--------|  |------- CC1-------|        |--------- CC2 ----------|
   * |- BWP0DL -|- BWP0UL-|  |------ BWP1 ------|        |--------- BWP2 ---------|
   *
   *
   * In this example, each UE generates numFlows flows with non-repeating QCI.
   * Since Static CA Algorithm is used, each flow will be transmitted on a
   * dedicated component carrier. Therefore, the number of component carriers
   * matches the number of flows. Each carrier will multiplex flows from
   * different UEs but with the same CQI
   */
  //uint8_t numCcs = numFlowsUe;

  uint8_t numCcBand40;
  uint8_t numCcBand38 = 1;

  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;

  OperationBandInfo band40;
  //FDD case
  std::unique_ptr<ComponentCarrierInfo> cc0 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp0dl (new BandwidthPartInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp0ul (new BandwidthPartInfo ());

  std::unique_ptr<ComponentCarrierInfo> cc1 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp1 (new BandwidthPartInfo ());


  // Create the configuration for band40
  if (operationMode == "TDD")
  {
      numCcBand40 = 2;

      CcBwpCreator::SimpleOperationBandConf bandConf40 (centralFrequencyBand40, bandwidthBand40, numCcBand40, BandwidthPartInfo::UMi_StreetCanyon_LoS);
      band40 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf40);
      mmWaveHelper->InitializeOperationBand (&band40);
  }
  else  //FDD case
  {
      numCcBand40 = 3;

      band40.m_centralFrequency  = centralFrequencyBand40;
      band40.m_channelBandwidth = bandwidthBand40;
      band40.m_lowerFrequency = band40.m_centralFrequency - band40.m_channelBandwidth / 2;
      band40.m_higherFrequency = band40.m_centralFrequency + band40.m_channelBandwidth / 2;

      uint8_t bwpCount = 0;

      // Component Carrier 0
      cc0->m_ccId = 0;
      cc0->m_centralFrequency = band40.m_lowerFrequency + 10e6;
      cc0->m_channelBandwidth = bandwidthCc0;
      cc0->m_lowerFrequency = cc0->m_centralFrequency - cc0->m_channelBandwidth / 2;
      cc0->m_higherFrequency = cc0->m_centralFrequency + cc0->m_channelBandwidth / 2;
      // BWP 0 DL
      bwp0dl->m_bwpId = bwpCount;
      bwp0dl->m_channelBandwidth = cc0->m_channelBandwidth / 2;
      bwp0dl->m_lowerFrequency = cc0->m_lowerFrequency;
      bwp0dl->m_higherFrequency = cc0->m_lowerFrequency + bwp0dl->m_channelBandwidth;
      bwp0dl->m_centralFrequency = bwp0dl->m_lowerFrequency +
                (bwp0dl->m_higherFrequency - bwp0dl->m_lowerFrequency) / 2;
      ++bwpCount;
      // BWP 0 UL
      bwp0ul->m_bwpId = bwpCount;
      bwp0ul->m_channelBandwidth = cc0->m_channelBandwidth - bwp0dl->m_channelBandwidth;
      bwp0ul->m_lowerFrequency = bwp0dl->m_higherFrequency;
      bwp0ul->m_higherFrequency = cc0->m_higherFrequency;
      bwp0ul->m_centralFrequency = bwp0ul->m_lowerFrequency +
                (bwp0ul->m_higherFrequency - bwp0ul->m_lowerFrequency) / 2;
      ++bwpCount;

      cc0->AddBwp (std::move(bwp0dl));
      cc0->AddBwp (std::move(bwp0ul));

      // Component Carrier 1
      cc1->m_ccId = 0;
      cc1->m_centralFrequency = band40.m_higherFrequency-10e6;
      cc1->m_channelBandwidth = bandwidthCc1;
      cc1->m_lowerFrequency = cc1->m_centralFrequency - cc1->m_channelBandwidth / 2;
      cc1->m_higherFrequency = cc1->m_centralFrequency + cc1->m_channelBandwidth / 2;
      // BWP 1
      bwp1->m_bwpId = bwpCount;
      bwp1->m_centralFrequency = cc1->m_centralFrequency;
      bwp1->m_channelBandwidth = cc1->m_channelBandwidth;
      bwp1->m_lowerFrequency = cc1->m_lowerFrequency;
      bwp1->m_higherFrequency = cc1->m_higherFrequency;
      ++bwpCount;

      cc1->AddBwp (std::move(bwp1));

      band40.AddCc (std::move(cc1));
      band40.AddCc (std::move(cc0));

  }

  // Create the configuration for band38 (CC2 - BWP2)
  CcBwpCreator::SimpleOperationBandConf bandConf38 (centralFrequencyBand38, bandwidthBand38, numCcBand38, BandwidthPartInfo::UMi_StreetCanyon_LoS);
  OperationBandInfo band38 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf38);

  mmWaveHelper->InitializeOperationBand (&band40);
  mmWaveHelper->InitializeOperationBand (&band38);

  allBwps = CcBwpCreator::GetAllBwps ({band38, band40});

  // Antennas for all the UEs
  mmWaveHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  mmWaveHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  mmWaveHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (true));

  // Antennas for all the gNbs
  mmWaveHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  mmWaveHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  mmWaveHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (true));

  uint32_t bwpIdForLowLat = 0;
  uint32_t bwpIdForVoice = 1;
  uint32_t bwpIdForVideo = 2;
  uint32_t bwpIdForVideoGaming = 3;

  mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
  mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VOICE_VIDEO_GAMING", UintegerValue (bwpIdForVideoGaming));

  mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
  mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VOICE_VIDEO_GAMING", UintegerValue (bwpIdForVideoGaming));

  // Enable CA if there are more than one component carrier. Disabled by default
/*  if (ccId > 0)
    {
      mmWaveHelper->SetAttribute ("UseCa", BooleanValue (true));
    }
*/

  // install mmWave net devices
  NetDeviceContainer enbNetDev = mmWaveHelper->InstallGnbDevice (gNbNodes, allBwps);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNodes, allBwps);


  // Share the total transmission power among CCs proportionally with the BW
  double x = pow(10, totalTxPower/10);
  double totalBandwidth = bandwidthBand40 + bandwidthBand38;;

  if (operationMode == "TDD")
    {
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp0));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower",
                                 DoubleValue (10 * log10 ((band40.GetBwpAt (0, 0)->m_channelBandwidth/totalBandwidth) * x)));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Pattern", StringValue (pattern));

      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (numerologyBwp1));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("TxPower",
                                 DoubleValue (10 * log10 ((band40.GetBwpAt (1, 0)->m_channelBandwidth/totalBandwidth) * x)));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Pattern", StringValue (pattern));

      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("Numerology", UintegerValue (numerologyBwp2));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("TxPower",
                                 DoubleValue (10 * log10 ((band38.GetBwpAt (0, 0)->m_channelBandwidth/totalBandwidth) * x)));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("Pattern", StringValue (pattern));
    }
  else  //FDD case
    {
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp0));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower",
                                 DoubleValue (10 * log10 ((band40.GetBwpAt (0, 0)->m_channelBandwidth/totalBandwidth) * x)));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Pattern", StringValue (patternDL));

      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (numerologyBwp0));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("TxPower", DoubleValue (0.0));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Pattern", StringValue (patternUL));

      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("Numerology", UintegerValue (numerologyBwp1));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("TxPower",
                                 DoubleValue (10 * log10 ((band40.GetBwpAt (1, 0)->m_channelBandwidth/totalBandwidth) * x)));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("Pattern", StringValue (pattern));

      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 3)->SetAttribute ("Numerology", UintegerValue (numerologyBwp2));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 3)->SetAttribute ("TxPower",
                                 DoubleValue (10 * log10 ((band38.GetBwpAt (0, 0)->m_channelBandwidth/totalBandwidth) * x)));
      mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 3)->SetAttribute ("Pattern", StringValue (pattern));

      // Link the two FDD BWP:
      mmWaveHelper->GetBwpManagerGnb (enbNetDev.Get (0))->SetOutputLink (1, 0);
    }

  // Set the UE routing:
  for (uint32_t i = 0; i < ueNetDev.GetN (); i++)
    {
      mmWaveHelper->GetBwpManagerUe (ueNetDev.Get (i))->SetOutputLink (0, 1);
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


  /*
  *  In FDD, DL and UL might not be symmetric. A simple way to automatically set
  *  BWP powers is to loop all PHYs and apply the BW of the attached BWP
  */
/*  std::vector<Ptr<BandwidthPartInfo>> bwpList;
  ccBwpManager.GetConfiguredBwp (&bwpList);

  for (uint32_t j = 0; j < enbNetDev.GetN (); ++j)
    {
      ObjectMapValue objectMapValue;
      enbNetDev.Get(j)->GetAttribute("BandwidthPartMap", objectMapValue);
      for (uint32_t i = 0; i < objectMapValue.GetN(); i++)
        {
          Ptr<BandwidthPartGnb> bandwidthPart = DynamicCast<BandwidthPartGnb>(objectMapValue.Get(i));
          uint8_t bwdId = bandwidthPart->GetPhy ()->GetConfigurationParameters ()->GetCcId ();
          uint32_t bw = (bwpList.at (bwdId))->m_bandwidth;
          bandwidthPart->GetPhy ()->SetTxPower (10 * log10 ((bw / totalBandwidth) * x));
          std::cout << "\n txPower" << i << " = " << 10 * log10 ((bw / totalBandwidth) * x) << std::endl;
        }
    }
*/

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
              ulpf.remotePortStart = ulPort;
              ulpf.remotePortEnd = ulPort;
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
  mmWaveHelper->EnableTraces();


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
   * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> MmWaveEnbPhy -> MmWavePhyMacCommong-> Numerology, Bandwidth, ...
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


