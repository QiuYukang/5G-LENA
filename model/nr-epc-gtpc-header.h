// Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#ifndef NR_EPC_GTPC_HEADER_H
#define NR_EPC_GTPC_HEADER_H

#include "nr-qos-flow.h"
#include "nr-qos-rule.h"

#include "ns3/header.h"

namespace ns3
{

/**
 * @ingroup nr
 *
 * @brief Header of the GTPv2-C protocol
 *
 * Implementation of the GPRS Tunnelling Protocol for Control Plane (GTPv2-C) header
 * according to the 3GPP TS 29.274 document
 *
 * Note, this has been ported to 5G NR terminology to replace concepts such as the EPS
 * bearer with the QoS flow, as has been updated in other classes in this module.
 * However, this may eventually be removed from 5G NR module in favor of newer
 * standards in the TS 29.500 series that use HTTP/2 protocol rather than GTP-C.
 */
class NrGtpcHeader : public Header
{
  public:
    NrGtpcHeader();
    ~NrGtpcHeader() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;

    /**
     * Get the message size.
     *
     * Subclasses are supposed to have a message size greater than zero.
     *
     * @returns the message size
     */
    virtual uint32_t GetMessageSize() const;

    /**
     * Get message type
     * @returns the message type
     */
    uint8_t GetMessageType() const;
    /**
     * Get message length
     * @returns the message length
     */
    uint16_t GetMessageLength() const;
    /**
     * Get TEID
     * @returns the TEID
     */
    uint32_t GetTeid() const;
    /**
     * Get sequence number
     * @returns the sequence number
     */
    uint32_t GetSequenceNumber() const;

    /**
     * Set message type
     * @param messageType the message type
     */
    void SetMessageType(uint8_t messageType);
    /**
     * Set message length
     * @param messageLength the message length
     */
    void SetMessageLength(uint16_t messageLength);
    /**
     * Set TEID
     * @param teid the TEID
     */
    void SetTeid(uint32_t teid);
    /**
     * Set sequence number
     * @param sequenceNumber the sequence number
     */
    void SetSequenceNumber(uint32_t sequenceNumber);
    /**
     * Set IEs length. It is used to compute the message length
     * @param iesLength the IEs length
     */
    void SetIesLength(uint16_t iesLength);

    /**
     * Compute the message length according to the message type
     */
    void ComputeMessageLength();

    /// Interface Type enumeration
    enum InterfaceType_t
    {
        S1U_GNB_GTPU = 0,
        S5_SGW_GTPU = 4,
        S5_PGW_GTPU = 5,
        S5_SGW_GTPC = 6,
        S5_PGW_GTPC = 7,
        S11_MME_GTPC = 10,
    };

    /// FTEID structure
    struct Fteid_t
    {
        InterfaceType_t interfaceType{}; //!< Interface type
        Ipv4Address addr;                //!< IPv4 address
        uint32_t teid;                   //!< TEID
    };

    /// Message Type enumeration
    enum MessageType_t
    {
        Reserved = 0,
        CreateSessionRequest = 32,
        CreateSessionResponse = 33,
        ModifyFlowRequest = 34,
        ModifyFlowResponse = 35,
        DeleteSessionRequest = 36,
        DeleteSessionResponse = 37,
        DeleteFlowCommand = 66,
        DeleteFlowRequest = 99,
        DeleteFlowResponse = 100,
    };

  private:
    /**
     * TEID flag.
     * This flag indicates if TEID field is present or not
     */
    bool m_teidFlag;
    /**
     * Message type field.
     * It can be one of the values of MessageType_t
     */
    uint8_t m_messageType;
    /**
     * Message length field.
     * This field indicates the length of the message in octets excluding
     * the mandatory part of the GTP-C header (the first 4 octets)
     */
    uint16_t m_messageLength;
    /**
     * Tunnel Endpoint Identifier (TEID) field
     */
    uint32_t m_teid;
    /**
     * GTP Sequence number field
     */
    uint32_t m_sequenceNumber;

  protected:
    /**
     * Serialize the GTP-C header in the GTP-C messages
     * @param i the buffer iterator
     */
    void PreSerialize(Buffer::Iterator& i) const;
    /**
     * Deserialize the GTP-C header in the GTP-C messages
     * @param i the buffer iterator
     * @return number of bytes deserialized
     */
    uint32_t PreDeserialize(Buffer::Iterator& i);
};

/**
 * @ingroup nr
 * GTP-C Information Elements
 */
class NrGtpcIes
{
  public:
    /**
     * Cause
     */
    enum Cause_t
    {
        RESERVED = 0,
        REQUEST_ACCEPTED = 16,
    };

    const uint32_t serializedSizeImsi = 12;    //!< IMSI serialized size
    const uint32_t serializedSizeCause = 6;    //!< Cause serialized size
    const uint32_t serializedSizeQfi = 5;      //!< QFI serialized size
    const uint32_t serializedSizeQosFlow = 26; //!< QoS Flow serialized size
    const uint32_t serializedSizePacketFilter =
        2 + 9 + 9 + 5 + 5 + 3; //!< Packet filter serialized size
    /**
     * @return the QoS rule serialized size
     * @param packetFilters The packet filter
     */
    uint32_t GetSerializedSizeQosRule(std::list<NrQosRule::PacketFilter> packetFilters) const;
    const uint32_t serializedSizeUliEcgi = 12;          //!< UliEcgi serialized size
    const uint32_t serializedSizeFteid = 13;            //!< Fteid serialized size
    const uint32_t serializedSizeFlowContextHeader = 4; //!< Fteid serialized size

    /**
     * Serialize the IMSI
     * @param i Buffer iterator
     * @param imsi The IMSI
     */
    void SerializeImsi(Buffer::Iterator& i, uint64_t imsi) const;
    /**
     * Deserialize the IMSI
     * @param i Buffer iterator
     * @param [out] imsi The IMSI
     * @return the number of deserialized bytes
     */
    uint32_t DeserializeImsi(Buffer::Iterator& i, uint64_t& imsi) const;

    /**
     * Serialize the Cause
     * @param i Buffer iterator
     * @param cause The Cause
     */
    void SerializeCause(Buffer::Iterator& i, Cause_t cause) const;
    /**
     * Deserialize the Cause
     * @param i Buffer iterator
     * @param [out] cause The cause
     * @return the number of deserialized bytes
     */
    uint32_t DeserializeCause(Buffer::Iterator& i, Cause_t& cause) const;

    /**
     * Serialize the QoS Flow Id
     * @param i Buffer iterator
     * @param  qfi The QoS Flow Id
     */
    void SerializeQfi(Buffer::Iterator& i, uint8_t qfi) const;
    /**
     * Deserialize the QoS Flow Id
     * @param i Buffer iterator
     * @param [out] qfi The QoS Flow Id
     * @return the number of deserialized bytes
     */
    uint32_t DeserializeQfi(Buffer::Iterator& i, uint8_t& qfi) const;

    /**
     * @param i Buffer iterator
     * @param data data to write in buffer
     *
     * Write the data in buffer and advance the iterator position
     * by five bytes. The data is written in network order and the
     * input data is expected to be in host order.
     */
    void WriteHtonU40(Buffer::Iterator& i, uint64_t data) const;
    /**
     * @param i Buffer iterator
     * @return the five bytes read in the buffer.
     *
     * Read data and advance the Iterator by the number of bytes
     * read.
     * The data is read in network format and returned in host format.
     */
    uint64_t ReadNtohU40(Buffer::Iterator& i);

    /**
     * Serialize the QoS flow
     * @param i Buffer iterator
     * @param flow The QoS Flow
     */
    void SerializeQosFlow(Buffer::Iterator& i, NrQosFlow qosFlow) const;
    /**
     * Deserialize the QoS flow
     * @param i Buffer iterator
     * @param [out] flow The QoS Flow
     * @return the number of deserialized bytes
     */
    uint32_t DeserializeQosFlow(Buffer::Iterator& i, NrQosFlow& flow);

    /**
     * Serialize the QoS rule
     * @param i Buffer iterator
     * @param precedence the QoS rule precedence
     * @param packetFilters The Packet filters
     */
    void SerializeQosRule(Buffer::Iterator& i,
                          uint8_t precedence,
                          std::list<NrQosRule::PacketFilter> packetFilters) const;
    /**
     * Deserialize the QoS rule
     * @param i Buffer iterator
     * @param [out] rule The  QoS rule
     * @return the number of deserialized bytes
     */
    uint32_t DeserializeQosRule(Buffer::Iterator& i, Ptr<NrQosRule> rule) const;

    /**
     * Serialize the UliEcgi
     * @param i Buffer iterator
     * @param uliEcgi The UliEcgi
     */
    void SerializeUliEcgi(Buffer::Iterator& i, uint32_t uliEcgi) const;
    /**
     * Deserialize the UliEcgi
     * @param i Buffer iterator
     * @param [out] uliEcgi UliEcgi
     * @return the number of deserialized bytes
     */
    uint32_t DeserializeUliEcgi(Buffer::Iterator& i, uint32_t& uliEcgi) const;

    /**
     * Serialize the Fteid_t
     * @param i Buffer iterator
     * @param fteid The Fteid_t
     */
    void SerializeFteid(Buffer::Iterator& i, NrGtpcHeader::Fteid_t fteid) const;
    /**
     * Deserialize the Fteid
     * @param i Buffer iterator
     * @param [out] fteid Fteid
     * @return the number of deserialized bytes
     */
    uint32_t DeserializeFteid(Buffer::Iterator& i, NrGtpcHeader::Fteid_t& fteid) const;

    /**
     * Serialize the Flow Context Header
     * @param i Buffer iterator
     * @param length The length
     */
    void SerializeFlowContextHeader(Buffer::Iterator& i, uint16_t length) const;
    /**
     * Deserialize the Flow Context Header
     * @param i Buffer iterator
     * @param [out] length length
     * @return the number of deserialized bytes
     */
    uint32_t DeserializeFlowContextHeader(Buffer::Iterator& i, uint16_t& length) const;
};

/**
 * @ingroup nr
 * GTP-C Create Session Request Message
 */
class NrGtpcCreateSessionRequestMessage : public NrGtpcHeader, public NrGtpcIes
{
  public:
    NrGtpcCreateSessionRequestMessage();
    ~NrGtpcCreateSessionRequestMessage() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;
    uint32_t GetMessageSize() const override;

    /**
     * Get the IMSI
     * @return IMSI
     */
    uint64_t GetImsi() const;
    /**
     * Set the IMSI
     * @param imsi IMSI
     */
    void SetImsi(uint64_t imsi);

    /**
     * Get the UliEcgi
     * @return UliEcgi
     */
    uint32_t GetUliEcgi() const;
    /**
     * Set the UliEcgi
     * @param uliEcgi UliEcgi
     */
    void SetUliEcgi(uint32_t uliEcgi);

    /**
     * Get the Sender CpFteid
     * @return Sender CpFteid
     */
    NrGtpcHeader::Fteid_t GetSenderCpFteid() const;
    /**
     * Set the Sender CpFteid
     * @param fteid Sender CpFteid
     */
    void SetSenderCpFteid(NrGtpcHeader::Fteid_t fteid);

    /**
     * Flow Context structure
     */
    struct FlowContextToBeCreated
    {
        NrGtpcHeader::Fteid_t sgwS5uFteid; ///< FTEID
        uint8_t qfi;                       ///< QoS Flow ID
        Ptr<NrQosRule> rule;               ///< QoS rule
        NrQosFlow flow;                    ///< QoS flow
    };

    /**
     * Get the Flow Contexts
     * @return the Flow Context list
     */
    std::list<FlowContextToBeCreated> GetFlowContextsToBeCreated() const;
    /**
     * Set the Flow Contexts
     * @param flowContexts the Flow Context list
     */
    void SetFlowContextsToBeCreated(std::list<FlowContextToBeCreated> flowContexts);

  private:
    uint64_t m_imsi;                       //!< IMSI
    uint32_t m_uliEcgi;                    //!< UliEcgi
    NrGtpcHeader::Fteid_t m_senderCpFteid; //!< Sender CpFteid

    /// Flow Context list
    std::list<FlowContextToBeCreated> m_flowContextsToBeCreated;
};

/**
 * @ingroup nr
 * GTP-C Create Session Response Message
 */
class NrGtpcCreateSessionResponseMessage : public NrGtpcHeader, public NrGtpcIes
{
  public:
    NrGtpcCreateSessionResponseMessage();
    ~NrGtpcCreateSessionResponseMessage() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;
    uint32_t GetMessageSize() const override;

    /**
     * Get the Cause
     * @return the Cause
     */
    Cause_t GetCause() const;
    /**
     * Set the Cause
     * @param cause The cause
     */
    void SetCause(Cause_t cause);

    /**
     * Get the Sender CpFteid
     * @return the Sender CpFteid
     */
    NrGtpcHeader::Fteid_t GetSenderCpFteid() const;
    /**
     * Set the Sender CpFteid
     * @param fteid the Sender CpFteid
     */
    void SetSenderCpFteid(NrGtpcHeader::Fteid_t fteid);

    /**
     * Flow Context structure
     */
    struct FlowContextCreated
    {
        uint8_t qfi;                 ///< QoS Flow ID
        uint8_t cause;               ///< Cause
        Ptr<NrQosRule> rule;         ///< QoS Rule
        NrGtpcHeader::Fteid_t fteid; ///< FTEID
        NrQosFlow flow;              ///< QoS Flow
    };

    /**
     * Get the Container of Flow Contexts
     * @return a list of Flow Contexts
     */
    std::list<FlowContextCreated> GetFlowContextsCreated() const;
    /**
     * Set the Flow Contexts
     * @param flowContexts a list of Flow Contexts
     */
    void SetFlowContextsCreated(std::list<FlowContextCreated> flowContexts);

  private:
    Cause_t m_cause;                       //!< Cause
    NrGtpcHeader::Fteid_t m_senderCpFteid; //!< Sender CpFteid
    /// Container of Flow Contexts
    std::list<FlowContextCreated> m_flowContextsCreated;
};

/**
 * @ingroup nr
 * GTP-C Modify Flow Request Message
 */
class NrGtpcModifyFlowRequestMessage : public NrGtpcHeader, public NrGtpcIes
{
  public:
    NrGtpcModifyFlowRequestMessage();
    ~NrGtpcModifyFlowRequestMessage() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;
    uint32_t GetMessageSize() const override;

    /**
     * Get the IMSI
     * @return IMSI
     */
    uint64_t GetImsi() const;
    /**
     * Set the IMSI
     * @param imsi IMSI
     */
    void SetImsi(uint64_t imsi);

    /**
     * Get the UliEcgi
     * @return UliEcgi
     */
    uint32_t GetUliEcgi() const;
    /**
     * Set the UliEcgi
     * @param uliEcgi UliEcgi
     */
    void SetUliEcgi(uint32_t uliEcgi);

    /**
     * Flow Context structure
     */
    struct FlowContextToBeModified
    {
        uint8_t qfi;                 ///< QoS flow ID
        NrGtpcHeader::Fteid_t fteid; ///< FTEID
    };

    /**
     * Get the Flow Contexts
     * @return the Flow Context list
     */
    std::list<FlowContextToBeModified> GetFlowContextsToBeModified() const;
    /**
     * Set the Flow Contexts
     * @param flowContexts the Flow Context list
     */
    void SetFlowContextsToBeModified(std::list<FlowContextToBeModified> flowContexts);

  private:
    uint64_t m_imsi;    //!< IMSI
    uint32_t m_uliEcgi; //!< UliEcgi

    /// Flow Context list
    std::list<FlowContextToBeModified> m_flowContextsToBeModified;
};

/**
 * @ingroup nr
 * GTP-C Modify Flow Response Message
 */
class NrGtpcModifyFlowResponseMessage : public NrGtpcHeader, public NrGtpcIes
{
  public:
    NrGtpcModifyFlowResponseMessage();
    ~NrGtpcModifyFlowResponseMessage() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;
    uint32_t GetMessageSize() const override;

    /**
     * Get the Cause
     * @return the Cause
     */
    Cause_t GetCause() const;
    /**
     * Set the Cause
     * @param cause The cause
     */
    void SetCause(Cause_t cause);

  private:
    Cause_t m_cause; //!< Cause
};

/**
 * @ingroup nr
 * GTP-C Delete Flow Command Message
 */
class NrGtpcDeleteFlowCommandMessage : public NrGtpcHeader, public NrGtpcIes
{
  public:
    NrGtpcDeleteFlowCommandMessage();
    ~NrGtpcDeleteFlowCommandMessage() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;
    uint32_t GetMessageSize() const override;

    /// Flow context
    struct FlowContext
    {
        uint8_t m_qfi; ///< QoS flow ID
    };

    /**
     * Get the Flow contexts
     * @return container of flow contexts
     */
    std::list<FlowContext> GetFlowContexts() const;
    /**
     * Set the Flow contexts
     * @param flowContexts container of beraer contexts
     */
    void SetFlowContexts(std::list<FlowContext> flowContexts);

  private:
    std::list<FlowContext> m_flowContexts; //!< Container of Flow Contexts
};

/**
 * @ingroup nr
 * GTP-C Delete Flow Request Message
 */
class NrGtpcDeleteFlowRequestMessage : public NrGtpcHeader, public NrGtpcIes
{
  public:
    NrGtpcDeleteFlowRequestMessage();
    ~NrGtpcDeleteFlowRequestMessage() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;
    uint32_t GetMessageSize() const override;

    /**
     * Get the QoS Flow IDs
     * @return a container of QoS Flow IDs
     */
    std::list<uint8_t> GetQosFlowIds() const;
    /**
     * Set the QoS Flow IDs
     * @param qosFlowIds The container of QoS Flow IDs
     */
    void SetQosFlowIds(std::list<uint8_t> qosFlowIds);

  private:
    std::list<uint8_t> m_qosFlowIds; //!< Container of QoS Flow IDs
};

/**
 * @ingroup nr
 * GTP-C Delete Flow Response Message
 */
class NrGtpcDeleteFlowResponseMessage : public NrGtpcHeader, public NrGtpcIes
{
  public:
    NrGtpcDeleteFlowResponseMessage();
    ~NrGtpcDeleteFlowResponseMessage() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
    void Print(std::ostream& os) const override;
    uint32_t GetMessageSize() const override;

    /**
     * Get the Cause
     * @return the Cause
     */
    Cause_t GetCause() const;
    /**
     * Set the Cause
     * @param cause The cause
     */
    void SetCause(Cause_t cause);

    /**
     * Get the QoS Flow IDs
     * @return a container of QoS Flow IDs
     */
    std::list<uint8_t> GetQosFlowIds() const;
    /**
     * Set the QoS Flow IDs
     * @param qosFlowIds The container of QoS Flow IDs
     */
    void SetQosFlowIds(std::list<uint8_t> qosFlowIds);

  private:
    Cause_t m_cause;                 //!< Cause
    std::list<uint8_t> m_qosFlowIds; //!< Container of QoS Flow IDs
};

} // namespace ns3

#endif // NR_EPC_GTPC_HEADER_H
