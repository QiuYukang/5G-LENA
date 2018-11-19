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

#ifndef ANTENNA_ARRAY_3GPP_MODEL_H_
#define ANTENNA_ARRAY_3GPP_MODEL_H_
#include <ns3/antenna-model.h>
#include <complex>
#include <ns3/net-device.h>
#include "antenna-array-model.h"
#include <map>

namespace ns3 {

class AntennaArray3gppModel : public AntennaArrayModel
{
public:

  enum GnbAntennaMount
  {
    GnbWallMount,
    GnbSingleSector
  };

  AntennaArray3gppModel ();

  virtual ~AntennaArray3gppModel ();

  static TypeId GetTypeId ();

  /**
   * \brief Must override the function to set 0 gain,
   * since the gain is already accounted for with GetRadiationPattern function
   * @param angles angles of gain
   * @return gain which is in this case always 0
   */
  virtual double GetGainDb (Angles a) override;

  /**
   * \brief Function sets the attribute m_isUe that is used to determine
   * which configuration parameter for antenna will be used, i.e. for UE or gNB
   * @param isUe whether the antenna is of UE or gNB
   */
  void SetIsUe (bool isUe);

  /**
   * \brief Function returns a boolean that determines whether the the antenna
   * if of UE or gNB
   * @return boolean value where true means UE, false gNB
   */
  bool GetIsUe ();

  virtual double GetRadiationPattern (double vAngle, double hAngle = 0) override;

  Vector GetAntennaLocation (uint8_t index, uint8_t* antennaNum) override;

private:

  bool m_isUe; ///<! the attribute that is saying if the antenna is of UE or gNB
  GnbAntennaMount m_antennaMount; ///<! the type of gNb antenna mount

};

std::ostream & operator<< (std::ostream & os, AntennaArrayModel::BeamId const & item);

} /* namespace ns3 */

#endif /* SRC_ANTENNA_3GPP_ANTENNA_MODEL_H_ */
