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

using namespace ns3;

/**
 * \file mmwave-system-test-schedulers.cc
 * \ingroup test
 * \brief System test for the scheduler classes
 *
 * This test case checks if the throughput obtained is as expected for the scheduling logic.
 * The test scenario consists of a scenario in which various UEs are attached to a single gNB.
 * UEs perform UDP full buffer downlink and/or uplink traffic.
 * gNB is configured to have 1 bandwidth part.
 * The traffic is full buffer traffic.
 */
class MmWaveSystemTestScheduling : public TestCase
{
public:
  MmWaveSystemTestScheduling (const std::string & name, uint32_t usersNum, uint32_t beamsNum,
                              uint32_t numerology, double bw1, bool isDownlink,
                              bool isUplink, const std::string & schedulerType);
  virtual ~MmWaveSystemTestScheduling ();

private:
  virtual void DoRun (void);

  uint32_t m_numerology; // the numerology to be used
  double m_bw1; // bandwidth of bandwidth part 1
  bool m_isDownlink; // whether to generate the downlink traffic
  bool m_isUplink; // whether to generate the uplink traffic
  uint32_t m_usersNum; // number of users
  uint32_t m_beamsNum; // currently the test is supposed to work with maximum 4 beams per gNb
  std::string m_schedulerType;
  std::string m_name;

};

// Add some help text to this case to describe what it is intended to test
MmWaveSystemTestScheduling::MmWaveSystemTestScheduling (const std::string & name, uint32_t usersNum,
                                                        uint32_t beamsNum, uint32_t numerology,
                                                        double bw1, bool isDownlnk, bool isUplink,
                                                        const std::string & schedulerType)
: TestCase (name)
{
  m_numerology = numerology;
  m_bw1 = bw1;
  m_isDownlink = isDownlnk;
  m_isUplink = isUplink;
  m_usersNum = usersNum;
  NS_ABORT_MSG_UNLESS (beamsNum <=4, "Test program is designed to support up to 4 beams per gNB" );
  m_beamsNum = beamsNum;
  m_schedulerType = schedulerType;
  m_name=name;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
MmWaveSystemTestScheduling::~MmWaveSystemTestScheduling ()
{
}


void
MmWaveSystemTestScheduling::DoRun (void)
{
    std::cout<<"\n\n\n"<<m_name<<std::endl;
    NS_ABORT_IF(!m_isUplink && !m_isDownlink);

   // set simulation time and mobility
    Time simTime = MilliSeconds (600);
    Time udpAppStartTimeDl = MilliSeconds (500);
    Time udpAppStartTimeUl = MilliSeconds (500);
    Time udpAppStopTimeDl = MilliSeconds (600);
    Time udpAppStopTimeUl = MilliSeconds (600);
    uint16_t gNbNum = 1;
    uint32_t packetSize = 1000;
    DataRate udpRate = DataRate ("400kbps");

    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("l"));
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon")); // with antenna height of 10 m
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(false));
    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault ("ns3::MmWave3gppChannel::CellScan", BooleanValue (true));
    Config::SetDefault ("ns3::MmWave3gppChannel::BeamSearchAngleStep", DoubleValue (10.0));
    Config::SetDefault ("ns3::MmWaveEnbPhy::TxPower", DoubleValue(4));
    Config::SetDefault ("ns3::MmWaveMacSchedulerNs3::StartingMcsDl", UintegerValue (28));
    Config::SetDefault ("ns3::MmWaveMacSchedulerNs3::StartingMcsUl", UintegerValue (28));
    Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));

    // setup the mmWave simulation
    Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();
    mmWaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
    mmWaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));

    Ptr<BandwidthPartsPhyMacConf> bwpConf = CreateObject <BandwidthPartsPhyMacConf> ();
    Ptr<MmWavePhyMacCommon> phyMacCommonBwp = CreateObject<MmWavePhyMacCommon>();
    phyMacCommonBwp->SetCentreFrequency(28e9);
    phyMacCommonBwp->SetBandwidth (m_bw1);
    phyMacCommonBwp->SetNumerology(m_numerology);
    phyMacCommonBwp->SetAttribute ("MacSchedulerType", TypeIdValue (TypeId::LookupByName(m_schedulerType)));

    bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp);
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
    gNbNodes.Create (gNbNum);
    ueNodes.Create (m_usersNum * m_beamsNum * gNbNum);

    Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
    Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();

    double gNbx = 0;
    double gNby = 0;

    for (uint gNb = 0 ; gNb < gNbNum; gNb++)
      {
        apPositionAlloc->Add (Vector (gNbx, gNby, gNbHeight));

        for (uint beam = 1; beam <= m_beamsNum ; beam ++)
          {
            for (uint ueBeam = 0; ueBeam < m_usersNum ; ueBeam ++)
              {
                if (beam == 1)
                  {
                    staPositionAlloc->Add (Vector (gNbx + 1 + 0.1*ueBeam , gNby + 10 + 0.1*ueBeam, ueHeight));
                  }
                else if (beam == 2)
                  {
                    staPositionAlloc->Add (Vector (gNbx + 10 + 0.1*ueBeam , gNby - 1  + 0.1*ueBeam, ueHeight));
                  }
                else if (beam == 3)
                  {
                    staPositionAlloc->Add (Vector (gNbx - 1 + 0.1*ueBeam  , gNby - 10  + 0.1*ueBeam, ueHeight));
                  }
                else if (beam ==4 )
                  {
                    staPositionAlloc->Add (Vector (gNbx - 10  + 0.1*ueBeam , gNby + 1 + 0.1*ueBeam, ueHeight));
                  }
              }

          }

        // position of next gNB and its UE is shiften for 20, 20
        gNbx += 1;
        gNby += 1;
      }
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator (apPositionAlloc);
    mobility.Install (gNbNodes);
    mobility.SetPositionAllocator (staPositionAlloc);
    mobility.Install (ueNodes);
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
    // in this container, interface 0 is the pgw, 1 is the remoteHost
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
    remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));


    // Set the default gateway for the UEs
    for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
      {
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get(j)->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      }

    // attach UEs to the closest eNB
    mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

    // assign IP address to UEs, and install UDP downlink applications
    uint16_t dlPort = 1234;
    uint16_t ulPort = 2000;
    ApplicationContainer clientAppsDl;
    ApplicationContainer serverAppsDl;
    ApplicationContainer clientAppsUl;
    ApplicationContainer serverAppsUl;
    ObjectMapValue objectMapValue;

    Time udpInterval = Time::FromDouble((packetSize*8) / static_cast<double> (udpRate.GetBitRate ()), Time::S);

    if (m_isUplink)
      {
        // configure here UDP traffic
        for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
          {
            UdpServerHelper ulPacketSinkHelper (ulPort);
            serverAppsUl.Add (ulPacketSinkHelper.Install (remoteHost));

            UdpClientHelper ulClient (remoteHostAddr, ulPort);
            ulClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
            ulClient.SetAttribute("PacketSize", UintegerValue(packetSize));
            ulClient.SetAttribute ("Interval", TimeValue (udpInterval)); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
            clientAppsUl.Add (ulClient.Install (ueNodes.Get(j)));
            ulPort++;
          }

        serverAppsUl.Start(udpAppStartTimeUl);
        clientAppsUl.Start(udpAppStartTimeUl);
        serverAppsUl.Stop(udpAppStopTimeUl);
        clientAppsUl.Stop(udpAppStopTimeUl);
      }


    if (m_isDownlink)
      {
        UdpServerHelper dlPacketSinkHelper (dlPort);
        serverAppsDl.Add (dlPacketSinkHelper.Install (ueNodes));

        // configure here UDP traffic
        for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
          {
            UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
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
     }

    //mmWaveHelper->EnableTraces();
    Simulator::Stop (simTime);
    Simulator::Run ();


    double throughputDl = 0;
    double throughputUl = 0;

    if (m_isDownlink)
      {
        Ptr<UdpServer> serverApp1 = serverAppsDl.Get(0)->GetObject<UdpServer>();
        double throuhgput1 = (serverApp1->GetReceived() * packetSize * 8)/(udpAppStopTimeDl-udpAppStartTimeDl).GetSeconds();
        throughputDl = throuhgput1;
        for ( uint32_t i = 1; i < serverAppsDl.GetN(); i++)
          {
            Ptr<UdpServer> serverApp2 = serverAppsDl.Get(i)->GetObject<UdpServer>();
            double throuhgput2 = (serverApp2->GetReceived() * packetSize * 8)/(udpAppStopTimeDl-udpAppStartTimeDl).GetSeconds();
            throughputDl += throuhgput2;
          }
        //std::cout<<"\n Total DL UDP throughput "<<throughputDl/1e6<<" Mbps"<<std::endl;
      }
    if (m_isUplink)
      {
        Ptr<UdpServer> serverApp1 = serverAppsUl.Get(0)->GetObject<UdpServer>();
        double throuhgput1 = (serverApp1->GetReceived() * packetSize * 8)/(udpAppStopTimeUl-udpAppStartTimeUl).GetSeconds();
        throughputUl=throuhgput1;

        for ( uint32_t i = 1; i < serverAppsUl.GetN(); i++)
          {
             Ptr<UdpServer> serverApp2 = serverAppsUl.Get(i)->GetObject<UdpServer>();
             double throuhgput2 = (serverApp2->GetReceived() * packetSize * 8)/(udpAppStopTimeUl-udpAppStartTimeUl).GetSeconds();
             throughputUl += throuhgput2;
          }
        //std::cout<<"\n Total UL UDP throughput "<<throughputUl/1e6<<" Mbps"<<std::endl;
      }

    NS_TEST_ASSERT_MSG_EQ_TOL (throughputDl + throughputUl, udpRate.GetBitRate() * ueNodes.GetN() * ((m_isUplink && m_isDownlink)? 2 : 1), 0.01, "Wrong total DL + UL throughput");
    Simulator::Destroy ();
}

// The TestSuite class names the TestMmWaveSystemTestSchedulingTestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class MmWaveSystemTestSchedulingTestSuite : public TestSuite
{
public:
  MmWaveSystemTestSchedulingTestSuite ();
};

MmWaveSystemTestSchedulingTestSuite::MmWaveSystemTestSchedulingTestSuite ()
: TestSuite ("mmwave-system-test-schedulers", SYSTEM)
{
  enum TxMode
  {
    DL,
    UL,
    DL_UL
  };

  std::list<std::string> subdivision = {"Ofdma", "Tdma"};
  std::list<std::string> scheds       = {"RR", "PF", "MR"};
  std::list<TxMode> mode             = {DL, UL, DL_UL};
  std::list<uint32_t>    ues         = {1, 2, 4, 8};
  std::list<uint32_t>    beams       = {1, 2};
  std::list<uint32_t>    numerologies = {0, 1, 2, 3, 4};

  // Three QUICK test cases
  AddTestCase (new MmWaveSystemTestScheduling ("DL, num 0 Tdma RR 1 2", 1, 2, 0, 20e6, true, false,
                                               "ns3::MmWaveMacSchedulerTdmaRR"), TestCase::QUICK);
  AddTestCase (new MmWaveSystemTestScheduling ("DL_UL, num 0 Tdma RR 1 2", 1, 2, 0, 20e6, true, true,
                                               "ns3::MmWaveMacSchedulerTdmaRR"), TestCase::QUICK);
  AddTestCase (new MmWaveSystemTestScheduling ("UL, num 0 Tdma RR 1 2", 1, 2, 0, 20e6, false, true,
                                               "ns3::MmWaveMacSchedulerTdmaRR"), TestCase::QUICK);

  for (const auto & num : numerologies)
    {
      for (const auto & subType : subdivision)
        {
          for (const auto & sched : scheds)
            {
              for (const auto & modeType : mode)
                {
                  for (const auto & ue : ues)
                    {
                      for (const auto & beam : beams)
                        {
                          std::stringstream ss, schedName;
                          if (modeType == DL)
                            {
                              ss << "DL";
                            }
                          else if (modeType == UL)
                            {
                              ss << "UL";
                            }
                          else
                            {
                              ss << "DL_UL";
                            }
                          ss << ", Num " << num << ", " << subType << " " << sched << ", "
                             << ue << " UE per beam, " << beam << " beam";
                          const bool isDl = modeType == DL || modeType == DL_UL;
                          const bool isUl = modeType == UL || modeType == DL_UL;

                          schedName << "ns3::MmWaveMacScheduler" << subType << sched;

                          AddTestCase (new MmWaveSystemTestScheduling (ss.str(), ue, beam, num,
                                                                       20e6, isDl, isUl,
                                                                       schedName.str()),
                                       TestCase::EXTENSIVE);
                        }
                    }
                }
            }
        }
    }
}

// Do not forget to allocate an instance of this TestSuite
static MmWaveSystemTestSchedulingTestSuite mmwaveTestSuite;


