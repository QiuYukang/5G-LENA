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

#include <ns3/object.h>
#include "beam-id.h"
#include <ns3/three-gpp-antenna-array-model.h>

namespace ns3 {

class SpectrumModel;
class SpectrumValue;
class MmWaveEnbNetDevice;
class MmWaveUeNetDevice;

/**
 * \brief IdealBeamformingAlgorithm purpose is to generate beams for the pair
 * of communicating devices. This group of algorithms assumes
 * a perfect knowledge of the channel, because of which is called "ideal"
 * algorithm.
 */
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
  IdealBeamformingAlgorithm ();

  /**
   * \brief destructor
   */
  virtual ~IdealBeamformingAlgorithm ();

  /**
   * \brief Set ccId to which belongs this ideal beamforming algorithm
   */
  void SetOwner (uint8_t ccId);

  virtual void GetBeamformingVectors (const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const;

  static Ptr<const SpectrumValue> CreateFakeTxPowerSpectralDensity (double powerTx, Ptr<const SpectrumModel> txSm);

private:

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void DoGetBeamformingVectors (const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const = 0;

protected:

  uint8_t m_ccId {0};

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

  virtual void DoGetBeamformingVectors (const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const override;

  void SetSector (uint16_t sector, double elevation,  Ptr<ThreeGppAntennaArrayModel> antennaArray) const;

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

  virtual void DoGetBeamformingVectors (const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const override;
};


class OptimalCovMatrixBeamforming : public IdealBeamformingAlgorithm
{

public:

  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);


private:

  virtual void DoGetBeamformingVectors (const Ptr<MmWaveEnbNetDevice>& gnbDev, const Ptr<MmWaveUeNetDevice>& ueDev, BeamformingVector* gnbBfv, BeamformingVector* ueBfv) const override;
};


} // end of ns3 namespace
#endif
