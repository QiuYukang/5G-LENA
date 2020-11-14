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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/eps-bearer-tag.h"
#include "ns3/config-store-module.h"
#include "ns3/nr-module.h"
#include "ns3/stats-module.h"
#include <ns3/sqlite-output.h>

using namespace ns3;

/**
 * \ingroup examples
 * \file realistic-beamforming.cc
 * \brief Simulation script for the realistic beamforming evaluation.
 * Simulation allows to configure various parameters out of which the
 * most important are:
 * - distance between transmitter and the receiver (since we want to evaluate how the distance impact
 * the selection of the correct beam). Distance will be configured with deltaX and delta Y
 * simulation parameters that define the relative position of UE with respect to gNB's position.
 * - type of the beamfoming methods (because we want to obtain results
 * for both ideal beamforming algorithm and realistic beamforming
 * algorithm. Expected behavior is that as the distance increases that
 * the error in estimating channel increases, thus realistic beamforming
 * algorithm makes more mistakes when selecting the correct beams at the
 * transmiter and the receiver).
 * -rngRun - random run number that will allow us to run many simulations
 * and to average the results
 *
 * The topology is very simple, consists of a single gNB and UE.
 *
 * <pre>
 *
 *                                                   + UE
 *                                                   |
 *                                                   |
 *                                                deltaY
 *                                                   |
 *                                                   |
 *                                                   |
 *   gNB+  ------------deltaX-------------------------
 * </pre>
 *
 * The results of the simulation are files containing data that is being
 * collected over the course of the simulation execution:
 *
 * - SINR values
 * - SNR values
 * - RSSI values
 *
 * The file names are created by default in the root project directory if not
 * configured differently by setting resultsDirPath parameter of the Run()
 * function.
 *
 * The file names by default start with the prefixes such as "sinrs", "snrs",
 * "rssi", which are followed by the string that briefly describes the
 * configuration parameters that are being set in the specific simulation execution.
 */


NS_LOG_COMPONENT_DEFINE ("CttcRealisticBeamforming");

/**
 * \brief Main class
 */
class CttcRealisticBeamforming
{

public:

  enum BeamformingMethod
  {
    IDEAL,
    REALISTIC,
  };

  /**
   * \brief This function converts a linear SINR value that is encapsulated in
   * params structure to dBs, and then it prints the dB value to an output file
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
   * \brief Function that will save the configuration parameters to be used later for
   * printing the results into the files.
   *
   * @param deltaX delta that will be used to determine X coordinate of UE wrt to gNB X coordindate
   * @param deltaY delta that will be used to determine Y coordinate of UE wrt to gNB Y coordinate
   * @param beamforming beamforming type: Ideal or Rea
   * @param rngRun rngRun number that will be used to run the simulation
   * @param numerology The numerology
   * @param gNbAntennaModel antenna model to be used by gNB device, can be ISO
   * directional 3GPP
   * @param ueAntennaModel antenna model to be used by gNB device, can be ISO
   * directional 3GPP
   * @param resultsDirPath results directory path
   * @param tag A tag that contains some simulation run specific values in order
   * to be able to distinguish the results file for different runs for different
   * parameters configuration
   */
  void Configure (double deltaX, double deltaY, BeamformingMethod beamforming, uint64_t rngRun,
            uint16_t numerology, bool gNbAntennaModel, bool ueAntennaModel,
            std::string resultsDirPath, std::string tag, std::string dbName, std::string tableName);


  /**
   * \brief Function that will actually configure all the simulation parameters,
   * topology and run the simulation by using the parameters that are being
   * configured for the specific run.
   */
  void RunSimulation ();
  /**
   * \brief Destructor that closes the output file stream and finished the
   *  writing into the files.
   */
  ~CttcRealisticBeamforming ();
  /**
  * \brief Creates a string tag that contains some simulation run specific values in
  * order to be able to distinguish the results files for different runs for
  * different parameters.
  */
  std::string BuildTag ();
  /**
   * \brief
   * Prepare files for the output of the results
   */
  void PrepareOutputFiles ();
  /**
   * \brief
   * Print the statistics to the output files
   */
  void PrintResultsToFiles ();
  /**
   * \brief
   * Create traffic applications
   */
  void CreateDlTrafficApplications (ApplicationContainer& serverAppDl, ApplicationContainer& clientAppDl,
                                    NodeContainer& ueNode, Ptr<Node> remoteHost, NetDeviceContainer ueNetDev,
                                    Ipv4InterfaceContainer& ueIpIface);

  /**
   * \brief Prepare the database to print the results, e.g., open it, and
   * create the necessary table if it does not exist.
   * Method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "(Sinr DOUBLE NOT NULL, "
   * - "Distance DOUBLE NULL,"
   *   "DeltaX DOUBLE NULL,
   *   "DeltaY DOUBLE NULL,
   *   "BeamformingType TEXT NOT NULL,"
   *   "RngRun INTEGER NOT NULL,"
   *   "numerology INTEGER NOT NULL,"
   *   *gNBAntenna TEXT NOT NULL,"
   *   "ueAntenna TEXT NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * NOTE: If the database already contains a table with
   * the same name, this method will clean existing values
   * with the same Run value.
   */
  void PrepareDatabase ();

  /**
   * \brief Insert results to the table in database.
   */
  void PrintResultsToDatabase ();

private:

  //output file streams
  std::ofstream m_outSinrFile;         //!< the output file stream for the SINR file in linear scale
  std::ofstream m_outSinrFileDb;       //!< the output file stream for the SINR values in dBs
  std::ofstream m_outSnrFile;          //!< the output file stream for the SNR file
  std::ofstream m_outRssiFile;         //!< the output file stream for the RSSI file

  // database related attributes
  sqlite3 *m_db { nullptr };                          //!< DB pointer
  std::string m_tableName {"results"};                //!< Table name
  std::string m_dbName {"realistic_beamforming.db"};  //!< Database name

  // statistics objects
  MinMaxAvgTotalCalculator<double> m_sinrStats;  //!< the statistics calculator for SINR values
  MinMaxAvgTotalCalculator<double> m_snrStats;   //!< the statistics calculator for SNR values
  MinMaxAvgTotalCalculator<double> m_rssiStats;  //!< the statistics calculator for RSSI values

  // main simulation parameters that is expected that user will change often
  double m_deltaX {1};
  double m_deltaY {1};
  BeamformingMethod m_beamforming {IDEAL};
  uint32_t m_rngRun {1};
  uint16_t m_numerology {0};
  bool m_gnbAntennaModel {true};
  bool m_ueAntennaModel {true};
  std::string m_resultsDirPath {""};
  std::string m_tag {""};

  // simulation parameters that are not expected to be changed often by the user
  Time m_simTime = MilliSeconds (150);
  Time m_udpAppStartTimeDl = MilliSeconds (100);
  Time m_udpAppStopTimeDl = MilliSeconds (150);
  uint32_t m_packetSize = 1000;
  DataRate m_udpRate = DataRate ("1kbps");
  double m_centralFrequency = 28e9;
  double m_bandwidth = 100e6;
  double m_gNbHeight = 3; // gNB antenna height is 3 meters
  double m_ueHeight = 1.5; // UE antenna height is 1.5 meters
  double m_gNbTxPower = 5;
  double m_ueTxPower = 5;
  BandwidthPartInfo::Scenario m_scenario = BandwidthPartInfo::InH_OfficeMixed;
  const uint8_t m_numCcPerBand = 1;
  double m_gNbX = 0;
  double m_gNbY = 0;
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
  oss << directoryName << filePrefix << tag;
  return oss.str ();
}

std::string
CttcRealisticBeamforming:: BuildTag ()
{
  std::ostringstream oss;

  std::string algorithm = (m_beamforming == CttcRealisticBeamforming::IDEAL) ? "I" : "R";
  std::string gnbAmodel = (m_gnbAntennaModel) ? "ISO" : "3GPP";
  std::string ueAmodel = (m_ueAntennaModel) ? "ISO" : "3GPP";

  double distance2D = sqrt (m_deltaX * m_deltaX + m_deltaY * m_deltaY);

  oss << "-"   << algorithm   <<
         "-d" << distance2D <<
         "-mu" << m_numerology <<
         "-gnb" << gnbAmodel  <<
         "-ue" << ueAmodel ;

  return oss.str ();
}


void
CttcRealisticBeamforming::PrepareOutputFiles ()
{
  // If simulation tag is not provided create one, user can provide his own tag through the command line
  if (m_tag == "")
    {
      m_tag = BuildTag ();
    }
  std::string fileSinr = BuildFileNameString ( m_resultsDirPath , "sinrs", m_tag);
  std::string fileSinrDb  = BuildFileNameString ( m_resultsDirPath , "sinrsDb", m_tag);
  std::string fileSnr = BuildFileNameString ( m_resultsDirPath , "snrs", m_tag);
  std::string fileRssi = BuildFileNameString ( m_resultsDirPath , "rssi", m_tag);

  m_outSinrFile.open (fileSinr.c_str (), std::_S_app);
  m_outSinrFile.setf (std::ios_base::fixed);
  NS_ABORT_MSG_IF (!m_outSinrFile.is_open (), "Can't open file " << fileSinr);

  m_outSinrFileDb.open (fileSinrDb.c_str (), std::_S_app);
  m_outSinrFileDb.setf (std::ios_base::fixed);
  NS_ABORT_MSG_IF (!m_outSinrFileDb.is_open (), "Can't open file " << fileSinrDb);

  m_outSnrFile.open (fileSnr.c_str (), std::_S_app);
  m_outSnrFile.setf (std::ios_base::fixed);
  NS_ABORT_MSG_IF (!m_outSnrFile.is_open (), "Can't open file " << fileSnr);

  m_outRssiFile.open (fileRssi.c_str (), std::_S_app);
  m_outRssiFile.setf (std::ios_base::fixed);
  NS_ABORT_MSG_IF (!m_outRssiFile.is_open(), "Can't open file " << fileRssi);
}

void
CttcRealisticBeamforming::PrepareDatabase ()
{
  int rc = sqlite3_open (m_dbName.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Failed to open DB");

  std::string cmd = "CREATE TABLE IF NOT EXISTS " + m_tableName + " ("
                    "SINR DOUBLE NOT NULL, "
                    "Distance DOUBLE NOT NULL,"
                    "DeltaX DOUBLE NOT NULL,"
                    "DeltaY DOUBLE NOT NULL,"
                    "BeamformingType TEXT NOT NULL,"
                    "RngRun INTEGER NOT NULL,"
                    "Numerology INTEGER NOT NULL,"
                    "GnbAntenna TEXT NOT NULL,"
                    "UeAntenna TEXT NOT NULL);";

   sqlite3_stmt *stmt;

   // prepare the statement for creating the table
   do
     {
       rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
     }
   while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
   // check if it went correctly
   NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not prepare correctly the statement for creating the table. Db error:" << sqlite3_errmsg (m_db)
                        <<"full command is: \n"<<cmd);

   // execute a step operation on a statement until the result is ok or an error
   do
      {
        rc = sqlite3_step (stmt);
      }
    while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
   // check if it went correctly
   NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement for creating the table. Db error:" << sqlite3_errmsg (m_db));

   // finalize the statement until the result is ok or an error occures
   do
     {
       rc = sqlite3_finalize (stmt);
     }
   while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
   // check if it went correctly
   NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement for creating the table. Db error:" << sqlite3_errmsg (m_db));
}

void
CttcRealisticBeamforming::PrintResultsToDatabase ()
{
  sqlite3_stmt *stmt;
  std::string cmd = "INSERT INTO " + m_tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
  std::string beamformingType = (m_beamforming == IDEAL)? "Ideal":"Real";
  std::string gnbAntenna = (m_gnbAntennaModel) ? "Iso":"3gpp";
  std::string ueAntenna = (m_ueAntennaModel) ? "Iso":"3gpp";
  int rc;
  double distance2D = sqrt (m_deltaX * m_deltaX + m_deltaY * m_deltaY);

  // prepare the statement for creating the table
  do
    {
      rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  // check if it went correctly
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not prepare correctly the insert into the table statement. "
                                                             " Db error:" << sqlite3_errmsg (m_db)<<". The full command is: \n"<<cmd);

  // add all parameters to the command
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 1, m_sinrStats.getMean()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 2, distance2D) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 3, m_deltaX) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 4, m_deltaY) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 5, beamformingType.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 6, m_rngRun) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 7, m_numerology) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 8, gnbAntenna.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 9, ueAntenna.c_str(), -1, SQLITE_STATIC) == SQLITE_OK);

  // finalize the command
   do
      {
        rc = sqlite3_step (stmt);
      }
    while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
   // check if it went correctly
   NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error:" << sqlite3_errmsg (m_db));
  do
    {
      rc = sqlite3_finalize (stmt);
    }
  while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error:" << sqlite3_errmsg (m_db));

}

void
CttcRealisticBeamforming::CreateDlTrafficApplications (ApplicationContainer& serverAppDl, ApplicationContainer& clientAppDl,
                                                     NodeContainer& ueNode, Ptr<Node> remoteHost, NetDeviceContainer ueNetDev,
                                                     Ipv4InterfaceContainer& ueIpIface)
{
  uint16_t dlPort = 1234;
  //Calculate UDP interval based on the packetSize and desired udp rate
  Time udpInterval = Time::FromDouble ((m_packetSize * 8) / static_cast<double> (m_udpRate.GetBitRate ()), Time::S);
  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverAppDl.Add (dlPacketSinkHelper.Install (ueNode));
  // Configure UDP downlink traffic
  for (uint32_t i = 0 ; i < ueNetDev.GetN (); i ++)
    {
      UdpClientHelper dlClient (ueIpIface.GetAddress (i), dlPort);
      dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      dlClient.SetAttribute("PacketSize", UintegerValue (m_packetSize));
      dlClient.SetAttribute ("Interval", TimeValue (udpInterval)); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
      clientAppDl.Add (dlClient.Install (remoteHost));
    }

  // Start UDP server and client app, and configure stop time
  serverAppDl.Start (m_udpAppStartTimeDl);
  clientAppDl.Start (m_udpAppStartTimeDl);
  serverAppDl.Stop (m_udpAppStopTimeDl);
  clientAppDl.Stop (m_udpAppStopTimeDl);
}

/**
 * A callback function that redirects a call to the simulation setup instance.
 * @param simSetup A pointer to a simulation instance
 * @param params RxPacketTraceParams structure containing RX parameters
 */
void UeReceptionTrace (CttcRealisticBeamforming* simSetup, RxPacketTraceParams params)
{
  simSetup->UeReception (params);
 }

/**
 * A callback function that redirects a call to the scenario instance.
 * @param simSetup A pointer to a simulation instance
 * @param snr SNR value
 */
void UeSnrPerProcessedChunkTrace (CttcRealisticBeamforming* simSetup, double snr)
{
  simSetup->UeSnrPerProcessedChunk (snr);
}

/**
 * A callback function that redirects a call to the scenario instance.
 * @param simSetup A pointer to a simulation instance
 * @param rssidBm rssidBm RSSI value in dBm
 */
void UeRssiPerProcessedChunkTrace (CttcRealisticBeamforming* simSetup, double rssidBm)
{
  simSetup->UeRssiPerProcessedChunk (rssidBm);
}

void
CttcRealisticBeamforming::UeReception (RxPacketTraceParams params)
{
  m_sinrStats.Update (params.m_sinr); // we have to pas the linear value
}

void
CttcRealisticBeamforming::UeSnrPerProcessedChunk (double snr)
{
  m_snrStats.Update (snr);
}

void
CttcRealisticBeamforming::UeRssiPerProcessedChunk (double rssidBm)
{
  m_rssiStats.Update (rssidBm);
}

CttcRealisticBeamforming::~CttcRealisticBeamforming ()
{
  m_outSinrFile.close ();
  m_outSinrFileDb.close ();
  m_outSnrFile.close ();
  m_outRssiFile.close ();

  // Failed to close the database
  int rc = SQLITE_FAIL;
  rc = sqlite3_close_v2 (m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Failed to close DB");
}

void
CttcRealisticBeamforming::PrintResultsToFiles ()
{
  m_outSinrFile << m_sinrStats.getMean () << std::endl;
  m_outSinrFileDb << 10 * log10 (m_sinrStats.getMean ()) << std::endl;
  m_outSnrFile <<  10 * log10 (m_snrStats.getMean ()) << std::endl;
  m_outRssiFile << 10 * log10 (m_rssiStats.getMean ()) << std::endl;
}

void
CttcRealisticBeamforming::Configure (double deltaX, double deltaY, BeamformingMethod beamforming, uint64_t rngRun,
                                     uint16_t numerology, bool gNbAntennaModel, bool ueAntennaModel,
                                     std::string resultsDirPath, std::string tag, std::string dbName, std::string tableName)

{
  m_deltaX = deltaX;
  m_deltaY = deltaY;
  m_beamforming = beamforming;
  m_rngRun = rngRun;
  m_numerology = numerology;
  m_gnbAntennaModel = gNbAntennaModel;
  m_ueAntennaModel = ueAntennaModel;
  m_resultsDirPath = resultsDirPath;
  m_tag = tag;
}

void
CttcRealisticBeamforming::RunSimulation ()
{
  // Set simulation run number
  SeedManager::SetRun (m_rngRun);

  // Create gNB and UE nodes
  NodeContainer gNbNode;
  NodeContainer ueNode;
  gNbNode.Create (1);
  ueNode.Create (1);

  // Set positions
  Ptr<ListPositionAllocator> positions = CreateObject<ListPositionAllocator> ();
  positions -> Add (Vector (m_gNbX, m_gNbY, m_gNbHeight));  //gNb will take this position
  positions -> Add (Vector (m_gNbX + m_deltaX, m_gNbY + m_deltaY, m_ueHeight)); //UE will take this position
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positions);
  mobility.Install (gNbNode);
  mobility.Install (ueNode);

  // Create NR helpers: nr helper, epc helper, and beamforming helper
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();

  // Initialize beamforming
  Ptr<BeamformingHelperBase> beamformingHelper;
  if (m_beamforming == CttcRealisticBeamforming::IDEAL)
    {
      beamformingHelper = CreateObject<IdealBeamformingHelper> ();
      beamformingHelper->SetBeamformingMethod (CellScanBeamforming::GetTypeId());
    }
  else if (m_beamforming == CttcRealisticBeamforming::REALISTIC)
    {
      beamformingHelper = CreateObject<RealisticBeamformingHelper> ();
      beamformingHelper->SetBeamformingMethod (SrsRealisticBeamformingAlgorithm::GetTypeId());
    }
  nrHelper->SetBeamformingHelper (beamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Configure the spectrum division: single operational band, containing single
   * component carrier, which contains a single bandwidth part.
   *
   * |------------------------Band-------------------------|
   * |-------------------------CC--------------------------|
   * |-------------------------BWP-------------------------|
   *
   */
  CcBwpCreator ccBwpCreator;
  // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates a single BWP per CC
  CcBwpCreator::SimpleOperationBandConf bandConf (m_centralFrequency,
                                                  m_bandwidth,
                                                  m_numCcPerBand,
                                                  m_scenario);
  // By using the configuration created, make the operation band
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
  nrHelper->InitializeOperationBand (&band);
  BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps ({band});

  // Configure antenna of gNb
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (m_gnbAntennaModel));
  // Configure antenna of UE
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (m_ueAntennaModel));

  // Install nr net devices
  NetDeviceContainer gNbDev = nrHelper->InstallGnbDevice (gNbNode, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNode, allBwps);

  for (uint32_t i = 0 ; i < gNbDev.GetN (); i ++)
    {
      nrHelper->GetGnbPhy (gNbDev.Get (i), 0)->SetAttribute ("Numerology", UintegerValue (m_numerology));
      nrHelper->GetGnbPhy (gNbDev.Get (i), 0)->SetAttribute ("TxPower", DoubleValue (10*log10 (m_gNbTxPower)));
    }
  for (uint32_t j = 0; j < ueNetDev.GetN (); j++)
    {
      nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (10*log10 (m_ueTxPower)));
    }

  // Update configuration
  for (auto it = gNbDev.Begin (); it != gNbDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  // Create the internet and install the IP stack on the UEs, get SGW/PGW and create a single RemoteHost
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

  // Configure routing
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNode);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // Set the default gateway for the UE
  for (uint32_t j = 0; j < ueNode.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode.Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach UE to gNB
  nrHelper->AttachToEnb (ueNetDev.Get(0), gNbDev.Get(0));

  // Install UDP downlink applications
  ApplicationContainer clientAppDl;
  ApplicationContainer serverAppDl;
  CreateDlTrafficApplications (clientAppDl, serverAppDl, ueNode, remoteHost, ueNetDev, ueIpIface);

  // Connect traces to our listener functions
  for (uint32_t i = 0 ; i < ueNetDev.GetN (); i ++)
    {
      Ptr<NrSpectrumPhy> ue1SpectrumPhy = DynamicCast <NrUeNetDevice> (ueNetDev.Get (i))->GetPhy (0)->GetSpectrumPhy ();
      ue1SpectrumPhy->TraceConnectWithoutContext ("RxPacketTraceUe", MakeBoundCallback (&UeReceptionTrace, this));
      Ptr<nrInterference> ue1SpectrumPhyInterference = ue1SpectrumPhy->GetNrInterference ();
      NS_ABORT_IF (!ue1SpectrumPhyInterference);
      ue1SpectrumPhyInterference->TraceConnectWithoutContext ("SnrPerProcessedChunk", MakeBoundCallback (&UeSnrPerProcessedChunkTrace, this));
      ue1SpectrumPhyInterference->TraceConnectWithoutContext ("RssiPerProcessedChunk", MakeBoundCallback (&UeRssiPerProcessedChunkTrace, this));
    }

  Simulator::Stop (m_simTime);
  Simulator::Run ();
  Simulator::Destroy ();
}

int
main (int argc, char *argv[])
{
  uint16_t numerology = 2;
  bool enableGnbIso = true;
  bool enableUeIso = true;
  std::string algType = "Ideal";
  std::string resultsDir = "./";
  std::string simTag = "";
  double deltaX = 10.0;
  double deltaY = 10.0;
  CttcRealisticBeamforming::BeamformingMethod beamformingType;
  uint64_t rngRun = 1;
  double ueTxPower = 0;
  double gnbTxPower = 0;
  std::string dbName = "realistic-beamforming.db";
  std::string tableName = "results";

  CommandLine cmd;

  cmd.AddValue ("deltaX",
                "Determines X coordinate of UE wrt. to gNB X coordinate [meters].",
                deltaX);
  cmd.AddValue ("deltaY",
                "Determines Y coordinate of UE wrt. to gNB Y coordinate [meters].",
                deltaY);
  cmd.AddValue ("algType",
                "Algorithm type to be used. Can be: 'Ideal' or 'Real'.",
                algType);
  cmd.AddValue ("rngRun",
                "Rng run random number.",
                rngRun);
  cmd.AddValue ("enableGnbIso",
                "Configure isotropic antenna elements at gNB. "
                "For ISO value to be provided is 1, for 3GPP value to be provide is 0.",
                enableGnbIso);
  cmd.AddValue ("enableGnbIso",
                "Configure isotropic antenna elements at UE."
                "For ISO value to be provided is 1, for 3GPP value to be provide is 0.",
                enableUeIso);
  cmd.AddValue ("resultsDir",
                "Directory where to store the simulation results.",
                resultsDir);
  cmd.AddValue ("simTag",
                "Tag to be appended to output filenames to distinguish simulation campaigns.",
                simTag);
  cmd.AddValue ("dbName",
                "Database name.",
                 dbName);
  cmd.AddValue ("tableName",
                "Table name.",
                 tableName);
  cmd.AddValue ("ueTxPower",
                "Tx power to be used by the UE [dBm].",
                ueTxPower);
  cmd.AddValue ("gnbTxPower",
                "Tx power to be used by the gNB [dBm].",
                gnbTxPower);


  cmd.Parse (argc, argv);


  if (algType == "Ideal")
    {
      beamformingType = CttcRealisticBeamforming::IDEAL;
    }
  else if (algType == "Real")
    {
      beamformingType = CttcRealisticBeamforming::REALISTIC;
    }
  else
    {
      NS_ABORT_MSG ("Not supported value for algType:" << algType);
    }

  CttcRealisticBeamforming simpleBeamformingScenario;
  simpleBeamformingScenario.Configure (deltaX, deltaY, beamformingType, rngRun, numerology, enableGnbIso,
                                       enableUeIso, resultsDir, simTag, dbName, tableName);
  simpleBeamformingScenario.PrepareDatabase ();
  simpleBeamformingScenario.PrepareOutputFiles ();
  simpleBeamformingScenario.RunSimulation ();
  simpleBeamformingScenario.PrintResultsToDatabase ();
  simpleBeamformingScenario.PrintResultsToFiles ();
}

