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


TypeId
RealisticBeamformingHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RealisticBeamformingHelper")
                      .SetParent<BeamformingHelperBase> ()
                      .AddConstructor<RealisticBeamformingHelper> ()
  return tid;
}



void
RealisticBeamformingHelper::AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                                const Ptr<NrUeNetDevice>& ueDev)
{
  NS_LOG_FUNCTION (this);

  auto itAlgorithms = m_devicePairToAlgorithmsPerCcId.find(std::make_pair(gNbDev, ueDev));
  NS_ABORT_IF_MSG ( itAlgorithms != m_devicePairToAlgorithmsPerCcId.end(), "Realistic beamforming task already created for the provided devices");

  // create new element in the map that
  itAlgorithms = (m_devicePairToAlgorithmsPerCcId [std::make_pair(gNbDev, ueDev)] = CcIdToBeamformingAlgorithm ());

  BeamformingHelperBase ::AddBeamformingTask (gNbDev, ueDev);

  for (uint8_t ccId = 0; ccId < gNbDev->GetCcMapSize () ; ccId++)
    {
      Ptr<RealisticBeamformingAlgorithm> beamformingAlgorithm = m_algorithmFactory.Create<RealisticBeamformingAlgorithm> ();
      GetSecond (itAlgorithms) [ccId] = beamformingAlgorithm;
      //connect trace of the corresponding gNB PHY to the RealisticBeamformingAlgorithm funcition
      gNbDev->GetPhy (ccId)->GetSpectrumPhy()->SetSrsSinrReportCallback (MakeCallback (&RealisticBeamformingAlgorithm::SaveSrsSinrReport, beamformingAlgorithm));
      beamformingAltorithm->SetTriggerCallback (MakeCallback (&RealisticBeamformingHelper::RunTask, this));
    }
}


void
RealisticBeamformingHelper::GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                                   const Ptr<const NrUeNetDevice>& ueDev,
                                                   BeamformingVector* gnbBfv,
                                                   BeamformingVector* ueBfv,
                                                   uint16_t ccId) const
{

  auto itDevPair = m_devicePairToAlgorithmsPerCcId.find(std::make_pair(gnbDev, ueDev));
  NS_ABORT_MSG_IF (itDevPair == m_devicePairToAlgorithmsPerCcId.end(), "There is no task for the specified pair of devices.");

  auto itAlgorithm = (GetSecond (itDevPair)).find (ccId);
  NS_ABORT_MSG_IF (itAlgorithm == (GetSecond (itDevPair)).end (), "There is no BF task for the specified component carrier ID.");

  GetSecond (itAlgorithm)->GetBeamformingVectors (gnbDev, ueDev, &gnbBfv, &ueBfv, ccId);
}

void
RealisticBeamformingHelper::SetBeamformingMethod (const TypeId &beamformingMethod)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (beamformingMethod == RealisticBeamformingAlgorithm::GetTypeId () ||
             beamformingMethod.IsChildOf (RealisticBeamformingAlgorithm::GetTypeId ()));

  m_algorithmFactory.SetTypeId (beamformingMethod);
}


}
