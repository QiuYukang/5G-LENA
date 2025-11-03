// Copyright (c) 2017-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#include "nr-epc-sgw-application.h"

#include "nr-epc-gtpu-header.h"

#include "ns3/log.h"

#include <map>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrEpcSgwApplication");

NS_OBJECT_ENSURE_REGISTERED(NrEpcSgwApplication);

NrEpcSgwApplication::NrEpcSgwApplication(const Ptr<Socket> s1uSocket,
                                         Ipv4Address s5Addr,
                                         const Ptr<Socket> s5uSocket,
                                         const Ptr<Socket> s5cSocket)
    : m_s5Addr(s5Addr),
      m_s5uSocket(s5uSocket),
      m_s5cSocket(s5cSocket),
      m_s1uSocket(s1uSocket),
      m_gtpuUdpPort(2152), // fixed by the standard
      m_gtpcUdpPort(2123), // fixed by the standard
      m_teidCount(0)
{
    NS_LOG_FUNCTION(this << s1uSocket << s5Addr << s5uSocket << s5cSocket);
    m_s1uSocket->SetRecvCallback(MakeCallback(&NrEpcSgwApplication::RecvFromS1uSocket, this));
    m_s5uSocket->SetRecvCallback(MakeCallback(&NrEpcSgwApplication::RecvFromS5uSocket, this));
    m_s5cSocket->SetRecvCallback(MakeCallback(&NrEpcSgwApplication::RecvFromS5cSocket, this));
}

NrEpcSgwApplication::~NrEpcSgwApplication()
{
    NS_LOG_FUNCTION(this);
}

void
NrEpcSgwApplication::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_s1uSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_s1uSocket = nullptr;
    m_s5uSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_s5uSocket = nullptr;
    m_s5cSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    m_s5cSocket = nullptr;
}

TypeId
NrEpcSgwApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrEpcSgwApplication").SetParent<Object>().SetGroupName("Nr");
    return tid;
}

void
NrEpcSgwApplication::AddMme(Ipv4Address mmeS11Addr, Ptr<Socket> s11Socket)
{
    NS_LOG_FUNCTION(this << mmeS11Addr << s11Socket);
    m_mmeS11Addr = mmeS11Addr;
    m_s11Socket = s11Socket;
    m_s11Socket->SetRecvCallback(MakeCallback(&NrEpcSgwApplication::RecvFromS11Socket, this));
}

void
NrEpcSgwApplication::AddPgw(Ipv4Address pgwAddr)
{
    NS_LOG_FUNCTION(this << pgwAddr);
    m_pgwAddr = pgwAddr;
}

void
NrEpcSgwApplication::AddGnb(uint16_t cellId, Ipv4Address gnbAddr, Ipv4Address sgwAddr)
{
    NS_LOG_FUNCTION(this << cellId << gnbAddr << sgwAddr);
    GnbInfo gnbInfo;
    gnbInfo.gnbAddr = gnbAddr;
    gnbInfo.sgwAddr = sgwAddr;
    m_gnbInfoByCellId[cellId] = gnbInfo;
}

void
NrEpcSgwApplication::RecvFromS11Socket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_ASSERT(socket == m_s11Socket);
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
NrEpcSgwApplication::RecvFromS5uSocket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_ASSERT(socket == m_s5uSocket);
    Ptr<Packet> packet = socket->Recv();
    NrGtpuHeader gtpu;
    packet->RemoveHeader(gtpu);
    uint32_t teid = gtpu.GetTeid();

    Ipv4Address gnbAddr = m_gnbByTeidMap[teid];
    NS_LOG_DEBUG("eNB " << gnbAddr << " TEID " << teid);
    SendToS1uSocket(packet, gnbAddr, teid);
}

void
NrEpcSgwApplication::RecvFromS5cSocket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_ASSERT(socket == m_s5cSocket);
    Ptr<Packet> packet = socket->Recv();
    NrGtpcHeader header;
    packet->PeekHeader(header);
    uint16_t msgType = header.GetMessageType();

    switch (msgType)
    {
    case NrGtpcHeader::CreateSessionResponse:
        DoRecvCreateSessionResponse(packet);
        break;

    case NrGtpcHeader::ModifyFlowResponse:
        DoRecvModifyFlowResponse(packet);
        break;

    case NrGtpcHeader::DeleteFlowRequest:
        DoRecvDeleteFlowRequest(packet);
        break;

    default:
        NS_FATAL_ERROR("GTP-C message not supported");
        break;
    }
}

void
NrEpcSgwApplication::RecvFromS1uSocket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_ASSERT(socket == m_s1uSocket);
    Ptr<Packet> packet = socket->Recv();
    NrGtpuHeader gtpu;
    packet->RemoveHeader(gtpu);
    uint32_t teid = gtpu.GetTeid();

    SendToS5uSocket(packet, m_pgwAddr, teid);
}

void
NrEpcSgwApplication::SendToS1uSocket(Ptr<Packet> packet, Ipv4Address gnbAddr, uint32_t teid)
{
    NS_LOG_FUNCTION(this << packet << gnbAddr << teid);

    NrGtpuHeader gtpu;
    gtpu.SetTeid(teid);
    // From 3GPP TS 29.281 v10.0.0 Section 5.1
    // Length of the payload + the non obligatory GTP-U header
    gtpu.SetLength(packet->GetSize() + gtpu.GetSerializedSize() - 8);
    packet->AddHeader(gtpu);
    m_s1uSocket->SendTo(packet, 0, InetSocketAddress(gnbAddr, m_gtpuUdpPort));
}

void
NrEpcSgwApplication::SendToS5uSocket(Ptr<Packet> packet, Ipv4Address pgwAddr, uint32_t teid)
{
    NS_LOG_FUNCTION(this << packet << pgwAddr << teid);

    NrGtpuHeader gtpu;
    gtpu.SetTeid(teid);
    // From 3GPP TS 29.281 v10.0.0 Section 5.1
    // Length of the payload + the non obligatory GTP-U header
    gtpu.SetLength(packet->GetSize() + gtpu.GetSerializedSize() - 8);
    packet->AddHeader(gtpu);
    m_s5uSocket->SendTo(packet, 0, InetSocketAddress(pgwAddr, m_gtpuUdpPort));
}

///////////////////////////////////
// Process messages from the MME
///////////////////////////////////

void
NrEpcSgwApplication::DoRecvCreateSessionRequest(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcCreateSessionRequestMessage msg;
    packet->RemoveHeader(msg);
    uint64_t imsi = msg.GetImsi();
    uint16_t cellId = msg.GetUliEcgi();
    NS_LOG_DEBUG("cellId " << cellId << " IMSI " << imsi);

    auto gnbit = m_gnbInfoByCellId.find(cellId);
    NS_ASSERT_MSG(gnbit != m_gnbInfoByCellId.end(), "unknown CellId " << cellId);
    Ipv4Address gnbAddr = gnbit->second.gnbAddr;
    NS_LOG_DEBUG("eNB " << gnbAddr);

    NrGtpcHeader::Fteid_t mmeS11Fteid = msg.GetSenderCpFteid();
    NS_ASSERT_MSG(mmeS11Fteid.interfaceType == NrGtpcHeader::S11_MME_GTPC, "wrong interface type");

    NrGtpcCreateSessionRequestMessage msgOut;
    msgOut.SetImsi(imsi);
    msgOut.SetUliEcgi(cellId);

    NrGtpcHeader::Fteid_t sgwS5cFteid;
    sgwS5cFteid.interfaceType = NrGtpcHeader::S5_SGW_GTPC;
    sgwS5cFteid.teid = imsi;
    m_mmeS11FteidBySgwS5cTeid[sgwS5cFteid.teid] = mmeS11Fteid;
    sgwS5cFteid.addr = m_s5Addr;
    msgOut.SetSenderCpFteid(sgwS5cFteid); // S5 SGW GTP-C TEID

    std::list<NrGtpcCreateSessionRequestMessage::FlowContextToBeCreated> flowContexts =
        msg.GetFlowContextsToBeCreated();
    NS_LOG_DEBUG("FlowContextToBeCreated size = " << flowContexts.size());
    std::list<NrGtpcCreateSessionRequestMessage::FlowContextToBeCreated> flowContextsOut;
    for (auto& flowContext : flowContexts)
    {
        // simple sanity check. If you ever need more than 4M teids
        // throughout your simulation, you'll need to implement a smarter teid
        // management algorithm.
        NS_ABORT_IF(m_teidCount == 0xFFFFFFFF);
        uint32_t teid = ++m_teidCount;

        NS_LOG_DEBUG("  TEID " << teid);
        m_gnbByTeidMap[teid] = gnbAddr;

        NrGtpcCreateSessionRequestMessage::FlowContextToBeCreated flowContextOut;
        flowContextOut.sgwS5uFteid.interfaceType = NrGtpcHeader::S5_SGW_GTPU;
        flowContextOut.sgwS5uFteid.teid = teid; // S5U SGW FTEID
        flowContextOut.sgwS5uFteid.addr = gnbit->second.sgwAddr;
        flowContextOut.qfi = flowContext.qfi;
        flowContextOut.flow = flowContext.flow;
        flowContextOut.rule = flowContext.rule;
        flowContextsOut.push_back(flowContextOut);
    }

    msgOut.SetFlowContextsToBeCreated(flowContextsOut);

    msgOut.SetTeid(0);
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send CreateSessionRequest to PGW " << m_pgwAddr);
    m_s5cSocket->SendTo(packetOut, 0, InetSocketAddress(m_pgwAddr, m_gtpcUdpPort));
}

void
NrEpcSgwApplication::DoRecvModifyFlowRequest(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcModifyFlowRequestMessage msg;
    packet->RemoveHeader(msg);
    uint64_t imsi = msg.GetImsi();
    uint16_t cellId = msg.GetUliEcgi();
    NS_LOG_DEBUG("cellId " << cellId << " IMSI " << imsi);

    auto gnbit = m_gnbInfoByCellId.find(cellId);
    NS_ASSERT_MSG(gnbit != m_gnbInfoByCellId.end(), "unknown CellId " << cellId);
    Ipv4Address gnbAddr = gnbit->second.gnbAddr;
    NS_LOG_DEBUG("eNB " << gnbAddr);

    NrGtpcModifyFlowRequestMessage msgOut;
    msgOut.SetImsi(imsi);
    msgOut.SetUliEcgi(cellId);

    std::list<NrGtpcModifyFlowRequestMessage::FlowContextToBeModified> flowContextsOut;
    std::list<NrGtpcModifyFlowRequestMessage::FlowContextToBeModified> flowContexts =
        msg.GetFlowContextsToBeModified();
    NS_LOG_DEBUG("FlowContextsToBeModified size = " << flowContexts.size());
    for (auto& flowContext : flowContexts)
    {
        NS_ASSERT_MSG(flowContext.fteid.interfaceType == NrGtpcHeader::S1U_GNB_GTPU,
                      "Wrong FTEID in ModifyFlowRequest msg");
        uint32_t teid = flowContext.fteid.teid;
        Ipv4Address gnbAddr = flowContext.fteid.addr;
        NS_LOG_DEBUG("qfi " << (uint16_t)flowContext.qfi << " TEID " << teid);
        auto addrit = m_gnbByTeidMap.find(teid);
        NS_ASSERT_MSG(addrit != m_gnbByTeidMap.end(), "unknown TEID " << teid);
        addrit->second = gnbAddr;
        NrGtpcModifyFlowRequestMessage::FlowContextToBeModified flowContextOut;
        flowContextOut.qfi = flowContext.qfi;
        flowContextOut.fteid.interfaceType = NrGtpcHeader::S5_SGW_GTPU;
        flowContextOut.fteid.addr = m_s5Addr;
        flowContextOut.fteid.teid = flowContext.fteid.teid;

        flowContextsOut.push_back(flowContextOut);
    }

    msgOut.SetTeid(imsi);
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send ModifyFlowRequest to PGW " << m_pgwAddr);
    m_s5cSocket->SendTo(packetOut, 0, InetSocketAddress(m_pgwAddr, m_gtpcUdpPort));
}

void
NrEpcSgwApplication::DoRecvDeleteFlowCommand(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcDeleteFlowCommandMessage msg;
    packet->RemoveHeader(msg);

    std::list<NrGtpcDeleteFlowCommandMessage::FlowContext> flowContextsOut;
    for (auto& flowContext : msg.GetFlowContexts())
    {
        NS_LOG_DEBUG("qfi " << (uint16_t)flowContext.m_qfi);
        NrGtpcDeleteFlowCommandMessage::FlowContext flowContextOut;
        flowContextOut.m_qfi = flowContext.m_qfi;
        flowContextsOut.push_back(flowContextOut);
    }

    NrGtpcDeleteFlowCommandMessage msgOut;
    msgOut.SetFlowContexts(flowContextsOut);
    msgOut.SetTeid(msg.GetTeid());
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send DeleteFlowCommand to PGW " << m_pgwAddr);
    m_s5cSocket->SendTo(packetOut, 0, InetSocketAddress(m_pgwAddr, m_gtpcUdpPort));
}

void
NrEpcSgwApplication::DoRecvDeleteFlowResponse(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcDeleteFlowResponseMessage msg;
    packet->RemoveHeader(msg);
    NrGtpcDeleteFlowResponseMessage msgOut;
    msgOut.SetQosFlowIds(msg.GetQosFlowIds());
    msgOut.SetTeid(msg.GetTeid());
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send DeleteFlowResponse to PGW " << m_pgwAddr);
    m_s5cSocket->SendTo(packetOut, 0, InetSocketAddress(m_pgwAddr, m_gtpcUdpPort));
}

////////////////////////////////////////////
// Process messages received from the PGW
////////////////////////////////////////////

void
NrEpcSgwApplication::DoRecvCreateSessionResponse(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcCreateSessionResponseMessage msg;
    packet->RemoveHeader(msg);

    NrGtpcHeader::Fteid_t pgwS5cFteid = msg.GetSenderCpFteid();
    NS_ASSERT_MSG(pgwS5cFteid.interfaceType == NrGtpcHeader::S5_PGW_GTPC, "wrong interface type");

    NrGtpcCreateSessionResponseMessage msgOut;
    msgOut.SetCause(NrGtpcCreateSessionResponseMessage::REQUEST_ACCEPTED);

    uint32_t teid = msg.GetTeid();
    NrGtpcHeader::Fteid_t mmeS11Fteid = m_mmeS11FteidBySgwS5cTeid[teid];

    std::list<NrGtpcCreateSessionResponseMessage::FlowContextCreated> flowContexts =
        msg.GetFlowContextsCreated();
    NS_LOG_DEBUG("FlowContextsCreated size = " << flowContexts.size());
    std::list<NrGtpcCreateSessionResponseMessage::FlowContextCreated> flowContextsOut;
    for (auto& flowContext : flowContexts)
    {
        NrGtpcCreateSessionResponseMessage::FlowContextCreated flowContextOut;
        flowContextOut.fteid.interfaceType = NrGtpcHeader::S5_SGW_GTPU;
        flowContextOut.fteid.teid = flowContext.fteid.teid;
        flowContextOut.fteid.addr = m_s5Addr;
        flowContextOut.qfi = flowContext.qfi;
        flowContextOut.flow = flowContext.flow;
        flowContextOut.rule = flowContext.rule;
        flowContextsOut.push_back(flowContext);
    }
    msgOut.SetFlowContextsCreated(flowContextsOut);

    msgOut.SetTeid(mmeS11Fteid.teid);
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send CreateSessionResponse to MME " << mmeS11Fteid.addr);
    m_s11Socket->SendTo(packetOut, 0, InetSocketAddress(mmeS11Fteid.addr, m_gtpcUdpPort));
}

void
NrEpcSgwApplication::DoRecvModifyFlowResponse(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcModifyFlowResponseMessage msg;
    packet->RemoveHeader(msg);

    NrGtpcModifyFlowResponseMessage msgOut;
    msgOut.SetCause(NrGtpcIes::REQUEST_ACCEPTED);
    msgOut.SetTeid(msg.GetTeid());
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send ModifyFlowResponse to MME " << m_mmeS11Addr);
    m_s11Socket->SendTo(packetOut, 0, InetSocketAddress(m_mmeS11Addr, m_gtpcUdpPort));
}

void
NrEpcSgwApplication::DoRecvDeleteFlowRequest(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);

    NrGtpcDeleteFlowRequestMessage msg;
    packet->RemoveHeader(msg);

    NrGtpcDeleteFlowRequestMessage msgOut;
    msgOut.SetQosFlowIds(msg.GetQosFlowIds());
    msgOut.SetTeid(msg.GetTeid());
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send DeleteFlowRequest to MME " << m_mmeS11Addr);
    m_s11Socket->SendTo(packetOut, 0, InetSocketAddress(m_mmeS11Addr, m_gtpcUdpPort));
}

} // namespace ns3
