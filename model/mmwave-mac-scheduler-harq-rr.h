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

#include "mmwave-mac-scheduler-ue-info.h"
#include "mmwave-mac-scheduler-ns3.h"
#include "mmwave-phy-mac-common.h"
#include "nr-amc.h"

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

  /**
   * \brief MmWaveMacSchedulerHarqRr constructor
   * \param config Mac-Phy config
   * \param amc AMC
   */
  MmWaveMacSchedulerHarqRr (const Ptr<NrAmc> &amc);

  /**
    * \brief Default deconstructor
    */
  virtual ~MmWaveMacSchedulerHarqRr () = default;

  /**
   * \brief Install a function to retrieve the bwp id
   * \param fn the function
   */
  void InstallGetBwpIdFn (const std::function<uint16_t ()> &fn);

  /**
   * \brief Install a function to retrieve the cell id
   * \param fn the function
   */
  void InstallGetCellIdFn (const std::function<uint16_t ()> & fn);

  /**
   * \brief Install a function to retrieve the bandwidth in RBG
   * \param fn
   */
  void InstallGetBwInRBG (const std::function<uint16_t ()> & fn);

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
  Ptr<NrAmc> m_amc;                    //!< AMC

protected:
  void BufferHARQFeedback (const std::vector <DlHarqInfo> &dlHarqFeedback,
                           std::vector<DlHarqInfo> *dlHarqToRetransmit,
                           uint16_t rnti, uint8_t harqProcess) const;
  /**
   * \brief Get the bwp id of this MAC
   * \return the bwp id
   */
  uint16_t GetBwpId () const;

  /**
   * \brief Get the cell id of this MAC
   * \return the cell id
   */
  uint16_t GetCellId () const;

  /**
   * \brief Get the bandwidth in RBG
   * \return the BW in RBG
   */
  uint16_t GetBandwidthInRbg () const;

private:
  std::function<uint16_t ()> m_getBwpId;  //!< Function to retrieve bwp id
  std::function<uint16_t ()> m_getCellId; //!< Function to retrieve cell id
  std::function<uint16_t ()> m_getBwInRbg; //!< Function to retrieve bw in rbg
};

} // namespace ns3
