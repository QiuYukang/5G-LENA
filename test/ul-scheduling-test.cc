// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ul-scheduling-test.h"

#include "ns3/applications-module.h"
#include "ns3/config.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
#include "ns3/nr-module.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-helper.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace ns3
{
namespace fs = std::filesystem;

NS_LOG_COMPONENT_DEFINE("UlSchedulingTestCase");

UlSchedulingTestSuite::UlSchedulingTestSuite()
    : TestSuite("nr-ul-scheduling-test", Type::SYSTEM)
{
    // The UE starts from position 60 and moves along the Y-axis at a speed of 5 m/s. It transmits a
    // packet every 2 seconds. After 10.5 seconds, it begins moving back toward the gNB
    AddTestCase(
        new UlSchedulingTest(1, MilliSeconds(10500), false, 60, Seconds(20), 5, Seconds(2), 1250),
        Duration::QUICK);

    AddTestCase(
        new UlSchedulingTest(2, MilliSeconds(10500), true, 60, Seconds(20), 5, Seconds(2), 1250),
        Duration::QUICK);
}

/**
 * @ingroup nr-test
 * Static variable for test initialization
 */
static UlSchedulingTestSuite m_UlSchedulingTestSuite; //!< Nr test suite

UlSchedulingTest::UlSchedulingTest(uint8_t testNum,
                                   Time reverseTime,
                                   bool harqActive,
                                   uint32_t startUEPosY,
                                   Time simTime,
                                   double speed,
                                   Time packetPeriod,
                                   uint32_t packetSize)
    : TestCase("UL transmissions Test Case")
{
    m_testNumber = testNum;
    m_reverseTime = reverseTime;
    m_harqActive = harqActive;
    m_startUEPosY = startUEPosY;
    m_simTime = simTime;
    m_speed = speed;
    m_packetPeriod = packetPeriod;
    m_packetSize = packetSize;
}

UlSchedulingTest::~UlSchedulingTest()
{
}

void
UlSchedulingTest::ShowScheduledNextPacketTransmission(Ptr<Node> ue, uint32_t ueNum)
{
    Vector currentPosition = ue->GetObject<MobilityModel>()->GetPosition();
    m_nextTime += m_packetPeriod;
    NS_LOG_INFO("Current position =" << currentPosition
                                     << " and Next packet transmission time = " << m_nextTime);
    Simulator::Schedule(m_packetPeriod,
                        &UlSchedulingTest::ShowScheduledNextPacketTransmission,
                        this,
                        ue,
                        ueNum);
}

void
UlSchedulingTest::ReverseUeDirection(Ptr<Node> ueNode)
{
    NS_LOG_FUNCTION(this);
    ueNode->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector(0, -m_speed, 0));
}

void
UlSchedulingTest::CreateAndStoreFileForResults(
    const std::string& basePath,
    uint16_t rnti,
    SfnSf sfn,
    std::string srState,
    std::unordered_map<uint8_t, NrMacSapProvider::BufferStatusReportParameters> m_ulBsrReceived)
{
    fs::path testUlTxPath = fs::path(basePath) / "results" / "test_ulTx";
    fs::create_directories(testUlTxPath);

    fs::path filePath = testUlTxPath / ("test" + std::to_string(m_testNumber) + "_" +
                                        std::to_string(rnti) + ".txt");

    // True if it is the first time of creating the file for current testNumber and rnti
    bool firstTime = m_storedRntis.find(rnti) == m_storedRntis.end() &&
                     m_storedTestNum.find(m_testNumber) == m_storedTestNum.end();
    if (firstTime)
    {
        m_storedTestNum.insert(m_testNumber);
        m_storedRntis.insert(rnti);
        if (fs::exists(filePath))
        {
            fs::remove(filePath);
        }
    }

    std::ofstream file(filePath, std::ios::app);
    if (!file)
    {
        std::cerr << "Error (can't create the file)" << filePath << std::endl;
        return;
    }

    if (firstTime)
    {
        file << "Sfnsf\t\t\t\t state\t\t\t Queue UL DATA\n";
    }

    for (auto it = m_ulBsrReceived.begin(); it != m_ulBsrReceived.end(); it++)
    {
        m_txQueue = (*it).second.txQueueSize;
    }

    if (m_ulSfn.find(rnti) != m_ulSfn.end())
    {
        SfnSf lastSfn = m_ulSfn[rnti];
        // period in slots (1 frame = 10ms, consisting of 10 sub-frames, with, in this case, each
        // sub-frame containing 1 slot; thus, there are 10 slots per frame)
        uint32_t Nslot = (m_packetPeriod.GetMilliSeconds() / 10) * 10 / 10;
        lastSfn.Add(Nslot);
        if (lastSfn < sfn)
        {
            file << "\n"; // Separate output data for each packet
        }
    }
    m_ulSfn[rnti] = sfn;
    file << sfn << "\t UE:" << srState << "\t" << m_txQueue << "\n";
}

void
UlSchedulingTest::UeMacStateMachine(
    SfnSf sfn,
    [[maybe_unused]] uint16_t nodeId,
    uint16_t rnti,
    [[maybe_unused]] uint8_t ccId,
    NrUeMac::SrBsrMachine srState,
    std::unordered_map<uint8_t, NrMacSapProvider::BufferStatusReportParameters> m_ulBsrReceived,
    int retxActive,
    std::string funcName)
{
    std::string basePath = "build/contrib/nr";
    std::string state = "INACTIVE";
    Time grantRxTime = MilliSeconds(10);
    if (srState == 0)
    {
        // TODO The UE transmits a BSR every time it receives a grant, even if it has no more data
        // to send. Therefore, when the gNB sends a grant that empties the UE's buffer, the UE
        // changes to the INACTIVE state but still transmits a BSR.
        // This behavior should be considered erroneous, as the UE is already in INACTIVE and should
        // be waiting to receive a new message instead.
        state = (funcName == "SendBufferStatusReport") ? "INACTIVE- Send BSR (ERROR)" : "INACTIVE";
        m_countHarq = 0;
    }
    else if (srState == 1)
    {
        state = "TO_SEND";
    }
    else
    {
        if (retxActive == 0)
        {
            state =
                (funcName == "DoTransmitBufferStatusReport") ? "ACTIVE(ReTxSR)" : "ACTIVE(HARQ)";
            if (state == "ACTIVE(HARQ)")
            {
                Simulator::Schedule(grantRxTime,
                                    &UlSchedulingTest::CheckGrantRxState,
                                    this,
                                    sfn,
                                    rnti);
                m_countHarq++;
            }
            else
            {
                if (0 < m_countHarq && m_countHarq < 3)
                {
                    NS_TEST_ASSERT_MSG_EQ(false,
                                          true,
                                          "An SR should not be retransmitted if all HARQ "
                                          "retransmissions have not been completed.");
                }
            }
        }
        else
        {
            if (funcName == "DoTransmitBufferStatusReport")
            {
                state = "ACTIVE";
            }
            else if (funcName == "DoSlotIndication")
            {
                state = "ACTIVE(waitGrant)";
            }
            else if (funcName == "SendBufferStatusReport")
            {
                // TODO The UE should send a BSR only if new data has arrived in the buffer and the
                // gNB is unaware of it
                state = "ACTIVE(sendBSR)";
            }
            else
            {
                state = "ACTIVE(grantRX)";
                Simulator::Schedule(grantRxTime,
                                    &UlSchedulingTest::CheckGrantRxState,
                                    this,
                                    sfn,
                                    rnti);
            }
        }
    }
    m_lastSfnSf = sfn;
    m_lastState = state;
    CreateAndStoreFileForResults(basePath, rnti, sfn, state, m_ulBsrReceived);
}

void
UlSchedulingTest::CheckGrantRxState(SfnSf sfn, uint16_t rnti)
{
    std::ofstream file = OpenResultFile(m_testNumber, rnti);
    if (!file)
    {
        return;
    }
    if ((m_lastState == "ACTIVE(sendBSR)" || m_lastState == "ACTIVE(HARQ)") && m_lastSfnSf == sfn &&
        m_txQueue > 0)
    {
        file << m_lastSfnSf << "\t is stuck in " << m_lastState
             << " state for 1 frame duration (ERROR) \n";
        NS_TEST_ASSERT_MSG_EQ(
            false,
            true,
            "The UE remains stuck in the ACTIVE state because the gNB "
            "does not receive the BSR, blocking the UE from obtaining a grant to transmit data.");
    }
    else
    {
        file << m_lastSfnSf << "\t current state is " << m_lastState << " last sfn = " << sfn
             << " and the bufSize is = " << m_txQueue << " \n";
    }
}

std::ofstream
UlSchedulingTest::OpenResultFile(uint16_t testNumber, uint16_t rnti)
{
    std::string basePath = "build/contrib/nr";
    fs::path testUlTxPath = fs::path(basePath) / "results" / "test_ulTx";
    fs::path filePath =
        testUlTxPath / ("test" + std::to_string(testNumber) + "_" + std::to_string(rnti) + ".txt");

    std::ofstream file(filePath, std::ios::app);
    if (!file)
    {
        std::cerr << "Error (can't open the file): " << filePath << std::endl;
    }
    return file;
}

void
UlSchedulingTest::gNBUlToSch(NrSchedulingCallbackInfo data)
{
    std::ofstream file = OpenResultFile(m_testNumber, data.m_rnti);
    if (!file)
    {
        return;
    }

    // TODO The Sfn used to schedule a grant transmission from the gNB to the UE appears later than
    // the moment the UE receives the grant.
    file << "FrameNum: " << data.m_frameNum << " SubFrameNum: " << uint32_t(data.m_subframeNum)
         << " SlotNum:" << data.m_slotNum << "\t gNB:ToSch \t\t " << data.m_tbSize << " \n";
}

void
UlSchedulingTest::gNBRxCtrl(SfnSf sfn,
                            [[maybe_unused]] uint16_t nodeId,
                            uint16_t rnti,
                            [[maybe_unused]] uint8_t ccId,
                            Ptr<const NrControlMessage> msg)
{
    std::ofstream file = OpenResultFile(m_testNumber, rnti);
    if (!file)
    {
        return;
    }

    if (msg->GetMessageType() == NrControlMessage::BSR)
    {
        file << sfn << "\t gNB:RxBSR \n";
    }
    else if (msg->GetMessageType() == NrControlMessage::SR)
    {
        file << sfn << "\t gNB:RxSR \n";
    }
}

void
UlSchedulingTest::DoRun()
{
    auto logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);
    auto logLevel1 =
        (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_INFO);
    auto logLevel2 =
        (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_DEBUG);
    LogComponentEnable("UlSchedulingTestCase", logLevel);
    LogComponentEnable("UlSchedulingTestCase", logLevel1);
    LogComponentEnable("UlSchedulingTestCase", logLevel2);

    /*LogComponentEnable("NrUeMac", logLevel1);
    LogComponentEnable("NrUeMac", logLevel2);
    LogComponentEnable("NrRlcUm", logLevel1);
    LogComponentEnable("NrRlcUm", logLevel2);
    LogComponentEnable("FlowMonitor", logLevel1);
    LogComponentEnable("FlowMonitor", logLevel2);

    LogComponentEnable("NrGnbMac", logLevel1);
    LogComponentEnable("NrGnbMac", logLevel2);
    LogComponentEnable("NrMacSchedulerNs3", logLevel1);
    LogComponentEnable("NrMacSchedulerNs3", logLevel2);*/

    // Simulation parameters //
    Time simTime = m_simTime;
    Time udpAppStartTimeUl = MilliSeconds(500);
    m_nextTime = udpAppStartTimeUl;

    // Create base stations and mobile terminals //
    NodeContainer gNbNode;
    NodeContainer ueNode;
    gNbNode.Create(1);
    ueNode.Create(1);

    // Add Mobility
    Ptr<ConstantPositionMobilityModel> mobility = CreateObject<ConstantPositionMobilityModel>();
    Ptr<Node> gnb = gNbNode.Get(0);
    gnb->AggregateObject(mobility);
    mobility->SetPosition(Vector(0, 0, 10));

    MobilityHelper ueMobility;
    ueMobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    ueMobility.Install(ueNode);
    Ptr<Node> ue = ueNode.Get(0);
    ue->GetObject<MobilityModel>()->SetPosition(Vector(116, m_startUEPosY, 1.5));
    ue->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector(0, m_speed, 0));

    Simulator::Schedule(m_reverseTime, &UlSchedulingTest::ReverseUeDirection, this, ue);
    Simulator::Schedule(m_nextTime + m_packetPeriod,
                        &UlSchedulingTest::ShowScheduledNextPacketTransmission,
                        this,
                        ue,
                        1);

    // Configure BandwidthParts //
    OperationBandInfo band0;
    band0.m_bandId = 0;
    // uint8_t bandMask = NrChannelHelper::INIT_PROPAGATION;
    auto totalBandwidth = 50e6;
    CcBwpCreator ccBwpCreator;
    CcBwpCreator::SimpleOperationBandConf bandConf(28e9, totalBandwidth, 1);
    band0 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);

    // Setup NR //
    Ptr<NrPointToPointEpcHelper> nrEpcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(QuasiOmniDirectPathBeamforming::GetTypeId()));
    nrHelper->SetEpcHelper(nrEpcHelper);

    // Error Model
    std::string errorModel = "ns3::NrEesmIrT2";
    nrHelper->SetUlErrorModel(errorModel);
    nrHelper->SetDlErrorModel(errorModel);

    nrHelper->SetGnbMacAttribute("NumHarqProcess", UintegerValue(m_harqActive ? 16 : 1));
    NS_LOG_INFO("HARQ is enabled? = " << m_harqActive);

    // Setup Channel //
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    channelHelper->ConfigureFactories("RMa", "Default");
    channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
    ObjectFactory distanceBasedChannelFactory;
    distanceBasedChannelFactory.SetTypeId(
        DistanceBasedThreeGppSpectrumPropagationLossModel::GetTypeId());
    auto distanceBased3gpp =
        distanceBasedChannelFactory.Create<DistanceBasedThreeGppSpectrumPropagationLossModel>();
    distanceBased3gpp->SetChannelModelAttribute(
        "Frequency",
        DoubleValue(band0.GetBwpAt(0, 0)->m_centralFrequency));
    distanceBased3gpp->SetChannelModelAttribute("Scenario", StringValue("RMa"));
    auto specChannelBand0 = channelHelper->CreateChannel(NrChannelHelper::INIT_PROPAGATION);
    band0.GetBwpAt(0, 0)->SetChannel(specChannelBand0);

    // Install devices //
    NetDeviceContainer gnbDevices;
    NetDeviceContainer ueDevices;

    auto allBwps = CcBwpCreator::GetAllBwps({band0});

    NetDeviceContainer gnbDevice = nrHelper->InstallGnbDevice(gNbNode, allBwps);
    gnbDevices.Add(gnbDevice);
    NrHelper::GetGnbPhy(gnbDevices.Get(0), 0)->SetAttribute("TxPower", DoubleValue(35));

    NetDeviceContainer ueDevice = nrHelper->InstallUeDevice(ueNode, allBwps);
    ueDevices.Add(ueDevice);

    // Setup Internet //
    Ipv4InterfaceContainer ueVoiceIpIface;

    Ptr<Node> pgw = nrEpcHelper->GetPgwNode();
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);

    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    // connect a remoteHost to pgw. Setup routing too
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(2500));
    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.000)));
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);
    internet.Install(ueNode);

    ueVoiceIpIface = nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevices));

    // Attach UEs to the closest gNB //
    nrHelper->AttachToClosestGnb(ueDevices, gnbDevices);

    // Configure Traffic //
    Ptr<NrEpcTft> voiceTft = Create<NrEpcTft>();
    UdpClientHelper ulClient;
    ApplicationContainer serverApps;
    ApplicationContainer clientApps;

    // UL data
    uint16_t ulPort = 20000;

    // The server, that is the application which
    // is listening, is installed in the remote host (UL)
    UdpServerHelper ulPacket(ulPort);
    serverApps.Add(ulPacket.Install(remoteHost));

    // Voice configuration and object creation:
    ulClient.SetAttribute("MaxPackets", UintegerValue(1000));
    ulClient.SetAttribute("Interval", TimeValue(m_packetPeriod)); // m_packetPeriod
    ulClient.SetAttribute("PacketSize", UintegerValue(m_packetSize));

    // The filter for the UL traffic (if it is DL this would be localPort)
    NrEpcTft::PacketFilter ulpf;
    ulpf.remotePortStart = ulPort;
    ulpf.remotePortEnd = ulPort;
    voiceTft->Add(ulpf);

    // The client, who is transmitting, is installed in the UE (UL data),
    // with destination address set to the address of the remoteHost
    ulClient.SetAttribute(
        "Remote",
        AddressValue(addressUtils::ConvertToSocketAddress(remoteHostAddr, ulPort)));
    clientApps.Add(ulClient.Install(ueNode.Get(0)));

    // Activate a dedicated bearer for the traffic type
    // The bearer that will carry voice traffic
    NrEpsBearer voiceBearer(NrEpsBearer::GBR_CONV_VOICE);
    nrHelper->ActivateDedicatedEpsBearer(ueDevices.Get(0), voiceBearer, voiceTft);

    serverApps.Start(udpAppStartTimeUl);
    clientApps.Start(udpAppStartTimeUl);
    serverApps.Stop(simTime);
    clientApps.Stop(simTime);

    FlowMonitorHelper flowmonHelper;
    NodeContainer endpointNodes;
    endpointNodes.Add(remoteHost);
    endpointNodes.Add(ueNode);

    Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install(endpointNodes);
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

    // UE MAC state machine trace
    NrHelper::GetUeMac(ueDevices.Get(0), 0)
        ->TraceConnectWithoutContext("UeMacStateMachineTrace",
                                     MakeCallback(&UlSchedulingTest::UeMacStateMachine, this));
    // gNB MAC info traces
    NrHelper::GetGnbMac(gnbDevices.Get(0), 0)
        ->TraceConnectWithoutContext("UlScheduling",
                                     MakeCallback(&UlSchedulingTest::gNBUlToSch, this));
    NrHelper::GetGnbMac(gnbDevices.Get(0), 0)
        ->TraceConnectWithoutContext("GnbMacRxedCtrlMsgsTrace",
                                     MakeCallback(&UlSchedulingTest::gNBRxCtrl, this));

    nrHelper->EnableTraces();

    Simulator::Stop(simTime);
    Simulator::Run();

    // Print per-flow statistics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    double averageFlowThroughput = 0.0;
    double averageFlowDelay = 0.0;

    std::ofstream outFile;
    std::string simTag = "debug_UlSchedulingTest";
    std::string outputDir = "./";
    std::string filename = outputDir + "/" + simTag;
    outFile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (!outFile.is_open())
    {
        std::cerr << "Can't open file " << filename << std::endl;
    }

    outFile.setf(std::ios_base::fixed);

    double flowDuration = (simTime - udpAppStartTimeUl).GetSeconds();
    for (auto i = stats.begin(); i != stats.end(); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
        std::stringstream protoStream;
        protoStream << (uint16_t)t.protocol;
        if (t.protocol == 6)
        {
            protoStream.str("TCP");
        }
        if (t.protocol == 17)
        {
            protoStream.str("UDP");
        }
        outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> "
                << t.destinationAddress << ":" << t.destinationPort << ") proto "
                << protoStream.str() << "\n";
        outFile << "  Tx Packets: " << i->second.txPackets << "\n";
        outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
        outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / flowDuration / 1000.0 / 1000.0
                << " Mbps\n";
        outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        if (i->second.rxPackets > 0)
        {
            // Measure the duration of the flow from receiver's perspective
            averageFlowThroughput += i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;
            averageFlowDelay += 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets;

            outFile << "  Throughput: " << i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000
                    << " Mbps\n";
            outFile << "  Mean delay:  "
                    << 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets << " ms\n";
            // outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << "
            // Mbps \n";
            outFile << "  Mean jitter:  "
                    << 1000 * i->second.jitterSum.GetSeconds() / i->second.rxPackets << " ms\n";
        }
        else
        {
            outFile << "  Throughput:  0 Mbps\n";
            outFile << "  Mean delay:  0 ms\n";
            outFile << "  Mean jitter: 0 ms\n";
        }
        outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
    }

    double meanFlowThroughput = averageFlowThroughput / stats.size();
    double meanFlowDelay = averageFlowDelay / stats.size();

    outFile << "\n\n  Mean flow throughput: " << meanFlowThroughput << "\n";
    outFile << "  Mean flow delay: " << meanFlowDelay << "\n";

    outFile.close();
    std::ifstream f(filename.c_str());
    if (f.is_open())
    {
        std::cout << f.rdbuf();
    }

    if (meanFlowThroughput == 0)
    {
        NS_TEST_ASSERT_MSG_EQ(false, true, "Some packets have to be received");
    }

    Simulator::Destroy();
}

} // namespace ns3
