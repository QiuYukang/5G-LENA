// Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>

#ifndef NR_EPC_UE_NAS_H
#define NR_EPC_UE_NAS_H

#include "nr-as-sap.h"
#include "nr-eps-bearer.h"
#include "nr-qos-rule-classifier.h"

#include "ns3/object.h"
#include "ns3/traced-callback.h"

namespace ns3
{

class NrEpcHelper;
class NetDevice;

class NrEpcUeNas : public Object
{
    /// allow MemberNrAsSapUser<NrEpcUeNas> class friend access
    friend class MemberNrAsSapUser<NrEpcUeNas>;

  public:
    /**
     * Constructor
     */
    NrEpcUeNas();

    /**
     * Destructor
     */
    ~NrEpcUeNas() override;

    // inherited from Object
    void DoDispose() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     *
     * @param dev the UE NetDevice
     */
    void SetDevice(Ptr<NetDevice> dev);

    /**
     *
     *
     * @param imsi the unique UE identifier
     */
    void SetImsi(uint64_t imsi);

    /**
     *
     * @param csgId Closed Subscriber Group identity
     */
    void SetCsgId(uint32_t csgId);

    /**
     *
     * @return csgId Closed Subscriber Group identity
     */
    uint32_t GetCsgId() const;

    /**
     * Set the AS SAP provider to interact with the NAS entity
     *
     * @param s the AS SAP provider
     */
    void SetAsSapProvider(NrAsSapProvider* s);

    /**
     *
     *
     * @return the AS SAP user exported by this RRC
     */
    NrAsSapUser* GetAsSapUser();

    /**
     * set the callback used to forward data packets up the stack
     *
     * @param cb the callback
     */
    void SetForwardUpCallback(Callback<void, Ptr<Packet>> cb);

    /**
     * @brief Causes NAS to tell AS to find a suitable cell and camp to it.
     *
     * @param arfcn the DL frequency of the gNB
     */
    void StartCellSelection(uint32_t arfcn);

    /**
     * @brief Causes NAS to tell AS to go to ACTIVE state.
     *
     * The end result is equivalent with EMM Registered + ECM Connected states.
     */
    void Connect();

    /**
     * @brief Causes NAS to tell AS to camp to a specific cell and go to ACTIVE
     *        state.
     * @param cellId the id of the gNB to camp on
     * @param arfcn the DL frequency of the gNB
     *
     * The end result is equivalent with EMM Registered + ECM Connected states.
     * Since RRC Idle Mode cell selection is not supported yet, we force the UE
     * RRC to be camped on a specific gNB.
     */
    void Connect(uint16_t cellId, uint32_t arfcn);

    /**
     * instruct the NAS to disconnect
     *
     */
    void Disconnect();

    /**
     * Activate an EPS bearer
     *
     * @param bearer the characteristics of the bearer to be created
     * @param rule the QoS rule identifying the traffic that will go on this bearer
     */
    void ActivateEpsBearer(NrEpsBearer bearer, Ptr<NrQosRule> rule);

    /**
     * Enqueue an IP packet on the proper bearer for uplink transmission
     *
     * @param p the packet
     * @param protocolNumber the protocol number of the packet
     *
     * @return true if successful, false if an error occurred
     */
    bool Send(Ptr<Packet> p, uint16_t protocolNumber);

    /**
     * Definition of NAS states as per "LTE - From theory to practice",
     * Section 3.2.3.2 "Connection Establishment and Release"
     *
     */
    enum State
    {
        OFF = 0,
        ATTACHING,
        IDLE_REGISTERED,
        CONNECTING_TO_EPC,
        ACTIVE,
        NUM_STATES
    };

    /**
     * @return The current state
     */
    State GetState() const;

    /**
     * TracedCallback signature for state change events.
     *
     * @param [in] oldState The old State.
     * @param [in] newState the new State.
     */
    typedef void (*StateTracedCallback)(const State oldState, const State newState);

  private:
    // NR AS SAP methods
    /// Notify successful connection
    void DoNotifyConnectionSuccessful();
    /// Notify connection failed
    void DoNotifyConnectionFailed();
    /// Notify connection released
    void DoNotifyConnectionReleased();
    /**
     * Receive data
     * @param packet the packet
     */
    void DoRecvData(Ptr<Packet> packet);

    // internal methods
    /**
     * Activate EPS Bearer
     * @param bearer the EPS bearer
     * @param rule the QoS rule
     */
    void DoActivateEpsBearer(NrEpsBearer bearer, Ptr<NrQosRule> rule);
    /**
     * Switch the UE RRC to the given state.
     * @param s the destination state
     */
    void SwitchToState(State s);

    /// The current UE NAS state.
    State m_state;

    /**
     * The `StateTransition` trace source. Fired upon every UE NAS state
     * transition. Exporting old state and new state.
     * @todo This should be a TracedValue
     */
    TracedCallback<State, State> m_stateTransitionCallback;

    /// The UE NetDevice.
    Ptr<NetDevice> m_device;

    /// The unique UE identifier.
    uint64_t m_imsi;

    /// Closed Subscriber Group identity.
    uint32_t m_csgId;

    /// NR SAP provider
    NrAsSapProvider* m_asSapProvider;
    /// NR SAP user
    NrAsSapUser* m_asSapUser;

    uint8_t m_bidCounter;                    ///< bid counter
    NrQosRuleClassifier m_qosRuleClassifier; ///< QoS rule classifier

    Callback<void, Ptr<Packet>> m_forwardUpCallback; ///< upward callback

    /// BearerToBeActivated structure
    struct BearerToBeActivated
    {
        NrEpsBearer bearer;  ///< EPS bearer
        Ptr<NrQosRule> rule; ///< QoS rule
    };

    std::list<BearerToBeActivated> m_bearersToBeActivatedList; ///< bearers to be activated list

    /**
     * bearers to be activated list maintained and to be used for reconnecting
     * an out-of-sync UE
     *
     */
    std::list<BearerToBeActivated> m_bearersToBeActivatedListForReconnection;
};

} // namespace ns3

#endif // NR_EPC_UE_NAS_H
