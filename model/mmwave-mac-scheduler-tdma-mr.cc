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
#include "mmwave-mac-scheduler-tdma-mr.h"
#include "mmwave-mac-scheduler-ue-info-mr.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("MmWaveMacSchedulerTdmaMR");
NS_OBJECT_ENSURE_REGISTERED (MmWaveMacSchedulerTdmaMR);

TypeId
MmWaveMacSchedulerTdmaMR::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacSchedulerTdmaMR")
    .SetParent<MmWaveMacSchedulerTdmaRR> ()
    .AddConstructor<MmWaveMacSchedulerTdmaMR> ()
  ;
  return tid;
}

MmWaveMacSchedulerTdmaMR::MmWaveMacSchedulerTdmaMR ()
  : MmWaveMacSchedulerTdmaRR ()
{
}

std::shared_ptr<MmWaveMacSchedulerUeInfo>
MmWaveMacSchedulerTdmaMR::CreateUeRepresentation (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters &params) const
{
  NS_LOG_FUNCTION (this);
  return std::make_shared <MmWaveMacSchedulerUeInfoMR> (params.m_rnti, params.m_beamId);
}

std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
MmWaveMacSchedulerTdmaMR::GetUeCompareDlFn () const
{
  return MmWaveMacSchedulerUeInfoMR::CompareUeWeightsDl;
}

std::function<bool(const MmWaveMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const MmWaveMacSchedulerNs3::UePtrAndBufferReq &rhs )>
MmWaveMacSchedulerTdmaMR::GetUeCompareUlFn () const
{
  return MmWaveMacSchedulerUeInfoMR::CompareUeWeightsUl;
}

} // namespace ns3
