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
#include "mmwave-mac-scheduler-ue-info-rr.h"

namespace ns3 {

/**
 * \ingroup mac-schedulers
 * \brief UE representation for a maximum rate scheduler
 *
 * The class does not store anything more than the MmWaveMacSchedulerUeInfo
 * base class. However, it provides functions to sort the UE based on their
 * maximum achievable rate.
 *
 * \see CompareUeWeightsDl
 */
class MmWaveMacSchedulerUeInfoMR : public MmWaveMacSchedulerUeInfo
{
public:
  /**
   * \brief MmWaveMacSchedulerUeInfoMR constructor
   * \param rnti RNTI of the UE
   * \param beamId Beam ID of the UE
   */
  MmWaveMacSchedulerUeInfoMR (uint16_t rnti, AntennaArrayModel::BeamId beamId)
    : MmWaveMacSchedulerUeInfo (rnti, beamId)
  {
  }

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the MCS of lue is greater than the MCS of rue
   *
   * The ordering is made by considering the MCS of the UE. The higher the MCS,
   * the higher the assigned resources until it has enough to transmit the data.
   */
  static bool CompareUeWeightsDl (const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    if (lue.first->m_dlMcs == rue.first->m_dlMcs)
      {
        return MmWaveMacSchedulerUeInfoRR::CompareUeWeightsDl (lue, rue);
      }

    return (lue.first->m_dlMcs > rue.first->m_dlMcs);
  }

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the MCS of lue is greater than the MCS of rue
   *
   * The ordering is made by considering the MCS of the UE. The higher the MCS,
   * the higher the assigned resources until it has enough to transmit the data.
   */
  static bool CompareUeWeightsUl (const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    if (lue.first->m_ulMcs == rue.first->m_ulMcs)
      {
        return MmWaveMacSchedulerUeInfoRR::CompareUeWeightsUl (lue, rue);
      }

    return (lue.first->m_ulMcs > rue.first->m_ulMcs);
  }
};

} // namespace ns3
