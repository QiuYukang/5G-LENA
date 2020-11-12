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
   * Function that will actually configure all the simulation parameters,
   * topology and run the simulation by using the parameters that are being
   * configured for the specific run.
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
  void Run (double deltaX, double deltaY, BeamformingMethod beamforming, uint64_t rngRun,
            uint16_t numerology, bool gNbAntennaModel, bool ueAntennaModel,
            std::string resultsDirPath, std::string tag);
  /**
   * \brief Destructor that closes the output file stream and finished the
   * writing into the files.
   */
  ~CttcRealisticBeamforming ();

private:

  std::ofstream m_outSinrFile;         //!< the output file stream for the SINR file
  std::ofstream m_outSnrFile;          //!< the output file stream for the SNR file
  std::ofstream m_outRssiFile;         //!< the output file stream for the RSSI file

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

/**
 * Creates a string tag that contains some simulation run specific values in
 * order to be able to distinguish the results files for different runs for
 * different parameters.
 * @param doubleX delta that defines UE X coordinate wrt gNB position
 * @param doubleY delta that defines UE Y coordinate wrt gNB position
 * @param beamformingMethod BeamformingMethod that will be used (IDEAL or REALISTIC)
 * @param gNbAntennaModel gNb antenna model
 * @param ueAntennaModel UE antenna model
 * @param scenario The indoor scenario to be used
 * @param speed The speed of UEs in km/h
 * @return the parameter specific simulation name
 */
std::string
BuildTag (double deltaX, double deltaY, CttcRealisticBeamforming::BeamformingMethod beamformingMethod,
          uint32_t rngRun, uint16_t numerology, bool gNbAntennaModel, bool ueAntennaModel)
{
  std::ostringstream oss;

  std::string algorithm = (beamformingMethod == CttcRealisticBeamforming::IDEAL) ? "I" : "R";
  std::string gnbAmodel = (gNbAntennaModel) ? "ISO" : "3GPP";
  std::string ueAmodel = (ueAntennaModel) ? "ISO" : "3GPP";

  oss << "-"   << algorithm  <<
         "-dX" << deltaX     <<
         "-dY" << deltaY     <<
         "-r"  << rngRun     <<
         "-mu" << numerology <<
         "-aG" << gnbAmodel  <<
         "-aU" << ueAmodel ;

  return oss.str ();
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
  m_outSinrFile << params.m_cellId << params.m_rnti <<
                   "\t" << 10*log10 (params.m_sinr) << std::endl;
}

void
CttcRealisticBeamforming::UeSnrPerProcessedChunk (double snr)
{
  m_outSnrFile << 10 * log10 (snr) << std::endl;
}


void
CttcRealisticBeamforming::UeRssiPerProcessedChunk (double rssidBm)
{
  m_outRssiFile << rssidBm << std::endl;
}

CttcRealisticBeamforming::~CttcRealisticBeamforming ()
{
  m_outSinrFile.close ();
  m_outSnrFile.close ();
  m_outRssiFile.close ();
}

void
CttcRealisticBeamforming::Run (double deltaX, double deltaY, BeamformingMethod beamforming, uint64_t rngRun,
                               uint16_t numerology, bool gNbAntennaModel, bool ueAntennaModel,
                               std::string resultsDirPath, std::string tag)
{
  uint32_t duration = 150; // in ms
  Time simTime = MilliSeconds (duration);
  Time udpAppStartTimeDl = MilliSeconds (100);
  Time udpAppStopTimeDl = MilliSeconds (duration);
  uint32_t packetSize = 1000;
  DataRate udpRate = DataRate ("1kbps");
  double centralFrequency = 28e9;
  double bandwidth = 100e6;
  double gNbHeight = 3; // gNB antenna height is 3 meters
  double ueHeight = 1.5; // UE antenna height is 1.5 meters
  double gNbTxPower = 5;
  double ueTxPower = 5;
  BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::InH_OfficeMixed;

  SeedManager::SetRun (rngRun);

  // if simulation tag is not provided create one
  if (tag == "")
    {
      tag = BuildTag (deltaX, deltaY, beamforming, rngRun, numerology, gNbAntennaModel, ueAntennaModel);
    }
  std::string fileSinr = BuildFileNameString ( resultsDirPath , "sinrs", tag);
  std::string fileSnr = BuildFileNameString ( resultsDirPath , "snrs", tag);
  std::string fileRssi = BuildFileNameString ( resultsDirPath , "rssi", tag);

  m_outSinrFile.open (fileSinr.c_str ());
  m_outSinrFile.setf (std::ios_base::fixed);
  NS_ABORT_MSG_IF (!m_outSinrFile.is_open (), "Can't open file " << fileSinr);

  m_outSnrFile.open (fileSnr.c_str ());
  m_outSnrFile.setf (std::ios_base::fixed);
  NS_ABORT_MSG_IF (!m_outSnrFile.is_open (), "Can't open file " << fileSnr);

  m_outRssiFile.open (fileRssi.c_str ());
  m_outRssiFile.setf (std::ios_base::fixed);
  NS_ABORT_MSG_IF (!m_outRssiFile.is_open(), "Can't open file " << fileRssi);

  // create gNB and UE nodes
  NodeContainer gNbNode;
  NodeContainer ueNode;
  gNbNode.Create (1);
  ueNode.Create (1);

  // set positions
  Ptr<ListPositionAllocator> positions = CreateObject<ListPositionAllocator> ();
  positions -> Add (Vector (0, 0, gNbHeight));  //gNb will take this position
  positions -> Add (Vector (deltaX, deltaY, ueHeight)); //UE will take this position
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positions);
  mobility.Install (gNbNode);
  mobility.Install (ueNode);

  // Create NR helpers: nr helper, epc helper, and beamforming helper
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();

  // initialize beamforming
  Ptr<BeamformingHelperBase> beamformingHelper;
  if (beamforming == CttcRealisticBeamforming::IDEAL)
    {
      beamformingHelper = CreateObject<IdealBeamformingHelper> ();
      beamformingHelper->SetBeamformingMethod (CellScanBeamforming::GetTypeId());
    }
  else if (beamforming == CttcRealisticBeamforming::REALISTIC)
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
  const uint8_t numCcPerBand = 1;
  // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates a single BWP per CC
  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency,
                                                  bandwidth,
                                                  numCcPerBand,
                                                  scenario);
  // By using the configuration created, make the operation band
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
  nrHelper->InitializeOperationBand (&band);
  BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps ({band});

  // Configure antenna of gNb
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (gNbAntennaModel));
  // Configure antenna of UE
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (ueAntennaModel));

  // install nr net devices
  NetDeviceContainer gNbDev = nrHelper->InstallGnbDevice (gNbNode, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNode, allBwps);

  for (uint32_t i = 0 ; i < gNbDev.GetN (); i ++)
    {
      nrHelper->GetGnbPhy (gNbDev.Get (i), 0)->SetAttribute ("Numerology", UintegerValue (numerology));
      nrHelper->GetGnbPhy (gNbDev.Get (i), 0)->SetAttribute ("TxPower", DoubleValue (10*log10 (gNbTxPower)));
    }
  for (uint32_t j = 0; j < ueNetDev.GetN (); j++)
    {
      nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (10*log10 (ueTxPower)));
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
  // in this container, interface 0 is the pgw, 1 is the remoteHost
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

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
  uint16_t dlPort = 1234;
  ApplicationContainer clientAppDl;
  ApplicationContainer serverAppDl;
  //Calculate UDP interval based on the packetSize and desired udp rate
  Time udpInterval = Time::FromDouble ((packetSize * 8) / static_cast<double> (udpRate.GetBitRate ()), Time::S);
  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverAppDl.Add (dlPacketSinkHelper.Install (ueNode));
  // Configure UDP downlink traffic
  for (uint32_t i = 0 ; i < ueNetDev.GetN (); i ++)
    {
      UdpClientHelper dlClient (ueIpIface.GetAddress (i), dlPort);
      dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      dlClient.SetAttribute("PacketSize", UintegerValue (packetSize));
      dlClient.SetAttribute ("Interval", TimeValue (udpInterval)); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
      clientAppDl.Add (dlClient.Install (remoteHost));
    }

  // Start UDP server and client app, and configure stop time
  serverAppDl.Start (udpAppStartTimeDl);
  clientAppDl.Start (udpAppStartTimeDl);
  serverAppDl.Stop (udpAppStopTimeDl);
  clientAppDl.Stop (udpAppStopTimeDl);

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

  Simulator::Stop (simTime);
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

  CommandLine cmd;

  cmd.AddValue ("deltaX",
                "Determines X coordinate of UE wrt to gNB X coordinate.",
                deltaX);
  cmd.AddValue ("deltaY",
                "Determines Y coordinate of UE wrt to gNB Y coordinate.",
                deltaY);
  cmd.AddValue ("algType",
                "Algorithm type to be used. Can be: Ideal or Real.",
                algType);
  cmd.AddValue ("rngRun",
                "Rng run random number",
                rngRun);
  cmd.AddValue ("enableGnbIso",
                "Configure isotropic antenna elements at gNB",
                enableGnbIso);
  cmd.AddValue ("enableGnbIso",
                "Configure Isotropic antenna elements at UE",
                enableUeIso);
  cmd.AddValue ("resultsDir",
                "directory where to store the simulation results",
                resultsDir);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);

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
      NS_ABORT_MSG ("Not supported value for algType:"<<algType);
    }


  CttcRealisticBeamforming simpleBeamformingScenario;
  simpleBeamformingScenario.Run (deltaX, deltaY, beamformingType, rngRun,
                                 numerology, enableGnbIso, enableUeIso,
                                 resultsDir, simTag);

  return 0;
}

