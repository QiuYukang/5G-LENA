/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_SL_UE_MAC_SCHEDULER_SIMPLE_H
#define NR_SL_UE_MAC_SCHEDULER_SIMPLE_H

#include "nr-sl-phy-mac-common.h"
#include "nr-sl-ue-mac-scheduler-ns3.h"

namespace ns3
{

/**
 * \ingroup scheduler
 *
 * \brief A simple NR Sidelink scheduler for NR SL UE in NS3
 */
class NrSlUeMacSchedulerSimple : public NrSlUeMacSchedulerNs3
{
  public:
    /**
     * \brief GetTypeId
     *
     * \return The TypeId of the class
     */
    static TypeId GetTypeId(void);

    /**
     * \brief NrSlUeMacSchedulerNs3 default constructor
     */
    NrSlUeMacSchedulerSimple();

    /**
     * \brief Do the NR Sidelink allocation
     *
     * The SCI 1-A is Txed with every new transmission and after the transmission
     * for, which \c txNumTb mod MaxNumPerReserved == 0 \c , where the txNumTb
     * is the transmission index of the TB, e.g., 0 for initial tx, 1 for a first
     * retransmission, and so on.
     *
     * \param txOpps The list of the txOpps from the UE MAC
     * \param dstInfo The pointer to the NrSlUeMacSchedulerDstInfo of the destination
     *        for which UE MAC asked the scheduler to allocate the recourses
     * \param slotAllocList The slot allocation list to be updated by this scheduler
     * \return The status of the allocation, true if the destination has been
     *         allocated some resources; false otherwise.
     */
    virtual bool DoNrSlAllocation(const std::list<NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps,
                                  const std::shared_ptr<NrSlUeMacSchedulerDstInfo>& dstInfo,
                                  std::set<NrSlSlotAlloc>& slotAllocList) override;

  private:
    /**
     * \ingroup scheduler
     * \brief The SbChInfo struct
     */
    struct SbChInfo
    {
        uint8_t numSubCh{
            0}; //!< The minimum number of contiguous subchannels that could be used for each slot.
        std::vector<std::vector<uint8_t>>
            availSbChIndPerSlot; //!< The vector containing the available subchannel index for each
                                 //!< slot
    };

    /**
     * \brief Select the slots randomly from the available slots
     *
     * \param txOpps The list of the available TX opportunities
     * \return the set containing the indices of the randomly chosen slots in the
     *         txOpps list
     */
    std::list<NrSlUeMacSchedSapProvider::NrSlSlotInfo>
    /**
     * \brief Randomly select the number of slots from the slots given by UE MAC
     *
     * If K denotes the total number of available slots, and N_PSSCH_maxTx is the
     * maximum number of PSSCH configured transmissions, then:
     *
     * N_Selected = N_PSSCH_maxTx , if K >= N_PSSCH_maxTx
     * otherwise;
     * N_Selected = K
     *
     * \param txOpps The list of the available slots
     * \return The list of randomly selected slots
     */
    RandomlySelectSlots(std::list<NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps);
    /**
     * \brief Get available subchannel information
     *
     * This method takes as input the randomly selected slots and computes the
     * maximum number of contiguous subchannels that are available for all
     * those slots. Moreover, it also returns the indexes of the available
     * subchannels for each slot.
     *
     * \param txOpps The list of randomly selected slots
     * \return A struct object of type SbChInfo
     */
    SbChInfo GetAvailSbChInfo(std::list<NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps);
    /**
     * \brief Randomly select the starting subchannel index
     *
     * This method, for each slot randomly selects the starting subchannel
     * index by taking into account the number of available contiguous subchannels
     * and the number of subchannels that needs to be assigned.
     *
     * \param sbChInfo A struct object of type SbChInfo
     * \param assignedSbCh The number of assigned subchannels
     * \return A vector containing the randomly chosen starting subchannel index
     *         for each slot.
     */
    std::vector<uint8_t> RandSelSbChStart(SbChInfo sbChInfo, uint8_t assignedSbCh);
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_SIMPLE_H */
