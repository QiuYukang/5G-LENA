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
 * \brief Assign frequencies in a maximum-rate fashion
 *
 * The UEs will be sorted by their MCS. Higher MCS will always be scheduled
 * before lower MCS, until they do not have any more bytes to transmit.
 *
 * \see MmWaveMacSchedulerUeInfoMR
 */
class MmWaveMacSchedulerOfdmaMR : public MmWaveMacSchedulerOfdmaRR
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief MmWaveMacSchedulerOfdmaMR constructor
   */
  MmWaveMacSchedulerOfdmaMR ();

  /**
   * \brief ~MmWaveMacSchedulerOfdmaMR deconstructor
   */
  virtual ~MmWaveMacSchedulerOfdmaMR () override
  {
  }

protected:
  /**
   * \brief Create an UE representation of the type MmWaveMacSchedulerUeInfoMR
   * \param params parameters
   * \return MmWaveMacSchedulerUeInfoRR instance
   */
  virtual std::shared_ptr<MmWaveMacSchedulerUeInfo>
  CreateUeRepresentation (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params) const override;

  /**
   * \brief Return the comparison function to sort DL UE according to the scheduler policy
   * \return a pointer to MmWaveMacSchedulerUeInfoMR::CompareUeWeightsDl
   */
  virtual std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareDlFn () const override;

  /**
   * \brief Return the comparison function to sort UL UE according to the scheduler policy
   * \return a pointer to MmWaveMacSchedulerUeInfoMR::CompareUeWeightsUl
   */
  virtual std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                             const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
  GetUeCompareUlFn () const override;
};

} // namespace ns3
