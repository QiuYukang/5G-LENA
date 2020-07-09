/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#ifndef NR_SL_UE_MAC_SCHEDULER_SIMPLE_H
#define NR_SL_UE_MAC_SCHEDULER_SIMPLE_H


#include "nr-sl-ue-mac-scheduler-ns3.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief A simple NR Sidelink scheduler for NR SL UE in NS3
 */
class NrSlUeMacSchedulerSimple : public NrSlUeMacSchedulerNs3
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrSlUeMacSchedulerNs3 default constructor
   */
  NrSlUeMacSchedulerSimple ();

  /**
   * \brief Do the NE Sidelink allocation
   * \param txOpps The list of the txOpps for the UE MAC
   * \param dstInfo The pointer to the NrSlUeMacSchedulerDstInfo of the destination
   *        for which UE MAC asked the scheduler to allocate the recourses
   * \param slotAlloc The slot allocation structure to be updated by this scheduler
   */
  virtual void DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps,
                                const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                                std::shared_ptr<NrSlUeMacSchedSapUser::NrSlSlotAlloc> &slotAlloc) override;

private:
  /**
   * \brief Select the slots randomly from the available slots
   *
   * This method is optimized to be always able to allocate the slots
   * for 1 or 2 retransmissions if needed. <b>For more than 2 retransmissions
   * this method should be updated.</b>
   *
   * \param txOpps The list of the available TX opportunities
   * \return the set containing the indices of the randomly chosen slots in the
   *         txOpps list
   */
  std::set <uint16_t>
  RandomlySelectSlots (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps);



};

} //namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_SIMPLE_H */
