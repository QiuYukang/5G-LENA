/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_SL_UE_MAC_SCHEDULER_NS3_H
#define NR_SL_UE_MAC_SCHEDULER_NS3_H

#include "nr-amc.h"
#include "nr-sl-phy-mac-common.h"
#include "nr-sl-ue-mac-scheduler-dst-info.h"
#include "nr-sl-ue-mac-scheduler.h"

#include <ns3/random-variable-stream.h>

#include <functional>
#include <list>
#include <memory>

namespace ns3
{

/**
 * \ingroup scheduler
 *
 * \brief A general scheduler for NR SL UE in NS3
 */
class NrSlUeMacSchedulerNs3 : public NrSlUeMacScheduler
{
  public:
    /**
     * \brief GetTypeId
     *
     * \return The TypeId of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief NrSlUeMacSchedulerNs3 default constructor
     */
    NrSlUeMacSchedulerNs3();

    /**
     * \brief NrSlUeMacSchedulerNs3 destructor
     */
    ~NrSlUeMacSchedulerNs3() override;

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
     * \brief Set the flag if the MCS for NR SL is fixed (in this case,
     *        it will take the initial value)
     *
     * \param fixMcs the flag to indicate if the NR SL MCS is fixed
     *
     * \see SetInitialMcsSl
     */
    void UseFixedNrSlMcs(bool fixMcs);
    /**
     * \brief Check if the MCS in NR SL is fixed
     * \return true if the NR SL MCS is fixed, false otherwise
     */
    bool IsNrSlMcsFixed() const;

    /**
     * \brief Set the initial value for the NR SL MCS
     *
     * \param mcs the MCS value
     */
    void SetInitialNrSlMcs(uint8_t mcs);

    /**
     * \brief Get the SL MCS initial value
     *
     * \return the value
     */
    uint8_t GetInitialNrSlMcs() const;

    /**
     * \brief Get Redundancy Version number
     *
     * We assume rvid = 0, so RV would take 0, 2, 3, 1. See TS 38.21 table 6.1.2.1-2
     *
     * \param txNumTb The transmission index of the TB, e.g., 0 for initial tx,
     *        1 for a first retransmission, and so on.
     * \return The Redundancy Version number
     */
    uint8_t GetRv(uint8_t txNumTb) const;

    /**
     * \brief Assign a fixed random variable stream number to the random variables
     * used by this model. Return the number of streams (possibly zero) that
     * have been assigned.
     *
     * \param stream The first stream index to use
     * \return The number of stream indices assigned by this model
     */
    int64_t AssignStreams(int64_t stream) override;

  protected:
    /**
     * \brief Do the NE Sidelink allocation
     *
     * All the child classes should implement this method
     *
     * For allocating resources to more than one LCs of a
     * destination so they can be multiplexed, one could consider
     * the following procedure.
     *
     * 1. Irrespective of the priority of LCc, sum their buffer size.
     * 2. Compute the TB size using the AMC given the available resources, the
     *    buffer size computed in step 1, and the MCS.
     * 3. Starting from the highest priority LC, distribute the bytes among LCs
     *    from the TB size computed in step 2 as per their buffer status report
     *    until we satisfy all the LCs or the TB size computed in step 2 is fully
     *    consumed. There may be more than one LCs with the same priority, which
     *    could have same or different buffer sizes. In case of equal buffer sizes,
     *    these LCs should be assigned equal number of bytes. If these LCs have
     *    unequal buffer sizes, we can use the minimum buffer size among the LCs
     *    to assign the same bytes.
     *
     * \param params The list of the txOpps from the UE MAC
     * \param dstInfo The pointer to the NrSlUeMacSchedulerDstInfo of the destination
     *        for which UE MAC asked the scheduler to allocate the recourses
     * \param slotAllocList The slot allocation list to be updated by a specific scheduler
     * \return The status of the allocation, true if the destination has been
     *         allocated some resources; false otherwise.
     */
    virtual bool DoNrSlAllocation(const std::list<NrSlSlotInfo>& params,
                                  const std::shared_ptr<NrSlUeMacSchedulerDstInfo>& dstInfo,
                                  std::set<NrSlSlotAlloc>& slotAllocList) = 0;
    Ptr<UniformRandomVariable> m_uniformVariable; //!< Uniform random variable

  private:
    // Implementation of SCHED API primitives for NR Sidelink
    void DoSchedNrSlTriggerReq(uint32_t dstL2Id, const std::list<NrSlSlotInfo>& params) override;
    void DoSchedNrSlRlcBufferReq(
        const struct NrSlMacSapProvider::NrSlReportBufferStatusParameters& params) override;
    // Implementation of CSCHED API primitives for NR Sidelink
    void DoCschedNrSlLcConfigReq(
        const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params) override;

    /**
     * \brief Create destination info
     *
     * If the scheduler does not have the destination info then it creates it,
     * and then save its pointer in the m_dstMap map.
     *
     * If the scheduler already have the destination info, it does noting. This
     * could happen when we are trying add more than one logical channels
     * for a destination.
     *
     * \param params params of the UE
     * \return A std::shared_ptr to newly created NrSlUeMacSchedulerDstInfo
     */
    std::shared_ptr<NrSlUeMacSchedulerDstInfo> CreateDstInfo(
        const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params);

    /**
     * \brief Create a NR Sidelink logical channel group
     *
     * A subclass can return its own representation of a logical channel by
     * implementing a proper subclass of NrSlUeMacSchedulerLCG and returning a
     * pointer to a newly created instance.
     *
     * \param lcGroup The logical channel group id
     * \return a pointer to the representation of a logical channel group
     */
    NrSlLCGPtr CreateLCG(uint8_t lcGroup) const;

    /**
     * \brief Create a NR Sidelink logical channel
     *
     * A subclass can return its own representation of a logical channel by
     * implementing a proper subclass of NrSlUeMacSchedulerLC and returning a
     * pointer to a newly created instance.
     *
     * \param params configuration of the logical channel
     * \return a pointer to the representation of a logical channel
     */

    NrSlLCPtr CreateLC(const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params) const;

    std::unordered_map<uint32_t, std::shared_ptr<NrSlUeMacSchedulerDstInfo>>
        m_dstMap; //!< The map of between destination layer 2 id and the destination info

    Ptr<NrAmc> m_nrSlAmc; //!< AMC pointer for NR SL

    bool m_fixedNrSlMcs{false}; //!< Fixed MCS for *all* the destinations

    uint8_t m_initialNrSlMcs{0}; //!< Initial (or fixed) value for NR SL MCS
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_NS3_H */
