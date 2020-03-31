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
*/


#include "mmwave-harq-phy.h"
#include <ns3/log.h>
#include <ns3/assert.h>

NS_LOG_COMPONENT_DEFINE ("MmWaveHarqPhy");

namespace ns3 {

MmWaveHarqPhy::~MmWaveHarqPhy ()
{
  NS_LOG_FUNCTION (this);
  m_dlHistory.clear ();
  m_ulHistory.clear ();
}

void MmWaveHarqPhy::SetHarqNum (uint32_t harqNum)
{
  NS_LOG_FUNCTION (this);
  m_harqNum = harqNum;
  for (auto & v : m_dlHistory)
    {
      v.second.resize (m_harqNum);
    }
  for (auto & v : m_ulHistory)
    {
      v.second.resize (m_harqNum);
    }
}

const NrErrorModel::NrErrorModelHistory &
MmWaveHarqPhy::GetHarqProcessInfoDl (uint16_t rnti, uint8_t harqProcId)
{
  NS_LOG_FUNCTION (this);

  auto it = m_dlHistory.find (rnti);
  if (it == m_dlHistory.end ())
    {
      // new entry
      m_dlHistory.insert(std::make_pair (rnti, std::vector<NrErrorModel::NrErrorModelHistory> ()));
      it = m_dlHistory.find (rnti);
      it->second.resize (m_harqNum);
      return it->second.at (harqProcId);
    }
  else
    {
      return (it->second.at (harqProcId));
    }
}

const NrErrorModel::NrErrorModelHistory &
MmWaveHarqPhy::GetHarqProcessInfoUl (uint16_t rnti, uint8_t harqProcId)
{
  NS_LOG_FUNCTION (this);

  auto it = m_ulHistory.find (rnti);
  if (it == m_ulHistory.end ())
    {
      // new entry

      m_ulHistory.insert(std::make_pair (rnti, std::vector<NrErrorModel::NrErrorModelHistory> ()));
      it = m_ulHistory.find (rnti);
      it->second.resize (m_harqNum);
      return it->second.at (harqProcId);
    }
  else
    {
      return (it->second.at (harqProcId));
    }
}

void
MmWaveHarqPhy::UpdateDlHarqProcessStatus (uint16_t rnti, uint8_t harqProcId,
                                          const Ptr<NrErrorModelOutput> &output)
{
  NS_LOG_FUNCTION (this);

  auto it = m_dlHistory.find (rnti);
  if (it == m_dlHistory.end ())
    {
      // new entry
      m_dlHistory.insert(std::make_pair (rnti, std::vector<NrErrorModel::NrErrorModelHistory> ()));
      it = m_dlHistory.find (rnti);
      it->second.resize (m_harqNum);

      it->second.at (harqProcId).emplace_back (output);
    }
  else
    {
      it->second.at (harqProcId).emplace_back (output);
    }
}


void
MmWaveHarqPhy::ResetDlHarqProcessStatus (uint16_t rnti, uint8_t id)
{
  NS_LOG_FUNCTION (this);

  auto it = m_dlHistory.find (rnti);
  if (it == m_dlHistory.end ())
    {
      // new entry
      m_dlHistory.insert(std::make_pair (rnti, std::vector<NrErrorModel::NrErrorModelHistory> ()));
      it = m_dlHistory.find (rnti);
      it->second.resize (m_harqNum);
    }
  else
    {
      it->second.at (id).clear ();
    }
}

void
MmWaveHarqPhy::UpdateUlHarqProcessStatus (uint16_t rnti, uint8_t harqProcId,
                                          const Ptr<NrErrorModelOutput> &output)
{
  NS_LOG_FUNCTION (this);

  auto it = m_ulHistory.find (rnti);
  if (it == m_ulHistory.end ())
    {
      // new entry
      m_ulHistory.insert(std::make_pair (rnti, std::vector<NrErrorModel::NrErrorModelHistory> ()));
      it = m_ulHistory.find (rnti);
      it->second.resize (m_harqNum);

      it->second.at (harqProcId).emplace_back (output);
    }
  else
    {
      it->second.at (harqProcId).emplace_back (output);
    }
}

void
MmWaveHarqPhy::ResetUlHarqProcessStatus (uint16_t rnti, uint8_t id)
{
  NS_LOG_FUNCTION (this);

  auto it = m_ulHistory.find (rnti);
  if (it == m_ulHistory.end ())
    {
      // new entry
      m_ulHistory.insert(std::make_pair (rnti, std::vector<NrErrorModel::NrErrorModelHistory> ()));
      it = m_ulHistory.find (rnti);
      it->second.resize (m_harqNum);
    }
  else
    {
      it->second.at (id).clear ();
    }
}




} // end namespace
