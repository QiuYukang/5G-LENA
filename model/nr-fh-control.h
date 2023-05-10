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

    void SetNrFhPhySapUser(NrFhPhySapUser* s);
    NrFhPhySapProvider* GetNrFhPhySapProvider();

    void SetNrFhSchedSapUser(NrFhSchedSapUser* s);
    NrFhSchedSapProvider* GetNrFhSchedSapProvider();

    /// let the forwarder class access the protected and private members
    friend class MemberNrFhPhySapProvider<NrFhControl>;
    /// let the forwarder class access the protected and private members
    friend class MemberNrFhSchedSapProvider<NrFhControl>;

    /**
     * \brief Set the limit model.
     *
     * 4 models are defined
     */
    void SetLimitModel();

    void DoGetDoesAllocationFit();

  private:
    // FH Control - PHY SAP
    NrFhPhySapUser* m_fhPhySapUser;         //!< FH Control - PHY SAP User
    NrFhPhySapProvider* m_fhPhySapProvider; //!< FH Control - PHY SAP Provider

    // FH Control - SCHEDULER SAP
    NrFhSchedSapUser* m_fhSchedSapUser;         //!< FH Control -  SCHED SAP User
    NrFhSchedSapProvider* m_fhSchedSapProvider; //!< FH Control -  SCHED SAP Provider
};

} // end namespace ns3

#endif // NR_FH_CONTROL_H
