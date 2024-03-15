/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only and NIST-Software

#ifndef NR_SL_UE_MAC_SCHEDULER_H
#define NR_SL_UE_MAC_SCHEDULER_H

#include "nr-amc.h"
#include "nr-sl-phy-mac-common.h"

#include <ns3/nr-sl-mac-sap.h>
#include <ns3/nr-sl-ue-cmac-sap.h>
#include <ns3/object.h>

#include <deque>

namespace ns3
{

class NrSlUeMac;

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

    //
    // SCHED API primitives for NR Sidelink
    // From FAPI 2.0.0 Small Cell Forum originated LTE MAC scheduler API
    //
    /**
     * \brief Starts the UL MAC scheduler for this subframe.
     *
     * \param sfn The current SfnSf
     * \param ids available HARQ process IDs from the MAC
     */
    void SchedNrSlTriggerReq(const SfnSf& sfn, const std::deque<uint8_t>& ids);

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
     * \param nrSlUeMac Pointer to NrSlUeMac
     */
    void SetNrSlUeMac(Ptr<NrSlUeMac> nrSlUeMac);

    /**
     * Getter for pointer to NrSlUeMac
     * \return Pointer to NrSlUeMac
     */
    Ptr<NrSlUeMac> GetNrSlUeMac() const;

    /**
     * \brief Install the AMC for the NR Sidelink
     *
     * Usually called by the helper
     *
     * \param nrSlAmc NR Sidelink AMC
     */
    void InstallNrSlAmc(const Ptr<NrAmc>& nrSlAmc);

    /**
     * \brief Get the AMC for NR Sidelink
     *
     * \return the NR Sidelink AMC
     */
    Ptr<const NrAmc> GetNrSlAmc() const;

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

  protected:
    void DoDispose() override;

  private:
    // Implementation of SCHED API primitives for NR Sidelink
    /**
     * \brief Starts the UL MAC scheduler for this subframe.
     *
     * \param sfn The current SfnSf
     * \param ids available HARQ process IDs from the MAC
     */
    virtual void DoSchedNrSlTriggerReq(const SfnSf& sfn, const std::deque<uint8_t>& ids) = 0;

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

    Ptr<NrSlUeMac> m_nrSlUeMac; //!< Pointer to NrSlUeMac instance
    Ptr<NrAmc> m_nrSlAmc;       //!< AMC pointer for NR SL
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_H */
