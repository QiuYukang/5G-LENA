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
#include "beamforming-vector.h"
#include <ns3/three-gpp-antenna-array-model.h>
#include <ns3/mobility-module.h>

namespace ns3 {

class SpectrumModel;
class SpectrumValue;
class NrGnbNetDevice;
class NrUeNetDevice;

/**
 * \ingroup gnb-phy
 * \brief Generate "Ideal" beamforming vectors
 *
 * IdealBeamformingAlgorithm purpose is to generate beams for the pair
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

  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const;

  /**
   * \return Gets value of BeamSearchAngleStep attribute
   */
  double GetBeamSearchAngleStep () const;

  /**
   * \brief Sets the value of BeamSearchAngleStep attribute
   */
  void SetBeamSearchAngleStep (double beamSearchAngleStep);

private:

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                        const Ptr<const NrUeNetDevice>& ueDev,
                                        BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const = 0;


};

/**
 * \ingroup gnb-phy
 * \brief The CellScanBeamforming class
 */
class CellScanBeamforming: public IdealBeamformingAlgorithm
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \return Gets value of BeamSearchAngleStep attribute
   */
  double GetBeamSearchAngleStep () const;

  /**
   * \brief Sets the value of BeamSearchAngleStep attribute
   */
  void SetBeamSearchAngleStep (double beamSearchAngleStep);

  /**
   * \brief constructor
   */
  CellScanBeamforming () = default;

  /**
   * \brief destructor
   */
  virtual ~CellScanBeamforming () override = default;

protected:

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using cell scan method
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                        const Ptr<const NrUeNetDevice>& ueDev,
                                        BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const override;

  double m_beamSearchAngleStep {30};

};

/**
 * \ingroup gnb-phy
 * \brief The DirectPathBeamforming class
 */
class DirectPathBeamforming: public IdealBeamformingAlgorithm
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);


protected:

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using the direct path direction
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                        const Ptr<const NrUeNetDevice>& ueDev,
                                        BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const override;

  /**
   * \brief Get directs path beamforming vector bfv for a device with the mobility model
   * a for transmission toward device with a mobility model b, by using antenna aAntenna.
   * \param [in] a mobility model of the first device
   * \param [in] b mobility model of the second device
   * \param [in] aAntenna antenaArray of the first device
   * \param [out] bfv resulting beamforming vector for antenna array for the first device
   */
  virtual void DoGetDirectPathBeamformingVector (const Ptr<MobilityModel>& a,
                                                 const Ptr<MobilityModel>& b,
                                                 const Ptr<const ThreeGppAntennaArrayModel>& aAntenna,
                                                 BeamformingVector* bfv, uint16_t ccId) const;
};

/**
 * \ingroup gnb-phy
 * \brief The QuasiOmniDirectPathBeamforming class
 */
class QuasiOmniDirectPathBeamforming: public DirectPathBeamforming
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);


protected:

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using the quasi omni beamforming vector for gNB
   * and direct path beamforming vector for UEs
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                        const Ptr<const NrUeNetDevice>& ueDev,
                                        BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const override;

};

/**
 * \ingroup gnb-phy
 * \brief The OptimalCovMatrixBeamforming class
 */
class OptimalCovMatrixBeamforming : public IdealBeamformingAlgorithm
{

public:

  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);


protected:

  virtual void DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                        const Ptr<const NrUeNetDevice>& ueDev,
                                        BeamformingVector* gnbBfv, BeamformingVector* ueBfv, uint16_t ccId) const override;
};


} // end of ns3 namespace
#endif
