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

#include "nr-sl-sci-f01-header.h"
#include <ns3/log.h>


namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (NrSlSciF01Header);
NS_LOG_COMPONENT_DEFINE ("NrSlSciF01Header");

NrSlSciF01Header::NrSlSciF01Header ()
{
}

NrSlSciF01Header::~NrSlSciF01Header ()
{
}

TypeId
NrSlSciF01Header::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlSciF01Header")
    .SetParent<Header> ()
    .AddConstructor<NrSlSciF01Header> ()
  ;
  return tid;
}

TypeId
NrSlSciF01Header::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
NrSlSciF01Header::SetPriority (uint8_t priority)
{
  m_priority = priority;
}

void
NrSlSciF01Header::SetTotalSubChannels (uint16_t totalSubChannels)
{
  NS_ASSERT_MSG (m_totalSubChannels > 0, "Total number of sub-channels must be greater than 0");
  m_totalSubChannels = totalSubChannels;
}

void
NrSlSciF01Header::SetIndexStartSubChannel (uint8_t indexStartSubChannel)
{
  m_indexStartSubChannel = indexStartSubChannel;
}

void
NrSlSciF01Header::SetLengthSubChannel (uint8_t lengthSubChannel)
{
  m_lengthSubChannel = lengthSubChannel;
}

void
NrSlSciF01Header::SetSlResourceReservePeriod (uint16_t slResourceReservePeriod)
{
  m_slResourceReservePeriod = slResourceReservePeriod;
}

void
NrSlSciF01Header::SetMcs (uint8_t mcs)
{
  m_mcs = mcs;
}

void
NrSlSciF01Header::SetSlMaxNumPerReserve (uint8_t slMaxNumPerReserve)
{
  //Just a sanity check
  bool valueCheck = false;
  valueCheck = (slMaxNumPerReserve == 1) || (slMaxNumPerReserve == 2) || (slMaxNumPerReserve == 3);
  NS_ASSERT_MSG (valueCheck, "Invalid value " << +slMaxNumPerReserve << " for SlMaxNumPerReserve. Only 1, 2, or 3 should be used");
  m_slMaxNumPerReserve = slMaxNumPerReserve;
}

void
NrSlSciF01Header::SetGapReTx1 (uint8_t gapReTx1)
{
  NS_ASSERT_MSG (m_slMaxNumPerReserve == 2 || m_slMaxNumPerReserve == 3, "SlMaxNumPerReserve should be set to 2 or 3 before setting GapReTx1");
  m_gapReTx1 = gapReTx1;
}

void
NrSlSciF01Header::SetGapReTx2 (uint8_t gapReTx2)
{
  NS_ASSERT_MSG (m_slMaxNumPerReserve == 3, "SlMaxNumPerReserve should be set to 3 before setting GapReTx2");
  NS_ASSERT_MSG (gapReTx2 != GetGapReTx1 (), "The second retransmission should be perform in a different slot than the first retransmission");
  m_gapReTx2 = gapReTx2;
}

uint8_t
NrSlSciF01Header::GetPriority () const
{
  return m_priority;
}
uint16_t
NrSlSciF01Header::GetTotalSubChannels () const
{
  return m_totalSubChannels;
}

uint8_t
NrSlSciF01Header::GetIndexStartSubChannel () const
{
  return m_indexStartSubChannel;
}

uint8_t
NrSlSciF01Header::GetLengthSubChannel () const
{
  return m_lengthSubChannel;
}

uint16_t
NrSlSciF01Header::GetSlResourceReservePeriod () const
{
  return m_slResourceReservePeriod;
}

uint8_t
NrSlSciF01Header::GetMcs () const
{
  return m_mcs;
}

uint8_t
NrSlSciF01Header::GetSlMaxNumPerReserve () const
{
  return m_slMaxNumPerReserve;
}

uint8_t
NrSlSciF01Header::GetGapReTx1 () const
{
  return m_gapReTx1;
}

uint8_t
NrSlSciF01Header::GetGapReTx2 () const
{
  return m_gapReTx2;
}

bool
NrSlSciF01Header::EnsureMandConfig () const
{
  bool shouldBeSet = m_priority != std::numeric_limits <uint8_t>::max ()
    && m_mcs != std::numeric_limits <uint8_t>::max ()
    && m_slResourceReservePeriod != std::numeric_limits <uint16_t>::max ()
    && m_totalSubChannels != std::numeric_limits <uint16_t>::max ()
    && m_indexStartSubChannel != std::numeric_limits <uint8_t>::max ()
    && m_lengthSubChannel != std::numeric_limits <uint8_t>::max ()
    && m_slMaxNumPerReserve != std::numeric_limits <uint8_t>::max ();

  return shouldBeSet;
}


void
NrSlSciF01Header::Print (std::ostream &os)  const
{
  NS_LOG_FUNCTION (this);
  os << "Priority " << +m_priority
     << ", MCS " << +m_mcs
     << ", Resource reservation period " << +m_slResourceReservePeriod
     << ", Total number of Subchannels " << +m_totalSubChannels
     << ", Index starting Subchannel " << +m_indexStartSubChannel
     << ", Total number of allocated Subchannel " << +m_lengthSubChannel
     << ", Maximum number of reservations " << +m_slMaxNumPerReserve
     << ", First retransmission gap in slots " << +m_gapReTx1
     << ", Second retransmission gap in slots " << +m_gapReTx2;
}

uint32_t
NrSlSciF01Header::GetSerializedSize (void) const
{
  uint32_t totalSize = 0; //bytes
  //Always present
  //priority =  1 byte
  //mcs =  1 byte
  //slResourceReservePeriod = 2 bytes
  //totalSubChannels = 2 bytes
  //indexStartSubChannel = 1 byte
  //lengthSubChannel = 1 byte
  //slMaxNumPerReserve = 1 byte

  //Optional fields
  //gapReTx1 = 1 byte if slMaxNumPerReserve == 2
  //gapReTx2 = 1 byte if slMaxNumPerReserve == 3
  totalSize = 1 + 1 + 2 + 2 + 1 + 1 + 1;
  totalSize = (m_slMaxNumPerReserve == 2 ? totalSize + 1 : totalSize + 0); //only gapReTx1
  totalSize = (m_slMaxNumPerReserve == 3 ? totalSize + 2 : totalSize + 0); //both gapReTx1 and gapReTx2

  return totalSize;
}

void
NrSlSciF01Header::Serialize (Buffer::Iterator start) const
{
  NS_ASSERT_MSG (EnsureMandConfig (), "All the mandatory fields must be set before serializing");
  Buffer::Iterator i = start;

  i.WriteU8 (m_priority);
  i.WriteU8 (m_mcs);
  i.WriteHtonU16 (m_slResourceReservePeriod);
  i.WriteHtonU16 (m_totalSubChannels);
  i.WriteU8 (m_indexStartSubChannel);
  i.WriteU8 (m_lengthSubChannel);

  i.WriteU8 (m_slMaxNumPerReserve);
  if (m_slMaxNumPerReserve == 2 || m_slMaxNumPerReserve == 3)
    {
      i.WriteU8 (m_gapReTx1);
    }
  if (m_slMaxNumPerReserve == 3)
    {
      i.WriteU8 (m_gapReTx2);
    }
}

uint32_t
NrSlSciF01Header::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_priority = i.ReadU8 ();
  m_mcs = i.ReadU8 ();
  m_slResourceReservePeriod = i.ReadNtohU16 ();
  m_totalSubChannels = i.ReadNtohU16 ();
  m_indexStartSubChannel = i.ReadU8 ();
  m_lengthSubChannel = i.ReadU8 ();

  m_slMaxNumPerReserve = i.ReadU8 ();
  if (m_slMaxNumPerReserve == 2 || m_slMaxNumPerReserve == 3)
    {
      m_gapReTx1 = i.ReadU8 ();
    }
  if (m_slMaxNumPerReserve == 3)
    {
      m_gapReTx2 = i.ReadU8 ();
    }

  return GetSerializedSize ();
}

bool
NrSlSciF01Header::operator == (const NrSlSciF01Header &b) const
{
  if (m_priority == b.m_priority
      && m_mcs == b.m_mcs
      && m_slResourceReservePeriod == b.m_slResourceReservePeriod
      && m_totalSubChannels == b.m_totalSubChannels
      && m_indexStartSubChannel == b.m_indexStartSubChannel
      && m_lengthSubChannel == b.m_lengthSubChannel
      && m_slMaxNumPerReserve == b.m_slMaxNumPerReserve
      && m_gapReTx1 == b.m_gapReTx1
      && m_gapReTx2 == b.m_gapReTx2
      )
    {
      return true;
    }

  return false;
}

}  // namespace ns3
