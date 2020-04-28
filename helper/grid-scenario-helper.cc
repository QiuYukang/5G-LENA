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

#include "grid-scenario-helper.h"
#include <ns3/position-allocator.h>
#include <ns3/mobility-helper.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <math.h>
#include "ns3/core-module.h"
#include "ns3/mobility-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GridScenarioHelper");

// Site positions in terms of distance and angle w.r.t. the central site
static std::vector<double> siteDistances {0,1,1,1,1,1,1,std::sqrt(3),std::sqrt(3),std::sqrt(3),std::sqrt(3),std::sqrt(3),std::sqrt(3),2,2,2,2,2,2};
static std::vector<double> siteAngles {0,30,90,150,210,270,330,0,60,120,180,240,300,30,90,150,210,270,330};

static const double MAX_ANTENNA_OFFSET = 1;  //!< Maximum distance between a sector antenna panel and the site it belongs to

/**
 * \brief Creates a GNUPLOT with the hexagonal deployment including base stations
 * (BS), their hexagonal cell areas and user terminals (UT). Positions and cell
 * radius must be given in meters
 *
 * \param sitePosVector Vector of site positions
 * \param cellCenterVector Vector of cell center positions
 * \param utPosVector Vector of user terminals positions
 * \param cellRadius Hexagonal cell radius in meters
 */
static void
PlotHexagonalDeployment (const Ptr<const ListPositionAllocator> &sitePosVector,
                         const Ptr<const ListPositionAllocator> &cellCenterVector,
                         const Ptr<const ListPositionAllocator> &utPosVector,
                         double cellRadius)
{

  NS_ASSERT (sitePosVector->GetSize() > 0);
  NS_ASSERT (cellCenterVector->GetSize() > 0);
  NS_ASSERT (utPosVector->GetSize() > 0);

  // Try to open a new GNUPLOT file
  std::ofstream topologyOutfile;
  NS_LOG_LOGIC ("generate gnuplottable topology file");
  std::string topologyFileName = "./hexagonal-topology.gnuplot";
  topologyOutfile.open (topologyFileName.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!topologyOutfile.is_open ())
    {
      NS_ABORT_MSG ("Can't open " << topologyFileName);
    }

  uint16_t numCells = cellCenterVector->GetSize ();
  uint16_t numSites = sitePosVector->GetSize ();
  uint16_t numSectors = numCells / numSites;
  NS_ASSERT (numSectors > 0);
  uint16_t numUts = utPosVector->GetSize ();

  topologyOutfile << "set term eps" << std::endl;
  topologyOutfile << "set output \"" << topologyFileName << ".pdf\"" << std::endl;
  topologyOutfile << "set style arrow 1 lc \"black\" lt 1 head filled" << std::endl;
//  topologyOutfile << "set autoscale" << std::endl;

  uint16_t margin = (8 * cellRadius) + 1;  //!< This is the farthest hexagonal vertex from the cell center
  topologyOutfile << "set xrange [-" << margin << ":" << margin <<"]" << std::endl;
  topologyOutfile << "set yrange [-" << margin << ":" << margin <<"]" << std::endl;
  //FIXME: Need to recalculate ranges if the scenario origin is different to (0,0)

  double arrowLength = cellRadius/4.0;  //<! Control the arrow length that indicates the orientation of the sectorized antenna
  std::vector<double> hx {0.0,-0.5,-0.5,0.0,0.5,0.5,0.0};   //<! Hexagon vertices in x-axis
  std::vector<double> hy {-1.0,-0.5,0.5,1.0,0.5,-0.5,-1.0}; //<! Hexagon vertices in y-axis
  Vector sitePos;

  for (uint16_t cellId = 0; cellId < numCells; ++cellId)
    {
      Vector cellPos = cellCenterVector->GetNext ();
      double angleDeg = 30 + 120 * (cellId % 3);
      double angleRad = angleDeg * M_PI / 180;
      double x, y;

      NS_LOG_LOGIC ("output gnuplottable arrow indicating macro cell antenna boresight");
      if (cellId % numSectors == 0)
        {
          sitePos = sitePosVector->GetNext ();
        }
      topologyOutfile << "set arrow " << cellId + 1 << " from " << sitePos.x
          << "," << sitePos.y << " rto " << arrowLength * std::cos(angleRad)
      << "," << arrowLength * std::sin(angleRad) << " arrowstyle 1 \n";

      NS_LOG_LOGIC ("output gnuplottable hexagon for macroCellId " << cellId);
      // Draw the hexagon arond the cell center
      topologyOutfile << "set object " << cellId + 1 << " polygon from \\\n";

      for (uint16_t vertexId = 0; vertexId <= 6; ++vertexId)
        {
          // angle of the vertex w.r.t. y-axis
          x = cellRadius * std::sqrt(3.0) * hx.at (vertexId) + cellPos.x;
          y = cellRadius * hy.at (vertexId) + cellPos.y;
          topologyOutfile << x << ", " << y;
          if (vertexId == 6)
            {
              topologyOutfile << " front fs empty \n";
            }
          else
            {
              topologyOutfile << " to \\\n";
            }
        }

      topologyOutfile << "set label " << cellId + 1 << " \"" << (cellId + 1) <<
          "\" at " << cellPos.x << " , " << cellPos.y << " center" << std::endl;

    }

  NS_LOG_LOGIC ("output gnuplottable location of UTs as points");

  for (uint16_t utId = 0; utId < numUts; ++utId)
    {
      Vector utPos = utPosVector->GetNext ();
//      set label at xPos, yPos, zPos "" point pointtype 7 pointsize 2
      topologyOutfile << "set label at " << utPos.x << " , " << utPos.y <<
          " point pointtype 7 pointsize 0.2 center" << std::endl;
    }

   topologyOutfile << "unset key" << std::endl; //!< Disable plot legends
   topologyOutfile << "plot 1/0" << std::endl;  //!< Need to plot a function

}


GridScenarioHelper::GridScenarioHelper ()
{
  m_initialPos.x = 0.0;
  m_initialPos.y = 0.0;
  m_initialPos.z = 0.0;
}

GridScenarioHelper::~GridScenarioHelper ()
{

}

void
GridScenarioHelper::SetHorizontalBsDistance (double d)
{
  m_horizontalBsDistance = d;
}

void
GridScenarioHelper::SetVerticalBsDistance (double d)
{
  m_verticalBsDistance = d;
}

void
GridScenarioHelper::SetRows (uint32_t r)
{
  m_rows = r;
}

void
GridScenarioHelper::SetColumns (uint32_t c)
{
  m_columns = c;
}

void GridScenarioHelper::SetScenarioLength(double m)
{
  m_length = m;
}

void GridScenarioHelper::SetScenarioHeight(double m)
{
  m_height = m;
}

void
GridScenarioHelper::CreateScenario ()
{
  NS_ASSERT (m_rows > 0);
  NS_ASSERT (m_columns > 0);
  NS_ASSERT (m_bsHeight >= 0.0);
  NS_ASSERT (m_utHeight >= 0.0);
  NS_ASSERT (m_bs.GetN () > 0);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> bsPos = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> utPos = CreateObject<ListPositionAllocator> ();

  // BS position
  if (m_bs.GetN () > 0)
    {
      uint32_t bsN = m_bs.GetN ();
      for (uint32_t i = 0; i < m_rows; ++i)
        {
          for (uint32_t j = 0; i < m_columns; ++j)
            {
              if (bsN == 0)
                {
                  break;
                }

              Vector pos (m_initialPos);
              pos.z = m_bsHeight;

              pos.x = m_initialPos.x + (i * m_horizontalBsDistance);
              pos.y = m_initialPos.y + (j * m_verticalBsDistance);

              NS_LOG_DEBUG ("GNB Position: " << pos);
              bsPos->Add (pos);

              bsN--;
            }
        }
    }

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (0.0));
  x->SetAttribute ("Max", DoubleValue (m_height));
  y->SetAttribute ("Min", DoubleValue (0.0));
  y->SetAttribute ("Max", DoubleValue (m_length));
  // UT position
  if (m_ut.GetN () > 0)
    {
      uint32_t utN = m_ut.GetN ();

      for (uint32_t i = 0; i < utN; ++i)
        {
          Vector pos = bsPos->GetNext ();

          pos.x += x->GetValue ();
          pos.y += y->GetValue ();
          pos.z = m_utHeight;

          NS_LOG_DEBUG ("UE Position: " << pos);

          utPos->Add (pos);
        }
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (bsPos);
  mobility.Install (m_bs);

  mobility.SetPositionAllocator (utPos);
  mobility.Install (m_ut);
}


HexagonalGridScenarioHelper::HexagonalGridScenarioHelper ()
{
  m_centralPos.x = 0.0;
  m_centralPos.y = 0.0;
  m_centralPos.z = 0.0;
}

HexagonalGridScenarioHelper::~HexagonalGridScenarioHelper ()
{

}

void
HexagonalGridScenarioHelper::SetNumRings (uint8_t numRings)
{
  NS_ABORT_MSG_IF(numRings > 3, "Unsupported number of outer rings (Maximum is 3");

  m_numRings = numRings;

  switch (numRings)
  {
    case 0:
      m_numSites = 1;
      break;
    case 1:
      m_numSites = 7;
      break;
    case 2:
      m_numSites = 13;
      break;
    case 3:
      m_numSites = 19;
      break;
  }
}

void
HexagonalGridScenarioHelper::SetSectorization (SiteSectorizationType numSectors)
{
  m_siteSectorization = numSectors;
}

uint8_t
HexagonalGridScenarioHelper::GetNumSites ()
{
  return m_numSites;
}

uint16_t
HexagonalGridScenarioHelper::GetNumCells ()
{
  return m_numCells;
}

SiteSectorizationType
HexagonalGridScenarioHelper::GetNumSectorsPerSite ()
{
  return m_siteSectorization;
}

double
HexagonalGridScenarioHelper::GetHexagonalCellRadius ()
{
  return m_hexagonalRadius;
}

void
HexagonalGridScenarioHelper::SetNumCells ()
{
  NS_ASSERT (m_numSites > 0);
  NS_ASSERT (m_siteSectorization != SiteSectorizationType::NONE);

  m_numCells = m_numSites * static_cast<uint16_t> (m_siteSectorization);

  NS_ASSERT (m_numCells > 0);

  m_bs.Create (m_numCells);
}

double
HexagonalGridScenarioHelper::GetAntennaOrientationDegrees (uint16_t cellId,
                                                           SiteSectorizationType numSectors)
{
  NS_ABORT_MSG_IF (numSectors != SINGLE && numSectors != TRIPLE, "Unsupported number of site sectors");

  double orientation = 0.0;
  if (numSectors == TRIPLE)
    {
      uint16_t sector = cellId % static_cast<uint16_t> (numSectors);
      double sectorSize = 360 / numSectors;
      orientation = 30 + sectorSize*sector;
    }
  return orientation;
}

double
HexagonalGridScenarioHelper::GetAntennaOrientationRadians (uint16_t cellId,
                                                           SiteSectorizationType numSectors)
{
  double orientationRads = GetAntennaOrientationDegrees (cellId, numSectors) * M_PI / 180;
  if (orientationRads > M_PI)
    {
      orientationRads -= 2 * M_PI;
    }

  return orientationRads;
}

Vector
HexagonalGridScenarioHelper::GetHexagonalCellCenter (const Vector &sitePos,
                                                     uint16_t cellId,
                                                     SiteSectorizationType numSectors,
                                                     double hexagonRadius)
{
  Vector center (sitePos);
  uint16_t siteNum = cellId;

  switch (numSectors)
  {
    case SiteSectorizationType::NONE:
      NS_ABORT_MSG ("Number of sectors has not been defined");
      break;

    case SiteSectorizationType::SINGLE:
      break;

    case SiteSectorizationType::TRIPLE:
      siteNum = cellId % (static_cast<uint16_t> (numSectors));
      if (siteNum == 0)
        {
          center.x += hexagonRadius * std::sqrt (0.75);
          center.y += hexagonRadius / 2;
        }
      else if (siteNum == 1)
        {
          center.x -= hexagonRadius * std::sqrt (0.75);
          center.y += hexagonRadius / 2;
        }
      else
        {
          center.y -= hexagonRadius;
        }
      break;

    default:
      NS_ABORT_MSG("Unsupported number of sectors");
      break;
  }

  return center;
}


uint16_t
HexagonalGridScenarioHelper::GetSiteIndex (uint16_t cellId)
{
  return cellId / static_cast<uint16_t> (m_siteSectorization);
}

void
HexagonalGridScenarioHelper::SetScenarioParamenters (const std::string &scenario)
{
  NS_ABORT_MSG_IF(scenario != "UMa" && scenario != "UMi", "Unrecognized scenario");

  if (scenario == "UMa")
    {
      SetUMaParameters ();
    }
  else if (scenario == "UMi")
    {
      SetUMiParameters ();
    }
  else
    {
      NS_ABORT_MSG ("Should never be here");
    }

}

void
HexagonalGridScenarioHelper::SetUMaParameters ()
{
  m_isd = 500;
  m_bsHeight = 25.0;
  m_utHeight = 1.5;
  m_siteSectorization = SiteSectorizationType::TRIPLE;
  m_hexagonalRadius = m_isd / 3;
  m_minBsUtdistance = 35;
  m_antennaOffset = 1.0;
}

void
HexagonalGridScenarioHelper::SetUMiParameters ()
{
  m_isd = 200;
  m_bsHeight = 25.0;
  m_utHeight = 1.5;
  m_siteSectorization = SiteSectorizationType::TRIPLE;
  m_hexagonalRadius = m_isd / 3;
  m_minBsUtdistance = 10;
  m_antennaOffset = 1.0;
}

Vector
HexagonalGridScenarioHelper::GetAntennaPos (const Vector &sitePos,
                                            uint16_t cellId,
                                            SiteSectorizationType numSectors,
                                            double antennaOffset)
{

  NS_ABORT_MSG_IF (antennaOffset > MAX_ANTENNA_OFFSET, "Antenna offset is too large");

  Vector pos (sitePos);

  double angle = GetAntennaOrientationDegrees(cellId, numSectors);
  pos.x += antennaOffset * cos (angle * M_PI / 180);
  pos.y += antennaOffset * sin (angle * M_PI / 180);
  return pos;
}

void
HexagonalGridScenarioHelper::CreateScenario ()
{
  NS_ASSERT (m_isd > 0);
  NS_ASSERT (m_numRings < 4);
  NS_ASSERT (m_numCells > 0);
  NS_ASSERT (m_siteSectorization > 0);
  NS_ASSERT (m_hexagonalRadius > 0);
  NS_ASSERT (m_bsHeight >= 0.0);
  NS_ASSERT (m_utHeight >= 0.0);
  NS_ASSERT (m_bs.GetN () > 0);
  NS_ASSERT (m_ut.GetN () > 0);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> bsPosVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> bsCenterVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> sitePosVector = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> utPosVector = CreateObject<ListPositionAllocator> ();

  // BS position
  for (uint16_t cellIndex = 0; cellIndex < m_numCells; cellIndex++)
    {
      uint16_t siteIndex = GetSiteIndex (cellIndex);
      Vector sitePos (m_centralPos);
      sitePos.x += m_isd * siteDistances.at(siteIndex) * cos(siteAngles.at(siteIndex) * M_PI / 180);
      sitePos.y += m_isd * siteDistances.at(siteIndex) * sin(siteAngles.at(siteIndex) * M_PI / 180);
      sitePos.z = m_bsHeight;

      if (cellIndex % static_cast<uint16_t> (m_siteSectorization) == 0)
        {
          sitePosVector->Add (sitePos);
        }

      // FIXME: Until sites can have more than one antenna array, it is necessary to apply some distance offset from the site center (gNBs cannot have the same location)
      Vector bsPos = GetAntennaPos (sitePos,
                                  cellIndex,
                                  m_siteSectorization,
                                  m_antennaOffset);

      NS_LOG_DEBUG ("GNB Position: " << bsPos);
      bsPosVector->Add (bsPos);

      // Store cell center position for plotting the deployment
      Vector cellCenterPos = GetHexagonalCellCenter (bsPos,
                                                     cellIndex,
                                                     m_siteSectorization,
                                                     m_hexagonalRadius);
      bsCenterVector->Add (cellCenterPos);

      //What about the antenna orientation? It should be dealt with when installing the gNB
    }

  // To allocate UEs, I need the center of the hexagonal cell. Allocate UE around the disk of radius isd/3
  Ptr<UniformRandomVariable> r = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> theta = CreateObject<UniformRandomVariable> ();
  r->SetAttribute ("Min", DoubleValue (0));
  r->SetAttribute ("Max", DoubleValue (m_hexagonalRadius * std::sqrt(3) / 2 - m_minBsUtdistance));  //Spread UEs inside the inner hexagonal radius
  theta->SetAttribute ("Min", DoubleValue (-1.0 * M_PI));
  theta->SetAttribute ("Max", DoubleValue (M_PI));
  // UT position
  if (m_ut.GetN () > 0)
    {
      uint32_t utN = m_ut.GetN ();

      for (uint32_t i = 0; i < utN; ++i)
        {
          // This is the cell center location, same for cells belonging to the same site
          Vector cellPos = bsPosVector->GetNext ();
          // UEs shall be spread over the cell area (hexagonal cell)
          uint16_t cellId = i % m_numCells;
          Vector cellCenterPos = GetHexagonalCellCenter (cellPos,
                                                         cellId,
                                                         m_siteSectorization,
                                                         m_hexagonalRadius);

          double d = r->GetValue ();
          double t = theta->GetValue ();

          Vector utPos (cellCenterPos);
          utPos.x += d * cos (t);
          utPos.y += d * sin (t);
          utPos.z = m_utHeight;

          NS_LOG_DEBUG ("UE Position: " << utPos);

          utPosVector->Add (utPos);
        }
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (bsPosVector);
  mobility.Install (m_bs);

  mobility.SetPositionAllocator (utPosVector);
  mobility.Install (m_ut);

  PlotHexagonalDeployment (sitePosVector, bsCenterVector, utPosVector, m_hexagonalRadius);

}


} // namespace ns3
