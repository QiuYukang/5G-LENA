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
    uint8_t m_overheadDyn{32}; //!< the overhead (OH) for dynamic adaptation (in bits)
};

} // end namespace ns3

#endif // NR_FH_CONTROL_H
