/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only and NIST-Software

#ifndef NR_SL_UE_MAC_SCHEDULER_FIXED_MCS_H
#define NR_SL_UE_MAC_SCHEDULER_FIXED_MCS_H

#include "nr-sl-phy-mac-common.h"
#include "nr-sl-ue-mac-scheduler-dst-info.h"
#include "nr-sl-ue-mac-scheduler.h"
#include "nr-sl-ue-mac.h"

#include <ns3/random-variable-stream.h>

#include <functional>
#include <list>
#include <memory>

namespace ns3
{

struct AllocationInfo
{
    uint8_t m_priority{0};     //!< Priority
    bool m_isDynamic{false};   //!< Dynamic (per-PDU) scheduling indication (SPS when false)
    bool m_harqEnabled{false}; //!< Whether HARQ is enabled
    uint32_t m_tbSize{0};      //!< The transport block size
    std::vector<SlRlcPduInfo> m_allocatedRlcPdus; //!< RLC PDUs
    Time m_rri{0};                                //!< Resource Reservation Interval (if SPS)
};

/**
 * \ingroup scheduler
 *
 * \brief A general scheduler for NR SL UE that uses a fixed MCS, but supports multiple
 *        logical channels, prioritization, and both dynamic and SPS grants
 */
class NrSlUeMacSchedulerFixedMcs : public NrSlUeMacScheduler
{
  public:
    /**
     * \brief GetTypeId
     *
     * \return The TypeId of the class
     */
    static TypeId GetTypeId(void);

    /**
     * \brief NrSlUeMacSchedulerFixedMcs default constructor
     */
    NrSlUeMacSchedulerFixedMcs();

    /**
     * \brief NrSlUeMacSchedulerFixedMcs destructor
     */
    ~NrSlUeMacSchedulerFixedMcs() override;

  private:
    void DoRemoveNrSlLcConfigReq(uint8_t lcid, uint32_t dstL2Id) override;

    void DoSchedNrSlRlcBufferReq(
        const struct NrSlMacSapProvider::NrSlReportBufferStatusParameters& params) override;

    void DoSchedNrSlTriggerReq(const SfnSf& sfn, const std::deque<uint8_t>& ids) override;

    void DoCschedNrSlLcConfigReq(
        const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo& params) override;

    void DoNotifyNrSlRlcPduDequeue(uint32_t dstL2Id, uint8_t lcId, uint32_t size) override;

    /**
     * \brief Perform the Tx resource (re-)selection check for the given destination and logical
     * channel
     *
     * \param dstL2Id The destination layer 2 ID
     * \param lcId The logical channel ID
     * \return True if the LC passes the check, false otherwise
     */
    bool TxResourceReselectionCheck(uint32_t dstL2Id, uint8_t lcId);
    /**
     * \brief Select the destinations and logical channels that need scheduling
     *
     * The function fills the dstsAndLcsToSched map with the destinations and logical channels that
     * pass the transmission resource (re-)selection check in function TxResourceReselectionCheck
     *
     * \param dstsAndLcsToSched The map of destinations and logical channels IDs to be updated
     */
    void GetDstsAndLcsNeedingScheduling(
        std::map<uint32_t, std::vector<uint8_t>>& dstsAndLcsToSched);
    /**
     * \brief Select the destination and logical channels to be allocated
     *
     * The selection and allocation is done according to TS 38.321 V16.11.0 Section 5.22.1.4.1
     * At the moment prioritized bitrate is not supported, thus the logic implemented in this
     * function assumes sPBR = infinity and sBj > 0 for all LCs.
     *
     * \param sfn The SfnSf
     * \param dstsAndLcsToSched The map of destinations and logical channels IDs to allocate
     * \param allocationInfo the allocation information to be updated
     * \param candResources the list of resources to be updated with the ones used for the
     * allocation
     *
     * \return destination layer 2 ID of the allocated destination if any, zero otherwise
     */
    uint32_t LogicalChannelPrioritization(
        const SfnSf& sfn,
        std::map<uint32_t, std::vector<uint8_t>> dstsAndLcsToSched,
        AllocationInfo& allocationInfo,
        std::list<NrSlSlotInfo>& candResources);
    /**
     * \brief Attempt to select new grant from the selection window
     *
     * If successful, CreateSpsGrant () will be called for SPS grants
     * or CreateSinglePduGrant () for dynamic grants
     *
     * \param dstL2Id The destination layer 2 id
     * \param candResources The list of candidate resources
     * \param ids available HARQ process IDs
     * \param allocatedRlcPdus the RLC PDU information vector
     */
    void AttemptGrantAllocation(uint32_t dstL2Id,
                                const std::list<NrSlSlotInfo>& candResources,
                                const std::deque<uint8_t>& harqIds,
                                AllocationInfo allocationInfo);
    /**
     * \brief Create future SPS grants based on slot allocation
     *
     * \param slotAllocList The slot allocation list
     * \param rri The resource reservation interval
     * \return The grant info for a destination based on the scheduler allocation
     *
     * \see NrSlSlotAlloc
     * \see NrSlUeMac::NrSlGrantInfo
     */
    NrSlUeMac::NrSlGrantInfo CreateSpsGrantInfo(const std::set<NrSlSlotAlloc>& params, Time rri);
    /**
     * \brief Create a single-PDU grant based on slot allocation
     *
     * \param slotAllocList The slot allocation list
     * \return The grant info for a destination based on the scheduler allocation
     *
     * \see NrSlSlotAlloc
     * \see NrSlUeMac::NrSlGrantInfo
     */
    NrSlUeMac::NrSlGrantInfo CreateSinglePduGrantInfo(const std::set<NrSlSlotAlloc>& params);

    /**
     * \brief Check if the resources indicated by two SFN/subchannel ranges overlap
     *
     * \param firstSfn The first SfnSf to compare
     * \param firstStart The starting subchannel index of the first resource
     * \param firstLength The subchannel length of the first resource
     * \param secondSfn The second  SfnSf to compare
     * \param secondStart The starting subchannel index of the second resource
     * \param secondLength The subchannel length of the second resource
     * \return Whether the two resources overlap
     */
    bool OverlappedResources(const SfnSf& firstSfn,
                             uint16_t firstStart,
                             uint16_t firstLength,
                             const SfnSf& secondSfn,
                             uint16_t secondStart,
                             uint16_t secondLength) const;

    /**
     * \brief Removes resources which are already part of an existing unpublished grant.
     *
     * \param txOppr The list of available slots
     * \return The list of resources which are not used by any existing unpublished grant.
     */
    std::list<NrSlSlotInfo> FilterTxOpportunities(std::list<NrSlSlotInfo> txOppr);
    /**
     * \brief Method to create future SPS grant repetitions
     * \param slotAllocList The slot allocation list from the selection window
     * \param ids The available HARQ process IDs
     * \param harqEnabled Whether HARQ is enabled
     * \param rri The resource reservation interval
     *
     * \see NrSlSlotAlloc
     */
    void CreateSpsGrant(const std::set<NrSlSlotAlloc>& slotAllocList,
                        const std::deque<uint8_t>& ids,
                        bool harqEnabled,
                        Time rri);
    /**
     * \brief Method to create a single-PDU grant
     * \param slotAllocList The slot allocation list from the selection window
     * \param ids The available HARQ process IDs
     * \param harqEnabled Whether HARQ is enabled
     *
     * \see NrSlSlotAlloc
     */
    void CreateSinglePduGrant(const std::set<NrSlSlotAlloc>& slotAllocList,
                              const std::deque<uint8_t>& ids,
                              bool harqEnabled);

    /**
     * \brief Get an available HARQ Id that hasn't been used for a unpublished grant
     * \param ids The available HARQ process IDs received from the MAC
     * \return an available HARQ Id
     */
    uint8_t GetUnusedHarqId(const std::deque<uint8_t>& ids);

    /**
     * \brief Check whether any grants are at the processing delay deadline
     *        to send back to NrUeMac
     * \param sfn The current SfnSf
     */
    void CheckForGrantsToPublish(const SfnSf& sfn);

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

    // From parent class
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
    // Inherited from Object
    void DoDispose() override;

    /**
     * \brief Calculate the transport block size for input parameters
     *
     * For a given modulation and coding scheme, number of subchannels,
     * subchannel size, and symbols per slot, calculate the resulting transport
     * block size in bytes.
     * \param nrAmc pointer to modulation and coding model
     * \param dstMcs MCS value to use
     * \param symbolsPerSlot number of symbols to assume in a slot
     * \param availableSubChannels number of subchannels
     * \param subChannelSize subchannel size in physical resource blocks
     * \return transport block size in bytes
     */
    uint32_t CalculateTbSize(Ptr<const NrAmc> nrAmc,
                             uint8_t dstMcs,
                             uint16_t symbolsPerSlot,
                             uint16_t availableSubChannels,
                             uint16_t subChannelSize) const;

    /**
     * \brief Do the NE Sidelink allocation
     *
     * This function selects resources from the candidate list and associate them
     * to the allocation parameters selected by the scheduler.
     * The SCI 1-A is Txed with every new transmission and after the transmission
     * for, which \c txNumTb mod MaxNumPerReserved == 0 \c , where the txNumTb
     * is the transmission index of the TB, e.g., 0 for initial tx, 1 for a first
     * retransmission, and so on.
     * Finally, the function updates the logical channels with the corresponding
     * assigned data.
     *
     * \param candResources The list of candidate resources received from the UE MAC
     * \param dstInfo The pointer to the NrSlUeMacSchedulerDstInfo of the destination
     *        for which UE MAC asked the scheduler to allocate the resourses
     * \param slotAllocList The slot allocation list to be updated by the scheduler
     * \param allocationInfo the allocation parameters to be associated to the selected
     *        resources
     * \return The status of the resource allocation, true if the destination has been
     *         allocated some resources; false otherwise.
     */
    virtual bool DoNrSlAllocation(const std::list<NrSlSlotInfo>& candResources,
                                  const std::shared_ptr<NrSlUeMacSchedulerDstInfo>& dstInfo,
                                  std::set<NrSlSlotAlloc>& slotAllocList,
                                  AllocationInfo allocationInfo);
    /**
     * \brief Method to get total number of sub-channels.
     *
     * \return the total number of sub-channels.
     */
    uint8_t GetTotalSubCh() const;

    /**
     * \brief Method to get the maximum transmission number
     *        (including new transmission and retransmission) for PSSCH.
     *
     * \return The max number of PSSCH transmissions
     */
    uint8_t GetSlMaxTxTransNumPssch() const;

    Ptr<UniformRandomVariable> m_grantSelectionUniformVariable; //!< Used for grant selection
    Ptr<UniformRandomVariable> m_destinationUniformVariable; //!< Used for destination randomization

  private:
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
     * \brief Remove destination info
     *
     * \param lcid LC ID
     * \param dstL2Id destination L2 ID
     */
    void RemoveDstInfo(uint8_t lcid, uint32_t dstL2Id);

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

    /**
     * \brief Return true if candidate resource overlaps in time (slot)
     *        with any resource on the list.
     *
     * \param resources List of resources to check
     * \param candidate candidate to check
     * \return true if candidate overlaps any slots in the list of resources
     */
    bool OverlappedSlots(const std::list<NrSlSlotInfo>& resources,
                         const NrSlSlotInfo& candidate) const;

    /**
     * \brief Randomly select resources for a grant from the candidate resources
     *
     * If K denotes the candidate resources, and N_PSSCH_maxTx is the
     * maximum number of PSSCH configured transmissions, then:
     *
     * N_Selected = N_PSSCH_maxTx , if K >= N_PSSCH_maxTx
     * otherwise;
     * N_Selected = K
     *
     * If HARQ is disabled, N_Selected = 1 resource.
     *
     * \param txOpps The list of the candidate resources
     * \return The list of randomly selected resources
     */
    std::list<NrSlSlotInfo> SelectResourcesForBlindRetransmissions(std::list<NrSlSlotInfo> txOpps);

    /**
     * \brief Randomly select resources for a grant from the candidate resources,
     *        subject to the constraint of a minimum time gap betweeen resources
     *
     * Select one or more resources, such that retransmission resources are
     * separated by a minimum time gap configured for this resource pool, and
     * such that a retransmission resource can be indicated by the time resource
     * assignment of a prior SCI according to clause 8.3.1.1 of TS 38.212
     *
     * If K denotes the candidate resources, and N_PSSCH_maxTx is the
     * maximum number of PSSCH configured transmissions, then:
     *
     * N_Selected <= N_PSSCH_maxTx , if K >= N_PSSCH_maxTx
     * otherwise;
     * N_Selected <= K
     *
     * If HARQ is disabled, N_Selected = 1 resource.
     *
     * \param txOpps The list of the candidate resources
     * \param harqEnabled Whether HARQ retransmission resources should be selected
     * \return The list of randomly selected resources
     */
    std::list<NrSlSlotInfo> SelectResourcesWithConstraint(std::list<NrSlSlotInfo> txOpps,
                                                          bool harqEnabled);

    /**
     * Check if the time difference between the two slots meets or exceeds the
     * minimum time gap for retransmission.
     *
     * \param first First slot
     * \param second Second slot (possible retransmission)
     * \param minTimeGapPsfch MinTimeGapPsfch value (slots)
     * \param minTimeGapProcessing MinTimeGapProcessing value (slots)
     * \return true if minimum time gap is satisfied
     */
    bool IsMinTimeGapSatisfied(const SfnSf& first,
                               const SfnSf& second,

                               uint8_t minTimeGapPsfch,
                               uint8_t minTimeGapProcessing) const;
    /**
     * Check that the candidate resource conforms to the minimum time gap between
     * any two selected resources as specified in TS 38.321 Section 5.22.1.1
     *
     * \param txOpps list of currently selected resources
     * \param slotInfo slot information of the candidate resource
     * \return true if minimum time gap of candidate resource is satisfied
     */
    bool IsCandidateResourceEligible(const std::list<NrSlSlotInfo>& txOpps,
                                     const NrSlSlotInfo& slotInfo) const;

    /**
     * \brief Get the random selection counter
     * \return The randomly selected reselection counter
     *
     * See 38.321 section 5.22.1.1 V16
     *
     * For 50 ms we use the range as per 36.321 section 5.14.1.1
     */
    uint8_t GetRandomReselectionCounter() const;
    /**
     * \brief Get the lower bound for the Sidelink resource re-selection
     *        counter when the resource reservation period is less than
     *        100 ms. It is as per the Change Request (CR) R2-2005970
     *        to TS 38.321.
     * \param pRsrv The resource reservation period
     * \return The lower bound of the range from which Sidelink resource re-selection
     *         counter will be drawn.
     */
    uint8_t GetLowerBoundReselCounter(uint16_t pRsrv) const;
    /**
     * \brief Get the upper bound for the Sidelink resource re-selection
     *        counter when the resource reservation period is less than
     *        100 ms. It is as per the Change Request (CR) R2-2005970
     *        to TS 38.321.
     * \param pRsrv The resource reservation period
     * \return The upper bound of the range from which Sidelink resource re-selection
     *         counter will be drawn.
     */
    uint8_t GetUpperBoundReselCounter(uint16_t pRsrv) const;

    std::unordered_map<uint32_t, std::shared_ptr<NrSlUeMacSchedulerDstInfo>>
        m_dstMap; //!< The map of between destination layer 2 id and the destination info

    Ptr<NrAmc> m_nrSlAmc; //!< AMC pointer for NR SL

    uint8_t m_mcs{0}; //!< (fixed) value for MCS

    std::map<uint32_t, std::vector<NrSlUeMac::NrSlGrantInfo>> m_grantInfo;

    Ptr<NrUeMac> m_nrUeMac{nullptr}; //!< Pointer to associated NrUeMac object
    Ptr<UniformRandomVariable>
        m_ueSelectedUniformVariable;  //!< uniform random variable used for NR Sidelink
    double m_slProbResourceKeep{0.0}; //!< Sidelink probability of keeping a resource after resource
                                      //!< re-selection counter reaches zero
    uint8_t m_reselCounter{0};        //!< The resource selection counter
    uint16_t m_cResel{0};             //!< The C_resel counter
    Time m_pRsvpTx{MilliSeconds(100)}; //!< Resource Reservation Interval for NR Sidelink in ms
    uint8_t m_t1{2}; //!< The offset in number of slots between the slot in which the resource
                     //!< selection is triggered and the start of the selection window

    bool m_prioToSps{true}; //!< Flag to give scheduling priority to logical channels that are
                            //!< configured with SPS in case of priority tie
    bool m_allowMultipleDestinationsPerSlot{
        false}; //!< Allow scheduling of multiple destinations in same slot
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_FIXED_MCS_H */
