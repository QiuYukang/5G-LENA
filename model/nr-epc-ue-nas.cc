// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>

#include "nr-epc-ue-nas.h"

#include "nr-as-sap.h"

#include "ns3/fatal-error.h"
#include "ns3/log.h"
#include "ns3/nr-epc-helper.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrEpcUeNas");

/// Map each of UE NAS states to its string representation.
static const std::string g_ueNasStateName[NrEpcUeNas::NUM_STATES] = {
    "OFF",
    "ATTACHING",
    "IDLE_REGISTERED",
    "CONNECTING_TO_EPC",
    "ACTIVE",
};

/**
 * @param s The UE NAS state.
 * @return The string representation of the given state.
 */
static inline const std::string&
ToString(NrEpcUeNas::State s)
{
    return g_ueNasStateName[s];
}

NS_OBJECT_ENSURE_REGISTERED(NrEpcUeNas);

NrEpcUeNas::NrEpcUeNas()
    : m_state(OFF),
      m_csgId(0),
      m_asSapProvider(nullptr),
      m_qfiCounter(0)
{
    NS_LOG_FUNCTION(this);
    m_asSapUser = new MemberNrAsSapUser<NrEpcUeNas>(this);
}

NrEpcUeNas::~NrEpcUeNas()
{
    NS_LOG_FUNCTION(this);
}

void
NrEpcUeNas::DoDispose()
{
    NS_LOG_FUNCTION(this);
    delete m_asSapUser;
}

TypeId
NrEpcUeNas::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrEpcUeNas")
            .SetParent<Object>()
            .SetGroupName("Nr")
            .AddConstructor<NrEpcUeNas>()
            .AddTraceSource("StateTransition",
                            "fired upon every UE NAS state transition",
                            MakeTraceSourceAccessor(&NrEpcUeNas::m_stateTransitionCallback),
                            "ns3::NrEpcUeNas::StateTracedCallback");
    return tid;
}

void
NrEpcUeNas::SetDevice(Ptr<NetDevice> dev)
{
    NS_LOG_FUNCTION(this << dev);
    m_device = dev;
}

void
NrEpcUeNas::SetImsi(uint64_t imsi)
{
    NS_LOG_FUNCTION(this << imsi);
    m_imsi = imsi;
}

void
NrEpcUeNas::SetCsgId(uint32_t csgId)
{
    NS_LOG_FUNCTION(this << csgId);
    m_csgId = csgId;
    m_asSapProvider->SetCsgWhiteList(csgId);
}

uint32_t
NrEpcUeNas::GetCsgId() const
{
    NS_LOG_FUNCTION(this);
    return m_csgId;
}

void
NrEpcUeNas::SetAsSapProvider(NrAsSapProvider* s)
{
    NS_LOG_FUNCTION(this << s);
    m_asSapProvider = s;
}

NrAsSapUser*
NrEpcUeNas::GetAsSapUser()
{
    NS_LOG_FUNCTION(this);
    return m_asSapUser;
}

void
NrEpcUeNas::SetForwardUpCallback(Callback<void, Ptr<Packet>> cb)
{
    NS_LOG_FUNCTION(this);
    m_forwardUpCallback = cb;
}

void
NrEpcUeNas::StartCellSelection(uint32_t arfcn)
{
    NS_LOG_FUNCTION(this << arfcn);
    m_asSapProvider->StartCellSelection(arfcn);
}

void
NrEpcUeNas::Connect()
{
    NS_LOG_FUNCTION(this);

    // tell RRC to go into connected mode
    m_asSapProvider->Connect();
}

void
NrEpcUeNas::Connect(uint16_t cellId, uint32_t arfcn)
{
    NS_LOG_FUNCTION(this << cellId << arfcn);

    // force the UE RRC to be camped on a specific eNB
    m_asSapProvider->ForceCampedOnGnb(cellId, arfcn);

    // tell RRC to go into connected mode
    m_asSapProvider->Connect();
}

void
NrEpcUeNas::Disconnect()
{
    NS_LOG_FUNCTION(this);
    SwitchToState(OFF);
    m_asSapProvider->Disconnect();
}

void
NrEpcUeNas::ActivateQosFlow(NrQosFlow flow, Ptr<NrQosRule> rule)
{
    NS_LOG_FUNCTION(this);
    switch (m_state)
    {
    case ACTIVE:
        NS_FATAL_ERROR(
            "the necessary NAS signaling to activate a QoS flow after the initial context "
            "has already been setup is not implemented");
        break;

    default:
        QosFlowToBeActivated qftba;
        qftba.flow = flow;
        qftba.rule = rule;
        m_qosFlowsToBeActivatedList.push_back(qftba);
        m_qosFlowsToBeActivatedListForReconnection.push_back(qftba);
        break;
    }
}

bool
NrEpcUeNas::Send(Ptr<Packet> packet, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << protocolNumber);

    switch (m_state)
    {
    case ACTIVE: {
        auto qfi = m_qosRuleClassifier.Classify(packet, NrQosRule::UPLINK, protocolNumber);
        if (!qfi.has_value())
        {
            return false;
        }
        else
        {
            m_asSapProvider->SendData(packet, qfi.value());
            return true;
        }
    }
    break;

    default:
        NS_LOG_WARN(this << " NAS OFF, discarding packet");
        return false;
    }
}

void
NrEpcUeNas::DoNotifyConnectionSuccessful()
{
    NS_LOG_FUNCTION(this);

    SwitchToState(ACTIVE); // will eventually activate dedicated QoS flows
}

void
NrEpcUeNas::DoNotifyConnectionFailed()
{
    NS_LOG_FUNCTION(this);

    // immediately retry the connection
    Simulator::ScheduleNow(&NrAsSapProvider::Connect, m_asSapProvider);
}

void
NrEpcUeNas::DoRecvData(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    m_forwardUpCallback(packet);
}

void
NrEpcUeNas::DoNotifyConnectionReleased()
{
    NS_LOG_FUNCTION(this);
    // remove rules
    while (m_qfiCounter > 0)
    {
        m_qosRuleClassifier.Delete(m_qfiCounter);
        m_qfiCounter--;
    }
    // restore the QoS flow list to be activated for the next RRC connection
    m_qosFlowsToBeActivatedList = m_qosFlowsToBeActivatedListForReconnection;

    Disconnect();
}

void
NrEpcUeNas::DoActivateQosFlow(NrQosFlow flow, Ptr<NrQosRule> rule)
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_MSG_UNLESS(m_qfiCounter < 64, "cannot have more than 64 QFIs");
    uint8_t qfi = ++m_qfiCounter;
    m_qosRuleClassifier.Add(rule, qfi);
}

NrEpcUeNas::State
NrEpcUeNas::GetState() const
{
    NS_LOG_FUNCTION(this);
    return m_state;
}

void
NrEpcUeNas::SwitchToState(State newState)
{
    NS_LOG_FUNCTION(this << ToString(newState));
    State oldState = m_state;
    m_state = newState;
    NS_LOG_INFO("IMSI " << m_imsi << " NAS " << ToString(oldState) << " --> "
                        << ToString(newState));
    m_stateTransitionCallback(oldState, newState);

    // actions to be done when entering a new state:
    switch (m_state)
    {
    case ACTIVE:
        for (auto it = m_qosFlowsToBeActivatedList.begin(); it != m_qosFlowsToBeActivatedList.end();
             m_qosFlowsToBeActivatedList.erase(it++))
        {
            DoActivateQosFlow(it->flow, it->rule);
        }
        break;

    default:
        break;
    }
}

} // namespace ns3
