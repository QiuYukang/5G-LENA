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
#include "ns3/test.h"
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
 * \ingroup test
 * \file test-antenna-3gpp-model-conf.cc
 * \brief Test the 3GPP Antenna
 *
 * This test case checks if the throughput/SINR/MCS
 * obtained is as expected for the configured antenna model and for
 * different positions of UE. The test scenario consists of a scenario in
 * which a single UE is attached to a gNB.
 * UE performs a UDP full buffer downlink traffic.
 * gNB is configured to have 1 bandwidth part.
 * Currently there are 2 types of antenna elements: omni and 3gpp directional,
 * and they are implemented in different antenna array models:
 * AntennaArrayModel and AntennaArray3gppModel.
 *
 */


class TestAntenna3gppModelConf : public TestCase
{
public:

  enum DirectionGnbUeXYAngle
  {
    DirectionGnbUe_45,
    DirectionGnbUe_135,
    DirectionGnbUe_225,
    DirectionGnbUe_315,
    DirectionGnbUe_0,
    DirectionGnbUe_90,
    DirectionGnbUe_180,
    DirectionGnbUe_270,
  };



  TestAntenna3gppModelConf (const std::string & name, DirectionGnbUeXYAngle conf, TypeId gNbAntennaArrayModelType,
                            TypeId ueAntennaArrayModelType, uint8_t ueNoOfAntennas, std::string losCondition);
  virtual ~TestAntenna3gppModelConf ();
  void UeReception (RxPacketTraceParams params);

private:

  virtual void DoRun (void);
  std::string m_name;
  DirectionGnbUeXYAngle m_conf;
  TypeId m_ueAntennaArrayModelType;
  TypeId m_gnbAntennaArrayModelType;

  uint8_t m_ueNoOfAntennas;
  std::string m_losCondition;
  Ptr<MinMaxAvgTotalCalculator<double> > m_sinrCell1;
  Ptr<MinMaxAvgTotalCalculator<double> > m_sinrCell2;
  Ptr<MinMaxAvgTotalCalculator<double> > m_mcsCell1;
  Ptr<MinMaxAvgTotalCalculator<double> > m_mcsCell2;
  Ptr<MinMaxAvgTotalCalculator<double> > m_rbNumCell1;
  Ptr<MinMaxAvgTotalCalculator<double> > m_rbNumCell2;

};

void UETraceReception (TestAntenna3gppModelConf* test, RxPacketTraceParams params)
{
  test->UeReception(params);
 }



void
TestAntenna3gppModelConf::UeReception (RxPacketTraceParams params)
{
  if (params.m_cellId == 1)
    {
      m_sinrCell1-> Update (params.m_sinr);
      m_mcsCell1->Update (params.m_mcs);
      m_rbNumCell1->Update (params.m_rbAssignedNum);
    }
  else if (params.m_cellId == 2)
    {
      m_sinrCell2->Update (params.m_sinr);
      m_mcsCell2->Update (params.m_mcs);
      m_rbNumCell2->Update (params.m_rbAssignedNum);
    }
  else
    {
      NS_ABORT_MSG("Cell does not exist ... ");
    }
}

TestAntenna3gppModelConf::TestAntenna3gppModelConf (const std::string & name, DirectionGnbUeXYAngle conf, TypeId gnbAntennaModelType,
                                                    TypeId ueAntennaModelType, uint8_t ueNoOfAntennas, std::string losCondition)
: TestCase (name)
{
  m_name = name;
  m_conf = conf;
  m_gnbAntennaArrayModelType = gnbAntennaModelType;
  m_ueAntennaArrayModelType = ueAntennaModelType;
  m_ueNoOfAntennas = ueNoOfAntennas;
  m_losCondition = losCondition;
  m_sinrCell1 = Create<MinMaxAvgTotalCalculator<double> >();
  m_sinrCell2 = Create<MinMaxAvgTotalCalculator<double> >();
  m_mcsCell1 = Create<MinMaxAvgTotalCalculator<double> >();
  m_mcsCell2 = Create<MinMaxAvgTotalCalculator<double> >();
  m_rbNumCell1 = Create<MinMaxAvgTotalCalculator<double> >();
  m_rbNumCell2 = Create<MinMaxAvgTotalCalculator<double> >();
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
TestAntenna3gppModelConf::~TestAntenna3gppModelConf ()
{
}

void
TestAntenna3gppModelConf::DoRun (void)
{
    std::cout<<"\n\n\n"<<m_name<<std::endl;
   // set simulation time and mobility
    Time simTime = MilliSeconds (1000);
    Time udpAppStartTimeDl = MilliSeconds (400);
    Time udpAppStopTimeDl = MilliSeconds (1000);
    uint32_t packetSize = 1000;
    DataRate udpRate = DataRate ("2Mbps");

    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon")); // with antenna height of 10 m
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(false));
    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault ("ns3::MmWave3gppChannel::CellScan", BooleanValue (true));
    Config::SetDefault ("ns3::MmWave3gppChannel::BeamSearchAngleStep", DoubleValue (30.0));

    Config::SetDefault ("ns3::MmWaveEnbPhy::TxPower", DoubleValue(1));

    Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue(28e9));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::Numerology", UintegerValue(3));
    Config::SetDefault ("ns3::MmWavePhyMacCommon::Bandwidth", DoubleValue(20e6));

    // set the number of antenna elements of UE
    Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNumDim1", UintegerValue(sqrt(m_ueNoOfAntennas)));
    Config::SetDefault("ns3::MmWaveUeNetDevice::AntennaNumDim2", UintegerValue(sqrt(m_ueNoOfAntennas)));

    // set the antenna array model type
    Config::SetDefault("ns3::MmWaveHelper::GnbAntennaArrayModelType", TypeIdValue(m_gnbAntennaArrayModelType));
    Config::SetDefault("ns3::MmWaveHelper::UeAntennaArrayModelType", TypeIdValue(m_ueAntennaArrayModelType));

    // set LOS,NLOS condition
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue(m_losCondition));

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

    double gNbHeight = 1.5;
    double ueHeight = 1.5;
    gNbNodes.Create (1);
    ueNodes.Create (1);

    Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator> ();
    Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();


    gNbPositionAlloc->Add(Vector(0,0,gNbHeight));

    if (m_conf == DirectionGnbUe_45)
      {
        uePositionAlloc->Add(Vector(20,20,ueHeight));
      }
    else if (m_conf == DirectionGnbUe_135)
      {
        uePositionAlloc->Add(Vector(-20,20,ueHeight));
      }
    else if (m_conf == DirectionGnbUe_225)
      {
        uePositionAlloc->Add(Vector(-20,-20,ueHeight));
      }
    else if (m_conf == DirectionGnbUe_315)
      {
        uePositionAlloc->Add(Vector(20,-20,ueHeight));
      } else   if (m_conf == DirectionGnbUe_0)
        {
          uePositionAlloc->Add(Vector(20,0,ueHeight));
        }
      else if (m_conf == DirectionGnbUe_90)
        {
          uePositionAlloc->Add(Vector(0,20,ueHeight));
        }
      else if (m_conf == DirectionGnbUe_180)
        {
          uePositionAlloc->Add(Vector(-20, 0,ueHeight));
        }
      else if (m_conf == DirectionGnbUe_270)
        {
          uePositionAlloc->Add(Vector(0,-20,ueHeight));
        }

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator (gNbPositionAlloc);
    mobility.Install (gNbNodes);
    mobility.SetPositionAllocator (uePositionAlloc);
    mobility.Install (ueNodes);
    // install mmWave net devices
    NetDeviceContainer gNbDevs = mmWaveHelper->InstallEnbDevice (gNbNodes);
    NetDeviceContainer ueNetDevs = mmWaveHelper->InstallUeDevice (ueNodes);

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
    mmWaveHelper->AttachToClosestEnb (NetDeviceContainer(ueNetDevs.Get(0)), NetDeviceContainer(gNbDevs.Get(0)));

    // assign IP address to UEs, and install UDP downlink applications
    uint16_t dlPort = 1234;
    ApplicationContainer clientAppsDl;
    ApplicationContainer serverAppsDl;

    Time udpInterval = Time::FromDouble((packetSize*8) / static_cast<double> (udpRate.GetBitRate ()), Time::S);


    UdpServerHelper dlPacketSinkHelper (dlPort);
    serverAppsDl.Add (dlPacketSinkHelper.Install (ueNodes));


    // configure UDP downlink traffic to test OFDMA
    UdpClientHelper dlClient (ueIpIface.GetAddress (0), dlPort);
    dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
    dlClient.SetAttribute("PacketSize", UintegerValue(packetSize));
    dlClient.SetAttribute ("Interval", TimeValue (udpInterval)); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
    clientAppsDl.Add (dlClient.Install (remoteHost));

    // start UDP server and client apps
    serverAppsDl.Start(udpAppStartTimeDl);
    clientAppsDl.Start(udpAppStartTimeDl);

    serverAppsDl.Stop(udpAppStopTimeDl);
    clientAppsDl.Stop(udpAppStopTimeDl);

    Ptr<MmWaveSpectrumPhy > ue1SpectrumPhy = DynamicCast<MmWaveUeNetDevice>
    (ueNetDevs.Get(0))->GetPhy(0)->GetDlSpectrumPhy();

    ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe", MakeBoundCallback(&UETraceReception, this));

    //mmWaveHelper->EnableTraces();
    Simulator::Stop (simTime);
    Simulator::Run ();

    Ptr<UdpServer> serverApp1 = serverAppsDl.Get(0)->GetObject<UdpServer>();
    double throughput1 = (serverApp1->GetReceived() * packetSize * 8)/(udpAppStopTimeDl-udpAppStartTimeDl).GetSeconds();


    std::cout<<"\n UE:  "<<throughput1/1e6<<" Mbps"<<
        "\t Avg.SINR:"<< 10*log10(m_sinrCell1->getMean()) << "\t Avg.MCS:"<<m_mcsCell1->getMean()<<"\t Avg. RB Num:"<<m_rbNumCell1->getMean();


    Simulator::Destroy ();
}


// The TestSuite class names the TestMmWaveSystemTestOfdmaTestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class Antenna3gppModelConfTestSuite : public TestSuite
{
public:
  Antenna3gppModelConfTestSuite ();
};

Antenna3gppModelConfTestSuite::Antenna3gppModelConfTestSuite ()
: TestSuite ("test-antenna-3gpp-model-conf", SYSTEM)
{


  std::list<TestAntenna3gppModelConf::DirectionGnbUeXYAngle> conf = { TestAntenna3gppModelConf::DirectionGnbUe_45,
                                                    TestAntenna3gppModelConf::DirectionGnbUe_135,
                                                    TestAntenna3gppModelConf::DirectionGnbUe_225,
                                                    TestAntenna3gppModelConf::DirectionGnbUe_315,
                                                    TestAntenna3gppModelConf::DirectionGnbUe_0,
                                                    TestAntenna3gppModelConf::DirectionGnbUe_90,
                                                    TestAntenna3gppModelConf::DirectionGnbUe_180,
                                                    TestAntenna3gppModelConf::DirectionGnbUe_270
  };

  std::list<uint8_t> ueNoOfAntennas = {16};

  std::list<std::string> losConditions = {"l"};

  std::list<TypeId> gNbantennaArrayModelTypes = {AntennaArrayModel::GetTypeId(), AntennaArray3gppModel::GetTypeId ()};

  std::list<TypeId> ueAntennaArrayModelTypes = {AntennaArrayModel::GetTypeId(), AntennaArray3gppModel::GetTypeId ()};


  for (const auto & losCondition : losConditions)
    {
      for (const auto & c : conf)
        {
          for (const auto & aaGnb : gNbantennaArrayModelTypes)
            {
              for (const auto & aaUe : ueAntennaArrayModelTypes)
                {
                  for (const auto & n : ueNoOfAntennas)
                    {
                      std::stringstream ss;
                      ss <<" Test: ";

                      if (c == TestAntenna3gppModelConf::DirectionGnbUe_45)
                        {
                          ss << "DirectionGnbUe_45";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_135)
                        {
                          ss << "DirectionGnbUe_135";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_225)
                        {
                          ss << "DirectionGnbUe_225";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_315)
                        {
                          ss << "DirectionGnbUe_315";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_0)
                        {
                          ss << "DirectionGnbUe_0";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_90)
                        {
                          ss << "DirectionGnbUe_90";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_180)
                        {
                          ss << "DirectionGnbUe_180";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_270)
                        {
                          ss << "DirectionGnbUe_270";
                        }

                      ss <<" , channelCondition: "<<losCondition;

                      ss <<" , UE number of antennas:" << (unsigned)n;

                      ss <<" , gNB antenna model type:"<<aaGnb.GetName();

                      ss <<" , UE antenna model type:"<<aaUe.GetName();

                      AddTestCase (new TestAntenna3gppModelConf (ss.str(), c, aaGnb, aaUe, n, losCondition), TestDuration::QUICK);
                    }
                }
            }
        }
    }

}

// Do not forget to allocate an instance of this TestSuite
static Antenna3gppModelConfTestSuite testSuite;

