/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
*/



#include <ns3/log.h>
#include <ns3/math.h>
#include <ns3/simulator.h>
#include "antenna-array-3gpp-model.h"
#include "ns3/double.h"


NS_LOG_COMPONENT_DEFINE ("AntennaArray3gppModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (AntennaArray3gppModel);


AntennaArray3gppModel::AntennaArray3gppModel ()
  : AntennaArrayModel ()
{
}

AntennaArray3gppModel::~AntennaArray3gppModel ()
{

}

TypeId
AntennaArray3gppModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::AntennaArray3gppModel")
    .SetParent<AntennaArrayModel> ()
    .AddConstructor<AntennaArray3gppModel> ()
  ;
  return tid;
}

void
AntennaArray3gppModel::SetIsUe (bool isUe)
{
  if (isUe)
    {
      m_hpbw = 90;           //HPBW value of each antenna element
      m_gMax = 5;           //directivity value expressed in dBi and valid only for TRP (see table A.1.6-3 in 38.802
    }
  else
    {
      m_hpbw = 65;           //HPBW value of each antenna element
      m_gMax = 8;           //directivity value expressed in dBi and valid only for TRP (see table A.1.6-3 in 38.802
    }
}


double
AntennaArray3gppModel::GetRadiationPattern (double vAngleRadian, double hAngleRadian)
{
  while (hAngleRadian >= M_PI)
    {
      hAngleRadian -= 2 * M_PI;
    }
  while (hAngleRadian < -M_PI)
    {
      hAngleRadian += 2 * M_PI;
    }

  double vAngle = vAngleRadian * 180 / M_PI;
  double hAngle = hAngleRadian * 180 / M_PI;
  //NS_LOG_INFO(" it is " << vAngle);
  NS_ASSERT_MSG (vAngle >= 0&&vAngle <= 180, "the vertical angle should be the range of [0,180]");
  //NS_LOG_INFO(" it is " << hAngle);
  NS_ASSERT_MSG (hAngle >= -180&&hAngle <= 180, "the horizontal angle should be the range of [-180,180]");

  double A_M = 30;       //front-back ratio expressed in dB
  double SLA = 30;       //side-lobe level limit expressed in dB

  double A_v = -1 * std::min (SLA,12 * pow ((vAngle - 90) / m_hpbw,2));      //TODO: check position of z-axis zero
  double A_h = -1 * std::min (A_M,12 * pow (hAngle / m_hpbw,2));
  double A = m_gMax - 1 * std::min (A_M,-1 * A_v - 1 * A_h);

  return sqrt (pow (10,A / 10));     //filed factor term converted to linear;
}


} /* namespace ns3 */
