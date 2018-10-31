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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      if (m_phyMacConfig)                                                \
        {                                                                \
          std::clog << " [ccId "                                         \
                    << static_cast<uint32_t> (m_phyMacConfig->GetCcId ())\
                    << "] ";                                             \
        }                                                                \
    }                                                                    \
  while (false);
#include "mmwave-mac-scheduler-ns3-base.h"
#include "mmwave-mac-scheduler-harq-rr.h"
#include <ns3/log.h>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("MmWaveMacSchedulerNs3Base");
NS_OBJECT_ENSURE_REGISTERED (MmWaveMacSchedulerNs3Base);

TypeId
MmWaveMacSchedulerNs3Base::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacSchedulerNs3Base")
    .SetParent<MmWaveMacSchedulerNs3> ()
    // Add the choice for HARQ scheduler as attribute
  ;
  return tid;
}

MmWaveMacSchedulerNs3Base::MmWaveMacSchedulerNs3Base () : MmWaveMacSchedulerNs3 ()
{

}

void
MmWaveMacSchedulerNs3Base::ConfigureCommonParameters (Ptr<MmWavePhyMacCommon> config)
{
  MmWaveMacSchedulerNs3::ConfigureCommonParameters (config);
  // Hardcoded, but the type can be a parameter if needed
  m_schedHarq = std::unique_ptr<MmWaveMacSchedulerHarqRr> (new MmWaveMacSchedulerHarqRr (m_phyMacConfig, m_amc));
}

/**
 * \brief Invoke MmWaveMacSchedulerHarqRr::ScheduleDlHarq
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
MmWaveMacSchedulerNs3Base::ScheduleDlHarq (MmWaveMacSchedulerNs3::PointInFTPlane *startingPoint,
                                           uint8_t symAvail,
                                           const ActiveHarqMap &activeDlHarq,
                                           const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &ueMap,
                                           std::vector<DlHarqInfo> *dlHarqToRetransmit,
                                           const std::vector<DlHarqInfo> &dlHarqFeedback,
                                           SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  return m_schedHarq->ScheduleDlHarq (startingPoint, symAvail, activeDlHarq,
                                      ueMap, dlHarqToRetransmit, dlHarqFeedback, slotAlloc);
}

/**
 * \brief Invoke MmWaveMacSchedulerHarqRr::ScheduleUlHarq
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
MmWaveMacSchedulerNs3Base::ScheduleUlHarq (MmWaveMacSchedulerNs3::PointInFTPlane *startingPoint,
                                           uint8_t symAvail,
                                           const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &ueMap,
                                           std::vector<UlHarqInfo> *ulHarqToRetransmit,
                                           const std::vector<UlHarqInfo> &ulHarqFeedback,
                                           SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  return m_schedHarq->ScheduleUlHarq (startingPoint, symAvail,
                                      ueMap, ulHarqToRetransmit, ulHarqFeedback, slotAlloc);
}

/**
 * \brief Invoke MmWaveMacSchedulerHarqRr::SortDlHarq
 * \param activeDlHarq active HARQ
 */
void
MmWaveMacSchedulerNs3Base::SortDlHarq (MmWaveMacSchedulerNs3::ActiveHarqMap *activeDlHarq) const
{
  NS_LOG_FUNCTION (this);
  m_schedHarq->SortDlHarq (activeDlHarq);
}

/**
 * \brief Invoke MmWaveMacSchedulerHarqRr::SortUlHarq
 * \param activeUlHarq active HARQ
 */
void
MmWaveMacSchedulerNs3Base::SortUlHarq (MmWaveMacSchedulerNs3::ActiveHarqMap *activeUlHarq) const
{
  NS_LOG_FUNCTION (this);
  m_schedHarq->SortDlHarq (activeUlHarq);
}

std::vector<MmWaveMacSchedulerNs3::UePtrAndBufferReq>
MmWaveMacSchedulerNs3Base::GetUeVectorFromActiveUeMap (const MmWaveMacSchedulerNs3::ActiveUeMap &activeUes)
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
