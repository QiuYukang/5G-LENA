// Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only and NIST-Software
// Authors : Gaurav Sathe <gaurav.sathe@tcs.com>
//           Tom Henderson <thomas.henderson@nist.gov>

#include "nr-test-deactivate-bearer.h"

#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/nr-bearer-stats-calculator.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-gnb-phy.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/nr-qos-flow.h"
#include "ns3/nr-stats-calculator.h"
#include "ns3/nr-ue-net-device.h"
#include "ns3/nr-ue-phy.h"
#include "ns3/nr-ue-rrc.h"
#include "ns3/object.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/ptr.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/type-id.h"
#include "ns3/udp-client-server-helper.h"

#include <iostream>
#include <sstream>
#include <string>

NS_LOG_COMPONENT_DEFINE("NrTestDeactivateBearer");

namespace ns3
{

/**
 * @brief Helper to count received bytes via packet sink tracing
 */
class PacketCounter
{
  public:
    PacketCounter()
        : m_bytes(0)
    {
    }

    void TraceRx(Ptr<const Packet> p, const Address& addr)
    {
        m_bytes += p->GetSize();
    }

    uint64_t GetBytes() const
    {
        return m_bytes;
    }

    void Reset()
    {
        m_bytes = 0;
    }

  private:
    uint64_t m_bytes;
};

NrTestBearerDeactivateSuite::NrTestBearerDeactivateSuite()
    : TestSuite("nr-test-deactivate-bearer", Type::SYSTEM)
{
    // Test configuration: single UE at 1 meter distance from gNB
    bool errorModelEnabled = false;
    std::vector<uint16_t> ueDistances{1}; // Single UE at 1 meter

    // Packet sizes for traffic flows
    std::vector<uint16_t> packetSizes;
    packetSizes.push_back(100); // IPv4 and IPv6 packets

    // Reference throughput values (not actively used in this test)
    // Add test case: single UE with 10ms traffic interval
    AddTestCase(new NrDeactivateBearerTestCase(ueDistances,
                                               packetSizes,
                                               MilliSeconds(10), // 10ms interval between packets
                                               errorModelEnabled,
                                               true), // Use ideal RRC
                TestCase::Duration::QUICK);
}

/**
 * @ingroup nr-test
 * Static variable for test initialization
 */
static NrTestBearerDeactivateSuite lenaTestBearerDeactivateSuite;

std::string
NrDeactivateBearerTestCase::BuildNameString(uint16_t numberOfUEs, std::vector<uint16_t> ueDistances)
{
    std::ostringstream oss;
    oss << "nr-dynamic-bearer-deactivation, " << numberOfUEs << " UE(s), distances (m) = [ ";
    for (auto it = ueDistances.begin(); it != ueDistances.end(); ++it)
    {
        oss << *it << " ";
    }
    oss << "]";
    return oss.str();
}

NrDeactivateBearerTestCase::NrDeactivateBearerTestCase(std::vector<uint16_t> ueDistances,
                                                       std::vector<uint16_t> packetSizes,
                                                       Time trafficInterval,
                                                       bool errorModelEnabled,
                                                       bool useIdealRrc)
    : TestCase(BuildNameString(ueDistances.size(), ueDistances)),
      m_numberOfUEs(ueDistances.size()),
      m_ueDistances(ueDistances),
      m_packetSizes(packetSizes),
      m_trafficInterval(trafficInterval),
      m_errorModelEnabled(errorModelEnabled)
{
}

NrDeactivateBearerTestCase::~NrDeactivateBearerTestCase()
{
}

void
NrDeactivateBearerTestCase::DoRun()
{
    NS_LOG_LOGIC("Starting NrDeactivateBearerTestCase::DoRun");

    // Cache the prevailing seed and run so they can be restored later
    uint32_t originalSeed = RngSeedManager::GetSeed();
    uint32_t originalRun = RngSeedManager::GetRun();
    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(1);

    // Configure PHY layer error model behavior
    if (!m_errorModelEnabled)
    {
        Config::SetDefault("ns3::NrSpectrumPhy::DataErrorModelEnabled", BooleanValue(false));
    }

    // Configure RRC and helper behavior
    Config::SetDefault("ns3::NrHelper::UseIdealRrc", BooleanValue(true));

    // Create NR and EPC helpers
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    Ptr<NrPointToPointEpcHelper> nrEpcHelper = CreateObject<NrPointToPointEpcHelper>();
    nrHelper->SetEpcHelper(nrEpcHelper);

    // Get the PGW node for internet connectivity
    Ptr<Node> pgwNode = nrEpcHelper->GetPgwNode();

    // Create remote host and install internet stack
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    // Create point-to-point link between PGW and remote host
    PointToPointHelper p2pHelper;
    p2pHelper.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2pHelper.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2pHelper.SetChannelAttribute("Delay", TimeValue(Seconds(0.001)));
    NetDeviceContainer internetDevices = p2pHelper.Install(pgwNode, remoteHost);

    // Configure IPv4 addresses and routing on internet link
    Ipv4AddressHelper ipv4AddressHelper;
    ipv4AddressHelper.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpv4Interfaces = ipv4AddressHelper.Assign(internetDevices);
    Ipv4Address remoteHostIpv4Addr = internetIpv4Interfaces.GetAddress(1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    // Configure IPv6 addresses and routing on internet link
    Ipv6AddressHelper ipv6AddressHelper;
    ipv6AddressHelper.SetBase(Ipv6Address("6001:db80::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer internetIpv6Interfaces = ipv6AddressHelper.Assign(internetDevices);
    internetIpv6Interfaces.SetForwarding(0, true);
    internetIpv6Interfaces.SetForwarding(1, true);

    Ipv6Address remoteHostIpv6Addr = internetIpv6Interfaces.GetAddress(1, 1);
    Ipv6Address pgwIpv6Addr = internetIpv6Interfaces.GetAddress(0, 1);
    Ipv6StaticRoutingHelper ipv6RoutingHelper;
    Ptr<Ipv6StaticRouting> remoteHostIpv6StaticRouting =
        ipv6RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv6>());
    remoteHostIpv6StaticRouting->AddNetworkRouteTo(Ipv6Address("6001:db80::"),
                                                   Ipv6Prefix(64),
                                                   pgwIpv6Addr,
                                                   1);
    // Route to EPC's internal IPv6 range (UE addresses are assigned from 7777:f00d::/64 by the EPC)
    remoteHostIpv6StaticRouting->AddNetworkRouteTo(Ipv6Address("7777:f00d::"),
                                                   Ipv6Prefix(64),
                                                   pgwIpv6Addr,
                                                   1);

    // Create gNB and UE nodes
    NodeContainer gnbNodes;
    NodeContainer ueNodes;
    gnbNodes.Create(1);
    ueNodes.Create(m_numberOfUEs);

    // Install mobility models
    MobilityHelper mobilityHelper;
    mobilityHelper.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobilityHelper.Install(gnbNodes);
    mobilityHelper.Install(ueNodes);

    // Create bandwidth parts for NR
    auto bandwidthAndBWPPair = nrHelper->CreateBandwidthParts({{2.8e9, 5e6, 1}}, "UMa");

    // Install NR devices on gNB and UEs
    NetDeviceContainer gnbDevices;
    NetDeviceContainer ueDevices;
    int64_t randomStreamIndex = 1;

    nrHelper->SetSchedulerTypeId(TypeId::LookupByName("ns3::NrMacSchedulerTdmaRR"));
    gnbDevices = nrHelper->InstallGnbDevice(gnbNodes, bandwidthAndBWPPair.second);
    randomStreamIndex += nrHelper->AssignStreams(gnbDevices, randomStreamIndex);

    ueDevices = nrHelper->InstallUeDevice(ueNodes, bandwidthAndBWPPair.second);
    randomStreamIndex += nrHelper->AssignStreams(ueDevices, randomStreamIndex);

    // Configure gNB PHY parameters
    Ptr<NrGnbNetDevice> nrGnbDevice = gnbDevices.Get(0)->GetObject<NrGnbNetDevice>();
    Ptr<NrGnbPhy> gnbPhy = nrGnbDevice->GetPhy(0);
    gnbPhy->SetAttribute("TxPower", DoubleValue(30.0));
    gnbPhy->SetAttribute("NoiseFigure", DoubleValue(5.0));

    // Configure UE positions and PHY parameters
    for (uint32_t ueIndex = 0; ueIndex < m_numberOfUEs; ueIndex++)
    {
        Ptr<ConstantPositionMobilityModel> ueMobilityModel =
            ueNodes.Get(ueIndex)->GetObject<ConstantPositionMobilityModel>();
        ueMobilityModel->SetPosition(Vector(m_ueDistances.at(ueIndex), 0.0, 0.0));

        Ptr<NrUeNetDevice> nrUeDevice = ueDevices.Get(ueIndex)->GetObject<NrUeNetDevice>();
        Ptr<NrUePhy> uePhy = nrUeDevice->GetPhy(0);
        uePhy->SetAttribute("TxPower", DoubleValue(23.0));
        uePhy->SetAttribute("NoiseFigure", DoubleValue(9.0));
    }

    // Install internet stack on UEs and assign IP addresses
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpv4Interfaces = nrEpcHelper->AssignUeIpv4Address(ueDevices);
    Ipv6InterfaceContainer ueIpv6Interfaces = nrEpcHelper->AssignUeIpv6Address(ueDevices);

    // Get references to the test UE and gNB devices
    Ptr<NetDevice> testUeDevice = ueDevices.Get(0);
    Ptr<NetDevice> testGnbDevice = gnbDevices.Get(0);
    Ipv4Address testUeIpv4Addr = ueIpv4Interfaces.GetAddress(0);
    Ipv6Address testUeIpv6Addr = ueIpv6Interfaces.GetAddress(0, 1);
    NS_LOG_INFO("UE IPv4 address: " << testUeIpv4Addr);
    NS_LOG_INFO("UE IPv6 address: " << testUeIpv6Addr);
    NS_LOG_INFO("Remote host IPv4 address: " << remoteHostIpv4Addr);
    NS_LOG_INFO("Remote host IPv6 address: " << remoteHostIpv6Addr);

    // Install downlink and uplink applications
    NS_LOG_INFO("Setting up IPv4 and IPv6 UDP traffic flows");

    // Track DL and UL bytes received
    PacketCounter ipv4DlCounter;
    PacketCounter ipv6DlCounter;
    PacketCounter ipv4UlCounter;
    PacketCounter ipv6UlCounter;

    // Setup IPv4 downlink (remote host sends to UE)
    {
        uint16_t dlPort = 1235;
        PacketSinkHelper dlPacketSinkHelper("ns3::UdpSocketFactory",
                                            InetSocketAddress(Ipv4Address::GetAny(), dlPort));
        ApplicationContainer dlSinkApp = dlPacketSinkHelper.Install(ueNodes.Get(0));
        dlSinkApp.Start(MilliSeconds(30));
        NS_LOG_INFO("IPv4 DL sink listening on UE at " << testUeIpv4Addr << ":" << dlPort);

        // Add trace to count received bytes
        dlSinkApp.Get(0)->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&PacketCounter::TraceRx, &ipv4DlCounter));

        UdpClientHelper dlUdpClient(testUeIpv4Addr, dlPort);
        dlUdpClient.SetAttribute("Interval", TimeValue(m_trafficInterval));
        dlUdpClient.SetAttribute("MaxPackets", UintegerValue(1000000));
        dlUdpClient.SetAttribute("PacketSize", UintegerValue(m_packetSizes.at(0)));
        ApplicationContainer dlClientApp = dlUdpClient.Install(remoteHost);
        dlClientApp.Start(MilliSeconds(30));
        NS_LOG_INFO("IPv4 DL client on remote host sending to " << testUeIpv4Addr << ":" << dlPort);
    }

    // Setup IPv4 uplink (UE sends to remote host)
    {
        uint16_t ulPort = 2001;
        PacketSinkHelper ulPacketSinkHelper("ns3::UdpSocketFactory",
                                            InetSocketAddress(Ipv4Address::GetAny(), ulPort));
        ApplicationContainer ulSinkApp = ulPacketSinkHelper.Install(remoteHost);
        ulSinkApp.Start(MilliSeconds(30));

        // Add trace to count received bytes
        ulSinkApp.Get(0)->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&PacketCounter::TraceRx, &ipv4UlCounter));

        UdpClientHelper ulUdpClient(remoteHostIpv4Addr, ulPort);
        ulUdpClient.SetAttribute("Interval", TimeValue(m_trafficInterval));
        ulUdpClient.SetAttribute("MaxPackets", UintegerValue(1000000));
        ulUdpClient.SetAttribute("PacketSize", UintegerValue(m_packetSizes.at(0)));
        ApplicationContainer ulClientApp = ulUdpClient.Install(ueNodes.Get(0));
        ulClientApp.Start(MilliSeconds(30));
    }

    // Setup IPv6 downlink (remote host sends to UE)
    {
        uint16_t dlPort = 1236;
        PacketSinkHelper dlPacketSinkHelper("ns3::UdpSocketFactory",
                                            Inet6SocketAddress(Ipv6Address::GetAny(), dlPort));
        ApplicationContainer dlSinkApp = dlPacketSinkHelper.Install(ueNodes.Get(0));
        dlSinkApp.Start(MilliSeconds(30));
        NS_LOG_INFO("IPv6 DL sink listening on UE at " << testUeIpv6Addr << ":" << dlPort);

        // Add trace to count received bytes
        dlSinkApp.Get(0)->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&PacketCounter::TraceRx, &ipv6DlCounter));

        UdpClientHelper dlUdpClient(testUeIpv6Addr, dlPort);
        dlUdpClient.SetAttribute("Interval", TimeValue(m_trafficInterval));
        dlUdpClient.SetAttribute("MaxPackets", UintegerValue(1000000));
        dlUdpClient.SetAttribute("PacketSize", UintegerValue(m_packetSizes.at(0)));
        ApplicationContainer dlClientApp = dlUdpClient.Install(remoteHost);
        dlClientApp.Start(MilliSeconds(30));
        NS_LOG_INFO("IPv6 DL client on remote host sending to " << testUeIpv6Addr << ":" << dlPort);
    }

    // Setup IPv6 uplink (UE sends to remote host)
    {
        uint16_t ulPort = 2002;
        PacketSinkHelper ulPacketSinkHelper("ns3::UdpSocketFactory",
                                            Inet6SocketAddress(Ipv6Address::GetAny(), ulPort));
        ApplicationContainer ulSinkApp = ulPacketSinkHelper.Install(remoteHost);
        ulSinkApp.Start(MilliSeconds(30));

        // Add trace to count received bytes
        ulSinkApp.Get(0)->TraceConnectWithoutContext(
            "Rx",
            MakeCallback(&PacketCounter::TraceRx, &ipv6UlCounter));

        UdpClientHelper ulUdpClient(remoteHostIpv6Addr, ulPort);
        ulUdpClient.SetAttribute("Interval", TimeValue(m_trafficInterval));
        ulUdpClient.SetAttribute("MaxPackets", UintegerValue(1000000));
        ulUdpClient.SetAttribute("PacketSize", UintegerValue(m_packetSizes.at(0)));
        ApplicationContainer ulClientApp = ulUdpClient.Install(ueNodes.Get(0));
        ulClientApp.Start(MilliSeconds(30));
    }

    // NOTE: Dynamic activation of QoS flows is not supported in the current ns-3 NR model.
    // The NrEpcUeNas::ActivateQosFlow() method explicitly rejects activation after the UE
    // enters the ACTIVE state (which occurs during AttachToGnb). The necessary NAS signaling
    // for post-attachment flow activation is not implemented. Therefore, all dedicated QoS
    // flows must be activated during the initial context setup (before simulation starts).
    // See NrEpcUeNas::ActivateQosFlows() for details.

    NS_LOG_INFO("Activating QoS flows during context setup (before simulation)");

    // Attach UE to gNB - this activates the default bearer (QFI=1, LCID=3)
    nrHelper->AttachToGnb(testUeDevice, testGnbDevice);

    // Activate QFI=3 (LCID=5, DRBID=5) with precedence=10
    // This will be the second-highest precedence (evaluated second)
    {
        NrGbrQosInformation qos;
        qos.gbrDl = 100000; // 100 kbps
        qos.gbrUl = 100000;
        qos.mbrDl = qos.gbrDl;
        qos.mbrUl = qos.gbrUl;

        NrQosFlow flow(NrQosFlow::GBR_CONV_VOICE, qos);
        flow.arp.priorityLevel = 9;
        flow.arp.preemptionCapability = true;
        flow.arp.preemptionVulnerability = true;

        Ptr<NrQosRule> rule = Create<NrQosRule>();
        rule->SetPrecedence(10);

        // IPv4 packet filter for remote host (1.0.0.0/8 range)
        NrQosRule::PacketFilter ipv4Filter;
        ipv4Filter.direction = NrQosRule::BIDIRECTIONAL;
        ipv4Filter.remoteAddress = Ipv4Address("1.0.0.0");
        ipv4Filter.remoteMask = Ipv4Mask("255.0.0.0");
        ipv4Filter.remotePortStart = 0;
        ipv4Filter.remotePortEnd = 65535;
        ipv4Filter.localPortStart = 0;
        ipv4Filter.localPortEnd = 65535;
        rule->Add(ipv4Filter);

        // IPv6 packet filter for 6001:db80::/64 range
        NrQosRule::PacketFilter ipv6Filter;
        ipv6Filter.direction = NrQosRule::BIDIRECTIONAL;
        ipv6Filter.remoteIpv6Address = Ipv6Address("6001:db80::");
        ipv6Filter.remoteIpv6Prefix = Ipv6Prefix(64);
        ipv6Filter.remotePortStart = 0;
        ipv6Filter.remotePortEnd = 65535;
        ipv6Filter.localPortStart = 0;
        ipv6Filter.localPortEnd = 65535;
        rule->Add(ipv6Filter);

        nrHelper->ActivateDedicatedQosFlow(testUeDevice, flow, rule);
        NS_LOG_INFO("QFI=3 activated (precedence=10)");
    }

    // Activate QFI=4 (LCID=6, DRBID=6) with precedence=5
    // This will be the lowest precedence value (evaluated first, highest precedence)
    {
        NrGbrQosInformation qos;
        qos.gbrDl = 100000; // 100 kbps
        qos.gbrUl = 100000;
        qos.mbrDl = qos.gbrDl;
        qos.mbrUl = qos.gbrUl;

        NrQosFlow flow(NrQosFlow::GBR_CONV_VOICE, qos);
        flow.arp.priorityLevel = 8;
        flow.arp.preemptionCapability = true;
        flow.arp.preemptionVulnerability = true;

        Ptr<NrQosRule> rule = Create<NrQosRule>();
        rule->SetPrecedence(5); // Lowest precedence value = evaluated first

        // IPv4 packet filter for remote host (1.0.0.0/8 range)
        NrQosRule::PacketFilter ipv4Filter;
        ipv4Filter.direction = NrQosRule::BIDIRECTIONAL;
        ipv4Filter.remoteAddress = Ipv4Address("1.0.0.0");
        ipv4Filter.remoteMask = Ipv4Mask("255.0.0.0");
        ipv4Filter.remotePortStart = 0;
        ipv4Filter.remotePortEnd = 65535;
        ipv4Filter.localPortStart = 0;
        ipv4Filter.localPortEnd = 65535;
        rule->Add(ipv4Filter);

        // IPv6 packet filter for 6001:db80::/64 range
        NrQosRule::PacketFilter ipv6Filter;
        ipv6Filter.direction = NrQosRule::BIDIRECTIONAL;
        ipv6Filter.remoteIpv6Address = Ipv6Address("6001:db80::");
        ipv6Filter.remoteIpv6Prefix = Ipv6Prefix(64);
        ipv6Filter.remotePortStart = 0;
        ipv6Filter.remotePortEnd = 65535;
        ipv6Filter.localPortStart = 0;
        ipv6Filter.localPortEnd = 65535;
        rule->Add(ipv6Filter);

        nrHelper->ActivateDedicatedQosFlow(testUeDevice, flow, rule);
        NS_LOG_INFO("QFI=4 activated (precedence=5, lowest precedence value)");
    }

    // Enable RLC statistics collection
    nrHelper->EnableRlcE2eTraces();

    // Schedule QoS flow deactivations during simulation
    NS_LOG_INFO("Scheduling bearer deactivations:");

    // At 1.0s: Deactivate QFI=4 (LCID=6)
    // After this, traffic falls back to QFI=3 (LCID=5)
    NS_LOG_INFO("  1.0s: Deactivate QFI=4 (LCID=6)");
    Simulator::Schedule(Seconds(1.0), [nrHelper, testUeDevice, testGnbDevice]() {
        nrHelper->DeActivateDedicatedQosFlow(testUeDevice, testGnbDevice, 4);
        NS_LOG_INFO("QFI=4 deactivated; traffic falls back to QFI=3");
    });

    // At 1.5s: Deactivate QFI=3 (LCID=5)
    // After this, traffic falls back to default QFI=1 (LCID=3)
    NS_LOG_INFO("  1.5s: Deactivate QFI=3 (LCID=5)");
    Simulator::Schedule(Seconds(1.5), [nrHelper, testUeDevice, testGnbDevice]() {
        nrHelper->DeActivateDedicatedQosFlow(testUeDevice, testGnbDevice, 3);
        NS_LOG_INFO("QFI=3 deactivated; traffic falls back to default QFI=1");
    });

    // Stop simulation at 2.0 seconds
    Simulator::Stop(Seconds(2.0));

    // Run simulation
    Simulator::Run();

    // Get test UE IMSI for statistics collection
    uint64_t testImsi = testUeDevice->GetObject<NrUeNetDevice>()->GetImsi();

    NS_LOG_INFO("Collecting statistics for IMSI " << testImsi);

    // Retrieve the RLC statistics calculator that was connected to the traces
    // during EnableRlcE2eTraces()
    Ptr<NrBearerStatsCalculator> rlcStats = nrHelper->GetRlcStatsCalculator();

    // Window 1: 0.03-1.0s - All traffic on QFI=4 (LCID=6)
    // QFI=4 has the lowest precedence value (5), so all traffic matches this rule first
    // Expected byte counts for Window 1 (0.97 seconds duration):
    // - Each flow: 10 packets/sec * 0.97s * 100 bytes/packet = 970 bytes payload
    // - With IP headers (20 bytes IPv4, 40 bytes IPv6) and UDP (8 bytes):
    //   IPv4: 97 packets * 128 bytes = 12,416 bytes
    //   IPv6: 97 packets * 148 bytes = 14,356 bytes
    //   Total: 26,772 bytes (accounts for RLC overhead, expect 26,000-28,000)
    NS_LOG_INFO("Window 1 (0.03-1.0s): QFI=4 (LCID=6) active with lowest precedence");
    uint64_t dlBytesWindow1Qfi4 = rlcStats->GetDlRxData(testImsi, 6);
    uint64_t ulBytesWindow1Qfi4 = rlcStats->GetUlRxData(testImsi, 6);
    NS_LOG_INFO("  LCID=6 (QFI=4): DL=" << dlBytesWindow1Qfi4 << " bytes, UL=" << ulBytesWindow1Qfi4
                                        << " bytes");
    NS_TEST_ASSERT_MSG_NE(dlBytesWindow1Qfi4,
                          0,
                          "No DL traffic on QFI=4 (LCID=6) with lowest precedence");
    NS_TEST_ASSERT_MSG_GT_OR_EQ(dlBytesWindow1Qfi4,
                                26000,
                                "DL bytes on LCID=6 below expected range (26000-28000)");
    NS_TEST_ASSERT_MSG_LT_OR_EQ(dlBytesWindow1Qfi4,
                                28000,
                                "DL bytes on LCID=6 above expected range (26000-28000)");
    NS_TEST_ASSERT_MSG_GT_OR_EQ(ulBytesWindow1Qfi4,
                                26000,
                                "UL bytes on LCID=6 below expected range (26000-28000)");
    NS_TEST_ASSERT_MSG_LT_OR_EQ(ulBytesWindow1Qfi4,
                                28000,
                                "UL bytes on LCID=6 above expected range (26000-28000)");

    // Window 2: 1.0-1.5s - Traffic on QFI=3 (LCID=5) after QFI=4 deactivation
    // After QFI=4 is deactivated, traffic falls back to QFI=3 with precedence=10
    // Expected byte counts for Window 2 (0.5 seconds duration):
    // - Each flow: 10 packets/sec * 0.5s * 100 bytes/packet = 500 bytes payload
    // - With IP headers and UDP:
    //   IPv4: 50 packets * 128 bytes = 6,400 bytes
    //   IPv6: 50 packets * 148 bytes = 7,400 bytes
    //   Total: 13,800 bytes (accounts for RLC overhead, expect 13,500-14,500)
    NS_LOG_INFO("Window 2 (1.0-1.5s): QFI=4 deactivated, QFI=3 (LCID=5) active");
    uint64_t dlBytesWindow2Qfi3 = rlcStats->GetDlRxData(testImsi, 5);
    uint64_t ulBytesWindow2Qfi3 = rlcStats->GetUlRxData(testImsi, 5);
    NS_LOG_INFO("  LCID=5 (QFI=3): DL=" << dlBytesWindow2Qfi3 << " bytes, UL=" << ulBytesWindow2Qfi3
                                        << " bytes");
    NS_TEST_ASSERT_MSG_NE(dlBytesWindow2Qfi3,
                          0,
                          "No DL traffic on QFI=3 (LCID=5) after QFI=4 deactivation");
    NS_TEST_ASSERT_MSG_GT_OR_EQ(dlBytesWindow2Qfi3,
                                13500,
                                "DL bytes on LCID=5 below expected range (13500-14500)");
    NS_TEST_ASSERT_MSG_LT_OR_EQ(dlBytesWindow2Qfi3,
                                14500,
                                "DL bytes on LCID=5 above expected range (13500-14500)");
    NS_TEST_ASSERT_MSG_GT_OR_EQ(ulBytesWindow2Qfi3,
                                13500,
                                "UL bytes on LCID=5 below expected range (13500-14500)");
    NS_TEST_ASSERT_MSG_LT_OR_EQ(ulBytesWindow2Qfi3,
                                14500,
                                "UL bytes on LCID=5 above expected range (13500-14500)");

    // Window 3: 1.5-2.0s - Traffic on QFI=1 (LCID=3, default) after all dedicated deactivations
    // After both QFI=4 and QFI=3 are deactivated, traffic falls back to the default bearer.
    // The default bearer has implicit precedence of 255 (highest precedence value = lowest
    // priority). Expected byte counts for Window 3 (0.5 seconds duration):
    // - Same as Window 2 since duration is identical
    // - Each flow: 10 packets/sec * 0.5s * 100 bytes/packet = 500 bytes payload
    // - With IP headers and UDP:
    //   IPv4: 50 packets * 128 bytes = 6,400 bytes
    //   IPv6: 50 packets * 148 bytes = 7,400 bytes
    //   Total: 13,800 bytes (accounts for RLC overhead, expect 13,500-14,500)
    NS_LOG_INFO(
        "Window 3 (1.5-2.0s): All dedicated flows deactivated, default QFI=1 (LCID=3) active");
    uint64_t dlBytesWindow3Qfi1 = rlcStats->GetDlRxData(testImsi, 3);
    uint64_t ulBytesWindow3Qfi1 = rlcStats->GetUlRxData(testImsi, 3);
    NS_LOG_INFO("  LCID=3 (QFI=1, default): DL=" << dlBytesWindow3Qfi1 << " bytes, UL="
                                                 << ulBytesWindow3Qfi1 << " bytes");
    NS_TEST_ASSERT_MSG_NE(dlBytesWindow3Qfi1,
                          0,
                          "No DL traffic on default QFI=1 (LCID=3) after all deactivations");
    NS_TEST_ASSERT_MSG_GT_OR_EQ(dlBytesWindow3Qfi1,
                                13500,
                                "DL bytes on LCID=3 below expected range (13500-14500)");
    NS_TEST_ASSERT_MSG_LT_OR_EQ(dlBytesWindow3Qfi1,
                                14500,
                                "DL bytes on LCID=3 above expected range (13500-14500)");
    NS_TEST_ASSERT_MSG_GT_OR_EQ(ulBytesWindow3Qfi1,
                                13500,
                                "UL bytes on LCID=3 below expected range (13500-14500)");
    NS_TEST_ASSERT_MSG_LT_OR_EQ(ulBytesWindow3Qfi1,
                                14500,
                                "UL bytes on LCID=3 above expected range (13500-14500)");

    NS_LOG_INFO("Application-level packet reception:");
    NS_LOG_INFO("Downlink (remote host -> UE):");
    NS_LOG_INFO("  IPv4 DL received: " << ipv4DlCounter.GetBytes() << " bytes");
    NS_LOG_INFO("  IPv6 DL received: " << ipv6DlCounter.GetBytes() << " bytes");
    NS_LOG_INFO("  Total DL: " << (ipv4DlCounter.GetBytes() + ipv6DlCounter.GetBytes())
                               << " bytes");
    NS_LOG_INFO("Uplink (UE -> remote host):");
    NS_LOG_INFO("  IPv4 UL received: " << ipv4UlCounter.GetBytes() << " bytes");
    NS_LOG_INFO("  IPv6 UL received: " << ipv6UlCounter.GetBytes() << " bytes");
    NS_LOG_INFO("  Total UL: " << (ipv4UlCounter.GetBytes() + ipv6UlCounter.GetBytes())
                               << " bytes");

    Simulator::Destroy();

    RngSeedManager::SetSeed(originalSeed);
    RngSeedManager::SetRun(originalRun);
}
} // namespace ns3
