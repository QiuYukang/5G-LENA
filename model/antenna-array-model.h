/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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
*
*   Author: Marco Mezzavilla < mezzavilla@nyu.edu>
*                Sourjya Dutta <sdutta@nyu.edu>
*                Russell Ford <russell.ford@nyu.edu>
*                Menglei Zhang <menglei@nyu.edu>
*/

#ifndef ANTENNA_ARRAY_MODEL_H_
#define ANTENNA_ARRAY_MODEL_H_
#include <ns3/antenna-model.h>
#include <complex>
#include <ns3/net-device.h>
#include <map>

namespace ns3 {

class AntennaArrayModel : public AntennaModel
{
public:
  static TypeId GetTypeId ();
  typedef std::vector< std::complex<double> > complexVector_t;
  typedef std::pair<uint8_t, double> BeamId;
  typedef std::pair<complexVector_t, BeamId>  BeamformingVector;

  /*!
   * \brief Get weight vector from a BeamformingVector
   * \param v the BeamformingVector
   * \return the weight vector
   */
  static complexVector_t GetVector (BeamformingVector v)
  {
    return v.first;
  }
  /*!
   * \return the beam id
   * \param v the beamforming vector
   */
  static BeamId GetBeamId (BeamformingVector v)
  {
    return v.second;
  }
  /*!
   * \return The sector of the beam
   * \param b beam
   */
  static uint8_t GetSector (BeamId b)
  {
    return b.first;
  }
  /*!
   * \return the elevation of the beam
   * \param b the beam
   */
  static double GetElevation (BeamId b)
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
    size_t operator() (const AntennaArrayModel::BeamId &x) const
    {
      return std::hash<uint32_t>()(Cantor (x.first, static_cast<uint32_t> (x.second)));
    }
  };

  AntennaArrayModel ();
  virtual ~AntennaArrayModel ();
  virtual double GetGainDb (Angles a);
  void SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                             Ptr<NetDevice> device = nullptr);
  void SetBeamformingVectorWithDelay (complexVector_t antennaWeights, BeamId beamId,
                                      Ptr<NetDevice> device = nullptr);

  void ChangeBeamformingVector (Ptr<NetDevice> device);
  void ChangeToOmniTx ();
  BeamformingVector GetCurrentBeamformingVector ();
  BeamformingVector GetBeamformingVector (Ptr<NetDevice> device);
  void SetToSector (uint32_t sector, uint32_t antennaNum);
  bool IsOmniTx ();
  double GetRadiationPattern (double vangle, double hangle = 0);
  Vector GetAntennaLocation (uint8_t index, uint8_t* antennaNum);
  void SetSector (uint8_t sector, uint8_t *antennaNum, double elevation = 90);

private:
  typedef std::map<Ptr<NetDevice>, BeamformingVector> BeamformingStorage;
  bool m_omniTx;
  double m_minAngle;
  double m_maxAngle;
  BeamformingVector m_currentBeamformingVector;
  BeamformingStorage m_beamformingVectorMap;

  double m_disV;       //antenna spacing in the vertical direction in terms of wave length.
  double m_disH;       //antenna spacing in the horizontal direction in terms of wave length.
};

std::ostream & operator<< (std::ostream & os, AntennaArrayModel::BeamId const & item);

} /* namespace ns3 */

#endif /* SRC_ANTENNA_MODEL_ANTENNA_ARRAY_MODEL_H_ */
