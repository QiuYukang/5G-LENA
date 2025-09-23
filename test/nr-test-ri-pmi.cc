// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/basic-data-calculators.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/nr-amc.h"
#include "ns3/nr-channel-helper.h"
#include "ns3/nr-eps-bearer.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/nr-ue-net-device.h"
#include "ns3/nr-ue-phy.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/test.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"

#include <cmath>
#include <iostream>
#include <map>
#include <string>

/**
 * @file nr-test-ri-pmi.cc
 * @ingroup test
 *
 * @brief System-testing for Rank Indicator and
 * Precoding Matrix Indicator (RI/PMI) selection.
 */
namespace ns3
{
struct CqiFeedbackTraceStats
{
    Ptr<MinMaxAvgTotalCalculator<uint8_t>> m_ri;
    Ptr<MinMaxAvgTotalCalculator<uint8_t>> m_mcs;

    CqiFeedbackTraceStats()
    {
        m_ri = CreateObject<MinMaxAvgTotalCalculator<uint8_t>>();
        m_mcs = CreateObject<MinMaxAvgTotalCalculator<uint8_t>>();
    }

    CqiFeedbackTraceStats(uint8_t rank, uint8_t mcs)
    {
        m_ri = CreateObject<MinMaxAvgTotalCalculator<uint8_t>>();
        m_ri->Update(rank);
        m_mcs = CreateObject<MinMaxAvgTotalCalculator<uint8_t>>();
        m_mcs->Update(mcs);
    }
};

void
CqiFeedbackTracedCallback(std::map<uint16_t, CqiFeedbackTraceStats>* stats,
                          uint16_t rnti,
                          [[maybe_unused]] uint8_t cqi,
                          uint8_t mcs,
                          uint8_t rank)
{
    auto it = stats->find(rnti);
    if (it != stats->end())
    {
        it->second.m_ri->Update(rank);
        it->second.m_mcs->Update(mcs);
    }
    else
    {
        (*stats)[rnti] = CqiFeedbackTraceStats(rank, mcs);
    }
}

std::string
GetRiPmiTestCaseName(double distanceGnbUe,
                     std::string riSelectionTechnique,
                     double riThreshold,
                     std::string pmiSelectionTechnique)
{
    std::stringstream ss;
    ss << distanceGnbUe << "-" << riSelectionTechnique << "-" << riThreshold << "-"
       << pmiSelectionTechnique;
    return ss.str();
}

/* Beginning of TestCttcNrMimoDemoTestCase */
class RiPmiTestCase : public TestCase
{
  public:
    RiPmiTestCase(double distanceGnbUe,
                  std::string riSelectionTechnique,
                  double riThreshold,
                  std::string pmiSelectionTechnique,
                  double throughput,
                  double latency,
                  double meanRank,
                  double meanMcs)
        : TestCase(GetRiPmiTestCaseName(distanceGnbUe,
                                        riSelectionTechnique,
                                        riThreshold,
                                        pmiSelectionTechnique)),
          m_distanceGnbUe(distanceGnbUe),
          m_riSelectionTechnique(riSelectionTechnique),
          m_riThreshold(riThreshold),
          m_pmiSelectionTechnique(pmiSelectionTechnique),
          m_targetThroughput(throughput),
          m_targetLatency(latency),
          m_targetMeanRank(meanRank),
          m_targetMeanMcs(meanMcs)
    {
    }

  private:
    void DoRun() override;
    double m_distanceGnbUe;
    std::string m_riSelectionTechnique;
    double m_riThreshold;
    std::string m_pmiSelectionTechnique;
    double m_targetThroughput;
    double m_targetLatency;
    double m_targetMeanRank;
    double m_targetMeanMcs;
};

void
RiPmiTestCase::DoRun()
{
    NrHelper::AntennaParams apUe;
    NrHelper::AntennaParams apGnb;
    apUe.antennaElem = "ns3::ThreeGppAntennaModel";
    apUe.nAntCols = 2;
    apUe.nAntRows = 2;
    apUe.nHorizPorts = 2;
    apUe.nVertPorts = 1;
    apUe.isDualPolarized = true;
    apGnb.antennaElem = "ns3::ThreeGppAntennaModel";
    apGnb.nAntCols = 4;
    apGnb.nAntRows = 2;
    apGnb.nHorizPorts = 2;
    apGnb.nVertPorts = 1;
    apGnb.isDualPolarized = true;
    double downtiltAngleGnb = 10;

    // The polarization slant angle in degrees in case of x-polarized
    double polSlantAngleGnb = 0.0;
    double polSlantAngleUe = 0.0;
    // The bearing angles in degrees
    double bearingAngleGnb = 0.0;
    double bearingAngleUe = 180.0;

    // Traffic parameters to fully saturate channel
    uint32_t udpPacketSize = 1000;
    Time packetInterval = NanoSeconds(40000);
    Time udpAppStartTime = MilliSeconds(400);

    // Other simulation scenario parameters
    Time simTime = MilliSeconds(1400);
    uint16_t numerology = 0;
    double centralFrequency = 3.5e9;
    double bandwidth = 10e6;
    double txPowerGnb = 23; // dBm
    double txPowerUe = 23;  // dBm
    uint16_t updatePeriodMs = 100;
    std::string errorModel = "ns3::NrEesmIrT2";
    std::string scheduler = "ns3::NrMacSchedulerTdmaRR";
    std::string beamformingMethod = "ns3::DirectPathBeamforming";

    uint32_t wbPmiUpdateIntervalMs = 10; // Wideband PMI update interval in ms
    uint32_t sbPmiUpdateIntervalMs = 2;  // Subband PMI update interval in ms

    NrHelper::MimoPmiParams mimoPmiParams;
    mimoPmiParams.rankLimit = 4;
    mimoPmiParams.subbandSize = 4;
    mimoPmiParams.fullSearchCb = "ns3::NrCbTypeOneSp";
    mimoPmiParams.pmSearchMethod = m_pmiSelectionTechnique;
    if (!m_riSelectionTechnique.empty())
    {
        mimoPmiParams.rankTechnique = m_riSelectionTechnique;
        mimoPmiParams.rankThreshold = m_riThreshold;
    }

    // convert angle values into radians
    apUe.bearingAngle = bearingAngleUe * (M_PI / 180);
    apUe.polSlantAngle = polSlantAngleUe * (M_PI / 180);
    apGnb.bearingAngle = bearingAngleGnb * (M_PI / 180);
    apGnb.polSlantAngle = polSlantAngleGnb * (M_PI / 180);

    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod",
                       TimeValue(MilliSeconds(updatePeriodMs)));

    uint16_t pairsToCreate = 1;
    NodeContainer gnbContainer;
    gnbContainer.Create(pairsToCreate);
    NodeContainer ueContainer;
    ueContainer.Create(pairsToCreate);

    /**
     * We configure the mobility model to ConstantPositionMobilityModel.
     * The default topology is the following:
     *
     * gNB1.................UE1..........
     *(0.0, 0.0, 25.0)  (d, 0.0, 1.5)
     * bearingAngle=0   bearingAngle=180
     */
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 25.0));
    positionAlloc->Add(Vector(m_distanceGnbUe, 0.0, 1.5));
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(gnbContainer.Get(0));
    mobility.Install(ueContainer.Get(0));

    /**
     * Create the NR helpers that will be used to create and setup NR devices, spectrum, ...
     */
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    nrHelper->SetEpcHelper(epcHelper);

    // Set the channel using the scenario, condition and channel model
    // then enable shadowing
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    channelHelper->ConfigureFactories("UMa", "LOS", "ThreeGpp");
    channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

    // Create and set the channel with the band
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1;
    CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency, bandwidth, numCcPerBand);
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    channelHelper->AssignChannelsToBands({band});
    /**
     * Configure NrHelper, prepare most of the parameters that will be used in the simulation.
     */
    nrHelper->SetDlErrorModel(errorModel);
    nrHelper->SetUlErrorModel(errorModel);
    nrHelper->SetGnbDlAmcAttribute("AmcModel", EnumValue(NrAmc::ErrorModel));
    nrHelper->SetGnbUlAmcAttribute("AmcModel", EnumValue(NrAmc::ErrorModel));
    nrHelper->SetSchedulerTypeId(TypeId::LookupByName(scheduler));
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(TypeId::LookupByName(beamformingMethod)));
    // Core latency
    epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));

    nrHelper->SetupMimoPmi(mimoPmiParams);
    nrHelper->SetupGnbAntennas(apGnb);
    nrHelper->SetGnbAntennaAttribute("DowntiltAngle", DoubleValue(downtiltAngleGnb * M_PI / 180.0));
    nrHelper->SetupUeAntennas(apUe);

    nrHelper->SetGnbPhyAttribute("Numerology", UintegerValue(numerology));
    nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(txPowerGnb));
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(txPowerUe));
    nrHelper->SetUePhyAttribute("WbPmiUpdateInterval",
                                TimeValue(MilliSeconds(wbPmiUpdateIntervalMs)));
    nrHelper->SetUePhyAttribute("SbPmiUpdateInterval",
                                TimeValue(MilliSeconds(sbPmiUpdateIntervalMs)));

    uint32_t bwpId = 0;
    // gNb routing between bearer type and bandwidth part
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("NGBR_LOW_LAT_EMBB", UintegerValue(bwpId));
    // UE routing between bearer type and bandwidth part
    nrHelper->SetUeBwpManagerAlgorithmAttribute("NGBR_LOW_LAT_EMBB", UintegerValue(bwpId));
    /**
     * Initialize channel and pathloss, plus other things inside band.
     */
    BandwidthPartInfoPtrVector allBwps;
    allBwps = CcBwpCreator::GetAllBwps({band});

    /**
     * Finally, create the gNB and the UE device.
     */
    NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice(gnbContainer, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueContainer, allBwps);

    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams(enbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);

    std::map<uint16_t, CqiFeedbackTraceStats> cqiTraces;
    for (auto it = ueNetDev.Begin(); it != ueNetDev.End(); ++it)
    {
        auto cqiCb = MakeBoundCallback(&CqiFeedbackTracedCallback, &cqiTraces);
        NrHelper::GetUePhy(*it, 0)->TraceConnectWithoutContext("CqiFeedbackTrace", cqiCb);
    }

    // create the Internet and install the IP stack on the UEs
    // get SGW/PGW and create a single RemoteHost
    auto [remoteHost, remoteHostIpv4Address] =
        epcHelper->SetupRemoteHost("100Gb/s", 2500, Seconds(0.000));

    InternetStackHelper internet;
    Ipv4InterfaceContainer ueIpIface;
    internet.Install(ueContainer);
    ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));

    // attach each UE to its gNB according to desired scenario
    nrHelper->AttachToGnb(ueNetDev.Get(0), enbNetDev.Get(0));
    /**
     * Install DL traffic part.
     */
    uint16_t dlPort = 1234;
    ApplicationContainer serverApps;
    // The sink will always listen to the specified ports
    UdpServerHelper dlPacketSink(dlPort);
    // The server, that is the application which is listening, is installed in the UE
    serverApps.Add(dlPacketSink.Install(ueContainer));
    /**
     * Configure attributes for the CBR traffic generator, using user-provided
     * parameters
     */
    UdpClientHelper dlClient;
    dlClient.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    dlClient.SetAttribute("PacketSize", UintegerValue(udpPacketSize));
    dlClient.SetAttribute("Interval", TimeValue(packetInterval));

    // The bearer that will carry the traffic
    NrEpsBearer epsBearer(NrEpsBearer::NGBR_LOW_LAT_EMBB);

    // The filter for the traffic
    Ptr<NrEpcTft> dlTft = Create<NrEpcTft>();
    NrEpcTft::PacketFilter dlPktFilter;
    dlPktFilter.localPortStart = dlPort;
    dlPktFilter.localPortEnd = dlPort;
    dlTft->Add(dlPktFilter);

    /**
     * Let's install the applications!
     */
    ApplicationContainer clientApps;

    for (uint32_t i = 0; i < ueContainer.GetN(); ++i)
    {
        Ptr<Node> ue = ueContainer.Get(i);
        Ptr<NetDevice> ueDevice = ueNetDev.Get(i);
        Address ueAddress = ueIpIface.GetAddress(i);

        // The client, who is transmitting, is installed in the remote host,
        // with destination address set to the address of the UE
        dlClient.SetAttribute(
            "Remote",
            AddressValue(
                InetSocketAddress(Ipv4Address::ConvertFrom(ueAddress), dlPort).ConvertTo()));
        clientApps.Add(dlClient.Install(remoteHost));

        // Activate a dedicated bearer for the traffic
        nrHelper->ActivateDedicatedEpsBearer(ueDevice, epsBearer, dlTft);
    }

    // start UDP server and client apps
    serverApps.Start(udpAppStartTime);
    clientApps.Start(udpAppStartTime);
    serverApps.Stop(simTime);
    clientApps.Stop(simTime);

    // enable the traces provided by the nr module
    nrHelper->EnableTraces();

    FlowMonitorHelper flowmonHelper;
    NodeContainer endpointNodes;
    endpointNodes.Add(remoteHost);
    endpointNodes.Add(ueContainer);

    Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install(endpointNodes);
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

    Simulator::Stop(simTime);
    Simulator::Run();

    // Print per-flow statistics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    double averageFlowThroughput = 0.0;
    double averageFlowDelay = 0.0;
    double averageMcsForAllUes = 0.0;
    double averageRiForAllUes = 0.0;
    for (const auto& ue : cqiTraces)
    {
        averageRiForAllUes += ue.second.m_ri->getMean();
        averageMcsForAllUes += ue.second.m_mcs->getMean();
    }

    NS_TEST_ASSERT_MSG_EQ(ueNetDev.GetN(),
                          cqiTraces.size(),
                          "Not all UEs have generated CQI feedback.");

    double flowDuration = (simTime - udpAppStartTime).GetSeconds();
    for (auto i = stats.begin(); i != stats.end(); ++i)
    {
        if (i->second.rxPackets > 0)
        {
            // Measure the duration of the flow from receiver's perspective
            averageFlowThroughput += i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;
            averageFlowDelay += 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets;
        }
    }

    // Tolerate results with a 1% tolerance
    NS_TEST_EXPECT_MSG_EQ_TOL(averageFlowThroughput,
                              m_targetThroughput,
                              m_targetThroughput * 0.05,
                              "Throughput is out of the expected range");
    NS_TEST_EXPECT_MSG_EQ_TOL(averageFlowDelay,
                              m_targetLatency,
                              m_targetLatency * 0.05,
                              "Delay is out of the expected range");
    NS_TEST_EXPECT_MSG_EQ_TOL(averageRiForAllUes,
                              m_targetMeanRank,
                              m_targetMeanRank * 0.05,
                              "Rank is out of the expected range");
    NS_TEST_EXPECT_MSG_EQ_TOL(averageMcsForAllUes,
                              m_targetMeanMcs,
                              m_targetMeanMcs * 0.05,
                              "MCS is out of the expected range");

    Simulator::Destroy();
}

/* End of TestCttcNrMimoDemoTestCase */
class TestRiPmiSystem : public TestSuite
{
  public:
    TestRiPmiSystem()
        : TestSuite("nr-test-ri-pmi", Type::SYSTEM)
    {
        // Fully saturated channel with 200Mbps traffic
        // Parameters (gNB-UE distance, RI selection technique, RI threshold, PMI selection
        // technique, expected throughput, latency, mean RI and mean MCS)
        // clang-format off
        AddTestCase(new RiPmiTestCase( 20,             "",  0.0,    "ns3::NrPmSearchFull", 133.0, 150.0, 3.1, 25.0), Duration::QUICK);
        AddTestCase(new RiPmiTestCase(500,             "",  0.0,    "ns3::NrPmSearchFull", 104.0, 243.7, 2.3, 26.6), Duration::QUICK);
        AddTestCase(new RiPmiTestCase( 20,             "",  0.0,   "ns3::NrPmSearchIdeal", 154.0,  71.4, 3.5, 25.3), Duration::QUICK);
        AddTestCase(new RiPmiTestCase(500,             "",  0.0,   "ns3::NrPmSearchIdeal", 106.2, 205.4, 2.9, 24.0), Duration::QUICK);
        AddTestCase(new RiPmiTestCase( 20,          "SVD",  0.0,    "ns3::NrPmSearchFast", 114.4, 165.9, 4.0, 17.0), Duration::QUICK);
        AddTestCase(new RiPmiTestCase( 20,          "SVD",  0.5,    "ns3::NrPmSearchFast",  86.1, 291.1, 1.7, 27.0), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase( 20,          "SVD",  0.9,    "ns3::NrPmSearchFast",  51.0, 376.5, 1.0, 27.0), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase(500,          "SVD",  0.0,    "ns3::NrPmSearchFast",  62.4, 284.4, 4.0,  9.6), Duration::QUICK);
        AddTestCase(new RiPmiTestCase(500,          "SVD",  0.5,    "ns3::NrPmSearchFast",  96.9, 250.8, 1.9, 27.0), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase(500,          "SVD",  0.9,    "ns3::NrPmSearchFast",  53.0, 400.3, 1.1, 27.0), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase( 20, "WaterFilling", 10.0,    "ns3::NrPmSearchFast", 126.4, 157.6, 3.6, 20.5), Duration::QUICK);
        AddTestCase(new RiPmiTestCase( 20, "WaterFilling", 50.0,    "ns3::NrPmSearchFast", 128.2, 156.9, 3.3, 23.0), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase( 20, "WaterFilling", 75.0,    "ns3::NrPmSearchFast", 129.5, 155.7, 3.1, 24.5), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase( 20, "WaterFilling", 90.0,    "ns3::NrPmSearchFast", 129.5, 155.7, 3.1, 24.5), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase(500, "WaterFilling", 10.0,    "ns3::NrPmSearchFast",  92.0, 282.9, 3.1, 18.5), Duration::QUICK);
        AddTestCase(new RiPmiTestCase(500, "WaterFilling", 50.0,    "ns3::NrPmSearchFast",  99.8, 268.1, 2.3, 24.7), Duration::QUICK);
        AddTestCase(new RiPmiTestCase(500, "WaterFilling", 75.0,    "ns3::NrPmSearchFast", 101.8, 260.8, 2.2, 27.0), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase(500, "WaterFilling", 90.0,    "ns3::NrPmSearchFast", 101.8, 260.8, 2.2, 27.0), Duration::EXTENSIVE);
        AddTestCase(new RiPmiTestCase( 20,      "Sasaoka",  0.0,    "ns3::NrPmSearchFast", 124.3, 170.3, 3.1, 23.0), Duration::QUICK);
        AddTestCase(new RiPmiTestCase( 20,      "Sasaoka",  0.0, "ns3::NrPmSearchSasaoka", 117.6, 153.9, 3.1, 23.0), Duration::QUICK);
        AddTestCase(new RiPmiTestCase(500,      "Sasaoka",  0.0,    "ns3::NrPmSearchFast",  75.9, 299.2, 3.1, 15.0), Duration::QUICK);
        AddTestCase(new RiPmiTestCase(500,      "Sasaoka",  0.0, "ns3::NrPmSearchSasaoka",  76.5, 301.5, 3.1, 15.0), Duration::QUICK);
#ifdef PMI_MALEKI
        // Maleki's PMI test is enabled via a conditional compilation flag due to external
        // dependencies such as Pybind11+Pyttb presence
        AddTestCase(new RiPmiTestCase( 20,             "",  0.0,  "ns3::NrPmSearchMaleki", 120.2, 169.8, 2.6, 27.0), Duration::QUICK);
        AddTestCase(new RiPmiTestCase(500,             "",  0.0,  "ns3::NrPmSearchMaleki", 104.9, 242.7, 2.2, 27.0), Duration::QUICK);
#endif
        // clang-format on
    }
};

static TestRiPmiSystem g_testRiPmiSystem; //!< RI/PMI system tests

} // namespace ns3
