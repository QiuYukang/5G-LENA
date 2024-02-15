/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_SL_UE_MAC_SCHEDULER_H
#define NR_SL_UE_MAC_SCHEDULER_H

#include "nr-sl-ue-mac-csched-sap.h"
#include "nr-sl-ue-mac-sched-sap.h"

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

    /**
     * \brief SetNrSlUeMacCschedSapUser
     * \param sap the pointer to the NR Sidelink UE MAC sap user
     */
    void SetNrSlUeMacCschedSapUser(NrSlUeMacCschedSapUser* sap);

    /**
     * \brief Get the MacCschedSapProvider pointer
     * \return the pointer to the sap provider
     */
    NrSlUeMacCschedSapProvider* GetNrSlUeMacCschedSapProvider();

    //
    // Implementation of the CSCHED API primitives for NR Sidelink
    //
    /**
     * \brief Send the NR Sidelink logical channel configuration from UE MAC to the UE scheduler
     *
     * \param params NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo
     */
    virtual void DoCschedUeNrSlLcConfigReq(
        const struct NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) = 0;

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
    void SchedNrSlRlcBufferReq(const struct NrSlReportBufferStatusParams& params);

    /**
     * \brief Send NR Sidelink RLC buffer status report from UE MAC to the UE scheduler
     *
     * \param params NrSlReportBufferStatusParams
     */
    virtual void DoSchedUeNrSlRlcBufferReq(const struct NrSlReportBufferStatusParams& params) = 0;
    /**
     * \brief Send NR Sidleink trigger request from UE MAC to the UE scheduler
     *
     * \param dstL2Id The destination layer 2 id
     * \param params NrSlSlotInfo
     */
    virtual void DoSchedUeNrSlTriggerReq(uint32_t dstL2Id,
                                         const std::list<NrSlSlotInfo>& params) = 0;

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

    NrSlUeMacCschedSapUser* m_nrSlUeMacCschedSapUser{nullptr};         //!< SAP User
    NrSlUeMacCschedSapProvider* m_nrSlUeMacCschedSapProvider{nullptr}; //!< SAP Provider
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
    virtual void DoSchedNrSlRlcBufferReq(const struct NrSlReportBufferStatusParams& params) = 0;

    Ptr<NrSlUeMac> m_nrSlUeMac; //!< Pointer to NrSlUeMac instance
};

/**
 * \ingroup scheduler
 * \brief Class implementing the NrSlUeMacCschedSapProvider methods
 */
class NrSlUeMacGeneralCschedSapProvider : public NrSlUeMacCschedSapProvider
{
  public:
    /**
     * \brief constructor
     * \param scheduler The pointer the NrSlUeMacScheduler API using this SAP
     */
    NrSlUeMacGeneralCschedSapProvider(NrSlUeMacScheduler* scheduler);

    ~NrSlUeMacGeneralCschedSapProvider() = default;

    // inherited from NrSlUeMacCschedSapProvider

    virtual void CschedUeNrSlLcConfigReq(
        const struct NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) override;

  private:
    NrSlUeMacScheduler* m_scheduler{nullptr}; //!< pointer to the scheduler API using this SAP
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_H */
