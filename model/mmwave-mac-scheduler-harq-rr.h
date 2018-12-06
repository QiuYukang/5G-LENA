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
 */
#pragma once

#include "mmwave-mac-scheduler-ue-info.h"
#include "mmwave-mac-scheduler-ns3.h"
#include "mmwave-phy-mac-common.h"
#include "mmwave-amc.h"

namespace ns3 {

/**
 * \ingroup mac-schedulers
 * \brief Schedule the HARQ retransmission
 *
 * The class manages, in a round-robin fashion, the retransmission to be
 * performed. It implements ScheduleDlHarq and ScheduleUlHarq that
 * has the same signature of the methods in MmWaveMacSchedulerNs3. For the
 * details about the HARQ scheduling, please refer to the method documentation.
 */
class MmWaveMacSchedulerHarqRr
{
public:
  using Ns3Sched = MmWaveMacSchedulerNs3;
  using BeamId = AntennaArrayModel::BeamId;

  /**
   * \brief MmWaveMacSchedulerHarqRr constructor
   * \param config Mac-Phy config
   * \param amc AMC
   */
  MmWaveMacSchedulerHarqRr (const Ptr<MmWavePhyMacCommon> &config, const Ptr<MmWaveAmc> &amc)
  {
    m_phyMacConfig = config;
    m_amc = amc;
  }

  /**
    * \brief Default deconstructor
    */
  virtual ~MmWaveMacSchedulerHarqRr () = default;

  virtual uint8_t ScheduleDlHarq (MmWaveMacSchedulerNs3::PointInFTPlane *startingPoint,
                                  uint8_t symAvail,
                                  const MmWaveMacSchedulerNs3::ActiveHarqMap &activeDlHarq,
                                  const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &ueMap,
                                  std::vector<DlHarqInfo> *dlHarqToRetransmit,
                                  const std::vector<DlHarqInfo> &dlHarqFeedback,
                                  SlotAllocInfo *slotAlloc) const;
  virtual uint8_t ScheduleUlHarq (MmWaveMacSchedulerNs3::PointInFTPlane *startingPoint,
                                  uint8_t symAvail,
                                  const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &ueMap,
                                  std::vector<UlHarqInfo> *ulHarqToRetransmit,
                                  const std::vector<UlHarqInfo> &ulHarqFeedback,
                                  SlotAllocInfo *slotAlloc) const;
  virtual void SortDlHarq (MmWaveMacSchedulerNs3::ActiveHarqMap *activeDlHarq) const;
  virtual void SortUlHarq (MmWaveMacSchedulerNs3::ActiveHarqMap *activeUlHarq) const;

protected:
  Ptr<MmWavePhyMacCommon> m_phyMacConfig;  //!< phy mac config
  Ptr<MmWaveAmc> m_amc;                    //!< AMC

protected:
  void BufferHARQFeedback (const std::vector <DlHarqInfo> &dlHarqFeedback,
                           std::vector<DlHarqInfo> *dlHarqToRetransmit,
                           uint16_t rnti, uint8_t harqProcess) const;
};

} // namespace ns3
