/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Biljana Bojovic <bbojovic@cttc.cat>
 */

#include "bwp-manager-gnb.h"
#include "bwp-manager-algorithm.h"

#include <ns3/log.h>
#include <ns3/uinteger.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BwpManagerGnb");
NS_OBJECT_ENSURE_REGISTERED (BwpManagerGnb);

void
BwpManagerGnb::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  RrComponentCarrierManager::DoInitialize ();
}

BwpManagerGnb::BwpManagerGnb () :
  RrComponentCarrierManager ()
{
  NS_LOG_FUNCTION (this);
  m_algorithm = new BwpManagerAlgorithmStatic (); // When we will have different types,
                                                  // Then we will add an Attribute.
}

BwpManagerGnb::~BwpManagerGnb ()
{
  NS_LOG_FUNCTION (this);
  delete m_algorithm;
}


TypeId
BwpManagerGnb::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BwpManagerGnb")
    .SetParent<NoOpComponentCarrierManager> ()
    .SetGroupName ("mmwave")
    .AddConstructor<BwpManagerGnb> ()
    ;
  return tid;
}

bool
BwpManagerGnb::IsGbr (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_ASSERT_MSG (m_rlcLcInstantiated.find (params.rnti) != m_rlcLcInstantiated.end (), "Trying to check the QoS of unknown UE");
  NS_ASSERT_MSG (m_rlcLcInstantiated.find (params.rnti)->second.find (params.lcid) != m_rlcLcInstantiated.find (params.rnti)->second.end (), "Trying to check the QoS of unknown logical channel");
  return m_rlcLcInstantiated.find (params.rnti)->second.find (params.lcid)->second.isGbr;
}

std::vector<LteCcmRrcSapProvider::LcsConfig>
BwpManagerGnb::DoSetupDataRadioBearer (EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this);

  std::vector<LteCcmRrcSapProvider::LcsConfig> lcsConfig = RrComponentCarrierManager::DoSetupDataRadioBearer (bearer, bearerId, rnti, lcid, lcGroup, msu);
  return lcsConfig;
}


void
BwpManagerGnb::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_rlcLcInstantiated.find (params.rnti) != m_rlcLcInstantiated.end (), "Unknown UE");
  NS_ASSERT_MSG (m_rlcLcInstantiated.find (params.rnti)->second.find (params.lcid) != m_rlcLcInstantiated.find (params.rnti)->second.end (), "Unknown logical channel of UE");

  uint8_t qci = m_rlcLcInstantiated.find (params.rnti)->second.find (params.lcid)->second.qci;

  // Force a conversion between the uint8_t type that comes from the LcInfo
  // struct (yeah, using the EpsBearer::Qci type was too hard ...)
  uint8_t bwpIndex = m_algorithm->GetBwpForEpsBearer (static_cast<EpsBearer::Qci> (qci));

  if (m_macSapProvidersMap.find (bwpIndex) != m_macSapProvidersMap.end ())
    {
      m_macSapProvidersMap.find (bwpIndex)->second->ReportBufferStatus (params);
    }
  else
    {
      NS_ABORT_MSG ("Bwp index not valid.");
    }
}


void
BwpManagerGnb::DoNotifyTxOpportunity (LteMacSapUser::TxOpportunityParameters txOpParams)
{
  NS_LOG_FUNCTION (this);
  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_ueAttached.find (txOpParams.rnti);
  NS_ASSERT_MSG (rntiIt != m_ueAttached.end (), "could not find RNTI" << txOpParams.rnti);

  std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (txOpParams.lcid);
  NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID " << (uint16_t) txOpParams.lcid);

  (*lcidIt).second->NotifyTxOpportunity (txOpParams);
}


void
BwpManagerGnb::DoUlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (bsr.m_macCeType == MacCeListElement_s::BSR, "Received a Control Message not allowed " << bsr.m_macCeType);

  NS_ASSERT_MSG (m_ccmMacSapProviderMap.find (componentCarrierId) != m_ccmMacSapProviderMap.end (), "Mac sap provider does not exist.");

  uint8_t qci = 9;

  if (m_rlcLcInstantiated.find (bsr.m_rnti) != m_rlcLcInstantiated.end ())
    {
      for (auto i: m_rlcLcInstantiated.find (bsr.m_rnti)->second)
        {
          // we do not consider first 3 lcids: signaling and default
          if (i.first > 3)
            {
              qci = i.second.qci;
              break;
            }
        }
    }

  uint8_t bwpIndex = m_algorithm->GetBwpForEpsBearer (static_cast<EpsBearer::Qci> (qci));

  NS_LOG_DEBUG ("Routing BSR for UE " << bsr.m_rnti << " to CC id " <<
                static_cast<uint32_t> (bwpIndex));

  if (m_ccmMacSapProviderMap.find (bwpIndex) != m_ccmMacSapProviderMap.end ())
    {
      m_ccmMacSapProviderMap.find (bwpIndex)->second->ReportMacCeToScheduler (bsr);
    }
  else
    {
      NS_ABORT_MSG ("Bwp index not valid.");
    }
}

void
BwpManagerGnb::DoUlReceiveSr(uint16_t rnti, uint8_t componentCarrierId)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (componentCarrierId);
  uint8_t qci = 9;

  if (m_rlcLcInstantiated.find (rnti) != m_rlcLcInstantiated.end ())
    {
      for (auto i: m_rlcLcInstantiated.find (rnti)->second)
        {
          // we do not consider first 3 lcids: signaling and default
          if (i.first > 3)
            {
              qci = i.second.qci;
              break;
            }
        }
    }

  uint8_t bwpIndex = m_algorithm->GetBwpForEpsBearer (static_cast<EpsBearer::Qci> (qci));

  NS_LOG_DEBUG ("Routing SR for UE " << rnti << " to CC id " <<
                static_cast<uint32_t> (bwpIndex));

  auto it = m_ccmMacSapProviderMap.find (bwpIndex);
  NS_ABORT_IF(it == m_ccmMacSapProviderMap.end ());

  m_ccmMacSapProviderMap.find (bwpIndex)->second->ReportSrToScheduler (rnti);
}


} // end of namespace ns3
