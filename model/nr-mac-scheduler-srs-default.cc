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
#include "nr-mac-scheduler-srs-default.h"

#include <ns3/uinteger.h>

#include <algorithm>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerSrsDefault");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerSrsDefault);

NrMacSchedulerSrsDefault::NrMacSchedulerSrsDefault ()
{
  m_random = CreateObject<UniformRandomVariable> ();
}

NrMacSchedulerSrsDefault::~NrMacSchedulerSrsDefault ()
{}

TypeId
NrMacSchedulerSrsDefault::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerSrsDefault")
    .SetParent<Object> ()
    .AddAttribute ("StartingPeriodicity",
                   "Starting value for the periodicity",
                   UintegerValue (80),
                   MakeUintegerAccessor (&NrMacSchedulerSrsDefault::SetStartingPeriodicity,
                                         &NrMacSchedulerSrsDefault::GetStartingPeriodicity),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

NrMacSchedulerSrs::SrsPeriodicityAndOffset
NrMacSchedulerSrsDefault::AddUe ()
{
  NS_LOG_FUNCTION (this);
  SrsPeriodicityAndOffset ret;

  if (m_usedOffsets.size () == m_periodicity)
    {
      return ret; // ret will be invalid
    }

  std::pair<std::set<uint32_t>::iterator, bool> v;
  uint32_t offset;

  // worst case: this loop never ends :(
  do
    {
      offset = m_random->GetValue (0, m_periodicity - 1);
      v = m_usedOffsets.insert (offset);
    }
  while (v.second == false);

  ret.m_offset = offset;
  ret.m_periodicity = m_periodicity;
  ret.m_isValid = true;

  return ret;
}

bool
NrMacSchedulerSrsDefault::IncreasePeriodicity (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap)
{
  NS_LOG_FUNCTION (this);
  static std::vector<uint32_t> StandardPeriodicity = {
    2, 4, 5, 8, 10, 16, 20, 32, 40, 64, 80, 160, 320, 640, 1280, 2560
  };

  m_usedOffsets.clear ();
  auto it = std::upper_bound (StandardPeriodicity.begin (), StandardPeriodicity.end (),
                              m_periodicity);
  if (it == StandardPeriodicity.end ())
    {
      return false;
    }

  m_periodicity = *it;
  ReassignSrsValue (ueMap);

  return true;
}

bool
NrMacSchedulerSrsDefault::DecreasePeriodicity (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap)
{
  NS_LOG_FUNCTION (this);

  static std::vector<uint32_t> StandardPeriodicity = {
    2, 4, 5, 8, 10, 16, 20, 32, 40, 64, 80, 160, 320, 640, 1280, 2560
  };

  m_usedOffsets.clear ();
  auto it = std::lower_bound (StandardPeriodicity.begin (), StandardPeriodicity.end (),
                              m_periodicity);
  if (it == StandardPeriodicity.end ())
    {
      return false;
    }

  m_periodicity = *it;
  ReassignSrsValue (ueMap);
  return true;
}

void
NrMacSchedulerSrsDefault::SetStartingPeriodicity (uint32_t start)
{
  NS_ABORT_MSG_IF (m_usedOffsets.size () != 0,
                   "We already started giving offset to UEs, you cannot alter the periodicity");
  m_periodicity = start;
}

uint32_t
NrMacSchedulerSrsDefault::GetStartingPeriodicity () const
{
  return m_periodicity;
}

void
NrMacSchedulerSrsDefault::ReassignSrsValue (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_usedOffsets.size () == 0);

  for (auto & ue : *ueMap)
    {
      auto srs = AddUe ();

      NS_ASSERT (srs.m_isValid);

      ue.second->m_srsPeriodicity = srs.m_periodicity;
      ue.second->m_srsOffset = srs.m_offset;
    }
}

} // namespace ns3
