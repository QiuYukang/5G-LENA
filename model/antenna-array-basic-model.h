/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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

#ifndef ANTENNA_ARRAY_BASIC_H_
#define ANTENNA_ARRAY_BASIC_H_

#include <ns3/antenna-model.h>
#include <complex>

namespace ns3 {

class NetDevice;

/**
 * \ingroup antenna
 * \brief The AntennaArrayBasicModel class
 *
 * The class provides a basic interface for any antenna that uses Beams.
 */
class AntennaArrayBasicModel : public AntennaModel
{
public:
  /**
   * \brief Constructor
   */
  AntennaArrayBasicModel ();
  /**
   * \brief Destructor
   */
  virtual ~AntennaArrayBasicModel ();
  /**
   * \brief GetTypeId
   * \return the TypeId of this instance
   */
  static TypeId GetTypeId ();

  /**
   * \brief Syntax sugar to express a vector of complex
   */
  typedef std::vector<std::complex<double>> complexVector_t;
  /**
   * \brief Representation of a beam id
   *
   * A beam id in ns-3 is a pair that contains the sector, stored as a uint8_t,
   * and the elevation, stored as a double. Utilities functions are provided to
   * extract the values. This ID usually come with the real physical representation
   * of a Beam, expressed by BeamformingVector.
   *
   * \see GetSector
   * \see GetElevation
   */
  typedef std::pair<uint8_t, double> BeamId;
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

  /**
   * \brief Get weight vector from a BeamformingVector
   * \param v the BeamformingVector
   * \return the weight vector
   */
  static complexVector_t GetVector (BeamformingVector v)
  {
    return v.first;
  }
  /**
   * \brief Extract the beam id from the beamforming vector specified
   * \return the beam id
   * \param v the beamforming vector
   */
  static BeamId GetBeamId (const BeamformingVector &v)
  {
    return v.second;
  }
  /**
   * \brief Extract the sector from the beam id
   * \return The sector of the beam
   * \param b beam
   */
  static uint8_t GetSector (const BeamId &b)
  {
    return b.first;
  }
  /**
   * \brief Extract the elevation from the beam id
   * \return the elevation of the beam
   * \param b the beam
   */
  static double GetElevation (const BeamId &b)
  {
    return b.second;
  }

  /**
   * \brief Calculate the Cantor function for two unsigned int
   * \param x1 first value
   * \param x2 second value
   * \return \f$ (((x1 + x2) * (x1 + x2 + 1))/2) + x2; \f$
   */
  static constexpr uint32_t Cantor (uint32_t x1, uint32_t x2)
  {
    return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
  }

  /**
   * \brief Calculate the hash of a BeamId
   */
  struct BeamIdHash
  {
    size_t operator() (const AntennaArrayBasicModel::BeamId &x) const
    {
      return std::hash<uint32_t>()(Cantor (x.first, static_cast<uint32_t> (x.second)));
    }
  };

  /**
    * \brief This method in inherited from the AntennaModel. It is
    * designed to return the power gain in dBi of the antenna
    * radiation pattern at the specified angles;
    * dBi means dB with respect to the gain of an isotropic radiator.
    * Since a power gain is used, the efficiency of
    * the antenna is expected to be included in the gain value.
    *
    * \param a the spherical angles at which the radiation pattern should
    * be evaluated
    *
    * \return the power gain in dBi
    */
  virtual double GetGainDb (Angles a) = 0;

  /**
   * \brief Function sets the beamforming weights of the antenna
   * for transmission or reception to/from a specified connected device
   * using the beam that is specified by the beamId
   * \param antennaWeights the weights of the beamforming vector
   * \param beamId the unique identifier of the beam
   * \param device device to which it is being transmitted, or from which is
   * being received
   */
  virtual void SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                                     Ptr<NetDevice> device = nullptr) = 0 ;

  /**
   * \brief Function that schedules the call to SetBeamformingVector witha a
   * predefined delay of 8 ms.
   * \param antennaWeights the weights of the beamforming vector
   * \param beamId the unique identifier of the beam
   * \param device device to which it is being transmitted, or from which is
   * being received
   */
  virtual void SetBeamformingVectorWithDelay (complexVector_t antennaWeights, BeamId beamId,
                                              Ptr<NetDevice> device = nullptr) = 0;

  /**
   * \brief Change the beamforming vector for a device
   * \param device Device to change the beamforming vector for
   */
  virtual void ChangeBeamformingVector (Ptr<NetDevice> device) = 0 ;

  /**
   * \brief Change the antenna model to omnidirectional (ignoring the beams)
   */
  virtual void ChangeToOmniTx () = 0;

  /**
   * \brief Function that returns the beamforming vector that is currently being
   * used by the antenna.
   * \return the current beamforming vector
   */
  virtual BeamformingVector GetCurrentBeamformingVector () = 0;

  /**
   * \brief Function that returns the beamforming vector weights that is used to
   * communicated with a specified device
   * \return the current beamforming vector
   */
  virtual BeamformingVector GetBeamformingVector (Ptr<NetDevice> device) = 0;

  virtual void SetToSector (uint32_t sector, uint32_t antennaNum) = 0;

  /**
   * Returns a bool that says if the current transmission is configured to be
   * omni.
   * \return whether the transmission is set to omni
   */
  virtual bool IsOmniTx () = 0;

  /**
   * Function returns the radiation pattern for the specified vertical
   * and the horizontal angle.
   * \param vangle vertical angle
   * \param hangle horizontal angle
   * \return returns the radiation pattern
   */
  virtual double GetRadiationPattern (double vangle, double hangle) = 0;

  /**
   * Function returns the location of the antenna element inside of the
   * sector assuming the left bottom corner is (0,0,0).
   * \param index index of the antenna element
   * \param antennaNum total number of antenna elements in the panel
   * \return returns the 3D vector that represents the position of the antenna
   * by specifing x, y and z coordinate
   */
  virtual Vector GetAntennaLocation (uint8_t index, uint8_t* antennaNum) = 0;

  /**
   * \brief Manually set the sector on the antenna
   * \param sector Sector
   * \param antennaNum Number of antenna
   * \param elevation Elevation
   */
  virtual void SetSector (uint8_t sector, uint8_t *antennaNum, double elevation = 90) = 0;

};

std::ostream & operator<< (std::ostream & os, AntennaArrayBasicModel::BeamId const & item);

} /* namespace ns3 */

#endif /* SRC_ANTENNA_ARRAY_BASIC_H_ */
