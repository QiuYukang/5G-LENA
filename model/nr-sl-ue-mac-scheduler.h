/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only and NIST-Software

#ifndef NR_SL_UE_MAC_SCHEDULER_H
#define NR_SL_UE_MAC_SCHEDULER_H

#include "nr-amc.h"
#include "nr-sl-phy-mac-common.h"
#include "nr-sl-ue-mac.h"

#include <ns3/nr-sl-mac-sap.h>
#include <ns3/nr-sl-ue-cmac-sap.h>
#include <ns3/object.h>
#include <ns3/traced-callback.h>

namespace ns3
{

/**
 * \ingroup scheduler
 * \brief Interface for all the NR Sidelink schedulers
 */
class NrSlUeMacScheduler : public Object
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief NrSlUeMacScheduler constructor
     */
    NrSlUeMacScheduler();

    /**
     * \brief NrSlUeMacScheduler destructor
     */
    ~NrSlUeMacScheduler() override;

    /**
     * \brief Internal structure to store grant information
     */
    struct GrantInfo
    {
        uint16_t cReselCounter{
            std::numeric_limits<uint8_t>::max()}; //!< The Cresel counter for the semi-persistently
                                                  //!< scheduled resources as per TS 38.214
        uint8_t slResoReselCounter{
            std::numeric_limits<uint8_t>::max()}; //!< The Sidelink resource re-selection counter
                                                  //!< for the semi-persistently scheduled resources
                                                  //!< as per TS 38.214
        std::set<SlGrantResource>
            slotAllocations; //!< List of all the slots available for transmission with the pool
        uint8_t prevSlResoReselCounter{
            std::numeric_limits<uint8_t>::max()};            //!< Previously drawn Sidelink resource
                                                             //!< re-selection counter
        uint8_t harqId{std::numeric_limits<uint8_t>::max()}; //!< The HARQ process id assigned at
                                                             //!< the time of transmitting new data
        uint8_t nSelected{
            0}; //!< The number of slots selected by the scheduler for first reservation period
        uint8_t tbTxCounter{
            0}; //!< The counter to count the number of time a TB is tx/reTx in a reservation period
        bool isDynamic{false}; //!< true if the grant is for dynamic scheduling (single-PDU), false
                               //!< if it is for semi-persistent scheduling
        bool harqEnabled{false}; //!< true if the grant should use HARQ
        Time rri{0}; //!< The resource reservation interval for the semi-persistent scheduled grant
        SidelinkInfo::CastType castType{SidelinkInfo::CastType::Invalid}; //!< Cast type
    };

    //
    // SCHED API primitives for NR Sidelink
    // From FAPI 2.0.0 Small Cell Forum originated LTE MAC scheduler API
    //
    /**
     * \brief Starts the UL MAC scheduler for this subframe.
     *
     * \param sfn The current SfnSf
     */
    void SchedNrSlTriggerReq(const SfnSf& sfn);

    /**
     * \brief Update buffer status of logical channel data in RLC.
     *
     * \param params Buffer status information
     */
    void SchedNrSlRlcBufferReq(
        const struct NrSlMacSapProvider::NrSlReportBufferStatusParameters& params);

    // CSCHED API primitives for NR Sidelink
    /**
     * \brief Send the NR Sidelink logical channel configuration from UE MAC to the UE scheduler
     *
     * \param params SL logical channel parameters
     */
    void CschedNrSlLcConfigReq(
        const struct NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params);

    /**
     * \brief Remove the NR Sidelink logical channel configuration from the scheduler and propagate
     * to the MAC
     *
     * \param lcid logical channel ID
     * \param dstL2Id destination layer 2 ID
     */
    void RemoveNrSlLcConfigReq(uint8_t lcid, uint32_t dstL2Id);

    /**
     * \brief Set pointer to associated NrSlUeMac object
     * \param ueMac Pointer to NrSlUeMac
     */
    void SetNrSlUeMac(Ptr<NrSlUeMac> ueMac);

    /**
     * Getter for pointer to NrSlUeMac
     * \return Pointer to NrSlUeMac
     */
    Ptr<NrSlUeMac> GetMac() const;

    /**
     * \brief Install the AMC for the NR Sidelink
     *
     * Usually called by the helper
     *
     * \param amc NR Sidelink AMC
     */
    void InstallAmc(const Ptr<NrAmc>& amc);

    /**
     * \brief Get the AMC for NR Sidelink
     *
     * \return the NR Sidelink AMC
     */
    Ptr<const NrAmc> GetAmc() const;

    /**
     * Assign a fixed random variable stream number to the random variables
     * used by this model.  Return the number of streams (possibly zero) that
     * have been assigned.
     *
     * \param stream first stream index to use
     * \return the number of stream indices assigned by this model
     */
    virtual int64_t AssignStreams(int64_t stream) = 0;

    /**
     * \brief Tell the scheduler that an RLC PDU packet has been dequeued and is now in the HARQ
     * buffer
     *
     * \param dstL2Id The destination layer 2 ID
     * \param lcId The logical channel ID
     * \param size The size of the RLC PDU
     */
    void NotifyNrSlRlcPduDequeue(uint32_t dstL2Id, uint8_t lcId, uint32_t size);

    /**
     * TracedCallback signature for report on grant creation
     * \param [in] grant grant information
     * \param [in] psfchPeriod PSFCH period for the configured resource pool
     */
    typedef void (*GrantCreatedTracedCallback)(const struct GrantInfo& grant, uint16_t psfchPeriod);

    /**
     * TracedCallback signature for report on grant publishing
     * \param [in] grant grant information
     * \param [in] psfchPeriod PSFCH period for the configured resource pool
     */
    typedef void (*GrantPublishedTracedCallback)(const struct NrSlUeMac::NrSlGrant& grant,
                                                 uint16_t psfchPeriod);

  protected:
    void DoDispose() override;

    /**
     * Trigger the 'GrantCreated' trace source
     * \param grant Grant information
     */
    void NotifyGrantCreated(const struct GrantInfo& grant) const;

    /**
     * Trigger the 'GrantCreated' trace source
     * \param grant Grant information
     */
    void NotifyGrantPublished(const struct NrSlUeMac::NrSlGrant& grant) const;

  private:
    // Implementation of SCHED API primitives for NR Sidelink
    /**
     * \brief Starts the UL MAC scheduler for this subframe.
     *
     * \param sfn The current SfnSf
     */
    virtual void DoSchedNrSlTriggerReq(const SfnSf& sfn) = 0;

    /**
     * \brief Update buffer status of logical channel data in RLC.
     *
     * \param params Buffer status information
     */
    virtual void DoSchedNrSlRlcBufferReq(
        const struct NrSlMacSapProvider::NrSlReportBufferStatusParameters& params) = 0;
    // Implementation of CSCHED API primitives for NR Sidelink
    /**
     * \brief Send the NR Sidelink logical channel configuration from UE MAC to the UE scheduler
     *
     * \param params SL logical channel parameters
     */
    virtual void DoCschedNrSlLcConfigReq(
        const struct NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params) = 0;
    /**
     * \brief Remove the NR Sidelink logical channel configuration from the scheduler and propagate
     * to the MAC
     *
     * \param lcid logical channel ID
     * \param dstL2Id destination layer 2 ID
     */
    virtual void DoRemoveNrSlLcConfigReq(uint8_t lcid, uint32_t dstL2Id) = 0;

    /**
     * \brief Tell the scheduler that an RLC PDU packet has been dequeued and is now in the HARQ
     * buffer
     *
     * \param dstL2Id The destination layer 2 ID
     * \param lcId The logical channel ID
     * \param size The size of the RLC PDU
     */
    virtual void DoNotifyNrSlRlcPduDequeue(uint32_t dstL2Id, uint8_t lcId, uint32_t size) = 0;

    Ptr<NrSlUeMac> m_ueMac; //!< Pointer to NrSlUeMac instance
    Ptr<NrAmc> m_amc;       //!< AMC pointer

    TracedCallback<const struct GrantInfo&, uint16_t>
        m_grantCreatedTrace; //!< Trace source for grant creation

    TracedCallback<const struct NrSlUeMac::NrSlGrant&, uint16_t>
        m_grantPublishedTrace; //!< Trace source for grant publishing
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_H */
