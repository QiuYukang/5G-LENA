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
#include "beamforming-algorithm.h"
#include <ns3/three-gpp-antenna-array-model.h>
#include <ns3/mobility-module.h>
#include <ns3/multi-model-spectrum-channel.h>
#include "nr-spectrum-phy.h"
#include "ns3/three-gpp-channel-model.h"
#include "realistic-bf-manager.h"
#include "nr-ue-net-device.h"
#include "nr-gnb-net-device.h"
//#include <ns3/nr-realistic-beamforming-test.h>

namespace ns3 {

class SpectrumModel;
class SpectrumValue;
class RealisticBeamformingHelper;
class NrRealisticBeamformingTestCase;

/**
 * \ingroup gnb-phy
 * \brief Generate "Real" beamforming vectors
 * This class is inherited by all algorithms that do not assume the
 * perfect knowledge of the channel, but instead are performing the
 * estimation of the channel based on measurements, e.g., based on
 * SRS SINR measurement.
 *
 * RealisticBeamformingAlgorithm purpose is to generate beams for the pair
 * of communicating devices based on the SRS measurements. Differently from
 * IdealBeamformingAlgorithm this type of algorithm does not assume a perfect
 * knowledge of the channel. It instead estimates the long-term fast fading
 * channel component based on the received SRS. Accordingly, this approach
 * could be used with any beamforming algorithm that makes use of the channel
 * estimation, e.g., beam search method (e.g., such as the one implemented in
 * CellScanBeamforming class). Note that the LOS type of method (e.g., such as
 * the one implemented in DirectPathBeamforming class) does not use the
 * channel matrix, but instead the angles of arrival and departure of the LOS
 * path, and so, the proposed method is not valid for it. Currently, it is
 * only compatible with the beam search method."
 */
class RealisticBeamformingAlgorithm: public BeamformingAlgorithm
{

  friend RealisticBeamformingHelper;
  friend NrRealisticBeamformingTestCase;

public:

  struct TriggerEventConf
  {
    RealisticBfManager::TriggerEvent event;
    uint16_t updatePeriodicity;
    Time updateDelay;
  };

  /**
   * \brief constructor
   */
  RealisticBeamformingAlgorithm ();

  /**
   * \brief constructor
   */
  RealisticBeamformingAlgorithm (Ptr<NrGnbNetDevice>& gNbDevice, Ptr<NrUeNetDevice>& ueDevice, uint8_t);

  /**
   * \brief destructor
   */
  virtual ~RealisticBeamformingAlgorithm ();

  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);


  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const override;
  /**
   * \return Gets value of BeamSearchAngleStep attribute
   */
  double GetBeamSearchAngleStep () const;

  /**
   * \brief Sets the value of BeamSearchAngleStep attribute
   */
  void SetBeamSearchAngleStep (double beamSearchAngleStep);

  /**
   * \brief Saves SRS SINR report for
   * \param srsSinr
   * \param rnti
   */
  void NotifySrsReport (uint16_t cellId, uint16_t rnti, double srsSinr);

  /**
   * \brief RunTask callback will be triggered when the event for updating the beamforming vectors occurs
   */
  typedef Callback<void, const Ptr<NrGnbNetDevice>&, Ptr<NrUeNetDevice>&, uint8_t> RealisticBfHelperCallback;


private:

  uint8_t GetSymbolsPerSlot ();

  RealisticBeamformingAlgorithm::TriggerEventConf GetTriggerEventConf ();

  void NotifyHelper ();

  /*
   * \brief Sets RealisticBeamformingHelperCallback that will be notified when it is necessary to update
   * the beamforming vectors. Function RunTask will then call back RealisticBeamformingAlgorithm that
   * notified it about the necessity to update the beamforming vectors. It is done in this way, in order
   * to split functionalities and responsibilities of the BF helper class, and BF algorithm class.
   * BF helper class takes care of necessary BF vector updates, and necessary calls of BeamManager class.
   * While BF algorithm class takes care of trigger event, paramters, and algorithm, but it is not
   * responsible to update the beamforming vector of devices.
   */
  void SetTriggerCallback (RealisticBfHelperCallback callback);

  virtual void DoGetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                        const Ptr<const NrUeNetDevice>& ueDev,
                                        BeamformingVector* gnbBfv,
                                        BeamformingVector* ueBfv,
                                        uint16_t ccId) const override;

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

  // attribute members, configuration variables
  double m_beamSearchAngleStep {30}; //!< The beam angle step that will be used to define the set of beams for which will be estimated the channel

  //variable members, counters, and saving values
  double m_maxSrsSinrPerSlot {0}; //!< maximum SRS SINR per slot in Watts, e.g. if there are 4 SRS symbols per UE, this value will represent the maximum
  uint8_t m_srsSymbolsCounter {0}; //!< counter that gets reset after reacing the number of symbols per SRS transmission
  uint16_t m_srsPeriodicityCounter {0}; // counter of SRS reports between consecutive beamforming updates, this counter is incremented once the counter m_srsSymbolsPerSlotCounter

  // random member variable, needed for algorithm calculation
  Ptr<NormalRandomVariable> m_normalRandomVariable; //!< The random variable used for the estimation of the error

  RealisticBfHelperCallback m_helperCallback; //!< When it is necessary to update the beamforming vectors for this pair of devices,
                                                   //the helper will be notified through this callback

  /*
   * \brief Parameters needed to pass to helper once that the helpers callback functions is being called
   */
  Ptr<NrGnbNetDevice> m_gNbDevice; //!< pointer to gNB device
  Ptr<NrUeNetDevice> m_ueDevice;  //!< pointer to UE device
  uint8_t m_ccId; //!< ccID index
  uint16_t m_rnti; //!< UE RNTI for which will be used this algorithm instance

};

} // end of namespace ns-3
#endif
