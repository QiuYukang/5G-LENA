/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef SRC_NR_MODEL_BEAM_CONF_ID_H
#define SRC_NR_MODEL_BEAM_CONF_ID_H

#include "beam-id.h"

namespace ns3
{

/**
 * \ingroup utils
 *
 * Represents the ID of the beam configuration of a PHY.
 * Beam configuration can contain one beamId, or two beamIds.
 * The order of the beams matters.
 * Beam configuration can have either configured only the
 * first beam, or only the second beam or both.
 */

class BeamConfId
{
  public:
    /**
     * Default constructor
     */
    BeamConfId();

    /**
     * \constructor Constructor used to configure this BeamConfId
     *
     * If the BeamConfId consists of two beams that the constructor should be called in the
     * following way: BeamConfig (beamId1, beamId2) if there is only the beamId1 BeamConfig
     * (beamId1, BeamId::GetEmptyBeamId() ) if there is only the beamId2 BeamConfig
     * (BeamId::GetEmptyBeamId(), beamId2 )
     *
     * \param firstBeam the first beam
     * \param secondBeam the second beam
     */
    BeamConfId(BeamId firstBeam, BeamId secondBeam);

    /**
     * \brief Objects of this class are used as key in hash
     * table. This class must implement operator ==()
     * to handle collisions.
     * \param p BeamConfId with which we compare this object
     */
    bool operator==(const BeamConfId& p) const;

    /**
     * \brief Overrides != operator for the general use case
     * \param p BeamConfId with which we compare this object
     */
    bool operator!=(const BeamConfId& p) const;

    /**
     * \brief Extract the sector from the beam id
     * \return The sector of the beam
     */
    BeamId GetFirstBeam() const;

    /**
     * \brief Extract the elevation from the beam id
     * \return the elevation of the beam
     */
    BeamId GetSecondBeam() const;

    /**
     * \brief Creates BeamConfId with the first
     * \return BeamId (0,0)
     */
    static BeamConfId GetEmptyBeamConfId();

  private:
    BeamId m_firstBeam = BeamId::GetEmptyBeamId();  //!< the first beam ID
    BeamId m_secondBeam = BeamId::GetEmptyBeamId(); //!< the second beam ID
};

/**
 * \brief Calculate the hash of a BeamConfId
 * \ingroup utils
 */
struct BeamConfIdHash
{
    /**
     * \brief operator ()
     * \param x beam conf id
     * \return the beam conf id hash
     */
    size_t operator()(const BeamConfId& x) const;
};

std::ostream& operator<<(std::ostream& os, const BeamConfId& item);

} // namespace ns3

#endif /* SRC_NR_MODEL_BEAM_CONF_ID_H */
