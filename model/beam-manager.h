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

#ifndef SRC_NR_MODEL_BEAM_MANAGER_H_
#define SRC_NR_MODEL_BEAM_MANAGER_H_

#include "beam-id.h"
#include "ideal-beamforming-algorithm.h"
#include "ns3/event-id.h"
#include <ns3/three-gpp-antenna-array-model.h>
#include <ns3/nstime.h>

namespace ns3 {

/**
 * \ingroup beam-management
 * \brief Physical representation of a beam.
 *
 * Contains the vector of the antenna weight, as well as the beam id. These
 * values are stored as std::pair, and we provide utilities functions to
 * extract them.
 *
 * \see GetVector
 * \see GetBeamId
 */
typedef std::pair<complexVector_t, BeamId>  BeamformingVector;


/**
 * \ingroup beam-management
 * \brief BeamManager is responsible of installation and configuration of antenna
 * array. Additionally, in the case of gNB it saves the map of beamforming
 * vectors per device.
 */
class BeamManager: public Object {

public:

  BeamManager();

  virtual ~BeamManager();

  /**
   * \brief GetTypeId
   * \return the TypeId of this instance
   */
  static TypeId GetTypeId ();

  /**
   * \brief Creates and object of antenna array and initialize its beamforming vector to
   * quasi omni beamforming vector
   * \param antennaNumDim1 the first antenna dimension in number of elements
   * \param antennaNumDim2 the second antenna dimension in number of elements
   * \param areIsotropicElements whether the antenna elements are isotropic or 3gpp
   */
  void InstallAntenna (uint32_t antennaNumDim1, uint32_t antennaNumDim2, bool areIsotropicElements);

  /**
   * \brief Get weight vector from a BeamformingVector
   * \param v the BeamformingVector
   * \return the weight vector
   */
  complexVector_t GetVector (const BeamformingVector& v) const;

  /**
   * \brief Extract the beam id from the beamforming vector specified
   * \return the beam id
   * \param v the beamforming vector
   */
  BeamId GetBeamId (const BeamformingVector &v) const;


  typedef std::map<const Ptr<const NetDevice>, BeamformingVector> BeamformingStorage; //!< BeamformingStorage type used to save the map of beamforming vectors per device

  /**
   * \brief Function sets the beamforming weights of the antenna
   * for transmission or reception to/from a specified connected device.
   * It also configures the beamId of this beamforming vector.  *
   * \param antennaWeights the weights of the beamforming vector
   * \param beamId the unique identifier of the beam
   * \param device device to which it is being transmitted, or from which is
   * being received
   */
  virtual void SetBeamformingVector (const complexVector_t& antennaWeights, const BeamId& beamId,
                                     const Ptr<const NetDevice>& device);

  /**
   * \brief Change the beamforming vector for tx/rx to/from specified device
   * \param device Device to change the beamforming vector for
   */
  virtual void ChangeBeamformingVector (const Ptr<const NetDevice>& device);

  /**
   * \brief Change the antenna model to omnidirectional (ignoring the beams) TODO check this
   */
  virtual void ChangeToOmniTx ();

  /**
   * \brief Function that returns the beamforming vector that is currently being
   * used by the antenna.
   * \return the current beamforming vector
   */
  virtual complexVector_t GetCurrentBeamformingVector ();

  /**
   * \brief Function that returns the beamforming vector weights that is used to
   * communicated with a specified device
   * \return the current beamforming vector
   */
  virtual complexVector_t GetBeamformingVector (const Ptr<NetDevice>& device) const;


  /**
   * \brief Function that returns the beamId of the beam that is used to
   * communicated with a specified device
   * \return the current beamforming vector
   */
  virtual BeamId GetBeamId (const Ptr<NetDevice>& device) const;

  /**
   * \brief Generate a omni directional beamforming vector
   * \param antennaNumDim1 First dimension of the antenna
   * \param antennaNumDim2 Second dimension of the antenna
   * \return the beamforming vector
   */
  BeamformingVector GenerateOmniTxRxW (uint32_t antennaNumDim1, uint32_t antennaNumDim2) const;

  /**
  * TODO remove this from BeamManager, we agreed (N&B) that only SpectrumPhy or Phy will have a pointer to Antenna
  */
  Ptr<ThreeGppAntennaArrayModel> GetAntennaArray () const;

  /**
   * \brief The beamforming timer has expired; at the next slot, perform beamforming.
   *
   * This function just set to true a boolean variable that will be checked in
   * StartVarTti().
   */
  void ExpireBeamformingTimer ();

  void SetIdeamBeamformingAlgorithm (const Ptr<IdealBeamformingAlgorithm>& algorithm);

  Ptr<IdealBeamformingAlgorithm> GetIdealBeamformingAlgorithm() const;

private:

  Ptr<ThreeGppAntennaArrayModel> m_antennaArray;  // the antenna array instance for which is responsible this BeamManager
  BeamformingVector m_omniTxRxW; //!< Beamforming vector that emulates omnidirectional transmission and reception
  Time m_beamformingPeriodicity; //!< Periodicity of beamforming (0 for never)
  EventId m_beamformingTimer;    //!< Beamforming timer

  //only gNB beam manager needs this part
  BeamformingStorage m_beamformingVectorMap; //!< device to beamforming vector mapping

  //only genie beaforming
  bool m_performGenieBeamforming {true}; //!< True when we have to do beamforming. Default to true or we will not perform beamforming the first time..
  Ptr<IdealBeamformingAlgorithm> m_genieAlgorithm;
};

} /* namespace ns3 */

#endif /* SRC_NR_MODEL_BEAM_MANAGER_H_ */
