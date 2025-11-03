// Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.cat>

#ifndef NR_EPC_S11_SAP_H
#define NR_EPC_S11_SAP_H

#include "nr-qos-flow.h"
#include "nr-qos-rule.h"

#include "ns3/address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>

namespace ns3
{

/**
 * NrEpcS11Sap
 */
class NrEpcS11Sap
{
  public:
    virtual ~NrEpcS11Sap() = default;

    /**
     * GTPC message
     */
    struct GtpcMessage
    {
        uint32_t teid; ///< TEID
    };

    /**
     * Fully-qualified TEID, see 3GPP TS 29.274 section 8.22
     */
    struct Fteid
    {
        uint32_t teid;       ///< TEID
        Ipv4Address address; ///< IP address
    };

    /**
     * TS 29.274 8.21  User Location Information (ULI)
     */
    struct Uli
    {
        uint16_t gci; ///< GCI
    };
};

/**
 * @ingroup nr
 *
 * MME side of the S11 Service Access Point (SAP), provides the MME
 * methods to be called when an S11 message is received by the MME.
 */
class NrEpcS11SapMme : public NrEpcS11Sap
{
  public:
    /**
     * 3GPP TS 29.274 version 8.3.1 Release 8 section 8.28
     */
    struct FlowContextCreated
    {
        NrEpcS11Sap::Fteid sgwFteid; ///< EPC FTEID
        uint8_t qfi;                 ///< QoS flow ID
        NrQosFlow flow;              ///< QoS flow
        Ptr<NrQosRule> rule;         ///< QoS rule
    };

    /**
     * Create Session Response message, see 3GPP TS 29.274 7.2.2
     */
    struct CreateSessionResponseMessage : public GtpcMessage
    {
        std::list<FlowContextCreated> bearerContextsCreated; ///< bearer contexts created
    };

    /**
     * send a Create Session Response message
     *
     * @param msg the message
     */
    virtual void CreateSessionResponse(CreateSessionResponseMessage msg) = 0;

    /**
     * Flow Context Removed structure
     */
    struct FlowContextRemoved
    {
        uint8_t qosFlowId; ///< QoS Flow ID
    };

    /**
     * Delete Flow Request message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.9.2
     */
    struct DeleteFlowRequestMessage : public GtpcMessage
    {
        std::list<FlowContextRemoved> bearerContextsRemoved; ///< list of bearer context removed
    };

    /**
     * @brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Flow Request message shall be sent
     * on the S11 interface by PGW to SGW and from SGW to MME
     * @param msg the message
     */
    virtual void DeleteFlowRequest(DeleteFlowRequestMessage msg) = 0;

    /**
     * Modify Flow Response message, see 3GPP TS 29.274 7.2.7
     */
    struct ModifyFlowResponseMessage : public GtpcMessage
    {
        /// Cause enumeration
        enum Cause
        {
            REQUEST_ACCEPTED = 0,
            REQUEST_ACCEPTED_PARTIALLY,
            REQUEST_REJECTED,
            CONTEXT_NOT_FOUND
        };

        Cause cause; ///< the cause
    };

    /**
     * Send a Modify Flow Response message
     *
     * @param msg the message
     */
    virtual void ModifyFlowResponse(ModifyFlowResponseMessage msg) = 0;
};

/**
 * @ingroup nr
 *
 * SGW side of the S11 Service Access Point (SAP), provides the SGW
 * methods to be called when an S11 message is received by the SGW.
 */
class NrEpcS11SapSgw : public NrEpcS11Sap
{
  public:
    /// FlowContextToBeCreated structure
    struct FlowContextToBeCreated
    {
        NrEpcS11Sap::Fteid sgwFteid; ///< FTEID
        uint8_t qfi;                 ///< QoS Flow ID
        NrQosFlow flow;              ///< QoS flow
        Ptr<NrQosRule> rule;         ///< QoS rule
    };

    /**
     * Create Session Request message, see 3GPP TS 29.274 7.2.1
     */
    struct CreateSessionRequestMessage : public GtpcMessage
    {
        uint64_t imsi; ///< IMSI
        Uli uli;       ///< ULI
        std::list<FlowContextToBeCreated>
            bearerContextsToBeCreated; ///< list of bearer contexts to be created
    };

    /**
     * Send a Create Session Request message
     *
     * @param msg the message
     */
    virtual void CreateSessionRequest(CreateSessionRequestMessage msg) = 0;

    /// FlowContextToBeCreated structure
    struct FlowContextToBeRemoved
    {
        uint8_t qosFlowId; ///< QoS flow ID
    };

    /**
     * Delete Flow Command message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.17.1
     */
    struct DeleteFlowCommandMessage : public GtpcMessage
    {
        std::list<FlowContextToBeRemoved>
            bearerContextsToBeRemoved; ///< list of bearer contexts to be removed
    };

    /**
     * @brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Flow Command message shall be sent
     * on the S11 interface by the MME to the SGW
     * @param msg the DeleteFlowCommandMessage
     */
    virtual void DeleteFlowCommand(DeleteFlowCommandMessage msg) = 0;

    /// FlowContextRemovedSgwPgw structure
    struct FlowContextRemovedSgwPgw
    {
        uint8_t qosFlowId; ///< QoS flow ID
    };

    /**
     * Delete Flow Response message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.10.2
     */
    struct DeleteFlowResponseMessage : public GtpcMessage
    {
        std::list<FlowContextRemovedSgwPgw>
            bearerContextsRemoved; ///< list of bearer contexts removed
    };

    /**
     * @brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Flow Command message shall be sent
     * on the S11 interface by the MME to the SGW
     * @param msg the message
     */
    virtual void DeleteFlowResponse(DeleteFlowResponseMessage msg) = 0;

    /**
     * Modify Flow Request message, see 3GPP TS 29.274 7.2.7
     */
    struct ModifyFlowRequestMessage : public GtpcMessage
    {
        Uli uli; ///< ULI
    };

    /**
     * Send a Modify Flow Request message
     *
     * @param msg the message
     */
    virtual void ModifyFlowRequest(ModifyFlowRequestMessage msg) = 0;
};

/**
 * Template for the implementation of the NrEpcS11SapMme as a member
 * of an owner class of type C to which all methods are forwarded
 */
template <class C>
class NrMemberEpcS11SapMme : public NrEpcS11SapMme
{
  public:
    /**
     * Constructor
     *
     * @param owner the owner class
     */
    NrMemberEpcS11SapMme(C* owner);

    // Delete default constructor to avoid misuse
    NrMemberEpcS11SapMme() = delete;

    // inherited from NrEpcS11SapMme
    void CreateSessionResponse(CreateSessionResponseMessage msg) override;
    void ModifyFlowResponse(ModifyFlowResponseMessage msg) override;
    void DeleteFlowRequest(DeleteFlowRequestMessage msg) override;

  private:
    C* m_owner; ///< owner class
};

template <class C>
NrMemberEpcS11SapMme<C>::NrMemberEpcS11SapMme(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
NrMemberEpcS11SapMme<C>::CreateSessionResponse(CreateSessionResponseMessage msg)
{
    m_owner->DoCreateSessionResponse(msg);
}

template <class C>
void
NrMemberEpcS11SapMme<C>::DeleteFlowRequest(DeleteFlowRequestMessage msg)
{
    m_owner->DoDeleteFlowRequest(msg);
}

template <class C>
void
NrMemberEpcS11SapMme<C>::ModifyFlowResponse(ModifyFlowResponseMessage msg)
{
    m_owner->DoModifyFlowResponse(msg);
}

/**
 * Template for the implementation of the NrEpcS11SapSgw as a member
 * of an owner class of type C to which all methods are forwarded
 */
template <class C>
class NrMemberEpcS11SapSgw : public NrEpcS11SapSgw
{
  public:
    /**
     * Constructor
     *
     * @param owner the owner class
     */
    NrMemberEpcS11SapSgw(C* owner);

    // Delete default constructor to avoid misuse
    NrMemberEpcS11SapSgw() = delete;

    // inherited from NrEpcS11SapSgw
    void CreateSessionRequest(CreateSessionRequestMessage msg) override;
    void ModifyFlowRequest(ModifyFlowRequestMessage msg) override;
    void DeleteFlowCommand(DeleteFlowCommandMessage msg) override;
    void DeleteFlowResponse(DeleteFlowResponseMessage msg) override;

  private:
    C* m_owner; ///< owner class
};

template <class C>
NrMemberEpcS11SapSgw<C>::NrMemberEpcS11SapSgw(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
NrMemberEpcS11SapSgw<C>::CreateSessionRequest(CreateSessionRequestMessage msg)
{
    m_owner->DoCreateSessionRequest(msg);
}

template <class C>
void
NrMemberEpcS11SapSgw<C>::ModifyFlowRequest(ModifyFlowRequestMessage msg)
{
    m_owner->DoModifyFlowRequest(msg);
}

template <class C>
void
NrMemberEpcS11SapSgw<C>::DeleteFlowCommand(DeleteFlowCommandMessage msg)
{
    m_owner->DoDeleteFlowCommand(msg);
}

template <class C>
void
NrMemberEpcS11SapSgw<C>::DeleteFlowResponse(DeleteFlowResponseMessage msg)
{
    m_owner->DoDeleteFlowResponse(msg);
}

} // namespace ns3

#endif /* NR_EPC_S11_SAP_H */
