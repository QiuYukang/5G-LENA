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

using namespace ns3;

/**
 * \file mmwave-test-fdm-of-numerologies.cc
 * \ingroup test
 * \brief Test FDM of numerologies
 *
 * This test case checks if the throughput achieved over certain bandwidth part
 * is proportional to the bandwidth of that bandwidth part.
 * The test scenario consists of a scenario in which two UEs are attached to a gNB,
 * and perform UDP full buffer downlink traffic.
 * gNB is configured to have 2 bandwidth parts, which are configured with the same numerology,
 * but can have different bandwidth.
 * Bandwidth part manager is configured to forward first flow over the first bandwidth part,
 * and the second flow over the second bandwidth part.
 * Since the traffic is full buffer traffic, it is expected that more bandwidth is provided,
 * more throughput will be achieved and vice versa.
 */
class MmWaveTestFdmOfNumerologiesCase1 : public TestCase
{
public:
  MmWaveTestFdmOfNumerologiesCase1 (std::string name, uint32_t numerology, double bw1, double bw2, bool isDownlink, bool isUplink);
  virtual ~MmWaveTestFdmOfNumerologiesCase1 ();

private:
  virtual void DoRun (void);

  uint32_t m_numerology; // the numerology to be used
  double m_bw1; // bandwidth of bandwidth part 1
  double m_bw2; // bandwidth of bandwidth part 2
  bool m_isDownlink; // whether to generate the downlink traffic
  bool m_isUplink; // whether to generate the uplink traffic
};

// Add some help text to this case to describe what it is intended to test
MmWaveTestFdmOfNumerologiesCase1::MmWaveTestFdmOfNumerologiesCase1 (std::string name, uint32_t numerology, double bw1, double bw2, bool isDownlnk, bool isUplink)
: TestCase (name)
{
  m_numerology = numerology;
  m_bw1 = bw1;
  m_bw2 = bw2;
  m_isDownlink = isDownlnk;
  m_isUplink = isUplink;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
MmWaveTestFdmOfNumerologiesCase1::~MmWaveTestFdmOfNumerologiesCase1 ()
{
}


void
MmWaveTestFdmOfNumerologiesCase1::DoRun (void)
{
   // set simulation time and mobility
    double simTime = 0.5; // seconds
    double udpAppStartTime = 0.4; //seconds
    double totalTxPower = 4;
    uint16_t gNbNum = 1;
    uint16_t ueNumPergNb = 2;
    uint32_t packetSize = 1000;

    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("l"));
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon")); // with antenna height of 10 m
    Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(false));
    Config::SetDefault ("ns3::MmWaveHelper::NumberOfComponentCarriers", UintegerValue (2));
    Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::NGBR_LOW_LAT_EMBB", UintegerValue (0));
    Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::GBR_CONV_VOICE", UintegerValue (1));
    Config::SetDefault ("ns3::MmWaveHelper::EnbComponentCarrierManager", StringValue ("ns3::BwpManagerGnb"));
    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));

    // setup the mmWave simulation
    Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();
    mmWaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
    mmWaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));

    Ptr<BandwidthPartsPhyMacConf> bwpConf = CreateObject <BandwidthPartsPhyMacConf> ();
    Ptr<MmWavePhyMacCommon> phyMacCommonBwp1 = CreateObject<MmWavePhyMacCommon>();
    phyMacCommonBwp1->SetCentreFrequency(28e9);
    phyMacCommonBwp1->SetBandwidth (m_bw1);
    phyMacCommonBwp1->SetNumerology(m_numerology);
    phyMacCommonBwp1->SetCcId(0);
    bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp1);
    Ptr<MmWavePhyMacCommon> phyMacCommonBwp2 = CreateObject<MmWavePhyMacCommon>();
    phyMacCommonBwp2->SetCentreFrequency(28.2e9);
    phyMacCommonBwp2->SetBandwidth (m_bw2);
    phyMacCommonBwp2->SetNumerology(m_numerology);
    phyMacCommonBwp2->SetCcId(1);
    bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp2);
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
    ueNodes.Create (ueNumPergNb * gNbNum);

    Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
    Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();
    apPositionAlloc->Add (Vector (0.0, 20, gNbHeight));
    staPositionAlloc->Add (Vector (1, 1, ueHeight));
    staPositionAlloc->Add (Vector (-1, 1, ueHeight));
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

    double x = pow(10, totalTxPower/10);
    double totalBandwidth = 0;

    totalBandwidth = m_bw1 + m_bw2;

    Ptr<MmWaveEnbNetDevice> netDevice = DynamicCast<MmWaveEnbNetDevice>(enbNetDev.Get(0));
    netDevice->GetAttribute("ComponentCarrierMap", objectMapValue);
    for (uint32_t i = 0; i < objectMapValue.GetN(); i++)
      {
        Ptr<ComponentCarrierGnb> bandwidthPart = DynamicCast<ComponentCarrierGnb>(objectMapValue.Get(i));
        if (i==0)
          {
            bandwidthPart->GetPhy()->SetTxPower(10*log10((m_bw1/totalBandwidth)*x));
          }
        else if (i==1)
          {
            bandwidthPart->GetPhy()->SetTxPower(10*log10((m_bw2/totalBandwidth)*x));
          }
      }

    // set tx power of UE devices
    for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
      {
        Ptr<MmWaveUeNetDevice> netDevice = DynamicCast<MmWaveUeNetDevice>(ueNetDev.Get(j));
        netDevice->GetAttribute("ComponentCarrierMapUe", objectMapValue);
        for (uint32_t i = 0; i < objectMapValue.GetN(); i++)
          {
            Ptr<ComponentCarrierMmWaveUe> bandwidthPart = DynamicCast<ComponentCarrierMmWaveUe>(objectMapValue.Get(i));
            if (i==0)
              {
                bandwidthPart->GetPhy()->SetTxPower(10*log10((m_bw1/totalBandwidth)*x));
              }
            else if (i==1)
              {
                bandwidthPart->GetPhy()->SetTxPower(10*log10((m_bw2/totalBandwidth)*x));
              }
          }
      }


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
            ulClient.SetAttribute ("Interval", TimeValue (Seconds(0.00001))); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
            clientAppsUl.Add (ulClient.Install (ueNodes.Get(j)));

            Ptr<EpcTft> tft = Create<EpcTft> ();
            EpcTft::PacketFilter ulpf;
            ulpf.localPortStart = ulPort;
            ulpf.localPortEnd = ulPort;
            tft->Add (ulpf);

            enum EpsBearer::Qci q;

            if (j == 0)
              {
                q = EpsBearer::NGBR_LOW_LAT_EMBB;
              }
            else
              {
                q = EpsBearer::GBR_CONV_VOICE;
              }

            EpsBearer bearer (q);
            mmWaveHelper->ActivateDedicatedEpsBearer(ueNetDev.Get(j), bearer, tft);

            ulPort++;
          }

        serverAppsUl.Start(Seconds(udpAppStartTime));
        clientAppsUl.Start(Seconds(udpAppStartTime));
        serverAppsUl.Stop(Seconds(simTime));
        clientAppsUl.Stop(Seconds(simTime));
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
            dlClient.SetAttribute ("Interval", TimeValue (Seconds(0.00001))); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
            clientAppsDl.Add (dlClient.Install (remoteHost));

            Ptr<EpcTft> tft = Create<EpcTft> ();
            EpcTft::PacketFilter dlpf;
            dlpf.localPortStart = dlPort;
            dlpf.localPortEnd = dlPort;
            tft->Add (dlpf);

            enum EpsBearer::Qci q;

            if (j == 0)
              {
                q = EpsBearer::NGBR_LOW_LAT_EMBB;
              }
            else
              {
                q = EpsBearer::GBR_CONV_VOICE;
              }

            EpsBearer bearer (q);
            mmWaveHelper->ActivateDedicatedEpsBearer(ueNetDev.Get(j), bearer, tft);
          }


        // start UDP server and client apps
       serverAppsDl.Start(Seconds(udpAppStartTime));
       clientAppsDl.Start(Seconds(udpAppStartTime));
       serverAppsDl.Stop(Seconds(simTime));
       clientAppsDl.Stop(Seconds(simTime));
     }

    //mmWaveHelper->EnableTraces();
    Simulator::Stop (Seconds (simTime));
    Simulator::Run ();

    if (m_isDownlink)
      {
        Ptr<UdpServer> serverApp1 = serverAppsDl.Get(0)->GetObject<UdpServer>();
        Ptr<UdpServer> serverApp2 = serverAppsDl.Get(1)->GetObject<UdpServer>();
        double throuhgput1 = (serverApp1->GetReceived() * (packetSize+28)*8)/(simTime-udpAppStartTime);
        double throuhgput2 = (serverApp2->GetReceived() * (packetSize+28)*8)/(simTime-udpAppStartTime);
        NS_TEST_ASSERT_MSG_EQ_TOL (throuhgput2, throuhgput1 * m_bw2/m_bw1, std::max(throuhgput1, throuhgput2) * 0.1, "Throughputs are not equal within tolerance");
        NS_TEST_ASSERT_MSG_NE(throuhgput1, 0, "Throughput should be a non-zero value");
        //std::cout<<"\n Total DL UDP throughput 1 (bps):"<<throuhgput1/10e6<<"Mbps"<<std::endl;
//        std::cout<<"\n Total DL UDP throughput 2 (bps):"<<throuhgput2/10e6<<"Mbps"<<std::endl;
  //      std::cout<<"\n Test value:"<<(throuhgput1 * m_bw2/m_bw1)/10e6<<"Mbps"<<std::endl;
      }
    if (m_isUplink)
      {
        Ptr<UdpServer> serverApp1 = serverAppsUl.Get(0)->GetObject<UdpServer>();
        Ptr<UdpServer> serverApp2 = serverAppsUl.Get(1)->GetObject<UdpServer>();
        double throughput1 = (serverApp1->GetReceived() * (packetSize+28)*8)/(simTime-udpAppStartTime);
        double throughput2 = (serverApp2->GetReceived() * (packetSize+28)*8)/(simTime-udpAppStartTime);
        NS_LOG_UNCOND ("Throughput1: " << throughput1 <<
                       " Throughput2: " << throughput2 << " bw2 " << m_bw2 <<
                       " bw1: " << m_bw1);
        NS_TEST_ASSERT_MSG_EQ_TOL (throughput2, throughput1 * m_bw2/m_bw1,
                                   std::max(throughput1, throughput2) * 0.1,
                                   "Throughputs are not equal within tolerance");

        NS_TEST_ASSERT_MSG_NE(throughput1, 0, "Throughput should be a non-zero value");
    //    std::cout<<"\n Total UL UDP throughput 1 (bps):"<<throuhgput1/10e6<<"Mbps"<<std::endl;
      //  std::cout<<"\n Total UL UDP throughput 2 (bps):"<<throuhgput2/10e6<<"Mbps"<<std::endl;
        //std::cout<<"\n Test value:"<<(throuhgput1 * m_bw2/m_bw1)/10e6<<"Mbps"<<std::endl;

      }

    Simulator::Destroy ();
}

// The TestSuite class names the TestMmWaveTestFdmOfNumerologiesTestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class MmWaveTestFdmOfNumerologiesTestSuite : public TestSuite
{
public:
  MmWaveTestFdmOfNumerologiesTestSuite ();
};

MmWaveTestFdmOfNumerologiesTestSuite::MmWaveTestFdmOfNumerologiesTestSuite ()
: TestSuite ("mmwave-test-fdm-of-numerologies", SYSTEM)
{
  // downlink test cases
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl 4, 50e6, 150e6", 4, 50e6, 150e6, true, false), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl 4, 100e6, 100e6", 4, 100e6, 100e6, true, false), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl 4, 80e6, 120e6", 4, 80e6, 120e6, true, false), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl 4 60e6, 140e6", 4, 60e6, 140e6, true, false), TestCase::EXTENSIVE);

   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl 2 50e6 150e6", 2, 50e6, 150e6, true, false), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl 2 100e6 100e6", 2, 100e6, 100e6, true, false), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl 2 80e6 120e6" , 2, 80e6, 120e6, true, false), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl 2 60e6 140e6", 2, 60e6, 140e6, true, false), TestCase::EXTENSIVE);


   // uplink test cases
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 4, 50e6, 150e6", 4, 50e6, 150e6, false, true), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 4, 100e6, 100e6", 4, 100e6, 100e6, false, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 4, 80e6, 120e6", 4, 80e6, 120e6, false, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 4 60e6, 140e6", 4, 60e6, 140e6, false, true), TestCase::EXTENSIVE);

   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 2 50e6 150e6", 2, 50e6, 150e6, false, true), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 2 100e6 100e6", 2, 100e6, 100e6, false, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 2 80e6 120e6" , 2, 80e6, 120e6, false, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 2 60e6 140e6", 2, 60e6, 140e6, false, true), TestCase::EXTENSIVE);

   // downlink + uplink cases
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 4, 50e6, 150e6", 4, 50e6, 150e6, true, true), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 4, 100e6, 100e6", 4, 100e6, 100e6, true, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 4, 80e6, 120e6", 4, 80e6, 120e6, true, true), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 4 60e6, 140e6", 4, 60e6, 140e6, true, true), TestCase::EXTENSIVE);

   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 2 50e6 150e6", 2, 50e6, 150e6, true, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 2 100e6 100e6", 2, 100e6, 100e6, true, true), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 2 80e6 120e6" , 2, 80e6, 120e6, true, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 2 60e6 140e6", 2, 60e6, 140e6, true, true), TestCase::EXTENSIVE);
}

// Do not forget to allocate an instance of this TestSuite
static MmWaveTestFdmOfNumerologiesTestSuite mmwaveTestSuite;


