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
#include <ns3/log.h>
#include "nr-sl-sci-f02-header.h"


namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (NrSlSciF02Header);
NS_LOG_COMPONENT_DEFINE ("NrSlSciF02Header");

NrSlSciF02Header::NrSlSciF02Header ()
{
}

NrSlSciF02Header::~NrSlSciF02Header ()
{
}

TypeId
NrSlSciF02Header::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlSciF02Header")
    .SetParent<Header> ()
    .AddConstructor<NrSlSciF02Header> ()
  ;
  return tid;
}

TypeId
NrSlSciF02Header::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
NrSlSciF02Header::SetHarqId (uint8_t harqId)
{
  m_harqId = harqId;
}

void
NrSlSciF02Header::SetNdi (uint8_t ndi)
{
  m_ndi = ndi;
}

void
NrSlSciF02Header::SetRv (uint8_t rv)
{
  m_rv = rv;
}

void
NrSlSciF02Header::SetSrcId (uint32_t srcId)
{
  m_srcId = srcId;
}

void
NrSlSciF02Header::SetDstId (uint32_t dstId)
{
  m_dstId = dstId;
}

void
NrSlSciF02Header::SetCsiReq (uint8_t csiReq)
{
  m_csiReq = csiReq;
}

void
NrSlSciF02Header::SetZoneId (uint16_t zoneId)
{
  m_zoneId = zoneId;
}

void
NrSlSciF02Header::SetCommRange (uint8_t commRange)
{
  m_commRange = commRange;
}


uint8_t
NrSlSciF02Header::GetHarqId () const
{
  return m_harqId;
}

uint8_t
NrSlSciF02Header::GetNdi () const
{
  return m_ndi;
}

uint8_t
NrSlSciF02Header::GetRv () const
{
  return m_rv;
}

uint8_t
NrSlSciF02Header::GetSrcId () const
{
  return m_srcId;
}

uint16_t
NrSlSciF02Header::GetDstId () const
{
  return m_dstId;
}

uint8_t
NrSlSciF02Header::GetCsiReq () const
{
  return m_csiReq;
}

uint16_t
NrSlSciF02Header::GetZoneId () const
{
  return m_zoneId;
}

uint8_t
NrSlSciF02Header::GetCommRange () const
{
  return m_commRange;
}


bool
NrSlSciF02Header::EnsureMandConfig () const
{
  return m_harqId != std::numeric_limits <uint8_t>::max ()
         && m_ndi != std::numeric_limits <uint8_t>::max ()
         && m_rv != std::numeric_limits <uint8_t>::max ()
         && m_srcId != std::numeric_limits <uint32_t>::max ()
         && m_dstId != std::numeric_limits <uint32_t>::max ();
}


void
NrSlSciF02Header::Print (std::ostream &os)  const
{
  NS_LOG_FUNCTION (this);
  os << "HARQ process id " << +m_harqId
     << ", New data indicator " << +m_ndi
     << ", Redundancy version " << +m_rv
     << ", Source layer 2 Id " << +m_srcId
     << ", Destination layer 2 id " << m_dstId
     << ", Channel state information request " << +m_csiReq
     << ", Zone id " << m_zoneId
     << ", Communication range requirement " << +m_commRange;
}

uint32_t
NrSlSciF02Header::GetSerializedSize (void) const
{
  return 8;
}

void
NrSlSciF02Header::Serialize (Buffer::Iterator start) const
{
  NS_ASSERT_MSG (EnsureMandConfig (), "All the mandatory fields must be set before serializing");
  Buffer::Iterator i = start;

  uint32_t scif02Seg1 = 0;

  scif02Seg1 = (m_harqId & 0x1F);
  scif02Seg1 = (m_ndi & 0x1) | (scif02Seg1 << 1);
  scif02Seg1 = (m_rv & 0x3) | (scif02Seg1 << 2);
  scif02Seg1 = (m_srcId & 0xFF) | (scif02Seg1 << 8);
  scif02Seg1 = (m_dstId & 0xFFFF) | (scif02Seg1 << 16);

  i.WriteHtonU32 (scif02Seg1);

  uint32_t scif02Seg2 = 0;
  scif02Seg2 = (m_csiReq & 0x1);
  scif02Seg2 = (m_zoneId & 0xFFF) | (scif02Seg2 << 12);
  scif02Seg2 = (m_commRange & 0xF) | (scif02Seg2 << 4);

  //now the stinky 15 bits of padding
  uint16_t padding = 0;
  scif02Seg2 = (padding & 0x7FFF) | (scif02Seg2 << 15);

  i.WriteHtonU32 (scif02Seg2);


}

uint32_t
NrSlSciF02Header::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  uint32_t scif02Seg1 = i.ReadNtohU32 ();

  m_harqId = (scif02Seg1 >> 27) & 0x1F;
  m_ndi = (scif02Seg1 >> 26) & 0x1;
  m_rv = (scif02Seg1 >> 24) & 0x3;
  m_srcId = (scif02Seg1 >> 16) & 0xFF;
  m_dstId = (scif02Seg1 & 0xFFFF);

  uint32_t scif02Seg2 = i.ReadNtohU32 ();
  m_csiReq = (scif02Seg2 >> 31) & 0x1;
  m_zoneId = (scif02Seg2 >> 19) & 0xFFF;
  m_commRange = (scif02Seg2 >> 15) & 0xF;

  return GetSerializedSize ();
}

bool
NrSlSciF02Header::operator == (const NrSlSciF02Header &b) const
{
  if (m_harqId == b.m_harqId
      && m_ndi == b.m_ndi
      && m_rv == b.m_rv
      && m_srcId == b.m_srcId
      && m_dstId == b.m_dstId
      && m_csiReq == b.m_csiReq
      && m_zoneId == b.m_zoneId
      && m_commRange == b.m_commRange
      )
    {
      return true;
    }

  return false;
}

}  // namespace ns3
