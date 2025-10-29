/*
 * Copyright (c) 2011-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors:
 *   Nicola Baldo <nbaldo@cttc.es>
 *   Manuel Requena <manuel.requena@cttc.es>
 */

#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/log.h"
#include "ns3/nr-qos-rule-classifier.h"
#include "ns3/packet.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/test.h"
#include "ns3/udp-header.h"
#include "ns3/udp-l4-protocol.h"

#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("NrTestQosRuleClassifier");

/**
 * @ingroup nr-test
 *
 * @brief Test case to check the functionality of the QoS rule Classifier. Test
 * consist of defining different QoS rule configurations, i.e. direction, ports,
 * address, and it is checking if the clasiffication of UDP packets is
 * done correctly.
 */
class NrQosRuleClassifierTestCase : public TestCase
{
  public:
    /**
     * Constructor
     *
     * @param c the QoS rule classifier
     * @param d the QoS rule direction
     * @param sa the source address (in IPv4 format)
     * @param da the destination address (in IPv4 format)
     * @param sp the source port
     * @param dp the destination port
     * @param tos the TOS
     * @param ruleId the QoS rule ID
     * @param useIpv6 use IPv6 or IPv4 addresses. If set, addresses will be used as IPv4 mapped
     * addresses
     */
    NrQosRuleClassifierTestCase(Ptr<NrQosRuleClassifier> c,
                                NrQosRule::Direction d,
                                std::string sa,
                                std::string da,
                                uint16_t sp,
                                uint16_t dp,
                                uint8_t tos,
                                uint32_t ruleId,
                                bool useIpv6);

    ~NrQosRuleClassifierTestCase() override;

  private:
    Ptr<NrQosRuleClassifier> m_c; ///< the QoS rule classifier
    NrQosRule::Direction m_d;     ///< the QoS rule direction
    uint8_t m_ruleId;             ///< the QoS rule ID
    bool m_useIpv6;               ///< use IPv4 or IPv6 header/addresses
    Ipv4Header m_ipHeader;        ///< the IPv4 header
    Ipv6Header m_ipv6Header;      ///< the IPv6 header
    UdpHeader m_udpHeader;        ///< the UDP header
    TcpHeader m_tcpHeader;        ///< the TCP header

    /**
     * Build name string
     * @param c the QoS rule classifier
     * @param d the QoS rule direction
     * @param sa the source address
     * @param da the destination address
     * @param sp the source port
     * @param dp the destination port
     * @param tos the TOS
     * @param ruleId the QoS rule ID
     * @param useIpv6 use IPv6 or IPv4 addresses. If set, addresses will be used as IPv4
     * mapped addresses
     * @returns the name string
     */
    static std::string BuildNameString(Ptr<NrQosRuleClassifier> c,
                                       NrQosRule::Direction d,
                                       std::string sa,
                                       std::string da,
                                       uint16_t sp,
                                       uint16_t dp,
                                       uint8_t tos,
                                       uint32_t ruleId,
                                       bool useIpv6);

    void DoRun() override;
};

NrQosRuleClassifierTestCase::NrQosRuleClassifierTestCase(Ptr<NrQosRuleClassifier> c,
                                                         NrQosRule::Direction d,
                                                         std::string sa,
                                                         std::string da,
                                                         uint16_t sp,
                                                         uint16_t dp,
                                                         uint8_t tos,
                                                         uint32_t ruleId,
                                                         bool useIpv6)
    : TestCase(BuildNameString(c, d, sa, da, sp, dp, tos, ruleId, useIpv6)),
      m_c(c),
      m_d(d),
      m_ruleId(ruleId),
      m_useIpv6(useIpv6)
{
    NS_LOG_FUNCTION(this << c << d << sa << da << sp << dp << tos << ruleId << useIpv6);

    if (m_useIpv6)
    {
        m_ipv6Header.SetSource(Ipv6Address::MakeIpv4MappedAddress(Ipv4Address(sa.c_str())));
        m_ipv6Header.SetDestination(Ipv6Address::MakeIpv4MappedAddress(Ipv4Address(da.c_str())));
        m_ipv6Header.SetTrafficClass(tos);
        m_ipv6Header.SetPayloadLength(8); // Full UDP header
        m_ipv6Header.SetNextHeader(UdpL4Protocol::PROT_NUMBER);
    }
    else
    {
        m_ipHeader.SetSource(Ipv4Address(sa.c_str()));
        m_ipHeader.SetDestination(Ipv4Address(da.c_str()));
        m_ipHeader.SetTos(tos);
        m_ipHeader.SetPayloadSize(8); // Full UDP header
        m_ipHeader.SetProtocol(UdpL4Protocol::PROT_NUMBER);
    }

    m_udpHeader.SetSourcePort(sp);
    m_udpHeader.SetDestinationPort(dp);
}

NrQosRuleClassifierTestCase::~NrQosRuleClassifierTestCase()
{
}

std::string
NrQosRuleClassifierTestCase::BuildNameString(Ptr<NrQosRuleClassifier> c,
                                             NrQosRule::Direction d,
                                             std::string sa,
                                             std::string da,
                                             uint16_t sp,
                                             uint16_t dp,
                                             uint8_t tos,
                                             uint32_t ruleId,
                                             bool useIpv6)
{
    std::ostringstream oss;
    oss << c << "  d = " << d;
    if (useIpv6)
    {
        oss << ", sa = " << Ipv6Address::MakeIpv4MappedAddress(Ipv4Address(sa.c_str()))
            << ", da = " << Ipv6Address::MakeIpv4MappedAddress(Ipv4Address(da.c_str()));
    }
    else
    {
        oss << ", sa = " << sa << ", da = " << da;
    }
    oss << ", sp = " << sp << ", dp = " << dp << ", tos = 0x" << std::hex << (int)tos
        << " --> ruleId = " << ruleId;
    return oss.str();
}

void
NrQosRuleClassifierTestCase::DoRun()
{
    ns3::PacketMetadata::Enable();

    Ptr<Packet> udpPacket = Create<Packet>();
    udpPacket->AddHeader(m_udpHeader);
    if (m_useIpv6)
    {
        udpPacket->AddHeader(m_ipv6Header);
    }
    else
    {
        udpPacket->AddHeader(m_ipHeader);
    }
    NS_LOG_LOGIC(this << *udpPacket);
    uint32_t obtainedRuleId =
        m_c->Classify(udpPacket,
                      m_d,
                      m_useIpv6 ? Ipv6L3Protocol::PROT_NUMBER : Ipv4L3Protocol::PROT_NUMBER);
    NS_TEST_ASSERT_MSG_EQ(obtainedRuleId, (uint16_t)m_ruleId, "bad classification of UDP packet");
}

/**
 * @ingroup nr-test
 *
 * @brief QoS Rule Classifier Test Suite
 */
class NrQosRuleClassifierTestSuite : public TestSuite
{
  public:
    NrQosRuleClassifierTestSuite();
};

/**
 * @ingroup nr-test
 * Static variable for test initialization
 */
static NrQosRuleClassifierTestSuite g_nrQosRuleClassifierTestSuite;

NrQosRuleClassifierTestSuite::NrQosRuleClassifierTestSuite()
    : TestSuite("nr-qos-rule-classifier", Type::UNIT)
{
    NS_LOG_FUNCTION(this);

    /////////////////////////////////////////////////////////////////////////////////
    // Same testcases using IPv4 and IPv6 addresses
    // IPv6 addresses are IPv4 mapped addresses, i.e. 1.2.3.4 -> 0::ffff:1.2.3.4
    // Currently, we use the format '0::ffff:0102:0304' because
    // the format '0::ffff:1.2.3.4' is not supported by the Ipv6Address class
    /////////////////////////////////////////////////////////////////////////////////

    for (bool useIpv6 : {false, true})
    {
        //////////////////////////
        // check some rule matches
        //////////////////////////

        Ptr<NrQosRuleClassifier> c1 = Create<NrQosRuleClassifier>();

        Ptr<NrQosRule> rule1_1 = Create<NrQosRule>();

        NrQosRule::PacketFilter pf1_1_1;
        if (useIpv6)
        {
            pf1_1_1.remoteIpv6Address.Set("0::ffff:0100:0000");
            pf1_1_1.remoteIpv6Prefix = Ipv6Prefix(96 + 8);
            pf1_1_1.localIpv6Address.Set("0::ffff:0200:0000");
            pf1_1_1.localIpv6Prefix = Ipv6Prefix(96 + 8);
        }
        else
        {
            pf1_1_1.remoteAddress.Set("1.0.0.0");
            pf1_1_1.remoteMask.Set(0xff000000);
            pf1_1_1.localAddress.Set("2.0.0.0");
            pf1_1_1.localMask.Set(0xff000000);
        }
        rule1_1->Add(pf1_1_1);

        NrQosRule::PacketFilter pf1_1_2;
        if (useIpv6)
        {
            pf1_1_2.remoteIpv6Address.Set("0::ffff:0303:0300");
            pf1_1_2.remoteIpv6Prefix = Ipv6Prefix(96 + 24);
            pf1_1_2.localIpv6Address.Set("0::ffff:0404:0400");
            pf1_1_2.localIpv6Prefix = Ipv6Prefix(96 + 24);
        }
        else
        {
            pf1_1_2.remoteAddress.Set("3.3.3.0");
            pf1_1_2.remoteMask.Set(0xffffff00);
            pf1_1_2.localAddress.Set("4.4.4.0");
            pf1_1_2.localMask.Set(0xffffff00);
        }
        rule1_1->Add(pf1_1_2);

        c1->Add(rule1_1, 1);

        Ptr<NrQosRule> rule1_2 = Create<NrQosRule>();

        NrQosRule::PacketFilter pf1_2_1;
        pf1_2_1.remotePortStart = 1024;
        pf1_2_1.remotePortEnd = 1035;
        rule1_2->Add(pf1_2_1);

        NrQosRule::PacketFilter pf1_2_2;
        pf1_2_2.localPortStart = 3456;
        pf1_2_2.localPortEnd = 3489;
        rule1_2->Add(pf1_2_2);

        NrQosRule::PacketFilter pf1_2_3;
        pf1_2_3.localPortStart = 7895;
        pf1_2_3.localPortEnd = 7895;
        rule1_2->Add(pf1_2_3);

        NrQosRule::PacketFilter pf1_2_4;
        pf1_2_4.remotePortStart = 5897;
        pf1_2_4.remotePortEnd = 5897;
        rule1_2->Add(pf1_2_4);

        c1->Add(rule1_2, 2);

        // --------------------------------classifier----direction-------src_addr---dst_addr--src_port--dst_port--ToS--rule_id

        // test IP addresses
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "2.2.3.4",
                                                    "1.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "2.2.3.4",
                                                    "1.0.0.0",
                                                    2,
                                                    123,
                                                    5,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "6.2.3.4",
                                                    "1.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "3.3.3.4",
                                                    "4.4.4.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "3.3.4.4",
                                                    "4.4.4.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "3.3.3.4",
                                                    "4.4.2.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        // test remote port
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1024,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1025,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1035,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1024,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1025,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1035,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        // test local port
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3456,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3457,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3489,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    3456,
                                                    6,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    3461,
                                                    3461,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    9,
                                                    3489,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    9,
                                                    7895,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    7895,
                                                    10,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    9,
                                                    5897,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c1,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    5897,
                                                    10,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        ///////////////////////////
        // check default rule
        ///////////////////////////

        Ptr<NrQosRuleClassifier> c2 = Create<NrQosRuleClassifier>();
        c2->Add(NrQosRule::Default(), 1);

        // --------------------------------classifier---direction--------src_addr---dst_addr--src_port--dst_port--ToS--rule
        // id

        // test IP addresses
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "2.2.3.4",
                                                    "1.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "2.2.3.4",
                                                    "1.0.0.0",
                                                    2,
                                                    123,
                                                    5,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "6.2.3.4",
                                                    "1.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::DOWNLINK,
                                                    "3.3.3.4",
                                                    "4.4.4.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::DOWNLINK,
                                                    "3.3.4.4",
                                                    "4.4.4.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "3.3.3.4",
                                                    "4.4.2.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        // test remote port
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1024,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1025,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1035,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1024,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1025,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1035,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        // test local port
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3456,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3457,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3489,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    3456,
                                                    6,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    3461,
                                                    3461,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c2,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    9,
                                                    3489,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        ///////////////////////////////////////////
        // check default rule plus dedicated ones
        ///////////////////////////////////////////

        Ptr<NrQosRuleClassifier> c3 = Create<NrQosRuleClassifier>();
        c3->Add(NrQosRule::Default(), 1);
        c3->Add(rule1_1, 2);
        c3->Add(rule1_2, 3);

        // --------------------------------classifier---direction--------src_addr---dst_addr---src_port--dst_port--ToS--rule_id

        // test IP addresses
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "2.2.3.4",
                                                    "1.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "2.2.3.4",
                                                    "1.0.0.0",
                                                    2,
                                                    123,
                                                    5,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "6.2.3.4",
                                                    "1.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::DOWNLINK,
                                                    "3.3.3.4",
                                                    "4.4.4.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::DOWNLINK,
                                                    "3.3.4.4",
                                                    "4.4.4.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "3.3.3.4",
                                                    "4.4.2.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        // test remote port
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1024,
                                                    0,
                                                    3,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1025,
                                                    0,
                                                    3,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1035,
                                                    0,
                                                    3,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1234,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1024,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1025,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    1035,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        // test local port
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3456,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3457,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    4,
                                                    3489,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    3456,
                                                    6,
                                                    0,
                                                    3,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    3461,
                                                    3461,
                                                    0,
                                                    3,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c3,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    9,
                                                    3489,
                                                    0,
                                                    3,
                                                    useIpv6),
                    TestCase::Duration::QUICK);

        ///////////////////////////////////////////
        // check two rules with different ports
        ///////////////////////////////////////////

        Ptr<NrQosRuleClassifier> c4 = Create<NrQosRuleClassifier>();
        Ptr<NrQosRule> rule4_1 = Create<NrQosRule>();
        rule4_1->Add(pf1_2_3);
        c4->Add(rule4_1, 1);
        Ptr<NrQosRule> rule4_2 = Create<NrQosRule>();
        rule4_2->Add(pf1_2_4);
        c4->Add(rule4_2, 2);
        AddTestCase(new NrQosRuleClassifierTestCase(c4,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    9,
                                                    3489,
                                                    0,
                                                    0,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c4,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    9,
                                                    7895,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c4,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    7895,
                                                    10,
                                                    0,
                                                    1,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c4,
                                                    NrQosRule::UPLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    9,
                                                    5897,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
        AddTestCase(new NrQosRuleClassifierTestCase(c4,
                                                    NrQosRule::DOWNLINK,
                                                    "9.1.1.1",
                                                    "8.1.1.1",
                                                    5897,
                                                    10,
                                                    0,
                                                    2,
                                                    useIpv6),
                    TestCase::Duration::QUICK);
    }
}
