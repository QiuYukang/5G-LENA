/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_SL_UE_MAC_SCHEDULER_H
#define NR_SL_UE_MAC_SCHEDULER_H

#include "nr-sl-phy-mac-common.h"

#include <ns3/nr-sl-mac-sap.h>
#include <ns3/nr-sl-ue-cmac-sap.h>
#include <ns3/object.h>

namespace ns3
{

class NrSlUeMac;

/**
 * \ingroup scheduler
 * \brief Interface for all the NR Sidelink schedulers
 *
 * \see NrSlUeMacSchedulerNs3
 */
class NrSlUeMacScheduler : public Object
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId(void);

    NrSlUeMacScheduler();
    ~NrSlUeMacScheduler() override;

    //
    // SCHED API primitives for NR Sidelink
    // From FAPI 2.0.0 Small Cell Forum originated LTE MAC scheduler API
    //
    /**
     * \brief Starts the UL MAC scheduler for this subframe.
     *
     * Requests scheduling to a destination based on provided sensing information
     *
     * \param dstL2Id The destination layer 2 id
     * \param params List of NrSlSlotInfo (sensing information)
     */
    void SchedNrSlTriggerReq(uint32_t dstL2Id, const std::list<NrSlSlotInfo>& params);

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
     * Assign a fixed random variable stream number to the random variables
     * used by this model.  Return the number of streams (possibly zero) that
     * have been assigned.
     *
     * \param stream first stream index to use
     * \return the number of stream indices assigned by this model
     */
    virtual int64_t AssignStreams(int64_t stream) = 0;

    /**
     * Setter for pointer to NrSlUeMac
     * \param nrSlUeMac Pointer to NrSlUeMac
     */
    void SetNrSlUeMac(Ptr<NrSlUeMac> nrSlUeMac);

    /**
     * Getter for pointer to NrSlUeMac
     * \return Pointer to NrSlUeMac
     */
    Ptr<NrSlUeMac> GetNrSlUeMac(void) const;

  protected:
    void DoDispose() override;

  private:
    // Implementation of SCHED API primitives for NR Sidelink
    /**
     * \brief Starts the UL MAC scheduler for this subframe.
     *
     * Requests scheduling to a destination based on provided sensing information
     *
     * \param dstL2Id The destination layer 2 id
     * \param params List of NrSlSlotInfo (sensing information)
     */
    virtual void DoSchedNrSlTriggerReq(uint32_t dstL2Id, const std::list<NrSlSlotInfo>& params) = 0;
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

    Ptr<NrSlUeMac> m_nrSlUeMac; //!< Pointer to NrSlUeMac instance
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_H */
