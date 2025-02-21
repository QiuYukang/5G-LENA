// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "nr-mac-scheduler-ofdma-symbol-per-beam.h"
#include "nr-mac-scheduler-tdma.h"

#include "ns3/traced-value.h"

#include <set>
#include <unordered_set>

namespace ns3
{
class NrMacSchedulerOfdmaSymbolPerBeam;
class NrSchedOfdmaSymbolPerBeamTestCase;

/**
 * @ingroup scheduler
 * @brief The base for all the OFDMA schedulers
 *
 * An example of OFDMA-based scheduling is the following:
 * <pre>
 * (f)
 * ^
 * |=|======|=======|=|
 * |C| U  E | U  E  |C|
 * |T|  1   |  3    |T|
 * | |======|=======| |
 * |R| U  E | U  E  |R|
 * |L|  2   |   4   |L|
 * |----------------------------> (t)
 * </pre>
 *
 * The UEs are scheduled by prioritizing the assignment of frequencies: the entire
 * available spectrum is divided among UEs of the same beam, by a number of
 * symbols which is pre-computed and depends on the total byte to transmit
 * of each beam.
 *
 * The OFDMA scheduling is only done in downlink. In uplink, the division in
 * time is used, and therefore the class is based on top of NrMacSchedulerTdma.
 *
 * The implementation details to construct a slot like the one showed before
 * are in the functions AssignDLRBG() and AssignULRBG().
 * The choice of the UEs to be scheduled is, however, demanded to the subclasses.
 *
 * The DCI is created by CreateDlDci() or CreateUlDci(), which call CreateDci()
 * to perform the "hard" work.
 *
 * @see NrMacSchedulerOfdmaRR
 * @see NrMacSchedulerOfdmaPF
 * @see NrMacSchedulerOfdmaMR
 */
class NrMacSchedulerOfdma : public NrMacSchedulerTdma
{
  public:
    /**
     * @brief GetTypeId
     * @return The TypeId of the class
     */
    static TypeId GetTypeId();

    /**
     * @brief NrMacSchedulerOfdma constructor
     */
    NrMacSchedulerOfdma();

    /**
     * @brief Deconstructor
     */
    ~NrMacSchedulerOfdma() override
    {
    }

  protected:
    BeamSymbolMap AssignDLRBG(uint32_t symAvail, const ActiveUeMap& activeDl) const override;
    BeamSymbolMap AssignULRBG(uint32_t symAvail, const ActiveUeMap& activeUl) const override;

    std::shared_ptr<DciInfoElementTdma> CreateDlDci(
        PointInFTPlane* spoint,
        const std::shared_ptr<NrMacSchedulerUeInfo>& ueInfo,
        uint32_t maxSym) const override;
    std::shared_ptr<DciInfoElementTdma> CreateUlDci(
        PointInFTPlane* spoint,
        const std::shared_ptr<NrMacSchedulerUeInfo>& ueInfo,
        uint32_t maxSym) const override;

    /**
     * @brief Advance the starting point by the number of symbols specified,
     * resetting the RB count to 0
     * @param spoint Starting point
     * @param symOfBeam Number of symbols for the beam
     */
    void ChangeDlBeam(PointInFTPlane* spoint, uint32_t symOfBeam) const override;

    void ChangeUlBeam(PointInFTPlane* spoint, uint32_t symOfBeam) const override;

    /**
     * @brief Calculate the number of symbols to assign to each beam by calling using the technique
     * selected in m_symPerBeamType
     * @param symAvail Number of available symbols
     * @param activeDl Map of active DL UE and their beam
     * @return symbols per beam allocation map
     */
    NrMacSchedulerOfdma::BeamSymbolMap GetSymPerBeam(uint32_t symAvail,
                                                     const ActiveUeMap& activeDl) const;

    uint8_t GetTpc() const override;

    /**
     * @brief Enumeration of techniques to distribute the available symbols to the active beams
     */
    enum class SymPerBeamType
    {
        LOAD_BASED, //!< Distributes symbols to beams proportionally to the buffer size of its users
        ROUND_ROBIN, //!< Distributes all symbols to the first active beam in the m_rrBeams queue
        PROPORTIONAL_FAIR //!< Distributes symbols to beams proportionally to mean achievable rate
    };

  private:
    /**
     * @brief Create RBG bitmask from allocated RBG vector
     * @param allocatedRbgs vector of allocated RBGs
     * @return RBG bitmask
     */
    std::vector<bool> CreateRbgBitmaskFromAllocatedRbgs(
        const std::vector<uint16_t>& allocatedRbgs) const;

    /**
     * @brief Advances schedInfoIt iterator to the next UE to be scheduled
     *
     * Iterate all the way from beginning to the end iterators of the vector of UEs to schedule,
     * looking for the first UE that hasn't had scheduled enough resources to fill its entire
     * buffer.
     *
     * In case an UE hasn't had enough resources scheduled, and the fronthaul policy allows
     * (ShouldScheduleUeBasedOnFronthaul), it will be scheduled in the next iteration, returning
     * true.
     *
     * If the fronthaul does now allow it, or there is no UE with more data to transmit, then return
     * false to indicate scheduling has ended for this beam.
     * @param schedInfoIt iterator pointing to the beginning of vector with UEs to be scheduled
     * @param end end iterator of vector of UEs to be scheduled
     * @param resourcesAssignable number of resources that can be scheduled per time (symbols per
     * beam * 1 rbg)
     * @return true in case scheduling should be done for a UE pointed by current schedInfoIt, false
     * in case scheduling for beam should stop
     */
    bool AdvanceToNextUeToSchedule(std::vector<UePtrAndBufferReq>::iterator& schedInfoIt,
                                   const std::vector<UePtrAndBufferReq>::iterator end,
                                   uint32_t resourcesAssignable) const;
    /**
     * @brief Decides whether UE pointed by schedInfoIt should be scheduled based on fronthaul
     * policy
     *
     * @param schedInfoIt iterator pointing to the beginning of vector with UEs to be scheduled
     * @param resourcesAssignable number of resources that can be scheduled per time (symbols per
     * beam * 1 rbg)
     * @return true in case scheduling should be done for a UE pointed by current schedInfoIt, false
     * in case UE should not be scheduled
     */
    bool ShouldScheduleUeBasedOnFronthaul(
        const std::vector<UePtrAndBufferReq>::iterator& schedInfoIt,
        uint32_t resourcesAssignable) const;

    /**
     * @brief Apply fronthaul control policy to constrain allocation post-fact, by deallocating
     * resources
     * @param ueVector Reference to vector of UEs scheduled for a beam
     * @param beamSym Reference to number of resources (symbols per beam * 1 rbg) available
     * @param assignedResources Reference to currently assigned resources (symbols, rbgs)
     * @param availableRbgs Reference to vector of available RBGs
     */
    void DeallocateResourcesDueToFronthaulConstraint(const std::vector<UePtrAndBufferReq>& ueVector,
                                                     const uint32_t& beamSym,
                                                     FTResources& assignedResources,
                                                     std::vector<bool>& availableRbgs) const;
    /**
     * @brief Allocate resources defined by currentRbg (RBG)*beamSym (symbols per beam) from
     * currentUe, then update list of assignedResources and availableRbgs
     * @param currentUe Pointer to UE that should have currentRbg deallocated
     * @param currentRbg RBG to be deallocated
     * @param beamSym Symbols per beam to be deallocated
     * @param assignedResources Reference to total assigned resources
     * @param availableRbgs Reference vector of available RBGs for scheduling
     */
    static void AllocateCurrentResourceToUe(std::shared_ptr<NrMacSchedulerUeInfo> currentUe,
                                            const uint32_t& currentRbg,
                                            const uint32_t beamSym,
                                            FTResources& assignedResources,
                                            std::vector<bool>& availableRbgs);
    /**
     * @brief Deallocate resources defined by currentRbg (RBG)*beamSym (symbols per beam) from
     * currentUe, then update list of assignedResources and availableRbgs
     * @param currentUe Pointer to UE that should have currentRbg deallocated
     * @param currentRbg RBG to be deallocated
     * @param beamSym Symbols per beam to be deallocated
     * @param assignedResources Reference to total assigned resources
     * @param availableRbgs Reference vector of available RBGs for scheduling
     */
    static void DeallocateCurrentResourceFromUe(std::shared_ptr<NrMacSchedulerUeInfo> currentUe,
                                                const uint32_t& currentRbg,
                                                const uint32_t beamSym,
                                                FTResources& assignedResources,
                                                std::vector<bool>& availableRbgs);
    /**
     * @brief Try to schedule the best RBG out of remainingRbgSet to an UE referenced by
     * schedInfoIt, for beamSym symbols, then update the list of assignedResources and availableRbgs
     * @param schedInfoIt Reference to the UE to be scheduled
     * @param remainingRbgSet Reference to set of available RBGs to be scheduled
     * @param beamSym Number of symbols per beam to be scheduled
     * @param assignedResources Number of resources scheduled
     * @param availableRbgs Mask of available RBGs
     * @return true if scheduled, false if not scheduled
     */
    bool AttemptAllocationOfCurrentResourceToUe(
        std::vector<UePtrAndBufferReq>::iterator schedInfoIt,
        std::set<uint32_t>& remainingRbgSet,
        const uint32_t beamSym,
        FTResources& assignedResources,
        std::vector<bool>& availableRbgs) const;

    void SetSymPerBeamType(SymPerBeamType type);
    TracedValue<uint32_t>
        m_tracedValueSymPerBeam; //!< Variable to trace symbols per beam allocation

    SymPerBeamType m_symPerBeamType; //!< Holds the type of symbol scheduling done for each beam
    Ptr<NrMacSchedulerOfdmaSymbolPerBeam> m_symPerBeam; //!< Holds a symbol per beam allocator
    /// Make it friend of the test case, so that the test case can access m_symPerBeam
    friend class NrSchedOfdmaSymbolPerBeamTestCase;
};
} // namespace ns3
