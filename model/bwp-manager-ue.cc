/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include "bwp-manager-ue.h"
#include "bwp-manager-algorithm.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BwpManagerUe");
NS_OBJECT_ENSURE_REGISTERED (BwpManagerUe);

BwpManagerUe::BwpManagerUe() : SimpleUeComponentCarrierManager ()
{
  NS_LOG_FUNCTION (this);
  m_algorithm = new BwpManagerAlgorithmStatic (); // When we will have different types,
                                                           // Then we will add an Attribute.
}

BwpManagerUe::~BwpManagerUe ()
{
  NS_LOG_FUNCTION (this);
  delete m_algorithm;
}


TypeId
BwpManagerUe::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BwpManagerUe")
    .SetParent<SimpleUeComponentCarrierManager> ()
    .SetGroupName ("nr")
    .AddConstructor<BwpManagerUe> ()
    ;
  return tid;
}

void
BwpManagerUe::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);

  return SimpleUeComponentCarrierManager::DoReportBufferStatus (params);
  /*
  uint8_t qci = m_lcToBearerMap.at (params.lcid);

  // Force a conversion between the uint8_t type that comes from the LcInfo
  // struct (yeah, using the EpsBearer::Qci type was too hard ...)
  uint8_t bwpIndex = m_algorithm->GetBwpForEpsBearer (static_cast<EpsBearer::Qci> (qci));


  NS_LOG_DEBUG ("BSR of size " << params.txQueueSize << " from RLC for LCID = " <<
                static_cast<uint32_t> (params.lcid) << " reported to CcId " <<
                static_cast<uint32_t>(bwpIndex));

  m_componentCarrierLcMap.at(bwpIndex).at(params.lcid)->ReportBufferStatus(params);
  */
}

std::vector<LteUeCcmRrcSapProvider::LcsConfig>
BwpManagerUe::DoAddLc(uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this);

  /*
  NS_LOG_DEBUG ("Add LC ID " << static_cast<uint32_t> (lcId) << " with Bearer ID " <<
                static_cast<uint32_t> (lcConfig.epsBearer));

  LteUeCcmRrcSapProvider::LcsConfig elem;
  std::vector<LteUeCcmRrcSapProvider::LcsConfig> ret;
  std::map<uint8_t, LteMacSapUser*>::iterator it = m_lcAttached.find (lcId);

  NS_ABORT_MSG_IF (it != m_lcAttached.end (), "Warning, LCID " << lcId << " already exist");

  m_lcAttached.insert (std::make_pair (lcId, msu)); // I can't touch this, it's in LTE.
  m_lcToBearerMap.insert (std::make_pair (lcId, lcConfig.epsBearer));

  uint8_t bwpIndex = m_algorithm->GetBwpForEpsBearer (static_cast<EpsBearer::Qci> (lcConfig.epsBearer));

  NS_LOG_DEBUG ("The LCID " << static_cast<uint32_t> (lcId) << " bearer id " <<
                static_cast<uint32_t> (lcConfig.epsBearer) << " is assigned to BWP ID " <<
                static_cast<uint32_t> (bwpIndex));
  NS_LOG_INFO ("We have " << m_componentCarrierLcMap.size () << " elements in CC to LC map");

  auto ccLcMapIt = m_componentCarrierLcMap.find (bwpIndex);
  if (ccLcMapIt != m_componentCarrierLcMap.end())
    {
      NS_LOG_INFO ("Insert for the BWP " << static_cast<uint32_t> (bwpIndex) <<
                   " the LC " << static_cast<uint32_t> (lcId));
      ccLcMapIt->second.insert (std::make_pair (lcId, m_macSapProvidersMap.at (bwpIndex)));
    }
  else
    {
      std::map<uint8_t, LteMacSapProvider*> empty;
      std::pair <std::map <uint8_t, std::map<uint8_t, LteMacSapProvider*> >::iterator, bool>
      ret = m_componentCarrierLcMap.insert (std::make_pair (bwpIndex, empty));
      NS_LOG_INFO ("Push BWP " << static_cast<uint32_t> (bwpIndex) << " in the CC to LC map");
      NS_ABORT_MSG_IF (!ret.second, "element already present, ComponentCarrierId already exist");
      ccLcMapIt = m_componentCarrierLcMap.find (bwpIndex);
      ccLcMapIt->second.insert (std::make_pair (lcId, m_macSapProvidersMap.at (bwpIndex)));
      NS_LOG_INFO ("Insert for the BWP " << static_cast<uint32_t> (bwpIndex) <<
                   " the LC " << static_cast<uint32_t> (lcId));
    }

  elem.componentCarrierId = bwpIndex;
  elem.lcConfig = lcConfig;
  elem.msu = m_ccmMacSapUser;

  ret.push_back (elem);
  return ret;
  */
  return SimpleUeComponentCarrierManager::DoAddLc(lcId, lcConfig, msu);
}

LteMacSapUser
*BwpManagerUe::DoConfigureSignalBearer(uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this);

  return SimpleUeComponentCarrierManager::DoConfigureSignalBearer(lcId, lcConfig, msu);

  // Don't do nothig.
  NS_LOG_DEBUG ("Don't configure Signal Bearer. We don't even have the bearer ID..");

  return m_ccmMacSapUser;
}

} // namespace ns3
