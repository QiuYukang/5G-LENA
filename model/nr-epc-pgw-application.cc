// Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>
//         (based on epc-sgw-pgw-application.cc)

#include "nr-epc-pgw-application.h"

#include "nr-epc-gtpu-header.h"

#include "ns3/abort.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrEpcPgwApplication");

/////////////////////////
// NrUeInfo
/////////////////////////

NrEpcPgwApplication::NrUeInfo::NrUeInfo()
{
    NS_LOG_FUNCTION(this);
}

void
NrEpcPgwApplication::NrUeInfo::AddFlow(uint8_t qfi, uint32_t teid, Ptr<NrQosRule> rule)
{
    NS_LOG_FUNCTION(this << (uint16_t)qfi << teid << rule);
    m_teidByFlowIdMap[qfi] = teid;
    NS_LOG_INFO("Add entry to TEID: " << teid << " by flow ID: " << +qfi << " map");
    m_qosRuleClassifier.Add(rule, qfi);
    NS_LOG_INFO("Add QosRule entry to classifier for QFI: " << +qfi);
}

void
NrEpcPgwApplication::NrUeInfo::RemoveFlow(uint8_t qfi)
{
    NS_LOG_FUNCTION(this << (uint16_t)qfi);
    auto it = m_teidByFlowIdMap.find(qfi);
    bool found = m_qosRuleClassifier.Delete(qfi);
    if (!found)
    {
        NS_LOG_WARN("Could not remove entry in classifier for QFI: " << +qfi);
    }
    else
    {
        NS_LOG_INFO("Removed QosRule entry from classifier for QFI: " << +qfi);
    }
    std::size_t erasedCount = m_teidByFlowIdMap.erase(qfi);
    if (!erasedCount)
    {
        NS_LOG_WARN("TEID by Flow ID map did not erase flow ID: " << +qfi << " (not found)");
    }
    else
    {
        NS_LOG_INFO("Removed entry from TEID: " << it->second << " by flow ID: " << +qfi << " map");
    }
}

std::optional<uint32_t>
NrEpcPgwApplication::NrUeInfo::Classify(Ptr<Packet> p, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << p);
    // We hardcode DOWNLINK direction since the PGW is expected to
    // classify only downlink packets (uplink packets will go to the
    // internet without any classification).
    auto qfi = m_qosRuleClassifier.Classify(p, NrQosRule::DOWNLINK, protocolNumber);
    if (!qfi.has_value())
    {
        return std::nullopt;
    }

    // Look up the TEID corresponding to the matched QFI
    auto it = m_teidByFlowIdMap.find(qfi.value());
    if (it == m_teidByFlowIdMap.end())
    {
        NS_LOG_WARN("QFI " << +qfi.value() << " not found in TEID map");
        return std::nullopt;
    }

    return std::optional<uint32_t>(it->second);
}

Ipv4Address
NrEpcPgwApplication::NrUeInfo::GetSgwAddr()
{
    return m_sgwAddr;
}

void
NrEpcPgwApplication::NrUeInfo::SetSgwAddr(Ipv4Address sgwAddr)
{
    m_sgwAddr = sgwAddr;
}

Ipv4Address
NrEpcPgwApplication::NrUeInfo::GetUeAddr()
{
    return m_ueAddr;
}

void
NrEpcPgwApplication::NrUeInfo::SetUeAddr(Ipv4Address ueAddr)
{
    m_ueAddr = ueAddr;
}

Ipv6Address
NrEpcPgwApplication::NrUeInfo::GetUeAddr6()
{
    return m_ueAddr6;
}

void
NrEpcPgwApplication::NrUeInfo::SetUeAddr6(Ipv6Address ueAddr)
{
    m_ueAddr6 = ueAddr;
}

/////////////////////////
// NrEpcPgwApplication
/////////////////////////

TypeId
NrEpcPgwApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrEpcPgwApplication")
            .SetParent<Object>()
            .SetGroupName("Nr")
            .AddTraceSource("RxFromTun",
                            "Receive data packets from internet in Tunnel NetDevice",
                            MakeTraceSourceAccessor(&NrEpcPgwApplication::m_rxTunPktTrace),
                            "ns3::NrEpcPgwApplication::RxTracedCallback")
            .AddTraceSource("RxFromS1u",
                            "Receive data packets from S5 Socket",
                            MakeTraceSourceAccessor(&NrEpcPgwApplication::m_rxS5PktTrace),
                            "ns3::NrEpcPgwApplication::RxTracedCallback");
    return tid;
}

void
NrEpcPgwApplication::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_s5uSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_s5uSocket = nullptr;
    m_s5cSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_s5cSocket = nullptr;
}

NrEpcPgwApplication::NrEpcPgwApplication(const Ptr<VirtualNetDevice> tunDevice,
                                         Ipv4Address s5Addr,
                                         const Ptr<Socket> s5uSocket,
                                         const Ptr<Socket> s5cSocket)
    : m_pgwS5Addr(s5Addr),
      m_s5uSocket(s5uSocket),
      m_s5cSocket(s5cSocket),
      m_tunDevice(tunDevice),
      m_gtpuUdpPort(2152), // fixed by the standard
      m_gtpcUdpPort(2123)  // fixed by the standard
{
    NS_LOG_FUNCTION(this << tunDevice << s5Addr << s5uSocket << s5cSocket);
    m_s5uSocket->SetRecvCallback(MakeCallback(&NrEpcPgwApplication::RecvFromS5uSocket, this));
    m_s5cSocket->SetRecvCallback(MakeCallback(&NrEpcPgwApplication::RecvFromS5cSocket, this));
}

NrEpcPgwApplication::~NrEpcPgwApplication()
{
    NS_LOG_FUNCTION(this);
}

bool
NrEpcPgwApplication::RecvFromTunDevice(Ptr<Packet> packet,
                                       const Address& source,
                                       const Address& dest,
                                       uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << source << dest << protocolNumber << packet << packet->GetSize());
    m_rxTunPktTrace(packet->Copy());

    // Downlink packet routing (internet to UE).
    // This method handles downlink packets arriving from the internet via the TUN device.
    // The routing procedure is:
    // 1. Extract UE destination address from IP header
    // 2. Find the NrUeInfo context for this UE using the address
    // 3. Call Classify() which internally:
    //    a. Classifies packet using QoS rules to obtain QFI
    //    b. Looks up TEID from m_teidByFlowIdMap[qfi]
    //    c. Returns TEID directly
    // 4. Encapsulate packet in GTP-U header with TEID for tunneling to SGW
    // 5. Send via S5-U interface to SGW
    //
    // Note on TEID allocation: The TEID is allocated by SGW and received during
    // bearer setup in DoRecvCreateSessionRequest(). At PGW, we maintain the mapping
    // from QFI to TEID in m_teidByFlowIdMap. The gNB maintains the reverse mapping
    // (TEID -> (RNTI, QFI)) via m_teidRqfiMap for routing downlink packets back to
    // the correct bearer.

    // get IP address of UE
    if (protocolNumber == Ipv4L3Protocol::PROT_NUMBER)
    {
        Ipv4Header ipv4Header;
        packet->PeekHeader(ipv4Header);
        Ipv4Address ueAddr = ipv4Header.GetDestination();
        NS_LOG_LOGIC("packet addressed to UE " << ueAddr);

        // find corresponding NrUeInfo address
        auto it = m_ueInfoByAddrMap.find(ueAddr);
        if (it == m_ueInfoByAddrMap.end())
        {
            NS_LOG_WARN("unknown UE address " << ueAddr);
        }
        else
        {
            Ipv4Address sgwAddr = it->second->GetSgwAddr();
            auto teid = it->second->Classify(packet, protocolNumber);
            if (!teid.has_value())
            {
                NS_LOG_WARN("no matching flow for this packet");
            }
            else
            {
                SendToS5uSocket(packet, sgwAddr, teid.value());
            }
        }
    }
    else if (protocolNumber == Ipv6L3Protocol::PROT_NUMBER)
    {
        Ipv6Header ipv6Header;
        packet->PeekHeader(ipv6Header);
        Ipv6Address ueAddr = ipv6Header.GetDestination();
        NS_LOG_LOGIC("packet addressed to UE " << ueAddr);

        // find corresponding NrUeInfo address
        auto it = m_ueInfoByAddrMap6.find(ueAddr);
        if (it == m_ueInfoByAddrMap6.end())
        {
            NS_LOG_WARN("unknown UE address " << ueAddr);
        }
        else
        {
            Ipv4Address sgwAddr = it->second->GetSgwAddr();
            auto teid = it->second->Classify(packet, protocolNumber);
            if (!teid.has_value())
            {
                NS_LOG_WARN("no matching flow for this packet");
            }
            else
            {
                SendToS5uSocket(packet, sgwAddr, teid.value());
            }
        }
    }
    else
    {
        NS_ABORT_MSG("Unknown IP type");
    }

    // there is no reason why we should notify the TUN
    // VirtualNetDevice that he failed to send the packet: if we receive
    // any bogus packet, it will just be silently discarded.
    const bool succeeded = true;
    return succeeded;
}

void
NrEpcPgwApplication::RecvFromS5uSocket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_ASSERT(socket == m_s5uSocket);
    Ptr<Packet> packet = socket->Recv();
    m_rxS5PktTrace(packet->Copy());

    NrGtpuHeader gtpu;
    packet->RemoveHeader(gtpu);
    uint32_t teid = gtpu.GetTeid();

    SendToTunDevice(packet, teid);
}

void
NrEpcPgwApplication::RecvFromS5cSocket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_ASSERT(socket == m_s5cSocket);
    Ptr<Packet> packet = socket->Recv();
    NrGtpcHeader header;
    packet->PeekHeader(header);
    uint16_t msgType = header.GetMessageType();

    switch (msgType)
    {
    case NrGtpcHeader::CreateSessionRequest:
        DoRecvCreateSessionRequest(packet);
        break;

    case NrGtpcHeader::ModifyFlowRequest:
        DoRecvModifyFlowRequest(packet);
        break;

    case NrGtpcHeader::DeleteFlowCommand:
        DoRecvDeleteFlowCommand(packet);
        break;

    case NrGtpcHeader::DeleteFlowResponse:
        DoRecvDeleteFlowResponse(packet);
        break;

    default:
        NS_FATAL_ERROR("GTP-C message not supported");
        break;
    }
}

void
NrEpcPgwApplication::DoRecvCreateSessionRequest(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcCreateSessionRequestMessage msg;
    packet->RemoveHeader(msg);
    uint64_t imsi = msg.GetImsi();
    uint16_t cellId = msg.GetUliEcgi();
    NS_LOG_DEBUG("cellId " << cellId << " IMSI " << imsi);

    auto ueit = m_ueInfoByImsiMap.find(imsi);
    NS_ASSERT_MSG(ueit != m_ueInfoByImsiMap.end(), "unknown IMSI " << imsi);
    ueit->second->SetSgwAddr(m_sgwS5Addr);

    NrGtpcHeader::Fteid_t sgwS5cFteid = msg.GetSenderCpFteid();
    NS_ASSERT_MSG(sgwS5cFteid.interfaceType == NrGtpcHeader::S5_SGW_GTPC, "Wrong interface type");

    NrGtpcCreateSessionResponseMessage msgOut;
    msgOut.SetTeid(sgwS5cFteid.teid);
    msgOut.SetCause(NrGtpcCreateSessionResponseMessage::REQUEST_ACCEPTED);

    NrGtpcHeader::Fteid_t pgwS5cFteid;
    pgwS5cFteid.interfaceType = NrGtpcHeader::S5_PGW_GTPC;
    pgwS5cFteid.teid = sgwS5cFteid.teid;
    pgwS5cFteid.addr = m_pgwS5Addr;
    msgOut.SetSenderCpFteid(pgwS5cFteid);

    std::list<NrGtpcCreateSessionRequestMessage::FlowContextToBeCreated> flowContexts =
        msg.GetFlowContextsToBeCreated();
    NS_LOG_DEBUG("FlowContextsToBeCreated size = " << flowContexts.size());

    std::list<NrGtpcCreateSessionResponseMessage::FlowContextCreated> flowContextsCreated;
    for (auto& flowContext : flowContexts)
    {
        uint32_t teid = flowContext.sgwS5uFteid.teid;
        NS_LOG_DEBUG("qfi " << (uint16_t)flowContext.qfi << " SGW " << flowContext.sgwS5uFteid.addr
                            << " TEID " << teid);

        ueit->second->AddFlow(flowContext.qfi, teid, flowContext.rule);

        NrGtpcCreateSessionResponseMessage::FlowContextCreated flowContextOut;
        flowContextOut.fteid.interfaceType = NrGtpcHeader::S5_PGW_GTPU;
        flowContextOut.fteid.teid = teid;
        flowContextOut.fteid.addr = m_pgwS5Addr;
        flowContextOut.qfi = flowContext.qfi;
        flowContextOut.flow = flowContext.flow;
        flowContextOut.rule = flowContext.rule;
        flowContextsCreated.push_back(flowContextOut);
    }

    NS_LOG_DEBUG("FlowContextsCreated size = " << flowContextsCreated.size());
    msgOut.SetFlowContextsCreated(flowContextsCreated);
    msgOut.SetTeid(sgwS5cFteid.teid);
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send CreateSessionResponse to SGW " << sgwS5cFteid.addr);
    m_s5cSocket->SendTo(packetOut, 0, InetSocketAddress(sgwS5cFteid.addr, m_gtpcUdpPort));
}

void
NrEpcPgwApplication::DoRecvModifyFlowRequest(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcModifyFlowRequestMessage msg;
    packet->RemoveHeader(msg);
    uint64_t imsi = msg.GetImsi();
    uint16_t cellId = msg.GetUliEcgi();
    NS_LOG_DEBUG("cellId " << cellId << " IMSI " << imsi);

    auto ueit = m_ueInfoByImsiMap.find(imsi);
    NS_ASSERT_MSG(ueit != m_ueInfoByImsiMap.end(), "unknown IMSI " << imsi);
    ueit->second->SetSgwAddr(m_sgwS5Addr);

    std::list<NrGtpcModifyFlowRequestMessage::FlowContextToBeModified> flowContexts =
        msg.GetFlowContextsToBeModified();
    NS_LOG_DEBUG("FlowContextsToBeModified size = " << flowContexts.size());

    for (auto& flowContext : flowContexts)
    {
        Ipv4Address sgwAddr = flowContext.fteid.addr;
        uint32_t teid = flowContext.fteid.teid;
        NS_LOG_DEBUG("qfi " << (uint16_t)flowContext.qfi << " SGW " << sgwAddr << " TEID " << teid);
    }

    NrGtpcModifyFlowResponseMessage msgOut;
    msgOut.SetCause(NrGtpcIes::REQUEST_ACCEPTED);
    msgOut.SetTeid(imsi);
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send ModifyFlowResponse to SGW " << m_sgwS5Addr);
    m_s5cSocket->SendTo(packetOut, 0, InetSocketAddress(m_sgwS5Addr, m_gtpcUdpPort));
}

void
NrEpcPgwApplication::DoRecvDeleteFlowCommand(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcDeleteFlowCommandMessage msg;
    packet->RemoveHeader(msg);

    std::list<uint8_t> qosFlowIds;
    for (auto& flowContext : msg.GetFlowContexts())
    {
        NS_LOG_DEBUG("QFI to delete " << (uint16_t)flowContext.m_qfi);
        qosFlowIds.push_back(flowContext.m_qfi);
    }

    NrGtpcDeleteFlowRequestMessage msgOut;
    msgOut.SetQosFlowIds(qosFlowIds);
    msgOut.SetTeid(msg.GetTeid());
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send DeleteFlowRequest to SGW " << m_sgwS5Addr);
    m_s5cSocket->SendTo(packetOut, 0, InetSocketAddress(m_sgwS5Addr, m_gtpcUdpPort));
}

void
NrEpcPgwApplication::DoRecvDeleteFlowResponse(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcDeleteFlowResponseMessage msg;
    packet->RemoveHeader(msg);

    uint64_t imsi = msg.GetTeid();
    auto ueit = m_ueInfoByImsiMap.find(imsi);
    NS_ASSERT_MSG(ueit != m_ueInfoByImsiMap.end(), "unknown IMSI " << imsi);

    for (auto& qfi : msg.GetQosFlowIds())
    {
        // Remove de-activated flow contexts from PGW side
        NS_LOG_INFO("PGW removing flow " << (uint16_t)qfi << " of IMSI " << imsi);
        ueit->second->RemoveFlow(qfi);
    }
}

void
NrEpcPgwApplication::SendToTunDevice(Ptr<Packet> packet, uint32_t teid)
{
    NS_LOG_FUNCTION(this << packet << teid);
    NS_LOG_LOGIC("packet size: " << packet->GetSize() << " bytes");

    uint8_t ipType;
    packet->CopyData(&ipType, 1);
    ipType = (ipType >> 4) & 0x0f;

    uint16_t protocol = 0;
    if (ipType == 0x04)
    {
        protocol = 0x0800;
    }
    else if (ipType == 0x06)
    {
        protocol = 0x86DD;
    }
    else
    {
        NS_ABORT_MSG("Unknown IP type");
    }

    m_tunDevice->Receive(packet,
                         protocol,
                         m_tunDevice->GetAddress(),
                         m_tunDevice->GetAddress(),
                         NetDevice::PACKET_HOST);
}

void
NrEpcPgwApplication::SendToS5uSocket(Ptr<Packet> packet, Ipv4Address sgwAddr, uint32_t teid)
{
    NS_LOG_FUNCTION(this << packet << sgwAddr << teid);

    NrGtpuHeader gtpu;
    gtpu.SetTeid(teid);
    // From 3GPP TS 29.281 v10.0.0 Section 5.1
    // Length of the payload + the non obligatory GTP-U header
    gtpu.SetLength(packet->GetSize() + gtpu.GetSerializedSize() - 8);
    packet->AddHeader(gtpu);
    uint32_t flags = 0;
    NS_LOG_INFO("Sending packet to S5U socket with TEID " << teid << " address " << sgwAddr
                                                          << " port " << m_gtpuUdpPort);
    m_s5uSocket->SendTo(packet, flags, InetSocketAddress(sgwAddr, m_gtpuUdpPort));
}

void
NrEpcPgwApplication::AddSgw(Ipv4Address sgwS5Addr)
{
    NS_LOG_FUNCTION(this << sgwS5Addr);
    m_sgwS5Addr = sgwS5Addr;
}

void
NrEpcPgwApplication::AddUe(uint64_t imsi)
{
    NS_LOG_FUNCTION(this << imsi);
    Ptr<NrUeInfo> ueInfo = Create<NrUeInfo>();
    m_ueInfoByImsiMap[imsi] = ueInfo;
}

void
NrEpcPgwApplication::SetUeAddress(uint64_t imsi, Ipv4Address ueAddr)
{
    NS_LOG_FUNCTION(this << imsi << ueAddr);
    auto ueit = m_ueInfoByImsiMap.find(imsi);
    NS_ASSERT_MSG(ueit != m_ueInfoByImsiMap.end(), "unknown IMSI" << imsi);
    ueit->second->SetUeAddr(ueAddr);
    m_ueInfoByAddrMap[ueAddr] = ueit->second;
}

void
NrEpcPgwApplication::SetUeAddress6(uint64_t imsi, Ipv6Address ueAddr)
{
    NS_LOG_FUNCTION(this << imsi << ueAddr);
    auto ueit = m_ueInfoByImsiMap.find(imsi);
    NS_ASSERT_MSG(ueit != m_ueInfoByImsiMap.end(), "unknown IMSI " << imsi);
    m_ueInfoByAddrMap6[ueAddr] = ueit->second;
    ueit->second->SetUeAddr6(ueAddr);
}

} // namespace ns3
