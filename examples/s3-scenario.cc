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
$ ./waf --run "s3-scenario --Help"
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
NS_LOG_COMPONENT_DEFINE ("S3Scenario");


class RadioNetworkParametersHelper
{
public:

  /**
   * \brief Set the radio network parameters to LTE.
   * \param freqReuse The cell frequency reuse.
   */
  void SetNetworkToLte (const std::string scenario,
                        const std::string operationMode,
                        uint16_t numCcs);

  /**
   * \brief Set the radio network parameters to NR.
   * \param scenario Urban scenario (UMa or UMi).
   * \param numerology Numerology to use.
   * \param freqReuse The cell frequency reuse.
   */
  void SetNetworkToNr (const std::string scenario,
                       const std::string operationMode,
                       uint16_t numerology,
                       uint16_t numCcs);

  /**
   * \brief Gets the BS transmit power
   * \return Transmit power in dBW
   */
  double GetTxPower ();

  /**
   * \brief Gets the operation bandwidth
   * \return Bandwidth in Hz
   */
  double GetBandwidth ();

  /**
   * \brief Gets the central frequency
   * \return Central frequency in Hz
   */
  double GetCentralFrequency ();

  /**
   * \brief Gets the band numerology
   * \return Numerology
   */
  uint16_t GetNumerology ();

private:
  double m_txPower {-1.0};            //!< Transmit power in dBm
  double m_bandwidth {0.0};           //!< System bandwidth in Hz
  double m_centralFrequency {-1.0};   //!< Band central frequency in Hz
  uint16_t m_numerology {0};          //!< Operation band numerology
};

void
RadioNetworkParametersHelper::SetNetworkToLte (const std::string scenario,
                                               const std::string operationMode,
                                               uint16_t numCcs)
{
  NS_ABORT_MSG_IF (scenario != "UMa" && scenario != "UMi",
                   "Unsupported scenario");

  m_numerology = 0;
  m_centralFrequency = 2e9;
  m_bandwidth = 18e6 * numCcs;  // 100 RBs per CC (freqReuse)
  if (operationMode == "FDD")
    {
      m_bandwidth += m_bandwidth;
    }
  if (scenario == "UMa")
    {
      m_txPower = 49;
    }
  else
    {
      m_txPower = 44;
    }
}

void
RadioNetworkParametersHelper::SetNetworkToNr (const std::string scenario,
                                              const std::string operationMode,
                                              uint16_t numerology,
                                              uint16_t numCcs)
{
  NS_ABORT_MSG_IF (scenario != "UMa" && scenario != "UMi",
                   "Unsupported scenario");

  m_numerology = numerology;
  m_centralFrequency = 2e9;
  m_bandwidth = 18e6 * numCcs;  // 100 RBs per CC (freqReuse)
  if (operationMode == "FDD")
    {
      m_bandwidth += m_bandwidth;
    }
  if (scenario == "UMa")
    {
      m_txPower = 49;
    }
  else
    {
      m_txPower = 44;
    }
}

double
RadioNetworkParametersHelper::GetTxPower ()
{
  return m_txPower;
}

double
RadioNetworkParametersHelper::GetBandwidth ()
{
  return m_bandwidth;
}

double
RadioNetworkParametersHelper::GetCentralFrequency ()
{
  return m_centralFrequency;
}

uint16_t
RadioNetworkParametersHelper::GetNumerology ()
{
  return m_numerology;
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
  std::string scenario = "UMa";
  std::string radioNetwork = "NR";  // LTE or NR
  std::string operationMode = "TDD";  // TDD or FDD

  // Traffic parameters (that we will use inside this script:)
  uint32_t udpPacketSize = 600;
  uint32_t lambda = 10000;

  // Simulation parameters. Please don't use double to indicate seconds, use
  // milliseconds and integers to avoid representation errors.
  uint32_t simTimeMs = 1400;
  uint32_t udpAppStartTimeMs = 400;
  std::string direction = "DL";

  // Spectrum parameters. We will take the input from the command line, and then
  //  we will pass them inside the NR module.
  uint16_t numerologyBwp = 0;
  double centralFrequencyBand = 0.0;  // RadioNetworkParametersHelper provides this hard-coded value
  double bandwidthBand = 0.0;  // RadioNetworkParametersHelper provides this hard-coded values
  std::string pattern = "F|F|F|F|F|F|F|F|F|F|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"

  // Where we will store the output files.
  std::string simTag = "default";
  std::string outputDir = "./";

  // Error models
  std::string errorModel = "";

  /*
   * From here, we instruct the ns3::CommandLine class of all the input parameters
   * that we may accept as input, as well as their description, and the storage
   * variable.
   */
  CommandLine cmd;

  cmd.AddValue ("scenario",
                "The urban scenario string (UMa or UMi)",
                scenario);
  cmd.AddValue ("numRings",
                "The number of rings around the central site",
                numOuterRings);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per cell or gNB in multiple-ue topology",
                ueNumPergNb);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.AddValue ("packetSize",
                "packet size in bytes to be used by UE traffic",
                udpPacketSize);
  cmd.AddValue ("lambda",
                "Number of UDP packets generated in one second per UE",
                lambda);
  cmd.AddValue ("simTimeMs",
                "Simulation time",
                simTimeMs);
  cmd.AddValue ("numerologyBwp",
                "The numerology to be used (NR only)",
                numerologyBwp);
  cmd.AddValue ("pattern",
                "The TDD pattern to use",
                pattern);
  cmd.AddValue ("direction",
                "The flow direction (DL or UL)",
                direction);
//  cmd.AddValue ("centralFrequencyBand",
//                "The system frequency to be used in band 1",
//                centralFrequencyBand);
//  cmd.AddValue ("bandwidthBand",
//                "The system bandwidth to be used in band 1",
//                bandwidthBand);
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

  // Parse the command line
  cmd.Parse (argc, argv);

  /*
   * Check if the frequency and numerology are in the allowed range.
   * If you need to add other checks, here is the best position to put them.
   */
  NS_ABORT_IF (centralFrequencyBand > 100e9);
  NS_ABORT_IF (numerologyBwp > 4);
  NS_ABORT_MSG_IF (direction != "DL" && direction != "UL", "Flow direction can only be DL or UL");
  NS_ABORT_MSG_IF (operationMode != "TDD" && operationMode != "FDD", "Operation mode can only be TDD or FDD");
  NS_ABORT_MSG_IF (radioNetwork != "LTE" && radioNetwork != "NR", "Unrecognized radio network technology");

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
//      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
//      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
//      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
      LogComponentEnable ("MmWaveMacSchedulerOfdma", LOG_LEVEL_ALL);
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
  HexagonalGridScenarioHelper gridScenario;
  gridScenario.SetNumRings (numOuterRings);
  gridScenario.SetScenarioParamenters (scenario);
  gridScenario.SetNumCells ();  // Note that the call takes no arguments since the number is obtained from the parameters in SetUMaParameters or SetUMiParameters
  uint16_t gNbNum = gridScenario.GetNumCells ();
  uint32_t ueNum = ueNumPergNb * gNbNum;
  gridScenario.SetUtNumber (ueNum);
  gridScenario.CreateScenario ();  //!< Creates and plots the network deployment
  const uint16_t ffr = 3; // Fractional Frequency Reuse scheme to mitigate intra-site inter-sector interferences

  /*
   * Create the radio network related parameters
   */
  RadioNetworkParametersHelper ranHelper;
  uint16_t numScPerRb = 1;
  if (radioNetwork == "LTE")
    {
      ranHelper.SetNetworkToLte (scenario, operationMode, 1);
      if (errorModel == "")
        {
          errorModel = "ns3::NrLteMiErrorModel";
        }
      else if (errorModel != "ns3::NrLteMiErrorModel")
        {
          NS_ABORT_MSG ("The selected error model is not recommended for LTE");
        }
      numScPerRb = 4;
    }
  else if (radioNetwork == "NR")
    {
      ranHelper.SetNetworkToNr (scenario, operationMode, numerologyBwp, 1);
      if (errorModel == "")
        {
          errorModel = "ns3::NrEesmCcT2";
        }
      else if (errorModel == "ns3::NrLteMiErrorModel")
        {
          NS_ABORT_MSG ("The selected error model is not recommended for NR");
        }
    }
  else
    {
      NS_ABORT_MSG ("Unrecognized radio network technology");
    }

  /**
   * Adjust the average number of Reference symbols per RB only for LTE case,
   * which is larger than in NR. We assume a value of 4 (could be 3 too).
   */
  Config::SetDefault("ns3::NrAmc::NumRefScPerRb", UintegerValue (numScPerRb));

  NodeContainer gnbLowLatContainer, gnbVoiceContainer, gnbVideoContainer;
  for (uint32_t j = 0; j < gridScenario.GetBaseStations ().GetN (); ++j)
    {
      Ptr<Node> gnb = gridScenario.GetBaseStations ().Get (j);
      switch (j % ffr)
      {
        case 0:
          gnbLowLatContainer.Add (gnb);
          break;
        case 1:
          gnbVoiceContainer.Add (gnb);
          break;
        case 2:
          gnbVideoContainer.Add (gnb);
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
  NodeContainer ueLowLatContainer, ueVoiceContainer, ueVideoContainer;

  for (uint32_t j = 0; j < gridScenario.GetUserTerminals ().GetN (); ++j)
    {
      Ptr<Node> ue = gridScenario.GetUserTerminals ().Get (j);
      switch (j % ffr)
      {
        case 0:
          ueLowLatContainer.Add (ue);
          break;
        case 1:
          ueVoiceContainer.Add (ue);
          break;
        case 2:
          ueVideoContainer.Add (ue);
          break;
        default:
          NS_ABORT_MSG("ffr param cannot be larger than 3");
          break;
      }
    }

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
   * Spectrum division. We create one operational band containing three
   * component carriers, and each CC containing a single bandwidth part
   * centered at the frequency specified by the input parameters.
   * Each spectrum part length is, as well, specified by the input parameters.
   * The operational band will use StreetCanyon channel or UrbanMacro modeling.
   */
  BandwidthPartInfoPtrVector allBwps, bwps1, bwps2, bwps3;
  CcBwpCreator ccBwpCreator;
  // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
  // a single BWP per CC. Get the spectrum values from the RadioNetworkParametersHelper
  centralFrequencyBand = ranHelper.GetCentralFrequency ();
  bandwidthBand = ranHelper.GetBandwidth ();
  const uint8_t numCcPerBand = 1; // In this example, each cell will have one CC with one BWP
  BandwidthPartInfo::Scenario scene;
  if (scenario == "UMi")
    {
      scene =  BandwidthPartInfo::UMi_StreetCanyon_LoS;
    }
  else if (scenario == "UMa")
    {
      scene = BandwidthPartInfo::UMa_LoS;
    }
  else
    {
      NS_ABORT_MSG ("Unsupported scenario");
    }

  /*
   * Attributes of ThreeGppChannelModel still cannot be set in our way.
   * TODO: Coordinate with Tommaso
   */
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds(100)));
  mmWaveHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  mmWaveHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  // Error Model: UE and GNB with same spectrum error model.
  mmWaveHelper->SetUlErrorModel (errorModel);
  mmWaveHelper->SetDlErrorModel (errorModel);

  // Both DL and UL AMC will have the same model behind.
  mmWaveHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
  mmWaveHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel

  /*
   * Create the necessary operation bands. In this example, each sector operates
   * in a separate band. Each band contains a single component carrier (CC),
   * which is made of one BWP in TDD operation mode or two BWPs in FDD mode.
   * Note that BWPs have the same bandwidth. Therefore, CCs and bands in FDD are
   * twice larger than in TDD.
   *
   * The configured spectrum division for TDD operation is:
   * |---Band1---|---Band2---|---Band3---|
   * |----CC1----|----CC2----|----CC3----|
   * |----BWP1---|----BWP2---|----BWP3---|
   *
   * And the configured spectrum division for FDD operation is:
   * |---------Band1---------|---------Band2---------|---------Band3---------|
   * |----------CC1----------|----------CC2----------|----------CC3----------|
   * |----BWP1---|----BWP2---|----BWP3---|----BWP4---|----BWP5---|----BWP6---|
   */
  double centralFrequencyBand1 = centralFrequencyBand - bandwidthBand;
  double centralFrequencyBand2 = centralFrequencyBand;
  double centralFrequencyBand3 = centralFrequencyBand + bandwidthBand;
  double bandwidthBand1 = bandwidthBand;
  double bandwidthBand2 = bandwidthBand;
  double bandwidthBand3 = bandwidthBand;

  uint8_t numBwpPerCc = 1;
  if (operationMode == "FDD")
    {
      numBwpPerCc = 2; // FDD will have 2 BWPs per CC
    }

  CcBwpCreator::SimpleOperationBandConf bandConf1 (centralFrequencyBand1, bandwidthBand1, numCcPerBand, scene);
  bandConf1.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC
  CcBwpCreator::SimpleOperationBandConf bandConf2 (centralFrequencyBand2, bandwidthBand2, numCcPerBand, scene);
  bandConf2.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC
  CcBwpCreator::SimpleOperationBandConf bandConf3 (centralFrequencyBand3, bandwidthBand3, numCcPerBand, scene);
  bandConf3.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC

  // By using the configuration created, it is time to make the operation bands
  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);
  OperationBandInfo band2 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf2);
  OperationBandInfo band3 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf3);

  /*
   * Initialize channel and pathloss, plus other things inside band1. If needed,
   * the band configuration can be done manually, but we leave it for more
   * sophisticated examples. For the moment, this method will take care
   * of all the spectrum initialization needs.
   */
  mmWaveHelper->InitializeOperationBand (&band1);
  mmWaveHelper->InitializeOperationBand (&band2);
  mmWaveHelper->InitializeOperationBand (&band3);
  allBwps = CcBwpCreator::GetAllBwps ({band1,band2,band3});
  bwps1 = CcBwpCreator::GetAllBwps ({band1});
  bwps2 = CcBwpCreator::GetAllBwps ({band2});
  bwps3 = CcBwpCreator::GetAllBwps ({band3});



  /*
   * Start to account for the bandwidth used by the example, as well as
   * the total power that has to be divided among the BWPs. Since there is only
   * one band and one BWP occupying the entire band, there is no need to divide
   * power among BWPs.
   */
  double totalTxPower = ranHelper.GetTxPower (); //Convert to mW
  double x = pow (10, totalTxPower/10);

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
  mmWaveHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  mmWaveHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  mmWaveHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (true));

  // Antennas for all the gNbs
  mmWaveHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (2));
  mmWaveHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (2));
  mmWaveHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (false));

//  // Error Model
//  mmWaveHelper->SetGnbSpectrumAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));
//  mmWaveHelper->SetUeSpectrumAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));

  // We assume a common traffic pattern for all UEs
  uint32_t bwpIdForLowLat = 0;
  if (direction == "UL")
    {
      bwpIdForLowLat = 1;
    }

  // gNb routing between Bearer and bandwidth part
  mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));

  // Ue routing between Bearer and bandwidth part
  mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));

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

//  NetDeviceContainer enbNetDev = mmWaveHelper->InstallGnbDevice (gridScenario.GetBaseStations (), allBwps);
  NetDeviceContainer gnbLowLatNetDev = mmWaveHelper->InstallGnbDevice (gnbLowLatContainer, bwps1);
  NetDeviceContainer gnbVoiceNetDev = mmWaveHelper->InstallGnbDevice (gnbVoiceContainer, bwps2);
  NetDeviceContainer gnbVideoNetDev = mmWaveHelper->InstallGnbDevice (gnbVideoContainer, bwps3);
  NetDeviceContainer ueLowLatNetDev = mmWaveHelper->InstallUeDevice (ueLowLatContainer, bwps1);
  NetDeviceContainer ueVoiceNetDev = mmWaveHelper->InstallUeDevice (ueVoiceContainer, bwps2);
  NetDeviceContainer ueVideoNetDev = mmWaveHelper->InstallUeDevice (ueVideoContainer, bwps3);

  /*
   * Case (iii): Go node for node and change the attributes we have to setup
   * per-node.
   */

  // Sectors (cells) of a site are pointing at different directions
  double orientationRads = gridScenario.GetAntennaOrientationRadians (0, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbLowLatNetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbLowLatNetDev.Get (numCell);
      uint32_t numBwps = mmWaveHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<MmWaveEnbPhy> phy = mmWaveHelper->GetEnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
                    ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));

          // Set TDD pattern
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<MmWaveEnbPhy> phy0 = mmWaveHelper->GetEnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
              ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray());
          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));
          Ptr<MmWaveEnbPhy> phy1 = mmWaveHelper->GetEnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
              ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));
          mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          if (direction == "DL")
            {
              mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));
              mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));
            }
          else
            {
              mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (-30.0));
              mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));
            }
          // Set TDD pattern
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          mmWaveHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG("Incorrect number of BWPs per CC");
        }
    }

  orientationRads = gridScenario.GetAntennaOrientationRadians (1, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbVoiceNetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbVoiceNetDev.Get (numCell);
      uint32_t numBwps = mmWaveHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<MmWaveEnbPhy> phy = mmWaveHelper->GetEnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
              ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));

          // Set TDD pattern
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<MmWaveEnbPhy> phy0 = mmWaveHelper->GetEnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
              ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray());
          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));
          Ptr<MmWaveEnbPhy> phy1 = mmWaveHelper->GetEnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
              ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));
          mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          if (direction == "DL")
            {
              mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));
              mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));
            }
          else
            {
              mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (-30.0));
              mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));
            }

          // Set TDD pattern
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          mmWaveHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG("Incorrect number of BWPs per CC");
        }
    }

  orientationRads = gridScenario.GetAntennaOrientationRadians (2, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbVideoNetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbVideoNetDev.Get (numCell);
      uint32_t numBwps = mmWaveHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<MmWaveEnbPhy> phy = mmWaveHelper->GetEnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
              ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));

          // Set TDD pattern
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<MmWaveEnbPhy> phy0 = mmWaveHelper->GetEnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
              ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray());
          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));
          Ptr<MmWaveEnbPhy> phy1 = mmWaveHelper->GetEnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
              ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));
          mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          if (direction == "DL")
            {
              mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));
              mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));
            }
          else
            {
              mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (-30.0));
              mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (10*log10 (x)));
            }

          // Set TDD pattern
          mmWaveHelper->GetEnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          mmWaveHelper->GetEnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          mmWaveHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG("Incorrect number of BWPs per CC");
        }
    }


  // Set the UE routing:

  if (operationMode == "FDD")
    {
      for (uint32_t i = 0; i < ueLowLatNetDev.GetN (); i++)
        {
          mmWaveHelper->GetBwpManagerUe (ueLowLatNetDev.Get (i))->SetOutputLink (0, 1);
        }

      for (uint32_t i = 0; i < ueVoiceNetDev.GetN (); i++)
        {
          mmWaveHelper->GetBwpManagerUe (ueVoiceNetDev.Get (i))->SetOutputLink (0, 1);
        }

      for (uint32_t i = 0; i < ueVideoNetDev.GetN (); i++)
        {
          mmWaveHelper->GetBwpManagerUe (ueVideoNetDev.Get (i))->SetOutputLink (0, 1);
        }
    }

  // When all the configuration is done, explicitly call UpdateConfig ()

  for (auto it = gnbLowLatNetDev.Begin (); it != gnbLowLatNetDev.End (); ++it)
    {
      DynamicCast<MmWaveEnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = gnbVoiceNetDev.Begin (); it != gnbVoiceNetDev.End (); ++it)
    {
      DynamicCast<MmWaveEnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = gnbVideoNetDev.Begin (); it != gnbVideoNetDev.End (); ++it)
    {
      DynamicCast<MmWaveEnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueLowLatNetDev.Begin (); it != ueLowLatNetDev.End (); ++it)
    {
      DynamicCast<MmWaveUeNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueVoiceNetDev.Begin (); it != ueVoiceNetDev.End (); ++it)
    {
      DynamicCast<MmWaveUeNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueVideoNetDev.Begin (); it != ueVideoNetDev.End (); ++it)
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

  Ipv4InterfaceContainer ueLowLatIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLowLatNetDev));
  Ipv4InterfaceContainer ueVoiceIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueVoiceNetDev));
  Ipv4InterfaceContainer ueVideoIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueVideoNetDev));

  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < gridScenario.GetUserTerminals ().GetN(); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (gridScenario.GetUserTerminals ().Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to its eNB
  mmWaveHelper->AttachToClosestEnb (ueLowLatNetDev, gnbLowLatNetDev);
  mmWaveHelper->AttachToClosestEnb (ueVoiceNetDev, gnbVoiceNetDev);
  mmWaveHelper->AttachToClosestEnb (ueVideoNetDev, gnbVideoNetDev);
//  for (uint32_t i = 0; i < ueNum; ++i)
//    {
//      Ptr<NetDevice> gnbNetDev, ueNetDev;
//      gnbNetDev = enbNetDev.Get (i % gNbNum);
//      switch (i % 3)
//      {
//        case 0:
//          ueNetDev = ueLowLatNetDev.Get(i / 3);
//          break;
//        case 1:
//          ueNetDev = ueVoiceNetDev.Get(i / 3);
//          break;
//        case 2:
//          ueNetDev = ueVideoNetDev.Get(i / 3);
//          break;
//        default:
//          NS_ABORT_MSG ("Programming error");
//          break;
//      }
//      mmWaveHelper->AttachToEnb (ueNetDev, gnbNetDev);
//      if (logging == true)
//        {
//          Vector gnbpos = gnbNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
//          Vector uepos = ueNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
//          double distance = CalculateDistance (gnbpos, uepos);
//          std::cout << "Distance = " << distance << " meters" << std::endl;
//        }
//    }

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
      serverApps.Add (dlPacketSinkLowLat.Install (ueLowLatContainer));
      serverApps.Add (dlPacketSinkLowLat.Install (ueVoiceContainer));
      serverApps.Add (dlPacketSinkLowLat.Install (ueVideoContainer));
    }
  else
    {
      serverApps.Add (dlPacketSinkLowLat.Install (remoteHost));
      serverApps.Add (dlPacketSinkLowLat.Install (remoteHost));
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
  EpsBearer lowLatBearer (EpsBearer::NGBR_LOW_LAT_EMBB);

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

  for (uint32_t i = 0; i < ueLowLatContainer.GetN (); ++i)
    {
      Ptr<Node> ue = ueLowLatContainer.Get (i);
      Ptr<NetDevice> ueDevice = ueLowLatNetDev.Get(i);
      Address ueAddress = ueLowLatIpIface.GetAddress (i);

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
//          NS_ABORT_MSG ("UL not supported yet");
        }
      // Activate a dedicated bearer for the traffic type
      mmWaveHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
    }

  for (uint32_t i = 0; i < ueVoiceContainer.GetN (); ++i)
    {
      Ptr<Node> ue = ueVoiceContainer.Get (i);
      Ptr<NetDevice> ueDevice = ueVoiceNetDev.Get(i);
      Address ueAddress = ueVoiceIpIface.GetAddress (i);

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
//          NS_ABORT_MSG ("UL not supported yet");
        }
      // Activate a dedicated bearer for the traffic type
      mmWaveHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
    }

  for (uint32_t i = 0; i < ueVideoContainer.GetN (); ++i)
    {
      Ptr<Node> ue = ueVideoContainer.Get (i);
      Ptr<NetDevice> ueDevice = ueVideoNetDev.Get(i);
      Address ueAddress = ueVideoIpIface.GetAddress (i);

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
//          NS_ABORT_MSG ("UL not supported yet");
        }
      // Activate a dedicated bearer for the traffic type
      mmWaveHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
    }

  // start UDP server and client apps
  serverApps.Start(MilliSeconds(udpAppStartTimeMs));
  clientApps.Start(MilliSeconds(udpAppStartTimeMs));
  serverApps.Stop(MilliSeconds(simTimeMs));
  clientApps.Stop(MilliSeconds(simTimeMs));

  // enable the traces provided by the mmWave module
  mmWaveHelper->EnableTraces ();


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


