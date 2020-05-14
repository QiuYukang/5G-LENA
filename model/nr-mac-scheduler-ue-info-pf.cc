/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "nr-mac-scheduler-ue-info-pf.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerUeInfoPF");

/**
 * \brief Update PF metrics
 * \param timeWindow Time window to consider
 * \param config Config
 * \param amc AMC
 *
 * Updates m_currTputDl and m_avgTputDl by keeping in consideration
 * the assigned resources (in form of TBS) and the time window.
 */
void
NrMacSchedulerUeInfoPF::UpdateDlPFMetric (const NrMacSchedulerNs3::FTResources &totAssigned,
                                              double timeWindow,
                                              const Ptr<const NrAmc> &amc)
{
  NS_LOG_FUNCTION (this);
  NrMacSchedulerUeInfo::UpdateDlMetric (amc);

  m_currTputDl = static_cast<double> (m_dlTbSize) / (totAssigned.m_sym);
  m_avgTputDl = ((1.0 - (1.0 / static_cast<double> (timeWindow))) * m_lastAvgTputDl) +
    ((1.0 / timeWindow) * m_currTputDl);
  NS_LOG_DEBUG ("Update PF Metric for UE " << static_cast<uint32_t> (m_rnti) <<
                " TBS: " << m_dlTbSize << " Updated currTput " <<
                m_currTputDl << " avgTput " << m_avgTputDl << " over n. of syms: " <<
                +totAssigned.m_sym <<  ", last Avg TH " <<
                m_lastAvgTputDl << " total sym assigned " << static_cast<uint32_t> (totAssigned.m_sym) <<
                " updated metric: " << m_potentialTput / std::max (1E-9, m_avgTputDl));
}

void
NrMacSchedulerUeInfoPF::CalculatePotentialTPut (const NrMacSchedulerNs3::FTResources &assignableInIteration,
                                                    const Ptr<const NrAmc> &amc)
{
  NS_LOG_FUNCTION (this);
  uint32_t rbsAssignable = assignableInIteration.m_rbg * GetNumRbPerRbg ();
  m_potentialTput =  amc->CalculateTbSize (m_dlMcs, rbsAssignable);
  m_potentialTput /= assignableInIteration.m_sym;
  NS_LOG_INFO ("UE " << m_rnti << " potentialTput " << m_potentialTput <<
               " lastAvgTh " << m_lastAvgTputDl << " metric: " << m_potentialTput / std::max (1E-9, m_avgTputDl));
}

} // namespace ns3
