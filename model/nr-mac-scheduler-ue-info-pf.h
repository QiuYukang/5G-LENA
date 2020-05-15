/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "nr-mac-scheduler-ue-info-rr.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief UE representation for a proportional fair scheduler
 *
 * The representation stores the current throughput, the average throughput,
 * and the last average throughput, as well as providing comparison functions
 * to sort the UEs in case of a PF scheduler.
 *
 * \see CompareUeWeightsDl
 */
class NrMacSchedulerUeInfoPF : public NrMacSchedulerUeInfo
{
public:
  /**
   * \brief NrMacSchedulerUeInfoPF constructor
   * \param rnti RNTI of the UE
   * \param beamId Beam ID of the UE
   * \param fn A function that tells how many RB per RBG
   */
  NrMacSchedulerUeInfoPF (float alpha, uint16_t rnti, BeamId beamId, const GetRbPerRbgFn &fn)
    : NrMacSchedulerUeInfo (rnti, beamId, fn),
    m_alpha (alpha)
  {
  }

  /**
   * \brief Reset PF scheduler info
   *
   * Set the last average throughput to the current average throughput,
   * and zeroes the average throughput as well as the current throughput.
   *
   * It calls also NrMacSchedulerUeInfo::ResetDlSchedInfo.
   */
  virtual void ResetDlSchedInfo () override
  {
    m_lastAvgTputDl = m_avgTputDl;
    m_avgTputDl = 0.0;
    m_currTputDl = 0.0;
    m_potentialTput = 0.0;
    NrMacSchedulerUeInfo::ResetDlSchedInfo ();
  }

  /**
   * \brief Reset the avg Th to the last value
   */
  virtual void ResetDlMetric () override
  {
    NrMacSchedulerUeInfo::ResetDlMetric ();
    m_avgTputDl = m_lastAvgTputDl;
  }

  /**
   * \brief Update the PF metric for downlink
   * \param totAssigned the resources assigned
   * \param timeWindow the time window
   * \param amc a pointer to the AMC
   */
  void UpdateDlPFMetric (const NrMacSchedulerNs3::FTResources &totAssigned,
                         double timeWindow,
                         const Ptr<const NrAmc> &amc);
  /**
   * \brief Calculate the Potential throughput
   * \param assignableInIteration resources assignable
   * \param amc a pointer to the AMC
   */
  void CalculatePotentialTPut (const NrMacSchedulerNs3::FTResources &assignableInIteration,
                               const Ptr<const NrAmc> &amc);

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the PF metric of the left UE are higher than the right UE
   *
   * The PF metric is calculated as following:
   *
   * \f$ pfMetric_{i} = std::pow(potentialTPut_{i}, alpha) / std::max (1E-9, m_avgTput_{i}) \f$
   *
   * Alpha is a fairness metric. Please note that the throughput is calculated
   * in bit/symbol.
   */
  static bool CompareUeWeightsDl (const NrMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const NrMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    auto luePtr = dynamic_cast<NrMacSchedulerUeInfoPF*> (lue.first.get ());
    auto ruePtr = dynamic_cast<NrMacSchedulerUeInfoPF*> (rue.first.get ());

    double lPfMetric = std::pow (luePtr->m_potentialTput, luePtr->m_alpha) / std::max (1E-9, luePtr->m_avgTputDl);
    double rPfMetric = std::pow (ruePtr->m_potentialTput, ruePtr->m_alpha) / std::max (1E-9, ruePtr->m_avgTputDl);

    return (lPfMetric > rPfMetric);
  }

  double m_currTputDl {0.0};    //!< Current slot throughput
  double m_avgTputDl  {0.0};    //!< Average throughput during all the slots
  double m_lastAvgTputDl {0.0}; //!< Last average throughput
  double m_potentialTput {0.0}; //!< Potential throughput in one assignable resource (can be a symbol or a RBG)
  float  m_alpha {0.0};         //!< PF fairness metric
};

} // namespace ns3
