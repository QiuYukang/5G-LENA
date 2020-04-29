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

#include "ns3/mmwave-helper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include "ns3/log.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/eps-bearer-tag.h"
#include "ns3/mmwave-enb-net-device.h"
#include "ns3/mmwave-ue-net-device.h"
#include "ns3/mmwave-enb-phy.h"
#include "ns3/mmwave-ue-phy.h"
#include "ns3/test.h"
//#include "ns3/component-carrier-gnb.h"
//#include "ns3/component-carrier-mmwave-ue.h"
using namespace ns3;

/**
 * \file mmwave-test-fdm-of-numerologies.cc
 * \ingroup test
 * \brief Test FDM of numerologies
 *
 * This test case checks if the throughput achieved over certain bandwidth part
 * is proportional to the bandwidth of that bandwidth part.
 * The test scenario consists of a scenario in which two UEs are attached to a
 * gNB, and perform UDP full buffer downlink traffic.
 * gNB is configured to have 2 bandwidth parts, which are configured with the
 * same numerology, but can have different bandwidth.
 * Bandwidth part manager is configured to forward first flow over the first
 * bandwidth part, and the second flow over the second bandwidth part.
 * Since the traffic is full buffer traffic, it is expected that when more
 * bandwidth is provided, more throughput will be achieved and vice versa.
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
    double simTime = 0.2; // seconds
    double udpAppStartTime = 0.1; //seconds
    double totalTxPower = 4;
    uint16_t gNbNum = 1;
    uint16_t ueNumPergNb = 2;
    uint32_t packetSize = 1000;

    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));


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

    double totalBandwidth = 0;
    totalBandwidth = m_bw1 + m_bw2;

    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();

    // Put the pointers inside mmWaveHelper
    mmWaveHelper->SetIdealBeamformingHelper (idealBeamformingHelper);
    mmWaveHelper->SetEpcHelper (epcHelper);


    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;

    const uint8_t numCcPerBand = 2;

    CcBwpCreator::SimpleOperationBandConf bandConf1 (28e9, totalBandwidth, numCcPerBand,
                                                     BandwidthPartInfo::UMi_StreetCanyon_LoS);
    OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);

    //Set BW of each BWP
    band1.m_cc[0]->m_bwp[0]->m_channelBandwidth = m_bw1;
    band1.m_cc[1]->m_bwp[0]->m_channelBandwidth = m_bw2;


    mmWaveHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

    mmWaveHelper->InitializeOperationBand (&band1);

    allBwps = CcBwpCreator::GetAllBwps ({band1});


    // gNb routing between Bearer and bandwidh part
    mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (0));
    mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (1));
    // Ue routing between Bearer and bandwidth part
    mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (0));
    mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (1));


    // install mmWave net devices
    NetDeviceContainer enbNetDev = mmWaveHelper->InstallGnbDevice (gNbNodes, allBwps);
    NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNodes, allBwps);

    double x = pow(10, totalTxPower/10);

    mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (m_numerology));
    mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (m_numerology));

    mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw1/totalBandwidth) * x)));
    mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw2/totalBandwidth) * x)));

    mmWaveHelper->GetUePhy (ueNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw1/totalBandwidth) * x)));
    mmWaveHelper->GetUePhy (ueNetDev.Get (0), 1)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw2/totalBandwidth) * x)));


    for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
      {
        DynamicCast<MmWaveEnbNetDevice> (*it)->UpdateConfig ();
      }

    for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
      {
        DynamicCast<MmWaveUeNetDevice> (*it)->UpdateConfig ();
      }

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
    //ObjectMapValue objectMapValue;


    if (m_isUplink)
      {
        // configure here UDP traffic
        for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
          {
            UdpServerHelper ulPacketSinkHelper (ulPort);
            serverAppsUl.Add (ulPacketSinkHelper.Install (remoteHost));

            UdpClientHelper ulClient (remoteHostAddr, ulPort);
            ulClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
            ulClient.SetAttribute("PacketSize", UintegerValue (packetSize));
            ulClient.SetAttribute ("Interval", TimeValue (Seconds (0.00001))); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
            clientAppsUl.Add (ulClient.Install (ueNodes.Get (j)));

            Ptr<EpcTft> tft = Create<EpcTft> ();
            EpcTft::PacketFilter ulpf;
            ulpf.remotePortStart = ulPort;
            ulpf.remotePortEnd = ulPort;
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
            mmWaveHelper->ActivateDedicatedEpsBearer (ueNetDev.Get (j), bearer, tft);

            ulPort++;
          }

        serverAppsUl.Start (Seconds (udpAppStartTime));
        clientAppsUl.Start (Seconds (udpAppStartTime));
        serverAppsUl.Stop (Seconds (simTime));
        clientAppsUl.Stop (Seconds (simTime));
      }


    if (m_isDownlink)
      {
        UdpServerHelper dlPacketSinkHelper (dlPort);
        serverAppsDl.Add (dlPacketSinkHelper.Install (ueNodes));

        // configure here UDP traffic
        for (uint32_t j = 0; j < ueNodes.GetN(); ++j)
          {
            UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
            dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
            dlClient.SetAttribute("PacketSize", UintegerValue (packetSize));
            dlClient.SetAttribute ("Interval", TimeValue (Seconds (0.00001))); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
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
            mmWaveHelper->ActivateDedicatedEpsBearer (ueNetDev.Get (j), bearer, tft);
          }


        // start UDP server and client apps
       serverAppsDl.Start (Seconds (udpAppStartTime));
       clientAppsDl.Start (Seconds (udpAppStartTime));
       serverAppsDl.Stop (Seconds (simTime));
       clientAppsDl.Stop (Seconds (simTime));
     }

    //mmWaveHelper->EnableTraces();
    Simulator::Stop (Seconds (simTime));
    Simulator::Run ();

    if (m_isDownlink)
      {
        Ptr<UdpServer> serverApp1 = serverAppsDl.Get (0)->GetObject<UdpServer> ();
        Ptr<UdpServer> serverApp2 = serverAppsDl.Get (1)->GetObject<UdpServer> ();
        double throuhgput1 = (serverApp1->GetReceived () * (packetSize + 28) * 8) / (simTime - udpAppStartTime);
        double throuhgput2 = (serverApp2->GetReceived () * (packetSize + 28) * 8) / (simTime - udpAppStartTime);

        NS_TEST_ASSERT_MSG_EQ_TOL (throuhgput2,
                                   throuhgput1 * m_bw2 / m_bw1,
                                   std::max (throuhgput1, throuhgput2) * 0.2, "Throughputs are not equal within tolerance");
        NS_TEST_ASSERT_MSG_NE (throuhgput1, 0, "Throughput should be a non-zero value");
        std::cout << "Total DL UDP throughput 1 (bps):" << throuhgput1 / 10e6 << "Mbps" << std::endl;
        std::cout << "Total DL UDP throughput 2 (bps):" << throuhgput2 / 10e6 << "Mbps" << std::endl;
        std::cout << "\n Test value throughput 1: "<< (throuhgput2 * m_bw1 / m_bw2) / 10e6 << "Mbps" << std::endl;
        std::cout << "\n Test value throughput 2: "<< (throuhgput1 * m_bw2 / m_bw1) / 10e6 << "Mbps" << std::endl;
      }
    if (m_isUplink)
      {
        Ptr<UdpServer> serverApp1 = serverAppsUl.Get (0)->GetObject<UdpServer> ();
        Ptr<UdpServer> serverApp2 = serverAppsUl.Get (1)->GetObject<UdpServer> ();
        double throughput1 = (serverApp1->GetReceived () * (packetSize + 28) * 8) / (simTime - udpAppStartTime);
        double throughput2 = (serverApp2->GetReceived () * (packetSize + 28) * 8) / (simTime - udpAppStartTime);

        NS_TEST_ASSERT_MSG_EQ_TOL (throughput2, throughput1 * m_bw2 / m_bw1,
                                   std::max (throughput1, throughput2) * 0.5,
                                   "Throughputs are not equal within tolerance");

        NS_TEST_ASSERT_MSG_NE (throughput1, 0, "Throughput should be a non-zero value");
        std::cout << "Total UL UDP throughput 1 (bps):" << throughput1 / 10e6 << "Mbps" << std::endl;
        std::cout << "Total UL UDP throughput 2 (bps):" << throughput2 / 10e6 << "Mbps" << std::endl;
        std::cout << "\n Test value throughput 1: "<< (throughput2 * m_bw1 / m_bw2) / 10e6 << "Mbps" << std::endl;
        std::cout << "\n Test value throughput 2: "<< (throughput1 * m_bw2 / m_bw1) / 10e6 << "Mbps" << std::endl;
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
//     AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 4, 50e6, 150e6", 4, 50e6, 150e6, false, true), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 4, 100e6, 100e6", 4, 100e6, 100e6, false, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 4, 80e6, 120e6", 4, 80e6, 120e6, false, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 4 60e6, 140e6", 4, 60e6, 140e6, false, true), TestCase::EXTENSIVE);

//     AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 2 50e6 150e6", 2, 50e6, 150e6, false, true), TestCase::QUICK);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 2 100e6 100e6", 2, 100e6, 100e6, false, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 2 80e6 120e6" , 2, 80e6, 120e6, false, true), TestCase::EXTENSIVE);
   AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm ul 2 60e6 140e6", 2, 60e6, 140e6, false, true), TestCase::EXTENSIVE);

   // downlink + uplink cases
   // REMOVED as uplink eats all downlink because is full buffer
   //AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 4, 50e6, 150e6", 4, 50e6, 150e6, true, true), TestCase::QUICK);
   //AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 4, 100e6, 100e6", 4, 100e6, 100e6, true, true), TestCase::EXTENSIVE);
   //AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 4, 80e6, 120e6", 4, 80e6, 120e6, true, true), TestCase::QUICK);
   //AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 4 60e6, 140e6", 4, 60e6, 140e6, true, true), TestCase::EXTENSIVE);

   //AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 2 50e6 150e6", 2, 50e6, 150e6, true, true), TestCase::EXTENSIVE);
   //AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 2 100e6 100e6", 2, 100e6, 100e6, true, true), TestCase::QUICK);
   //AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 2 80e6 120e6" , 2, 80e6, 120e6, true, true), TestCase::EXTENSIVE);
   //AddTestCase (new MmWaveTestFdmOfNumerologiesCase1 ("fdm dl+ul 2 60e6 140e6", 2, 60e6, 140e6, true, true), TestCase::EXTENSIVE);
}

// Do not forget to allocate an instance of this TestSuite
static MmWaveTestFdmOfNumerologiesTestSuite mmwaveTestSuite;


