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

#include "nr-radio-environment-map-helper.h"

#include <ns3/abort.h>
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/uinteger.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/node.h>
#include <ns3/constant-position-mobility-model.h>


#include <fstream>
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRadioEnvironmentMapHelper");

NS_OBJECT_ENSURE_REGISTERED (NrRadioEnvironmentMapHelper);


NrRadioEnvironmentMapHelper::NrRadioEnvironmentMapHelper ()
{
}

NrRadioEnvironmentMapHelper::~NrRadioEnvironmentMapHelper ()
{
}


void
NrRadioEnvironmentMapHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrRadioEnvironmentMapHelper::GetTypeId (void)
{
  NS_LOG_FUNCTION ("NrRadioEnvironmentMapHelper::GetTypeId");
  static TypeId tid = TypeId ("ns3::NrRadioEnvironmentMapHelper")
                      .SetParent<Object> ()
                      .SetGroupName("Nr")
                      .AddConstructor<NrRadioEnvironmentMapHelper> ()
                      .AddAttribute ("OutputFile",
                                     "the filename to which the NR Radio"
                                     "Environment Map is saved",
                                     StringValue ("NR_REM.out"),
                                     MakeStringAccessor (&NrRadioEnvironmentMapHelper::m_outputFile),
                                     MakeStringChecker ())
                      .AddAttribute ("XMin",
                                     "The min x coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMinX,
                                                         &NrRadioEnvironmentMapHelper::GetMinX),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("YMin",
                                     "The min y coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMinY,
                                                         &NrRadioEnvironmentMapHelper::GetMinY),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("XMax",
                                     "The max x coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMaxX,
                                                         &NrRadioEnvironmentMapHelper::GetMaxX),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("YMax",
                                     "The max y coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMaxY,
                                                         &NrRadioEnvironmentMapHelper::GetMaxY),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("XRes",
                                     "The resolution (number of points) of the"
                                     "map along the x axis.",
                                     UintegerValue (100),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetResX,
                                                           &NrRadioEnvironmentMapHelper::GetResX),
                                     MakeUintegerChecker<uint32_t> (2,std::numeric_limits<uint16_t>::max ()))
                      .AddAttribute ("YRes",
                                     "The resolution (number of points) of the"
                                     "map along the y axis.",
                                     UintegerValue (100),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetResY,
                                                           &NrRadioEnvironmentMapHelper::GetResY),
                                     MakeUintegerChecker<uint16_t> (2,std::numeric_limits<uint16_t>::max ()))
                      .AddAttribute ("Z",
                                     "The value of the z coordinate for which"
                                     "the map is to be generated.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetZ,
                                                         &NrRadioEnvironmentMapHelper::GetZ),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("MaxPointsPerIteration",
                                     "Maximum number of REM points to be "
                                     "calculated per iteration.",
                                     UintegerValue (20000),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetMaxPointsPerIt,
                                                           &NrRadioEnvironmentMapHelper::GetMaxPointsPerIt),
                                     MakeUintegerChecker<uint32_t> (1,std::numeric_limits<uint32_t>::max ()))
    ;
  return tid;
}


void
NrRadioEnvironmentMapHelper::SetMinX (double xMin)
{
  m_xMin = xMin;
}

void
NrRadioEnvironmentMapHelper::SetMinY (double yMin)
{
  m_yMin = yMin;
}

void
NrRadioEnvironmentMapHelper::SetMaxX (double xMax)
{
  m_xMax = xMax;
}

void
NrRadioEnvironmentMapHelper::SetMaxY (double yMax)
{
  m_yMax = yMax;
}

void
NrRadioEnvironmentMapHelper::SetResX (uint16_t xRes)
{
  m_xRes = xRes;
}

void
NrRadioEnvironmentMapHelper::SetResY (uint16_t yRes)
{
  m_yRes = yRes;
}

void
NrRadioEnvironmentMapHelper::SetZ (double z)
{
  m_z = z;
}

void
NrRadioEnvironmentMapHelper::SetMaxPointsPerIt (uint32_t maxPointsPerIt)
{
  m_maxPointsPerIteration = maxPointsPerIt;
}

double
NrRadioEnvironmentMapHelper::GetMinX () const
{
  return m_xMin;
}

double
NrRadioEnvironmentMapHelper::GetMinY () const
{
  return m_yMin;
}

double
NrRadioEnvironmentMapHelper::GetMaxX () const
{
  return m_xMax;
}

double
NrRadioEnvironmentMapHelper::GetMaxY () const
{
  return m_yMax;
}

uint16_t
NrRadioEnvironmentMapHelper::GetResX () const
{
  return m_xRes;
}

uint16_t
NrRadioEnvironmentMapHelper::GetResY () const
{
  return m_yRes;
}

double
NrRadioEnvironmentMapHelper::GetZ () const
{
  return m_z;
}

uint32_t
NrRadioEnvironmentMapHelper::GetMaxPointsPerIt () const
{
  return m_maxPointsPerIteration;
}

void
NrRadioEnvironmentMapHelper::DelayedInstall ()
{
  NS_LOG_FUNCTION (this);

  m_outFile.open (m_outputFile.c_str ());
  if (!m_outFile.is_open ())
    {
      NS_FATAL_ERROR ("Can't open file " << (m_outputFile));
      return;
    }

  m_xStep = (m_xMax - m_xMin)/(m_xRes-1);
  m_yStep = (m_yMax - m_yMin)/(m_yRes-1);

  if ((double)m_xRes * (double) m_yRes < (double) m_maxPointsPerIteration)
    {
      m_maxPointsPerIteration = m_xRes * m_yRes;
    }

  for (uint32_t i = 0; i < m_maxPointsPerIteration; ++i)
    {
      RemPoint p;
      p.bmm = CreateObject<ConstantPositionMobilityModel> ();
      m_rem.push_back (p);
    }

  double remIterationStartTime = 0.0001;
  double xMinNext = m_xMin;
  double yMinNext = m_yMin;
  uint32_t numPointsCurrentIteration = 0;
  bool justScheduled = false;
  for (double x = m_xMin; x < m_xMax + 0.5*m_xStep; x += m_xStep)
    {
      for (double y = m_yMin; y < m_yMax + 0.5*m_yStep ; y += m_yStep)
        {
          if (justScheduled)
            {
              xMinNext = x;
              yMinNext = y;
              justScheduled = false;
            }

          ++numPointsCurrentIteration;
          if ((numPointsCurrentIteration == m_maxPointsPerIteration)
              || ((x > m_xMax - 0.5*m_xStep) && (y > m_yMax - 0.5*m_yStep)) )
            {
              Simulator::Schedule (Seconds (remIterationStartTime),
                                   &NrRadioEnvironmentMapHelper::RunOneIteration,
                                   this, xMinNext, x, yMinNext, y);
              remIterationStartTime += 0.001;
              justScheduled = true;
              numPointsCurrentIteration = 0;
            }
        }
    }

  Simulator::Schedule (Seconds (remIterationStartTime),
                       &NrRadioEnvironmentMapHelper::Finalize,
                       this);
}

void
NrRadioEnvironmentMapHelper::RunOneIteration (double xMin, double xMax, double yMin, double yMax)
{
  NS_LOG_FUNCTION (this << xMin << xMax << yMin << yMax);

  std::list<RemPoint>::iterator remIt = m_rem.begin ();

  double x = 0.0;
  double y = 0.0;

  for (x = xMin; x < xMax + 0.5*m_xStep; x += m_xStep)
    {
      for (y = (x == xMin) ? yMin : m_yMin;
           y < ((x == xMax) ? yMax : m_yMax) + 0.5*m_yStep;
           y += m_yStep)
        {
          NS_ASSERT (remIt != m_rem.end ());
          remIt->bmm->SetPosition (Vector (x, y, m_z));
          ++remIt;
        }
    }

  if (remIt != m_rem.end ())
    {
      NS_ASSERT ((x > m_xMax - 0.5*m_xStep) && (y > m_yMax - 0.5*m_yStep));
      NS_LOG_LOGIC ("deactivating RemSpectrumPhys that are unneeded in the last iteration");
    }

  Simulator::Schedule (Seconds (0.0005), &NrRadioEnvironmentMapHelper::PrintAndReset, this);
}

void
NrRadioEnvironmentMapHelper::PrintAndReset ()
{
  NS_LOG_FUNCTION (this);

  for (std::list<RemPoint>::iterator it = m_rem.begin ();
       it != m_rem.end ();
       ++it)
    {

      Vector pos = it->bmm->GetPosition ();
      NS_LOG_LOGIC ("output: " << pos.x << "\t" << pos.y << "\t" << pos.z << "\t");
      m_outFile << pos.x << "\t"
                << pos.y << "\t"
                << pos.z << "\t"
                << std::endl;
    }
}

void
NrRadioEnvironmentMapHelper::Finalize ()
{
  NS_LOG_FUNCTION (this);
  m_outFile.close ();

  //Simulator::Stop ();
}


} // namespace ns3
