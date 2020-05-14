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
#include "ns3/test.h"

using namespace ns3;

/**
 * \file mmwave-system-test-schedulers.cc
 * \ingroup test
 * \brief System test for the scheduler classes
 * This test case checks if the throughput obtained per UE is as expected for
 * the specified scheduling logic.
 * The test scenario consists of a scenario in which various UEs are attached to a single gNB.
 * UEs perform UDP full buffer downlink and/or uplink traffic.
 * gNB is configured to have 1 bandwidth part.
 * UEs can belong to the same or different beams.
 * This examples uses beam search beamforming method.
 * The traffic is full buffer traffic.
 */
class MmWaveSystemTestScheduling : public TestCase
{
public:
  /**
   * \brief MmWaveSystemTestScheduling is a test constructor which is used to initialise the test parameters.
   * @param name A unique test configuration name
   * @param usersPerBeamNum How many users will be installed per beam
   * @param beamsNum Into how many beams of gNB will be distributed UEs attached to it. The maximum for this test case is 4.
   * @param numerology The numerology to be used in the simulation
   * @param bw1 The system bandwidth
   * @param isDownlnk Is the downlink traffic going to be present in the test case
   * @param isUplink Is the uplink traffic going to be present in the test case
   * @param schedulerType Which scheduler is going to be used in the test case Ofdma/Tdma" and the scheduling logic RR, PF, of MR
   */
  MmWaveSystemTestScheduling (const std::string & name, uint32_t usersPerBeamNum, uint32_t beamsNum,
                              uint32_t numerology, double bw1, bool isDownlink,
                              bool isUplink, const std::string & schedulerType);
  virtual ~MmWaveSystemTestScheduling ();

private:
  virtual void DoRun (void);

  uint32_t m_numerology; // the numerology to be used
  double m_bw1; // bandwidth of bandwidth part 1
  bool m_isDownlink; // whether to generate the downlink traffic
  bool m_isUplink; // whether to generate the uplink traffic
  uint32_t m_usersPerBeamNum; // number of users
  uint32_t m_beamsNum; // currently the test is supposed to work with maximum 4 beams per gNb
  std::string m_schedulerType;
  std::string m_name;

};

/**
 * MmWaveSystemTestScheduling is a test constructor which is used to initialise the test parameters.  
 * @param name A unique test configuration name
 * @param usersPerBeamNum How many users will be installed per beam
 * @param beamsNum Into how many beams of gNB will be distributed UEs attached to it. The maximum for this test case is 4. 
 * @param numerology The numerology to be used in the simulation
 * @param bw1 The system bandwidth
 * @param isDownlnk Is the downlink traffic going to be present in the test case
 * @param isUplink Is the uplink traffic going to be present in the test case
 * @param schedulerType Which scheduler is going to be used in the test case Ofdma/Tdma" and the scheduling logic RR, PF, of MR
 */
MmWaveSystemTestScheduling::MmWaveSystemTestScheduling (const std::string & name, uint32_t usersPerBeamNum,
                                                        uint32_t beamsNum, uint32_t numerology,
                                                        double bw1, bool isDownlnk, bool isUplink,
                                                        const std::string & schedulerType)
: TestCase (name)
{
  m_numerology = numerology;
  m_bw1 = bw1;
  m_isDownlink = isDownlnk;
  m_isUplink = isUplink;
  m_usersPerBeamNum = usersPerBeamNum;
  NS_ABORT_MSG_UNLESS (beamsNum <=4, "Test program is designed to support up to 4 beams per gNB" );
  m_beamsNum = beamsNum;
  m_schedulerType = schedulerType;
  m_name = name;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
MmWaveSystemTestScheduling::~MmWaveSystemTestScheduling ()
{
}


void
MmWaveSystemTestScheduling::DoRun (void)
{
    NS_ABORT_IF(!m_isUplink && !m_isDownlink);

   // set simulation time and mobility
    Time simTime = MilliSeconds (1500);
    Time udpAppStartTimeDl = MilliSeconds (500);
    Time udpAppStartTimeUl = MilliSeconds (500);
    Time udpAppStopTimeDl = MilliSeconds (1500); // Let's give 1s to end the tx
    Time udpAppStopTimeUl = MilliSeconds (1500); // Let's give 1 to end the tx
    uint16_t gNbNum = 1;
    uint32_t packetSize = 100;
    uint32_t maxPackets = 400;
    DataRate udpRate = DataRate ("320kbps"); // 400 packets of 800 bits

    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));


    // create base stations and mobile terminals
    NodeContainer gNbNodes;
    NodeContainer ueNodes;
    MobilityHelper mobility;

    double gNbHeight = 10;
    double ueHeight = 1.5;
    gNbNodes.Create (gNbNum);
    ueNodes.Create (m_usersPerBeamNum * m_beamsNum * gNbNum);

    Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
    Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();

    double gNbx = 0;
    double gNby = 0;

    for (uint gNb = 0 ; gNb < gNbNum; gNb++)
      {
        apPositionAlloc->Add (Vector (gNbx, gNby, gNbHeight));

        for (uint beam = 1; beam <= m_beamsNum ; beam ++)
          {
            for (uint uePerBeamIndex = 0; uePerBeamIndex < m_usersPerBeamNum ; uePerBeamIndex ++)
              {
                if (beam == 1)
                  {
                    staPositionAlloc->Add (Vector (gNbx + 1 + 0.1*uePerBeamIndex , gNby + 10 + 0.1*uePerBeamIndex, ueHeight));
                  }
                else if (beam == 2)
                  {
                    staPositionAlloc->Add (Vector (gNbx + 10 + 0.1*uePerBeamIndex , gNby - 1  + 0.1*uePerBeamIndex, ueHeight));
                  }
                else if (beam == 3)
                  {
                    staPositionAlloc->Add (Vector (gNbx - 1 + 0.1*uePerBeamIndex  , gNby - 10  + 0.1*uePerBeamIndex, ueHeight));
                  }
                else if (beam ==4 )
                  {
                    staPositionAlloc->Add (Vector (gNbx - 10  + 0.1*uePerBeamIndex , gNby + 1 + 0.1*uePerBeamIndex, ueHeight));
                  }
              }

          }

        // position of next gNB and its UE is shiftened for 20, 20
        gNbx += 1;
        gNby += 1;
      }
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator (apPositionAlloc);
    mobility.Install (gNbNodes);
    mobility.SetPositionAllocator (staPositionAlloc);
    mobility.Install (ueNodes);


    // setup the mmWave simulation
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();

    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
    idealBeamformingHelper->SetIdealBeamFormingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (10.0));

    Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();
    mmWaveHelper->SetIdealBeamformingHelper (idealBeamformingHelper);

    // set the number of antenna elements of UE
    mmWaveHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
    mmWaveHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
    mmWaveHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (true));

    // UE transmit power
    mmWaveHelper->SetUePhyAttribute ("TxPower", DoubleValue (20.0));

    // set the number of antenna elements of gNbs
    mmWaveHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
    mmWaveHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
    mmWaveHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (false));

    // gNB transmit power
    mmWaveHelper->SetGnbPhyAttribute("TxPower", DoubleValue (44.0));

    // gNB numerology
    mmWaveHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (m_numerology));

    // Set the scheduler type
    mmWaveHelper->SetSchedulerTypeId (TypeId::LookupByName (m_schedulerType));
    Config::SetDefault("ns3::NrAmc::ErrorModelType", TypeIdValue (TypeId::LookupByName ("ns3::NrEesmCcT1")));
    mmWaveHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue(true));
    mmWaveHelper->SetSchedulerAttribute ("FixedMcsUl", BooleanValue(true));
    mmWaveHelper->SetSchedulerAttribute ("StartingMcsDl", UintegerValue (28));
    mmWaveHelper->SetSchedulerAttribute ("StartingMcsUl", UintegerValue (28));


    mmWaveHelper->SetEpcHelper (epcHelper);

    /*
    * Spectrum division. We create two operational bands, each of them containing
    * one component carrier, and each CC containing a single bandwidth part
    * centered at the frequency specified by the input parameters.
    * Each spectrum part length is, as well, specified by the input parameters.
    * Both operational bands will use the StreetCanyon channel modeling.
    */
    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    double centralFrequency = 28e9;
    double bandwidth = m_bw1;
    const uint8_t numCcPerBand = 1;
    BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::UMi_StreetCanyon_LoS;
    CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency, bandwidth, numCcPerBand, scenario);

    // By using the configuration created, it is time to make the operation bands
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

    Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (0)));

    // Shadowing
    mmWaveHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

    /*
     * Initialize channel and pathloss, plus other things inside band1. If needed,
     * the band configuration can be done manually, but we leave it for more
     * sophisticated examples. For the moment, this method will take care
     * of all the spectrum initialization needs.
     */
    mmWaveHelper->InitializeOperationBand (&band);
    allBwps = CcBwpCreator::GetAllBwps ({band});


    uint32_t bwpIdForLowLat = 0;
    // gNb routing between Bearer and bandwidh part
    mmWaveHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
    // UE routing between Bearer and bandwidh part
    mmWaveHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));


    // install mmWave net devices
    NetDeviceContainer gNbNetDevs = mmWaveHelper->InstallGnbDevice (gNbNodes, allBwps);
    NetDeviceContainer ueNetDevs = mmWaveHelper->InstallUeDevice (ueNodes, allBwps);


    for (auto it = gNbNetDevs.Begin (); it != gNbNetDevs.End (); ++it)
      {
        DynamicCast<MmWaveEnbNetDevice> (*it)->UpdateConfig ();
      }

    for (auto it = ueNetDevs.Begin (); it != ueNetDevs.End (); ++it)
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
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDevs));


    // Set the default gateway for the UEs
    for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
      {
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get(j)->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      }

    // attach UEs to the closest eNB
    mmWaveHelper->AttachToClosestEnb (ueNetDevs, gNbNetDevs);


    // assign IP address to UEs, and install UDP downlink applications
    uint16_t dlPort = 1234;
    uint16_t ulPort = 2000;
    ApplicationContainer clientAppsDl;
    ApplicationContainer serverAppsDl;
    ApplicationContainer clientAppsUl;
    ApplicationContainer serverAppsUl;
//    ObjectMapValue objectMapValue;

    Time udpInterval = NanoSeconds (1);

    if (m_isUplink)
      {
        UdpServerHelper ulPacketSinkHelper (ulPort);
        serverAppsUl.Add (ulPacketSinkHelper.Install (remoteHost));

        // configure here UDP traffic flows
        for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
          {

            UdpClientHelper ulClient (remoteHostAddr, ulPort);
            ulClient.SetAttribute ("MaxPackets", UintegerValue(maxPackets));
            ulClient.SetAttribute("PacketSize", UintegerValue(packetSize));
            ulClient.SetAttribute ("Interval", TimeValue (udpInterval)); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
            clientAppsUl.Add (ulClient.Install (ueNodes.Get(j)));

            Ptr<EpcTft> tft = Create<EpcTft> ();
            EpcTft::PacketFilter ulpf;
            ulpf.remotePortStart = ulPort;
            ulpf.remotePortEnd = ulPort;
            ulpf.direction = EpcTft::UPLINK;
            tft->Add (ulpf);

            EpsBearer bearer (EpsBearer::NGBR_LOW_LAT_EMBB);
            mmWaveHelper->ActivateDedicatedEpsBearer (ueNetDevs.Get(j), bearer, tft);
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

        // configure here UDP traffic flows
        for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
          {
            UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
            dlClient.SetAttribute ("MaxPackets", UintegerValue(maxPackets));
            dlClient.SetAttribute("PacketSize", UintegerValue(packetSize));
            dlClient.SetAttribute ("Interval", TimeValue (udpInterval)); // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
            clientAppsDl.Add (dlClient.Install (remoteHost));

            Ptr<EpcTft> tft = Create<EpcTft> ();
            EpcTft::PacketFilter dlpf;
            dlpf.localPortStart = dlPort;
            dlpf.localPortEnd = dlPort;
            dlpf.direction = EpcTft::DOWNLINK;
            tft->Add (dlpf);

            EpsBearer bearer (EpsBearer::NGBR_LOW_LAT_EMBB);
            mmWaveHelper->ActivateDedicatedEpsBearer (ueNetDevs.Get(j), bearer, tft);
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


    double dataRecvDl = 0;
    double dataRecvUl = 0;

    if (m_isDownlink)
      {
        for ( uint32_t i = 0; i < serverAppsDl.GetN (); i++)
          {
            Ptr<UdpServer> serverApp = serverAppsDl.Get (i)->GetObject<UdpServer> ();
            double data = (serverApp->GetReceived () * packetSize * 8);
            dataRecvDl += data;
          }
      }
    if (m_isUplink)
      {
        for ( uint32_t i = 0; i < serverAppsUl.GetN (); i++)
          {
             Ptr<UdpServer> serverApp = serverAppsUl.Get (i)->GetObject<UdpServer>();
             double data = (serverApp->GetReceived () * packetSize * 8);
             dataRecvUl += data;
          }
      }


    NS_TEST_ASSERT_MSG_EQ_TOL (dataRecvDl + dataRecvUl, udpRate.GetBitRate () * ueNodes.GetN () * ((m_isUplink && m_isDownlink)? 2 : 1), 0.01, "Wrong total DL + UL throughput");

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

  std::list<std::string> subdivision     = {"Ofdma", "Tdma"};
  std::list<std::string> scheds          = {"RR", "PF", "MR"};
  std::list<TxMode>      mode            = {DL, UL, DL_UL};
  std::list<uint32_t>    uesPerBeamList  = {1, 2, 4, 8};
  std::list<uint32_t>    beams           = {1, 2};
  std::list<uint32_t>    numerologies    = {0, 1, 2, 3, 4};


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
                  for (const auto & uesPerBeam : uesPerBeamList)
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
                             << uesPerBeam << " UE per beam, " << beam << " beam";
                          const bool isDl = modeType == DL || modeType == DL_UL;
                          const bool isUl = modeType == UL || modeType == DL_UL;

                          schedName << "ns3::MmWaveMacScheduler" << subType << sched;

                          AddTestCase (new MmWaveSystemTestScheduling (ss.str(), uesPerBeam, beam, num,
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


