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

#ifndef SRC_NR_MODEL_BEAM_ID_H_
#define SRC_NR_MODEL_BEAM_ID_H_

#include <stdint.h>
#include <map>
#include <complex>
#include <unordered_map>

namespace ns3 {

 /**
   * \brief Representation of a beam id
   *
   * A beam id in ns-3 is a pair that contains the sector, stored as a uint16_t,
   * and the elevation, stored as a double. Utilities functions are provided to
   * extract the values. This ID usually comes with the real physical representation
   * of a Beam, expressed by BeamformingVector.
   *
   * \see GetSector
   * \see GetElevation
   */

  class BeamId {

    uint16_t m_sector {0};
    double m_elevation {0};

  public:

    BeamId (){};

    BeamId (uint16_t sector, double elevation)
    {
      m_sector = sector;
      m_elevation = elevation;
    }

    /**
     * \brief Objects of this class are used as key in hash
     * table. This class must implement operator ==()
     * to handle collisions.
     */
    inline bool operator==(const BeamId& p) const
    {
      return m_sector == p.GetSector() && m_elevation == p.GetElevation();
    }


    /**
     * \brief Extract the sector from the beam id
     * \return The sector of the beam
     * \param b beam
     */
    inline uint16_t GetSector () const
    {
      return m_sector;
    }

    /**
     * \brief Extract the elevation from the beam id
     * \return the elevation of the beam
     * \param b the beam
     */
    inline double GetElevation () const
    {
      return m_elevation;
    }

  };

  // we reserve pair 65535, 65535 to identify the OMNI beam
  const BeamId OMNI_BEAM_ID = BeamId (UINT16_MAX, UINT16_MAX);

  /**
   * \brief Calculate the Cantor function for two unsigned int
   * \param x1 first value max value 65535
   * \param x2 second value max value 65535
   * \return \f$ (((x1 + x2) * (x1 + x2 + 1))/2) + x2; \f$ max value 4294836225
   */
  static constexpr uint32_t Cantor (uint16_t x1, uint16_t x2)
  {
    return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
  }

  /**
   * \brief Calculate the hash of a BeamId
   */
  struct BeamIdHash
  {
    size_t operator() (const BeamId &x) const
    {
      return std::hash<uint32_t>()(Cantor (x.GetSector(), static_cast<uint16_t> (x.GetElevation())));
    }
  };

  std::ostream &operator<< (std::ostream &os, const BeamId &item);

} /* namespace ns3 */

#endif /* SRC_NR_MODEL_ID_MANAGER_H_ */
