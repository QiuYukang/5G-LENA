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
#include "mmwave-mac-scheduler-tdma-pf.h"
#include "mmwave-mac-scheduler-ue-info-pf.h"
#include <ns3/log.h>
#include <ns3/double.h>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveMacSchedulerTdmaPF");
NS_OBJECT_ENSURE_REGISTERED (MmWaveMacSchedulerTdmaPF);

TypeId
MmWaveMacSchedulerTdmaPF::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacSchedulerTdmaPF")
    .SetParent<MmWaveMacSchedulerTdmaRR> ()
    .AddConstructor<MmWaveMacSchedulerTdmaPF> ()
    .AddAttribute ("FairnessIndex",
                   "Value (between 0 and 1) that defines the PF metric (1 is the traditional 3GPP PF, 0 is RR in throughput",
                   DoubleValue (0),
                   MakeDoubleAccessor (&MmWaveMacSchedulerTdmaPF::m_alpha),
                   MakeDoubleChecker<float> (0, 1))
    .AddAttribute ("LastAvgTPutWeight",
                   "Weight of the last average throughput in the average throughput calculation",
                   DoubleValue (99),
                   MakeDoubleAccessor (&MmWaveMacSchedulerTdmaPF::m_timeWindow),
                   MakeDoubleChecker<float> (0))
  ;
  return tid;
}

MmWaveMacSchedulerTdmaPF::MmWaveMacSchedulerTdmaPF () : MmWaveMacSchedulerTdmaRR ()
{
  NS_LOG_FUNCTION (this);
}

std::shared_ptr<MmWaveMacSchedulerUeInfo>
MmWaveMacSchedulerTdmaPF::CreateUeRepresentation (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters &params) const
{
  NS_LOG_FUNCTION (this);
  return std::make_shared <MmWaveMacSchedulerUeInfoPF> (m_alpha, params.m_rnti, params.m_beamId);
}

std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
MmWaveMacSchedulerTdmaPF::GetUeCompareDlFn () const
{
  return MmWaveMacSchedulerUeInfoPF::CompareUeWeightsDl;
}

std::function<bool (const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                    const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs)>
MmWaveMacSchedulerTdmaPF::GetUeCompareUlFn () const
{
  return MmWaveMacSchedulerUeInfoRR::CompareUeWeightsUl;
}

void
MmWaveMacSchedulerTdmaPF::AssignedDlResources (const UePtrAndBufferReq &ue,
                                               const FTResources &assigned,
                                               const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (assigned);
  auto uePtr = std::dynamic_pointer_cast<MmWaveMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateDlPFMetric (totAssigned, m_timeWindow, m_phyMacConfig, m_amc);
}

void
MmWaveMacSchedulerTdmaPF::NotAssignedDlResources (const MmWaveMacSchedulerNs3::UePtrAndBufferReq &ue,
                                                  const MmWaveMacSchedulerNs3::FTResources &notAssigned,
                                                  const MmWaveMacSchedulerNs3::FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (notAssigned);
  auto uePtr = std::dynamic_pointer_cast<MmWaveMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateDlPFMetric (totAssigned, m_timeWindow, m_phyMacConfig, m_amc);
}

void
MmWaveMacSchedulerTdmaPF::BeforeDlSched (const UePtrAndBufferReq &ue,
                                         const FTResources &assignableInIteration) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<MmWaveMacSchedulerUeInfoPF> (ue.first);
  uePtr->CalculatePotentialTPut (assignableInIteration, m_phyMacConfig, m_amc);
}

} //namespace ns3
