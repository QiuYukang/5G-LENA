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
 */
#pragma once

#include "mmwave-mac-scheduler-tdma.h"

namespace ns3 {

/**
 * \ingroup mac-schedulers
 * \brief Assign entire symbols in a round-robin fashion
 *
 * Each UE will receive a proportional number of symbols. With \f$n\f$ UE,
 * each one will receive:
 *
 * \f$ sym_{i} = \frac{totSym}{n} \f$
 *
 * If \f$ n > totSym \f$, then there will be UEs which will not have any
 * symbol assigned. The class does not remember the UEs which did not get
 * any symbol in the previous slot, so this opens the door to a possible
 * starvation.
 *
 * \see MmWaveMacSchedulerUeInfoRR
 */
class MmWaveMacSchedulerTdmaRR : public MmWaveMacSchedulerTdma
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief MmWaveMacSchedulerTdmaRR constructor
   */
  MmWaveMacSchedulerTdmaRR ();

  /**
   * \brief ~MmWaveMacSchedulerTdmaRR deconstructor
   */
  virtual ~MmWaveMacSchedulerTdmaRR () override
  {
  }

protected:
  /**
   * \brief Create an UE representation of the type MmWaveMacSchedulerUeInfoRR
   * \param params parameters
   * \return MmWaveMacSchedulerUeInfoRR instance
   */
  virtual std::shared_ptr<MmWaveMacSchedulerUeInfo>
  CreateUeRepresentation (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params) const override;

  /**
   * \brief Return the comparison function to sort DL UE according to the scheduler policy
   * \return a pointer to MmWaveMacSchedulerUeInfoRR::CompareUeWeightsDl
   */
  virtual std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareDlFn () const override;

  /**
   * \brief Return the comparison function to sort UL UE according to the scheduler policy
   * \return a pointer to MmWaveMacSchedulerUeInfoRR::CompareUeWeightsUl
   */
  virtual std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareUlFn () const override;

  /**
   * \brief Update the UE representation after a symbol (DL) has been assigned to it
   * \param ue UE to which a symbol has been assigned
   * \param assigned the amount of resources assigned
   * \param totAssigned the total amount of resources assigned in the slot
   *
   * Update DL metrics by calling MmWaveMacSchedulerUeInfoRR::UpdateDlMetric
   */
  virtual void AssignedDlResources (const UePtrAndBufferReq &ue,
                                    const FTResources &assigned,
                                    const FTResources &totAssigned) const override;

  /**
   * \brief Update the UE representation after a symbol (DL) has been assigned to it
   * \param ue UE to which a symbol has been assigned
   * \param assigned the amount of resources assigned
   * \param totAssigned the total amount of resources assigned in the slot
   *
   * Update DL metrics by calling MmWaveMacSchedulerUeInfoRR::UpdateUlMetric
   */
  virtual void AssignedUlResources (const UePtrAndBufferReq &ue,
                                    const FTResources &assigned,
                                    const FTResources &totAssigned) const override;

  // RR is a simple scheduler: it doesn't do anything in the next
  // inherited calls.
  virtual void NotAssignedDlResources (const UePtrAndBufferReq &ue,
                                       const FTResources &notAssigned,
                                       const FTResources &totalAssigned) const override
  {
  }

  virtual void NotAssignedUlResources (const UePtrAndBufferReq &ue,
                                       const FTResources &notAssigned,
                                       const FTResources &totalAssigned) const override
  {
  }

  virtual void
  BeforeDlSched (const UePtrAndBufferReq &ue,
                 const FTResources &assignableInIteration) const override
  {
  }

  virtual void
  BeforeUlSched (const UePtrAndBufferReq &ue,
                 const FTResources &assignableInIteration) const override
  {
  }
};

} // namespace ns3
