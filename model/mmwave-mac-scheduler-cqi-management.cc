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
#include "mmwave-mac-scheduler-cqi-management.h"
#include "mmwave-spectrum-value-helper.h"
#include "mmwave-amc.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveMacSchedulerCQIManagement");

void
MmWaveMacSchedulerCQIManagement::DlSBCQIReported (const DlCqiInfo &info,
                                                  const std::shared_ptr<MmWaveMacSchedulerUeInfo>&ueInfo) const
{
  NS_LOG_INFO (this);
  NS_UNUSED (info);
  NS_UNUSED (ueInfo);

  // TODO
}

void
MmWaveMacSchedulerCQIManagement::UlSBCQIReported (uint32_t expirationTime,
                                                  uint8_t numSym,
                                                  uint32_t tbs,
                                                  const MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params,
                                                  const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo) const
{
  NS_LOG_INFO (this);

  uint32_t frameNum = params.m_sfnSf.m_frameNum;
  uint32_t subframeNum = params.m_sfnSf.m_subframeNum;
  uint32_t slotNum = params.m_sfnSf.m_slotNum;
  uint32_t startSymIdx =  params.m_sfnSf.m_varTtiNum;

  ueInfo->m_ulCqi.m_sinr = params.m_ulCqi.m_sinr;
  ueInfo->m_ulCqi.m_cqiType = MmWaveMacSchedulerUeInfo::CqiInfo::SB;
  ueInfo->m_ulCqi.m_timer = expirationTime;

  uint32_t i = 0;
  for (double value : params.m_ulCqi.m_sinr)
    {
      NS_LOG_INFO ("UL CQI report for RNTI " << ueInfo->m_rnti <<
                   " SINR " << value <<
                   " in chunk " << i++ <<
                   " frame " << frameNum <<
                   " subframe " << subframeNum <<
                   " slot " << slotNum <<
                   " startSym " << startSymIdx);
    }


  SpectrumValue specVals (MmWaveSpectrumValueHelper::GetSpectrumModel (m_phyMacConfig));
  Values::iterator specIt = specVals.ValuesBegin ();
  for (uint32_t ichunk = 0; ichunk < m_phyMacConfig->GetBandwidthInRbs (); ichunk++)
    {
      NS_ASSERT (specIt != specVals.ValuesEnd ());
      *specIt = ueInfo->m_ulCqi.m_sinr.at (ichunk);   //sinrLin;
      specIt++;
    }

  // MCS updated inside the function; crappy API... but we can't fix everything
  ueInfo->m_ulCqi.m_cqi = m_amc->CreateCqiFeedbackWbTdma (specVals, numSym,
                                                          tbs, ueInfo->m_ulMcs);
}

void
MmWaveMacSchedulerCQIManagement::DlWBCQIReported (const DlCqiInfo &info,
                                                  const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo,
                                                  uint32_t expirationTime) const
{
  NS_LOG_INFO (this);

  ueInfo->m_dlCqi.m_cqiType = MmWaveMacSchedulerUeInfo::CqiInfo::WB;
  ueInfo->m_dlCqi.m_cqi = info.m_wbCqi;
  ueInfo->m_dlCqi.m_timer = expirationTime;
  ueInfo->m_dlMcs = static_cast<uint8_t> (m_amc->GetMcsFromCqi (ueInfo->m_dlCqi.m_cqi));
  NS_LOG_INFO ("Calculated MCS for UE " << static_cast<uint16_t> (ueInfo->m_rnti) <<
               " is " << static_cast<uint32_t> (ueInfo->m_dlMcs));

  NS_LOG_INFO ("Updated WB CQI of UE " << info.m_rnti << " to " <<
               static_cast<uint32_t> (info.m_wbCqi) << ". It will expire in " <<
               ueInfo->m_dlCqi.m_timer << " slots.");
}

void
MmWaveMacSchedulerCQIManagement::RefreshDlCqiMaps (const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &ueMap) const
{
  NS_LOG_FUNCTION (this);

  for (const auto &itUe : ueMap)
    {
      const std::shared_ptr<MmWaveMacSchedulerUeInfo>&ue = itUe.second;

      if (ue->m_dlCqi.m_timer == 0)
        {
          ue->m_dlCqi.m_cqi = 1; // lowest value for trying a transmission
          ue->m_dlCqi.m_cqiType = MmWaveMacSchedulerUeInfo::CqiInfo::WB;
          ue->m_dlMcs = m_startMcsDl;
        }
      else
        {
          ue->m_dlCqi.m_timer -= 1;
        }
    }
}

void
MmWaveMacSchedulerCQIManagement::RefreshUlCqiMaps (const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &ueMap) const
{
  NS_LOG_FUNCTION (this);

  for (const auto &itUe : ueMap)
    {
      const std::shared_ptr<MmWaveMacSchedulerUeInfo>&ue = itUe.second;

      if (ue->m_ulCqi.m_timer == 0)
        {
          ue->m_ulCqi.m_cqi = 1; // lowest value for trying a transmission
          ue->m_ulCqi.m_cqiType = MmWaveMacSchedulerUeInfo::CqiInfo::WB;
          ue->m_ulMcs = m_startMcsUl;
        }
      else
        {
          ue->m_ulCqi.m_timer -= 1;
        }
    }
}

} // namespace ns3
