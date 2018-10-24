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

#include <ns3/log.h>
#include <ns3/random-variable-stream.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include "bwp-manager.h"
#include "ns3/trace-helper.h"
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
  m_gbr_conv_voice_bwp (0),
  m_gbr_conv_video_bwp (0),
  m_gbr_gaming_bwp (0),
  m_gbr_non_conv_video_bwp (0),
  m_ngbr_ims_bwp (0),
  m_ngbr_video_tcp_operator_bwp (0),
  m_ngbr_voice_video_gaming_bwp (0),
  m_ngbr_video_tcp_premium_bwp (0),
  m_ngbr_video_tcp_default_bwp (0),
  m_gbr_ultra_low_lat_bwp (0)
{
  NS_LOG_FUNCTION (this);
}

BwpManager::~BwpManager ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
BwpManager::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BwpManager")
    .SetParent<NoOpComponentCarrierManager> ()
    .SetGroupName ("mmwave")
    .AddConstructor<BwpManager> ()
    .AddAttribute ("GBR_CONV_VOICE",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_gbr_conv_voice_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("GBR_CONV_VIDEO",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_gbr_conv_video_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("GBR_GAMING",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci ype.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_gbr_gaming_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("GBR_NON_CONV_VIDEO",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_gbr_non_conv_video_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("NGBR_IMS",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_ngbr_ims_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("NGBR_VIDEO_TCP_OPERATOR",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_ngbr_video_tcp_operator_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("NGBR_VOICE_VIDEO_GAMING",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_ngbr_voice_video_gaming_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("NGBR_VIDEO_TCP_PREMIUM",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_ngbr_video_tcp_premium_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("NGBR_VIDEO_TCP_DEFAULT",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_ngbr_video_tcp_default_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC))
    .AddAttribute ("GBR_ULTRA_LOW_LAT",
                   "Defines the index of BWP to which it should be forwarded the flow of this Qci type.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&BwpManager::m_gbr_ultra_low_lat_bwp),
                   MakeUintegerChecker<uint8_t> (0, MAX_NO_CC));
  return tid;
}

bool BwpManager::IsGbr (LteMacSapProvider::ReportBufferStatusParameters params)
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
  switch (qci)
    {
    case ns3::EpsBearer::GBR_CONV_VOICE:
      bwpIndex = m_gbr_conv_voice_bwp;
      break;
    case ns3::EpsBearer::GBR_CONV_VIDEO:
      bwpIndex = m_gbr_conv_video_bwp;
      break;
    case ns3::EpsBearer::GBR_GAMING:
      bwpIndex = m_gbr_gaming_bwp;
      break;
    case ns3::EpsBearer::GBR_NON_CONV_VIDEO:
      bwpIndex = m_gbr_non_conv_video_bwp;
      break;
    case ns3::EpsBearer::NGBR_IMS:
      bwpIndex = m_ngbr_ims_bwp;
      break;
    case ns3::EpsBearer::NGBR_VIDEO_TCP_OPERATOR:
      bwpIndex = m_ngbr_video_tcp_operator_bwp;
      break;
    case ns3::EpsBearer::NGBR_VOICE_VIDEO_GAMING:
      bwpIndex = m_ngbr_voice_video_gaming_bwp;
      break;
    case ns3::EpsBearer::NGBR_VIDEO_TCP_PREMIUM:
      bwpIndex = m_ngbr_video_tcp_premium_bwp;
      break;
    case ns3::EpsBearer::NGBR_VIDEO_TCP_DEFAULT:
      bwpIndex = m_ngbr_video_tcp_default_bwp;
      break;
    case ns3::EpsBearer::GBR_ULTRA_LOW_LAT:
      bwpIndex = m_gbr_ultra_low_lat_bwp;
      break;
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
BwpManager::DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this);
  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_ueAttached.find (rnti);
  NS_ASSERT_MSG (rntiIt != m_ueAttached.end (), "could not find RNTI" << rnti);

  std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (lcid);
  NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID " << (uint16_t) lcid);

  NS_LOG_DEBUG (this << " rnti= " << rnti << " lcid= " << (uint32_t) lcid << " layer= " << (uint32_t)layer << " ccId=" << (uint32_t)componentCarrierId);
  (*lcidIt).second->NotifyTxOpportunity (bytes, layer, harqId, componentCarrierId, rnti, lcid);

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
              if (i.second.qci  < qci)
                {
                  qci = i.second.qci;
                }
              else if (i.second.qci == ns3::EpsBearer::GBR_ULTRA_LOW_LAT)
                {
                  qci = i.second.qci;
                  break;
                }
            }
        }
    }

  uint8_t bwpIndex = 0;
  switch (qci)
    {
    case ns3::EpsBearer::GBR_CONV_VOICE:
      bwpIndex = m_gbr_conv_voice_bwp;
      break;
    case ns3::EpsBearer::GBR_CONV_VIDEO:
      bwpIndex = m_gbr_conv_video_bwp;
      break;
    case ns3::EpsBearer::GBR_GAMING:
      bwpIndex = m_gbr_gaming_bwp;
      break;
    case ns3::EpsBearer::GBR_NON_CONV_VIDEO:
      bwpIndex = m_gbr_non_conv_video_bwp;
      break;
    case ns3::EpsBearer::NGBR_IMS:
      bwpIndex = m_ngbr_ims_bwp;
      break;
    case ns3::EpsBearer::NGBR_VIDEO_TCP_OPERATOR:
      bwpIndex = m_ngbr_video_tcp_operator_bwp;
      break;
    case ns3::EpsBearer::NGBR_VOICE_VIDEO_GAMING:
      bwpIndex = m_ngbr_voice_video_gaming_bwp;
      break;
    case ns3::EpsBearer::NGBR_VIDEO_TCP_PREMIUM:
      bwpIndex = m_ngbr_video_tcp_premium_bwp;
      break;
    case ns3::EpsBearer::NGBR_VIDEO_TCP_DEFAULT:
      bwpIndex = m_ngbr_video_tcp_default_bwp;
      break;
    case ns3::EpsBearer::GBR_ULTRA_LOW_LAT:
      bwpIndex = m_gbr_ultra_low_lat_bwp;
      break;
    }

  if (m_ccmMacSapProviderMap.find (bwpIndex) != m_ccmMacSapProviderMap.end ())
    {
      m_ccmMacSapProviderMap.find (bwpIndex)->second->ReportMacCeToScheduler (bsr);
    }
  else
    {
      NS_ABORT_MSG ("Bwp index not valid.");
    }


}


} // end of namespace ns3
