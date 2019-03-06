/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
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
#include "ns3/antenna-array-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcSimpleInterferenceExample");


class Scenario
{
public:
  virtual ~Scenario () { }
  virtual const NodeContainer & GetGnbs () const = 0;
  virtual const NodeContainer & GetUes () const = 0;
};

class SimpleInterferenceScenario : public Scenario
{
public:
  enum ScenarioMode
  {
    BASIC = 0,
    NO_INTERF = 1,
  };

  SimpleInterferenceScenario (uint32_t gnbNum, const Vector& gnbReferencePos, double ueY,
                              ScenarioMode scenario);
  ~SimpleInterferenceScenario () { }

  virtual const NodeContainer & GetGnbs () const { return m_gNb; }
  virtual const NodeContainer & GetUes () const { return m_ue; }

private:
  NodeContainer m_gNb;
  NodeContainer m_ue;
};

SimpleInterferenceScenario::SimpleInterferenceScenario (uint32_t gnbNum,
                                                        const Vector& gnbReferencePos,
                                                        double ueX,
                                                        ScenarioMode scenario)
{
  // create base stations and mobile terminals
  static MobilityHelper mobility;

  m_gNb.Create (gnbNum);
  m_ue.Create (gnbNum);

  for (uint32_t i = 0; i < gnbNum; ++i)
    {
      std::stringstream ss;
      ss << "gNb" << m_gNb.Get(i)->GetId();
      Names::Add(ss.str(), m_gNb.Get(i));
      std::cout << " GNB ID " << m_gNb.Get(i)->GetId() << std::endl;
    }

  for (uint32_t i = 0; i < gnbNum; ++i)
    {
      std::stringstream ss;
      ss << "UE" << m_ue.Get(i)->GetId();
      Names::Add(ss.str(), m_ue.Get(i));
      std::cout << " UE ID " << m_ue.Get(i)->GetId() << std::endl;
    }

  Ptr<ListPositionAllocator> gnbPos = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> uePos = CreateObject<ListPositionAllocator> ();

  if (scenario == BASIC || scenario == NO_INTERF)
    {
      double delta = 0.0;
      for (uint32_t i = 0; i < gnbNum; ++i)
        {
          Vector pos (gnbReferencePos);
          pos.y = pos.y + delta;
          delta += 0.5;
          std::cout << "gnb " << i << " pos " << pos << std::endl;
          gnbPos->Add (pos);
        }
    }

  if (scenario == BASIC)
    {
      double delta = 0.0;
      for (uint32_t i = 0; i < gnbNum; ++i)
        {
          Vector pos (gnbReferencePos.x + ueX, gnbReferencePos.y + delta, 1.5);
          delta += 0.5;
          std::cout << "ue " << i << " pos " << pos << std::endl;
          uePos->Add(pos);
        }
    }
  else if (scenario == NO_INTERF)
    {
      uePos->Add (Vector (gnbReferencePos.x + ueX, gnbReferencePos.y, 1.5));
      std::cout << "ue2 " " pos " << Vector (gnbReferencePos.x + ueX, gnbReferencePos.y, 1.5) << std::endl;
      uePos->Add (Vector (20, 20, 1.5));
      std::cout << "ue3 " " pos " << Vector (20, 20, 1.5) << std::endl;
    }



  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gnbPos);
  mobility.Install (m_gNb);

  mobility.SetPositionAllocator (uePos);
  mobility.Install (m_ue);
}

class NrSetup
{
public:
  virtual ~NrSetup () { }
  virtual Ptr<NrPointToPointEpcHelper> GetEpcHelper () const = 0;
  virtual const NetDeviceContainer & GetUeDev () const = 0;
  virtual const NetDeviceContainer & GetGnbDev () const = 0;
};

class OutputManager
{
public:
  OutputManager (const std::string &prefix)
  {
    m_outSinrFile.open ((prefix + "-sinr.txt").c_str (), std::ios::trunc);
  }

  void UeReceive (RxPacketTraceParams params);
  void UeSnrPerProcessedChunk (double snr);

private:
  std::string m_prefix;
  std::ofstream m_outSinrFile;
  std::ofstream m_outSnrFile;
  std::ofstream m_outRssiFile;
};

void OutputManager::UeReceive (RxPacketTraceParams params)
{
  m_outSinrFile << params.m_cellId << " " << params.m_rnti
                << " " << params.m_sinr
                << std::endl;
}

void
OutputManager::UeSnrPerProcessedChunk (double snr)
{
  m_outSnrFile << 10*log10(snr) << std::endl;
}

class NrSingleBwpSetup : public NrSetup
{
public:
  NrSingleBwpSetup (const Scenario &scenario, double freq, double bw,
                    uint32_t num, double txPower, OutputManager *manager);

  virtual Ptr<NrPointToPointEpcHelper> GetEpcHelper () const { return m_epcHelper; }
  virtual Ptr<MmWaveHelper> GetHelper () const { return m_helper; }
  virtual const NetDeviceContainer & GetUeDev () const { return m_ueDev; }
  virtual const NetDeviceContainer & GetGnbDev () const { return m_gnbDev; }

private:
  Ptr<MmWaveHelper> m_helper;
  Ptr<NrPointToPointEpcHelper> m_epcHelper;
  NetDeviceContainer m_ueDev;
  NetDeviceContainer m_gnbDev;
  OutputManager *m_manager;
};

NrSingleBwpSetup::NrSingleBwpSetup (const Scenario &scenario, double freq,
                                    double bw, uint32_t num, double txPower,
                                    OutputManager *manager)
{
  m_manager = manager;
  // setup the mmWave simulation
  m_helper = CreateObject<MmWaveHelper> ();
  m_helper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
  m_helper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));

  Ptr<BandwidthPartsPhyMacConf> bwpConf = CreateObject <BandwidthPartsPhyMacConf> ();

  Ptr<MmWavePhyMacCommon> phyMacCommonBwp1 = CreateObject<MmWavePhyMacCommon>();
  phyMacCommonBwp1->SetCentreFrequency(freq);
  phyMacCommonBwp1->SetBandwidth (bw);
  phyMacCommonBwp1->SetNumerology(num);
  phyMacCommonBwp1->SetAttribute ("MacSchedulerType", TypeIdValue (MmWaveMacSchedulerTdmaRR::GetTypeId ()));
  phyMacCommonBwp1->SetCcId(0);

  bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp1);

  m_helper->SetBandwidthPartMap (bwpConf);

  m_epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  m_helper->SetEpcHelper (m_epcHelper);
  m_helper->Initialize();

  // install mmWave net devices
  m_gnbDev = m_helper->InstallEnbDevice (scenario.GetGnbs());
  m_ueDev = m_helper->InstallUeDevice (scenario.GetUes());

  double x = pow(10, txPower/10);

  double totalBandwidth = bw;

  for (uint32_t j = 0; j < m_gnbDev.GetN(); ++j)
    {
      ObjectMapValue objectMapValue;
      Ptr<MmWaveEnbNetDevice> netDevice = DynamicCast<MmWaveEnbNetDevice>(m_gnbDev.Get(j));
      netDevice->GetAttribute("ComponentCarrierMap", objectMapValue);
      for (uint32_t i = 0; i < objectMapValue.GetN(); i++)
        {
          Ptr<ComponentCarrierGnb> bandwidthPart = DynamicCast<ComponentCarrierGnb>(objectMapValue.Get(i));
          if (i==0)
            {
              bandwidthPart->GetPhy()->SetTxPower(10*log10((bw/totalBandwidth)*x));
            }
          else
            {
              NS_FATAL_ERROR ("\n Please extend power assignment for additional bandwidht parts...");
            }
        }
    }

  for (uint32_t i = 0 ; i < m_ueDev.GetN(); ++i)
    {
      Ptr<MmWaveSpectrumPhy > ue1SpectrumPhy = DynamicCast<MmWaveUeNetDevice>
      (m_ueDev.Get(i))->GetPhy(0)->GetDlSpectrumPhy();
      ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe", MakeCallback (&OutputManager::UeReceive, m_manager));
      Ptr<mmWaveInterference> ue1SpectrumPhyInterference = ue1SpectrumPhy->GetMmWaveInterference();
      NS_ABORT_IF(!ue1SpectrumPhyInterference);
      //ue1SpectrumPhyInterference->TraceConnectWithoutContext("SnrPerProcessedChunk", MakeBoundCallback (&UeSnrPerProcessedChunkTrace, this));
      //ue1SpectrumPhyInterference->TraceConnectWithoutContext("RssiPerProcessedChunk", MakeBoundCallback (&UeRssiPerProcessedChunkTrace, this));
    }

  // enable the traces provided by the mmWave module
  // m_helper->EnableTraces();
}

static void
ConfigureDefaultValues (bool cellScan = true, double beamSearchAngleStep = 10.0,
                        uint32_t eesmTable = 1, uint32_t mcs = 13,
                        const std::string &errorModel = "ns3::NrEesmErrorModel")
{
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition",
                      StringValue("l"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario",
                      StringValue("InH-OfficeMixed")); // with antenna height of 10 m
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing",
                      BooleanValue(false));

  Config::SetDefault ("ns3::MmWave3gppChannel::CellScan",
                      BooleanValue(cellScan));
  Config::SetDefault ("ns3::MmWave3gppChannel::UpdatePeriod",
                      TimeValue(MilliSeconds(200)));
  Config::SetDefault ("ns3::MmWave3gppChannel::BeamSearchAngleStep",
                      DoubleValue(beamSearchAngleStep));

  Config::SetDefault ("ns3::MmWaveEnbPhy::AntennaNumDim1", UintegerValue (4));
  Config::SetDefault ("ns3::MmWaveEnbPhy::AntennaNumDim2", UintegerValue (8));

  Config::SetDefault ("ns3::MmWaveUePhy::AntennaNumDim1", UintegerValue (2));
  Config::SetDefault ("ns3::MmWaveUePhy::AntennaNumDim2", UintegerValue (4));

  Config::SetDefault("ns3::AntennaArrayModel::AntennaOrientation", EnumValue (AntennaArrayModel::X0));


  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize",
                      UintegerValue(999999999));
  Config::SetDefault ("ns3::MmWaveHelper::NumberOfComponentCarriers", UintegerValue (1));

  Config::SetDefault("ns3::PointToPointEpcHelper::S1uLinkDelay", TimeValue (MilliSeconds(0)));
  Config::SetDefault("ns3::PointToPointEpcHelper::X2LinkDelay", TimeValue (MilliSeconds(0)));

  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::FixedMcsDl", BooleanValue(false));
  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::FixedMcsUl", BooleanValue(false));
  //Config::SetDefault("ns3::MmWaveMacSchedulerNs3::StartingMcsDl", UintegerValue (mcs));
  //Config::SetDefault("ns3::MmWaveMacSchedulerNs3::StartingMcsUl", UintegerValue (mcs));

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
  Config::SetDefault("ns3::NrAmc::AmcModel", EnumValue (NrAmc::PiroEW2010));
  Config::SetDefault("ns3::MmWaveSpectrumPhy::ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));
}

int
main (int argc, char *argv[])
{
  uint32_t mcs = 13;
  uint16_t gNbNum = 1;
  bool cellScan = false;
  double beamSearchAngleStep = 30.0;
  double totalTxPower = 4;
  uint16_t numerologyBwp1 = 0;
  double frequencyBwp1 = 28e9;
  double bandwidthBwp1 = 100e6;
  double ueY = 300.0;

  double simTime = 5; // seconds
  double udpAppStartTime = 1.0; //seconds

  std::string errorModel = "ns3::NrLteMiErrorModel";
  uint32_t eesmTable = 1;

  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("mcs",
                "The MCS that will be used in this example",
                mcs);
  cmd.AddValue ("gNbNum",
                "The number of gNbs in multiple-ue topology",
                gNbNum);
  cmd.AddValue ("cellScan",
                "Use beam search method to determine beamforming vector,"
                " the default is long-term covariance matrix method"
                " true to use cell scanning method, false to use the default"
                " power method.",
                cellScan);
  cmd.AddValue ("beamSearchAngleStep",
                "Beam search angle step for beam search method",
                beamSearchAngleStep);
  cmd.AddValue ("totalTxPower",
                "total tx power that will be proportionally assigned to"
                " bandwidth parts depending on each BWP bandwidth ",
                totalTxPower);
  cmd.AddValue("errorModelType",
               "Error model type: ns3::NrEesmErrorModel , ns3::NrLteErrorModel",
               errorModel);
  cmd.AddValue("eesmTable",
               "Table to use when error model is Eesm (1 for McsTable1 or 2 for McsTable2)",
               eesmTable);
  cmd.AddValue("ueY",
               "Y position of any UE",
               ueY);

  cmd.Parse (argc, argv);

  ConfigureDefaultValues(cellScan, beamSearchAngleStep, eesmTable, mcs, errorModel);

  SimpleInterferenceScenario scenario (gNbNum, Vector(0, 0, 10), ueY, SimpleInterferenceScenario::BASIC);
  std::stringstream ss;
  ss << "cttc-simple-interference-scenario-" << ueY;
  OutputManager manager (ss.str ());

  NrSingleBwpSetup setup (scenario, frequencyBwp1, bandwidthBwp1, numerologyBwp1, 4.0, &manager);

  // create the internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost
  Ptr<Node> pgw = setup.GetEpcHelper()->GetPgwNode ();
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
  internet.Install (scenario.GetUes());
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = setup.GetEpcHelper()->AssignUeIpv4Address (setup.GetUeDev());

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < scenario.GetUes().GetN(); ++j)
    {
      auto ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (scenario.GetUes().Get(j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (setup.GetEpcHelper()->GetUeDefaultGatewayAddress (), 1);
    }

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps, serverApps;

  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverApps.Add (dlPacketSinkHelper.Install (scenario.GetUes()));

  // configure here UDP traffic
  for (uint32_t j = 0; j < scenario.GetUes().GetN(); ++j)
    {
      UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
      dlClient.SetAttribute ("MaxPackets", UintegerValue(2));
      dlClient.SetAttribute("PacketSize", UintegerValue(500));
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(10)));
      clientApps.Add (dlClient.Install (remoteHost));
    }

  // start UDP server and client apps
  serverApps.Start(Seconds(udpAppStartTime));
  clientApps.Start(Seconds(udpAppStartTime));
  serverApps.Stop(Seconds(simTime));
  clientApps.Stop(Seconds(simTime));

  // attach UEs to the closest eNB
  for (uint32_t i = 0; i < gNbNum ; ++i)
    {
      setup.GetHelper()->AttachToEnb (setup.GetUeDev().Get(i), setup.GetGnbDev().Get(i));
    }

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

