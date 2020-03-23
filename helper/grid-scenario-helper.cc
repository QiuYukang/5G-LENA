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

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GridScenarioHelper");

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

          pos.x = x->GetValue ();
          pos.y = y->GetValue ();
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
HexagonalGridScenarioHelper::GetAntennaOrientation (uint16_t cellId,
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

Vector
HexagonalGridScenarioHelper::GetHexagonalCellCenter (Vector sitePos,
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

void
HexagonalGridScenarioHelper::SetUMaParameters ()
{
  m_isd = 500;
  m_bsHeight = 25.0;
  m_utHeight = 1.5;
  m_siteSectorization = SiteSectorizationType::TRIPLE;
  m_hexagonalRadius = m_isd / 2 / 3;
}

void
HexagonalGridScenarioHelper::SetUMiParameters ()
{
  m_isd = 200;
  m_bsHeight = 25.0;
  m_utHeight = 1.5;
  m_siteSectorization = SiteSectorizationType::TRIPLE;
  m_hexagonalRadius = m_isd / 2 / 3;
}

void
HexagonalGridScenarioHelper::CreateScenario ()
{
  NS_ASSERT (m_isd > 0);
  NS_ASSERT (m_numRings > 0 && m_numRings < 4);
  NS_ASSERT (m_numCells > 0);
  NS_ASSERT (m_siteSectorization > 0);
  NS_ASSERT (m_hexagonalRadius > 0);
  NS_ASSERT (m_bsHeight >= 0.0);
  NS_ASSERT (m_utHeight >= 0.0);
  NS_ASSERT (m_bs.GetN () > 0);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> bsPos = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> utPos = CreateObject<ListPositionAllocator> ();

  // Site positions in terms of distance and angle w.r.t. the central site
  float val = 2*cos (30 * M_PI / 180);
  std::vector<float> siteDistances {0,1,1,1,1,1,1,val,val,val,val,val,val,2,2,2,2,2,2};
  std::vector<float> siteAngles {0,30,90,150,210,270,330,0,60,120,180,240,300,30,90,150,210,270,330};

  // BS position
  for (uint16_t cellIndex = 0; cellIndex < m_numSites; cellIndex++)
    {
        uint16_t siteIndex = cellIndex % static_cast<uint16_t> (m_siteSectorization);
        Vector pos (m_centralPos);
        pos.x += 0.5 * m_isd * siteDistances.at(siteIndex) * cos(siteAngles.at(siteIndex)*M_PI/180);
        pos.y += 0.5 * m_isd * siteDistances.at(siteIndex) * sin(siteAngles.at(siteIndex)*M_PI/180);
        pos.z = m_bsHeight;

        NS_LOG_DEBUG ("GNB Position: " << pos);
        bsPos->Add (pos);

        //What about the antenna orientation? It should be dealt with when installing the gNB
    }

  //TODO: To allocate UEs, I need the center of the hexagonal cell. Allocate UE around the disk of radius isd/3
  Ptr<UniformRandomVariable> r = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> theta = CreateObject<UniformRandomVariable> ();
  r->SetAttribute ("Min", DoubleValue (0.0));
  r->SetAttribute ("Max", DoubleValue (m_hexagonalRadius));
  theta->SetAttribute ("Min", DoubleValue (0.0));
  theta->SetAttribute ("Max", DoubleValue (360.0));
  // UT position
  if (m_ut.GetN () > 0)
    {
      uint32_t utN = m_ut.GetN ();

      for (uint32_t i = 0; i < utN; ++i)
        {
          // This is the cell center location, same for cells belonging to the same site
          Vector cellPos = bsPos->GetNext ();
          // UEs shall be spread over the cell area (hexagonal cell)
          uint16_t cellId = i % m_numCells;
          Vector cellCenterPos = GetHexagonalCellCenter (cellPos,
                                                         cellId,
                                                         m_siteSectorization,
                                                         m_hexagonalRadius);
          float d = r->GetValue ();
          float t = theta->GetValue ();

          Vector pos (cellCenterPos);
          pos.x += d * cos (t * M_PI / 180);
          pos.y += d * sin (t * M_PI / 180);
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


} // namespace ns3
