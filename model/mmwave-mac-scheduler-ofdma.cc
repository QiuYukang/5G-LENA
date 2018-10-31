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
#include "mmwave-mac-scheduler-ofdma.h"
#include <ns3/log.h>
#include <algorithm>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("MmWaveMacSchedulerOfdma");
NS_OBJECT_ENSURE_REGISTERED (MmWaveMacSchedulerOfdma);

TypeId
MmWaveMacSchedulerOfdma::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacSchedulerOfdma")
    .SetParent<MmWaveMacSchedulerTdma> ()
  ;
  return tid;
}

MmWaveMacSchedulerOfdma::MmWaveMacSchedulerOfdma () : MmWaveMacSchedulerTdma ()
{
}

/**
 *
 * \brief Calculate the number of symbols to assign to each beam
 * \param symAvail Number of available symbols
 * \param activeDl Map of active DL UE and their beam
 *
 * Each beam has a different requirement in terms of byte that should be
 * transmitted with that beam. That requirement depends on the number of UE
 * that are inside such beam, and how many bytes they have to transmit.
 *
 * For the beam \f$ b \f$, the number of assigned symbols is the following:
 *
 * \f$ sym_{b} = BufSize(b) * \frac{symAvail}{BufSizeTotal} \f$
 */
MmWaveMacSchedulerOfdma::BeamSymbolMap
MmWaveMacSchedulerOfdma::GetSymPerBeam (uint32_t symAvail,
                                        const MmWaveMacSchedulerNs3::ActiveUeMap &activeDl) const
{
  NS_LOG_FUNCTION (this);

  GetSecond GetUeVector;
  GetSecond GetUeBufSize;
  GetFirst GetBeamId;
  double bufTotal = 0.0;
  uint8_t symUsed = 0;
  BeamSymbolMap ret;

  // Compute buf total
  for (const auto &el : activeDl)
    {
      for (const auto & ue : GetUeVector (el))
        {
          bufTotal += GetUeBufSize (ue);
        }
    }

  for (const auto &el : activeDl)
    {
      uint32_t bufSizeBeam = 0;
      for (const auto &ue : GetUeVector (el))
        {
          bufSizeBeam += GetUeBufSize (ue);
        }

      double tmp = symAvail / bufTotal;
      uint32_t symForBeam = static_cast<uint32_t> (bufSizeBeam * tmp);
      symUsed += symForBeam;
      ret.emplace (std::make_pair (GetBeamId (el), symForBeam));
      NS_LOG_DEBUG ("Assigned to beam " << GetBeamId (el) << " symbols " << symForBeam);
    }

  NS_ASSERT (symAvail >= symUsed);
  if (symAvail - symUsed > 0)
    {
      uint8_t symToRedistribute = symAvail - symUsed;
      while (symToRedistribute > 0)
        {
          BeamSymbolMap::iterator min = ret.end ();
          for (auto it = ret.begin (); it != ret.end (); ++it)
            {
              if (min == ret.end () || it->second < min->second)
                {
                  min = it;
                }
            }
          min->second += 1;
          symToRedistribute--;
          NS_LOG_DEBUG ("Assigned to beam " << min->first <<
                        " an additional symbol, for a total of " << min->second);
        }
    }

  return ret;
}

/**
 * \brief Assign the available DL RBG to the UEs
 * \param symAvail Available symbols
 * \param activeDl Map of active UE and their beams
 * \return a map between beams and the symbol they need
 *
 * The algorithm redistributes the frequencies to all the UEs inside a beam.
 * The pre-requisite is to calculate the symbols for each beam, done with
 * the function GetSymPerBeam().
 * The pseudocode is the following (please note that sym_of_beam is a value
 * returned by the GetSymPerBeam() function):
 * <pre>
 * while frequencies > 0:
 *    sort (ueVector);
 *    ueVector.first().m_dlRBG += 1 * sym_of_beam;
 *    frequencies--;
 *    UpdateUeDlMetric (ueVector.first());
 * </pre>
 *
 * To sort the UEs, the method uses the function returned by GetUeCompareDlFn().
 * Two fairness helper are hard-coded in the method: the first one is avoid
 * to assign resources to UEs that already have their buffer requirement covered,
 * and the other one is avoid to assign symbols when all the UEs have their
 * requirements covered.
 */
MmWaveMacSchedulerNs3::BeamSymbolMap
MmWaveMacSchedulerOfdma::AssignDLRBG (uint32_t symAvail, const ActiveUeMap &activeDl) const
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("# beams active flows: " << activeDl.size () << ", # sym: " << symAvail);

  GetFirst GetBeamId;
  GetSecond GetUeVector;
  BeamSymbolMap symPerBeam = GetSymPerBeam (symAvail, activeDl);

  // Iterate through the different beams
  for (const auto &el : activeDl)
    {
      // Distribute the RBG evenly among UEs of the same beam
      uint32_t resources = m_phyMacConfig->GetBandwidthInRbg ();
      uint32_t beamSym = symPerBeam.at (GetBeamId (el));
      uint32_t rbgAssignable = beamSym;
      std::vector<UePtrAndBufferReq> ueVector;
      FTResources assigned (0,0);
      for (const auto &ue : GetUeVector (el))
        {
          ueVector.emplace_back (ue);
        }

      for (auto & ue : ueVector)
        {
          BeforeDlSched (ue, FTResources (rbgAssignable * beamSym, beamSym));
        }

      while (resources > 0)
        {
          GetFirst GetUe;
          std::sort (ueVector.begin (), ueVector.end (), GetUeCompareDlFn ());
          auto schedInfoIt = ueVector.begin ();
          uint32_t bufQueueSize = schedInfoIt->second;

          // Ensure fairness: pass over UEs which already has enough resources to transmit
          while (schedInfoIt != ueVector.end ())
            {
              if (GetUe (*schedInfoIt)->m_dlTbSize >= bufQueueSize)
                {
                  schedInfoIt++;
                }
              else
                {
                  break;
                }
            }

          // In the case that all the UE already have their requirements fullfilled,
          // then stop the beam processing and pass to the next
          if (schedInfoIt == ueVector.end ())
            {
              break;
            }

          // Assign 1 RBG for each available symbols for the beam,
          // and then update the count of available resources
          GetUe (*schedInfoIt)->m_dlRBG += rbgAssignable;
          assigned.m_rbg += rbgAssignable;

          GetUe (*schedInfoIt)->m_dlSym = beamSym;
          assigned.m_sym = beamSym;

          resources -= 1; // Resources are RBG, so they do not consider the beamSym

          // Update metrics
          NS_LOG_DEBUG ("Assigned " << rbgAssignable <<
                        " DL RBG, spanned over " << beamSym << " SYM, to UE " <<
                        GetUe (*schedInfoIt)->m_rnti);
          AssignedDlResources (*schedInfoIt, FTResources (rbgAssignable, beamSym),
                               assigned);

          // Update metrics for the unsuccessfull UEs (who did not get any resource in this iteration)
          for (auto & ue : ueVector)
            {
              if (GetUe (ue)->m_rnti != GetUe (*schedInfoIt)->m_rnti)
                {
                  NotAssignedDlResources (ue, FTResources (m_phyMacConfig->GetBandwidthInRbg (), 1),
                                          assigned);
                }
            }
        }
    }

  return symPerBeam;
}

/**
 * \brief Create the DL DCI in OFDMA mode
 * \param spoint Starting point
 * \param ueInfo UE representation
 * \param maxSym Maximum symbols to use
 * \return a pointer to the newly created instance
 *
 * The function calculates the TBS and then call CreateDci().
 */
std::shared_ptr<DciInfoElementTdma>
MmWaveMacSchedulerOfdma::CreateDlDci (MmWaveMacSchedulerNs3::PointInFTPlane *spoint,
                                      const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo,
                                      uint32_t maxSym) const
{
  NS_LOG_FUNCTION (this);

  uint32_t tbs = m_amc->GetTbSizeFromMcsSymbols (ueInfo->m_dlMcs,
                                                 ueInfo->m_dlRBG * m_phyMacConfig->GetNumRbPerRbg ()) / 8;
  NS_ASSERT_MSG (ueInfo->m_dlRBG % maxSym == 0, " MaxSym " << maxSym << " RBG: " << ueInfo->m_dlRBG);
  NS_ASSERT (ueInfo->m_dlRBG <= maxSym * m_phyMacConfig->GetBandwidthInRbg ());
  NS_ABORT_IF (maxSym > UINT8_MAX);

  // If is less than 4, then we can't transmit any new data, so don't create dci.
  if (tbs <= 4)
    {
      NS_LOG_DEBUG ("While creating DCI for UE " << ueInfo->m_rnti <<
                    " assigned " << ueInfo->m_dlRBG << " DL RBG, but TBS < 4");
      return nullptr;
    }

  return CreateDci (spoint, ueInfo, tbs, DciInfoElementTdma::DL, ueInfo->m_dlMcs,
                    static_cast<uint8_t> (maxSym));
}

/**
 * \brief Create an OFDMA DCI
 * \param spoint Starting point
 * \param ueInfo UE representation
 * \param tbs TBS
 * \param fmt Format (DL or UL)
 * \param mcs MCS
 * \param numSym maximum number of symbol to use
 * \return a pointer to the newly created instance of the DCI
 *
 * Create a DCI suitable for OFDMA. The DCI is created on top of the available
 * Resource Blocks, starting from spoint, for an amount that is given by the
 * formula
 *
 * \f$ RB = RbPerRbg * (ueInfo->m_dlRBG / numSym) \f$
 *
 * So, the available RB will be from spoint->m_rb to spoint->m_rb + RB calculated
 * by the above formula.
 */
std::shared_ptr<DciInfoElementTdma>
MmWaveMacSchedulerOfdma::CreateDci (MmWaveMacSchedulerNs3::PointInFTPlane *spoint,
                                    const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo,
                                    uint32_t tbs, DciInfoElementTdma::DciFormat fmt,
                                    uint32_t mcs, uint8_t numSym) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (tbs > 0);
  NS_ASSERT (numSym > 0);
  NS_ASSERT (ueInfo->m_dlRBG % numSym == 0);

  uint32_t RBGNum = ueInfo->m_dlRBG / numSym;
  std::vector<uint8_t> rbgBitmask;

  for (uint32_t i = 0; i < m_phyMacConfig->GetBandwidthInRbg (); ++i)
    {
      if (i >= spoint->m_rbg && i < spoint->m_rbg + RBGNum)
        {
          rbgBitmask.push_back (1);
        }
      else
        {
          rbgBitmask.push_back (0);
        }
    }

  NS_LOG_INFO ("UE " << ueInfo->m_rnti << " assigned RBG from " <<
               static_cast<uint32_t> (spoint->m_rbg) << " to " <<
               static_cast<uint32_t> (spoint->m_rbg + RBGNum) << " for " <<
               static_cast<uint32_t> (numSym) << " SYM.");

  std::shared_ptr<DciInfoElementTdma> dci = std::make_shared<DciInfoElementTdma>
      (ueInfo->m_rnti, fmt, spoint->m_sym, numSym, mcs, tbs, 1, 0);

  dci->m_rbgBitmask = std::move (rbgBitmask);

  spoint->m_rbg += RBGNum;

  return dci;
}

} // namespace ns3
