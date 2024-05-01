/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-sl-ue-mac-harq.h"

#include <ns3/log.h>
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrSlUeMacHarq");
NS_OBJECT_ENSURE_REGISTERED(NrSlUeMacHarq);

TypeId
NrSlUeMacHarq::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::NrSlUeMacHarq")
            .SetParent<Object>()
            .AddConstructor<NrSlUeMacHarq>()
            .SetGroupName("nr")
            .AddTraceSource("RxHarqFeedback",
                            "Receive HARQ feedback trace",
                            MakeTraceSourceAccessor(&NrSlUeMacHarq::m_rxHarqFeedback),
                            "ns3::NrSlUeMacHarq::RxHarqFeedbackTracedCallback")
            .AddTraceSource("Allocate",
                            "Trace HARQ process ID allocation",
                            MakeTraceSourceAccessor(&NrSlUeMacHarq::m_allocateTrace),
                            "ns3::NrSlUeMacHarq::AllocateTracedCallback")
            .AddTraceSource("Deallocate",
                            "Trace HARQ process ID deallocation",
                            MakeTraceSourceAccessor(&NrSlUeMacHarq::m_deallocateTrace),
                            "ns3::NrSlUeMacHarq::DeallocateTracedCallback")
            .AddTraceSource("RequestPacketBurst",
                            "Trace requests for packet bursts (tx and retx)",
                            MakeTraceSourceAccessor(&NrSlUeMacHarq::m_packetBurstTrace),
                            "ns3::NrSlUeMacHarq::PacketBurstTracedCallback")
            .AddTraceSource("Timeout",
                            "Trace HARQ process timer expiry",
                            MakeTraceSourceAccessor(&NrSlUeMacHarq::m_timeoutTrace),
                            "ns3::NrSlUeMacHarq::TimeoutTracedCallback");
    return tid;
}

NrSlUeMacHarq::NrSlUeMacHarq()
{
    NS_LOG_FUNCTION(this);
}

NrSlUeMacHarq::~NrSlUeMacHarq()
{
    NS_LOG_FUNCTION(this);
}

void
NrSlUeMacHarq::DoDispose()
{
    NS_LOG_FUNCTION(this);
    for (auto it : m_pktBuffer)
    {
        it.pktBurst = nullptr;
    }
    m_pktBuffer.clear();
}

void
NrSlUeMacHarq::InitHarqBuffer(uint8_t maxSlProcessesMultiplePdu, uint8_t maxSlProcesses)
{
    NS_LOG_FUNCTION(this << +maxSlProcessesMultiplePdu << +maxSlProcesses);

    m_maxSlProcessesMultiplePdu = maxSlProcessesMultiplePdu;
    m_maxSlProcesses = maxSlProcesses;
    NS_ABORT_MSG_UNLESS(maxSlProcesses >= maxSlProcessesMultiplePdu, "Misconfiguration");
    m_pktBuffer.resize(maxSlProcesses);
    for (uint8_t i = 0; i < maxSlProcesses; i++)
    {
        ResetHarqBuffer(i);
        m_idBuffer.push_back(i);
    }
}

std::optional<uint8_t>
NrSlUeMacHarq::AllocateHarqProcessId(uint32_t dstL2Id, bool multiplePdu, Time timeout)
{
    NS_LOG_FUNCTION(this << dstL2Id << timeout);
    std::optional<uint8_t> harqId;
    if (!m_idBuffer.size())
    {
        NS_LOG_INFO("No HARQ process IDs available for " << dstL2Id);
        return harqId;
    }
    if (multiplePdu && m_numProcessesMultiplePdu == m_maxSlProcessesMultiplePdu)
    {
        NS_LOG_INFO("No HARQ process IDs for multiple PDUs available for " << dstL2Id);
        return harqId;
    }
    harqId = m_idBuffer.front();
    m_idBuffer.pop_front();
    if (multiplePdu)
    {
        m_numProcessesMultiplePdu++;
    }
    NS_LOG_INFO("Allocating HARQ ID " << +harqId.value() << " dstL2Id " << dstL2Id << " timeout "
                                      << timeout.As(Time::MS) << " multiple PDU " << multiplePdu
                                      << " remaining " << m_idBuffer.size());
    m_allocateTrace(harqId.value(), dstL2Id, multiplePdu, timeout, m_idBuffer.size());
    // set the given destination in m_pktBuffer at the index equal to
    // availableHarqId so we can check it while adding the packet.
    m_pktBuffer.at(harqId.value()).dstL2Id = dstL2Id;
    NS_LOG_INFO("Scheduling HARQ process ID " << +harqId.value() << " timer to expire in "
                                              << timeout.As(Time::MS) << " at "
                                              << (Now() + timeout).As(Time::S));
    m_pktBuffer.at(harqId.value()).timer =
        Simulator::Schedule(timeout, &NrSlUeMacHarq::HarqProcessTimerExpiry, this, harqId.value());
    m_pktBuffer.at(harqId.value()).multiplePdu = multiplePdu;
    m_pktBuffer.at(harqId.value()).allocated = true;
    return harqId;
}

void
NrSlUeMacHarq::DeallocateHarqProcessId(uint8_t harqId)
{
    NS_LOG_FUNCTION(this << +harqId);
    if (m_pktBuffer.at(harqId).allocated)
    {
        if (m_pktBuffer.at(harqId).multiplePdu && m_numProcessesMultiplePdu)
        {
            m_numProcessesMultiplePdu--;
        }
        m_idBuffer.push_back(harqId);
        NS_LOG_INFO("Deallocating ID " << +harqId << " remaining " << m_idBuffer.size());
        m_deallocateTrace(harqId, m_idBuffer.size());
        ResetHarqBuffer(harqId);
    }
}

bool
NrSlUeMacHarq::RenewHarqProcessIdTimer(uint8_t harqId, Time timeout)
{
    NS_LOG_FUNCTION(this << harqId << timeout);
    if (!m_pktBuffer.at(harqId).allocated)
    {
        NS_LOG_INFO("HARQ process ID " << +harqId << " is not allocated; not renewing timer");
        NS_ASSERT_MSG(m_pktBuffer.at(harqId).timer.IsRunning(),
                      "Timer should not be running on a deallocated process");
        return false;
    }
    if (m_pktBuffer.at(harqId).timer.IsRunning())
    {
        m_pktBuffer.at(harqId).timer.Cancel();
    }
    NS_LOG_INFO("Renewing HARQ process ID " << +harqId << " timer to expire in "
                                            << timeout.As(Time::MS) << " at "
                                            << (Now() + timeout).As(Time::S));
    m_pktBuffer.at(harqId).timer =
        Simulator::Schedule(timeout, &NrSlUeMacHarq::HarqProcessTimerExpiry, this, harqId);
    return true;
}

uint32_t
NrSlUeMacHarq::GetNumAvailableHarqIds() const
{
    return m_idBuffer.size();
}

bool
NrSlUeMacHarq::IsHarqIdAvailable(uint8_t harqId) const
{
    return !(m_pktBuffer.at(harqId).allocated);
}

void
NrSlUeMacHarq::AddPacket(uint32_t dstL2Id, uint8_t lcId, uint8_t harqId, Ptr<Packet> pkt)
{
    NS_LOG_FUNCTION(this << dstL2Id << +lcId << +harqId);
    NS_ABORT_MSG_IF(m_pktBuffer.at(harqId).dstL2Id == std::numeric_limits<uint16_t>::max(),
                    "Trying to add packet but dstL2Id for HARQ ID " << +harqId << " is unassigned");
    NS_ABORT_MSG_IF(m_pktBuffer.at(harqId).dstL2Id != dstL2Id,
                    "the HARQ id " << +harqId << " does not belongs to the destination " << dstL2Id
                                   << "; instead belongs to: " << m_pktBuffer.at(harqId).dstL2Id);
    NS_ASSERT_MSG(m_pktBuffer.at(harqId).pktBurst != nullptr,
                  " Packet burst not initialized for HARQ id " << +harqId);
    if (m_pktBuffer.at(harqId).multiplePdu && m_pktBuffer.at(harqId).pktBurst->GetNPackets())
    {
        // If there is an SPS grant and no HARQ feedback, there is no way
        // to clear out the previous TB, so flush it here.
        NS_LOG_INFO("Flushing buffer for for dstL2Id " << dstL2Id << " LC ID " << +lcId
                                                       << " HARQ ID " << +harqId);
    }
    NS_LOG_INFO("Adding packet for dstL2Id " << dstL2Id << " LC ID " << +lcId << " HARQ ID "
                                             << +harqId);
    m_pktBuffer.at(harqId).lcidList.insert(lcId);
    m_pktBuffer.at(harqId).pktBurst->AddPacket(pkt);
    // Each LC have one MAC PDU in a TB. Packet burst here, imitates a TB, therefore,
    // the number of LCs inside lcidList and the packets inside the packet burst
    // must be equal.
    NS_ABORT_MSG_IF(m_pktBuffer.at(harqId).lcidList.size() !=
                        m_pktBuffer.at(harqId).pktBurst->GetNPackets(),
                    "Mismatch in number of LCIDs and the number of packets for SL HARQ ID "
                        << +harqId << " dest " << dstL2Id);
}

void
NrSlUeMacHarq::RecvHarqFeedback(SlHarqInfo harqInfo)
{
    NS_LOG_FUNCTION(this << harqInfo.m_dstL2Id << +harqInfo.m_harqProcessId
                         << harqInfo.IsReceivedOk());
    m_rxHarqFeedback(harqInfo);
    if (IsHarqIdAvailable(harqInfo.m_harqProcessId))
    {
        NS_LOG_DEBUG("Feedback (possibly stale) received for unused HARQ ID "
                     << +harqInfo.m_harqProcessId);
        return;
    }
    if (harqInfo.IsReceivedOk() &&
        (m_pktBuffer.at(harqInfo.m_harqProcessId).dstL2Id != harqInfo.m_dstL2Id))
    {
        NS_LOG_DEBUG("Feedback (possibly stale) received for different dstL2Id "
                     << harqInfo.m_dstL2Id << " on HARQ ID " << +harqInfo.m_harqProcessId);
        return;
    }
    // If transmission is ACKed, and it is a dynamic grant, free both the
    // packet buffer and the HARQ ID.  If transmission is ACKed and it is an
    // SPS grant, do not free the HARQ ID but mark the buffer as not allocated.
    if (harqInfo.IsReceivedOk())
    {
        NS_LOG_INFO("Positive feedback for dstL2Id " << harqInfo.m_dstL2Id << " on HARQ ID "
                                                     << +harqInfo.m_harqProcessId);
        if (m_pktBuffer.at(harqInfo.m_harqProcessId).pktBurst->GetSize())
        {
            // Only deallocate process IDs for dynamic grants upon ACK feedback
            if (m_pktBuffer.at(harqInfo.m_harqProcessId).multiplePdu)
            {
                FlushHarqBuffer(harqInfo.m_harqProcessId);
            }
            else
            {
                m_idBuffer.push_back(harqInfo.m_harqProcessId);
                NS_LOG_INFO("Deallocating ID " << +harqInfo.m_harqProcessId << " remaining "
                                               << m_idBuffer.size());
                m_deallocateTrace(harqInfo.m_harqProcessId, m_idBuffer.size());
                ResetHarqBuffer(harqInfo.m_harqProcessId);
            }
        }
    }
    else
    {
        NS_LOG_INFO("Negative feedback for dstL2Id " << harqInfo.m_dstL2Id << " on HARQ ID "
                                                     << +harqInfo.m_harqProcessId);
    }
}

void
NrSlUeMacHarq::FlushHarqBuffer(uint8_t harqId)
{
    NS_LOG_FUNCTION(this << harqId);
    NS_LOG_INFO("Flush packet buffer for HARQ ID " << +harqId);
    Ptr<PacketBurst> pb = CreateObject<PacketBurst>();
    m_pktBuffer.at(harqId).pktBurst = pb;
    m_pktBuffer.at(harqId).lcidList.clear();
}

Ptr<PacketBurst>
NrSlUeMacHarq::GetPacketBurst(uint32_t dstL2Id, uint8_t harqId) const
{
    NS_LOG_FUNCTION(this << dstL2Id << +harqId);
    if (m_pktBuffer.at(harqId).dstL2Id != dstL2Id || !m_pktBuffer.at(harqId).allocated)
    {
        // This operation can fail to return a packet burst if retransmissions
        // have been completed on this HARQ Process ID
        NS_LOG_DEBUG("No packet to return");
        return nullptr;
    }
    NS_LOG_INFO("Packet burst retrieved for dstL2Id " << dstL2Id << " HARQ ID " << +harqId);
    m_packetBurstTrace(dstL2Id, harqId);
    return m_pktBuffer.at(harqId).pktBurst;
}

void
NrSlUeMacHarq::HarqProcessTimerExpiry(uint8_t harqId)
{
    NS_LOG_FUNCTION(this << +harqId);
    NS_LOG_INFO("HARQ process ID " << +harqId << " timed out");
    m_timeoutTrace(harqId);
    // If this was a dynamic grant, deallocate the HARQ process ID and Reset
    if (m_pktBuffer.at(harqId).multiplePdu)
    {
        FlushHarqBuffer(harqId);
    }
    else
    {
        m_idBuffer.push_back(harqId);
        NS_LOG_INFO("Deallocating ID " << +harqId << " remaining " << m_idBuffer.size());
        m_deallocateTrace(harqId, m_idBuffer.size());
        ResetHarqBuffer(harqId);
    }
}

void
NrSlUeMacHarq::ResetHarqBuffer(uint8_t harqId)
{
    NS_LOG_FUNCTION(this << +harqId);
    FlushHarqBuffer(harqId);
    if (m_pktBuffer.at(harqId).timer.IsRunning())
    {
        m_pktBuffer.at(harqId).timer.Cancel();
    }
    m_pktBuffer.at(harqId).dstL2Id = std::numeric_limits<uint16_t>::max();
    m_pktBuffer.at(harqId).multiplePdu = false;
    m_pktBuffer.at(harqId).allocated = false;
}

} // namespace ns3
