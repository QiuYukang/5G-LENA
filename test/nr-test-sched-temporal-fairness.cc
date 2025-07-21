// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#include "ns3/parabolic-antenna-model.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/test.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"

#include <cmath>
#include <iostream>
#include <map>
#include <string>

/**
 * @file nr-test-sched-temporal-fairness.cc
 * @ingroup test
 *
 * @brief System-testing for scheduler's temporal fairness,
 * ensuring not only resources are fairly distributed in a slot,
 * but across slots
 */
namespace ns3
{
/* Beginning of SchedTemporalFairnessTestCase */
class SchedTemporalFairnessTestCase : public TestCase
{
  public:
    SchedTemporalFairnessTestCase(std::string schedulerType)
        : TestCase(schedulerType),
          m_schedulerType(schedulerType)
    {
    }

  private:
    void DoRun() override;
    std::string m_schedulerType;
};

void
SchedTemporalFairnessTestCase::DoRun()
{
    // Traffic parameters to fully saturate channel
    uint32_t udpPacketSize = 1000;
    Time packetInterval = NanoSeconds(40000);
    Time udpAppStartTime = MilliSeconds(400);

    // Other simulation scenario parameters
    Time simTime = MilliSeconds(1000);
    uint16_t numerology = 0;
    double centralFrequency = 3.5e9;
    double bandwidth = 10e6;
    double txPowerGnb = 40; // dBm, super high to ensure no error
    double txPowerUe = 23;  // dBm
    std::string errorModel = "ns3::NrEesmIrT2";
    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));

    NodeContainer gnbContainer;
    gnbContainer.Create(1);
    NodeContainer ueContainer;
    ueContainer.Create(24);

    /**
     * We configure the mobility model to ConstantPositionMobilityModel.
     * All UEs are positioned at same UE1 position.
     * The default topology is the following:
     *
     * gNB1.................UE1..........
     *(0.0, 0.0, 25.0)  (d, 0.0, 1.5)
     */
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 25.0));
    for (std::size_t ueI = 0; ueI < ueContainer.GetN(); ueI++)
    {
        positionAlloc->Add(Vector(100, 0.0, 1.5));
    }
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(gnbContainer);
    mobility.Install(ueContainer);

    /**
     * Create the NR helpers that will be used to create and setup NR devices, spectrum, ...
     */
    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
    epcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));

    // Configure NR helper for SISO configuration and scheduler set by the test
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    nrHelper->SetEpcHelper(epcHelper);
    nrHelper->SetAttribute("CsiFeedbackFlags", UintegerValue(CsiFeedbackFlag::CQI_PDSCH_SISO));
    nrHelper->SetDlErrorModel(errorModel);
    nrHelper->SetUlErrorModel(errorModel);
    nrHelper->SetGnbDlAmcAttribute("AmcModel", EnumValue(NrAmc::ErrorModel));
    nrHelper->SetGnbUlAmcAttribute("AmcModel", EnumValue(NrAmc::ErrorModel));
    nrHelper->SetSchedulerTypeId(TypeId::LookupByName(m_schedulerType));
    nrHelper->SetGnbPhyAttribute("Numerology", UintegerValue(numerology));
    nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(txPowerGnb));
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(txPowerUe));
    nrHelper->SetUePhyAttribute("WbPmiUpdateInterval", TimeValue(MilliSeconds(0)));
    nrHelper->SetUePhyAttribute("SbPmiUpdateInterval", TimeValue(MilliSeconds(0)));

    // Set the channel using the scenario, condition and channel model
    // then enable shadowing
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();

    // Override the default antenna model with ParabolicAntennaModel
    nrHelper->SetUeAntennaTypeId(ParabolicAntennaModel::GetTypeId().GetName());
    nrHelper->SetGnbAntennaTypeId(ParabolicAntennaModel::GetTypeId().GetName());

    // Configure Friis propagation loss model before assign it to band
    channelHelper->ConfigurePropagationFactory(FriisPropagationLossModel::GetTypeId());

    // Create and set the channel with the band
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1;
    CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency, bandwidth, numCcPerBand);
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    channelHelper->AssignChannelsToBands({band});

    // Create bandwidth part from band
    BandwidthPartInfoPtrVector allBwps;
    allBwps = CcBwpCreator::GetAllBwps({band});

    // Create gNB and UE network devices
    NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice(gnbContainer, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueContainer, allBwps);

    // Assign random variable streams for reproducible results
    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams(enbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);

    // Create the Internet and install the IP stack on the UEs
    // get SGW/PGW and create a single RemoteHost
    auto [remoteHost, remoteHostIpv4Address] =
        epcHelper->SetupRemoteHost("100Gb/s", 2500, Seconds(0.000));

    InternetStackHelper internet;
    Ipv4InterfaceContainer ueIpIface;
    internet.Install(ueContainer);
    ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));

    // Attach each UE to its gNB according to desired scenario
    nrHelper->AttachToClosestGnb(ueNetDev, enbNetDev);

    // Install dlPacketSink applications on UEs to receive CBR traffic from remote host
    uint16_t dlPort = 1234;
    ApplicationContainer serverApps;
    UdpServerHelper dlPacketSink(dlPort);
    serverApps.Add(dlPacketSink.Install(ueContainer));

    // Install UdpClient on remote host configured to generate CBR traffic towards UEs
    UdpClientHelper dlClient;
    dlClient.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    dlClient.SetAttribute("PacketSize", UintegerValue(udpPacketSize));
    dlClient.SetAttribute("Interval", TimeValue(packetInterval));
    ApplicationContainer clientApps;
    for (uint32_t i = 0; i < ueContainer.GetN(); ++i)
    {
        Ptr<Node> ue = ueContainer.Get(i);
        Ptr<NetDevice> ueDevice = ueNetDev.Get(i);
        Address ueAddress = ueIpIface.GetAddress(i);
        dlClient.SetAttribute(
            "Remote",
            AddressValue(
                InetSocketAddress(Ipv4Address::ConvertFrom(ueAddress), dlPort).ConvertTo()));
        clientApps.Add(dlClient.Install(remoteHost));
    }

    // start UDP server and client apps
    serverApps.Start(udpAppStartTime);
    clientApps.Start(udpAppStartTime);
    serverApps.Stop(simTime);
    clientApps.Stop(simTime);

    // We want to monitor flows to UEs
    FlowMonitorHelper flowmonHelper;
    NodeContainer monitoredNodes;
    monitoredNodes.Add(remoteHost);
    monitoredNodes.Add(ueContainer);
    Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install(monitoredNodes);
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

    Simulator::Stop(simTime);
    Simulator::Run();

    // Collect per-flow statistics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    double flowDuration = (simTime - udpAppStartTime).GetSeconds();
    double avgThr = 0.0;
    std::deque<std::pair<double, Ipv4Address>> flowThroughputs;

    // Measure the duration of the flow from receiver's perspective
    for (auto i = stats.begin(); i != stats.end(); ++i)
    {
        // Retrieve flow src/tgt addresses and ports
        Ipv4FlowClassifier::FiveTuple t =
            DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier())->FindFlow(i->first);

        // We expect the UE to have received at least one packet
        NS_TEST_ASSERT_MSG_GT(i->second.rxPackets,
                              0,
                              "Expected at least one received packet at " << t.destinationAddress);
        auto flowThr = i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;

        // We finally check if scheduler is minimally fair, by checking if we have unexpected zero
        // throughput's
        NS_TEST_EXPECT_MSG_GT(flowThr,
                              0,
                              "Expected throughput higher than zero at " << t.destinationAddress);
        flowThroughputs.emplace_back(flowThr, t.destinationAddress);
        avgThr += flowThr;
    }
    avgThr /= flowThroughputs.size();

    // Now we check if all throughputs are within 5% of the average one
    for (auto [flowThr, ueAddress] : flowThroughputs)
    {
        NS_TEST_EXPECT_MSG_EQ_TOL(flowThr,
                                  avgThr,
                                  0.05 * avgThr,
                                  "Expected UE throughput closer to average at " << ueAddress);
    }

    Simulator::Destroy();
}

/* End of SchedTemporalFairnessTestCase */
class TestSchedTemporalFairnessSystem : public TestSuite
{
  public:
    TestSchedTemporalFairnessSystem()
        : TestSuite("nr-test-sched-temporal-fairness", Type::SYSTEM)
    {
        // clang-format off
        AddTestCase(new SchedTemporalFairnessTestCase("ns3::NrMacSchedulerTdmaRR"),Duration::QUICK);
        AddTestCase(new SchedTemporalFairnessTestCase("ns3::NrMacSchedulerTdmaPF"), Duration::QUICK);
        AddTestCase(new SchedTemporalFairnessTestCase("ns3::NrMacSchedulerTdmaQos"),Duration::QUICK);
        AddTestCase(new SchedTemporalFairnessTestCase("ns3::NrMacSchedulerOfdmaRR"),   Duration::QUICK);
        AddTestCase(new SchedTemporalFairnessTestCase("ns3::NrMacSchedulerOfdmaPF"),  Duration::QUICK);
        AddTestCase(new SchedTemporalFairnessTestCase("ns3::NrMacSchedulerOfdmaQos"), Duration::QUICK);
        // clang-format on
    }
};

static TestSchedTemporalFairnessSystem
    g_testSchedTemporalFairnessSystem; //!< scheduler's temporal fairness system tests

} // namespace ns3
