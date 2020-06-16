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
#ifndef NR_MAC_SCHEDULER_SRS_DEFAULT_H
#define NR_MAC_SCHEDULER_SRS_DEFAULT_H

#include <ns3/object.h>
#include <ns3/random-variable-stream.h>

#include "nr-mac-scheduler-srs.h"

#include <set>

namespace ns3 {

class NrMacSchedulerSrsDefault : public NrMacSchedulerSrs, public Object
{
public:
  NrMacSchedulerSrsDefault ();
  virtual ~NrMacSchedulerSrsDefault ();

  static TypeId GetTypeId ();

  virtual SrsPeriodicityAndOffset AddUe (void) override;
  virtual bool IncreasePeriodicity (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap) override;
  virtual bool DecreasePeriodicity (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap) override;

  void SetStartingPeriodicity (uint32_t start);
  uint32_t GetStartingPeriodicity () const;

private:
  void ReassignSrsValue (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap);

private:
  uint32_t m_periodicity {0};
  std::set<uint32_t> m_usedOffsets;
  Ptr<UniformRandomVariable> m_random;
};

} // namespace ns3

#endif // NR_MAC_SCHEDULER_SRS_DEFAULT_H
