// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "nr-mac-scheduler-tdma.h"

#include <unordered_set>

namespace ns3
{

/**
 * @class NrMacSchedulerTdmaRR
 * @brief Implements a Round-Robin (RR) Time-Division Multiple Access (TDMA) MAC scheduler for NR
 * (New Radio).
 *
 * This class extends NrMacSchedulerTdma and provides a Round-Robin scheduling mechanism for user
 * equipment (UE). The scheduler allocates resources to UEs in a cyclic and fair manner using RR
 * principles.
 *
 * Key functionalities include:
 * - Managing UE representations using NrMacSchedulerUeInfoRR.
 * - Providing comparison functions to handle Downlink (DL) and Uplink (UL) UE scheduling based on
 * RR policy.
 * - Updating resource allocation metrics for UEs after DL/UL assignments.
 * - Maintaining relevant data structures to track UEs in the scheduling queue.
 */
class NrMacSchedulerTdmaRR : public NrMacSchedulerTdma
{
  public:
    /**
     * @brief GetTypeId
     * @return The TypeId of the class
     */
    static TypeId GetTypeId();

    /**
     * @brief NrMacSchedulerTdmaRR constructor
     */
    NrMacSchedulerTdmaRR();

    /**
     * @brief ~NrMacSchedulerTdmaRR deconstructor
     */
    ~NrMacSchedulerTdmaRR() override
    {
    }

  protected:
    /**
     * @brief Create an UE representation of the type NrMacSchedulerUeInfoRR
     * @param params parameters
     * @return NrMacSchedulerUeInfoRR instance
     */
    std::shared_ptr<NrMacSchedulerUeInfo> CreateUeRepresentation(
        const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) const override;

    /**
     * @brief Return the comparison function to sort DL UE according to the scheduler policy
     * @return a pointer to NrMacSchedulerUeInfoRR::CompareUeWeightsDl
     */
    std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq& lhs,
                       const NrMacSchedulerNs3::UePtrAndBufferReq& rhs)>
    GetUeCompareDlFn() const override;

    /**
     * @brief Return the comparison function to sort UL UE according to the scheduler policy
     * @return a pointer to NrMacSchedulerUeInfoRR::CompareUeWeightsUl
     */
    std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq& lhs,
                       const NrMacSchedulerNs3::UePtrAndBufferReq& rhs)>
    GetUeCompareUlFn() const override;

    /**
     * @brief Update the UE representation after a symbol (DL) has been assigned to it
     * @param ue UE to which a symbol has been assigned
     * @param assigned the amount of resources assigned
     * @param totAssigned the total amount of resources assigned in the slot
     *
     * Update DL metrics by calling NrMacSchedulerUeInfoRR::UpdateDlMetric
     */
    void AssignedDlResources(const UePtrAndBufferReq& ue,
                             const FTResources& assigned,
                             const FTResources& totAssigned) const override;

    /**
     * @brief Update the UE representation after a symbol (DL) has been assigned to it
     * @param ue UE to which a symbol has been assigned
     * @param assigned the amount of resources assigned
     * @param totAssigned the total amount of resources assigned in the slot
     *
     * Update DL metrics by calling NrMacSchedulerUeInfoRR::UpdateUlMetric
     */
    void AssignedUlResources(const UePtrAndBufferReq& ue,
                             const FTResources& assigned,
                             const FTResources& totAssigned) const override;

    // RR is a simple scheduler: it doesn't do anything in the next
    // inherited calls.
    void NotAssignedDlResources(const UePtrAndBufferReq& ue,
                                const FTResources& notAssigned,
                                const FTResources& totalAssigned) const override
    {
    }

    void NotAssignedUlResources(const UePtrAndBufferReq& ue,
                                const FTResources& notAssigned,
                                const FTResources& totalAssigned) const override
    {
    }

    void BeforeDlSched(const UePtrAndBufferReq& ue,
                       const FTResources& assignableInIteration) const override
    {
    }

    void BeforeUlSched(const UePtrAndBufferReq& ue,
                       const FTResources& assignableInIteration) const override
    {
    }

  private:
    /**
     * Deque used to keep priority order of round-robin.
     * Higher-priority UEs will be at front.
     * Lower-priority UEs will be at end.
     * Active UEs are pulled from anywhere when a new resource is allocated to them,
     * and put at the end whenever the scheduling is done.
     */
    mutable std::deque<uint16_t> m_dlRrRntiDeque;
    mutable std::unordered_set<uint16_t> m_dlRntiSet; ///< Set of known RNTIs in RR deque
};

} // namespace ns3
