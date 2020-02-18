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

#include <ns3/three-gpp-antenna-array-model.h>
#include <ns3/object.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <complex>
#include <ns3/nstime.h>
#include <ns3/net-device.h>
#include <map>
#include <ns3/spectrum-model.h>
#include <functional>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/event-id.h>
#include "beam-id.h"
#include "ideal-beamforming-algorithm.h"
#include <ns3/mobility-model.h>

namespace ns3 {


/**
 * \ingroup beam-management
 * \brief BeamManager is TODO
 */

  /**
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


class BeamManager: public Object {

public:

  BeamManager();

  virtual ~BeamManager();

  /**
   * \brief GetTypeId
   * \return the TypeId of this instance
   */
  static TypeId GetTypeId ();


  void InstallAntenna (uint32_t antennaNumDim1, uint32_t antennaNumDim2, bool areIsotropicElements);

  /**
   * \brief Get weight vector from a BeamformingVector
   * \param v the BeamformingVector
   * \return the weight vector
   */
  complexVector_t GetVector (BeamformingVector v)
  {
    return v.first;
  }
  /**
   * \brief Extract the beam id from the beamforming vector specified
   * \return the beam id
   * \param v the beamforming vector
   */
  BeamId GetBeamId (const BeamformingVector &v)
  {
    return v.second;
  }

 typedef std::map<Ptr<NetDevice>, BeamformingVector> BeamformingStorage;


  /**
   * \brief Function sets the beamforming weights of the antenna
   * for transmission or reception to/from a specified connected device.
   * It also configures the beamId of this beamforming vector.  *
   * \param antennaWeights the weights of the beamforming vector
   * \param beamId the unique identifier of the beam
   * \param device device to which it is being transmitted, or from which is
   * being received
   */
   virtual void SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                                      Ptr<NetDevice> device);

   /**
    * \brief Change the beamforming vector for tx/rx to/from specified device
    * \param device Device to change the beamforming vector for
    */
   virtual void ChangeBeamformingVector (Ptr<NetDevice> device);

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
   virtual complexVector_t GetBeamformingVector (Ptr<NetDevice> device);


   /**
    * \brief Function that returns the beamId of the beam that is used to
    * communicated with a specified device
    * \return the current beamforming vector
    */
   virtual BeamId GetBeamId (Ptr<NetDevice> device);


   /**
    * \brief Generate a omni directional beamforming vector
    * \param antennaNumDim1 First dimension of the antenna
    * \param antennaNumDim2 Second dimension of the antenna
    * \return the beamforming vector
    */
   BeamformingVector GenerateOmniTxRxW (uint32_t antennaNumDim1, uint32_t antennaNumDim2) const;

   Ptr<ThreeGppAntennaArrayModel> GetAntennaArray ();

   /**
    * \brief The beamforming timer has expired; at the next slot, perform beamforming.
    *
    * This function just set to true a boolean variable that will be checked in
    * StartVarTti().
    */
   void ExpireBeamformingTimer ();

   void SetIdeamBeamformingAlgorithm (Ptr<IdealBeamformingAlgorithm> algorithm);

   Ptr<IdealBeamformingAlgorithm> GetIdealBeamformingAlgorithm();

private:

   Ptr<ThreeGppAntennaArrayModel> m_antennaArray;  // the antenna array instance for which is responsible this BeamManager
   BeamformingVector m_omniTxRxW; //!< Beamforming vector that emulates omnidirectional transmission and reception
   Time m_beamformingPeriodicity; //!< Periodicity of beamforming (0 for never)
   EventId m_beamformingTimer;    //!< Beamforming timer

   //only gNB beam manager is needs this part
   BeamformingStorage m_beamformingVectorMap; //!< device to beamforming vector mapping

   //only genie beaforming
   bool m_performGenieBeamforming {true}; //!< True when we have to do beamforming. Default to true or we will not perform beamforming the first time..
   Ptr<IdealBeamformingAlgorithm> m_genieAlgorithm;
};

} /* namespace ns3 */

#endif /* SRC_NR_MODEL_BEAM_MANAGER_H_ */
