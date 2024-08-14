// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NS3_RESOURCE_ASSIGNMENT_MATRIX_H
#define NS3_RESOURCE_ASSIGNMENT_MATRIX_H

#include "beam-id.h"
#include "nr-mac-scheduler-ue-info.h"
#include "nr-phy-mac-common.h"

#include <bitset>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <vector>

namespace ns3
{

/**
 *
 * @brief A resource assignment matrix.
 *
 * This class allows for the creation and management of spectrum resources
 * in time (symbol) and frequency (RBG).
 *
 * It is currently limited to SU-MIMO,
 * by enforcing a single BeamId is assigned to a given RBG in a given symbol.
 *
 * For MU-MIMO, this check must be relaxed and the matrix representation needs
 * an additional dimension to track multiple beams at the same symbol.
 *
 * It can also plot the resource matrix, making it easier to visualize allocation.
 */
class ResourceAssignmentMatrix
{
  public:
    using Rnti = uint16_t;
    using Rbg = uint16_t;

    ResourceAssignmentMatrix(std::vector<bool> notchingMask, uint8_t numSymbols);
    /**
     * @brief Assigns beam ID to symbols
     *
     * This function assigns a beam ID to a range of symbols. The beam ID is applied to all
     * the symbols in the specified range, and it overwrites any existing beam IDs for those
     * symbols.
     *
     * @param beamId The beam ID to be assigned
     * @param startingSymbol The symbol index at which the beam ID should start
     * @param numSymbols The number of symbols that should have the same beam ID (default: 1)
     */
    void AssignBeamIdToSymbols(BeamId beamId, uint8_t startingSymbol, uint8_t numSymbols = 1);

    /**
     * @brief ResourceType enum represents the type of resource in a transmission.
     */
    enum class ResourceType : uint8_t
    {
        EMPTY,
        // Data
        DL_DATA,
        UL_DATA,
        HARQ,
        // Reference signals
        SRS,
        CSI_RS,
        PDSCH_DMRS,
        PUSCH_DMRS,
        PBSCH_DMRS,
        PTRS,
        TRS,
        DL_DCI,
        UL_DCI,
        MSG3,
    };

    /**
     * @brief Assigns TDMA channel during a symbol to the UE.
     *
     * This function assigns an TDMA channel (all RBGs) during a specific symbol to the UE with the
     * given RNTI. The function takes into account the notching mask and ensures that the assignment
     * is valid. It also updates the number of assigned resources for the UE.
     *
     * @param type Type of resource to be assigned (e.g., UL_DATA, DL_DATA)
     * @param rnti The RNTI of the UE for which to retrieve the assigned resources
     * @param startingSymbol Symbol index at which the assignment should start
     * @param numSymbols Number of symbols that should have the same assignment (default: 1)
     */
    void AssignTdmaChannelDuringSymbolToUe(ResourceType type,
                                           Rnti rnti,
                                           uint8_t startingSymbol,
                                           uint8_t numSymbols = 1);
    /**
     * @brief Assigns OFDMA RBG during a symbol to the UE.
     *
     * This function assigns an OFDMA RBG during a specific symbol to the UE with the given RNTI.
     * The function takes into account the notching mask and ensures that the assignment is valid.
     * It also updates the number of assigned resources for the UE.
     *
     * @param type Type of resource to be assigned (e.g., UL_DATA, DL_DATA)
     * @param rbg Index of the OFDMA RBG to be assigned
     * @param startingSymbol Symbol index at which the assignment should start
     * @param numSymbols Number of symbols that should have the same assignment (default: 1)
     */
    void AssignOfdmaRbgDuringSymbolToUe(ResourceType type,
                                        Rnti rnti,
                                        Rbg rbg,
                                        uint8_t startingSymbol,
                                        uint8_t numSymbols = 1);
    /**
     * @brief Assigns TDMA channel during a symbol to the control plane.
     *
     * This function assigns a TDMA channel (all RBGs) during a specific symbol to the control
     * plane. The function takes into account the notching mask and ensures that the assignment is
     * valid. It also updates the number of assigned resources for the control plane.
     *
     * @param type Type of resource to be assigned (e.g., UL_DATA, DL_DATA)
     * @param startingSymbol Symbol index at which the assignment should start
     * @param numSymbols Number of symbols that should have the same assignment (default: 1)
     */
    void AssignTdmaChannelDuringSymbolToCtrl(ResourceType type,
                                             uint8_t startingSymbol,
                                             uint8_t numSymbols = 1);
    /**
     * @brief Assigns OFDMA RBG during a symbol to the control plane.
     *
     * This function assigns an OFDMA RBG during a specific symbol to the control plane.
     * The function takes into account the notching mask and ensures that the assignment is valid.
     * It also updates the number of assigned resources for the control plane.
     *
     * @param type Type of resource to be assigned (e.g., UL_DATA, DL_DATA)
     * @param rbg Index of the OFDMA RBG to be assigned
     * @param startingSymbol Symbol index at which the assignment should start
     * @param numSymbols Number of symbols that should have the same assignment (default: 1)
     */
    void AssignOfdmaRbgDuringSymbolToCtrl(ResourceType type,
                                          Rbg rbg,
                                          uint8_t startingSymbol,
                                          uint8_t numSymbols = 1);

    /**
     * @brief Returns the total number of assigned resources in the matrix.
     *
     * @return Number of assigned resources
     */
    std::size_t GetAssignedResourcesTotal();
    /**
     * @brief Returns the total number of free resources in the matrix.
     * @return Number of free resources in the matrix
     */
    std::size_t GetFreeResourcesTotal();
    /**
     * @brief Returns the number of assigned resources to a specific UE RNTI.
     *
     * @param rnti The RNTI of the UE for which to retrieve the number of assigned resources
     *
     * @return The number of assigned resources to the specified UE
     */
    std::size_t GetNumAssignedResourcesToUe(Rnti rnti);

    struct AssignedResourceElement
    {
        BeamId beamId;
        Rbg rbg;
        uint8_t symbol;
        ResourceType type;
    };

    /**
     * @brief Returns the assigned resources to a specific UE.
     *
     * This function returns the assigned resources to a specific UE with the given RNTI.
     *
     * @param rnti The RNTI of the UE for which to retrieve the assigned resources
     *
     * @return A vector of AssignedResourceElement, where each element represents an assigned
     * resource
     */
    std::vector<AssignedResourceElement> GetAssignedResourcesToUe(Rnti rnti);

    /**
     * Plots the ResourceAssignmentMatrix
     */
    [[maybe_unused]] void PlotResourceMatrix();

    /**
     * Checks the ResourceAssignmentMatrix from VarTtiAllocInfo
     *
     * @param allocInfo - The deque of VarTtiAllocInfo to check
     * @param ueMap - The unordered map of UE information
     * @param notchingMask - The vector of booleans indicating which RBGs are notching
     * @param numSym - The number of symbols in the allocation period
     * @param plot - Whether or not to plot the ResourceAssignmentMatrix
     */
    [[maybe_unused]] static void CheckResourceMatrixFromVarTtiAllocInfo(
        const std::deque<VarTtiAllocInfo>& allocInfo,
        const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& ueMap,
        const std::vector<bool>& notchingMask,
        const uint8_t numSym,
        bool plot = false);

  private:
    /**
     * Contains the UE and type of resource usage for an RBG.
     * The index of the RBG is determined by its position in the SymbolResources.rbgs vector.
     */
    struct ResourceMatrixEntry
    {
        Rnti allocatedUe;                   ///< RBG position and allocated UE
        ResourceType allocatedResourceType; ///< Type of allocated resource
    };

    /**
     * @class SymbolResources
     * @brief Represents the resources used for a symbol in OFDM transmission.
     *
     * This class contains the following member variables:
     * - beamId: BeamId of current symbol
     * - rbgs: Vector containing ResourceMatrixEntry elements, each representing a different RBG
     * with scheduled UE and usage type
     */
    struct SymbolResources
    {
        BeamId beamId;                         ///< BeamId for current symbol
        std::vector<ResourceMatrixEntry> rbgs; ///< Resource for allocation
    };

    std::vector<bool> m_notchingMask; ///< Notching bitmask applied to channel bandwidth
    std::vector<bool>
        m_beamIdAssigned; ///< Bitmask indicating whether a beamId was already assigned to a symbol
    std::vector<SymbolResources> m_symbolResources; ///< The allocation resource matrix itself
    std::unordered_map<Rnti, uint32_t>
        m_ueNumberOfResources; ///< Tally of number of resources per UE
};

} // namespace ns3

#endif // NS3_RESOURCE_ASSIGNMENT_MATRIX_H
