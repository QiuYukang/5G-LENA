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
*
*/



#include "antenna-array-basic-model.h"

#include <ns3/log.h>
#include <ns3/math.h>
#include <ns3/simulator.h>
#include "ns3/double.h"


NS_LOG_COMPONENT_DEFINE ("AntennaArrayBasicModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (AntennaArrayBasicModel);


AntennaArrayBasicModel::AntennaArrayBasicModel ()
{
}

AntennaArrayBasicModel::~AntennaArrayBasicModel ()
{

}

TypeId
AntennaArrayBasicModel::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::AntennaArrayBasicModel")
    .SetParent<Object> ()
    .SetGroupName("Antenna")
  ;
  return tid;
}

double
AntennaArrayBasicModel::GetGainDb (Angles a)
{
  NS_ABORT_MSG("Function not implemented, should not be called.");
  return 0;
}


} /* namespace ns3 */
