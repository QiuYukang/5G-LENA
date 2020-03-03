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

#include "beam-manager.h"
#include <ns3/uinteger.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/node.h>
#include <ns3/boolean.h>
#include "mmwave-enb-net-device.h"
#include "mmwave-ue-net-device.h"
#include "mmwave-ue-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BeamManager");
NS_OBJECT_ENSURE_REGISTERED (BeamManager);

BeamManager::BeamManager() {
  // TODO Auto-generated constructor stub
}

void
BeamManager::InstallAntenna (uint32_t antennaNumDim1, uint32_t antennaNumDim2, bool areIsotropicElements) //TODO check whether to remove spectrum model as parameter, it is needed for cell scan?
{
  NS_ASSERT_MSG (m_antennaArray == nullptr, "Antenna already installed, access directly to antenna object to change parameters");
  m_antennaArray = CreateObject<ThreeGppAntennaArrayModel> ();
  m_antennaArray->SetAttribute ("NumColumns", UintegerValue(antennaNumDim1));
  m_antennaArray->SetAttribute ("NumRows", UintegerValue(antennaNumDim2));
  m_antennaArray->SetAttribute ("IsotropicElements", BooleanValue (areIsotropicElements));

  // we assume that the antenna dimension will not change during the simulation,
  // thus we create this omni vector only once
  m_omniTxRxW = GenerateOmniTxRxW (antennaNumDim1, antennaNumDim2);

  ChangeToOmniTx ();

  if (m_beamformingPeriodicity != MilliSeconds (0))
    {
      m_beamformingTimer = Simulator::Schedule (m_beamformingPeriodicity,
                                                &BeamManager::ExpireBeamformingTimer, this);
    }
}

complexVector_t
BeamManager::GetVector (const BeamformingVector& v) const
{
  return v.first;
}

BeamId
BeamManager::GetBeamId (const BeamformingVector&v) const
{
  return v.second;
}

BeamManager::~BeamManager() {
  // TODO Auto-generated destructor stub
}

TypeId
BeamManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BeamManager")
    .SetParent<Object> ()
    .AddConstructor<BeamManager> ()
    .AddAttribute ("BeamformingPeriodicity",
                   "Interval between beamforming phases",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&BeamManager::m_beamformingPeriodicity),
                   MakeTimeChecker())
    ;
  return tid;
}


void
BeamManager::SaveBeamformingVector (const BeamformingVector& bfv,
                                    const Ptr<const NetDevice>& device)
{
  NS_LOG_INFO ("SetBeamformingVector for BeamId:" << bfv.second << " node id: " << device->GetNode()->GetId());

  if (device != nullptr)
    {
      BeamformingStorage::iterator iter = m_beamformingVectorMap.find (device);
      if (iter != m_beamformingVectorMap.end ())
        {
          (*iter).second = bfv;
        }
      else
        {
          m_beamformingVectorMap.insert (std::make_pair (device, bfv));
        }
    }
  m_antennaArray->SetBeamformingVector(bfv.first);
}

void
BeamManager::ChangeBeamformingVector (const Ptr<const NetDevice>& device)
{
  NS_LOG_FUNCTION(this);
  if (m_performGenieBeamforming)
    {
      m_performGenieBeamforming = false;

      for (const auto & ueDev : m_ueDeviceMap)
        {
          BeamformingVector gnbBfv, ueBfv;
          m_genieAlgorithm->GetBeamformingVectors (m_gnbNetDevice, ueDev, &gnbBfv, &ueBfv);
          SaveBeamformingVector (gnbBfv, ueDev);
          (ueDev->GetPhy(m_ccId))->GetBeamManager()->SaveBeamformingVector(ueBfv, m_gnbNetDevice);
        }
    }
  BeamformingStorage::iterator it = m_beamformingVectorMap.find (device);
  if (it == m_beamformingVectorMap.end ())
    {
      NS_LOG_INFO ("Could not find the beamforming vector for the provided device");
      m_antennaArray->ChangeToOmniTx();
    }
  else
    {
      NS_LOG_INFO ("Beamforming vector found");
      m_antennaArray->SetBeamformingVector(it->second.first);
    }
}

void
BeamManager::AddUeDevice (const Ptr<MmWaveUeNetDevice> ueDevice)
{
  m_ueDeviceMap.push_back(ueDevice);
}

void
BeamManager::SetOwner (const Ptr<MmWaveEnbNetDevice> gnbDevice)
{
  m_gnbNetDevice = gnbDevice;
}

complexVector_t
BeamManager::GetCurrentBeamformingVector ()
{
  return m_antennaArray->GetBeamformingVector();
}

void
BeamManager::ChangeToOmniTx ()
{
  NS_LOG_FUNCTION (this);
  m_antennaArray->SetBeamformingVector(m_omniTxRxW.first);
}

complexVector_t
BeamManager::GetBeamformingVector (const Ptr<NetDevice>& device) const
{
  NS_LOG_FUNCTION (this);
  complexVector_t beamformingVector ;
  BeamformingStorage::const_iterator it = m_beamformingVectorMap.find (device);
  if (it != m_beamformingVectorMap.end ())
    {
      beamformingVector = it->second.first;
    }
  else
    {
      beamformingVector = m_antennaArray->GetBeamformingVector();
    }
  return beamformingVector;
}

BeamId
BeamManager::GetBeamId (const Ptr<NetDevice>& device) const
{
  BeamId beamId;
  BeamformingStorage::const_iterator it = m_beamformingVectorMap.find (device);
  if (it != m_beamformingVectorMap.end ())
    {
      beamId = it->second.second;
    }
  else
    {
      beamId = OMNI_BEAM_ID;
    }
  return beamId;
}

BeamformingVector
BeamManager::GenerateOmniTxRxW (uint32_t antennaNumDim1, uint32_t antennaNumDim2) const
{
  NS_LOG_FUNCTION (this);
  complexVector_t omni;
  uint32_t size = antennaNumDim1 * antennaNumDim2;
  double power = 1 / sqrt (size);
  for (uint32_t ind = 0; ind < antennaNumDim1; ind++)
    {
      std::complex<double> c = 0.0;
      if (antennaNumDim1 % 2 == 0)
        {
          c = exp(std::complex<double> (0, M_PI*ind*ind/antennaNumDim1));
        }
      else
        {
          c = exp(std::complex<double> (0, M_PI*ind*(ind+1)/antennaNumDim1));
        }

      for (uint32_t ind2 = 0; ind2 < antennaNumDim2; ind2++)
        {
          std::complex<double> d = 0.0;
          if (antennaNumDim2 % 2 == 0)
            {
              d = exp(std::complex<double> (0, M_PI*ind2*ind2/antennaNumDim2));
            }
          else
            {
              d = exp(std::complex<double> (0, M_PI*ind2*(ind2+1)/antennaNumDim2));
            }

          omni.push_back (c * d * power);
        }
    }

  return std::make_pair(omni, OMNI_BEAM_ID);
}

Ptr<ThreeGppAntennaArrayModel>
BeamManager::GetAntennaArray () const
{
  return m_antennaArray;
}

void
BeamManager::ExpireBeamformingTimer()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Beamforming timer expired; programming a beamforming");
  m_performGenieBeamforming = true;
  m_beamformingTimer = Simulator::Schedule (m_beamformingPeriodicity,
                                            &BeamManager::ExpireBeamformingTimer, this);
}

void
BeamManager::SetIdealBeamformingAlgorithm (const Ptr<IdealBeamformingAlgorithm>& algorithm)
{
  m_genieAlgorithm = algorithm;
}

Ptr<IdealBeamformingAlgorithm>
BeamManager::GetIdealBeamformingAlgorithm() const
{
  return m_genieAlgorithm;
}


} /* namespace ns3 */
