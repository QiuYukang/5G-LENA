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
#include "mmwave-mac-scheduler-harq-rr.h"

namespace ns3 {

/**
 * \ingroup mac-schedulers
 * \brief The MmWaveMacSchedulerNs3Base class: add Harq on top of MmWaveMacSchedulerNs3
 *
 * The class is responsible to manage the HARQ retransmission on behalf of
 * its parent, MmWaveMacSchedulerNs3. Right now, all the duties are
 * delegated to a class of type MmWaveMacSchedulerHarqRr, which schedules
 * the retransmission in a round robin fashion.
 *
 * It would be awesome if different types of HARQ scheduling could be selected
 * through an attribute of the class. To do so, it is necessary to create
 * a pure virtual interface for HARQ schedulers (the methods in
 * MmWaveMacSchedulerHarqRr could be used as reference) and then create
 * various subclasses of the interface, which specialize the behavior.
 *
 * This class will then be used to choose among the different policies with
 * an attribute.
 */
class MmWaveMacSchedulerNs3Base : public MmWaveMacSchedulerNs3
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief MmWaveMacSchedulerNs3Base default constructor
   */
  MmWaveMacSchedulerNs3Base ();
  /**
   * \brief ~MmWaveMacSchedulerNs3Base
   */
  virtual ~MmWaveMacSchedulerNs3Base () override = default;

  /**
   * \brief Retrieve the UE vector from an ActiveUeMap
   * \param activeUes UE map
   * \return A Vector of UEs and their buffer requirements (in B)
   *
   * Really used only in TDMA scheduling. Worth moving?
   */
  static std::vector<UePtrAndBufferReq>
  GetUeVectorFromActiveUeMap (const ActiveUeMap &activeUes);

  // inherited
  virtual void ConfigureCommonParameters (Ptr<MmWavePhyMacCommon> config) override;

protected:
  virtual uint8_t ScheduleDlHarq (MmWaveMacSchedulerNs3::PointInFTPlane *startingPoint,
                                  uint8_t symAvail,
                                  const ActiveHarqMap &activeDlHarq,
                                  const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &ueMap,
                                  std::vector<DlHarqInfo> *dlHarqToRetransmit,
                                  const std::vector<DlHarqInfo> &dlHarqFeedback,
                                  SlotAllocInfo *slotAlloc) const override;
  virtual uint8_t ScheduleUlHarq (MmWaveMacSchedulerNs3::PointInFTPlane *startingPoint,
                                  uint8_t symAvail,
                                  const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &ueMap,
                                  std::vector<UlHarqInfo> *ulHarqToRetransmit,
                                  const std::vector<UlHarqInfo> &ulHarqFeedback,
                                  SlotAllocInfo *slotAlloc) const override;
  virtual void SortDlHarq (ActiveHarqMap *activeDlHarq) const override;
  virtual void SortUlHarq (ActiveHarqMap *activeUlHarq) const override;

private:
  std::unique_ptr <MmWaveMacSchedulerHarqRr> m_schedHarq; //!< Pointer to the real HARQ scheduler
};

} // namespace ns3
