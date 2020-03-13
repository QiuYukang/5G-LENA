/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
*   Author: Marco Miozzo <marco.miozzo@cttc.es>
*           Nicola Baldo  <nbaldo@cttc.es>
*
*   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
*                           Sourjya Dutta <sdutta@nyu.edu>
*                           Russell Ford <russell.ford@nyu.edu>
*                          Menglei Zhang <menglei@nyu.edu>
*                          Biljana Bojovic <biljana.bojovic@cttc.es>
*/


#include "mmwave-phy-mac-common.h"
#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/string.h>
#include <ns3/attribute-accessor-helper.h>
#include <algorithm>
#include "mmwave-mac-scheduler-tdma-rr.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWavePhyMacCommon");

NS_OBJECT_ENSURE_REGISTERED (MmWavePhyMacCommon);

TypeId
MmWavePhyMacCommon::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWavePhyMacCommon")
    .SetParent<Object> ()
    .AddConstructor<MmWavePhyMacCommon> ()
    .AddAttribute ("L1L2CtrlLatency",
                   "L1L2 CTRL decode latency in slot",
                   UintegerValue (2),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_l1L2CtrlLatency),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

MmWavePhyMacCommon::MmWavePhyMacCommon ()
  :
  m_l1L2CtrlLatency (2)
{
  NS_LOG_INFO ("MmWavePhyMacCommon constructor");
}


MmWavePhyMacCommon::~MmWavePhyMacCommon (void)
{
}


uint32_t
MmWavePhyMacCommon::GetSlotsPerSubframe (void) const
{
  return 13;
}

uint16_t
MmWavePhyMacCommon::GetL1L2CtrlLatency (void) const
{
  return m_l1L2CtrlLatency;
}

void
MmWavePhyMacCommon::SetL1L2CtrlLatency (uint32_t delaySfs)
{
  m_l1L2CtrlLatency = delaySfs;
}


void
SlotAllocInfo::Merge (const SlotAllocInfo &other)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (other.m_type != NONE && m_type != NONE);
  NS_ASSERT (other.m_sfnSf == m_sfnSf);

  if (other.m_type * m_type == 6)
    {
      m_type = BOTH;
    }

  m_numSymAlloc += other.m_numSymAlloc;

  for (auto extAlloc : other.m_varTtiAllocInfo)
    {
      m_varTtiAllocInfo.push_front (extAlloc);
    }

  // Sort over the symStart of the DCI (VarTtiAllocInfo::operator <)
  std::sort (m_varTtiAllocInfo.begin (), m_varTtiAllocInfo.end ());
}

bool
SlotAllocInfo::ContainsDataAllocation () const
{
  NS_LOG_FUNCTION (this);
  for (const auto & allocation : m_varTtiAllocInfo)
    {
      if (allocation.m_dci->m_type == DciInfoElementTdma::DATA)
        {
          return true;
        }
    }
  return false;
}

bool
SlotAllocInfo::operator < (const SlotAllocInfo &rhs) const
{
  return m_sfnSf < rhs.m_sfnSf;
}

std::ostream & operator<< (std::ostream & os, DciInfoElementTdma::DciFormat const & item)
{
  if (item == DciInfoElementTdma::DL)
    {
      os << "DL";
    }
  else if (item == DciInfoElementTdma::UL)
    {
      os << "UL";
    }
  else
    {
      os << "NA";
    }
  return os;
}

std::ostream &operator<< (std::ostream &os, const DlHarqInfo &item)
{
  if (item.IsReceivedOk ())
    {
      os << "ACK feedback ";
    }
  else
    {
      os << "NACK feedback ";
    }
  os << "for ProcessID: " << static_cast<uint32_t> (item.m_harqProcessId) << " of UE "
     << static_cast<uint32_t> (item.m_rnti) << " Num Retx: " << static_cast<uint32_t> (item.m_numRetx);
  return os;
}

std::ostream &operator<< (std::ostream &os, const UlHarqInfo &item)
{
  if (item.IsReceivedOk ())
    {
      os << "ACK feedback ";
    }
  else
    {
      os << "NACK feedback ";
    }
  os << "for ProcessID: " << static_cast<uint32_t> (item.m_harqProcessId) << " of UE "
     << static_cast<uint32_t> (item.m_rnti) << " Num Retx: " << static_cast<uint32_t> (item.m_numRetx);

  return os;
}

std::ostream & operator<< (std::ostream & os, SfnSf const & item)
{
  os << "FrameNum: " << static_cast<uint32_t> (item.m_frameNum) <<
    " SubFrameNum: " << static_cast<uint32_t> (item.m_subframeNum) <<
    " SlotNum: " << static_cast<uint32_t> (item.m_slotNum) <<
    " VarTtiNum: " << static_cast<uint32_t> (item.m_varTtiNum);
  return os;
}

std::ostream &operator<< (std::ostream &os, const SlotAllocInfo &item)
{
  os << "Allocation for slot " << item.m_sfnSf << " total symbols allocated: "
     << item.m_numSymAlloc << " of type " << item.m_type
     << ", tti: " << item.m_varTtiAllocInfo.size ()
     << " composed by the following allocations: " << std::endl;
  for (const auto & alloc : item.m_varTtiAllocInfo)
    {
      std::string direction, type;
      if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL)
        {
          type = "CTRL";
        }
      else if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL_DATA)
        {
          type = "CTRL_DATA";
        }
      else
        {
          type = "DATA";
        }

      if (alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
          direction = "UL";
        }
      else
        {
          direction = "DL";
        }
      os << "[Allocation from sym " << static_cast<uint32_t> (alloc.m_dci->m_symStart) <<
            " to sym " << static_cast<uint32_t> (alloc.m_dci->m_numSym + alloc.m_dci->m_symStart) <<
            " direction " << direction << " type " << type << "]" << std::endl;
    }
  return os;
}

std::ostream &operator<<(std::ostream &os, const SlotAllocInfo::AllocationType &item)
{
  switch (item)
    {
    case SlotAllocInfo::NONE:
      os << "NONE";
      break;
    case SlotAllocInfo::DL:
      os << "DL";
      break;
    case SlotAllocInfo::UL:
      os << "UL";
      break;
    case SlotAllocInfo::BOTH:
      os << "BOTH";
      break;
    }

  return os;
}

}
