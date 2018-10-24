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
#include "mmwave-mac-scheduler-tdma-rr.h"
#include "mmwave-mac-scheduler-ue-info-rr.h"
#include <ns3/log.h>
#include <algorithm>
#include <functional>

namespace ns3  {

NS_LOG_COMPONENT_DEFINE ("MmWaveMacSchedulerTdmaRR");
NS_OBJECT_ENSURE_REGISTERED (MmWaveMacSchedulerTdmaRR);

TypeId
MmWaveMacSchedulerTdmaRR::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacSchedulerTdmaRR")
    .SetParent<MmWaveMacSchedulerTdma> ()
    .AddConstructor<MmWaveMacSchedulerTdmaRR> ()
  ;
  return tid;
}

MmWaveMacSchedulerTdmaRR::MmWaveMacSchedulerTdmaRR ()
  : MmWaveMacSchedulerTdma ()
{
  NS_LOG_FUNCTION (this);
}

std::shared_ptr<MmWaveMacSchedulerUeInfo>
MmWaveMacSchedulerTdmaRR::CreateUeRepresentation (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters &params) const
{
  NS_LOG_FUNCTION (this);
  return std::make_shared <MmWaveMacSchedulerUeInfoRR> (params.m_rnti, params.m_beamId);
}

void
MmWaveMacSchedulerTdmaRR::AssignedDlResources (const UePtrAndBufferReq &ue,
                                               const FTResources &assigned,
                                               const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (assigned);
  NS_UNUSED (totAssigned);
  GetFirst GetUe;
  GetUe (ue)->UpdateDlMetric (m_phyMacConfig, m_amc);
}

void
MmWaveMacSchedulerTdmaRR::AssignedUlResources (const UePtrAndBufferReq &ue,
                                               const FTResources &assigned,
                                               const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (assigned);
  NS_UNUSED (totAssigned);
  GetFirst GetUe;
  GetUe (ue)->UpdateUlMetric (m_phyMacConfig, m_amc);
}

std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
MmWaveMacSchedulerTdmaRR::GetUeCompareDlFn () const
{
  return MmWaveMacSchedulerUeInfoRR::CompareUeWeightsDl;
}

std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
MmWaveMacSchedulerTdmaRR::GetUeCompareUlFn () const
{
  return MmWaveMacSchedulerUeInfoRR::CompareUeWeightsUl;
}

} //namespace ns3
