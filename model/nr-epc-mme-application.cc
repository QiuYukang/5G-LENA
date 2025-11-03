// Copyright (c) 2017-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#include "nr-epc-mme-application.h"

#include "nr-epc-gtpc-header.h"

#include "ns3/log.h"

#include <map>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrEpcMmeApplication");

NS_OBJECT_ENSURE_REGISTERED(NrEpcMmeApplication);

NrEpcMmeApplication::NrEpcMmeApplication()
    : m_gtpcUdpPort(2123) // fixed by the standard
{
    NS_LOG_FUNCTION(this);
    m_s1apSapMme = new NrMemberEpcS1apSapMme<NrEpcMmeApplication>(this);
}

NrEpcMmeApplication::~NrEpcMmeApplication()
{
    NS_LOG_FUNCTION(this);
}

void
NrEpcMmeApplication::DoDispose()
{
    NS_LOG_FUNCTION(this);
    delete m_s1apSapMme;
}

TypeId
NrEpcMmeApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrEpcMmeApplication")
                            .SetParent<Object>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrEpcMmeApplication>();
    return tid;
}

NrEpcS1apSapMme*
NrEpcMmeApplication::GetS1apSapMme()
{
    return m_s1apSapMme;
}

void
NrEpcMmeApplication::AddSgw(Ipv4Address sgwS11Addr,
                            Ipv4Address mmeS11Addr,
                            Ptr<Socket> mmeS11Socket)
{
    NS_LOG_FUNCTION(this << sgwS11Addr << mmeS11Addr << mmeS11Socket);
    m_sgwS11Addr = sgwS11Addr;
    m_mmeS11Addr = mmeS11Addr;
    m_s11Socket = mmeS11Socket;
    m_s11Socket->SetRecvCallback(MakeCallback(&NrEpcMmeApplication::RecvFromS11Socket, this));
}

void
NrEpcMmeApplication::AddGnb(uint16_t gci, Ipv4Address gnbS1uAddr, NrEpcS1apSapGnb* gnbS1apSap)
{
    NS_LOG_FUNCTION(this << gci << gnbS1uAddr << gnbS1apSap);
    Ptr<GnbInfo> gnbInfo = Create<GnbInfo>();
    gnbInfo->gci = gci;
    gnbInfo->s1uAddr = gnbS1uAddr;
    gnbInfo->s1apSapGnb = gnbS1apSap;
    m_gnbInfoMap[gci] = gnbInfo;
}

void
NrEpcMmeApplication::AddUe(uint64_t imsi)
{
    NS_LOG_FUNCTION(this << imsi);
    Ptr<NrUeInfo> ueInfo = Create<NrUeInfo>();
    ueInfo->imsi = imsi;
    ueInfo->mmeUeS1Id = imsi;
    ueInfo->flowCounter = 0;
    m_ueInfoMap[imsi] = ueInfo;
}

uint8_t
NrEpcMmeApplication::AddFlow(uint64_t imsi, Ptr<NrQosRule> rule, NrQosFlow flow)
{
    NS_LOG_FUNCTION(this << imsi);
    auto it = m_ueInfoMap.find(imsi);
    NS_ASSERT_MSG(it != m_ueInfoMap.end(), "could not find any UE with IMSI " << imsi);
    NS_ASSERT_MSG(it->second->flowCounter < 64,
                  "too many flows already! " << it->second->flowCounter);
    FlowInfo flowInfo;
    flowInfo.qfi = ++(it->second->flowCounter);
    flowInfo.rule = rule;
    flowInfo.flow = flow;
    it->second->flowsToBeActivated.push_back(flowInfo);
    return flowInfo.qfi;
}

// S1-AP SAP MME forwarded methods

void
NrEpcMmeApplication::DoInitialUeMessage(uint64_t mmeUeS1Id,
                                        uint16_t gnbUeS1Id,
                                        uint64_t imsi,
                                        uint16_t gci)
{
    NS_LOG_FUNCTION(this << mmeUeS1Id << gnbUeS1Id << imsi << gci);
    auto it = m_ueInfoMap.find(imsi);
    NS_ASSERT_MSG(it != m_ueInfoMap.end(), "could not find any UE with IMSI " << imsi);
    it->second->cellId = gci;

    NrGtpcCreateSessionRequestMessage msg;
    msg.SetImsi(imsi);
    msg.SetUliEcgi(gci);

    NrGtpcHeader::Fteid_t mmeS11Fteid;
    mmeS11Fteid.interfaceType = NrGtpcHeader::S11_MME_GTPC;
    mmeS11Fteid.teid = imsi;
    mmeS11Fteid.addr = m_mmeS11Addr;
    msg.SetSenderCpFteid(mmeS11Fteid); // S11 MME GTP-C F-TEID

    std::list<NrGtpcCreateSessionRequestMessage::FlowContextToBeCreated> flowContexts;
    for (auto bit = it->second->flowsToBeActivated.begin();
         bit != it->second->flowsToBeActivated.end();
         ++bit)
    {
        NrGtpcCreateSessionRequestMessage::FlowContextToBeCreated flowContext{};
        flowContext.qfi = bit->qfi;
        flowContext.rule = bit->rule;
        flowContext.flow = bit->flow;
        flowContexts.push_back(flowContext);
    }
    NS_LOG_DEBUG("FlowContextToBeCreated size = " << flowContexts.size());
    msg.SetFlowContextsToBeCreated(flowContexts);

    msg.SetTeid(0);
    msg.ComputeMessageLength();

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(msg);
    NS_LOG_DEBUG("Send CreateSessionRequest to SGW " << m_sgwS11Addr);
    m_s11Socket->SendTo(packet, 0, InetSocketAddress(m_sgwS11Addr, m_gtpcUdpPort));
}

void
NrEpcMmeApplication::DoInitialContextSetupResponse(
    uint64_t mmeUeS1Id,
    uint16_t gnbUeS1Id,
    std::list<NrEpcS1apSapMme::ErabSetupItem> erabSetupList)
{
    NS_LOG_FUNCTION(this << mmeUeS1Id << gnbUeS1Id);
    NS_FATAL_ERROR("unimplemented");
}

void
NrEpcMmeApplication::DoPathSwitchRequest(
    uint64_t gnbUeS1Id,
    uint64_t mmeUeS1Id,
    uint16_t gci,
    std::list<NrEpcS1apSapMme::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
{
    NS_LOG_FUNCTION(this << mmeUeS1Id << gnbUeS1Id << gci);
    uint64_t imsi = mmeUeS1Id;
    auto it = m_ueInfoMap.find(imsi);
    NS_ASSERT_MSG(it != m_ueInfoMap.end(), "could not find any UE with IMSI " << imsi);
    NS_LOG_INFO("IMSI " << imsi << " old gNB: " << it->second->cellId << ", new gNB: " << gci);
    it->second->cellId = gci;
    it->second->gnbUeS1Id = gnbUeS1Id;

    NrGtpcModifyFlowRequestMessage msg;
    msg.SetImsi(imsi);
    msg.SetUliEcgi(gci);

    std::list<NrGtpcModifyFlowRequestMessage::FlowContextToBeModified> flowContexts;
    for (auto& erab : erabToBeSwitchedInDownlinkList)
    {
        NS_LOG_DEBUG("erabId " << erab.erabId << " gNB " << erab.gnbTransportLayerAddress
                               << " TEID " << erab.gnbTeid);

        NrGtpcModifyFlowRequestMessage::FlowContextToBeModified flowContext;
        flowContext.qfi = erab.erabId;
        flowContext.fteid.interfaceType = NrGtpcHeader::S1U_GNB_GTPU;
        flowContext.fteid.addr = erab.gnbTransportLayerAddress;
        flowContext.fteid.teid = erab.gnbTeid;
        flowContexts.push_back(flowContext);
    }
    msg.SetFlowContextsToBeModified(flowContexts);
    msg.SetTeid(imsi);
    msg.ComputeMessageLength();

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(msg);
    NS_LOG_DEBUG("Send ModifyFlowRequest to SGW " << m_sgwS11Addr);
    m_s11Socket->SendTo(packet, 0, InetSocketAddress(m_sgwS11Addr, m_gtpcUdpPort));
}

void
NrEpcMmeApplication::DoErabReleaseIndication(
    uint64_t mmeUeS1Id,
    uint16_t gnbUeS1Id,
    std::list<NrEpcS1apSapMme::ErabToBeReleasedIndication> erabToBeReleaseIndication)
{
    NS_LOG_FUNCTION(this << mmeUeS1Id << gnbUeS1Id);
    uint64_t imsi = mmeUeS1Id;
    auto it = m_ueInfoMap.find(imsi);
    NS_ASSERT_MSG(it != m_ueInfoMap.end(), "could not find any UE with IMSI " << imsi);

    NrGtpcDeleteFlowCommandMessage msg;
    std::list<NrGtpcDeleteFlowCommandMessage::FlowContext> flowContexts;
    for (auto& erab : erabToBeReleaseIndication)
    {
        NS_LOG_DEBUG("erabId " << (uint16_t)erab.erabId);
        NrGtpcDeleteFlowCommandMessage::FlowContext flowContext;
        flowContext.m_qfi = erab.erabId;
        flowContexts.push_back(flowContext);
    }
    msg.SetFlowContexts(flowContexts);
    msg.SetTeid(imsi);
    msg.ComputeMessageLength();

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(msg);
    NS_LOG_DEBUG("Send DeleteFlowCommand to SGW " << m_sgwS11Addr);
    m_s11Socket->SendTo(packet, 0, InetSocketAddress(m_sgwS11Addr, m_gtpcUdpPort));
}

void
NrEpcMmeApplication::RemoveFlow(Ptr<NrUeInfo> ueInfo, uint8_t qfi)
{
    NS_LOG_FUNCTION(this << qfi);
    auto bit = ueInfo->flowsToBeActivated.begin();
    while (bit != ueInfo->flowsToBeActivated.end())
    {
        if (bit->qfi == qfi)
        {
            ueInfo->flowsToBeActivated.erase(bit);
            ueInfo->flowCounter = ueInfo->flowCounter - 1;
            break;
        }
        ++bit;
    }
}

void
NrEpcMmeApplication::RecvFromS11Socket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_ASSERT(socket == m_s11Socket);
    Ptr<Packet> packet = socket->Recv();
    NrGtpcHeader header;
    packet->PeekHeader(header);
    uint16_t msgType = header.GetMessageType();

    switch (msgType)
    {
    case NrGtpcHeader::CreateSessionResponse:
        DoRecvCreateSessionResponse(header, packet);
        break;

    case NrGtpcHeader::ModifyFlowResponse:
        DoRecvModifyFlowResponse(header, packet);
        break;

    case NrGtpcHeader::DeleteFlowRequest:
        DoRecvDeleteFlowRequest(header, packet);
        break;

    default:
        NS_FATAL_ERROR("GTP-C message not supported");
        break;
    }
}

void
NrEpcMmeApplication::DoRecvCreateSessionResponse(NrGtpcHeader& header, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << header);
    uint64_t imsi = header.GetTeid();
    NS_LOG_DEBUG("TEID/IMSI " << imsi);
    auto it = m_ueInfoMap.find(imsi);
    NS_ASSERT_MSG(it != m_ueInfoMap.end(), "could not find any UE with IMSI " << imsi);
    uint16_t cellId = it->second->cellId;
    uint16_t gnbUeS1Id = it->second->gnbUeS1Id;
    uint64_t mmeUeS1Id = it->second->mmeUeS1Id;
    NS_LOG_DEBUG("cellId " << cellId << " mmeUeS1Id " << mmeUeS1Id << " gnbUeS1Id " << gnbUeS1Id);
    auto jt = m_gnbInfoMap.find(cellId);
    NS_ASSERT_MSG(jt != m_gnbInfoMap.end(), "could not find any gNB with CellId " << cellId);

    NrGtpcCreateSessionResponseMessage msg;
    packet->RemoveHeader(msg);

    std::list<NrEpcS1apSapGnb::ErabToBeSetupItem> erabToBeSetupList;
    std::list<NrGtpcCreateSessionResponseMessage::FlowContextCreated> flowContexts =
        msg.GetFlowContextsCreated();
    NS_LOG_DEBUG("FlowContextsCreated size = " << flowContexts.size());
    for (auto& flowContext : flowContexts)
    {
        NrEpcS1apSapGnb::ErabToBeSetupItem erab;
        erab.erabId = flowContext.qfi;
        erab.erabLevelQosParameters = flowContext.flow;
        erab.transportLayerAddress = flowContext.fteid.addr; // SGW S1U address
        erab.sgwTeid = flowContext.fteid.teid;
        NS_LOG_DEBUG("SGW " << erab.transportLayerAddress << " TEID " << erab.sgwTeid);
        erabToBeSetupList.push_back(erab);
    }

    NS_LOG_DEBUG("Send InitialContextSetupRequest to gNB " << jt->second->s1apSapGnb);
    jt->second->s1apSapGnb->InitialContextSetupRequest(mmeUeS1Id, gnbUeS1Id, erabToBeSetupList);
}

void
NrEpcMmeApplication::DoRecvModifyFlowResponse(NrGtpcHeader& header, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << header);
    NrGtpcModifyFlowResponseMessage msg;
    packet->RemoveHeader(msg);
    NS_ASSERT(msg.GetCause() == NrGtpcModifyFlowResponseMessage::REQUEST_ACCEPTED);

    uint64_t imsi = header.GetTeid();
    NS_LOG_DEBUG("TEID/IMSI " << imsi);
    auto it = m_ueInfoMap.find(imsi);
    NS_ASSERT_MSG(it != m_ueInfoMap.end(), "could not find any UE with IMSI " << imsi);
    uint16_t cellId = it->second->cellId;
    uint16_t gnbUeS1Id = it->second->gnbUeS1Id;
    uint64_t mmeUeS1Id = it->second->mmeUeS1Id;
    NS_LOG_DEBUG("cellId " << cellId << " mmeUeS1Id " << mmeUeS1Id << " gnbUeS1Id " << gnbUeS1Id);
    std::list<NrEpcS1apSapGnb::ErabSwitchedInUplinkItem>
        erabToBeSwitchedInUplinkList; // unused for now
    auto jt = m_gnbInfoMap.find(cellId);
    NS_ASSERT_MSG(jt != m_gnbInfoMap.end(), "could not find any gNB with CellId " << cellId);

    NS_LOG_DEBUG("Send PathSwitchRequestAcknowledge to gNB " << jt->second->s1apSapGnb);
    jt->second->s1apSapGnb->PathSwitchRequestAcknowledge(gnbUeS1Id,
                                                         mmeUeS1Id,
                                                         cellId,
                                                         erabToBeSwitchedInUplinkList);
}

void
NrEpcMmeApplication::DoRecvDeleteFlowRequest(NrGtpcHeader& header, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << header);
    uint64_t imsi = header.GetTeid();
    NS_LOG_DEBUG("TEID/IMSI " << imsi);
    auto it = m_ueInfoMap.find(imsi);
    NS_ASSERT_MSG(it != m_ueInfoMap.end(), "could not find any UE with IMSI " << imsi);

    NrGtpcDeleteFlowRequestMessage msg;
    packet->RemoveHeader(msg);

    NrGtpcDeleteFlowResponseMessage msgOut;

    std::list<uint8_t> qfis;
    for (auto& qfi : msg.GetQosFlowIds())
    {
        qfis.push_back(qfi);
        /*
         * This condition is added to not remove flow info at MME
         * when UE gets disconnected since the flows are only added
         * at beginning of simulation at MME and if it is removed the
         * flows cannot be activated again unless scheduled for
         * addition of the flow during simulation
         *
         */
        if (it->second->cellId == 0)
        {
            RemoveFlow(it->second,
                       qfi); // schedules function to erase, context of de-activated flow
        }
    }
    msgOut.SetQosFlowIds(qfis);
    msgOut.SetTeid(imsi);
    msgOut.ComputeMessageLength();

    Ptr<Packet> packetOut = Create<Packet>();
    packetOut->AddHeader(msgOut);
    NS_LOG_DEBUG("Send DeleteFlowResponse to SGW " << m_sgwS11Addr);
    m_s11Socket->SendTo(packetOut, 0, InetSocketAddress(m_sgwS11Addr, m_gtpcUdpPort));
}

} // namespace ns3
