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
#include "ns3/core-module.h"
#include "ns3/mobility-model.h"

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

} // namespace ns3
