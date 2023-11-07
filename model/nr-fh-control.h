// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_FH_CONTROL_H
#define NR_FH_CONTROL_H

#include "nr-fh-phy-sap.h"
#include "nr-fh-sched-sap.h"

#include <ns3/object.h>

namespace ns3
{

class NrFhPhySapUser;
class NrFhPhySapProvider;
class NrFhSchedSapUser;
class NrFhSchedSapProvider;

/**
 * \ingroup
 * \brief Fronthaul Capacity Control
 *
 */

class NrFhControl : public Object
{
  public:
    /**
     * \brief NrFhControl constructor
     */
    NrFhControl();

    /**
     * \brief ~NrFhControl deconstructor
     */
    ~NrFhControl() override;

    /**
     * \brief GetTypeId
     * \return the TypeId of the Object
     */
    static TypeId GetTypeId();

    /**
     * \brief GetInstanceTypeId
     * \return the instance typeid
     */
    // TypeId GetInstanceTypeId() const override;

    /**
     * \brief Set the Fh control - PHY SAP User
     *        PHY is per bwp as such we store in a map the bwpId
     *        and the corresponding NrFhPhySapUser ptr.
     * \param bwpId The bwpId
     * \param s The ptr of the SAP User
     */
    void SetNrFhPhySapUser(uint16_t bwpId, NrFhPhySapUser* s);

    /**
     * \brief Get the Fh control - PHY SAP User ptr
     * \return the ptr of the SAP User
     */
    NrFhPhySapProvider* GetNrFhPhySapProvider();

    /**
     * \brief Set the Fh control - Sched SAP User
     *        Sched is per bwp as such we store in a map the bwpId
     *        and the corresponding SetNrFhSchedSapUser ptr.
     * \param bwpId The bwpId
     * \param s The ptr of the SAP User
     */
    void SetNrFhSchedSapUser(uint16_t bwpId, NrFhSchedSapUser* s);

    /**
     * \brief Get the Fh control - Sched SAP User ptr
     * \return the ptr of the SAP User
     */
    NrFhSchedSapProvider* GetNrFhSchedSapProvider();

    /// let the forwarder class access the protected and private members
    friend class MemberNrFhPhySapProvider<NrFhControl>;
    /// let the forwarder class access the protected and private members
    friend class MemberNrFhSchedSapProvider<NrFhControl>;

    /**
     * \brief The optimization models (FH Control method) of the NrFhControl
     */
    enum FhControlMethod
    {
        Dropping,    //!< Drop DCI + DATA at the PHY Layer
        Postponing,  //!< Postpone sending data (MAC Layer)
        OptimizeMcs, //!< Optimize MCS
        OptimizeRBs, //!< Optimize RBs allocated
    };

    /**
     * \brief Set the FH Control method type.
     * \param model The FH Control method type
     */
    void SetFhControlMethod(FhControlMethod model);

    /**
     * \brief Set the available fronthaul capacity of the cell.
     *        The capacity will be shared among all the active
     *        BWPs of the cell. This is done later on when
     *        nrHelper configures the nrFhControl through the
     *        function NrHelper::ConfigureFhControl.
     * \param capacity The fronthaul capacity (in Mbps)
     */
    void SetCellFhCapacity(uint16_t capacity);

    /**
     * \brief Split the available FH capacity based on the number
     *        of active BWPs of the cell. This function is called
     *        when the nrHelper configures the nrFhControl through
     *        the function NrHelper::ConfigureFhControl.
     * \param numberOfActiveBwps The number of active BWPs
     */
    void ConfigureFhCapacityPerBwp(uint32_t numberOfActiveBwps);

    /**
     * \brief Set the overhead for dynamic modulation compression
     * \param overhead The overhead for dynamic modulation compression (in bits)
     */
    void SetOverheadDyn(uint8_t overhead);

    /**
     * \brief Set the ErrorModelType based on which the MCS Table
     *        (1 or 2) will be set."
              "ns3::NrEesmIrT1 and ns3::NrEesmCcT1 for MCS Table 1"
              "ns3::NrEesmIrT2 and ns3::NrEesmCcT2 for MCS Table 2.
     * \param erroModelType The error model type
     */
    void SetErrorModelType(std::string erroModelType);

    /**
     * \brief Set the physical cell Id of the cell to which this NrFhControl
     *        instance belongs to.
     * \param physCellId The physical cell Id
     */
    void SetPhysicalCellId(uint16_t physCellId);

    /**
     * \brief Set the numerology
     * \param num the numerology
     *
     */
    void SetNumerology(uint16_t bwpId, uint16_t num);

  private:
    /**
     * \brief Get the FH Control method.
     * \return the FH Control method type
     */
    FhControlMethod GetFhControlMethod() const;

    /**
     * \brief Get the FH control method through the SAP interfaces.
     * \return the FH control method (uint8_t)
     */
    uint8_t DoGetFhControlMethod() const;

    /**
     * \brief Get the physical CellId for this NrFhControl instance.
     * \return the physical CellId
     */
    uint16_t DoGetPhysicalCellId() const;

    /**
     * \brief Returns a boolean indicating whether the current allocation can
     * fit in the available FH bandwidth.
     */
    void DoGetDoesAllocationFit();

    /**
     * \brief Set the a UE as active (with data in RLC queue) for a slot, saving
     *        a map of the bwpId, and rnti for such a UE, and the amount of bytes
     *        in its RLC buffers.
     *
     * \param bwpId the BWP ID
     * \param rnti the RNTI
     * \param bytes the bytes in RLC buffers of the UE
     */
    void DoSetActiveUe(uint16_t bwpId, uint16_t rnti, uint32_t bytes);

    /**
     * \brief Updates the map of the UEs that have been served by this cell in a
     *        bwpId, based on the allocation. If a UE has been fully served (no
     *        remaining bytes in its RLC queues), then the entry of the map is
     *        removed. Otherwise, if a UE has still bytes in its RLC queues after
     *        the allocation, the amount of bytes stored in the map is updated.
     *
     * \param bwpId the BWP ID
     * \param allocation the allocation structure of a slot
     */
    void DoUpdateActiveUesMap(uint16_t bwpId, const std::deque<VarTtiAllocInfo>& allocation);

    /**
     * \brief Returns a boolean indicating whether the current allocation can
     *        fit in the available FH bandwidth.
     */
    bool DoGetDoesAllocationFit(uint16_t bwpId, uint32_t mcs, uint32_t nRegs);

    /**
     * \brief Returns the maximum MCS that can be assigned to a
     *        specific UE (rnti, bwpId) with a RB allocation.
     *
     * \param bwpId the BWP ID
     * \param reg the allocated REGs
     * \param rnti the RNTI
     *
     * \return the maximum MCS
     */
    uint8_t DoGetMaxMcsAssignable(uint16_t bwpId, uint32_t reg, uint32_t rnti);

    /**
     * \brief Returns the maximum number of REGs that can be assigned to a
     *        specific UE (rnti, bwpId) with a specific MCS (mcs).
     *
     * \param bwpId the BWP ID
     * \param mcs the MCS
     * \param rnti the RNTI
     */
    uint32_t DoGetMaxRegAssignable(uint16_t bwpId, uint32_t mcs, uint32_t rnti);

    /**
     * \brief Updates the FH DL trace based on some dropped data+allocation,
     *        called from phy.
     *
     * \param bwpId the BWP ID
     * \param mcs the MCS
     * \param nRbgs the number of RBGs
     * \param nSymb the number of symbols
     */
    void DoUpdateTracesBasedOnDroppedData(uint16_t bwpId,
                                          uint32_t mcs,
                                          uint32_t nRbgs,
                                          uint32_t nSymb);

    /**
     * \brief End slot notification from gnb-phy, to track the required fronthaul
     *        throughput for that slot
     *
     * \param currentSlot The current slot
     */
    void DoNotifyEndSlot(uint16_t bwpId, SfnSf currentSlot);

    /**
     * \brief Returns the FH throughput associated to a specific allocation
     */
    uint64_t GetFhThr(uint16_t bwpId, uint32_t mcs, uint32_t Nres) const;

    /**
     * \brief Returns the number of active BWPs, i.e., BWPs with data in their
     *        RLC queues.
     */
    uint16_t GetNumberActiveBwps() const;

    /**
     * \brief Returns the number of active UEs in a BWP
     *
     * \param bwpId The bwpId for which we want the number of active UEs
     * \return the number of active UEs in a BWP
     */
    uint16_t GetNumberActiveUes(uint16_t bwpId) const;

    /**
     * \brief Returns the max MCS based on the MCS Table (1 or 2)
     *        and the max modulation order.
     */
    uint8_t GetMaxMcs(uint8_t mcsTable, uint16_t modOrder);

    /**
     * \brief Returns the modulation order of a specific mcs of MCS Table1.
     */
    uint32_t GetModulationOrderTable1(const uint32_t mcs) const;

    /**
     * \brief Returns the modulation order of a specific mcs of MCS Table2.
     */
    uint32_t GetModulationOrderTable2(const uint32_t mcs) const;

    /**
     * \brief Returns the max MCS index of a given modulation order of MCS Table1.
     */
    uint8_t GetMcsTable1(const uint8_t modOrder) const;

    /**
     * \brief Returns the max MCS index of a given modulation order of MCS Table2.
     */
    uint8_t GetMcsTable2(const uint8_t modOrder) const;

    uint16_t m_physicalCellId; //!< Physical cell ID to which the NrFhControl instance belongs to.

    // FH Control - PHY SAP
    std::map<uint16_t, NrFhPhySapUser*> m_fhPhySapUser; //!< FH Control - PHY SAP User (per bwpId)
    NrFhPhySapProvider* m_fhPhySapProvider;             //!< FH Control - PHY SAP Provider

    // FH Control - SCHEDULER SAP
    std::map<uint16_t, NrFhSchedSapUser*>
        m_fhSchedSapUser;                       //!< FH Control - PHY SAP User (per bwpId)
    NrFhSchedSapProvider* m_fhSchedSapProvider; //!< FH Control -  SCHED SAP Provider

    enum FhControlMethod m_fhControlMethod;
    uint16_t m_fhCapacity{
        1000}; //!< the available FH capacity (in Mbps) for DL and UL (full-duplex FH link)
    uint8_t m_overheadDyn{32};    //!< the overhead (OH) for dynamic adaptation (in bits)
    uint8_t m_mcsTable{2};        //!< the MCS table
    std::string m_errorModelType; //!< the error model type based on which the MCS Table will be set

    std::unordered_map<uint32_t, uint32_t>
        m_rntiQueueSize; //!< Map for the number of bytes in RLC queues of a specific UE (bwpId,
                         //!< rnti, bytes)
    std::unordered_map<uint16_t, uint16_t> m_activeUesPerBwp;  //!< Map of active bwpIds
    std::unordered_map<uint16_t, uint16_t> m_numerologyPerBwp; //!< Map of bwpIds and numerologies

    uint64_t m_allocFhThroughput{
        0}; //!< the allocated fronthaul throughput after scheduling (in DL)
    std::unordered_map<uint16_t, uint64_t>
        m_allocCellThrPerBwp; //!< Map for FH allocated throughput of a specific bwpId (in DL)

    std::unordered_map<uint16_t, uint64_t>
        m_reqFhDlThrTracedValuePerBwp; //!< the required fronthaul throughput (in DL) per BWP
    std::unordered_map<uint16_t, uint32_t>
        m_rbsAirTracedValue; //!< Map for the used RBs of the air of a specific bwpId
    std::unordered_map<uint16_t, SfnSf> m_waitingSlotPerBwp;

    // SfnSf, bwpId, FH throughput
    TracedCallback<const SfnSf&, uint16_t, uint64_t>
        m_reqFhDlThrTrace; //!< Report the required FH throughput (in DL) per BWP
    // SfnSf, bwpId, RBs used
    TracedCallback<const SfnSf&, uint16_t, uint32_t>
        m_rbsAirTrace; //!< Report the RBs used of the AI (in DL) per BWP
};

} // end namespace ns3

#endif // NR_FH_CONTROL_H
