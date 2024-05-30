/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_RRC_PROTOCOL_IDEAL_H
#define NR_RRC_PROTOCOL_IDEAL_H

#include <ns3/lte-rrc-sap.h>
#include <ns3/object.h>
#include <ns3/ptr.h>

#include <map>

namespace ns3
{

class LteUeRrcSapProvider;
class LteUeRrcSapUser;
class LteEnbRrcSapProvider;
class LteUeRrc;

/**
 * \ingroup ue
 * \ingroup gnb
 *
 * \brief RRC message passing from the UE to the GNB
 *
 * Models the transmission of RRC messages from the UE to the gNB in
 * an ideal fashion, without errors and without consuming any radio
 * resources.
 *
 */
class nrUeRrcProtocolIdeal : public Object
{
    friend class MemberLteUeRrcSapUser<nrUeRrcProtocolIdeal>;

  public:
    /**
     * \brief nrUeRrcProtocolIdeal constructor
     */
    nrUeRrcProtocolIdeal();
    /**
     * \brief ~nrUeRrcProtocolIdeal
     */
    ~nrUeRrcProtocolIdeal() override;

    // inherited from Object
    void DoDispose() override;
    /**
     * \brief GetTypeId
     * \return the type id of the object
     */
    static TypeId GetTypeId();

    /**
     * \brief SetLteUeRrcSapProvider
     * \param p
     */
    void SetLteUeRrcSapProvider(LteUeRrcSapProvider* p);
    /**
     * \brief GetLteUeRrcSapUser
     * \return
     */
    LteUeRrcSapUser* GetLteUeRrcSapUser();

    /**
     * \brief SetUeRrc
     * \param rrc
     */
    void SetUeRrc(Ptr<LteUeRrc> rrc);

  private:
    // methods forwarded from LteUeRrcSapUser
    void DoSetup(LteUeRrcSapUser::SetupParameters params);
    void DoSendRrcConnectionRequest(LteRrcSap::RrcConnectionRequest msg);
    void DoSendRrcConnectionSetupCompleted(LteRrcSap::RrcConnectionSetupCompleted msg);
    void DoSendRrcConnectionReconfigurationCompleted(
        LteRrcSap::RrcConnectionReconfigurationCompleted msg);
    void DoSendRrcConnectionReestablishmentRequest(
        LteRrcSap::RrcConnectionReestablishmentRequest msg);
    void DoSendRrcConnectionReestablishmentComplete(
        LteRrcSap::RrcConnectionReestablishmentComplete msg);
    void DoSendMeasurementReport(LteRrcSap::MeasurementReport msg);
    /**
     * \brief Send Ideal UE context remove request function
     *
     * Notify eNodeB to release UE context once radio link failure
     * or random access failure is detected. It is needed since no
     * RLF detection mechanism at eNodeB is implemented
     *
     * \param rnti the RNTI of the UE
     */
    void DoSendIdealUeContextRemoveRequest(uint16_t rnti);

    void SetEnbRrcSapProvider();

    Ptr<LteUeRrc> m_rrc;
    uint16_t m_rnti;
    LteUeRrcSapProvider* m_ueRrcSapProvider;
    LteUeRrcSapUser* m_ueRrcSapUser;
    LteEnbRrcSapProvider* m_enbRrcSapProvider;
};

/**
 * Models the transmission of RRC messages from the UE to the gNB in
 * an ideal fashion, without errors and without consuming any radio
 * resources.
 *
 */
class NrGnbRrcProtocolIdeal : public Object
{
    friend class MemberLteEnbRrcSapUser<NrGnbRrcProtocolIdeal>;

  public:
    NrGnbRrcProtocolIdeal();
    ~NrGnbRrcProtocolIdeal() override;

    // inherited from Object
    void DoDispose() override;
    static TypeId GetTypeId();

    void SetLteEnbRrcSapProvider(LteEnbRrcSapProvider* p);
    LteEnbRrcSapUser* GetLteEnbRrcSapUser();

    LteUeRrcSapProvider* GetUeRrcSapProvider(uint16_t rnti);
    void SetUeRrcSapProvider(uint16_t rnti, LteUeRrcSapProvider* p);

  private:
    // methods forwarded from LteEnbRrcSapUser
    void DoSetupUe(uint16_t rnti, LteEnbRrcSapUser::SetupUeParameters params);
    void DoRemoveUe(uint16_t rnti);
    void DoSendSystemInformation(uint16_t cellId, LteRrcSap::SystemInformation msg);
    void SendSystemInformation(uint16_t cellId, LteRrcSap::SystemInformation msg);
    void DoSendRrcConnectionSetup(uint16_t rnti, LteRrcSap::RrcConnectionSetup msg);
    void DoSendRrcConnectionReconfiguration(uint16_t rnti,
                                            LteRrcSap::RrcConnectionReconfiguration msg);
    void DoSendRrcConnectionReestablishment(uint16_t rnti,
                                            LteRrcSap::RrcConnectionReestablishment msg);
    void DoSendRrcConnectionReestablishmentReject(
        uint16_t rnti,
        LteRrcSap::RrcConnectionReestablishmentReject msg);
    void DoSendRrcConnectionRelease(uint16_t rnti, LteRrcSap::RrcConnectionRelease msg);
    void DoSendRrcConnectionReject(uint16_t rnti, LteRrcSap::RrcConnectionReject msg);
    Ptr<Packet> DoEncodeHandoverPreparationInformation(LteRrcSap::HandoverPreparationInfo msg);
    LteRrcSap::HandoverPreparationInfo DoDecodeHandoverPreparationInformation(Ptr<Packet> p);
    Ptr<Packet> DoEncodeHandoverCommand(LteRrcSap::RrcConnectionReconfiguration msg);
    LteRrcSap::RrcConnectionReconfiguration DoDecodeHandoverCommand(Ptr<Packet> p);

    uint16_t m_rnti;
    LteEnbRrcSapProvider* m_enbRrcSapProvider;
    LteEnbRrcSapUser* m_enbRrcSapUser;
    std::map<uint16_t, LteUeRrcSapProvider*> m_enbRrcSapProviderMap;
};

} // namespace ns3

#endif // NR_RRC_PROTOCOL_IDEAL_H
