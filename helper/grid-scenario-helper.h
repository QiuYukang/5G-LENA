/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef GRID_SCENARIO_HELPER_H
#define GRID_SCENARIO_HELPER_H

#include "node-distribution-scenario-interface.h"
#include <ns3/vector.h>

namespace ns3 {


/**
 * @brief The GridScenarioHelper class
 *
 * TODO: Documentation, tests
 */
class GridScenarioHelper : public NodeDistributionScenarioInterface
{
public:
  /**
   * \brief GridScenarioHelper
   */
  GridScenarioHelper ();

  /**
   * \brief ~GridScenarioHelper
   */
  virtual ~GridScenarioHelper () override;

  /**
   * @brief SetHorizontalBsDistance
   */
  void SetHorizontalBsDistance (double d);

  /**
   * @brief SetVerticalBsDistance
   */
  void SetVerticalBsDistance (double d);

  /**
   * @brief SetRows
   */
  void SetRows (uint32_t r);

  /**
   * @brief SetColumns
   */
  void SetColumns (uint32_t c);

  void SetStartingPosition (const Vector &initialPos);

  void SetScenarioLength (double m);

  void SetScenarioHeight (double m);

  // inherited
  virtual void CreateScenario () override;

private:
  double m_verticalBsDistance {-1.0}; //!< Distance between gnb
  double m_horizontalBsDistance {-1.0}; //!< Distance between gnb
  uint32_t m_rows {0};        //!< Grid rows
  uint32_t m_columns {0};     //!< Grid columns
  Vector m_initialPos;        //!< Initial Position
  double m_length {0};        //!< Scenario length
  double m_height {0};        //!< Scenario height
};



/**
 * \brief Type of site sectorization
 */
enum SiteSectorizationType
{
  NONE = 0,   //!< Unconfigured value
  SINGLE = 1, //!< Site with a 360º-width sector
  TRIPLE = 3  //!< Site with 3 120º-width sectors
};


/**
 * @brief The HexagonalGridScenarioHelper class
 *
 * TODO: Documentation, tests
 */
class HexagonalGridScenarioHelper : public NodeDistributionScenarioInterface
{
public:
  /**
   * \brief HexagonalGridScenarioHelper
   */
  HexagonalGridScenarioHelper ();

  /**
   * \brief ~HexagonalGridScenarioHelper
   */
  virtual ~HexagonalGridScenarioHelper () override;

  /**
   * \brief Sets the number of outer rings of sites around the central site
   */
  void SetNumRings (uint8_t numRings);

  /**
   * \brief Sets the number of sectors of every site.
   * \param numSectors Number of sectors. Values can be 1 or 3.
   */
  void SetSectorization (SiteSectorizationType numSectors);

  /**
   * \brief Gets the number of cells deployed
   * \return Number of sites in the network deployment
   */
  uint8_t GetNumSites ();

  /**
   * \brief Gets the number of cells deployed
   * \return Number of cells in the network deployment
   */
  uint16_t GetNumCells ();

  /**
   * \brief Gets the number of sectors per site
   */
  SiteSectorizationType GetNumSectorsPerSite ();

  /**
   * \brief Gets the radius of the hexagonal cell
   * \returns Cell radius in meters
   */
  double GetHexagonalCellRadius ();

  /**
   * \brief Sets the number of cells from the previously configured number of outer rings or sites and the site sectorization
   */
  void SetNumCells ();

  /**
   * \brief Returns the orientation in degrees of the antenna array for the given cellId and number of sectors of the site the cell belongs to
   * \param cellId Cell Id
   * \param numSecors The number of sectors of a site
   * \return The antenna orientation in degrees [0º, 360º]
   */
  double GetAntennaOrientationDegrees (uint16_t cellId,
                                       SiteSectorizationType numSectors);

  /**
     * \brief Returns the orientation in radians of the antenna array for the given cellId and number of sectors of the site the cell belongs to
     * \param cellId Cell Id
     * \param numSecors The number of sectors of a site
     * \return The antenna orientation in radians [-PI, PI]
     */
    double GetAntennaOrientationRadians (uint16_t cellId,
                                         SiteSectorizationType numSectors);


  /**
   * \brief Returns the cell center coordinates
   * \param sitePos Site position coordinates
   * \param cellId Cell Id
   * \param numSecors The number of sectors of a site
   * \param hexagonRadius Radius of the hexagonal cell
   */
  Vector GetHexagonalCellCenter (const Vector &sitePos,
                                 uint16_t cellId,
                                 SiteSectorizationType numSectors,
                                 double hexagonRadius);

  /**
   * \brief Gets the site index the queried cell id belongs to
   * \param cellId Cell index
   * \return site id
   */
  uint16_t GetSiteIndex (uint16_t cellId);

  /**
   * \brief Returns the position of the cell antenna
   * \param sitePos Site position coordinates in meters
   * \param cell Id Cell id of the antenna
   * \param numSectors Number of sectors of the site
   * \param antennaOffset Distance in meters between the antenna and the site locations
   */
  Vector GetAntennaPos (const Vector &sitePos,
                        uint16_t cellId,
                        SiteSectorizationType numSectors,
                        double antennaOffset);

  /**
   * \brief Sets parameters to the specified scenario
   * \param scenario Scenario to simulate
   */
  void SetScenarioParamenters (const std::string &scenario);

  /**
   * \brief Sets the Urban Macro (UMa) scenario parameters
   */
  void SetUMaParameters ();

  /**
   * \brief Sets the Urban Micro (UMi) scenario parameters
   */
  void SetUMiParameters ();

  /**
   * \brief
   */
  // inherited
  virtual void CreateScenario () override;

private:
  double m_isd {-1.0};     //!< Inter-site distance (ISD) in meters, constant distance among neighboring sites
  uint8_t m_numRings {0};  //!< Number of outer rings of sites around the central site
  uint16_t m_numSites {0}; //!< Number of sites
  uint16_t m_numCells {0}; //!< Number of cells
  SiteSectorizationType m_siteSectorization {NONE};  //!< Number of sectors per site
  Vector m_centralPos;     //!< Central site position
  double m_antennaOffset {-1.0};   // Cell antenna offset in meters w.r.t. site location
  double m_hexagonalRadius {0.0};  //!< Cell radius
};

} // namespace ns3
#endif // GRID_SCENARIO_HELPER_H
