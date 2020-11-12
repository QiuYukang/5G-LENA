/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "realistic-beamforming-helper.h"
#include <ns3/log.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-gnb-phy.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/beam-manager.h>
#include <ns3/vector.h>
#include <ns3/uinteger.h>
#include <ns3/lte-ue-rrc.h>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("RealisticBeamformingHelper");
NS_OBJECT_ENSURE_REGISTERED (RealisticBeamformingHelper);


RealisticBeamformingHelper::RealisticBeamformingHelper ()
{
  // TODO Auto-generated constructor stub
  NS_LOG_FUNCTION (this);
}

RealisticBeamformingHelper::~RealisticBeamformingHelper ()
{
  // TODO Auto-generated destructor stub
  NS_LOG_FUNCTION (this);
}

TypeId
RealisticBeamformingHelper::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::RealisticBeamformingHelper")
      .SetParent<IdealBeamformingHelper> ()
      .AddConstructor<RealisticBeamformingHelper> ()
      .AddAttribute ("TriggerEvent",
                     "Defines a beamforming trigger event",
                      EnumValue (RealisticBeamformingHelper::SRS_COUNT),
                      MakeEnumAccessor (&RealisticBeamformingHelper::SetTriggerEvent,
                                        &RealisticBeamformingHelper::GetTriggerEvent),
                      MakeEnumChecker (RealisticBeamformingHelper::SRS_COUNT, "SrsCount",
                                       RealisticBeamformingHelper::DELAYED_UPDATE, "DelayedUpdate"))
      .AddAttribute ("SrsCountPeriodicity",
                     "Interval between consecutive beamforming update method executions expressed in the number of SRS SINR reports"
                     "to wait before triggering the next beamforming update method execution.",
                      UintegerValue (1),
                      MakeUintegerAccessor (&RealisticBeamformingHelper::SetSrsCountPeriodicity,
                                            &RealisticBeamformingHelper::GetSrsCountPeriodicity),
                      MakeUintegerChecker <uint16_t>())
      .AddAttribute ("SrsToBeamformingDelay",
                     "Delay between SRS SINR report and the beamforming vectors update. ",
                      TimeValue (MilliSeconds (10)),
                      MakeTimeAccessor (&RealisticBeamformingHelper::SetSrsToBeamformingDelay,
                                        &RealisticBeamformingHelper::GetSrsToBeamformingDelay),
                      MakeTimeChecker());
    return tid;
}

void
RealisticBeamformingHelper::DoInitialize ()
{

}

void
RealisticBeamformingHelper::SetTriggerEvent (RealisticBeamformingHelper::TriggerEvent triggerEvent)
{
  m_triggerEvent = triggerEvent;
}

RealisticBeamformingHelper::TriggerEvent
RealisticBeamformingHelper::GetTriggerEvent () const
{
  return m_triggerEvent;
}

void
RealisticBeamformingHelper::SetSrsCountPeriodicity (uint16_t periodicity)
{
  m_srsSinrPeriodicity = periodicity;
}

uint16_t
RealisticBeamformingHelper::GetSrsCountPeriodicity () const
{
  return m_srsSinrPeriodicity;
}

void
RealisticBeamformingHelper::SetSrsToBeamformingDelay (Time delay)
{
  m_srsToBeamformingDelay = delay;
}

Time
RealisticBeamformingHelper::GetSrsToBeamformingDelay () const
{
  return m_srsToBeamformingDelay;
}

void
RealisticBeamformingHelper::AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                                const Ptr<NrUeNetDevice>& ueDev)
{
  NS_LOG_FUNCTION (this);
  m_beamformingTasks.push_back (std::make_pair (gNbDev, ueDev));

  for (uint8_t ccId = 0; ccId < gNbDev->GetCcMapSize () ; ccId++)
    {
      gNbDev->GetPhy (ccId)->GetSpectrumPhy()->SetSrsSinrReportCallback (MakeCallback (&RealisticBeamformingHelper::SaveSrsSinrReport, this));
      auto cellId = gNbDev->GetPhy (ccId)->GetCellId ();
      if (m_srsSinrReportsListsPerCellId.find (cellId) == m_srsSinrReportsListsPerCellId.end())
        {
          m_srsSinrReportsListsPerCellId [cellId] = SrsReports ();
        }
    }
}

void
RealisticBeamformingHelper::Run () const
{
  NS_FATAL_ERROR ("Run function should not be called when RealisticBeamforming is being used.");
  // run function is used to run all beamforming tasks (updates) at the same time, this is different
  // from realistic beamforming behaviour in which we will not have updates at the same time,
  // instead each beamforming task will be triggered based on its own event (SRS count or delay)
}

void
RealisticBeamformingHelper::ExpireBeamformingTimer ()
{
  NS_FATAL_ERROR ("ExpireBeamformingTimer function should not be called when RealisticBeamforming is being used.");
}

void
RealisticBeamformingHelper::SaveSrsSinrReport (uint16_t cellId, uint16_t rnti, double srsSinr)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_beamformingAlgorithm, "Beamforming algorithm not set");

  GetSecond srsList;
  GetSecond srs;
  auto itCell = m_srsSinrReportsListsPerCellId.find (cellId);
  auto itRnti = srsList (*itCell).find (rnti);

  NS_ASSERT_MSG (itCell != m_srsSinrReportsListsPerCellId.end (),
                 "SRS report map for this cellId not initialized in AddBeaformingTask");

  if (itRnti != srsList (*itCell).end ())
    {
      srs (*itRnti).time = Simulator::Now ();
      srs (*itRnti).srsSinr = srsSinr;

      if (m_triggerEvent == SRS_COUNT)
        {
          srs (*itRnti).counter++;
        }
    }
  else
    {
      srsList (*itCell)[rnti] = SrsSinrReport (Simulator::Now (), srsSinr, 0);
    }

  switch (m_triggerEvent)
  {
    case SRS_COUNT:
      {
        if (srs (*itRnti).counter == m_srsSinrPeriodicity)
          {
            srs (*itRnti).counter = 0;
            TriggerBeamformingAlgorithm (cellId, rnti, srsSinr);
          }
        break;
      }
    case DELAYED_UPDATE:
      {
        Simulator::Schedule (GetSrsToBeamformingDelay (),
                             &RealisticBeamformingHelper::TriggerBeamformingAlgorithm, this,
                             cellId, rnti, srsSinr);
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("Unknown trigger event type.");
      }
  }
}

void
RealisticBeamformingHelper::TriggerBeamformingAlgorithm (uint16_t cellId, uint16_t rnti, double srsSinr)
{
  NS_LOG_FUNCTION (this);
  Ptr<RealisticBeamformingAlgorithm> realBeamforming = DynamicCast <RealisticBeamformingAlgorithm> (m_beamformingAlgorithm);
  NS_ASSERT_MSG (m_beamformingAlgorithm, "Beamforming algorithm not initialized yet or is of the wrong type. Should be RealisticBeamformingAlgorithm." );

  for (const auto& task:m_beamformingTasks)
    {
      for (uint8_t ccId = 0; ccId < task.first->GetCcMapSize () ; ccId++)
        {
          auto currentCellId = task.first->GetPhy (ccId)->GetCellId ();
          auto currentRnti = task.second->GetRrc ()->GetRnti ();

          if (currentCellId == cellId && currentRnti == rnti)
            {
              realBeamforming->SetSrsSinr (srsSinr);
              RunTask (task.first, task.second, ccId);
              return;
            }
        }
    }

  NS_FATAL_ERROR ("Beamforming task not found for cellId and rnti: "<< cellId << "," << rnti);
}


}
