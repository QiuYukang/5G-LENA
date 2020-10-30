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

#ifndef SRC_NR_MODEL_REALISTIC_BEAMFORMING_ALGORITHM_H_
#define SRC_NR_MODEL_REALISTIC_BEAMFORMING_ALGORITHM_H_

#include <ns3/object.h>
#include "beam-id.h"
#include "beamforming-vector.h"
#include <ns3/three-gpp-antenna-array-model.h>
#include <ns3/mobility-module.h>
#include <ns3/multi-model-spectrum-channel.h>
#include "nr-spectrum-phy.h"
#include "ns3/three-gpp-channel-model.h"

namespace ns3 {

class SpectrumModel;
class SpectrumValue;
class NrGnbNetDevice;
class NrUeNetDevice;

/**
 * \ingroup gnb-phy
 * \brief Generate "Ideal" beamforming vectors
 *
 * RealisticBeamformingAlgorithm purpose is to generate beams for the pair
 * of communicating devices based on the SRS measurements. Differently from
 * IdealBeamformingAlgorithm this type of algorithm does not assume
 * a perfect knowledge of the channel.
 * It instead estimates the long-term fast fading channel
 * component based on the received SRS. Accordingly, this approach
 * could be used with any beamforming algorithm that makes use of the channel estimation,
 * e.g., beam search method (e.g., such as the one implemented in CellScanBeamforming
 * class). Note that the LOS type of method (e.g., such as the one implemented in
 * DirectPathBeamforming class) does not use the channel matrix, but instead the
 * angles of arrival and departure of the LOS path, and so, the proposed method
 * is not valid for it. Currently, it is only compatible with the beam search method. "
 */

class RealisticBeamformingAlgorithm: public Object
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
  RealisticBeamformingAlgorithm ();

  /**
   * \brief destructor
   */
  virtual ~RealisticBeamformingAlgorithm ();

  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const;

  /**
   * \return Gets value of BeamSearchAngleStep attribute
   */
  double GetBeamSearchAngleStep () const;

  /**
   * \brief Sets the value of BeamSearchAngleStep attribute
   */
  void SetBeamSearchAngleStep (double beamSearchAngleStep);

  /**
   * \brief Sets the sirn SRS value to be used. In lineal unit.
   */
  void SetSrsSinr (double sinrSrs);

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
                                        BeamformingVector* gnbBfv,
                                        BeamformingVector* ueBfv,
                                        uint16_t ccId) const;

  /**
   * \brief Calculates an estimation of the long term component based on the channel measurements
   * \param channelMatrix the channel matrix H
   * \param sW the beamforming vector of the s device
   * \param uW the beamforming vector of the u device
   * \return the estimated long term component
   */
  ThreeGppAntennaArrayModel::ComplexVector GetEstimatedLongTermComponent (const Ptr<const MatrixBasedChannelModel::ChannelMatrix>& channelMatrix,
                                                                          const ThreeGppAntennaArrayModel::ComplexVector &sW,
                                                                          const ThreeGppAntennaArrayModel::ComplexVector &uW) const;

  /*
   * \brief Calculates the total metric based on the each element of the long term component
   * \param longTermComponent the vector of complex numbers representing the long term component per cluster
   */
  double CalculateTheEstimatedLongTermMetric (const ThreeGppAntennaArrayModel::ComplexVector& longTermComponent) const;


  double m_beamSearchAngleStep {30}; //!< The beam angle step that will be used to define the set of beams for which will be estimated the channel
  double m_lastRerportedSrsSinr; //!< The last reported SRS sinr notified by gNB PHY to its beam manager and beamforming algorithm
};

} // end of namespace ns-3
#endif
