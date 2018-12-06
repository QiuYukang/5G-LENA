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

#include "mmwave-mac-scheduler-ns3.h"

namespace ns3 {

/**
 * \ingroup mac-schedulers
 * \brief UE representation for a round-robin scheduler
 *
 * The UE representation does not store any additional information,
 * but provides a way for a RR scheduler to order the UE based on the assigned
 * RBG.
 *
 * \see CompareUeWeightsDl
 */
class MmWaveMacSchedulerUeInfoRR : public MmWaveMacSchedulerUeInfo
{
public:
  /**
   * \brief MmWaveMacSchedulerUeInfoRR constructor
   * \param rnti RNTI of the UE
   * \param beamId Beam ID of the UE
   */
  MmWaveMacSchedulerUeInfoRR (uint16_t rnti, AntennaArrayModel::BeamId beamId)
    : MmWaveMacSchedulerUeInfo (rnti, beamId)
  {
  }

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the assigned RBG of lue is less than the assigned RBG of rue
   *
   * The ordering is made by considering the RBG. An UE with 0 RBG will always
   * be the first (i.e., has an higher priority) in a RR scheduler. The objective
   * is to distribute evenly all the resources, in order to have the same RBG
   * number for all the UEs.
   */
  static bool CompareUeWeightsDl (const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    return (lue.first->m_dlRBG < rue.first->m_dlRBG);
  }

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the assigned RBG of lue is less than the assigned RBG of rue
   *
   * The ordering is made by considering the RBG. An UE with 0 RBG will always
   * be the first (i.e., has an higher priority) in a RR scheduler. The objective
   * is to distribute evenly all the resources, in order to have the same RBG
   * number for all the UEs.
   */
  static bool CompareUeWeightsUl (const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    return (lue.first->m_ulRBG < rue.first->m_ulRBG);
  }
};

} // namespace ns3
