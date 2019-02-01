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
#include "mmwave-mac-scheduler-tdma.h"
#include "mmwave-mac-scheduler-ue-info-pf.h"
#include <ns3/log.h>
#include <algorithm>
#include <functional>

namespace ns3  {

NS_LOG_COMPONENT_DEFINE ("MmWaveMacSchedulerTdma");
NS_OBJECT_ENSURE_REGISTERED (MmWaveMacSchedulerTdma);

TypeId
MmWaveMacSchedulerTdma::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacSchedulerTdma")
    .SetParent<MmWaveMacSchedulerNs3Base> ()
  ;
  return tid;
}

MmWaveMacSchedulerTdma::MmWaveMacSchedulerTdma ()
  : MmWaveMacSchedulerNs3Base ()
{
}

MmWaveMacSchedulerTdma::~MmWaveMacSchedulerTdma ()
{
}

/**
 * \brief Assign the available RBG in a TDMA fashion
 * \param symAvail Number of available symbols
 * \param activeUe active flows and UE
 * \param type String representing the type of allocation currently in act (DL or UL)
 * \param BeforeSchedFn Function to call before any scheduling is started
 * \param GetTBSFn Function to call to get a reference of the UL or DL TBS
 * \param GetRBGFn Function to call to get a reference of the UL or DL RBG
 * \param GetSymFn Function to call to get a reference of the UL or DL symbols
 * \param GetRBGFn Function to call to compare UEs during assignment
 * \param SuccessfullAssignmentFn Function to call one time for the UE that got the resources assigned in one iteration
 * \param UnSuccessfullAssignmentFn Function to call for the UEs that did not get anything in one iteration
 *
 * \return a map between the beam and the symbols assigned to each one
 *
 * The algorithm redistributes the number of symbols to all the UEs. The
 * pseudocode is the following:
 * <pre>
 * for (ue : activeUe):
 *    BeforeSchedFn (ue);
 *
 * while symbols > 0:
 *    sort (ueVector);
 *    GetRBGFn(ueVector.first()) += BandwidthInRBG();
 *    symbols--;
 *    SuccessfullAssignmentFn (ueVector.first());
 *    for each ue that did not get anything assigned:
 *        UnSuccessfullAssignmentFn (ue);
 * </pre>
 *
 * To sort the UEs, the method uses the function returned by GetUeCompareDlFn().
 * Two fairness helper are hard-coded in the method: the first one is avoid
 * to assign resources to UEs that already have their buffer requirement covered,
 * and the other one is avoid to assign symbols when all the UEs have their
 * requirements covered.
 *
 * The distribution of each symbol is called 'iteration' in other part of the
 * class documentation.
 *
 * The function, thanks to the callback parameters, can be adapted to do
 * a UL or DL allocation. Please make sure the callbacks return references
 * (or no effects will be seen on the caller).
 *
 * \see BeforeDlSched
 */
MmWaveMacSchedulerTdma::BeamSymbolMap
MmWaveMacSchedulerTdma::AssignRBGTDMA (uint32_t symAvail, const ActiveUeMap &activeUe,
                                       const std::string &type, const BeforeSchedFn &BeforeSchedFn,
                                       const GetCompareUeFn &GetCompareFn,
                                       const GetTBSFn &GetTBSFn, const GetRBGFn &GetRBGFn,
                                       const GetSymFn &GetSymFn,
                                       const AfterSuccessfullAssignmentFn &SuccessfullAssignmentFn,
                                       const AfterUnsucessfullAssignmentFn &UnSuccessfullAssignmentFn) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Assigning RBG in " << type <<  ", # beams active flows: " <<
                activeUe.size () << ", # sym: " << symAvail);

  // Create vector of UE (without considering the beam)
  std::vector<UePtrAndBufferReq> ueVector = GetUeVectorFromActiveUeMap (activeUe);

  for (auto & ue : ueVector)
    {
      BeforeSchedFn (ue, FTResources (m_phyMacConfig->GetBandwidthInRbg (), 1));
    }

  // Distribute the symbols following the selected behaviour among UEs
  uint32_t resources = symAvail;
  FTResources assigned (0, 0);

  while (resources > 0)
    {
      GetFirst GetUe;

      auto schedInfoIt = ueVector.begin ();
      uint32_t bufQueueSize = schedInfoIt->second;
      NS_ASSERT (bufQueueSize > 0); // Otherwise, something broken in the calculation of active UE

      std::sort (ueVector.begin (), ueVector.end (), GetCompareFn ());

      // Ensure fairness: pass over UEs which already has enough resources to transmit
      while (schedInfoIt != ueVector.end ())
        {
          bufQueueSize = schedInfoIt->second;

          if (GetTBSFn (GetUe (*schedInfoIt)) >= bufQueueSize)
            {
              NS_LOG_INFO ("UE " << GetUe (*schedInfoIt)->m_rnti << " TBS " <<
                           GetTBSFn (GetUe (*schedInfoIt)) << " queue " <<
                           bufQueueSize << ", passing");
              schedInfoIt++;
            }
          else
            {
              break;
            }
        }

      // In the case that all the UE already have their requirements fullfilled,
      // then stop the assignment
      if (schedInfoIt == ueVector.end ())
        {
          NS_LOG_INFO ("All the UE already have their resources allocated. Skipping the beam");
          break;
        }

      // Assign 1 entire symbol (full RBG) to the selected UE and to the total
      // resources assigned count
      GetRBGFn (GetUe (*schedInfoIt)) += m_phyMacConfig->GetBandwidthInRbg ();
      assigned.m_rbg += m_phyMacConfig->GetBandwidthInRbg ();

      GetSymFn (GetUe (*schedInfoIt)) += 1;
      assigned.m_sym += 1;

      // substract 1 SYM from the number of sym available for the while loop
      resources -= 1;

      // Update metrics for the successfull UE
      NS_LOG_DEBUG ("Assigned " << m_phyMacConfig->GetBandwidthInRbg () <<
                    " " << type << " RBG (= 1 SYM) to UE " << GetUe (*schedInfoIt)->m_rnti);
      SuccessfullAssignmentFn (*schedInfoIt, FTResources (m_phyMacConfig->GetBandwidthInRbg (), 1),
                               assigned);

      // Update metrics for the unsuccessfull UEs (who did not get any resource in this iteration)
      for (auto & ue : ueVector)
        {
          if (GetUe (ue)->m_rnti != GetUe (*schedInfoIt)->m_rnti)
            {
              UnSuccessfullAssignmentFn (ue, FTResources (m_phyMacConfig->GetBandwidthInRbg (), 1),
                                         assigned);
            }
        }
    }

  // Count the number of assigned symbol of each beam.
  MmWaveMacSchedulerTdma::BeamSymbolMap ret;
  for (const auto &el : activeUe)
    {
      uint32_t symOfBeam = 0;
      for (const auto &ue : el.second)
        {
          symOfBeam += GetRBGFn (ue.first) / m_phyMacConfig->GetBandwidthInRbg ();
          NS_ASSERT (GetRBGFn (ue.first) % m_phyMacConfig->GetBandwidthInRbg () == 0);
        }
      ret.insert (std::make_pair (el.first, symOfBeam));
    }
  return ret;
}

/**
 * \brief Assign the available DL RBG to the UEs
 * \param symAvail Number of available symbols
 * \param activeDl active DL flows and UE
 * \return a map between the beam and the symbols assigned to each one
 *
 * The function will prepare all the needed callbacks to return UE DL parameters
 * (e.g., the DL TBS, the DL RBG) and then will call MmWaveMacSchedulerTdma::AssignRBGTDMA.
 */
MmWaveMacSchedulerTdma::BeamSymbolMap
MmWaveMacSchedulerTdma::AssignDLRBG (uint32_t symAvail, const ActiveUeMap &activeDl) const
{
  NS_LOG_FUNCTION (this);

  BeforeSchedFn beforeSched = std::bind (&MmWaveMacSchedulerTdma::BeforeDlSched, this,
                                         std::placeholders::_1, std::placeholders::_2);
  AfterSuccessfullAssignmentFn SuccFn = std::bind (&MmWaveMacSchedulerTdma::AssignedDlResources, this,
                                                   std::placeholders::_1, std::placeholders::_2,
                                                   std::placeholders::_3);
  AfterUnsucessfullAssignmentFn UnSuccFn = std::bind (&MmWaveMacSchedulerTdma::NotAssignedDlResources, this,
                                                      std::placeholders::_1, std::placeholders::_2,
                                                      std::placeholders::_3);
  GetCompareUeFn compareFn = std::bind (&MmWaveMacSchedulerTdma::GetUeCompareDlFn, this);

  GetTBSFn GetTbs = &MmWaveMacSchedulerUeInfo::GetDlTBS;
  GetRBGFn GetRBG = &MmWaveMacSchedulerUeInfo::GetDlRBG;
  GetSymFn GetSym = &MmWaveMacSchedulerUeInfo::GetDlSym;

  return AssignRBGTDMA (symAvail, activeDl, "DL", beforeSched, compareFn,
                        GetTbs, GetRBG, GetSym, SuccFn, UnSuccFn);
}

/**
 * \brief Assign the available UL RBG to the UEs
 * \param symAvail Number of available symbols
 * \param activeUl active DL flows and UE
 * \return a map between the beam and the symbols assigned to each one
 *
 * The function will prepare all the needed callbacks to return UE UL parameters
 * (e.g., the UL TBS, the UL RBG) and then will call MmWaveMacSchedulerTdma::AssignRBGTDMA.
 */
MmWaveMacSchedulerTdma::BeamSymbolMap
MmWaveMacSchedulerTdma::AssignULRBG (uint32_t symAvail, const ActiveUeMap &activeUl) const
{
  NS_LOG_FUNCTION (this);
  BeforeSchedFn beforeSched = std::bind (&MmWaveMacSchedulerTdma::BeforeUlSched, this,
                                         std::placeholders::_1, std::placeholders::_2);
  AfterSuccessfullAssignmentFn SuccFn = std::bind (&MmWaveMacSchedulerTdma::AssignedUlResources, this,
                                                   std::placeholders::_1, std::placeholders::_2,
                                                   std::placeholders::_3);
  GetCompareUeFn compareFn = std::bind (&MmWaveMacSchedulerTdma::GetUeCompareUlFn, this);
  AfterUnsucessfullAssignmentFn UnSuccFn = std::bind (&MmWaveMacSchedulerTdma::NotAssignedUlResources, this,
                                                      std::placeholders::_1, std::placeholders::_2,
                                                      std::placeholders::_3);

  GetTBSFn GetTbs = &MmWaveMacSchedulerUeInfo::GetUlTBS;
  GetRBGFn GetRBG = &MmWaveMacSchedulerUeInfo::GetUlRBG;
  GetSymFn GetSym = &MmWaveMacSchedulerUeInfo::GetUlSym;

  return AssignRBGTDMA (symAvail, activeUl, "UL", beforeSched, compareFn,
                        GetTbs, GetRBG, GetSym, SuccFn, UnSuccFn);
}

/**
 * \brief Create a DL DCI starting from spoint and spanning maxSym symbols
 * \param spoint Starting point of the DCI
 * \param ueInfo UE representation
 * \param maxSym Maximum number of symbols for the creation of the DCI
 * \return a pointer to the newly created DCI
 *
 * The method calculates the TBS and the real number of symbols needed, and
 * then call CreateDci().
 */
std::shared_ptr<DciInfoElementTdma>
MmWaveMacSchedulerTdma::CreateDlDci (PointInFTPlane *spoint,
                                     const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo,
                                     uint32_t maxSym) const
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (maxSym);
  uint32_t tbs = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_dlMcs,
                                                 ueInfo->m_dlRBG * m_phyMacConfig->GetNumRbPerRbg ()) / 8;
  if (tbs < 4)
    {
      NS_LOG_DEBUG ("While creating DCI for UE " << ueInfo->m_rnti <<
                    " assigned " << ueInfo->m_dlRBG << " DL RBG, but TBS < 4");
      return nullptr;
    }

  uint8_t numSym = static_cast<uint8_t> (ueInfo->m_dlRBG / m_phyMacConfig->GetBandwidthInRbg ());

  auto dci = CreateDci (spoint, ueInfo, tbs, DciInfoElementTdma::DL, ueInfo->m_dlMcs,
                        std::max (numSym, static_cast<uint8_t> (1)));

  // The starting point must advance.
  spoint->m_rbg = 0;
  spoint->m_sym += numSym;

  return dci;
}

/**
 * \brief Create a UL DCI starting from spoint and spanning maxSym symbols
 * \param spoint Starting point of the DCI
 * \param ueInfo UE representation
 * \return a pointer to the newly created DCI
 *
 * The method calculates the TBS and the real number of symbols needed, and
 * then call CreateDci().
 * Allocate the DCI going bacward from the starting point (it should be called
 * ending point maybe).
 */
std::shared_ptr<DciInfoElementTdma>
MmWaveMacSchedulerTdma::CreateUlDci (MmWaveMacSchedulerNs3::PointInFTPlane *spoint,
                                     const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo) const
{
  NS_LOG_FUNCTION (this);
  uint32_t tbs = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_ulMcs,
                                                 ueInfo->m_ulRBG * m_phyMacConfig->GetNumRbPerRbg ()) / 8;
  if (tbs < 4)
    {
      NS_LOG_DEBUG ("While creating DCI for UE " << ueInfo->m_rnti <<
                    " assigned " << ueInfo->m_ulRBG << " UL RBG, but TBS < 4");
      return nullptr;
    }

  uint8_t numSym = static_cast<uint8_t> (ueInfo->m_ulRBG / m_phyMacConfig->GetBandwidthInRbg ());
  numSym = std::max (numSym, static_cast<uint8_t> (1));

  NS_ASSERT (spoint->m_sym >= numSym);

  // The starting point must go backward to accomodate the needed sym
  spoint->m_sym -= numSym;

  auto dci = CreateDci (spoint, ueInfo, tbs, DciInfoElementTdma::UL, ueInfo->m_ulMcs,
                        numSym);

  // Reset the RBG (we are TDMA)
  spoint->m_rbg = 0;

  return dci;
}

/**
 * \brief Create a DCI with the parameters specified as input
 * \param spoint starting point
 * \param ueInfo ue representation
 * \param tbs Transport Block Size
 * \param fmt Format of the DCI (UL or DL)
 * \param mcs MCS
 * \param numSym Number of symbols
 * \return a pointer to the newly created DCI
 *
 * Creates a TDMA DCI (a DCI with all the resource block assigned for the
 * specified number of symbols)
 */
std::shared_ptr<DciInfoElementTdma>
MmWaveMacSchedulerTdma::CreateDci (MmWaveMacSchedulerNs3::PointInFTPlane *spoint,
                                   const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo,
                                   uint32_t tbs, DciInfoElementTdma::DciFormat fmt,
                                   uint32_t mcs, uint8_t numSym) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (tbs > 0);
  NS_ASSERT (numSym > 0);
  std::vector<uint8_t> rbgAssigned (m_phyMacConfig->GetBandwidthInRbg (), 1);

  NS_LOG_INFO ("UE " << ueInfo->m_rnti << " assigned RBG from " <<
               static_cast<uint32_t> (spoint->m_rbg) << " to " <<
               m_phyMacConfig->GetBandwidthInRbg () + spoint->m_rbg <<
               " for " << static_cast<uint32_t> (numSym) << " SYM ");

  std::shared_ptr<DciInfoElementTdma> dci = std::make_shared<DciInfoElementTdma>
      (ueInfo->m_rnti, fmt, spoint->m_sym, numSym, mcs, tbs, 1, 0);

  dci->m_rbgBitmask = std::move (rbgAssigned);

  return dci;
}


} //namespace ns3
