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
#include "antenna-array-basic-model.h"

namespace ns3 {

class AntennaArrayModel : public AntennaArrayBasicModel
{
public:

  /**
   * \brief Predefined antenna orientation options
   */
  enum AntennaOrientation{
    X0,//!< X0 Means that antenna's X axis is set to 0, hence the antenna is placed in Z-Y plane
    Z0,//!< Z0 Means that antenna's Z axis is set to 0, hence the antenna is placed in X-Y plane
    Y0 //!< Y0 Means that antenna's Y axis is set to 0, hence the antenna is placed in X-Z plane
  };

  AntennaArrayModel ();

  virtual ~AntennaArrayModel ();

  static TypeId GetTypeId ();

  virtual double GetGainDb (Angles a) override;

  virtual void SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                             Ptr<NetDevice> device = nullptr);

  virtual void SetBeamformingVectorWithDelay (complexVector_t antennaWeights, BeamId beamId,
                                      Ptr<NetDevice> device = nullptr);

  virtual void ChangeBeamformingVector (Ptr<NetDevice> device);

  virtual void ChangeToOmniTx ();

  virtual BeamformingVector GetCurrentBeamformingVector ();

  virtual BeamformingVector GetBeamformingVector (Ptr<NetDevice> device);

  virtual void SetToSector (uint32_t sector, uint32_t antennaNum);

  virtual bool IsOmniTx ();

  virtual double GetRadiationPattern (double vangle, double hangle = 0);

  virtual Vector GetAntennaLocation (uint8_t index, uint8_t* antennaNum);

  virtual void SetSector (uint8_t sector, uint8_t *antennaNum, double elevation = 90);

  void SetAntennaOrientation (enum AntennaArrayModel::AntennaOrientation orientation);

  enum AntennaArrayModel::AntennaOrientation GetAntennaOrientation () const;


private:

  typedef std::map<Ptr<NetDevice>, BeamformingVector> BeamformingStorage;
  bool m_omniTx;
  double m_minAngle;
  double m_maxAngle;
  BeamformingVector m_currentBeamformingVector;
  BeamformingStorage m_beamformingVectorMap;

protected:
  double m_disV;       //antenna spacing in the vertical direction in terms of wave length.
  double m_disH;       //antenna spacing in the horizontal direction in terms of wave length.
  AntennaOrientation m_orientation; // antenna orientation, for example, when set to "X0" (x=0) it means that the antenna will be in y-z plane
  double m_antennaGain; //antenna gain
};

std::ostream & operator<< (std::ostream & os, AntennaArrayModel::BeamId const & item);

} /* namespace ns3 */

#endif /* SRC_ANTENNA_MODEL_ANTENNA_ARRAY_MODEL_H_ */
