// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef SRC_NR_MODEL_BEAMFORMING_VECTOR_H_
#define SRC_NR_MODEL_BEAMFORMING_VECTOR_H_

#include "beam-id.h"

#include "ns3/mobility-model.h"
#include "ns3/uniform-planar-array.h"

namespace ns3
{

/**
 * @ingroup utils
 * @brief Physical representation of a beam.
 *
 * Contains the vector of the antenna weight, as well as the beam id. These
 * values are stored as std::pair, and we provide utilities functions to
 * extract them.
 *
 * @see GetVector
 * @see GetBeamId
 */
typedef std::pair<PhasedArrayModel::ComplexVector, BeamId> BeamformingVector;

typedef std::pair<BeamformingVector, BeamformingVector> BeamformingVectorPair;

/**
 * @brief Create a quasi omni beamforming vector
 * @ingroup utils
 * @param antenna Antenna array for which the beamforming vector will be created
 * @return the beamforming vector
 */
PhasedArrayModel::ComplexVector CreateQuasiOmniBfv(const Ptr<const UniformPlanarArray>& antenna);

/**
 * @brief Creates a beamforming vector for a given sector and elevation
 * @ingroup utils
 * @param antenna Antenna array for which will be created the beamforming vector
 * @param sector sector to be used
 * @param elevation elevation to be used
 * @return the beamforming vector
 */
PhasedArrayModel::ComplexVector CreateDirectionalBfv(const Ptr<const UniformPlanarArray>& antenna,
                                                     double sector,
                                                     double elevation);

/**
 * @brief Creates a beamforming vector for a given azimuth and zenith
 * @ingroup utils
 * @param antenna Antenna array for which will be created the beamforming vector
 * @param azimuth azimuth to be used
 * @param zenith zenith to be used
 * @return the beamforming vector
 */
PhasedArrayModel::ComplexVector CreateDirectionalBfvAz(const Ptr<const UniformPlanarArray>& antenna,
                                                       double azimuth,
                                                       double zenith);

/**
 * @brief Get directs path beamforming vector bfv for a device with the mobility model
 * a for transmission toward device with a mobility model b, by using antenna aAntenna.
 * @param [in] a mobility model of the first device
 * @param [in] b mobility model of the second device
 * @param [in] antenna antenaArray of the first device
 * @return the resulting beamforming vector for antenna array for the first device
 */
PhasedArrayModel::ComplexVector CreateDirectPathBfv(const Ptr<MobilityModel>& a,
                                                    const Ptr<MobilityModel>& b,
                                                    const Ptr<const UniformPlanarArray>& antenna);
/**
 * @brief Creates a beamforming vector using Kronecker method for a given azimuth and zenith
 *
 * This function creates a PhasedArrayAngleConvention object for every call.
 * The object determines how to interpret the passed angles to compute the beamforming vector.
 * The angles can be the 3GPP azimuth and zenith angles, OR the vertical and horizontal ULA angles
 * (depending on the adopted convention).
 * To change the interpretation of the angles, from default ULA VH to 3GPP, use
 * Config::SetDefault("ns3::PhasedArrayAngleConvention::AngleConvention", StringValue("3GPP"))
 * or
 * Config::SetDefault("ns3::PhasedArrayAngleConvention::AngleConvention",
 * EnumValue(PhasedArrayAngleConvention::THREE_GPP)).
 *
 * The AngleConvention is then used to execute either CreateKroneckerBfvThreeGpp() or
 * CreateKroneckerBfvUla(), that compute the vertical and horizontal phase per element according to
 * the different conventions. These phases are then passed to CreateKroneckerBfvImpl() to compute
 * the Kronecker product.
 *
 * @param [in] antenna Antenna array for which will be created the beamforming vector
 * @param [in] rowAngle row angle to be used
 * @param [in] colAngle column angle to be used
 * @return the beamforming vector
 */
PhasedArrayModel::ComplexVector CreateKroneckerBfv(const Ptr<const UniformPlanarArray>& antenna,
                                                   double rowAngle,
                                                   double colAngle);

/**
 * @brief Helper class to select phased-array angle convention.
 *
 * PhasedArrayAngleConvention stores the convention used to interpret
 * the two input angles passed to beamforming vector generation
 * routines (e.g., for a Uniform Planar Array).
 *
 * Two conventions are currently supported:
 *  - THREE_GPP: angles are given as 3GPP-style zenith and azimuth
 *               angles, typically in degrees.
 *  - ULA_VH:    angles are given as vertical and horizontal angles
 *               for separate ULAs (row/column angles), typically
 *               in degrees.
 *
 * This class is designed to be exposed as an ns-3 attribute so that
 * simulation scripts can configure the desired convention without
 * modifying the underlying beamforming code.
 */
class PhasedArrayAngleConvention : public Object
{
  public:
    /**
     * @brief Angle convention used for interpreting input angles.
     *
     * THREE_GPP represents 3GPP reference angles (zenith, azimuth),
     * while ULA_VH represents vertical and horizontal ULA angles
     * (rowAngle, colAngle).
     */
    enum AngleConvention
    {
        THREE_GPP, //!< Use 3GPP zenith/azimuth angle convention.
        ULA_VH     //!< Use vertical/horizontal ULA angle convention.
    };

    static TypeId GetTypeId();
    PhasedArrayAngleConvention() = default;

    /**
     * @brief Get the currently configured angle convention.
     *
     * This method returns the convention that should be used to
     * interpret angle parameters in the associated beamforming code.
     *
     * @return The configured AngleConvention value.
     */
    AngleConvention GetConvention() const;

  private:
    AngleConvention m_angleConvention = ULA_VH; //!< Stored angle convention.
};

} // namespace ns3

#endif /* SRC_NR_MODEL_BEAMFORMING_VECTOR_H_ */
