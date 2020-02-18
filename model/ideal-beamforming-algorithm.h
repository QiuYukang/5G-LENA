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

#ifndef SRC_NR_MODEL_IDEAL_BEAMFORMING_ALGORITHM_H_
#define SRC_NR_MODEL_IDEAL_BEAMFORMING_ALGORITHM_H_

#include <ns3/ptr.h>
#include <ns3/net-device.h>
#include <ns3/mobility-model.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/spectrum-model.h>
#include <ns3/spectrum-channel.h>
#include <ns3/mmwave-phy.h>
#include <ns3/three-gpp-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/mmwave-spectrum-phy.h>

namespace ns3 {

class BeamManager;

class IdealBeamformingAlgorithm: public Object
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);


  /**
   * \brief constructor
   */
  IdealBeamformingAlgorithm () {}

  /**
   * \brief destructor
   */
  virtual ~IdealBeamformingAlgorithm () {}

  /**
   * \brief Set owner gNB device of this ideal beamforming algorithm
   */
  void SetOwner (Ptr<NetDevice>, uint8_t ccId);

  /**
   * \brief Add UE device in the list of UE devices for which will be performed
   * ideal beamforming method
   */
  void AddUeDevice (Ptr<NetDevice> ueDevice);

  virtual void Run ();

  static Ptr<const SpectrumValue> CreateFakeTxPowerSpectralDensity (double powerTx, Ptr<const SpectrumModel> txSm);

private:

  virtual void DoRun (Ptr<NetDevice> gNbDev, Ptr<NetDevice> ueDev) const = 0;


  std::vector< Ptr<NetDevice> > m_ueDeviceMap;  // list of UE devices for which genie beamforming should be performed

protected:

  Ptr<NetDevice> m_netDevice;
  uint8_t m_ccId;

};


class CellScanBeamforming: public IdealBeamformingAlgorithm
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief constructor
   */
  CellScanBeamforming () {}

  /**
   * \brief destructor
   */
  virtual ~CellScanBeamforming () {}

private:

  virtual void DoRun (Ptr<NetDevice> gNbDev, Ptr<NetDevice> ueDev) const override;

  void SetSector (uint16_t sector, double elevation,  Ptr<BeamManager> beamManager) const;

  double m_beamSearchAngleStep {30};

};


class DirectPathBeamforming: public IdealBeamformingAlgorithm {

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);


private:

  virtual void DoRun (Ptr<NetDevice> gNbDev, Ptr<NetDevice> ueDev) const override;
};


class OptimalCovMatrixBeamforming : public IdealBeamformingAlgorithm {

public:

  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);


private:

  virtual void DoRun (Ptr<NetDevice> gNbDev, Ptr<NetDevice> ueDev) const override;
};


} // end of ns3 namespace
#endif
