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



#include "antenna-array-model.h"
#include <ns3/log.h>
#include <ns3/math.h>
#include <ns3/simulator.h>
#include "ns3/double.h"
#include "ns3/enum.h"

NS_LOG_COMPONENT_DEFINE ("AntennaArrayModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (AntennaArrayModel);

static const double Enb22DegreeBFVectorReal[8][64] = {
  {1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,},
  {1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,},
  {1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,},
  {1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,},
  {1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,1.000000,0.817987,0.338204,-0.264694,-0.771236,-0.997028,-0.859874,-0.409703,},
  {1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,1.000000,-0.173694,-0.939661,0.500120,0.765926,-0.766193,-0.499760,0.939803,},
  {1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,1.000000,-0.863083,0.489825,0.017564,-0.520144,0.880290,-0.999383,0.844811,},
  {1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,1.000000,-0.998179,0.992721,-0.983647,0.970990,-0.954796,0.935123,-0.912045,},
};
static const double Enb22DegreeBFVectorImag[8][64] = {
  {-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,-0.000000,-0.060328,0.120437,-0.180106,0.239120,-0.297262,0.354322,-0.410091,},
  {-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,-0.000000,-0.505062,0.871821,-0.999846,0.854079,-0.474436,-0.035123,0.535065,},
  {-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,-0.000000,-0.984800,0.342107,0.865956,-0.642929,-0.642611,0.866164,0.341717,},
  {-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,-0.000000,-0.575237,-0.941073,-0.964332,-0.636549,-0.077045,0.510506,0.912219,},
  {0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,-0.000000,0.575237,0.941073,0.964332,0.636549,0.077045,-0.510506,-0.912219,},
  {0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,-0.000000,0.984800,-0.342107,-0.865956,0.642929,0.642611,-0.866164,-0.341717,},
  {0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,-0.000000,0.505062,-0.871821,0.999846,-0.854079,0.474436,0.035123,-0.535065,},
  {0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,-0.000000,0.060328,-0.120437,0.180106,-0.239120,0.297262,-0.354322,0.410091,},
};

static const double Ue45DegreeBFVectorReal[4][16] = {
  {1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,},
  {1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,},
  {1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,1.000000,0.360273,-0.740406,-0.893771,},
  {1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,1.000000,-0.971542,0.887788,-0.753505,},
};
static const double Ue45DegreeBFVectorImag[4][16] = {
  {-0.000000,-0.236867,0.460252,-0.657442,-0.000000,-0.236867,0.460252,-0.657442,-0.000000,-0.236867,0.460252,-0.657442,-0.000000,-0.236867,0.460252,-0.657442,},
  {-0.000000,-0.932847,-0.672160,0.448524,-0.000000,-0.932847,-0.672160,0.448524,-0.000000,-0.932847,-0.672160,0.448524,-0.000000,-0.932847,-0.672160,0.448524,},
  {0.000000,0.932847,0.672160,-0.448524,-0.000000,0.932847,0.672160,-0.448524,-0.000000,0.932847,0.672160,-0.448524,-0.000000,0.932847,0.672160,-0.448524,},
  {0.000000,0.236867,-0.460252,0.657442,-0.000000,0.236867,-0.460252,0.657442,-0.000000,0.236867,-0.460252,0.657442,-0.000000,0.236867,-0.460252,0.657442,},
};

static const double All90DegreeBFVectorReal[2][4] = {
  {1.000000,-0.605700,1.000000,-0.605700,},
  {1.000000,-0.605700,1.000000,-0.605700,},
};
static const double All90DegreeBFVectorImag[2][4] = {
  {-0.000000,-0.795693,-0.000000,-0.795693,},
  {0.000000,0.795693,-0.000000,0.795693,},
};


AntennaArrayModel::AntennaArrayModel ()
  : AntennaArrayBasicModel (),
    m_minAngle (0),
    m_maxAngle (2 * M_PI)
{
  m_omniTx = false;
}

AntennaArrayModel::~AntennaArrayModel ()
{

}

TypeId
AntennaArrayModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::AntennaArrayModel")
    .SetParent<AntennaArrayBasicModel> ()
    .AddConstructor<AntennaArrayModel> ()
    .AddAttribute ("AntennaHorizontalSpacing",
                   "Horizontal spacing between antenna elements, in multiples of lambda",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&AntennaArrayModel::m_disH),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AntennaVerticalSpacing",
                   "Vertical spacing between antenna elements, in multiples of lambda",
                   DoubleValue (0.5),
                   MakeDoubleAccessor (&AntennaArrayModel::m_disV),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AntennaOrientation",
                   "The orentation of the antenna",
                   EnumValue (AntennaOrientation::X0),
                   MakeEnumAccessor(&AntennaArrayModel::SetAntennaOrientation,
                                    &AntennaArrayModel::GetAntennaOrientation),
                   MakeEnumChecker(AntennaOrientation::X0, "X0",
                                   AntennaOrientation::Z0, "Z0",
                                   AntennaOrientation::Y0, "Y0"))
    .AddAttribute ("AntennaGain",
                   "Antenna gain in dBi",
                   DoubleValue (0),
                   MakeDoubleAccessor (&AntennaArrayModel::m_antennaGain),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

double
AntennaArrayModel::GetGainDb (Angles a)
{
  return m_antennaGain;
}
void
AntennaArrayModel::SetBeamformingVectorWithDelay (complexVector_t antennaWeights, BeamId beamId,
                                                  Ptr<NetDevice> device)
{
  Simulator::Schedule (MilliSeconds (8), &AntennaArrayModel::SetBeamformingVector,
                       this, antennaWeights, beamId, device);
}


void
AntennaArrayModel::SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                                         Ptr<NetDevice> device)
{
  m_omniTx = false;
  if (device != nullptr)
    {
      BeamformingStorage::iterator iter = m_beamformingVectorMap.find (device);
      if (iter != m_beamformingVectorMap.end ())
        {
          (*iter).second = std::make_pair (antennaWeights, beamId);
        }
      else
        {
          m_beamformingVectorMap.insert (std::make_pair (device,
                                                         std::make_pair (antennaWeights, beamId)));
        }
    }
  m_currentBeamformingVector = std::make_pair (antennaWeights, beamId);
}

void
AntennaArrayModel::ChangeBeamformingVector (Ptr<NetDevice> device)
{
  m_omniTx = false;
  BeamformingStorage::iterator it = m_beamformingVectorMap.find (device);
  NS_ASSERT_MSG (it != m_beamformingVectorMap.end (), "could not find");
  m_currentBeamformingVector = it->second;
}

AntennaArrayModel::BeamformingVector
AntennaArrayModel::GetCurrentBeamformingVector ()
{
  NS_ABORT_MSG_IF (m_omniTx, "omi transmission do not need beamforming vector");
  return m_currentBeamformingVector;
}

void
AntennaArrayModel::ChangeToOmniTx ()
{
  m_omniTx = true;
}

bool
AntennaArrayModel::IsOmniTx ()
{
  return m_omniTx;
}


AntennaArrayModel::BeamformingVector
AntennaArrayModel::GetBeamformingVector (Ptr<NetDevice> device)
{
  AntennaArrayModel::BeamformingVector beamformingVector;
  BeamformingStorage::iterator it = m_beamformingVectorMap.find (device);
  if (it != m_beamformingVectorMap.end ())
    {
      beamformingVector = it->second;
    }
  else
    {
      beamformingVector = m_currentBeamformingVector;
    }
  return beamformingVector;
}

void
AntennaArrayModel::SetToSector (uint32_t sector, uint32_t antennaNum)
{
  NS_LOG_LOGIC(this);
  m_omniTx = false;
  complexVector_t cmplxVector;
  switch (antennaNum)
    {
    case 64:
      {
        if (sector == 0 || sector == 1 || sector == 14 || sector == 15)
          {
            m_minAngle = -0.5 * M_PI;
            m_maxAngle = 0.5 * M_PI;
          }
        else if (sector == 2 || sector == 3 || sector == 4 || sector == 5)
          {
            m_minAngle = 0;
            m_maxAngle = M_PI;
          }
        else if (sector == 6 || sector == 7 || sector == 8 || sector == 9)
          {
            m_minAngle = 0.5 * M_PI;
            m_maxAngle = 1.5 * M_PI;
          }

        else if (sector == 10 || sector == 11 || sector == 12 || sector == 13)
          {
            m_minAngle = -1 * M_PI;
            m_maxAngle = 0;
          }
        else
          {
            NS_FATAL_ERROR ("64 antenna only need 16 sectors");

          }

        if (sector > 7)
          {
            sector = 15 - sector;
          }
        for (unsigned int i = 0; i < antennaNum; i++)
          {
            std::complex<double> cmplx (Enb22DegreeBFVectorReal[sector][i],Enb22DegreeBFVectorImag[sector][i]);
            cmplxVector.push_back (cmplx);
          }
        break;
      }
    case 16:
      {
        if (sector == 0 || sector == 7)
          {
            m_minAngle = -0.5 * M_PI;
            m_maxAngle = 0.5 * M_PI;
          }
        else if (sector == 1 || sector == 2)
          {
            m_minAngle = 0;
            m_maxAngle = M_PI;
          }
        else if (sector == 3 || sector == 4)
          {
            m_minAngle = 0.5 * M_PI;
            m_maxAngle = 1.5 * M_PI;
          }
        else if (sector == 5 || sector == 6)
          {
            m_minAngle = -1 * M_PI;
            m_maxAngle = 0;
          }
        else
          {
            NS_FATAL_ERROR ("16 antenna only need 8 sectors");

          }
        if (sector > 3)
          {
            sector = 7 - sector;
          }
        for (unsigned int i = 0; i < antennaNum; i++)
          {
            std::complex<double> cmplx (Ue45DegreeBFVectorReal[sector][i],Ue45DegreeBFVectorImag[sector][i]);
            cmplxVector.push_back (cmplx);
          }
        break;
      }
    case 4:
      {
        if (sector == 0)
          {
            m_minAngle = 0;
            m_maxAngle = 0.5 * M_PI;
          }
        else if (sector == 1)
          {
            m_minAngle = 0.5 * M_PI;
            m_maxAngle = M_PI;
          }
        else if (sector == 2)
          {
            m_minAngle = -1 * M_PI;
            m_maxAngle = -0.5 * M_PI;
          }
        else if (sector == 3)
          {
            m_minAngle = -0.5 * M_PI;
            m_maxAngle = 0;
          }
        else
          {
            NS_FATAL_ERROR ("4 antenna only need 4 sectors");
          }
        if (sector > 1)
          {
            sector = 3 - sector;
          }
        for (unsigned int i = 0; i < antennaNum; i++)
          {
            std::complex<double> cmplx (All90DegreeBFVectorReal[sector][i],All90DegreeBFVectorImag[sector][i]);
            cmplxVector.push_back (cmplx);
          }
        break;
      }
    default:
      {
        NS_FATAL_ERROR ("the antenna number should only be 64 or 16");
      }
    }

  //normalize antennaWeights;
  double weightSum = 0;
  for (unsigned int i = 0; i < antennaNum; i++)
    {
      weightSum += pow (std::abs (cmplxVector.at (i)),2);
    }
  for (unsigned int i = 0; i < antennaNum; i++)
    {
      cmplxVector.at (i) = cmplxVector.at (i) / sqrt (weightSum);
    }
  // Wow.. we miss theta here. Default theta = 0.
  m_currentBeamformingVector = std::make_pair (cmplxVector, std::make_pair (sector, 0));
}

double
AntennaArrayModel::GetRadiationPattern (double vAngle, double hAngle)
{
  NS_ASSERT_MSG (vAngle >= 0&&vAngle <= 180, "the vertical angle should be the range of [0,180]");
  NS_ASSERT_MSG (hAngle >= -180&&vAngle <= 180, "the vertical angle should be the range of [0,180]");
  return 1;       //for testing
}

Vector
AntennaArrayModel::GetAntennaLocation (uint8_t index, uint8_t* antennaNum)
{
  Vector loc;

  if (m_orientation == AntennaOrientation::X0)
    {
      //assume the left bottom corner is (0,0,0), and the rectangular antenna array is on the y-z plane.
      loc.x = 0;
      loc.y = m_disH * (index % antennaNum[0]);
      loc.z = m_disV * floor (index / antennaNum[0]);
    }
  else if (m_orientation == AntennaOrientation::Z0)
    {
      //assume the left bottom corner is (0,0,0), and the rectangular antenna array is on the x-y plane.
      loc.z = 0;
      loc.x = m_disH * (index % antennaNum[0]);
      loc.y = m_disV * floor (index / antennaNum[0]);
    }
  else
    {
      NS_ABORT_MSG("Not defined antenna orientation");
    }

  return loc;
}

void
AntennaArrayModel::SetSector (uint8_t sector, uint8_t *antennaNum, double elevation)
{
  complexVector_t tempVector;
  double hAngle_radian = M_PI * (double)sector / (double)antennaNum[1] - 0.5 * M_PI;
  double vAngle_radian = elevation * M_PI / 180;
  uint16_t size = antennaNum[0] * antennaNum[1];
  double power = 1 / sqrt (size);
  for (int ind = 0; ind < size; ind++)
    {
      Vector loc = GetAntennaLocation (ind, antennaNum);
      double phase = -2 * M_PI * (sin (vAngle_radian) * cos (hAngle_radian) * loc.x
                                  + sin (vAngle_radian) * sin (hAngle_radian) * loc.y
                                  + cos (vAngle_radian) * loc.z);
      tempVector.push_back (exp (std::complex<double> (0, phase)) * power);
    }
  m_currentBeamformingVector = std::make_pair (tempVector, std::make_pair (sector, elevation));
}

std::ostream &operator<< (std::ostream &os, const AntennaArrayModel::BeamId &item)
{
  os << "[Sector: " << static_cast<uint32_t> (AntennaArrayModel::GetSector (item))
     << " elevation: " << AntennaArrayModel::GetElevation (item) << "]";
  return os;
}


void AntennaArrayModel::SetAntennaOrientation (enum AntennaArrayModel::AntennaOrientation orientation)
{
  m_orientation = orientation;
}

enum AntennaArrayModel::AntennaOrientation
AntennaArrayModel::GetAntennaOrientation () const
{
  return m_orientation;
}



} /* namespace ns3 */
