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

double
AntennaArray3gppModel::GetGainDb (Angles a)
{
  return 0;
}

void
AntennaArray3gppModel::SetIsUe (bool isUe)
{
  NS_LOG_INFO("Set 3GPP antenna model parameters for "<< ((isUe)?"UE":"gNB"));
  m_isUe = isUe;
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

  NS_ASSERT_MSG (vAngle >= 0 && vAngle <= 180, "The vertical angle should be in the range of [0,180]");
  NS_ASSERT_MSG (hAngle >= -180 && hAngle <= 180, "The horizontal angle should be in the range of [-180,180]");

  double A = 0 ;

  if (m_isUe)
    {
      double gMax = 5;  // maximum directional gain of an antenna element in dBi according to UE antenna radiation pattern in 38.802 table A.2.1-8
      double hpbw = 90; //HPBW value of each antenna element
      double A_M = 25;  //front-back ratio expressed in dB
      double SLA = 25;  //side-lobe level limit expressed in dB
      double A_v= -1 * std::min (12 * pow ((vAngle - 90) / hpbw, 2), SLA);
      double A_h = -1 * std::min (12 * pow (hAngle / hpbw, 2), A_M);

      A = gMax - 1 * std::min (-1 * A_v - 1 * A_h, A_M);
    }
  else
    {
      double gMax = 5;  // maximum directional gain of an antenna element of Wall Mount radiation pattern (38.802 table A.2.1.7)
      double hpbw = 90; //HPBW value of each antenna element
      double A_M = 25;  //front-back ratio expressed in dB
      double SLA = 25;  //side-lobe level limit expressed in dB
      double A_v= -1 * std::min (12 * pow ((vAngle - 90) / hpbw, 2), SLA);
      double A_h = -1 * std::min (12 * pow (hAngle / hpbw, 2), A_M);

      A = gMax - 1 * std::min (A_M, -1 * A_v - 1 * A_h);
    }

  return sqrt (pow (10, A / 10)); //field factor term converted to linear;
}



} /* namespace ns3 */
