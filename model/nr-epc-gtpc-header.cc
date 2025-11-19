// Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#include "nr-epc-gtpc-header.h"

#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrGtpcHeader");

NS_OBJECT_ENSURE_REGISTERED(NrGtpcHeader);

/// GTPv2-C protocol version number
static const uint8_t VERSION = 2;

TypeId
NrGtpcHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcHeader")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcHeader>();
    return tid;
}

NrGtpcHeader::NrGtpcHeader()
    : m_teidFlag(false),
      m_messageType(0),
      m_messageLength(4),
      m_teid(0),
      m_sequenceNumber(0)
{
}

NrGtpcHeader::~NrGtpcHeader()
{
}

TypeId
NrGtpcHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcHeader::GetSerializedSize() const
{
    return m_teidFlag ? 12 : 8;
}

void
NrGtpcHeader::Serialize(Buffer::Iterator start) const
{
    NS_FATAL_ERROR("Serialize GTP-C header is forbidden");
}

void
NrGtpcHeader::PreSerialize(Buffer::Iterator& i) const
{
    i.WriteU8((VERSION << 5) | (1 << 3));
    i.WriteU8(m_messageType);
    i.WriteHtonU16(m_messageLength);
    i.WriteHtonU32(m_teid);
    i.WriteU8((m_sequenceNumber & 0x00ff0000) >> 16);
    i.WriteU8((m_sequenceNumber & 0x0000ff00) >> 8);
    i.WriteU8(m_sequenceNumber & 0x000000ff);
    i.WriteU8(0);
}

uint32_t
NrGtpcHeader::Deserialize(Buffer::Iterator start)
{
    return PreDeserialize(start);
}

uint32_t
NrGtpcHeader::PreDeserialize(Buffer::Iterator& i)
{
    uint8_t firstByte = i.ReadU8();
    uint8_t version = (firstByte >> 5) & 0x07;
    if (version != 2)
    {
        NS_FATAL_ERROR("GTP-C version not supported");
        return 0;
    }

    m_teidFlag = ((firstByte >> 3) & 0x01) == 1;
    if (!m_teidFlag)
    {
        NS_FATAL_ERROR("TEID is missing");
        return 0;
    }

    m_messageType = i.ReadU8();
    m_messageLength = i.ReadNtohU16();
    if (m_teidFlag)
    {
        m_teid = i.ReadNtohU32();
    }
    m_sequenceNumber = i.ReadU8() << 16 | i.ReadU8() << 8 | i.ReadU8();
    i.ReadU8();

    return NrGtpcHeader::GetSerializedSize();
}

void
NrGtpcHeader::Print(std::ostream& os) const
{
    os << " messageType " << (uint32_t)m_messageType << " messageLength " << m_messageLength;
    os << " TEID " << m_teid << " sequenceNumber " << m_sequenceNumber;
}

uint32_t
NrGtpcHeader::GetMessageSize() const
{
    return 0;
}

uint8_t
NrGtpcHeader::GetMessageType() const
{
    return m_messageType;
}

uint16_t
NrGtpcHeader::GetMessageLength() const
{
    return m_messageLength;
}

uint32_t
NrGtpcHeader::GetTeid() const
{
    return m_teid;
}

uint32_t
NrGtpcHeader::GetSequenceNumber() const
{
    return m_sequenceNumber;
}

void
NrGtpcHeader::SetMessageType(uint8_t messageType)
{
    m_messageType = messageType;
}

void
NrGtpcHeader::SetMessageLength(uint16_t messageLength)
{
    m_messageLength = messageLength;
}

void
NrGtpcHeader::SetTeid(uint32_t teid)
{
    m_teidFlag = true;
    m_teid = teid;
    m_messageLength = m_teidFlag ? 8 : 4;
}

void
NrGtpcHeader::SetSequenceNumber(uint32_t sequenceNumber)
{
    m_sequenceNumber = sequenceNumber;
}

void
NrGtpcHeader::SetIesLength(uint16_t iesLength)
{
    m_messageLength = iesLength;
    m_messageLength += (m_teidFlag) ? 8 : 4;
}

void
NrGtpcHeader::ComputeMessageLength()
{
    SetIesLength(GetMessageSize());
}

/////////////////////////////////////////////////////////////////////

void
NrGtpcIes::SerializeImsi(Buffer::Iterator& i, uint64_t imsi) const
{
    i.WriteU8(1);      // IE Type = IMSI
    i.WriteHtonU16(8); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteHtonU64(imsi);
}

uint32_t
NrGtpcIes::DeserializeImsi(Buffer::Iterator& i, uint64_t& imsi) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 1, "Wrong IMSI IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 8, "Wrong IMSI IE length");
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong IMSI IE instance");
    imsi = i.ReadNtohU64();

    return serializedSizeImsi;
}

void
NrGtpcIes::SerializeCause(Buffer::Iterator& i, Cause_t cause) const
{
    i.WriteU8(2);      // IE Type = Cause
    i.WriteHtonU16(2); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteU8(cause);  // Cause value
    i.WriteU8(0);      // Spare + CS
}

uint32_t
NrGtpcIes::DeserializeCause(Buffer::Iterator& i, Cause_t& cause) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 2, "Wrong Cause IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 2, "Wrong Cause IE length");
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong Cause IE instance");
    cause = Cause_t(i.ReadU8());
    i.ReadU8();

    return serializedSizeCause;
}

void
NrGtpcIes::SerializeQfi(Buffer::Iterator& i, uint8_t qfi) const
{
    i.WriteU8(73);     // IE Type = QoS Flow ID
    i.WriteHtonU16(1); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteU8(qfi & 0x0f);
}

uint32_t
NrGtpcIes::DeserializeQfi(Buffer::Iterator& i, uint8_t& qfi) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 73, "Wrong EBI IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 1, "Wrong EBI IE length");
    uint8_t instance = i.ReadU8();
    NS_ASSERT_MSG(instance == 0, "Wrong EBI IE instance");
    qfi = i.ReadU8() & 0x0f;

    return serializedSizeQfi;
}

void
NrGtpcIes::WriteHtonU40(Buffer::Iterator& i, uint64_t data) const
{
    i.WriteU8((data >> 32) & 0xff);
    i.WriteU8((data >> 24) & 0xff);
    i.WriteU8((data >> 16) & 0xff);
    i.WriteU8((data >> 8) & 0xff);
    i.WriteU8((data >> 0) & 0xff);
}

uint64_t
NrGtpcIes::ReadNtohU40(Buffer::Iterator& i)
{
    uint64_t retval = 0;
    retval |= i.ReadU8();
    retval <<= 8;
    retval |= i.ReadU8();
    retval <<= 8;
    retval |= i.ReadU8();
    retval <<= 8;
    retval |= i.ReadU8();
    retval <<= 8;
    retval |= i.ReadU8();
    return retval;
}

void
NrGtpcIes::SerializeQosFlow(Buffer::Iterator& i, NrQosFlow flow) const
{
    i.WriteU8(80);      // IE Type = QoS Flow
    i.WriteHtonU16(22); // Length
    i.WriteU8(0);       // Spare + Instance
    i.WriteU8(0);       // MRE TODO: flow.arp
    i.WriteU8(flow.fiveQi);
    WriteHtonU40(i, flow.gbrQosInfo.mbrUl);
    WriteHtonU40(i, flow.gbrQosInfo.mbrDl);
    WriteHtonU40(i, flow.gbrQosInfo.gbrUl);
    WriteHtonU40(i, flow.gbrQosInfo.gbrDl);
}

uint32_t
NrGtpcIes::DeserializeQosFlow(Buffer::Iterator& i, NrQosFlow& flow)
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 80, "Wrong QoS Flow IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 22, "Wrong QoS Flow IE length");
    uint8_t instance = i.ReadU8();
    NS_ASSERT_MSG(instance == 0, "Wrong QoS Flow IE instance");
    i.ReadU8();
    flow.fiveQi = NrQosFlow::FiveQi(i.ReadU8());
    flow.gbrQosInfo.mbrUl = ReadNtohU40(i);
    flow.gbrQosInfo.mbrDl = ReadNtohU40(i);
    flow.gbrQosInfo.gbrUl = ReadNtohU40(i);
    flow.gbrQosInfo.gbrDl = ReadNtohU40(i);
    return serializedSizeQosFlow;
}

void
NrGtpcIes::SerializeQosRule(Buffer::Iterator& i, Ptr<const NrQosRule> rule) const
{
    std::list<NrQosRule::PacketFilter> packetFilters = rule->GetPacketFilters();
    i.WriteU8(84); // IE Type = QoS rule
    i.WriteHtonU16(1 + packetFilters.size() * serializedSizePacketFilter);
    i.WriteU8(0); // Spare + Instance
    i.WriteU8(rule->GetPrecedence());
    i.WriteU8(rule->GetQfi());
    i.WriteU8(0x20 + (packetFilters.size() & 0x0f)); // Create new rule + Number of packet filters

    for (auto& pf : packetFilters)
    {
        i.WriteU8((pf.direction << 4) & 0x30);
        i.WriteU8(serializedSizePacketFilter - 2); // Length of Packet filter contents

        i.WriteU8(0x10); // IPv4 remote address type
        i.WriteHtonU32(pf.remoteAddress.Get());
        i.WriteHtonU32(pf.remoteMask.Get());
        i.WriteU8(0x11); // IPv4 local address type
        i.WriteHtonU32(pf.localAddress.Get());
        i.WriteHtonU32(pf.localMask.Get());
        i.WriteU8(0x41); // Local port range type
        i.WriteHtonU16(pf.localPortStart);
        i.WriteHtonU16(pf.localPortEnd);
        i.WriteU8(0x51); // Remote port range type
        i.WriteHtonU16(pf.remotePortStart);
        i.WriteHtonU16(pf.remotePortEnd);
        i.WriteU8(0x70); // Type of service
        i.WriteU8(pf.typeOfService);
        i.WriteU8(pf.typeOfServiceMask);
    }
}

uint32_t
NrGtpcIes::DeserializeQosRule(Buffer::Iterator& i, Ptr<NrQosRule> rule) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 84, "Wrong QoS rule IE type = " << (uint16_t)type);
    i.ReadNtohU16();
    i.ReadU8();
    rule->SetPrecedence(i.ReadU8());
    rule->SetQfi(i.ReadU8());
    uint8_t numberOfPacketFilters = i.ReadU8() & 0x0f;

    for (uint8_t pf = 0; pf < numberOfPacketFilters; ++pf)
    {
        NrQosRule::PacketFilter packetFilter;
        packetFilter.direction = NrQosRule::Direction((i.ReadU8() & 0x30) >> 4);
        i.ReadU8(); // Length of Packet filter contents
        i.ReadU8();
        packetFilter.remoteAddress = Ipv4Address(i.ReadNtohU32());
        packetFilter.remoteMask = Ipv4Mask(i.ReadNtohU32());
        i.ReadU8();
        packetFilter.localAddress = Ipv4Address(i.ReadNtohU32());
        packetFilter.localMask = Ipv4Mask(i.ReadNtohU32());
        i.ReadU8();
        packetFilter.localPortStart = i.ReadNtohU16();
        packetFilter.localPortEnd = i.ReadNtohU16();
        i.ReadU8();
        packetFilter.remotePortStart = i.ReadNtohU16();
        packetFilter.remotePortEnd = i.ReadNtohU16();
        i.ReadU8();
        packetFilter.typeOfService = i.ReadU8();
        packetFilter.typeOfServiceMask = i.ReadU8();
        rule->Add(packetFilter);
    }

    return GetSerializedSizeQosRule(rule->GetPacketFilters());
}

uint32_t
NrGtpcIes::GetSerializedSizeQosRule(std::list<NrQosRule::PacketFilter> packetFilters) const
{
    return (7 + packetFilters.size() * serializedSizePacketFilter);
}

void
NrGtpcIes::SerializeUliEcgi(Buffer::Iterator& i, uint32_t uliEcgi) const
{
    i.WriteU8(86);     // IE Type = ULI (ECGI)
    i.WriteHtonU16(8); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteU8(0x10);   // ECGI flag
    i.WriteU8(0);      // Dummy MCC and MNC
    i.WriteU8(0);      // Dummy MCC and MNC
    i.WriteU8(0);      // Dummy MCC and MNC
    i.WriteHtonU32(uliEcgi);
}

uint32_t
NrGtpcIes::DeserializeUliEcgi(Buffer::Iterator& i, uint32_t& uliEcgi) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 86, "Wrong ULI ECGI IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 8, "Wrong ULI ECGI IE length");
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong ULI ECGI IE instance");
    i.Next(4);
    uliEcgi = i.ReadNtohU32() & 0x0fffffff;

    return serializedSizeUliEcgi;
}

void
NrGtpcIes::SerializeFteid(Buffer::Iterator& i, NrGtpcHeader::Fteid_t fteid) const
{
    i.WriteU8(87);     // IE Type = Fully Qualified TEID (F-TEID)
    i.WriteHtonU16(9); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteU8(0x80 | ((uint8_t)fteid.interfaceType & 0x1f)); // IP version flag + Iface type
    i.WriteHtonU32(fteid.teid);                              // TEID
    i.WriteHtonU32(fteid.addr.Get());                        // IPv4 address
}

uint32_t
NrGtpcIes::DeserializeFteid(Buffer::Iterator& i, NrGtpcHeader::Fteid_t& fteid) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 87, "Wrong FTEID IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 9, "Wrong FTEID IE length");
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong FTEID IE instance");
    uint8_t flags = i.ReadU8(); // IP version flag + Iface type
    fteid.interfaceType = NrGtpcHeader::InterfaceType_t(flags & 0x1f);
    fteid.teid = i.ReadNtohU32();    // TEID
    fteid.addr.Set(i.ReadNtohU32()); // IPv4 address

    return serializedSizeFteid;
}

void
NrGtpcIes::SerializeFlowContextHeader(Buffer::Iterator& i, uint16_t length) const
{
    i.WriteU8(93); // IE Type = Flow Context
    i.WriteU16(length);
    i.WriteU8(0); // Spare + Instance
}

uint32_t
NrGtpcIes::DeserializeFlowContextHeader(Buffer::Iterator& i, uint16_t& length) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 93, "Wrong Flow Context IE type = " << (uint16_t)type);
    length = i.ReadNtohU16();
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong Flow Context IE instance");

    return serializedSizeFlowContextHeader;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcCreateSessionRequestMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcCreateSessionRequestMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcCreateSessionRequestMessage>();
    return tid;
}

NrGtpcCreateSessionRequestMessage::NrGtpcCreateSessionRequestMessage()
{
    SetMessageType(NrGtpcHeader::CreateSessionRequest);
    SetSequenceNumber(0);
    m_imsi = 0;
    m_uliEcgi = 0;
    m_senderCpFteid = {};
}

NrGtpcCreateSessionRequestMessage::~NrGtpcCreateSessionRequestMessage()
{
}

TypeId
NrGtpcCreateSessionRequestMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcCreateSessionRequestMessage::GetMessageSize() const
{
    uint32_t serializedSize = serializedSizeImsi + serializedSizeUliEcgi + serializedSizeFteid;
    for (auto& bc : m_flowContextsToBeCreated)
    {
        serializedSize += serializedSizeFlowContextHeader + serializedSizeQfi +
                          GetSerializedSizeQosRule(bc.rule->GetPacketFilters()) +
                          serializedSizeFteid + serializedSizeQosFlow;
    }

    return serializedSize;
}

uint32_t
NrGtpcCreateSessionRequestMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcCreateSessionRequestMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeImsi(i, m_imsi);
    SerializeUliEcgi(i, m_uliEcgi);
    SerializeFteid(i, m_senderCpFteid);

    for (auto& bc : m_flowContextsToBeCreated)
    {
        std::list<NrQosRule::PacketFilter> packetFilters = bc.rule->GetPacketFilters();

        SerializeFlowContextHeader(i,
                                   serializedSizeQfi + GetSerializedSizeQosRule(packetFilters) +
                                       serializedSizeFteid + serializedSizeQosFlow);

        SerializeQfi(i, bc.qfi);
        SerializeQosRule(i, bc.rule);
        SerializeFteid(i, bc.sgwS5uFteid);
        SerializeQosFlow(i, bc.flow);
    }
}

uint32_t
NrGtpcCreateSessionRequestMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeImsi(i, m_imsi);
    DeserializeUliEcgi(i, m_uliEcgi);
    DeserializeFteid(i, m_senderCpFteid);

    m_flowContextsToBeCreated.clear();
    while (i.GetRemainingSize() > 0)
    {
        uint16_t length;
        DeserializeFlowContextHeader(i, length);

        FlowContextToBeCreated flowContext;
        DeserializeQfi(i, flowContext.qfi);

        Ptr<NrQosRule> rule = Create<NrQosRule>();
        DeserializeQosRule(i, rule);
        flowContext.rule = rule;

        DeserializeFteid(i, flowContext.sgwS5uFteid);
        DeserializeQosFlow(i, flowContext.flow);

        m_flowContextsToBeCreated.push_back(flowContext);
    }

    return GetSerializedSize();
}

void
NrGtpcCreateSessionRequestMessage::Print(std::ostream& os) const
{
    os << " imsi " << m_imsi << " uliEcgi " << m_uliEcgi;
}

uint64_t
NrGtpcCreateSessionRequestMessage::GetImsi() const
{
    return m_imsi;
}

void
NrGtpcCreateSessionRequestMessage::SetImsi(uint64_t imsi)
{
    m_imsi = imsi;
}

uint32_t
NrGtpcCreateSessionRequestMessage::GetUliEcgi() const
{
    return m_uliEcgi;
}

void
NrGtpcCreateSessionRequestMessage::SetUliEcgi(uint32_t uliEcgi)
{
    m_uliEcgi = uliEcgi;
}

NrGtpcHeader::Fteid_t
NrGtpcCreateSessionRequestMessage::GetSenderCpFteid() const
{
    return m_senderCpFteid;
}

void
NrGtpcCreateSessionRequestMessage::SetSenderCpFteid(NrGtpcHeader::Fteid_t fteid)
{
    m_senderCpFteid = fteid;
}

std::list<NrGtpcCreateSessionRequestMessage::FlowContextToBeCreated>
NrGtpcCreateSessionRequestMessage::GetFlowContextsToBeCreated() const
{
    return m_flowContextsToBeCreated;
}

void
NrGtpcCreateSessionRequestMessage::SetFlowContextsToBeCreated(
    std::list<NrGtpcCreateSessionRequestMessage::FlowContextToBeCreated> flowContexts)
{
    m_flowContextsToBeCreated = flowContexts;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcCreateSessionResponseMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcCreateSessionResponseMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcCreateSessionResponseMessage>();
    return tid;
}

NrGtpcCreateSessionResponseMessage::NrGtpcCreateSessionResponseMessage()
{
    SetMessageType(NrGtpcHeader::CreateSessionResponse);
    SetSequenceNumber(0);
    m_cause = Cause_t::RESERVED;
    m_senderCpFteid = {};
}

NrGtpcCreateSessionResponseMessage::~NrGtpcCreateSessionResponseMessage()
{
}

TypeId
NrGtpcCreateSessionResponseMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcCreateSessionResponseMessage::GetMessageSize() const
{
    uint32_t serializedSize = serializedSizeCause + serializedSizeFteid;
    for (auto& bc : m_flowContextsCreated)
    {
        serializedSize += serializedSizeFlowContextHeader + serializedSizeQfi +
                          GetSerializedSizeQosRule(bc.rule->GetPacketFilters()) +
                          serializedSizeFteid + serializedSizeQosFlow;
    }

    return serializedSize;
}

uint32_t
NrGtpcCreateSessionResponseMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcCreateSessionResponseMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeCause(i, m_cause);
    SerializeFteid(i, m_senderCpFteid);

    for (auto& bc : m_flowContextsCreated)
    {
        std::list<NrQosRule::PacketFilter> packetFilters = bc.rule->GetPacketFilters();

        SerializeFlowContextHeader(i,
                                   serializedSizeQfi + GetSerializedSizeQosRule(packetFilters) +
                                       serializedSizeFteid + serializedSizeQosFlow);

        SerializeQfi(i, bc.qfi);
        SerializeQosRule(i, bc.rule);
        SerializeFteid(i, bc.fteid);
        SerializeQosFlow(i, bc.flow);
    }
}

uint32_t
NrGtpcCreateSessionResponseMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeCause(i, m_cause);
    DeserializeFteid(i, m_senderCpFteid);

    m_flowContextsCreated.clear();
    while (i.GetRemainingSize() > 0)
    {
        FlowContextCreated flowContext;
        uint16_t length;

        DeserializeFlowContextHeader(i, length);
        DeserializeQfi(i, flowContext.qfi);

        Ptr<NrQosRule> rule = Create<NrQosRule>();
        DeserializeQosRule(i, rule);
        flowContext.rule = rule;

        DeserializeFteid(i, flowContext.fteid);
        DeserializeQosFlow(i, flowContext.flow);

        m_flowContextsCreated.push_back(flowContext);
    }

    return GetSerializedSize();
}

void
NrGtpcCreateSessionResponseMessage::Print(std::ostream& os) const
{
    os << " cause " << m_cause << " FTEID " << m_senderCpFteid.addr << "," << m_senderCpFteid.teid;
}

NrGtpcCreateSessionResponseMessage::Cause_t
NrGtpcCreateSessionResponseMessage::GetCause() const
{
    return m_cause;
}

void
NrGtpcCreateSessionResponseMessage::SetCause(NrGtpcCreateSessionResponseMessage::Cause_t cause)
{
    m_cause = cause;
}

NrGtpcHeader::Fteid_t
NrGtpcCreateSessionResponseMessage::GetSenderCpFteid() const
{
    return m_senderCpFteid;
}

void
NrGtpcCreateSessionResponseMessage::SetSenderCpFteid(NrGtpcHeader::Fteid_t fteid)
{
    m_senderCpFteid = fteid;
}

std::list<NrGtpcCreateSessionResponseMessage::FlowContextCreated>
NrGtpcCreateSessionResponseMessage::GetFlowContextsCreated() const
{
    return m_flowContextsCreated;
}

void
NrGtpcCreateSessionResponseMessage::SetFlowContextsCreated(
    std::list<NrGtpcCreateSessionResponseMessage::FlowContextCreated> flowContexts)
{
    m_flowContextsCreated = flowContexts;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcModifyFlowRequestMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcModifyFlowRequestMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcModifyFlowRequestMessage>();
    return tid;
}

NrGtpcModifyFlowRequestMessage::NrGtpcModifyFlowRequestMessage()
{
    SetMessageType(NrGtpcHeader::ModifyFlowRequest);
    SetSequenceNumber(0);
    m_imsi = 0;
    m_uliEcgi = 0;
}

NrGtpcModifyFlowRequestMessage::~NrGtpcModifyFlowRequestMessage()
{
}

TypeId
NrGtpcModifyFlowRequestMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcModifyFlowRequestMessage::GetMessageSize() const
{
    uint32_t serializedSize =
        serializedSizeImsi + serializedSizeUliEcgi +
        m_flowContextsToBeModified.size() *
            (serializedSizeFlowContextHeader + serializedSizeQfi + serializedSizeFteid);
    return serializedSize;
}

uint32_t
NrGtpcModifyFlowRequestMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcModifyFlowRequestMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeImsi(i, m_imsi);
    SerializeUliEcgi(i, m_uliEcgi);

    for (auto& bc : m_flowContextsToBeModified)
    {
        SerializeFlowContextHeader(i, serializedSizeQfi + serializedSizeFteid);

        SerializeQfi(i, bc.qfi);
        SerializeFteid(i, bc.fteid);
    }
}

uint32_t
NrGtpcModifyFlowRequestMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeImsi(i, m_imsi);
    DeserializeUliEcgi(i, m_uliEcgi);

    while (i.GetRemainingSize() > 0)
    {
        FlowContextToBeModified flowContext;
        uint16_t length;

        DeserializeFlowContextHeader(i, length);

        DeserializeQfi(i, flowContext.qfi);
        DeserializeFteid(i, flowContext.fteid);

        m_flowContextsToBeModified.push_back(flowContext);
    }

    return GetSerializedSize();
}

void
NrGtpcModifyFlowRequestMessage::Print(std::ostream& os) const
{
    os << " imsi " << m_imsi << " uliEcgi " << m_uliEcgi;
}

uint64_t
NrGtpcModifyFlowRequestMessage::GetImsi() const
{
    return m_imsi;
}

void
NrGtpcModifyFlowRequestMessage::SetImsi(uint64_t imsi)
{
    m_imsi = imsi;
}

uint32_t
NrGtpcModifyFlowRequestMessage::GetUliEcgi() const
{
    return m_uliEcgi;
}

void
NrGtpcModifyFlowRequestMessage::SetUliEcgi(uint32_t uliEcgi)
{
    m_uliEcgi = uliEcgi;
}

std::list<NrGtpcModifyFlowRequestMessage::FlowContextToBeModified>
NrGtpcModifyFlowRequestMessage::GetFlowContextsToBeModified() const
{
    return m_flowContextsToBeModified;
}

void
NrGtpcModifyFlowRequestMessage::SetFlowContextsToBeModified(
    std::list<NrGtpcModifyFlowRequestMessage::FlowContextToBeModified> flowContexts)
{
    m_flowContextsToBeModified = flowContexts;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcModifyFlowResponseMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcModifyFlowResponseMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcModifyFlowResponseMessage>();
    return tid;
}

NrGtpcModifyFlowResponseMessage::NrGtpcModifyFlowResponseMessage()
{
    SetMessageType(NrGtpcHeader::ModifyFlowResponse);
    SetSequenceNumber(0);
    m_cause = Cause_t::RESERVED;
}

NrGtpcModifyFlowResponseMessage::~NrGtpcModifyFlowResponseMessage()
{
}

TypeId
NrGtpcModifyFlowResponseMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcModifyFlowResponseMessage::GetMessageSize() const
{
    return serializedSizeCause;
}

uint32_t
NrGtpcModifyFlowResponseMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcModifyFlowResponseMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeCause(i, m_cause);
}

uint32_t
NrGtpcModifyFlowResponseMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeCause(i, m_cause);

    return GetSerializedSize();
}

void
NrGtpcModifyFlowResponseMessage::Print(std::ostream& os) const
{
    os << " cause " << (uint16_t)m_cause;
}

NrGtpcModifyFlowResponseMessage::Cause_t
NrGtpcModifyFlowResponseMessage::GetCause() const
{
    return m_cause;
}

void
NrGtpcModifyFlowResponseMessage::SetCause(NrGtpcModifyFlowResponseMessage::Cause_t cause)
{
    m_cause = cause;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcDeleteFlowCommandMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcDeleteFlowCommandMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcDeleteFlowCommandMessage>();
    return tid;
}

NrGtpcDeleteFlowCommandMessage::NrGtpcDeleteFlowCommandMessage()
{
    SetMessageType(NrGtpcHeader::DeleteFlowCommand);
    SetSequenceNumber(0);
}

NrGtpcDeleteFlowCommandMessage::~NrGtpcDeleteFlowCommandMessage()
{
}

TypeId
NrGtpcDeleteFlowCommandMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcDeleteFlowCommandMessage::GetMessageSize() const
{
    uint32_t serializedSize =
        m_flowContexts.size() * (serializedSizeFlowContextHeader + serializedSizeQfi);
    return serializedSize;
}

uint32_t
NrGtpcDeleteFlowCommandMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcDeleteFlowCommandMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    for (auto& flowContext : m_flowContexts)
    {
        SerializeFlowContextHeader(i, serializedSizeQfi);

        SerializeQfi(i, flowContext.m_qfi);
    }
}

uint32_t
NrGtpcDeleteFlowCommandMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    while (i.GetRemainingSize() > 0)
    {
        uint16_t length;
        DeserializeFlowContextHeader(i, length);

        FlowContext flowContext;
        DeserializeQfi(i, flowContext.m_qfi);
        m_flowContexts.push_back(flowContext);
    }

    return GetSerializedSize();
}

void
NrGtpcDeleteFlowCommandMessage::Print(std::ostream& os) const
{
    os << " flowContexts [";
    for (auto& flowContext : m_flowContexts)
    {
        os << (uint16_t)flowContext.m_qfi << " ";
    }
    os << "]";
}

std::list<NrGtpcDeleteFlowCommandMessage::FlowContext>
NrGtpcDeleteFlowCommandMessage::GetFlowContexts() const
{
    return m_flowContexts;
}

void
NrGtpcDeleteFlowCommandMessage::SetFlowContexts(
    std::list<NrGtpcDeleteFlowCommandMessage::FlowContext> flowContexts)
{
    m_flowContexts = flowContexts;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcDeleteFlowRequestMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcDeleteFlowRequestMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcDeleteFlowRequestMessage>();
    return tid;
}

NrGtpcDeleteFlowRequestMessage::NrGtpcDeleteFlowRequestMessage()
{
    SetMessageType(NrGtpcHeader::DeleteFlowRequest);
    SetSequenceNumber(0);
}

NrGtpcDeleteFlowRequestMessage::~NrGtpcDeleteFlowRequestMessage()
{
}

TypeId
NrGtpcDeleteFlowRequestMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcDeleteFlowRequestMessage::GetMessageSize() const
{
    uint32_t serializedSize = m_qosFlowIds.size() * serializedSizeQfi;
    return serializedSize;
}

uint32_t
NrGtpcDeleteFlowRequestMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcDeleteFlowRequestMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    for (auto& qfi : m_qosFlowIds)
    {
        SerializeQfi(i, qfi);
    }
}

uint32_t
NrGtpcDeleteFlowRequestMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    while (i.GetRemainingSize() > 0)
    {
        uint8_t qfi;
        DeserializeQfi(i, qfi);
        m_qosFlowIds.push_back(qfi);
    }

    return GetSerializedSize();
}

void
NrGtpcDeleteFlowRequestMessage::Print(std::ostream& os) const
{
    os << " qfis [";
    for (auto& qfi : m_qosFlowIds)
    {
        os << (uint16_t)qfi << " ";
    }
    os << "]";
}

std::list<uint8_t>
NrGtpcDeleteFlowRequestMessage::GetQosFlowIds() const
{
    return m_qosFlowIds;
}

void
NrGtpcDeleteFlowRequestMessage::SetQosFlowIds(std::list<uint8_t> qosFlowIds)
{
    m_qosFlowIds = qosFlowIds;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcDeleteFlowResponseMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcDeleteFlowResponseMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcDeleteFlowResponseMessage>();
    return tid;
}

NrGtpcDeleteFlowResponseMessage::NrGtpcDeleteFlowResponseMessage()
{
    SetMessageType(NrGtpcHeader::DeleteFlowResponse);
    SetSequenceNumber(0);
    m_cause = Cause_t::RESERVED;
}

NrGtpcDeleteFlowResponseMessage::~NrGtpcDeleteFlowResponseMessage()
{
}

TypeId
NrGtpcDeleteFlowResponseMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcDeleteFlowResponseMessage::GetMessageSize() const
{
    uint32_t serializedSize = serializedSizeCause + m_qosFlowIds.size() * serializedSizeQfi;
    return serializedSize;
}

uint32_t
NrGtpcDeleteFlowResponseMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcDeleteFlowResponseMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeCause(i, m_cause);

    for (auto& qfi : m_qosFlowIds)
    {
        SerializeQfi(i, qfi);
    }
}

uint32_t
NrGtpcDeleteFlowResponseMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeCause(i, m_cause);

    while (i.GetRemainingSize() > 0)
    {
        uint8_t qfi;
        DeserializeQfi(i, qfi);
        m_qosFlowIds.push_back(qfi);
    }

    return GetSerializedSize();
}

void
NrGtpcDeleteFlowResponseMessage::Print(std::ostream& os) const
{
    os << " cause " << (uint16_t)m_cause << " qosFlowIds [";
    for (auto& qfi : m_qosFlowIds)
    {
        os << (uint16_t)qfi << " ";
    }
    os << "]";
}

NrGtpcDeleteFlowResponseMessage::Cause_t
NrGtpcDeleteFlowResponseMessage::GetCause() const
{
    return m_cause;
}

void
NrGtpcDeleteFlowResponseMessage::SetCause(NrGtpcDeleteFlowResponseMessage::Cause_t cause)
{
    m_cause = cause;
}

std::list<uint8_t>
NrGtpcDeleteFlowResponseMessage::GetQosFlowIds() const
{
    return m_qosFlowIds;
}

void
NrGtpcDeleteFlowResponseMessage::SetQosFlowIds(std::list<uint8_t> qosFlowIds)
{
    m_qosFlowIds = qosFlowIds;
}

} // namespace ns3
