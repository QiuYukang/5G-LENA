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
#include "sfnsf.h"

namespace ns3 {

SfnSf::SfnSf(uint16_t frameNum, uint8_t sfNum, uint16_t slotNum, uint8_t varTtiNum)
  : m_frameNum (frameNum),
    m_subframeNum (sfNum),
    m_slotNum (slotNum),
    m_varTtiNum (varTtiNum)
{
}

uint64_t
SfnSf::Encode() const
{
  uint64_t ret = 0ULL;
  ret = (static_cast<uint64_t> (m_frameNum) << 32 ) |
      (static_cast<uint64_t> (m_subframeNum) << 24) |
      (static_cast<uint64_t> (m_slotNum) << 8) |
      (static_cast<uint64_t> (m_varTtiNum));
  return ret;
}

uint32_t
SfnSf::GetSubframesPerFrame()
{
  return 10;
}

uint64_t
SfnSf::Encode(const SfnSf &p)
{
  uint64_t ret = 0ULL;
  ret = (static_cast<uint64_t> (p.m_frameNum) << 32 ) |
      (static_cast<uint64_t> (p.m_subframeNum) << 24) |
      (static_cast<uint64_t> (p.m_slotNum) << 8) |
      (static_cast<uint64_t> (p.m_varTtiNum));
  return ret;
}

void
SfnSf::Decode(uint64_t sfn)
{
  m_frameNum    = (sfn & 0x0000FFFF00000000) >> 32;
  m_subframeNum = (sfn & 0x00000000FF000000) >> 24;
  m_slotNum     = (sfn & 0x0000000000FFFF00) >> 8;
  m_varTtiNum   = (sfn & 0x00000000000000FF);
}

SfnSf
SfnSf::FromEncoding(uint64_t sfn)
{
  SfnSf ret;
  ret.m_frameNum    = (sfn & 0x0000FFFF00000000) >> 32;
  ret.m_subframeNum = (sfn & 0x00000000FF000000) >> 24;
  ret.m_slotNum     = (sfn & 0x0000000000FFFF00) >> 8;
  ret.m_varTtiNum   = (sfn & 0x00000000000000FF);
  return ret;
}

SfnSf
SfnSf::IncreaseNoOfSlots(uint32_t slotsPerSubframe) const
{
  return IncreaseNoOfSlotsWithLatency (1, slotsPerSubframe);
}

SfnSf
SfnSf::CalculateUplinkSlot(uint32_t k2Delay, uint32_t slotsPerSubframe) const
{
  return IncreaseNoOfSlotsWithLatency (k2Delay, slotsPerSubframe);
}

SfnSf
SfnSf::IncreaseNoOfSlotsWithLatency(uint32_t latency, uint32_t slotsPerSubframe) const
{
  SfnSf retVal = *this;
  // currently the default value of L1L2 latency is set to 2 and is interpreted as in the number of slots
  // will be probably reduced to order of symbols
  retVal.m_frameNum += (this->m_subframeNum + (this->m_slotNum + latency) / slotsPerSubframe) / GetSubframesPerFrame ();
  retVal.m_subframeNum = (this->m_subframeNum + (this->m_slotNum + latency) / slotsPerSubframe) % GetSubframesPerFrame ();
  retVal.m_slotNum = (this->m_slotNum + latency) % slotsPerSubframe;
  return retVal;
}

uint64_t
SfnSf::Normalize(uint32_t slotsPerSubframe) const
{
  uint64_t ret = 0;
  ret += m_slotNum;
  ret += m_subframeNum * slotsPerSubframe;
  ret += m_frameNum * GetSubframesPerFrame () * slotsPerSubframe;
  return ret;
}

void
SfnSf::Add(uint32_t slotN, uint32_t slotsPerSubframe)
{
  m_frameNum += (m_subframeNum + (m_slotNum + slotN) / slotsPerSubframe) / GetSubframesPerFrame ();
  m_subframeNum = (m_subframeNum + (m_slotNum + slotN) / slotsPerSubframe) % GetSubframesPerFrame ();
  m_slotNum = (m_slotNum + slotN) % slotsPerSubframe;
}

bool
SfnSf::operator <(const SfnSf &rhs) const
{
  if (m_frameNum < rhs.m_frameNum)
    {
      return true;
    }
  else if ((m_frameNum == rhs.m_frameNum ) && (m_subframeNum < rhs.m_subframeNum))
    {
      return true;
    }
  else if (((m_frameNum == rhs.m_frameNum ) && (m_subframeNum == rhs.m_subframeNum)) && (m_slotNum < rhs.m_slotNum))
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool
SfnSf::operator ==(const SfnSf &o) const
{
  return (m_frameNum == o.m_frameNum) && (m_subframeNum == o.m_subframeNum)
      && (m_slotNum == o.m_slotNum);
}

bool
SfnSf::IsTtiEqual(const SfnSf &o) const
{
  return (*this == o) && (m_varTtiNum == o.m_varTtiNum);
}

} // namespace ns3
