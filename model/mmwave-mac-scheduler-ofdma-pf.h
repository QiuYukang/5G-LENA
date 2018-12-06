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
#include "mmwave-mac-scheduler-ofdma-rr.h"

namespace ns3 {

/**
 * \ingroup mac-schedulers
 * \brief Assign frequencies in a proportional fair fashion
 *
 * Sort the UE by their current throughput. Number of symbols is fixed depending
 * on the beam requirements.
 *
 * Details of the sorting function in the class MmWaveMacSchedulerUeInfoPF.
 */
class MmWaveMacSchedulerOfdmaPF : public MmWaveMacSchedulerOfdmaRR
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);
  /**
   * \brief MmWaveMacSchedulerOfdmaPF constructor
   */
  MmWaveMacSchedulerOfdmaPF ();

  /**
   * \brief ~MmWaveMacSchedulerOfdmaPF deconstructor
   */
  virtual ~MmWaveMacSchedulerOfdmaPF () override
  {
  }

protected:
  // inherit
  /**
   * \brief Create an UE representation of the type MmWaveMacSchedulerUeInfoPF
   * \param params parameters
   * \return MmWaveMacSchedulerUeInfoRR instance
   */
  virtual std::shared_ptr<MmWaveMacSchedulerUeInfo>
  CreateUeRepresentation (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params) const override;

  /**
   * \brief Return the comparison function to sort DL UE according to the scheduler policy
   * \return a pointer to MmWaveMacSchedulerUeInfoPF::CompareUeWeightsDl
   */
  virtual std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareDlFn () const override;

  /**
   * \brief Return the comparison function to sort UL UE according to the scheduler policy
   * \return a pointer to MmWaveMacSchedulerUeInfoPF::CompareUeWeightsUl
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

  virtual void NotAssignedDlResources (const UePtrAndBufferReq &ue,
                                       const FTResources &notAssigned,
                                       const FTResources &totalAssigned) const override;

  virtual void
  BeforeDlSched (const UePtrAndBufferReq &ue,
                 const FTResources &assignableInIteration) const override;


private:
  double m_timeWindow {99.0}; //!< Time window to calculate the throughput. Better to make it an attribute.
  float m_alpha {0.0}; //!< PF Fairness index
};

} // namespace ns3
