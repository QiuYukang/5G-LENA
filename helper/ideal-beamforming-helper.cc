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

#include "ideal-beamforming-helper.h"
#include <ns3/log.h>
#include <ns3/mmwave-enb-net-device.h>
#include <ns3/mmwave-ue-net-device.h>
#include <ns3/mmwave-enb-phy.h>
#include <ns3/mmwave-ue-phy.h>
#include <ns3/beam-manager.h>
#include <ns3/vector.h>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("IdealBeamformingHelper");
NS_OBJECT_ENSURE_REGISTERED (IdealBeamformingHelper);


IdealBeamformingHelper::IdealBeamformingHelper ()
{
  // TODO Auto-generated constructor stub
  NS_LOG_FUNCTION (this);
}

IdealBeamformingHelper::~IdealBeamformingHelper ()
{
  // TODO Auto-generated destructor stub
  NS_LOG_FUNCTION (this);
}

TypeId
IdealBeamformingHelper::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::IdealBeamformingHelper")
      .SetParent<Object> ()
      .AddConstructor<IdealBeamformingHelper> ()
      .AddAttribute ("IdealBeamformingMethod",
                     "Type of the ideal beamforming method in the case that it is enabled, by default is \"cell scan\" method.",
                     TypeIdValue (CellScanBeamforming::GetTypeId ()),
                     MakeTypeIdAccessor (&IdealBeamformingHelper::SetIdealBeamformingMethod),
                     MakeTypeIdChecker ())
      .AddAttribute ("BeamformingPeriodicity",
                     "Interval between consecutive beamforming method executions.",
                      TimeValue (MilliSeconds (100)),
                      MakeTimeAccessor (&IdealBeamformingHelper::m_beamformingPeriodicity),
                      MakeTimeChecker())
      ;
    return tid;
}

void
IdealBeamformingHelper::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  ObjectFactory objectFactory = ObjectFactory ();
  objectFactory.SetTypeId (m_idealBeamformingAlgorithmType);
  m_idealBeamformingAlgorithm = objectFactory.Create<IdealBeamformingAlgorithm>();
  ExpireBeamformingTimer ();
}

void
IdealBeamformingHelper::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}

void
IdealBeamformingHelper::AddBeamformingTask (const Ptr<MmWaveEnbNetDevice>& gNbDev,
                                            const Ptr<MmWaveUeNetDevice>& ueDev)
{
  NS_LOG_FUNCTION (this);
  m_beamformingTasks.push_back(std::make_pair(gNbDev, ueDev));
}

void
IdealBeamformingHelper::Run () const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Running the ideal beamforming method. There are :" <<m_beamformingTasks.size()<<" tasks.");

  for (const auto& task:m_beamformingTasks)
    {
      NS_LOG_INFO ("Running the ideal beamforming method. There are :" <<task.first->GetCcMapSize()<<" antennas per device.");

      for (uint8_t ccId = 0; ccId < task.first->GetCcMapSize() ; ccId++)
        {
           Ptr<MmWaveEnbNetDevice> gNbDev = task.first;
           Ptr<MmWaveUeNetDevice> ueDev = task.second;

           BeamformingVector gnbBfv, ueBfv;
           m_idealBeamformingAlgorithm->GetBeamformingVectors (gNbDev, ueDev, &gnbBfv, &ueBfv);
           Ptr<MmWaveEnbPhy> gNbPhy = gNbDev->GetPhy (ccId);
           Ptr<MmWaveUePhy> uePhy = ueDev->GetPhy (ccId);

           NS_ABORT_IF (gNbPhy == nullptr || uePhy == nullptr);

           gNbPhy->GetBeamManager()->SaveBeamformingVector (gnbBfv, ueDev);
           uePhy->GetBeamManager()->SaveBeamformingVector (ueBfv, gNbDev);
           uePhy->GetBeamManager()->ChangeBeamformingVector (gNbDev);
        }
    }
}

void
IdealBeamformingHelper::SetIdealBeamformingMethod (TypeId beamformingMethod)
{
  NS_LOG_FUNCTION (this);
  m_idealBeamformingAlgorithmType = beamformingMethod;
}

void
IdealBeamformingHelper::ExpireBeamformingTimer()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Beamforming timer expired; programming a beamforming");

  Run (); //Run beamforming tasks

  m_beamformingTimer = Simulator::Schedule (m_beamformingPeriodicity,
                                            &IdealBeamformingHelper::ExpireBeamformingTimer, this);
}

}
