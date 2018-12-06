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

#include "mmwave-mac-scheduler-ue-info-rr.h"

namespace ns3 {

/**
 * \ingroup mac-schedulers
 * \brief UE representation for a proportional fair scheduler
 *
 * The representation stores the current throughput, the average throughput,
 * and the last average throughput, as well as providing comparison functions
 * to sort the UEs in case of a PF scheduler.
 *
 * \see CompareUeWeightsDl
 */
class MmWaveMacSchedulerUeInfoPF : public MmWaveMacSchedulerUeInfo
{
public:
  /**
   * \brief MmWaveMacSchedulerUeInfoPF constructor
   * \param rnti RNTI of the UE
   * \param beamId Beam ID of the UE
   */
  MmWaveMacSchedulerUeInfoPF (float alpha, uint16_t rnti, AntennaArrayModel::BeamId beamId)
    : MmWaveMacSchedulerUeInfo (rnti, beamId),
    m_alpha (alpha)
  {
  }

  /**
   * \brief Reset PF scheduler info
   *
   * Set the last average throughput to the current average throughput,
   * and zeroes the average throughput as well as the current throughput.
   *
   * It calls also MmWaveMacSchedulerUeInfo::ResetDlSchedInfo.
   */
  virtual void ResetDlSchedInfo () override
  {
    m_lastAvgTputDl = m_avgTputDl;
    m_avgTputDl = 0.0;
    m_currTputDl = 0.0;
    m_potentialTput = 0.0;
    MmWaveMacSchedulerUeInfo::ResetDlSchedInfo ();
  }

  /**
   * \brief Reset the avg Th to the last value
   */
  virtual void ResetDlMetric () override
  {
    MmWaveMacSchedulerUeInfo::ResetDlMetric ();
    m_avgTputDl = m_lastAvgTputDl;
  }

  void UpdateDlPFMetric (const MmWaveMacSchedulerNs3::FTResources &totAssigned,
                         double timeWindow,
                         const Ptr<MmWavePhyMacCommon> &config,
                         const Ptr<MmWaveAmc> &amc);
  void CalculatePotentialTPut (const MmWaveMacSchedulerNs3::FTResources &assignableInIteration,
                               const Ptr<MmWavePhyMacCommon> &config,
                               const Ptr<MmWaveAmc> &amc);

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
   * Alpha is a fairness metric.
   */
  static bool CompareUeWeightsDl (const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    auto luePtr = dynamic_cast<MmWaveMacSchedulerUeInfoPF*> (lue.first.get ());
    auto ruePtr = dynamic_cast<MmWaveMacSchedulerUeInfoPF*> (rue.first.get ());

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
