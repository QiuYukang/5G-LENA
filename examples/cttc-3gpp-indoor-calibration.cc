/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 *   Author: Biljana Bojovic <bbojovic@cttc.es>

 */

#include "ns3/mmwave-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include "ns3/log.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/eps-bearer-tag.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/abort.h"
#include "ns3/object.h"
#include "ns3/mmwave-mac-scheduler-ns3.h"
#include "ns3/mmwave-mac-scheduler-ofdma.h"
#include "ns3/mmwave-mac-scheduler-ofdma-rr.h"
#include "ns3/mmwave-phy-mac-common.h"
#include "ns3/basic-data-calculators.h"
#include "ns3/antenna-array-3gpp-model.h"

using namespace ns3;

/**
 * \ingroup examples
 * \file cttc-3gpp-indoor-calibration.cc
 * \brief Simulation script for the NR-MIMO Phase 1 system-level calibration
 *
 * The scenario implemented in the present simulation script is according to
 * the topology described in 3GPP TR 38.900 V15.0.0 (2018-06) Figure 7.2-1:
 * "Layout of indoor office scenarios".
 *
 * The simulation assumptions and the configuration parameters follow
 * the evaluation assumptions agreed at 3GPP TSG RAN WG1 meeting #88,
 * and which are summarised in R1-1703534 Table 1.
 * In the following Figure is illustrated the scenario with the gNB positions
 * which are represented with "x". The UE nodes are randomly uniformly dropped
 * in the area. There are 10 UEs per gNB.
 *
 * <pre>
 *   +----------------------120 m------------------ +
 *   |                                              |
 *   |                                              |
 *   |      x      x      x      x      x-20m-x     |
 *   |                                        |     |
 *   50m                                     20m    |
     |                                        |     |
 *   |      x      x      x      x      x     x     |
 *   |                                              |
 *   |                                              |
 *   +----------------------------------------------+
 * </pre>
 * The results of the simulation are files containing data that is being
 * collected over the course of the simulation execution:
 *
 * - SINR values for all the 120 UEs
 * - SNR values for all the 120 UEs
 * - RSSI values for all the 120 UEs
 *
 * Additionally there are files that contain:
 *
 * - UE positions
 * - gNB positions
 * - distances of UEs from the gNBs to which they are attached
 *
 * The file names are created by default in the root project directory if not
 * configured differently by setting resultsDirPath parameter of the Run()
 * function.
 *
 * The file names by default start with the prefixes such as "sinrs", "snrs",
 * "rssi", "gnb-positions,", "ue-positions" which are followed by the
 * string that briefly describes the configuration parameters that are being
 * set in the specific simulation execution.
 */

enum AntennaModelEnum{
  _ISO,
  _3GPP,
};

NS_LOG_COMPONENT_DEFINE ("Nr3gppIndoorCalibration");

// Global Values are used in place of command line arguments so that these
// values may be managed in the ns-3 ConfigStore system.


static ns3::GlobalValue g_duration ("duration",
                                     "simulation duration in milliseconds",
                                     ns3::UintegerValue (100),
                                     ns3::MakeUintegerChecker<uint32_t>());

static ns3::GlobalValue g_shadowing ("shadowing",
                                     "if true, shadowing is enabled in 3gpp propagation loss model;"
                                     "if false, shadowing is disabled",
                                     ns3::BooleanValue (true),
                                     ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_antennaOrientation ("antennaOrientation",
                                              "the orientation of the antenna on gNB and UE",
                                              ns3::EnumValue (AntennaArray3gppModel::Z0),
                                              ns3::MakeEnumChecker (AntennaArray3gppModel::Z0, "Z0",
                                                                    AntennaArray3gppModel::X0, "X0"));
static ns3::GlobalValue g_antennaModelgNb ("antennaModelGnb",
                                           "the antenna model of gNb, can be ISO or 3GPP",
                                           ns3::EnumValue (_3GPP),
                                           ns3::MakeEnumChecker (_ISO, "ISO",
                                                                 _3GPP, "3GPP"));

static ns3::GlobalValue g_antennaModelUe ("antennaModelUe",
                                           "the antenna model of Ue, can be ISO or 3GPP",
                                           ns3::EnumValue (_3GPP),
                                           ns3::MakeEnumChecker (_ISO, "ISO",
                                                                 _3GPP, "3GPP"));

static ns3::GlobalValue g_resultsDir ("resultsDir",
                                      "directory where to store the simulation results",
                                      ns3::StringValue ("./"),
                                      ns3::MakeStringChecker ());

static ns3::GlobalValue g_simTag ("simTag",
                                  "tag to be appended to the output filenames to distinguish different simulation campaign output files",
                                  ns3::StringValue (""),
                                  ns3::MakeStringChecker ());

static ns3::GlobalValue g_indoorScenario ("indoorScenario",
                                          "the indoor scenario to be used can be: InH-OfficeMixed, InH-OfficeOpen or InH-ShoppingMall",
                                          ns3::StringValue ("InH-ShoppingMall"),
                                          ns3::MakeStringChecker ());

static ns3::GlobalValue g_speed ("speed",
                                 "UE speed in km/h",
                                 ns3::DoubleValue (3.00),
                                 ns3::MakeDoubleChecker<double> (0.0, 10.0));

static ns3::GlobalValue g_isBeamSearchMethod ("isBeamSearchMethod",
                                              "if true, beam search method will be used;"
                                              "if false, long term covariance matrix will be used",
                                              ns3::BooleanValue (true),
                                              ns3::MakeBooleanChecker ());

static ns3::GlobalValue g_beamSearchMethodAngle ("beamSearchMethodAngle",
                                                 "beam search method angle step",
                                                 ns3::DoubleValue (30.00),
                                                 ns3::MakeDoubleChecker<double> (0.0, 360.0));

static ns3::GlobalValue g_gNbAntennaMount ("gnbAntennaMount",
                                           "gNb antenna mount type. Can be Wall mount or Sector mount. Doc: 38.802-e20. A.2.1-7",
                                           ns3::EnumValue (ns3::AntennaArray3gppModel::GnbWallMount),
                                           ns3::MakeEnumChecker (ns3::AntennaArray3gppModel::GnbWallMount, "WALL",
                                                                 ns3::AntennaArray3gppModel::GnbSingleSector, "SECT"));

/**
 * \brief Main class
 */
class Nr3gppIndoorCalibration
{

public:

  /**
   * \brief This function converts a linear SINR value that is encapsulated in params
   * structure to dBs, and then it prints the dB value to an output file
   * containing SINR values.
   * @param params RxPacketTraceParams structure that contains different
   * attributes that define the reception of the packet
   *
   */
  void UeReception (RxPacketTraceParams params);

  /**
   * \brief This function converts a linear SNR value to dBs and prints it to
   * the output file containing SNR values.
   * @param snr SNR value in linear units
   */
  void UeSnrPerProcessedChunk (double snr);

  /**
   * \brief This function prints out the RSSI value in dBm to file.
   * @param rssidBm RSSI value in dBm
   */
  void UeRssiPerProcessedChunk (double rssidBm);

  /**
   * Function that will actually configure all the simulation parameters,
   * topology and run the simulation by using the parameters that are being
   * configured for the specific run.
   *
   * @param shadowing boolean value that determines whether the shadowing will
   * be enabled in the 3gpp channel model
   * @param antOrientation the parameter thar specifies the antenna orientation,
   * e.g. can be horizontal when Z=0, or vertical when X=0
   * @param gNbAntennaModel antenna model to be used by gNB device, can be ISO
   * directional 3GPP
   * @param ueAntennaModel antenna model to be used by gNB device, can be ISO
   * directional 3GPP
   * @param scenario defines the indoor scenario to be used in the simulation
   * campaign, currently the two different indoor scenarios are considered:
   * InH-OfficeOpen and InH-OfficeMixed
   * @param speed the speed of UEs in km/h
   * @param resultsDirPath results directory path
   * @param isBeamSearchMethod whether to use the beamSearchMethod or long term
   * covariance matrix
   * @param beamSearchMethodAngle beam search method angle step
   * @param
   */
  void Run (bool shadowing, AntennaArrayModel::AntennaOrientation antOrientation, TypeId gNbAntennaModel,
            TypeId ueAntennaModel, std::string scenario, double speed,
            std::string resultsDirPath, std::string tag,
            bool isBeamSearchMethod, double beamSearchMethodAngle,
            AntennaArray3gppModel::GnbAntennaMount gNbAntennaMount, uint32_t duration);
  /**
   * \brief Destructor that closes the output file stream and finished the
   * writing into the files.
   */
  ~Nr3gppIndoorCalibration ();

  /**
   * \brief Function selects UE nodes that are placed with a minimum
   * distance from its closest gNB.
   * \param ueNodes - container of UE nodes
   * \param gnbNodes - container of gNB nodes
   * \param min3DDistance - the minimum that shall be between UE and gNB
   * \param numberOfUesToBeSelected -the number of UE nodes to be selected from the original container
   */
  NodeContainer SelectWellPlacedUes (const NodeContainer ueNodes, const NodeContainer gnbNodes,
                                     double min3DDistance, uint32_t numberOfUesToBeSelected);

private:

  std::ofstream m_outSinrFile;         //!< the output file stream for the SINR file
  std::ofstream m_outSnrFile;          //!< the output file stream for the SNR file
  std::ofstream m_outRssiFile;         //!< the output file stream for the RSSI file
  std::ofstream m_outUePositionsFile;  //!< the output file stream for the UE positions file
  std::ofstream m_outGnbPositionsFile; //!< the output file stream for the gNB positions file
  std::ofstream m_outDistancesFile;    //!< the output file stream for the distances file

};

/**
 * Function that creates the output file name for the results.
 * @param directoryName Directory name
 * @param filePrefix The prefix for the file name, e.g. sinr, snr,..
 * @param tag A tag that contains some simulation run specific values in order to be
 * able to distinguish the results file for different runs for different parameters
 * configuration
 * @return returns The full path file name string
 */
std::string
BuildFileNameString (std::string directoryName, std::string filePrefix, std::string tag)
{
  std::ostringstream oss;
  oss << directoryName << filePrefix<<tag;
  return oss.str ();
}

/**
 * Creates a string tag that contains some simulation run specific values in
 * order to be able to distinguish the results files for different runs for
 * different parameters.
 * @param shadowing whether the shadowing is enabled
 * @param antOrientation antenna orientation
 * @param gNbAntennaModel gNb antenna model
 * @param ueAntennaModel UE antenna model
 * @param scenario The indoor scenario to be used
 * @param speed The speed of UEs in km/h
 * @return the parameter specific simulation name
 */
std::string
BuildTag(bool shadowing, AntennaArrayModel::AntennaOrientation antOrientation,
         TypeId gNbAntennaModel, TypeId ueAntennaModel, std::string scenario,
         double speed, bool isBeamSearchMethod, double beamSearchMethodAngle,
         AntennaArray3gppModel::GnbAntennaMount gNbAntennaMount = AntennaArray3gppModel::GnbWallMount)
{

  std::ostringstream oss;
  std::string ao;

  if (antOrientation == AntennaArrayModel::X0)
    {
      ao = "X0";
    }
  else if (antOrientation == AntennaArrayModel::Z0)
    {
      ao = "Z0";
    }
  else
    {
      NS_ABORT_MSG("unknown antenna orientation..");
    }

  std::string gnbAm;
  if (gNbAntennaModel == AntennaArrayModel::GetTypeId())
    {
      gnbAm = "ISO";
    }
  else if (gNbAntennaModel == AntennaArray3gppModel::GetTypeId())
    {
      gnbAm = "3GPP";
    }

  std::string ueAm;
  if (ueAntennaModel == AntennaArrayModel::GetTypeId())
    {
      ueAm = "ISO";
    }
  else if (ueAntennaModel == AntennaArray3gppModel::GetTypeId())
    {
      ueAm = "3GPP";
    }

  double angl = 0;

  if (isBeamSearchMethod)
    {
      angl = beamSearchMethodAngle;
    }

  std::string gm = "";

  if (gNbAntennaMount == AntennaArray3gppModel::GnbWallMount)
    {
      gm = "WALL";
    }
  else if (gNbAntennaMount == AntennaArray3gppModel::GnbSingleSector)
    {
      gm = "SECT";
    }
  else
    {
      NS_ABORT_MSG("unknown antenna orientation..");
    }

  oss <<"-sh"<<shadowing<<"-ao"<<ao<<"-amGnb"<<gnbAm<<"-amUE"<<ueAm<<
      "-sc"<<scenario<<"-sp"<<speed<<"-bs"<<isBeamSearchMethod<<"-angl"<<angl<<"-gm"<<gm;

  return oss.str ();
}

/**
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param params RxPacketTraceParams structure containing RX parameters
 */
void UeReceptionTrace (Nr3gppIndoorCalibration* scenario, RxPacketTraceParams params)
{
  scenario->UeReception(params);
 }

/**
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param snr SNR value
 */
void UeSnrPerProcessedChunkTrace (Nr3gppIndoorCalibration* scenario, double snr)
{
  scenario->UeSnrPerProcessedChunk (snr);
}

/**
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param rssidBm rssidBm RSSI value in dBm
 */
void UeRssiPerProcessedChunkTrace (Nr3gppIndoorCalibration* scenario, double rssidBm)
{
  scenario->UeRssiPerProcessedChunk (rssidBm);
}


void
Nr3gppIndoorCalibration::UeReception (RxPacketTraceParams params)
{
  m_outSinrFile<<params.m_cellId<<params.m_rnti<<"\t"<<10*log10(params.m_sinr)<<std::endl;
}

void
Nr3gppIndoorCalibration::UeSnrPerProcessedChunk (double snr)
{
  m_outSnrFile<<10*log10(snr)<<std::endl;
}


void
Nr3gppIndoorCalibration::UeRssiPerProcessedChunk (double rssidBm)
{
  m_outRssiFile<<rssidBm<<std::endl;
}

Nr3gppIndoorCalibration::~Nr3gppIndoorCalibration ()
{
  m_outSinrFile.close();
  m_outSnrFile.close();
  m_outRssiFile.close();
}

NodeContainer
Nr3gppIndoorCalibration::SelectWellPlacedUes (const NodeContainer ueNodes, const NodeContainer gnbNodes, double minDistance, uint32_t numberOfUesToBeSelected)
{
  NodeContainer ueNodesFiltered;
  bool correctDistance = true;

  for (NodeContainer::Iterator itUe = ueNodes.Begin(); itUe!=ueNodes.End(); itUe++)
    {
      correctDistance = true;
      Ptr<MobilityModel> ueMm = (*itUe)->GetObject<MobilityModel>();
      Vector uePos = ueMm->GetPosition ();

      for (NodeContainer::Iterator itGnb = gnbNodes.Begin(); itGnb!=gnbNodes.End(); itGnb++)
        {
          Ptr<MobilityModel> gnbMm = (*itGnb)->GetObject<MobilityModel>();
          Vector gnbPos = gnbMm->GetPosition ();
          double x = uePos.x - gnbPos.x;
          double y = uePos.y - gnbPos.y;
          double distance = sqrt (x * x + y * y);

          if (distance < minDistance)
            {
              correctDistance = false;
              //NS_LOG("The UE node "<<(*itUe)->GetId() << " has wrong position, discarded.");
              break;
            }
          else
            {
              m_outDistancesFile <<distance<<std::endl;
            }
        }

      if (correctDistance)
        {
          ueNodesFiltered.Add(*itUe);
        }
      if (ueNodesFiltered.GetN() >= numberOfUesToBeSelected)
        {
          // there are enough candidate UE nodes
          break;
        }
    }
  return ueNodesFiltered;
}

void
Nr3gppIndoorCalibration::Run (bool shadowing, AntennaArrayModel::AntennaOrientation antOrientation,
                              TypeId gNbAntennaModel, TypeId ueAntennaModel, std::string scenario, double speed,
                              std::string resultsDirPath, std::string tag,
                              bool isBeamSearchMethod, double beamSearchMethodAngle,
                              AntennaArray3gppModel::GnbAntennaMount gNbAntennaMount, uint32_t duration)
{
    Time simTime = MilliSeconds (duration);
    Time udpAppStartTimeDl = MilliSeconds (100);
    Time udpAppStopTimeDl = MilliSeconds (duration);
    uint32_t packetSize = 1000;
    DataRate udpRate = DataRate ("0.1kbps");
    // initially created 240 UE nodes, out of which will be selected 120 UEs that
    // are well placed respecting the minimum distance parameter that is configured
    uint16_t ueCount = 240;
    // the minimum distance parameter
    double minDistance = 0;
    // BS atnenna height is 3 meters
    double gNbHeight = 3;
    // UE antenna height is 1.5 meters
    double ueHeight = 1.5;

    // if simulation tag is not provided create one
    if (tag=="")
      {
        tag = BuildTag(shadowing, antOrientation, gNbAntennaModel, ueAntennaModel, scenario, speed, isBeamSearchMethod, beamSearchMethodAngle, gNbAntennaMount);
      }
    std::string filenameSinr = BuildFileNameString ( resultsDirPath , "sinrs", tag);
    std::string filenameSnr = BuildFileNameString ( resultsDirPath , "snrs", tag);
    std::string filenameRssi = BuildFileNameString ( resultsDirPath , "rssi", tag);
    std::string filenameUePositions = BuildFileNameString ( resultsDirPath , "ue-positions", tag);
    std::string filenameGnbPositions = BuildFileNameString( resultsDirPath , "gnb-positions", tag);
    std::string filenameDistances = BuildFileNameString ( resultsDirPath , "distances", tag);

    m_outSinrFile.open (filenameSinr.c_str ());
    m_outSinrFile.setf (std::ios_base::fixed);

    if(!m_outSinrFile.is_open())
      {
        NS_ABORT_MSG("Can't open file " << filenameSinr);
      }

    m_outSnrFile.open (filenameSnr.c_str ());
    m_outSnrFile.setf (std::ios_base::fixed);

    if(!m_outSnrFile.is_open())
      {
        NS_ABORT_MSG("Can't open file " << filenameSnr);
      }

    m_outRssiFile.open (filenameRssi.c_str ());
    m_outRssiFile.setf (std::ios_base::fixed);

    if(!m_outRssiFile.is_open())
      {
        NS_ABORT_MSG("Can't open file " << filenameRssi);
      }

    m_outUePositionsFile.open (filenameUePositions.c_str ());
    m_outUePositionsFile.setf (std::ios_base::fixed);

    if(!m_outUePositionsFile.is_open())
      {
        NS_ABORT_MSG("Can't open file " << filenameUePositions);
      }

    m_outGnbPositionsFile.open (filenameGnbPositions.c_str ());
    m_outGnbPositionsFile.setf (std::ios_base::fixed);

    if(!m_outGnbPositionsFile.is_open())
      {
        NS_ABORT_MSG("Can't open file " << filenameGnbPositions);
      }

    m_outDistancesFile.open (filenameDistances.c_str ());
    m_outDistancesFile.setf (std::ios_base::fixed);

    if(!m_outDistancesFile.is_open())
      {
         NS_ABORT_MSG("Can't open file " << filenameDistances);
       }

    Config::SetDefault ("ns3::AntennaArray3gppModel::GnbAntennaMountType", EnumValue (gNbAntennaMount));
    Config::SetDefault ("ns3::AntennaArrayModel::AntennaOrientation", EnumValue (antOrientation));
    // configure antenna gain for the ISO antenna
    Config::SetDefault ("ns3::AntennaArrayModel::AntennaGain", DoubleValue (5));
    Config::SetDefault("ns3::MmWaveHelper::GnbAntennaArrayModelType", TypeIdValue(gNbAntennaModel));
    Config::SetDefault("ns3::MmWaveHelper::UeAntennaArrayModelType", TypeIdValue(ueAntennaModel));
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue(scenario));
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(shadowing));

    // we tried also with the optional nLos model and both 3gpp antennas, and it is not better calibrated
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::OptionalNlos", BooleanValue(false));

    // Default scenario configuration that are summarised in to R1-1703534 3GPP TSG RAN WG1 Meeting #88
    Config::SetDefault ("ns3::MmWave3gppChannel::Speed", DoubleValue (speed*1000/3600));
    // Disable channel matrix update to speed up the simulation execution
    Config::SetDefault ("ns3::MmWave3gppChannel::UpdatePeriod", TimeValue (MilliSeconds(0)));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::MacSchedulerType", TypeIdValue (TypeId::LookupByName("ns3::MmWaveMacSchedulerTdmaPF")));
    // Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    // Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault ("ns3::MmWave3gppChannel::CellScan", BooleanValue (isBeamSearchMethod));
    Config::SetDefault ("ns3::MmWave3gppChannel::BeamSearchAngleStep", DoubleValue (beamSearchMethodAngle));

    Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
    // Parameters according to R1-1703534 3GPP TSG RAN WG1 Meetging #88, 2017
    // Evaluation assumptions for Phase 1 NR MIMO system level calibration,
    Config::SetDefault ("ns3::MmWaveEnbPhy::TxPower", DoubleValue(23));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue(30e9));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::Numerology", UintegerValue(2));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::Bandwidth", DoubleValue(40e6));
    // Shall be 4x8 = 32 antenna elements
    Config::SetDefault("ns3::MmWaveEnbNetDevice::AntennaNumDim1", UintegerValue(4));
    Config::SetDefault("ns3::MmWaveEnbNetDevice::AntennaNumDim2", UintegerValue(8));
    // Shall be 2x4 = 8 antenna elements
    Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNumDim1", UintegerValue(2));
    Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNumDim2", UintegerValue(4));
    // UE antenna gain shall be set to 5 dBi
    // gNB noise figure shall be set to 7 dB
    Config::SetDefault("ns3::MmWaveEnbPhy::NoiseFigure", DoubleValue (7));
    // UE noise figure shall be set to 10 dB
    Config::SetDefault("ns3::MmWaveUePhy::NoiseFigure", DoubleValue (10));
    // set LOS,NLOS condition
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("a"));
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

    gNbNodes.Create (12);
    ueNodes.Create (ueCount);

    // Creating positions of the gNB according to the 3gpp TR 38.900 Figure 7.2.-1
    Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator> ();

    for (uint8_t j = 0; j < 2; j++)
      {
        for (uint8_t i = 0; i < 6; i++)
          {
            gNbPositionAlloc->Add(Vector( i*20, j*20, gNbHeight));
          }
      }

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator (gNbPositionAlloc);
    mobility.Install (gNbNodes);

    double minBigBoxX = -10.0;
    double minBigBoxY = -15.0;
    double maxBigBoxX = 110.0;
    double maxBigBoxY =  35.0;

    // Creating positions of the gNB according to the 3gpp TR 38.900 and
    // R11700144, uniformly randombly distributed in the rectangular area

    NodeContainer selectedUeNodes;
    for (uint8_t j = 0; j < 2; j++)
      {
        double minSmallBoxY = minBigBoxY + j * (maxBigBoxY-minBigBoxY)/2;

        for (uint8_t i = 0; i < 6; i++)
          {
            double minSmallBoxX = minBigBoxX + i * (maxBigBoxX - minBigBoxX)/6;
            Ptr<UniformRandomVariable> ueRandomVarX = CreateObject<UniformRandomVariable>();

            double minX = minSmallBoxX;
            double maxX = minSmallBoxX + (maxBigBoxX - minBigBoxX)/6 - 0.0001;
            double minY = minSmallBoxY;
            double maxY = minSmallBoxY + (maxBigBoxY-minBigBoxY)/2 - 0.0001;

            Ptr<RandomBoxPositionAllocator> ueRandomRectPosAlloc = CreateObject<RandomBoxPositionAllocator> ();
            ueRandomVarX->SetAttribute ("Min", DoubleValue (minX));
            ueRandomVarX->SetAttribute ("Max", DoubleValue (maxX));
            ueRandomRectPosAlloc->SetX(ueRandomVarX);
            Ptr<UniformRandomVariable> ueRandomVarY = CreateObject<UniformRandomVariable>();
            ueRandomVarY->SetAttribute ("Min", DoubleValue (minY));
            ueRandomVarY->SetAttribute ("Max", DoubleValue (maxY));
            ueRandomRectPosAlloc->SetY(ueRandomVarY);
            Ptr<ConstantRandomVariable> ueRandomVarZ = CreateObject<ConstantRandomVariable>();
            ueRandomVarZ->SetAttribute("Constant", DoubleValue(ueHeight));
            ueRandomRectPosAlloc->SetZ(ueRandomVarZ);

            uint8_t smallBoxIndex = j*6 + i;

            NodeContainer smallBoxCandidateNodes;
            NodeContainer smallBoxGnbNode;

            smallBoxGnbNode.Add(gNbNodes.Get(smallBoxIndex));

            for (uint32_t n = smallBoxIndex * ueCount/12; n < smallBoxIndex * (uint32_t)(ueCount/12) + (uint32_t)(ueCount/12); n++ )
              {
                smallBoxCandidateNodes.Add(ueNodes.Get(n));
              }
            mobility.SetPositionAllocator (ueRandomRectPosAlloc);
            mobility.Install (smallBoxCandidateNodes);
            NodeContainer sn = SelectWellPlacedUes (smallBoxCandidateNodes, smallBoxGnbNode , minDistance, 10);
            selectedUeNodes.Add(sn);
          }
      }

    for (uint j = 0; j < selectedUeNodes.GetN(); j++)
      {
          Vector v = selectedUeNodes.Get(j)->GetObject<MobilityModel>()->GetPosition();
          m_outUePositionsFile<<j<<"\t"<<v.x<<"\t"<<v.y<<"\t"<<v.z<<" "<<std::endl;
      }

    for (uint j = 0; j < gNbNodes.GetN(); j++)
      {
          Vector v = gNbNodes.Get(j)->GetObject<MobilityModel>()->GetPosition();
          m_outGnbPositionsFile<<j<<"\t"<<v.x<<"\t"<<v.y<<"\t"<<v.z<<" "<<std::endl;
      }

    m_outUePositionsFile.close();
    m_outGnbPositionsFile.close();
    m_outDistancesFile.close();

    //mobility.SetPositionAllocator (ueRandomRectPosAlloc);
    //install mmWave net devices
    NetDeviceContainer gNbDevs = mmWaveHelper->InstallEnbDevice (gNbNodes);
    NetDeviceContainer ueNetDevs = mmWaveHelper->InstallUeDevice (selectedUeNodes);

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
    // in this container, interface 0 is the pgw, 1 is the remoteHost
    //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDevs));

    // Set the default gateway for the UEs
    for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
      {
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get(j)->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      }

    // attach UEs to the closest eNB
    mmWaveHelper->AttachToClosestEnb (NetDeviceContainer(ueNetDevs), NetDeviceContainer(gNbDevs));

    // assign IP address to UEs, and install UDP downlink applications
    uint16_t dlPort = 1234;
    ApplicationContainer clientAppsDl;
    ApplicationContainer serverAppsDl;

    Time udpInterval = Time::FromDouble((packetSize*8) / static_cast<double> (udpRate.GetBitRate ()), Time::S);

    UdpServerHelper dlPacketSinkHelper (dlPort);
    serverAppsDl.Add (dlPacketSinkHelper.Install (ueNodes));

    // configure UDP downlink traffic
    for (uint32_t i = 0 ; i < ueNetDevs.GetN(); i ++)
      {
        UdpClientHelper dlClient (ueIpIface.GetAddress (i), dlPort);
        dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
        dlClient.SetAttribute("PacketSize", UintegerValue(packetSize));
        dlClient.SetAttribute ("Interval", TimeValue (udpInterval)); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
        clientAppsDl.Add (dlClient.Install (remoteHost));
      }

    // start UDP server and client apps
    serverAppsDl.Start(udpAppStartTimeDl);
    clientAppsDl.Start(udpAppStartTimeDl);

    serverAppsDl.Stop(udpAppStopTimeDl);
    clientAppsDl.Stop(udpAppStopTimeDl);

    for (uint32_t i = 0 ; i < ueNetDevs.GetN(); i ++)
      {
        Ptr<MmWaveSpectrumPhy > ue1SpectrumPhy = DynamicCast<MmWaveUeNetDevice>
        (ueNetDevs.Get(i))->GetPhy(0)->GetDlSpectrumPhy();
        ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe", MakeBoundCallback(&UeReceptionTrace, this));
        Ptr<mmWaveInterference> ue1SpectrumPhyInterference = ue1SpectrumPhy->GetMmWaveInterference();
        NS_ABORT_IF(!ue1SpectrumPhyInterference);
        ue1SpectrumPhyInterference->TraceConnectWithoutContext("SnrPerProcessedChunk", MakeBoundCallback (&UeSnrPerProcessedChunkTrace, this));
        ue1SpectrumPhyInterference->TraceConnectWithoutContext("RssiPerProcessedChunk", MakeBoundCallback (&UeRssiPerProcessedChunkTrace, this));
      }

    Simulator::Stop (simTime);
    Simulator::Run ();
    Simulator::Destroy ();
}

int
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  // parse again so you can override input file default values via command line
  cmd.Parse (argc, argv);

  EnumValue enumValue;
  DoubleValue doubleValue;
  BooleanValue booleanValue;
  StringValue stringValue;
  TypeIdValue typeIdValue;
  UintegerValue uintValue;

  GlobalValue::GetValueByName ("duration", uintValue);
  uint32_t duration = uintValue.Get ();

  GlobalValue::GetValueByName ("shadowing", booleanValue);
  bool shadowing = booleanValue.Get ();

  GlobalValue::GetValueByName ("antennaOrientation", enumValue);
  enum AntennaArray3gppModel::AntennaOrientation antennaOrientation = (AntennaArray3gppModel::AntennaOrientation) enumValue.Get ();

  GlobalValue::GetValueByName ("antennaModelGnb", enumValue);
  enum AntennaModelEnum antennaGnb = (AntennaModelEnum) enumValue.Get ();
  TypeId antennaModelGnb;
  if (antennaGnb == _3GPP)
    {
      antennaModelGnb = AntennaArray3gppModel::GetTypeId();
    }
  else
    {
      antennaModelGnb = AntennaArrayModel::GetTypeId();
    }

  GlobalValue::GetValueByName ("antennaModelUe", enumValue);
  enum AntennaModelEnum antennaUe = (AntennaModelEnum) enumValue.Get ();
  TypeId antennaModelUe;
  if (antennaUe == _3GPP)
    {
      antennaModelUe = AntennaArray3gppModel::GetTypeId();
    }
  else
    {
      antennaModelUe = AntennaArrayModel::GetTypeId();
    }

  GlobalValue::GetValueByName ("indoorScenario", stringValue);
  std::string indoorScenario = stringValue.Get ();

  GlobalValue::GetValueByName ("speed", doubleValue);
  double speed = doubleValue.Get ();

  GlobalValue::GetValueByName ("resultsDir", stringValue);
  std::string resultsDir = stringValue.Get ();

  GlobalValue::GetValueByName ("simTag", stringValue);
  std::string tag = stringValue.Get ();

  GlobalValue::GetValueByName ("isBeamSearchMethod", booleanValue);
  bool isBeamSearchMethod = booleanValue.Get ();

  GlobalValue::GetValueByName ("beamSearchMethodAngle", doubleValue);
  double beamSearchMethodAngle = doubleValue.Get ();

  GlobalValue::GetValueByName ("gnbAntennaMount", enumValue);
  enum AntennaArray3gppModel::GnbAntennaMount gnbAntennaMount = (AntennaArray3gppModel::GnbAntennaMount) enumValue.Get ();


  Nr3gppIndoorCalibration phase1CalibrationScenario;
  phase1CalibrationScenario.Run(shadowing, antennaOrientation, antennaModelGnb, antennaModelUe,
             indoorScenario, speed, resultsDir, tag, isBeamSearchMethod, beamSearchMethodAngle, gnbAntennaMount, duration);

  return 0;
}

