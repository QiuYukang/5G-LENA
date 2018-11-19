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
 * Based on work done by CTTC/NYU
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
#include "mmwave-mac-scheduler-ns3.h"
#include "mmwave-mac-scheduler-harq-rr.h"

#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/log.h>
#include <ns3/eps-bearer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveMacSchedulerNs3");
NS_OBJECT_ENSURE_REGISTERED (MmWaveMacSchedulerNs3);

MmWaveMacSchedulerNs3::MmWaveMacSchedulerNs3 () : MmWaveMacScheduler ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

MmWaveMacSchedulerNs3::~MmWaveMacSchedulerNs3 ()
{
  m_ueMap.clear ();
}

TypeId
MmWaveMacSchedulerNs3::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacSchedulerNs3")
    .SetParent<MmWaveMacScheduler> ()
    .AddAttribute ("CqiTimerThreshold",
                   "The time while a CQI is valid",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&MmWaveMacSchedulerNs3::m_cqiTimersThreshold),
                   MakeTimeChecker ())
    .AddAttribute ("FixedMcsDl",
                   "Fix MCS to value set in McsDlDefault",
                   BooleanValue (false),
                   MakeBooleanAccessor (&MmWaveMacSchedulerNs3::m_fixedMcsDl),
                   MakeBooleanChecker ())
    .AddAttribute ("McsDefaultDl",
                   "Fixed DL MCS",
                   UintegerValue (1),
                   MakeUintegerAccessor (&MmWaveMacSchedulerNs3::m_mcsDefaultDl),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("FixedMcsUl",
                   "Fix MCS to value set in McsUlDefault (for testing)",
                   BooleanValue (false),
                   MakeBooleanAccessor (&MmWaveMacSchedulerNs3::m_fixedMcsUl),
                   MakeBooleanChecker ())
    .AddAttribute ("McsDefaultUl",
                   "Fixed UL MCS (for testing)",
                   UintegerValue (1),
                   MakeUintegerAccessor (&MmWaveMacSchedulerNs3::m_mcsDefaultUl),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("StartingMcsDl",
                   "Starting MCS for DL",
                   UintegerValue (0),
                   MakeUintegerAccessor (&MmWaveMacSchedulerNs3::m_startMcsDl),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("StartingMcsUl",
                   "Starting MCS for UL",
                   UintegerValue (0),
                   MakeUintegerAccessor (&MmWaveMacSchedulerNs3::m_startMcsUl),
                   MakeUintegerChecker<uint8_t> ())
  ;

  return tid;
}

/**
 * \brief Configure the common parameters, and create the instances of AMC and CQI management.
 * \param config Configuration options
 */
void
MmWaveMacSchedulerNs3::ConfigureCommonParameters (Ptr<MmWavePhyMacCommon> config)
{
  NS_LOG_FUNCTION (this << config);
  m_phyMacConfig = config;

  m_amc = CreateObject<MmWaveAmc> (config);
  m_cqiManagement.ConfigureCommonParameters (m_phyMacConfig, m_amc,
                                             m_startMcsDl, m_startMcsUl);

  NS_ABORT_IF (m_ulAllocationMap.size () > 0);
  SfnSf first (0, 0, 0, 0);

  for (uint16_t i = 0; i < m_phyMacConfig->GetL1L2CtrlLatency (); ++i)
    {
      first = first.IncreaseNoOfSlots (m_phyMacConfig->GetSlotsPerSubframe (),
                                       m_phyMacConfig->GetSubframesPerFrame ());
    }

  for (uint16_t i = 0; i < m_phyMacConfig->GetUlSchedDelay (); ++i)
    {
      NS_LOG_INFO ("Creating dummy UL allocation for slot " << first);
      m_ulAllocationMap.emplace (first.Encode (), SlotElem (0));
      first = first.IncreaseNoOfSlots (m_phyMacConfig->GetSlotsPerSubframe (),
                                       m_phyMacConfig->GetSubframesPerFrame ());
    }

  NS_LOG_DEBUG ("RB per RBG " << m_phyMacConfig->GetNumRbPerRbg () <<
                " total RBG " << m_phyMacConfig->GetBandwidthInRbg ());
  std::string tbs;
  for (uint32_t mcs = 0; mcs < 29; ++mcs)
    {
      std::stringstream ss;
      ss << "\nMCS " << mcs << " TBS 1 RBG " << m_amc->GetTbSizeFromMcsSymbols(mcs, m_phyMacConfig->GetNumRbPerRbg ()) << " 1 sym " << m_amc->GetTbSizeFromMcsSymbols(mcs, m_phyMacConfig->GetNumRbPerRbg() * m_phyMacConfig->GetBandwidthInRbg ());
      tbs += ss.str ();
    }
  NS_LOG_DEBUG (tbs);
}

/**
 * \brief Set a fixed MCS.
 * \param mcs The MCS.
 *
 * Set a fixed MCS for all UE that will be registered *AFTER* the call to this
 * function.
 */
void
MmWaveMacSchedulerNs3::DoSchedSetMcs (uint32_t mcs)
{
  NS_LOG_FUNCTION (this);
  m_fixedMcsDl = true;
  m_fixedMcsUl = true;
  m_mcsDefaultDl = static_cast<uint8_t> (mcs);
  m_mcsDefaultUl = static_cast<uint8_t> (mcs);
}

/**
 * \brief Cell configuration
 * \param params unused.
 *
 * Ignored. Always Success.
 */
void
MmWaveMacSchedulerNs3::DoCschedCellConfigReq (const MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (params);

  // IGNORE THE PARAMETERS.

  MmWaveMacCschedSapUser::CschedUeConfigCnfParameters cnf;
  cnf.m_result = SUCCESS;
  m_macCschedSapUser->CschedUeConfigCnf (cnf);
}

/**
 * \brief Register an UE
 * \param params params of the UE
 *
 * If the UE is not registered, then create its representation with a call to
 * CreateUeRepresentation, and then save its pointer in the m_ueMap map.
 *
 * If the UE is registered, update its corresponding beam.
 */
void
MmWaveMacSchedulerNs3::DoCschedUeConfigReq (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this <<
                   " RNTI " << params.m_rnti <<
                   " txMode " << static_cast<uint32_t> (params.m_transmissionMode));

  auto itUe = m_ueMap.find (params.m_rnti);
  GetSecond UeInfoOf;
  if (itUe == m_ueMap.end ())
    {
      NS_LOG_INFO ("Creating user, beam " << params.m_beamId << " and ue " << params.m_rnti);

      itUe = m_ueMap.insert (std::make_pair (params.m_rnti, CreateUeRepresentation (params))).first;

      UeInfoOf (*itUe)->m_dlHarq.SetMaxSize (static_cast<uint8_t> (m_phyMacConfig->GetNumHarqProcess ()));
      UeInfoOf (*itUe)->m_ulHarq.SetMaxSize (static_cast<uint8_t> (m_phyMacConfig->GetNumHarqProcess ()));
      UeInfoOf (*itUe)->m_dlMcs = m_startMcsDl;
      UeInfoOf (*itUe)->m_ulMcs = m_startMcsUl;
      if (m_fixedMcsDl)
        {
          UeInfoOf (*itUe)->m_dlMcs = m_mcsDefaultDl;
        }
      if (m_fixedMcsUl)
        {
          UeInfoOf (*itUe)->m_ulMcs = m_mcsDefaultUl;
        }
    }
  else
    {
      NS_LOG_LOGIC ("Updating Beam for UE " << params.m_rnti << " beam " << params.m_beamId);
      UeInfoOf (*itUe)->m_beamId = params.m_beamId;
    }
}

/**
 * \brief Release an UE
 * \param params params of the UE to release
 *
 * Remove the UE from the ueMap (m_ueMap).
 */
void
MmWaveMacSchedulerNs3::DoCschedUeReleaseReq (const MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this << " Release RNTI " << params.m_rnti);

  auto itUe = m_ueMap.find (params.m_rnti);
  NS_ABORT_IF (itUe == m_ueMap.end ());

  m_ueMap.erase (itUe);

  NS_LOG_INFO ("Release RNTI " << params.m_rnti);
}

/**
 * \brief Create a logical channel starting from a configuration
 * \param config configuration of the logical channel
 *
 * A subclass can return its own representation of a logical channel by
 * implementing a proper subclass of MmWaveMacSchedulerLC and returning a
 * pointer to a newly created instance.
 *
 * \return a pointer to the representation of a logical channel
 */
LCPtr
MmWaveMacSchedulerNs3::CreateLC (const LogicalChannelConfigListElement_s &config) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<MmWaveMacSchedulerLC> (new MmWaveMacSchedulerLC (config));
}

/**
 * \brief Create a logical channel group starting from a configuration
 * \param config configuration of the logical channel group
 *
 * A subclass can return its own representation of a logical channel by
 * implementing a proper subclass of MmWaveMacSchedulerLCG and returning a
 * pointer to a newly created instance.
 *
 * \return a pointer to the representation of a logical channel group
 */
LCGPtr
MmWaveMacSchedulerNs3::CreateLCG (const LogicalChannelConfigListElement_s &config) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<MmWaveMacSchedulerLCG> (new MmWaveMacSchedulerLCG (config.m_logicalChannelGroup));
}

/**
 * \brief Configure a logical channel for a UE
 * \param params the params of the LC
 *
 * The UE should be previously registered in the UE map. Then, for each logical
 * channel to configure, the UE representation is updated, creating an empty
 * LC. When the direction is set to DIR_BOTH, both UL and DL are created.
 *
 * Each LC is assigned to a LC group (LCG). If the group does not exists, then
 * it is created through the method CreateLCG, and then saved in the UE representation.
 * If the LCG exists or has been created, then the LC creation is done
 * through the method CreateLC and then saved in the UE representation.
 */
void
MmWaveMacSchedulerNs3::DoCschedLcConfigReq (const MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (params.m_rnti));
  auto itUe = m_ueMap.find (params.m_rnti);
  GetSecond UeInfoOf;
  NS_ABORT_IF (itUe == m_ueMap.end ());

  for (const auto & lcConfig : params.m_logicalChannelConfigList)
    {
      if (lcConfig.m_direction == LogicalChannelConfigListElement_s::DIR_DL
          || lcConfig.m_direction == LogicalChannelConfigListElement_s::DIR_BOTH)
        {
          auto itDl = UeInfoOf (*itUe)->m_dlLCG.find (lcConfig.m_logicalChannelGroup);
          auto itDlEnd = UeInfoOf (*itUe)->m_dlLCG.end ();
          if (itDl == itDlEnd)
            {
              NS_LOG_DEBUG ("Created DL LCG for UE " << UeInfoOf (*itUe)->m_rnti <<
                            " ID=" << static_cast<uint32_t> (lcConfig.m_logicalChannelGroup));
              std::unique_ptr<MmWaveMacSchedulerLCG> lcg = CreateLCG (lcConfig);
              itDl = UeInfoOf (*itUe)->m_dlLCG.emplace (std::make_pair (lcConfig.m_logicalChannelGroup, std::move (lcg))).first;
            }

          itDl->second->Insert (CreateLC (lcConfig));
          NS_LOG_DEBUG ("Created DL LC for UE " << UeInfoOf (*itUe)->m_rnti <<
                        " ID=" << static_cast<uint32_t> (lcConfig.m_logicalChannelIdentity) <<
                        " in LCG " << static_cast<uint32_t> (lcConfig.m_logicalChannelGroup));
        }
      if (lcConfig.m_direction == LogicalChannelConfigListElement_s::DIR_UL
          || lcConfig.m_direction == LogicalChannelConfigListElement_s::DIR_BOTH)
        {
          auto itUl = UeInfoOf (*itUe)->m_ulLCG.find (lcConfig.m_logicalChannelGroup);
          auto itUlEnd = UeInfoOf (*itUe)->m_ulLCG.end ();
          if (itUl == itUlEnd)
            {
              NS_LOG_DEBUG ("Created UL LCG for UE " << UeInfoOf (*itUe)->m_rnti <<
                            " ID=" << static_cast<uint32_t> (lcConfig.m_logicalChannelGroup));
              std::unique_ptr<MmWaveMacSchedulerLCG> lcg = CreateLCG (lcConfig);
              itUl = UeInfoOf (*itUe)->m_ulLCG.emplace (std::make_pair (lcConfig.m_logicalChannelGroup,
                                                                        std::move (lcg))).first;
            }

          // Create a LC ID only if it is the first. For detail, see documentation
          // of MmWaveMacSchedulerLCG.
          if (itUl->second->NumOfLC () == 0)
            {
              itUl->second->Insert (CreateLC (lcConfig));
              NS_LOG_DEBUG ("Created UL LC for UE " << UeInfoOf (*itUe)->m_rnti <<
                            " ID=" << static_cast<uint32_t> (lcConfig.m_logicalChannelIdentity) <<
                            " in LCG " << static_cast<uint32_t> (lcConfig.m_logicalChannelGroup));
            }
        }
    }
}

/**
 * \brief Release a LC
 * \param params params of the LC to release.
 *
 * Not implemented.
 */
void
MmWaveMacSchedulerNs3::DoCschedLcReleaseReq (const MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  for (const auto & lcId : params.m_logicalChannelIdentity)
    {
      auto itUe = m_ueMap.find (params.m_rnti);
      NS_ABORT_IF (itUe == m_ueMap.end ());

      // TODO !!!!

      NS_UNUSED (lcId);
      //UeInfoOf(itUe)->ReleaseLC (lcId);
    }
}

/**
 * \brief RLC informs of DL data
 * \param params parameters of the function
 *
 * The message contains the LC and the amount of data buffered. Therefore,
 * in this method we cycle through all the UE LCG to find the LC, and once
 * it is found, it is updated with the new amount of data.
 */
void
MmWaveMacSchedulerNs3::DoSchedDlRlcBufferReq (const MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this << params.m_rnti <<
                   static_cast<uint32_t> (params.m_logicalChannelIdentity));

  GetSecond UeInfoOf;
  auto itUe = m_ueMap.find (params.m_rnti);
  NS_ABORT_IF (itUe == m_ueMap.end ());

  for (const auto &lcg : UeInfoOf (*itUe)->m_dlLCG)
    {
      if (lcg.second->Contains (params.m_logicalChannelIdentity))
        {
          NS_LOG_INFO ("Updating DL LC Info: " << params <<
                       " in LCG: " << static_cast<uint32_t> (lcg.first));
          lcg.second->UpdateInfo (params);
          return;
        }
    }
  // Fail miserabily because we didn't found any LC
  NS_FATAL_ERROR ("The LC does not exist. Can't update");
}

/**
 * \brief Update the UL LC
 * \param bsr BSR received
 *
 * The UE notifies the buffer size as a sum of all the components. The BSR
 * is a vector of 4 uint8_t that represents the amount of data in each
 * LCG. A call to MmWaveMacSchedulerLCG::UpdateInfo is then issued with
 * the amount of data as parameter.
 */
void
MmWaveMacSchedulerNs3::BSRReceivedFromUe (const MacCeElement &bsr)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (bsr.m_macCeType == MacCeElement::BSR);
  GetSecond UeInfoOf;
  auto itUe = m_ueMap.find (bsr.m_rnti);
  NS_ABORT_IF (itUe == m_ueMap.end ());

  // The UE only notifies the buf size as sum of all components.
  // see mmwave-ue-mac.cc:395
  for (uint8_t lcg = 0; lcg < 4; ++lcg)
    {
      uint8_t bsrId = bsr.m_macCeValue.m_bufferStatus.at (lcg);
      uint32_t bufSize = BsrId2BufferSize (bsrId);

      auto itLcg = UeInfoOf (*itUe)->m_ulLCG.find (lcg);
      if (itLcg == UeInfoOf (*itUe)->m_ulLCG.end ())
        {
          NS_ABORT_MSG_IF (bufSize > 0, "LCG " << static_cast<uint32_t> (lcg) <<
                           " not found for UE " << itUe->second->m_rnti);
          continue;
        }

      if (itLcg->second->GetTotalSize () > 0 || bufSize > 0)
        {
          NS_LOG_INFO ("Updating UL LCG " << static_cast<uint32_t> (lcg) <<
                       " for UE " << bsr.m_rnti << " size " << bufSize);
        }

      itLcg->second->UpdateInfo (bufSize);
    }
}

/**
 * \brief Evaluate different types of control messages (only BSR for the moment)
 * \param params parameters of the control message
 *
 * For each BSR received, calls BSRReceivedFromUe. Ignore all the others control
 * messages.
 */
void
MmWaveMacSchedulerNs3::DoSchedUlMacCtrlInfoReq (const MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  for (const auto & element : params.m_macCeList)
    {
      if ( element.m_macCeType == MacCeElement::BSR )
        {
          BSRReceivedFromUe (element);
        }
      else
        {
          NS_LOG_INFO ("Ignoring received CTRL message because it's not a BSR");
        }
    }
}

/**
 * \brief Received a DL CQI message
 * \param params DL CQI message
 *
 * For each message in the list, calculate the expiration time in number of slots,
 * and then pass all the information to the MmWaveMacSchedulerCQIManagement class.
 *
 * If the CQI is sub-band, the method MmWaveMacSchedulerCQIManagement::SBCQIReported
 * will be called, otherwise MmWaveMacSchedulerCQIManagement::WBCQIReported.
 */
void
MmWaveMacSchedulerNs3::DoSchedDlCqiInfoReq (const MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  if (m_fixedMcsDl)
    {
      return;
    }

  NS_ASSERT (m_cqiTimersThreshold >= m_phyMacConfig->GetSlotPeriod ());

  uint32_t expirationTime = static_cast<uint32_t> (m_cqiTimersThreshold.GetNanoSeconds () /
                                                   m_phyMacConfig->GetSlotPeriod ().GetNanoSeconds ());

  for (const auto &cqi : params.m_cqiList)
    {
      NS_ASSERT (m_ueMap.find (cqi.m_rnti) != m_ueMap.end ());
      const std::shared_ptr<MmWaveMacSchedulerUeInfo> & ue = m_ueMap.find (cqi.m_rnti)->second;

      if (cqi.m_cqiType == DlCqiInfo::WB)
        {
          m_cqiManagement.DlWBCQIReported (cqi, ue, expirationTime);
        }
      else
        {
          m_cqiManagement.DlSBCQIReported (cqi, ue);
        }
    }
}

/**
 * \brief Received a UL CQI message
 * \param params UL CQI message
 *
 * Calculate the expiration time in number of slots, and then pass all the
 * information to the MmWaveMacSchedulerCQIManagement class.
 *
 * In UL, we have to know the previously allocated symbols and the total TBS
 * to be able to calculate CQI and MCS, so a special stack is maintained
 * (m_ulAllocationMap).
 *
 * Only UlCqiInfo::PUSCH is currently supported.
 */
void
MmWaveMacSchedulerNs3::DoSchedUlCqiInfoReq (const MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  if (m_fixedMcsUl)
    {
      return;
    }

  GetSecond UeInfoOf;

  uint32_t expirationTime = static_cast<uint32_t> (m_cqiTimersThreshold.GetNanoSeconds () /
                                                   m_phyMacConfig->GetSlotPeriod ().GetNanoSeconds ());

  switch (params.m_ulCqi.m_type)
    {
    case UlCqiInfo::PUSCH:
      {
        bool found = false;
        auto symStart = params.m_sfnSf.m_varTtiNum;
        auto ulSfnSf = params.m_sfnSf;
        ulSfnSf.m_varTtiNum = 0;

        NS_LOG_INFO ("CQI for allocation: " << params.m_sfnSf << " varTti: " <<
                     static_cast<uint32_t> (params.m_sfnSf.m_varTtiNum) <<
                     " modified allocation " << ulSfnSf <<
                     " sym Start " << static_cast<uint32_t> (symStart));

        auto itAlloc = m_ulAllocationMap.find (ulSfnSf.Encode ());
        NS_ASSERT_MSG (itAlloc != m_ulAllocationMap.end (),
                       "Can't find allocation for " << ulSfnSf);
        std::vector<AllocElem> & ulAllocations = itAlloc->second.m_ulAllocations;

        for (auto it = ulAllocations.cbegin (); it != ulAllocations.cend (); ++it)
          {
            const AllocElem & allocation = *(it);
            if (allocation.m_symStart == symStart)
              {
                auto itUe = m_ueMap.find (allocation.m_rnti);
                NS_ASSERT (allocation.m_rb == m_phyMacConfig->GetBandwidthInRbs ());
                NS_ASSERT (itUe != m_ueMap.end ());
                NS_ASSERT (allocation.m_numSym > 0);
                NS_ASSERT (allocation.m_tbs > 0);

                m_cqiManagement.UlSBCQIReported (expirationTime, allocation.m_numSym,
                                                 allocation.m_tbs, params, UeInfoOf (*itUe));
                found = true;
                ulAllocations.erase (it);
                break;
              }
          }
        NS_ASSERT (found);
        NS_UNUSED (found);

        if (ulAllocations.size () == 0)
          {
            // remove obsolete info on allocation; we already processed all the CQI
            NS_LOG_INFO ("Removing allocation for " << ulSfnSf);
            m_ulAllocationMap.erase (itAlloc);
          }
      }
      break;
    default:
      NS_FATAL_ERROR ("Unknown type of UL-CQI");
    }
}

/**
 * \brief Merge newly received HARQ feedbacks with existing feedbacks
 * \param existingFeedbacks a vector of old feedback (will be empty at the end)
 * \param inFeedbacks Received feedbacks
 * \param mode UL or DL, for debug printing
 * \return a vector of all the feedbacks (new + old)
 *
 * It is possible that, in one slot, some HARQ could not be transmitted (by
 * choice, or because there are not available resources). These feedbacks are
 * the 'old' ones, that should be merged with the newly arrived (the feedbacks
 * that arrived in the current slot) before processing.
 *
 */
template <typename T>
std::vector<T> MmWaveMacSchedulerNs3::MergeHARQ (std::vector<T> *existingFeedbacks,
                                                 const std::vector<T> &inFeedbacks,
                                                 const std::string &mode) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("To retransmit : " << existingFeedbacks->size () << " " << mode <<
               " HARQ, received " << inFeedbacks.size () << " " << mode <<
               " HARQ Feedback");
  uint64_t existingSize = existingFeedbacks->size ();
  uint64_t inSize = inFeedbacks.size ();
  existingFeedbacks->insert (existingFeedbacks->end (),
                             inFeedbacks.begin (),
                             inFeedbacks.end ());
  NS_ASSERT (existingFeedbacks->size () == existingSize + inSize);

  auto ret = std::vector<T> (std::make_move_iterator (existingFeedbacks->begin ()),
                             std::make_move_iterator (existingFeedbacks->end ()));
  existingFeedbacks->clear ();

  return ret;
}

/**
 * \brief Process HARQ feedbacks
 * \param harqInfo all the known HARQ feedbacks (can be UL or DL)
 * \param GetHarqVectorFn Function to retrieve the correct Harq Vector
 * \param direction "UL" or "DL" for debug messages
 *
 * For every received feedback (even the already processed ones) the method
 * checks if the feedback is ACK or NACK. In case of ACK (represented by
 * HarqInfo::IsReceivedOk) the feedback is eliminated and the corresponding
 * HARQ process erased; if the feedback is NACK, the corresponding process
 * is marked for retransmission. The decision to retransmit or not the process
 * will be taken later.
 *
 * \see DlHarqInfo
 * \see UlHarqInfo
 * \see HarqProcess
 */
template<typename T>
void
MmWaveMacSchedulerNs3::ProcessHARQFeedbacks (std::vector<T> *harqInfo,
                                             const MmWaveMacSchedulerUeInfo::GetHarqVectorFn &GetHarqVectorFn,
                                             const std::string &direction) const
{
  NS_LOG_FUNCTION (this);
  uint32_t nackReceived = 0;

  // Check the HARQ feedback, erase ACKed, updated NACKed
  for (auto harqFeedbackIt = harqInfo->begin (); harqFeedbackIt != harqInfo->end (); /* nothing as increment */)
    {
      uint8_t harqId = harqFeedbackIt->m_harqProcessId;
      uint16_t rnti = harqFeedbackIt->m_rnti;
      MmWaveMacHarqVector & ueHarqVector = GetHarqVectorFn (m_ueMap.find (rnti)->second);
      HarqProcess & ueProcess = ueHarqVector.Get (harqId);

      NS_LOG_INFO ("Evaluating feedback: " << *harqFeedbackIt);
      if (ueProcess.m_active == false)
        {
          NS_LOG_INFO ("UE " << rnti << " HARQ vector: " << ueHarqVector);
          NS_FATAL_ERROR ("Received feedback for a process which is not active");
        }
      NS_ABORT_IF (ueProcess.m_dciElement == nullptr);

      if (harqFeedbackIt->IsReceivedOk () || ueProcess.m_dciElement->m_rv == 3)
        {
          ueHarqVector.Erase (harqId);
          harqFeedbackIt = harqInfo->erase (harqFeedbackIt);
          NS_LOG_INFO ("Erased processID " << static_cast<uint32_t> (harqId) <<
                       " of UE " << rnti << " direction " << direction);
        }
      else if (!harqFeedbackIt->IsReceivedOk ())
        {
          ueProcess.m_status = HarqProcess::RECEIVED_FEEDBACK;
          nackReceived++;
          ++harqFeedbackIt;
          NS_LOG_INFO ("NACK received for UE " << static_cast<uint32_t> (rnti) <<
                       " process " << static_cast<uint32_t> (harqId) <<
                       " direction " << direction);
        }
    }

  NS_ASSERT (harqInfo->size () == nackReceived);
}

/**
 * \brief Reset expired HARQ
 * \param rnti RNTI of the user
 * \param harq HARQ process list
 *
 * For each process, check its timer. If it is expired, reset the
 * process.
 *
 * \see MmWaveMacHarqVector
 * \see HarqProcess
 */
void
MmWaveMacSchedulerNs3::ResetExpiredHARQ (uint16_t rnti, MmWaveMacHarqVector *harq)
{
  NS_LOG_FUNCTION (this << harq);

  for (auto harqIt = harq->Begin (); harqIt != harq->End (); ++harqIt)
    {
      HarqProcess & process = harqIt->second;
      uint8_t processId = harqIt->first;

      if (process.m_status == HarqProcess::INACTIVE)
        {
          continue;
        }

      if (process.m_timer < m_phyMacConfig->GetHarqTimeout ())
        {
          ++process.m_timer;
          NS_LOG_INFO ("Updated process for UE " << rnti << " number " <<
                       static_cast<uint32_t> (processId) <<
                       ", resulting process: " << process);
        }
      else
        {
          harq->Erase (processId);
          NS_LOG_INFO ("Erased process for UE " << rnti << " number " <<
                       static_cast<uint32_t> (processId) << " for time limits");
        }
    }
}

/**
 * \brief Prepend a CTRL symbol to the allocation list
 * \param symStart starting symbol
 * \param numSymToAllocate number of symbols to allocate (each CTRL take 1 symbol)
 * \param mode Mode of the allocation (UL, DL)
 * \param allocations list of allocations to which prepend the CTRL symbol
 * \return the symbol that can be used to append other things into the allocation list
 */
uint8_t
MmWaveMacSchedulerNs3::PrependCtrlSym (uint8_t symStart, uint8_t numSymToAllocate,
                                       VarTtiAllocInfo::TddMode mode,
                                       std::deque<VarTtiAllocInfo> *allocations) const
{
  std::vector<uint8_t> rbgBitmask (m_phyMacConfig->GetBandwidthInRbg (), 1);

  NS_ASSERT_MSG (rbgBitmask.size () == m_phyMacConfig->GetBandwidthInRbg (),
                 "bitmask size " << rbgBitmask.size () << " conf " <<
                 m_phyMacConfig->GetBandwidthInRbg ());
  if (mode == VarTtiAllocInfo::DL)
    {
      NS_ASSERT (allocations->size () == 0); // no previous allocations
      NS_ASSERT (symStart == 0); // start from the symbol 0
    }

  for (uint8_t sym = symStart; sym < symStart + numSymToAllocate; ++sym)
    {
      allocations->emplace_front (VarTtiAllocInfo (mode, VarTtiAllocInfo::CTRL,
                                                   std::make_shared<DciInfoElementTdma> (sym, 1, rbgBitmask)));
      NS_LOG_INFO ("Allocating CTRL symbol, type" << mode <<
                   " in TDMA. numSym=1, symStart=" <<
                   static_cast<uint32_t> (sym) <<
                   " Remaining CTRL sym to allocate: " << sym - symStart);
    }
  return symStart + numSymToAllocate;
}

/**
 * \brief Append a CTRL symbol to the allocation list
 * \param symStart starting symbol
 * \param numSymToAllocate number of symbols to allocate (each CTRL take 1 symbol)
 * \param mode Mode of the allocation (UL, DL)
 * \param allocations list of allocations to which append the CTRL symbol
 * \return the VarTtiAllocInfo ID that can be used to append other things into the allocation list
 */
uint8_t
MmWaveMacSchedulerNs3::AppendCtrlSym (uint8_t symStart, uint8_t numSymToAllocate,
                                      VarTtiAllocInfo::TddMode mode,
                                      std::deque<VarTtiAllocInfo> *allocations) const
{
  std::vector<uint8_t> rbgBitmask (m_phyMacConfig->GetBandwidthInRbg (), 1);

  NS_ASSERT (rbgBitmask.size () == m_phyMacConfig->GetBandwidthInRbg ());
  if (mode == VarTtiAllocInfo::DL)
    {
      NS_ASSERT (allocations->size () == 0); // no previous allocations
      NS_ASSERT (symStart == 0); // start from the symbol 0
    }

  for (uint8_t sym = symStart; sym < symStart + numSymToAllocate; ++sym)
    {
      allocations->emplace_back (VarTtiAllocInfo (mode, VarTtiAllocInfo::CTRL,
                                                  std::make_shared<DciInfoElementTdma> (sym, 1, rbgBitmask)));
      NS_LOG_INFO ("Allocating CTRL symbol, type" << mode <<
                   " in TDMA. numSym=1, symStart=" <<
                   static_cast<uint32_t> (sym) <<
                   " Remaining CTRL sym to allocate: " << sym - symStart);
    }
  return symStart + numSymToAllocate;
}

/**
 * \brief Compute the number of active DL HARQ to perform
 *
 * \param activeDlHarq list of DL HARQ to perform (should be empty at the beginning)
 * \param dlHarqFeedback list of DL HARQ feedback received
 *
 * After calculating the active HARQ, they should be sorted. It is done by
 * subclasses in the method SortDlHarq.
 * \see SortDlHarq
 */
void
MmWaveMacSchedulerNs3::ComputeActiveHarq (ActiveHarqMap *activeDlHarq,
                                          const std::vector<DlHarqInfo> &dlHarqFeedback) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (activeDlHarq->size () == 0);

  for (const auto &feedback : dlHarqFeedback)
    {
      uint16_t rnti = feedback.m_rnti;
      auto &schedInfo = m_ueMap.find (rnti)->second;
      auto beamIterator = activeDlHarq->find (schedInfo->m_beamId);

      if (beamIterator == activeDlHarq->end ())
        {
          std::vector<MmWaveMacHarqVector::iterator> harqVector;
          NS_ASSERT (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId)->second.m_active);

          harqVector.emplace_back (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId));
          activeDlHarq->emplace (std::make_pair (schedInfo->m_beamId, harqVector));
        }
      else
        {
          NS_ASSERT (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId)->second.m_active);
          beamIterator->second.emplace_back (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId));
        }
      NS_LOG_INFO ("Received feedback for UE " << rnti << " ID " <<
                   static_cast<uint32_t>(feedback.m_harqProcessId) <<
                   " marked as active");
      NS_ASSERT (schedInfo->m_dlHarq.Find (feedback.m_harqProcessId)->second.m_status == HarqProcess::RECEIVED_FEEDBACK);
    }

  SortDlHarq (activeDlHarq);
}

/**
 * \brief Compute the number of activeUL HARQ to perform
 *
 * \param activeDlHarq list of UL HARQ to perform
 * \param ulHarqFeedback list of UL HARQ feedback
 *
 * After calculating the active HARQ, they should be sorted. It is done by
 * subclasses in the method SortUlHarq.
 * \see SortUlHarq
 */
void
MmWaveMacSchedulerNs3::ComputeActiveHarq (ActiveHarqMap *activeUlHarq,
                                          const std::vector<UlHarqInfo> &ulHarqFeedback) const
{
  NS_LOG_FUNCTION (this);

  for (const auto &feedback : ulHarqFeedback)
    {
      uint16_t rnti = feedback.m_rnti;
      auto &schedInfo = m_ueMap.find (rnti)->second;
      auto beamIterator = activeUlHarq->find (schedInfo->m_beamId);

      if (beamIterator == activeUlHarq->end ())
        {
          std::vector<MmWaveMacHarqVector::iterator> harqVector;
          NS_ASSERT (schedInfo->m_ulHarq.Find (feedback.m_harqProcessId)->second.m_active);
          harqVector.emplace_back (schedInfo->m_ulHarq.Find (feedback.m_harqProcessId));
          activeUlHarq->emplace (std::make_pair (schedInfo->m_beamId, harqVector));
        }
      else
        {
          NS_ASSERT (schedInfo->m_ulHarq.Find (feedback.m_harqProcessId)->second.m_active);
          beamIterator->second.emplace_back (schedInfo->m_ulHarq.Find (feedback.m_harqProcessId));
        }
    }
  SortUlHarq (activeUlHarq);
}

/**
 * \brief Compute the number of active DL and UL UE
 * \param activeDlUe map of active DL UE to be filled
 * \param GetLCGFn Function to retrieve the LCG of a UE
 * \param mode UL or DL (to be printed in debug messages)
 *
 * The function loops all available UEs and checks their LC. If one (or more)
 * LC contains bytes, they are marked active and inserted in one of the
 * list passed as input parameters. The UE is not marked as active if
 * there is already an allocation for him in the list of allocations.
 */
void
MmWaveMacSchedulerNs3::ComputeActiveUe (ActiveUeMap *activeUe,
                                        SlotAllocInfo const *alloc,
                                        const MmWaveMacSchedulerUeInfo::GetLCGFn &GetLCGFn,
                                        const std::string &mode) const
{
  NS_LOG_FUNCTION (this);
  for (const auto &ueInfo : m_ueMap)
    {
      uint32_t totBuffer = 0;
      const auto & ue = ueInfo.second;
      bool ueAlreadyScheduled = false;

      for (const auto &allocation : alloc->m_varTtiAllocInfo)
        {
          if (allocation.m_dci->m_rnti == ue->m_rnti)
            {
              ueAlreadyScheduled = true;
              break;
            }
        }
      if (ueAlreadyScheduled)
        {
          // Do not schdule two times the same UE
          continue;
        }

      // compute total DL and UL bytes buffered
      for (const auto & lcgInfo : GetLCGFn (ue))
        {
          const auto & lcg = lcgInfo.second;
          if (lcg->GetTotalSize () > 0)
            {
              NS_LOG_INFO ("UE " << ue->m_rnti << " " << mode << " LCG " <<
                           static_cast<uint32_t> (lcgInfo.first) <<
                           " bytes " << lcg->GetTotalSize ());
            }
          totBuffer += lcg->GetTotalSize ();
        }

      if (totBuffer > 0)
        {
          auto it = activeUe->find (ue->m_beamId);
          if (it == activeUe->end ())
            {
              std::vector<std::pair<std::shared_ptr<MmWaveMacSchedulerUeInfo>, uint32_t> > tmp;
              tmp.emplace_back (ue, totBuffer);
              activeUe->insert (std::make_pair (ue->m_beamId, tmp));
            }
          else
            {
              it->second.emplace_back (ue, totBuffer);
            }
        }
    }
}

/**
 * \brief Method to decide how to distribute the assigned bytes to the different LCs
 * \param ueLCG LCG of an UE
 * \param tbs TBS to divide between the LCG/LC
 * \return A vector of Assignation
 *
 * The method distribute bytes evenly between LCG. This is a default;
 * more advanced methods can be inserted. Please note that the correct way
 * is to move this implementation in a class, and then implementing
 * a different method in a different class. Then, the selection between
 * the implementation should be done with an Attribute.
 *
 * Please don't try to insert if/switch statements here, NOR to make it virtual
 * and to change in the subclasses.
 */
// Assume LC are unique
std::vector<MmWaveMacSchedulerNs3::Assignation>
MmWaveMacSchedulerNs3::AssignBytesToLC (const std::unordered_map<uint8_t, LCGPtr> &ueLCG,
                                        uint32_t tbs) const
{
  NS_LOG_FUNCTION (this);
  GetFirst GetLCGID;
  GetSecond GetLCG;

  std::vector<Assignation> ret;

  NS_LOG_INFO ("To distribute: " << tbs << " bytes");
  for (const auto & lcg : ueLCG)
    {
      uint32_t lcgTotalSize = GetLCG (lcg)->GetTotalSize ();
      if (lcgTotalSize > 0)
        {
          std::vector<uint8_t> lcs = GetLCG (lcg)->GetLCId ();
          for (const auto & lcId : lcs)
            {
              if (GetLCG (lcg)->GetTotalSizeOfLC (lcId) > 0 && lcgTotalSize > 0)
                {
                  uint32_t amount = std::min (tbs, lcgTotalSize);
                  amount = std::min (amount, GetLCG (lcg)->GetTotalSizeOfLC (lcId));

                  tbs -= amount;
                  lcgTotalSize -= amount;

                  NS_LOG_INFO ("Assigned to LCID " << static_cast<uint32_t> (lcId) <<
                               " inside LCG " << static_cast<uint32_t> (GetLCGID (lcg)) <<
                               " an amount of " << amount << " B, remaining in the LCG " <<
                               lcgTotalSize);
                  ret.emplace_back (Assignation (GetLCGID (lcg), lcId, amount));

                  if (tbs == 0 || lcgTotalSize == 0)
                    {
                      break;
                    }
                }
            }
        }
    }

  return ret;
}

/**
 * \brief Scheduling new DL data
 * \param spoint Starting point of the blocks to add to the allocation list
 * \param symAvail Number of available symbols
 * \param activeDl List of active UE with data to transmit in DL
 * \param slotAlloc The allocation info to which append the allocations
 * \return The number of symbols used in the allocation
 *
 * The method is doing the scheduling of new data in the DL direction, delegating
 * three phases to subclasses:
 *
 * - How distribute the symbols between beams?
 * - How many RBG should be assigned to the each active UE?
 * - How to place the blocks in the 2D plan (in other words, how to create the DCIs)?
 *
 * The first two phases are managed by the function AssignDLRBG. Once the map
 * between the beamId and the symbols assigned to it has been returned, the
 * active user list has been updated by assigning to each user an amount of
 * RBG. Then, it is necessary to iterate through the beams, and for each beam,
 * iterating through the users of that beam, creating a DCI (function CreateDlDci).
 * Creating the DCI is the third phase in the previous list, because the DCI
 * specifies where the imaginary block containing data is placed.
 *
 * After the DCI has been created, it is necessary to prepare the HarqProcess,
 * store it in the UE pointer, and distribute the TBS among the active LC of the
 * UE, through the method AssignBytesToLC, and creating the corresponding list
 * of RlcPduInfo.
 *
 * Before looping and changing the beam, the starting point should be advanced.
 * How that is done is a matter for the subclasses (method ChangeDlBeam).
 */
uint8_t
MmWaveMacSchedulerNs3::DoScheduleDlData (PointInFTPlane *spoint, uint32_t symAvail,
                                         const ActiveUeMap &activeDl,
                                         SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this << symAvail);
  NS_ASSERT (spoint->m_rbg == 0);
  BeamSymbolMap symPerBeam = AssignDLRBG (symAvail, activeDl);
  GetFirst GetBeam;
  uint8_t usedSym = 0;
  for (const auto &beam : activeDl)
    {
      uint32_t availableRBG = (m_phyMacConfig->GetBandwidthInRbg () - spoint->m_rbg) * symPerBeam.at (GetBeam (beam));
      bool assigned = false;

      NS_LOG_DEBUG (activeDl.size () << " active DL beam, this beam has " <<
                    symPerBeam.at (GetBeam (beam)) << " SYM, starts from RB " << static_cast<uint32_t> (spoint->m_rbg) <<
                    " and symbol " << static_cast<uint32_t> (spoint->m_sym) << " for a total of " <<
                    availableRBG << " RBG. In one symbol we have " << m_phyMacConfig->GetBandwidthInRbg () <<
                    " RBG.");

      if (symPerBeam.at (GetBeam (beam)) == 0)
        {
          NS_LOG_INFO ("No available symbols for this beam, continue");
          continue;
        }

      for (const auto &ue : beam.second)
        {
          if (ue.first->m_dlRBG == 0)
            {
              NS_LOG_INFO ("UE " << ue.first->m_rnti << " does not have RBG assigned");
              continue;
            }

          std::shared_ptr<DciInfoElementTdma> dci = CreateDlDci (spoint, ue.first,
                                                                 symPerBeam.at (GetBeam (beam)));
          if (dci == nullptr)
            {
              NS_LOG_DEBUG ("No DCI has been created, ignoring");
              ue.first->ResetDlMetric ();
              continue;
            }

          assigned = true;
          NS_LOG_INFO ("UE " << ue.first->m_rnti << " has " << ue.first->m_dlRBG <<
                       " RBG assigned");
          NS_ASSERT_MSG (dci->m_symStart + dci->m_numSym < m_phyMacConfig->GetSymbolsPerSlot (),
                         "symStart: " << static_cast<uint32_t> (dci->m_symStart) << " symEnd: " <<
                         static_cast<uint32_t> (dci->m_numSym) << " symbols: " <<
                         static_cast<uint32_t> (m_phyMacConfig->GetSymbolsPerSlot ()));

          HarqProcess harqProcess (true, HarqProcess::WAITING_FEEDBACK, 0, dci);
          uint8_t id;

          if (!ue.first->m_dlHarq.CanInsert ())
            {
              NS_LOG_INFO ("Harq Vector condition for UE " << ue.first->m_rnti <<
                           std::endl << ue.first->m_dlHarq);
              NS_FATAL_ERROR ("UE " << ue.first->m_rnti << " does not have DL HARQ space");
            }

          ue.first->m_dlHarq.Insert (&id, harqProcess);
          ue.first->m_dlHarq.Get (id).m_dciElement->m_harqProcess = id;

          auto distributedBytes = AssignBytesToLC (ue.first->m_dlLCG, dci->m_tbSize);

          VarTtiAllocInfo slotInfo (VarTtiAllocInfo::DL, VarTtiAllocInfo::DATA, dci);

          NS_LOG_INFO ("Assigned process ID " << static_cast<uint32_t> (dci->m_harqProcess) <<
                       " to UE " << ue.first->m_rnti);
          NS_LOG_DEBUG (" UE" << dci->m_rnti <<
                        " gets DL symbols " << static_cast<uint32_t> (dci->m_symStart) <<
                        "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym) <<
                        " tbs " << dci->m_tbSize <<
                        " mcs " << static_cast<uint32_t> (dci->m_mcs) <<
                        " harqId " << static_cast<uint32_t> (id) <<
                        " rv " << static_cast<uint32_t> (dci->m_rv)
                        );

          for (const auto & byteDistribution : distributedBytes)
            {
              uint8_t lcId = byteDistribution.m_lcId;
              uint8_t lcgId = byteDistribution.m_lcg;
              uint32_t bytes = byteDistribution.m_bytes;

              RlcPduInfo newRlcPdu (lcId, bytes);
              HarqProcess & process = ue.first->m_dlHarq.Get (dci->m_harqProcess);

              slotInfo.m_rlcPduInfo.push_back (newRlcPdu);
              process.m_rlcPduInfo.push_back (newRlcPdu);

              ue.first->m_dlLCG.at (lcgId)->AssignedData (lcId, bytes);

              NS_LOG_DEBUG ("DL LCG " << static_cast<uint32_t> (lcgId) <<
                            " LCID " << static_cast<uint32_t> (lcId) <<
                            " got bytes " << newRlcPdu.m_size);
            }

          NS_ABORT_IF (slotInfo.m_rlcPduInfo.size () == 0);

          slotAlloc->m_varTtiAllocInfo.emplace_back (slotInfo);
        }
      if (assigned)
        {
          ChangeDlBeam (spoint, symPerBeam.at (GetBeam (beam)));
          usedSym += symPerBeam.at (GetBeam (beam));
          slotAlloc->m_numSymAlloc += symPerBeam.at (GetBeam (beam));
        }
    }

  for (auto & beam : activeDl)
    {
      for (auto & ue : beam.second)
        {
          ue.first->ResetDlSchedInfo ();
        }
    }

  NS_ASSERT (spoint->m_rbg == 0);

  return usedSym;
}

/**
 * \brief Scheduling new UL data
 * \param spoint Starting point of the blocks to add to the allocation list
 * \param symAvail Number of available symbols
 * \param activeDl List of active UE with data to transmit in UL
 * \param slotAlloc The allocation info to which append the allocations
 * \return The number of symbols used in the allocation
 *
 * The method is doing the scheduling of new data in the UL direction. Before
 * doing that, it is necessary to schedule the UEs that requested a SR.
 * Then, to decide how to schedule the data, it delegates three phases to subclasses:
 *
 * - How distribute the symbols between beams?
 * - How many RBG should be assigned to the each active UE?
 * - How to place the blocks in the 2D plan (in other words, how to create the DCIs)?
 *
 * The first two phases are managed by the function AssignULRBG. Once the map
 * between the beamId and the symbols assigned to it has been returned, the
 * active user list has been updated by assigning to each user an amount of
 * RBG. Then, it is necessary to iterate through the beams, and for each beam,
 * iterating through the users of that beam, creating a DCI (function CreateUlDci).
 * Creating the DCI is the third phase in the previous list, because the DCI
 * specifies where the imaginary block containing data is placed. The DCI should
 * be created by placing the blocks from the last available symbol (e.g., 13)
 * going backwards. This restriction comes from the need of keeping the order
 * of DCI: DL CTRL, DL Data, UL Data, UL CTRL.
 *
 * After the DCI has been created, it is necessary to prepare the HarqProcess,
 * store it in the UE pointer, and distribute the TBS among the active LC of the
 * UE, through the method AssignBytesToLC, and creating the corresponding list
 * of RlcPduInfo.
 *
 * Before looping and changing the beam, the starting point should be advanced.
 * How that is done is a matter for the subclasses (method ChangeUlBeam).
 */
uint8_t
MmWaveMacSchedulerNs3::DoScheduleUlData (PointInFTPlane *spoint, uint32_t symAvail,
                                         const ActiveUeMap &activeUl, SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (symAvail > 0 && activeUl.size () > 0);
  NS_ASSERT (spoint->m_rbg == 0);

  BeamSymbolMap symPerBeam = AssignULRBG (symAvail, activeUl);
  uint8_t usedSym = 0;
  GetFirst GetBeam;
  for (const auto &beam : activeUl)
    {
      uint32_t availableRBG = (m_phyMacConfig->GetBandwidthInRbg () - spoint->m_rbg) * symPerBeam.at (GetBeam (beam));
      bool assigned = false;

      NS_LOG_DEBUG (activeUl.size () << " active UL beam, this beam has " <<
                    symPerBeam.at (GetBeam (beam)) << " SYM, starts from RBG " <<
                    static_cast<uint32_t> (spoint->m_rbg) <<
                    " and symbol " << static_cast<uint32_t> (spoint->m_sym) <<
                    " (going backward) for a total of " << availableRBG <<
                    " RBG. In one symbol we have " <<
                    m_phyMacConfig->GetBandwidthInRbg () << " RBG.");

      if (symPerBeam.at (GetBeam (beam)) == 0)
        {
          NS_LOG_INFO ("No available symbols for this beam, continue");
          continue;
        }

      for (const auto &ue : beam.second)
        {
          if (ue.first->m_ulRBG == 0)
            {
              NS_LOG_INFO ("UE " << ue.first->m_rnti << " does not have RBG assigned");
              continue;
            }

          std::shared_ptr<DciInfoElementTdma> dci = CreateUlDci (spoint, ue.first);

          if (dci == nullptr)
            {
              NS_LOG_DEBUG ("No DCI has been created, ignoring");
              ue.first->ResetDlMetric ();
              continue;
            }

          assigned = true;

          if (!ue.first->m_ulHarq.CanInsert ())
            {
              NS_LOG_INFO ("Harq Vector condition for UE " << ue.first->m_rnti <<
                           std::endl << ue.first->m_ulHarq);
              NS_FATAL_ERROR ("UE " << ue.first->m_rnti << " does not have UL HARQ space");
            }

          HarqProcess harqProcess (true, HarqProcess::WAITING_FEEDBACK, 0, dci);
          uint8_t id;
          ue.first->m_ulHarq.Insert (&id, harqProcess);

          ue.first->m_ulHarq.Get (id).m_dciElement->m_harqProcess = id;

          VarTtiAllocInfo slotInfo (VarTtiAllocInfo::UL, VarTtiAllocInfo::DATA, dci);

          NS_LOG_INFO ("Assigned process ID " << static_cast<uint32_t> (dci->m_harqProcess) <<
                       " to UE " << ue.first->m_rnti);
          NS_LOG_DEBUG (" UE" << dci->m_rnti <<
                        " gets UL symbols " << static_cast<uint32_t> (dci->m_symStart) <<
                        "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym) <<
                        " tbs " << dci->m_tbSize <<
                        " mcs " << static_cast<uint32_t> (dci->m_mcs) <<
                        " harqId " << static_cast<uint32_t> (id) <<
                        " rv " << static_cast<uint32_t> (dci->m_rv)
                        );

          auto distributedBytes = AssignBytesToLC (ue.first->m_ulLCG, dci->m_tbSize);
          bool assignedToLC = false;
          for (const auto & byteDistribution : distributedBytes)
            {
              assignedToLC = true;
              ue.first->m_ulLCG.at (byteDistribution.m_lcg)->AssignedData (byteDistribution.m_lcId, byteDistribution.m_bytes);
              NS_LOG_DEBUG ("UL LCG " << static_cast<uint32_t> (byteDistribution.m_lcg) <<
                            " assigned bytes " << byteDistribution.m_bytes << " to LCID " <<
                            static_cast<uint32_t> (byteDistribution.m_lcId));
            }
          NS_ASSERT (assignedToLC);
          slotAlloc->m_varTtiAllocInfo.emplace_front (slotInfo);
        }
      if (assigned)
        {
          ChangeUlBeam (spoint, symPerBeam.at (GetBeam (beam)));
          usedSym += symPerBeam.at (GetBeam (beam));
          slotAlloc->m_numSymAlloc += symPerBeam.at (GetBeam (beam));
        }
    }

  for (auto & beam : activeUl)
    {
      for (auto & ue : beam.second)
        {
          ue.first->ResetUlSchedInfo ();
        }
    }

  NS_ASSERT (spoint->m_rbg == 0);

  return usedSym;
}

/**
 * \brief Schedule received SR
 * \param spoint Starting point for allocation
 * \param symAvail Available symbols
 * \param rntiList list of RNTI which asked for a SR
 * \param slotAlloc Slot allocation list
 * \return Number of symbols allocated for SR
 *
 * It schedules the SR. Each time an UE asks for SR, the scheduler will try
 * to allocate a default amount of 1 symbol for that UE. In such symbol,
 * it can transmit data and, eventually, a BSR.
 *
 * This function does not update any byte in LCGs, since the UE did not report
 * a BSR yet.
 */
uint8_t
MmWaveMacSchedulerNs3::DoScheduleUlSr (MmWaveMacSchedulerNs3::PointInFTPlane *spoint,
                                       uint32_t symAvail, std::list<uint16_t> *rntiList,
                                       SlotAllocInfo *slotAlloc) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (symAvail > 0);
  NS_ASSERT (spoint->m_rbg == 0);
  uint8_t usedSym = 0;
  std::list<uint16_t> notScheduled;

  while (symAvail > 0 && rntiList->size () > 0)
    {
      uint16_t rnti = rntiList->front ();
      rntiList->pop_front ();
      NS_ABORT_UNLESS (m_ueMap.find (rnti) != m_ueMap.end ());

      // Assign an entire symbol
      auto & ue = m_ueMap.find (rnti)->second;
      NS_ASSERT (ue->m_ulRBG == 0);
      uint32_t tbs = 0;
      uint32_t assignedSym = 0;
      do
        {
          ue->m_ulRBG += m_phyMacConfig->GetBandwidthInRbg ();

          assignedSym++;
          tbs = m_amc->GetTbSizeFromMcsSymbols (ue->m_ulMcs,
                                                ue->m_ulRBG * m_phyMacConfig->GetNumRbPerRbg ()) / 8;
        }
      while (tbs < 4 && (symAvail - assignedSym) > 0);    // Why 4? Because I suppose that's good, giving the MacHeader is 2.

      if (tbs < 4)
        {
          // Our try is a fail: we can't schedule this SR
          notScheduled.push_back (rnti);
          ue->ResetUlSchedInfo ();
          continue;
        }

      NS_ASSERT (symAvail >= assignedSym);

      usedSym += assignedSym;
      symAvail -= assignedSym;

      // Create DCI
      std::shared_ptr<DciInfoElementTdma> dci = CreateUlDci (spoint, ue);
      NS_ASSERT (dci != nullptr);

      NS_ABORT_MSG_UNLESS (ue->m_ulHarq.CanInsert (), " UE " << ue->m_rnti <<
                           " can't insert an HARQ for SR");
      HarqProcess harqProcess (true, HarqProcess::WAITING_FEEDBACK, 0, dci);
      uint8_t id;
      bool ret = ue->m_ulHarq.Insert (&id, harqProcess);
      NS_ASSERT (ret);

      ue->m_ulHarq.Get (id).m_dciElement->m_harqProcess = id;

      VarTtiAllocInfo slotInfo (VarTtiAllocInfo::UL, VarTtiAllocInfo::DATA, dci);

      NS_LOG_DEBUG (" UE" << dci->m_rnti <<
                    " gets UL symbols " << static_cast<uint32_t> (dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym) <<
                    " tbs " << dci->m_tbSize <<
                    " mcs " << static_cast<uint32_t> (dci->m_mcs) <<
                    " harqId " << static_cast<uint32_t> (id) <<
                    " rv " << static_cast<uint32_t> (dci->m_rv) <<
                    " process ID " << static_cast<uint32_t> (dci->m_harqProcess) <<
                    " thanks to a SR");

      ue->ResetUlSchedInfo ();
      slotAlloc->m_varTtiAllocInfo.emplace_front (slotInfo);
      slotAlloc->m_numSymAlloc += usedSym;
    }

  for (const auto & rnti : notScheduled)
    {
      rntiList->push_back (rnti);
    }

  return usedSym;
}

/**
 * \brief Do the process of scheduling for the DL
 * \param params scheduling parameters
 * \param dlHarqFeedback vector of DL HARQ negative feedback
 *
 * The scheduling process is composed by the UL and the DL parts, and it
 * decides how the resources should be divided between UEs. An important
 * thing to remember is that the slot being considered for DL decision can be
 * different for the slot for UL decision. This offset is due to the parameter
 * UlSchedDelay of the class MmWavePhyMacCommon.
 *
 * Another parameter to consider is the L1L2CtrlLatency (present in
 * MmWavePhyMacCommon as well) that defines the delay (in slots number) between
 * the slot that is currently "in the air" and the slot which is being prepared
 * for DL.
 * The default value for both L1L2CtrlLatency and UlSchedDelay is 2, so it means
 * that while the slot number (frame, subframe, slot) is in the air, the
 * scheduler in this function will take decisions for DL in slot number
 * (frame, subframe, slot) + 2 and for UL in slot number (frame, subframe, slot) + 4.
 *
 * The consequences are an additional complexity derived from the fact that the
 * DL scheduling for a slot should remember the previous UL scheduling done in the
 * past for that slot.
 *
 * The process of scheduling DL data is defined as follows:
 *
 * - Retrieve the allocation done in the past (UL) for this slot
 * - Prepend DL CTRL symbol to the allocation list;
 * - Perform a scheduling for DL HARQ/data for this (DoScheduleDl());
 * - Indicate to the MAC the decision for that slot through SchedConfigInd()
 *
 * To know how the scheduling for DL is performed, take a look to the
 * function documentation for DoScheduleDl().
 *
 * At the end, the return of DoScheduleDl is passed to MAC through the function
 * SchedConfigInd().
 *
 * \see PointInFTPlane
 */
void
MmWaveMacSchedulerNs3::ScheduleDl (const MmWaveMacSchedSapProvider::SchedDlTriggerReqParameters& params,
                                   const std::vector <DlHarqInfo> &dlHarqFeedback)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (params.m_snfSf.m_slotNum <= UINT8_MAX);
  NS_LOG_INFO ("Scheduling invoked for slot " << params.m_snfSf);

  MmWaveMacSchedSapUser::SchedConfigIndParameters dlSlot (params.m_snfSf);
  dlSlot.m_slotAllocInfo.m_sfnSf = params.m_snfSf;
  NS_ASSERT (m_ulAllocationMap.find (params.m_snfSf.Encode ()) != m_ulAllocationMap.end ());
  auto ulAllocationIt = m_ulAllocationMap.find (params.m_snfSf.Encode ()); // UL allocations for this slot
  auto & ulAllocations = ulAllocationIt->second;

  // add slot for DL control, at symbol 0
  PrependCtrlSym (0, m_phyMacConfig->GetDlCtrlSymbols (), VarTtiAllocInfo::DL,
                  &dlSlot.m_slotAllocInfo.m_varTtiAllocInfo);
  dlSlot.m_slotAllocInfo.m_numSymAlloc += m_phyMacConfig->GetDlCtrlSymbols ();

  DoScheduleDl (dlHarqFeedback, params.m_snfSf, ulAllocations, &dlSlot.m_slotAllocInfo);

  // if no UL allocation, then erase the element. If UL allocation, then
  // the element will be erased when the CQI for that UL allocation will be received
  if (ulAllocations.m_totUlSym == 0)
    {
      NS_LOG_INFO ("Removing UL allocation for slot " << params.m_snfSf <<
                   " size " << m_ulAllocationMap.size ());
      m_ulAllocationMap.erase (ulAllocationIt);
    }

  NS_LOG_INFO ("Total DCI for DL : " << dlSlot.m_slotAllocInfo.m_varTtiAllocInfo.size () <<
               " including DL CTRL");
  m_macSchedSapUser->SchedConfigInd (dlSlot);
}

/**
 * \brief Do the process of scheduling for the UL
 * \param params scheduling parameters
 * \param ulHarqFeedback vector of UL HARQ negative feedback
 *
 * The scheduling process is composed by the UL and the DL parts, and it
 * decides how the resources should be divided between UEs. An important
 * thing to remember is that the slot being considered for DL decision can be
 * different for the slot for UL decision. This offset is due to the parameter
 * UlSchedDelay of the class MmWavePhyMacCommon.
 *
 * Another parameter to consider is the L1L2CtrlLatency (present in
 * MmWavePhyMacCommon as well) that defines the delay (in slots number) between
 * the slot that is currently "in the air" and the slot which is being prepared
 * for DL.
 * The default value for both L1L2CtrlLatency and UlSchedDelay is 2, so it means
 * that while the slot number (frame, subframe, slot) is in the air, the
 * scheduler in this function will take decisions for DL in slot number
 * (frame, subframe, slot) + 2 and for UL in slot number (frame, subframe, slot) + 4.
 *
 * The consequences are an additional complexity derived from the fact that the
 * DL scheduling for a slot should remember the previous UL scheduling done in the
 * past for that slot.
 *
 * The process of scheduling UL data is defined as follows:
 *
 * - Perform a scheduling for UL HARQ/data for this (DoScheduleUl());
 * - Append a UL CTRL symbol to the allocation list;
 * - Indicate to the MAC the decision for that slot through SchedConfigInd()
 *
 * To know how the scheduling for UL is performed, take a look to the
 * function documentation for DoScheduleUl().
 *
 * At the end, the return of DoScheduleUl is passed to MAC through the function
 * SchedConfigInd().
 *
 * \see PointInFTPlane
 */
void
MmWaveMacSchedulerNs3::ScheduleUl (const MmWaveMacSchedSapProvider::SchedUlTriggerReqParameters& params,
                                   const std::vector <UlHarqInfo> &ulHarqFeedback)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (params.m_snfSf.m_slotNum <= UINT8_MAX);
  NS_LOG_INFO ("Scheduling invoked for slot " << params.m_snfSf);

  MmWaveMacSchedSapUser::SchedConfigIndParameters ulSlot (params.m_snfSf);
  ulSlot.m_slotAllocInfo.m_sfnSf = params.m_snfSf;

  // Doing UL for slot ulSlot
  DoScheduleUl (ulHarqFeedback, params.m_snfSf, &ulSlot.m_slotAllocInfo);

  // add slot for UL control, at last symbol (13 in most cases)
  AppendCtrlSym (static_cast<uint8_t> (m_phyMacConfig->GetSymbolsPerSlot () - 1),
                 1, VarTtiAllocInfo::UL,
                 &ulSlot.m_slotAllocInfo.m_varTtiAllocInfo);
  ulSlot.m_slotAllocInfo.m_numSymAlloc += 1;

  NS_LOG_INFO ("Total DCI for UL : " << ulSlot.m_slotAllocInfo.m_varTtiAllocInfo.size () <<
               " including UL CTRL");
  m_macSchedSapUser->SchedConfigInd (ulSlot);
}

/**
 * \brief Schedule UL HARQ and data
 * \param activeUlHarq List of active HARQ processes in UL
 * \param ulSfn Slot number
 * \param allocInfo Allocation info pointer (where to save the allocations)
 * \return the number of symbols used for the UL allocation
 *
 * The UL phase is, in this implementation, based completely on TDMA, so
 * in this method (even if the code is modular enough to be subclassed to
 * create a OFDMA-based scheduler) expects a TDMA-based allocations. You have
 * been warned!!
 *
 * The UL allocation is done for a slot number that is greater or equal to the
 * slot number for DL. Therefore, the maximum symbols available for this phase
 * are all the available data symbols in the slot. This opens up fairness problems
 * with DL, but how to balance UL and DL is still an open problem.
 *
 * If there are HARQ to retransmit, they will be retransmitted in the function
 * ScheduleUlHarq() until the code runs out of available symbols or
 * available HARQ to retransmit. The UE that can be selected for such assignation
 * are decided in the function ComputeActiveHarq.
 *
 * Then, for all the UE that requested a SR, it will be allocated one entire
 * symbol. After that, if any symbol remains, the function will schedule data.
 * The data allocation is made in DoScheduleUlData(), only for UEs that
 * have been selected by the function ComputeActiveUe.
 *
 * If any assignation is made, then the member variable m_ulAllocationMap (SlotElem)
 * is updated by storing the total UL symbols used in this slot, and
 * the details of each assignation, which include TBS, symStart, numSym, and
 * MCS. The assignations are stored as a list of AllocElem.
 *
 * Please note that the allocation must be done backwards from the last available
 * symbol to respect the final slot composition: DL CTRL, DL Data, UL Data, UL CTRL.
 */
uint8_t
MmWaveMacSchedulerNs3::DoScheduleUl (const std::vector <UlHarqInfo> &ulHarqFeedback,
                                     const SfnSf &ulSfn,
                                     SlotAllocInfo *allocInfo)
{
  NS_LOG_INFO (this);

  NS_ASSERT (allocInfo->m_varTtiAllocInfo.size () == 0);

  const uint8_t dataSymPerSlot = m_phyMacConfig->GetSymbolsPerSlot () -
    m_phyMacConfig->GetDlCtrlSymbols () - m_phyMacConfig->GetUlCtrlSymbols ();

  ActiveHarqMap activeUlHarq;
  ComputeActiveHarq (&activeUlHarq, ulHarqFeedback);

  // Start the assignation from the last available data symbol, and like a shrimp
  // go backward.
  uint8_t lastSym = m_phyMacConfig->GetSymbolsPerSlot () - m_phyMacConfig->GetUlCtrlSymbols ();
  PointInFTPlane ulAssignationStartPoint (0, lastSym);
  uint8_t ulSymAvail = dataSymPerSlot;

  // Create the UL allocation map entry
  m_ulAllocationMap.emplace (ulSfn.Encode (), SlotElem (0));

  NS_LOG_DEBUG ("Scheduling UL frame " << static_cast<uint32_t> (ulSfn.m_frameNum) <<
                " subframe " << static_cast<uint32_t> (ulSfn.m_subframeNum) <<
                " slot " << static_cast<uint32_t> (ulSfn.m_slotNum) <<
                " UL HARQ to retransmit: " << ulHarqFeedback.size () <<
                " Active Beams UL HARQ: " << activeUlHarq.size ());

  if (activeUlHarq.size () > 0)
    {
      uint8_t usedHarq = ScheduleUlHarq (&ulAssignationStartPoint, ulSymAvail,
                                         m_ueMap, &m_ulHarqToRetransmit, ulHarqFeedback,
                                         allocInfo);
      NS_ASSERT_MSG (ulSymAvail >= usedHarq, "Available: " << +ulSymAvail <<
                     " used by HARQ: " << +usedHarq);
      NS_LOG_INFO ("For the slot " << ulSfn << " reserved " <<
                   static_cast<uint32_t> (usedHarq) << " symbols for UL HARQ retx");
      ulSymAvail -= usedHarq;
    }

  NS_ASSERT (ulAssignationStartPoint.m_rbg == 0);

  // Now schedule SR if we have empty symbols
  uint8_t usedSr = 0;

  if (ulSymAvail > 0 && m_srList.size () > 0)
    {
      usedSr = DoScheduleUlSr (&ulAssignationStartPoint, ulSymAvail,
                               &m_srList, allocInfo);
      ulSymAvail -= usedSr;
    }


  ActiveUeMap activeUlUe;
  ComputeActiveUe (&activeUlUe, allocInfo, &MmWaveMacSchedulerUeInfo::GetUlLCG, "UL");

  if (ulSymAvail > 0 && activeUlUe.size () > 0)
    {
      uint8_t usedUl = DoScheduleUlData (&ulAssignationStartPoint, ulSymAvail,
                                         activeUlUe, allocInfo);
      NS_LOG_INFO ("For the slot " << ulSfn << " reserved " <<
                   static_cast<uint32_t> (usedUl) << " symbols for UL data tx");
      ulSymAvail -= usedUl;
    }

  if (allocInfo->m_varTtiAllocInfo.size () > 0)
    {
      auto & totUlSym = m_ulAllocationMap.at (ulSfn.Encode ()).m_totUlSym;
      auto & allocations = m_ulAllocationMap.at (ulSfn.Encode ()).m_ulAllocations;
      for (const auto &alloc : allocInfo->m_varTtiAllocInfo)
        {
          if (alloc.m_varTtiType == VarTtiAllocInfo::DATA && alloc.m_tddMode == VarTtiAllocInfo::UL)
            {
              // THIS DOESN'T WORK FOR UL OFDMA (IF EXISTS)
              NS_LOG_INFO ("Placed an allocation in the map for the CQI, RNTI " <<
                           alloc.m_dci->m_rnti << ", symStart " <<
                           static_cast<uint32_t>(alloc.m_dci->m_symStart) <<
                           " numSym " << alloc.m_dci->m_mcs);
              allocations.emplace_back (AllocElem (alloc.m_dci->m_rnti,
                                                   m_phyMacConfig->GetBandwidthInRbs (), // Assume UL in TDMA
                                                   alloc.m_dci->m_tbSize,
                                                   alloc.m_dci->m_symStart,
                                                   alloc.m_dci->m_numSym,
                                                   alloc.m_dci->m_mcs));
              totUlSym += alloc.m_dci->m_numSym;
            }
        }

      NS_ASSERT_MSG (dataSymPerSlot - ulSymAvail == totUlSym,
                     "UL Data symbols available: " << static_cast<uint32_t> (dataSymPerSlot) <<
                     " UL symbols available at end of sched: " << static_cast<uint32_t> (ulSymAvail) <<
                     " total of symbols registered in the allocation: " << static_cast<uint32_t> (totUlSym));

      NS_LOG_INFO ("For the slot " << ulSfn << " registered a total of " <<
                   static_cast<uint32_t> (totUlSym) <<
                   " symbols and " << allocations.size () << " allocations");
    }

  return dataSymPerSlot - ulSymAvail;
}

/**
 * \brief Schedule DL HARQ and data
 * \param dlSfnSf Slot number
 * \param ulAllocations Uplink allocation for this slot
 * \param allocInfo Allocation info pointer (where to save the allocations)
 * \return the number of symbols used for the DL allocation
 *
 * The DL phase can be OFDMA-based or TDMA-based. The method calculates the
 * number of available symbols as the total number of symbols in one slot
 * minus the number of symbols already allocated for UL. Then, it defines
 * the data starting point by keeping in consideration the number of symbols
 * previously allocated. In this way, DL and UL allocation will not overlap.
 *
 * HARQ retx processing is done in the function  ScheduleDlHarq(), while
 * DL new data processing in the function ScheduleDlData(). Before each phase,
 * the UEs are selected: for HARQ, the function ComputeActiveHarq() is used.
 * For data, the function ComputeActiveUe.
 *
 */
uint8_t
MmWaveMacSchedulerNs3::DoScheduleDl (const std::vector <DlHarqInfo> &dlHarqFeedback,
                                     const SfnSf &dlSfnSf,
                                     const SlotElem &ulAllocations,
                                     SlotAllocInfo *allocInfo)
{
  NS_LOG_INFO (this);

  // compute active ue in the current subframe, group them by BeamId
  ActiveHarqMap activeDlHarq;
  ComputeActiveHarq (&activeDlHarq, dlHarqFeedback);

  const uint8_t dataSymPerSlot = m_phyMacConfig->GetSymbolsPerSlot () -
    m_phyMacConfig->GetDlCtrlSymbols () - m_phyMacConfig->GetUlCtrlSymbols ();

  uint8_t dlSymAvail = dataSymPerSlot - ulAllocations.m_totUlSym;
  PointInFTPlane dlAssignationStartPoint (0, m_phyMacConfig->GetDlCtrlSymbols ());

  NS_LOG_DEBUG ("Scheduling DL frame " << static_cast<uint32_t> (dlSfnSf.m_frameNum) <<
                " subframe " << static_cast<uint32_t> (dlSfnSf.m_subframeNum) <<
                " slot " << static_cast<uint32_t> (dlSfnSf.m_slotNum) <<
                " DL HARQ to retransmit: " << dlHarqFeedback.size () <<
                " Active Beams DL HARQ: " << activeDlHarq.size () <<
                " sym available: " << static_cast<uint32_t> (dlSymAvail) <<
                " starting from sym " << static_cast<uint32_t> (m_phyMacConfig->GetDlCtrlSymbols ()));

  if (activeDlHarq.size () > 0)
    {
      uint8_t usedHarq = ScheduleDlHarq (&dlAssignationStartPoint, dlSymAvail,
                                         activeDlHarq, m_ueMap, &m_dlHarqToRetransmit,
                                         dlHarqFeedback, allocInfo);
      NS_ASSERT (dlSymAvail >= usedHarq);
      dlSymAvail -= usedHarq;
    }

  NS_ASSERT (dlAssignationStartPoint.m_rbg == 0);

  ActiveUeMap activeDlUe;
  ComputeActiveUe (&activeDlUe, allocInfo, &MmWaveMacSchedulerUeInfo::GetDlLCG, "DL");

  if (dlSymAvail > 0 && activeDlUe.size () > 0)
    {
      uint8_t usedDl = DoScheduleDlData (&dlAssignationStartPoint, dlSymAvail,
                                         activeDlUe, allocInfo);
      NS_ASSERT (dlSymAvail >= usedDl);
      dlSymAvail -= usedDl;
    }

  return (dataSymPerSlot - ulAllocations.m_totUlSym) - dlSymAvail;
}

/**
 * \brief Decide how to fill the frequency/time of a DL slot
 * \param params parameters for the scheduler
 *
 * The function starts by refreshing the CQI received, and eventually resetting
 * the expired values. Then, the HARQ feedback are processed (ProcessHARQFeedbacks),
 * and finally the expired HARQs are canceled (ResetExpiredHARQ).
 *
 * \see ScheduleDl
 */
void
MmWaveMacSchedulerNs3::DoSchedDlTriggerReq (const MmWaveMacSchedSapProvider::SchedDlTriggerReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  // process received CQIs
  m_cqiManagement.RefreshDlCqiMaps (m_ueMap);

  // reset expired HARQ
  for (const auto & itUe : m_ueMap)
    {
      ResetExpiredHARQ (itUe.second->m_rnti, &itUe.second->m_dlHarq);
    }

  // Merge not-retransmitted and received feedback
  std::vector <DlHarqInfo> dlHarqFeedback;

  if (params.m_dlHarqInfoList.size () > 0 || m_dlHarqToRetransmit.size () > 0)
    {
      // m_dlHarqToRetransmit will be cleared inside MergeHARQ
      uint64_t existingSize = m_dlHarqToRetransmit.size ();
      uint64_t inSize = params.m_dlHarqInfoList.size ();

      dlHarqFeedback = MergeHARQ (&m_dlHarqToRetransmit, params.m_dlHarqInfoList, "DL");

      NS_ASSERT (m_dlHarqToRetransmit.size () == 0);
      NS_ASSERT_MSG (existingSize + inSize == dlHarqFeedback.size (),
                     " existing: " << existingSize << " received: " << inSize <<
                     " calculated: " << dlHarqFeedback.size ());

      std::unordered_map<uint16_t, std::set<uint32_t>> feedbacksDup;

      // Let's find out:
      // 1) Feedback that arrived late (i.e., their process has been marked inactive
      //    due to timings
      // 2) Duplicated feedbacks (same UE, same process ID). I don't know why
      //    these are generated.. but anyway..
      for (auto it = dlHarqFeedback.begin (); it != dlHarqFeedback.end (); /* no inc */)
        {
          auto & ueInfo = m_ueMap.find (it->m_rnti)->second;
          auto & process = ueInfo->m_dlHarq.Find (it->m_harqProcessId)->second;
          NS_LOG_INFO ("Analyzing feedback for UE " << it->m_rnti << " process " <<
                       static_cast<uint32_t> (it->m_harqProcessId));
          if (!process.m_active)
            {
              NS_LOG_INFO ("Feedback for UE " << it->m_rnti << " process " <<
                           static_cast<uint32_t> (it->m_harqProcessId) <<
                           " ignored because process is INACTIVE");
              it = dlHarqFeedback.erase (it);    /* INC */
            }
          else
            {
              auto itDuplicated = feedbacksDup.find(it->m_rnti);
              if (itDuplicated == feedbacksDup.end())
                {
                  feedbacksDup.insert(std::make_pair (it->m_rnti, std::set<uint32_t> ()));
                  feedbacksDup.at(it->m_rnti).insert(it->m_harqProcessId);
                  ++it; /* INC */
                }
              else
                {
                  if (itDuplicated->second.find(it->m_harqProcessId) == itDuplicated->second.end())
                    {
                      itDuplicated->second.insert(it->m_harqProcessId);
                      ++it; /* INC */
                    }
                  else
                    {
                      NS_LOG_INFO ("Feedback for UE " << it->m_rnti << " process " <<
                                   static_cast<uint32_t> (it->m_harqProcessId) <<
                                   " ignored because is a duplicate of another feedback");
                      it = dlHarqFeedback.erase (it); /* INC */
                    }
                }
            }
        }

      ProcessHARQFeedbacks (&dlHarqFeedback, MmWaveMacSchedulerUeInfo::GetDlHarqVector,
                            "DL");
    }

  ScheduleDl (params, dlHarqFeedback);
}

/**
 * \brief Decide how to fill the frequency/time of a UL slot
 * \param params parameters for the scheduler
 *
 * The function starts by refreshing the CQI received, and eventually resetting
 * the expired values. Then, the HARQ feedback are processed (ProcessHARQFeedbacks),
 * and finally the expired HARQs are canceled (ResetExpiredHARQ).
 *
 * \see ScheduleUl
 */
void
MmWaveMacSchedulerNs3::DoSchedUlTriggerReq (const MmWaveMacSchedSapProvider::SchedUlTriggerReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  // process received CQIs
  m_cqiManagement.RefreshUlCqiMaps (m_ueMap);

  // reset expired HARQ
  for (const auto & itUe : m_ueMap)
    {
      ResetExpiredHARQ (itUe.second->m_rnti, &itUe.second->m_ulHarq);
    }

  // Merge not-retransmitted and received feedback
  std::vector <UlHarqInfo> ulHarqFeedback;
  if (params.m_ulHarqInfoList.size () > 0 || m_ulHarqToRetransmit.size () > 0)
    {
      // m_ulHarqToRetransmit will be cleared inside MergeHARQ
      uint64_t existingSize = m_ulHarqToRetransmit.size ();
      uint64_t inSize = params.m_ulHarqInfoList.size ();

      ulHarqFeedback = MergeHARQ (&m_ulHarqToRetransmit, params.m_ulHarqInfoList, "UL");

      NS_ASSERT (m_ulHarqToRetransmit.size () == 0);
      NS_ASSERT_MSG (existingSize + inSize == ulHarqFeedback.size (),
                     " existing: " << existingSize << " received: " << inSize <<
                     " calculated: " << ulHarqFeedback.size ());

      // if there are feedbacks for expired process, remove them
      for (auto it = ulHarqFeedback.begin (); it != ulHarqFeedback.end (); /* no inc */)
        {
          auto & ueInfo = m_ueMap.find (it->m_rnti)->second;
          auto & process = ueInfo->m_ulHarq.Find (it->m_harqProcessId)->second;
          if (!process.m_active)
            {
              NS_LOG_INFO ("Feedback for UE " << it->m_rnti << " process " <<
                           static_cast<uint32_t> (it->m_harqProcessId) <<
                           " ignored because process is INACTIVE");
              it = ulHarqFeedback.erase (it);
            }
          else
            {
              ++it;
            }
        }

      ProcessHARQFeedbacks (&ulHarqFeedback, MmWaveMacSchedulerUeInfo::GetUlHarqVector,
                            "UL");
    }

  ScheduleUl (params, ulHarqFeedback);
}

/**
 * \brief Save the SR list into m_srList
 * \param params SR list
 *
 * m_srList will be evaluated in DoScheduleUlSr()
 */
void
MmWaveMacSchedulerNs3::DoSchedUlSrInfoReq (const MmWaveMacSchedSapProvider::SchedUlSrInfoReqParameters &params)
{
  NS_LOG_FUNCTION (this);
  for (const auto & ue : params.m_srList)
    {
      NS_LOG_INFO ("UE " << ue << " asked for a SR ");
    }

  // Merge RNTI in our current list
  for (const auto & ue : params.m_srList)
    {
      m_srList.push_back (ue);
    }
  NS_ASSERT (m_srList.size () >= params.m_srList.size ());
}

} // namespace ns3
