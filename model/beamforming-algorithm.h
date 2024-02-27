/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef SRC_NR_MODEL_BEAMFORMING_ALGORITHM_H_
#define SRC_NR_MODEL_BEAMFORMING_ALGORITHM_H_

#include "beamforming-vector.h"

#include <ns3/object.h>

namespace ns3
{

class SpectrumModel;
class SpectrumValue;
class NrGnbNetDevice;
class NrUeNetDevice;

/**
 * \ingroup gnb-phy
 * \brief Generate "Ideal" beamforming vectors
 *
 * BeamformingAlgorithm purpose is to generate beams for the pair
 * of communicating devices.
 */
class BeamformingAlgorithm : public Object
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief constructor
     */
    BeamformingAlgorithm();

    /**
     * \brief destructor
     */
    ~BeamformingAlgorithm() override;

    /**
     * \brief Function that generates the beamforming vectors for a pair of communicating devices
     * \param [in] gnbDev gNb beamforming device
     * \param [in] ueDev UE beamforming device
     * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to
     * communicate with ueDev according to this algorithm criteria \param [out] ueBfv the best
     * beamforming vector for ueDev device antenna array to communicate with gNbDev device according
     * to this algorithm criteria
     */
    virtual void GetBeamformingVectors(const Ptr<const NrGnbNetDevice>& gnbDev,
                                       const Ptr<const NrUeNetDevice>& ueDev,
                                       BeamformingVector* gnbBfv,
                                       BeamformingVector* ueBfv,
                                       uint16_t ccId) const = 0;
};

} // namespace ns3
#endif
