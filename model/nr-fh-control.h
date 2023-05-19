/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

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
     * \param s The ptr of the SAP User
     */
    void SetNrFhPhySapUser(NrFhPhySapUser* s);

    /**
     * \brief Get the Fh control - PHY SAP User ptr
     * \return the ptr of the SAP User
     */
    NrFhPhySapProvider* GetNrFhPhySapProvider();

    /**
     * \brief Set the Fh control - Sched SAP User
     * \param s The ptr of the SAP User
     */
    void SetNrFhSchedSapUser(NrFhSchedSapUser* s);

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
     * \brief Set the available fronthaul capacity
     * \param capacity The fronthaul capacity (in Gbps)
     */
    void SetFhCapacity(uint16_t capacity);

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
    NrFhPhySapUser* m_fhPhySapUser;         //!< FH Control - PHY SAP User
    NrFhPhySapProvider* m_fhPhySapProvider; //!< FH Control - PHY SAP Provider

    // FH Control - SCHEDULER SAP
    NrFhSchedSapUser* m_fhSchedSapUser;         //!< FH Control -  SCHED SAP User
    NrFhSchedSapProvider* m_fhSchedSapProvider; //!< FH Control -  SCHED SAP Provider

    enum FhControlMethod m_fhControlMethod;
    uint16_t m_fhCapacity{
        1000}; //!< the available FH capacity (in Mbps) for DL and UL (full-duplex FH link)
    uint8_t m_overheadDyn{32};    //!< the overhead (OH) for dynamic adaptation (in bits)
    uint8_t m_numRbPerRbg{1};     //!< the number of RBs per RBG
    uint8_t m_mcsTable{2};        //!< the MCS table
    std::string m_errorModelType; //!< the error model type besed on which the MCS Table will be set

    std::unordered_map<uint32_t, uint32_t>
        m_rntiQueueSize; //!< Map for the number of bytes in RLC queues of a specific UE (bwpId,
                         //!< rnti, bytes)
    std::unordered_map<uint16_t, uint16_t> m_activeUes; //!< Map active bwpIds and active Ues
};

} // end namespace ns3

#endif // NR_FH_CONTROL_H
