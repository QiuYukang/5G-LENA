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

BwpManagerUe::BwpManagerUe () : SimpleUeComponentCarrierManager ()
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

  uint8_t bwpIndex = m_algorithm->GetBwpForEpsBearer (m_lcToBearerMap.at (params.lcid));

  NS_LOG_DEBUG ("BSR of size " << params.txQueueSize << " from RLC for LCID = " <<
                static_cast<uint32_t> (params.lcid) << " reported to CcId " <<
                static_cast<uint32_t> (bwpIndex));

  m_componentCarrierLcMap.at (bwpIndex).at (params.lcid)->ReportBufferStatus (params);
}

std::vector<LteUeCcmRrcSapProvider::LcsConfig>
BwpManagerUe::DoAddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("For LC ID " << static_cast<uint32_t> (lcId) << " bearer qci " <<
               static_cast<uint32_t> (lcConfig.priority) <<
               " from priority " << static_cast<uint32_t> (lcConfig.priority));

  // see lte-enb-rrc.cc:453
  m_lcToBearerMap.insert (std::make_pair (lcId, static_cast<EpsBearer::Qci> (lcConfig.priority)));

  return SimpleUeComponentCarrierManager::DoAddLc (lcId, lcConfig, msu);
}

LteMacSapUser
*BwpManagerUe::DoConfigureSignalBearer (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this);

  // Ignore signaling bearers for the moment. These are for an advanced use.
  // m_lcToBearerMap.insert (std::make_pair (lcId, EpsBearer::FromPriority (lcConfig.priority).qci));

  return SimpleUeComponentCarrierManager::DoConfigureSignalBearer (lcId, lcConfig, msu);
}

} // namespace ns3
