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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);
#include "nr-mac-scheduler-ns3-base.h"
#include "nr-mac-scheduler-harq-rr.h"
#include <ns3/log.h>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerNs3Base");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerNs3Base);

TypeId
NrMacSchedulerNs3Base::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerNs3Base")
    .SetParent<NrMacSchedulerNs3> ()
    // Add the choice for HARQ scheduler as attribute
  ;
  return tid;
}

NrMacSchedulerNs3Base::NrMacSchedulerNs3Base () : NrMacSchedulerNs3 ()
{
  // Hardcoded, but the type can be a parameter if needed
  m_schedHarq = std::unique_ptr<NrMacSchedulerHarqRr> (new NrMacSchedulerHarqRr ());
  m_schedHarq->InstallGetBwInRBG (std::bind (&NrMacSchedulerNs3Base::GetBandwidthInRbg, this));
  m_schedHarq->InstallGetBwpIdFn (std::bind (&NrMacSchedulerNs3Base::GetBwpId, this));
  m_schedHarq->InstallGetCellIdFn (std::bind (&NrMacSchedulerNs3Base::GetCellId, this));
}

/**
 * \brief Invoke NrMacSchedulerHarqRr::ScheduleDlHarq
 * \param startingPoint starting point of the first retransmission.
 * It should be set to the next available starting point
 * \param symAvail Available symbols
 * \param activeDlHarq Map of the active HARQ processes
 * \param ueMap Map of the UEs
 * \param dlHarqToRetransmit HARQ feedbacks that could not be transmitted (to fill)
 * \param dlHarqFeedback all the HARQ feedbacks
 * \param slotAlloc Slot allocation info
 * \return the VarTtiSlotAlloc ID to use next
 */
uint8_t
NrMacSchedulerNs3Base::ScheduleDlHarq (NrMacSchedulerNs3::PointInFTPlane *startingPoint,
                                           uint8_t symAvail,
                                           const ActiveHarqMap &activeDlHarq,
                                           const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > &ueMap,
                                           std::vector<DlHarqInfo> *dlHarqToRetransmit,
                                           const std::vector<DlHarqInfo> &dlHarqFeedback,
                                           SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  return m_schedHarq->ScheduleDlHarq (startingPoint, symAvail, activeDlHarq,
                                      ueMap, dlHarqToRetransmit, dlHarqFeedback, slotAlloc);
}

/**
 * \brief Invoke NrMacSchedulerHarqRr::ScheduleUlHarq
 * \param startingPoint starting point of the first retransmission.
 * It should be set to the next available starting point
 * \param symAvail Available symbols
 * \param ueMap Map of the UEs
 * \param ulHarqToRetransmit HARQ feedbacks that could not be transmitted (to fill)
 * \param ulHarqFeedback all the HARQ feedbacks
 * \param slotAlloc Slot allocation info
 * \return the VarTtiSlotAlloc ID to use next
 */
uint8_t
NrMacSchedulerNs3Base::ScheduleUlHarq (NrMacSchedulerNs3::PointInFTPlane *startingPoint,
                                           uint8_t symAvail,
                                           const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > &ueMap,
                                           std::vector<UlHarqInfo> *ulHarqToRetransmit,
                                           const std::vector<UlHarqInfo> &ulHarqFeedback,
                                           SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  return m_schedHarq->ScheduleUlHarq (startingPoint, symAvail,
                                      ueMap, ulHarqToRetransmit, ulHarqFeedback, slotAlloc);
}

/**
 * \brief Invoke NrMacSchedulerHarqRr::SortDlHarq
 * \param activeDlHarq active HARQ
 */
void
NrMacSchedulerNs3Base::SortDlHarq (NrMacSchedulerNs3::ActiveHarqMap *activeDlHarq) const
{
  NS_LOG_FUNCTION (this);
  m_schedHarq->SortDlHarq (activeDlHarq);
}

/**
 * \brief Invoke NrMacSchedulerHarqRr::SortUlHarq
 * \param activeUlHarq active HARQ
 */
void
NrMacSchedulerNs3Base::SortUlHarq (NrMacSchedulerNs3::ActiveHarqMap *activeUlHarq) const
{
  NS_LOG_FUNCTION (this);
  m_schedHarq->SortDlHarq (activeUlHarq);
}

std::vector<NrMacSchedulerNs3::UePtrAndBufferReq>
NrMacSchedulerNs3Base::GetUeVectorFromActiveUeMap (const NrMacSchedulerNs3::ActiveUeMap &activeUes)
{
  std::vector<UePtrAndBufferReq> ueVector;
  for (const auto &el : activeUes)
    {
      uint64_t size = ueVector.size ();
      GetSecond GetUeVector;
      for (const auto &ue : GetUeVector (el))
        {
          ueVector.emplace_back (ue);
        }
      NS_ASSERT (size + GetUeVector (el).size () == ueVector.size ());
    }
  return ueVector;
}

}  // namespace ns3
