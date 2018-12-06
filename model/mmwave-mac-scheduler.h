/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on work done by CTTC/NYU
 */

#pragma once

#include <ns3/object.h>
#include "mmwave-mac-csched-sap.h"
#include "mmwave-mac-sched-sap.h"

namespace ns3 {

/**
  * \defgroup mac-schedulers
  *
  * \brief NR-enabled schedulers module
  *
  * This group contains the copy of FAPI documentation that was included
  * in the original "LTE MAC Scheduler Interface v1.11" document by FemtoForum
  * (Document number: FF_Tech_001_v1.11 , Date issued: 12-10-2010,
  * Document status: Document for public distribution).
  *
  * \section what Description of the module
  * This group specifies the MAC Scheduler interface and their implementation.
  * The goal of this interface specification is to allow the use of a wide range
  * of schedulers which can be plugged into the eNodeB and to allow for
  * standardized interference coordination interface to the scheduler.
  *
  * Not only the interface between the MAC and the scheduler is standardized,
  * but also the internals of the scheduler. The objective is to be able
  * to write new kind of schedulers with minimum efforts and the minimum
  * amount of code, re-using existing classes and methods.
  *
  * \section interface FAPI interface overview
  *
  * The MAC scheduler is part of MAC from a logical view and the MAC scheduler
  * should be independent from the PHY interface.
  *
  * The description in this interface does not foresee any specific
  * implementation of the interface. What is specified in the FAPI document is
  * the structure of the parameters. In order to describe the interface
  * in detail the following model is used:
  * - The interface is defined as a service access point offered by the MAC
  * scheduler to the remaining MAC functionality, as shown in Figure 1.
  * - A _REQ primitive is from MAC to the MAC scheduler.
  * - A _IND/_CNF primitives are from the MAC scheduler to the MAC.
  *
  * The description using primitives does not foresee any specific implementation
  * and is used for illustration purposes. Therefore an implementation could be
  * message-based or function-based interface. Timing constrains applicable to
  * the MAC scheduler are not yet specified.
  *
  * For the MAC scheduler interface specification a push-based concept is employed,
  * that is all parameters needed by the scheduler are passed to the scheduler
  * at specific times rather than using a pull-based concept (i.e. fetching the
  * parameters from different places as needed). The parameters specified are as
  * far as possible aligned with the 3GPP specifications.
  *
  * [Figure 1]
  *
  * Figure 1 shows the functionality split between the MAC scheduler and the
  * remaining MAC. For the purposes of describing the MAC scheduler interface
  * the MAC consists of a control block and a subframe block, which uses the
  * CSCHED and SCHED SAP respectively. The subframe block triggers the MAC
  * scheduler every TTI and receives the scheduler results. The control block
  * forwards control information to the MAC scheduler as necessary.
  *
  * \note CTTC implementation
  * The documentation of the implementation starts with class MmWaveMacScheduler.
  * Please start your journey into the scheduler documentation from there.
  */
/**
 * \ingroup mac-schedulers
 * \brief Interface for all the mmWave schedulers
 *
 * TODO: Add description of SAP user/providers
 *
 * \see MmWaveMacSchedulerNs3
 */
class MmWaveMacScheduler : public Object
{
public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Table of the BSR values sent by the UEs, following the standard
   *
   * If 1 is sent as a value, it means the UE has 10 bytes stored, and so on.
   */
  static const uint32_t BufferSizeLevelBsrTable[64];

  /**
   * \brief Transform the BSR ID into a real bytes values, following the conversion table
   * \param id BSR ID
   * \return the bytes that the ID represents
   */
  static uint32_t
  BsrId2BufferSize (uint8_t id)
  {
    NS_ABORT_MSG_UNLESS (id < 64, "id = " << id << " is out of range");
    return BufferSizeLevelBsrTable[id];
  }

  /**
   * \brief MmWaveMacScheduler constructor
   */
  MmWaveMacScheduler ();

  /**
   * \brief MmWaveMacScheduler deconstructor
   */
  virtual ~MmWaveMacScheduler ();

  /**
   * \brief Save the common parameters between MAC and PHY
   * \param config the MAC PHY parameters
   */
  virtual void ConfigureCommonParameters (Ptr<MmWavePhyMacCommon> config) = 0;

  /**
   * \brief Set the MacSchedSapUser pointer
   * \param sap pointer to the mac sched sap user class
   */
  void SetMacSchedSapUser (MmWaveMacSchedSapUser* sap)
  {
    m_macSchedSapUser = sap;
  }

  /**
   * \brief Get the MacSchedSapProvider pointer
   * \return the pointer to the mac sched sap provider class
   */
  MmWaveMacSchedSapProvider* GetMacSchedSapProvider ()
  {
    return m_macSchedSapProvider;
  }

  /**
   * \brief SetMacCschedSapUser
   * \param sap the pointer to the sap user
   */
  void SetMacCschedSapUser (MmWaveMacCschedSapUser* sap)
  {
    m_macCschedSapUser = sap;
  }

  /**
   * \brief Get the MacCschedSapProvider pointer
   * \return the pointer to the sap provider
   */
  MmWaveMacCschedSapProvider* GetMacCschedSapProvider ()
  {
    return m_macCschedSapProvider;
  }

  //
  // Implementation of the CSCHED API primitives
  // (See 4.1 for description of the primitives)
  //

  /**
   * \brief Configure cell.
   *
   * (Re-)configure MAC scheduler with cell configuration and scheduler
   * configuration. The cell configuration will also setup the BCH, BCCH, PCCH
   * and CCCH LC configuration (for each component carrier).
   *
   * Ns-3 does nothing.
   *
   * \param params Cell configuration
   */
  virtual void
  DoCschedCellConfigReq (const MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params) = 0;

  /**
   * \brief Configure single UE.
   *
   * (Re-)configure MAC scheduler with single UE specific parameters.
   * A UE can only be configured when a cell configuration has been received.
   *
   * \param params UE configuration
   */
  virtual void
  DoCschedUeConfigReq (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params) = 0;

  /**
   * \brief Configure UE's logical channel(s).
   *
   * (Re-)configure MAC scheduler with UE's logical channel configuration.
   * A logical channel can only be configured when a UE configuration has
   * been received.
   * \param params UE's logical channel configuration
   */
  virtual void
  DoCschedLcConfigReq (const MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params) = 0;

  /**
   * \brief Release UE's logical channel(s).
   *
   * Release UE's logical channel(s) in the MAC scheduler. A logical channel
   * can only be released if it has been configured previously.
   *
   * \param UE's logical channel(s) to be released
   */
  virtual void
  DoCschedLcReleaseReq (const MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params) = 0;

  /**
   * \brief Release UE.
   *
   * Release a UE in the MAC scheduler. The release of the UE configuration
   * implies the release of LCs, which are still active. A UE can only be
   * released if it has been configured previously.
   *
   * \param params UE to be released
   */
  virtual void
  DoCschedUeReleaseReq (const MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params) = 0;

  //
  // Implementation of the SCHED API primitives
  // (See 4.2 for description of the primitives)
  //

  /**
   * \brief DoSchedDlRlcBufferReq
   *
   * Update buffer status of logical channel data in RLC. The update rate with
   * which the buffer status is updated in the scheduler is outside of the scope
   * of the document.
   *
   * \param params RLC Buffer status
   */
  virtual void
  DoSchedDlRlcBufferReq (const MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params) = 0;

  /**
   * \brief Provides CQI measurement report information to the scheduler.
   * \param params DL CQI information
   */
  virtual void
  DoSchedDlCqiInfoReq (const MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params) = 0;

  /**
   * \brief Provides UL CQI measurement information to the scheduler.
   * \param params UL CQI information
   */
  virtual void
  DoSchedUlCqiInfoReq (const MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params) = 0;

  /**
   * \brief Provides mac control information (power headroom, ul buffer status) to the scheduler.
   * \param params MAC control information received
   */
  virtual void
  DoSchedUlMacCtrlInfoReq (const MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params) = 0;

  /**
   * \brief Request for scheduling a slot in DL
   *
   * \param params DL HARQ information
   */
  virtual void
  DoSchedDlTriggerReq (const MmWaveMacSchedSapProvider::SchedDlTriggerReqParameters& params) = 0;

  /**
   * \brief Request for scheduling a slot in UL
   *
   * \param params UL HARQ information
   */
  virtual void
  DoSchedUlTriggerReq (const MmWaveMacSchedSapProvider::SchedUlTriggerReqParameters& params) = 0;

  /**
   * \brief One or more UE asked to be scheduled in UL
   * \param params SR information
   */
  virtual void
  DoSchedUlSrInfoReq (const MmWaveMacSchedSapProvider::SchedUlSrInfoReqParameters &params) = 0;

  /**
   * \brief Forcefully set a default MCS
   * \param mcs MCS
   */
  virtual void
  DoSchedSetMcs (uint32_t mcs) = 0;

protected:
  MmWaveMacSchedSapUser* m_macSchedSapUser           {nullptr};  //!< SAP user
  MmWaveMacCschedSapUser* m_macCschedSapUser         {nullptr};  //!< SAP User
  MmWaveMacCschedSapProvider* m_macCschedSapProvider {nullptr};  //!< SAP Provider
  MmWaveMacSchedSapProvider* m_macSchedSapProvider   {nullptr};  //!< SAP Provider

  uint32_t m_ccId {UINT32_MAX}; //!< Component carrier ID.
};

} // namespace ns3
