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

#include "bwp-manager.h"
#include <ns3/log.h>
#include <ns3/uinteger.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BwpManager");
NS_OBJECT_ENSURE_REGISTERED (BwpManager);

void
BwpManager::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  RrComponentCarrierManager::DoInitialize ();
}

BwpManager::BwpManager () :
  RrComponentCarrierManager ()
{
  NS_LOG_FUNCTION (this);
}

BwpManager::~BwpManager ()
{
  NS_LOG_FUNCTION (this);
}

#define BWP_MANAGER_DECLARE_ATTR(NAME,DESC,SETTER)                         \
  .AddAttribute (NAME,                                                     \
                 DESC,                                                     \
                 UintegerValue (0),                                        \
                 MakeUintegerAccessor (&BwpManager::SETTER),               \
                 MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))

TypeId
BwpManager::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BwpManager")
    .SetParent<NoOpComponentCarrierManager> ()
    .SetGroupName ("mmwave")
    .AddConstructor<BwpManager> ()
    BWP_MANAGER_DECLARE_ATTR("GBR_CONV_VOICE",
                             "BWP index to which flows of this Qci type should be forwarded.",
                             SetConvVoiceBwp)
    BWP_MANAGER_DECLARE_ATTR("GBR_CONV_VIDEO",
                             "BWP index to which flows of GBR_CONV_VIDEO Qci type should be forwarded.",
                             SetConvVideoBwp)
    BWP_MANAGER_DECLARE_ATTR("GBR_GAMING",
                             "BWP index to which flows of GBR_GAMING Qci type should be forwarded.",
                             SetGamingBwp)
    BWP_MANAGER_DECLARE_ATTR("GBR_NON_CONV_VIDEO",
                             "BWP index to which flows of GBR_NON_CONV_VIDEO Qci type should be forwarded.",
                             SetNonConvVideoBwp)
    BWP_MANAGER_DECLARE_ATTR("GBR_MC_PUSH_TO_TALK",
                             "BWP index to which flows of GBR_MC_PUSH_TO_TALK Qci type should be forwarded.",
                             SetMcPttBwp)
    BWP_MANAGER_DECLARE_ATTR("GBR_NMC_PUSH_TO_TALK",
                             "BWP index to which flows of GBR_NMC_PUSH_TO_TALK Qci type should be forwarded.",
                             SetNmcPttBwp)
    BWP_MANAGER_DECLARE_ATTR("GBR_MC_VIDEO",
                             "BWP index to which flows of GBR_MC_VIDEO Qci type should be forwarded.",
                             SetMcVideoBwp)
    BWP_MANAGER_DECLARE_ATTR("GBR_V2X",
                             "BWP index to which flows of GBR_V2X Qci type should be forwarded.",
                             SetGbrV2xBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_IMS",
                             "BWP index to which flows of NGBR_IMS Qci type should be forwarded.",
                             SetImsBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_VIDEO_TCP_OPERATOR",
                             "BWP index to which flows of NGBR_VIDEO_TCP_OPERATOR Qci type should be forwarded.",
                             SetVideoTcpOpBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_VOICE_VIDEO_GAMING",
                             "BWP index to which flows of NGBR_VOICE_VIDEO_GAMING Qci type should be forwarded.",
                             SetVideoGamingBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_VIDEO_TCP_PREMIUM",
                             "BWP index to which flows of NGBR_VIDEO_TCP_PREMIUM Qci type should be forwarded.",
                             SetVideoTcpPremiumBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_VIDEO_TCP_DEFAULT",
                             "BWP index to which flows of NGBR_VIDEO_TCP_DEFAULT Qci type should be forwarded.",
                             SetVideoTcpDefaultBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_MC_DELAY_SIGNAL",
                             "BWP index to which flows of NGBR_MC_DELAY_SIGNAL Qci type should be forwarded.",
                             SetMcDelaySignalBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_MC_DATA",
                             "BWP index to which flows of NGBR_MC_DATA Qci type should be forwarded.",
                             SetMcDataBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_V2X",
                             "BWP index to which flows of NGBR_V2X Qci type should be forwarded.",
                             SetNgbrV2xBwp)
    BWP_MANAGER_DECLARE_ATTR("NGBR_LOW_LAT_EMBB",
                             "BWP index to which flows of NGBR_LOW_LAT_EMBB Qci type should be forwarded.",
                             SetLowLatEmbbBwp)
    BWP_MANAGER_DECLARE_ATTR("DGBR_DISCRETE_AUT_SMALL",
                             "BWP index to which flows of DGBR_DISCRETE_AUT_SMALL Qci type should be forwarded.",
                             SetDiscreteAutSmallBwp)
    BWP_MANAGER_DECLARE_ATTR("DGBR_DISCRETE_AUT_LARGE",
                             "BWP index to which flows of DGBR_DISCRETE_AUT_LARGE Qci type should be forwarded.",
                             SetDiscreteAutLargeBwp)
    BWP_MANAGER_DECLARE_ATTR("DGBR_ITS",
                             "BWP index to which flows of DGBR_ITS Qci type should be forwarded.",
                             SetItsBwp)
    BWP_MANAGER_DECLARE_ATTR("DGBR_ELECTRICITY",
                             "BWP index to which flows of DGBR_ELECTRICITY Qci type should be forwarded.",
                             SetElectricityBwp)
    ;
  return tid;
}

bool
BwpManager::IsGbr (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_ASSERT_MSG (m_rlcLcInstantiated.find (params.rnti) != m_rlcLcInstantiated.end (), "Trying to check the QoS of unknown UE");
  NS_ASSERT_MSG (m_rlcLcInstantiated.find (params.rnti)->second.find (params.lcid) != m_rlcLcInstantiated.find (params.rnti)->second.end (), "Trying to check the QoS of unknown logical channel");
  return m_rlcLcInstantiated.find (params.rnti)->second.find (params.lcid)->second.isGbr;
}

std::vector<LteCcmRrcSapProvider::LcsConfig>
BwpManager::DoSetupDataRadioBearer (EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this);

  std::vector<LteCcmRrcSapProvider::LcsConfig> lcsConfig = RrComponentCarrierManager::DoSetupDataRadioBearer (bearer, bearerId, rnti, lcid, lcGroup, msu);
  return lcsConfig;
}


void
BwpManager::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_rlcLcInstantiated.find (params.rnti) != m_rlcLcInstantiated.end (), "Unknown UE");
  NS_ASSERT_MSG (m_rlcLcInstantiated.find (params.rnti)->second.find (params.lcid) != m_rlcLcInstantiated.find (params.rnti)->second.end (), "Unknown logical channel of UE");

  uint8_t qci = m_rlcLcInstantiated.find (params.rnti)->second.find (params.lcid)->second.qci;

  uint8_t bwpIndex = 0;

  if (m_qciToBwpMap.find (qci) != m_qciToBwpMap.end ())
    {
      bwpIndex = m_qciToBwpMap.at (qci);
    }

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
BwpManager::DoNotifyTxOpportunity (LteMacSapUser::TxOpportunityParameters txOpParams)
{
  NS_LOG_FUNCTION (this);
  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_ueAttached.find (txOpParams.rnti);
  NS_ASSERT_MSG (rntiIt != m_ueAttached.end (), "could not find RNTI" << txOpParams.rnti);

  std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (txOpParams.lcid);
  NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID " << (uint16_t) txOpParams.lcid);

  (*lcidIt).second->NotifyTxOpportunity (txOpParams);
}


void
BwpManager::DoUlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId)
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

  uint8_t bwpIndex = 0;

  if (m_qciToBwpMap.find (qci) != m_qciToBwpMap.end ())
    {
      bwpIndex = m_qciToBwpMap.at (qci);
    }

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
BwpManager::DoUlReceiveSr(uint16_t rnti, uint8_t componentCarrierId)
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

  uint8_t bwpIndex = 0;

  if (m_qciToBwpMap.find (qci) != m_qciToBwpMap.end ())
    {
      bwpIndex = m_qciToBwpMap.at (qci);
    }

  NS_LOG_DEBUG ("Routing SR for UE " << rnti << " to CC id " <<
                static_cast<uint32_t> (bwpIndex));

  auto it = m_ccmMacSapProviderMap.find (bwpIndex);
  NS_ABORT_IF(it == m_ccmMacSapProviderMap.end ());

  m_ccmMacSapProviderMap.find (bwpIndex)->second->ReportSrToScheduler (rnti);
}


} // end of namespace ns3
