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


#include "mmwave-mac-pdu-tag.h"
#include "mmwave-phy-mac-common.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MmWaveMacPduTag);


MmWaveMacPduTag::MmWaveMacPduTag (SfnSf sfn, uint8_t symStart, uint8_t numSym)
  :  m_sfnSf (sfn), m_symStart (symStart), m_numSym (numSym)
{
}

TypeId
MmWaveMacPduTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacPduTag")
    .SetParent<Tag> ()
    .AddConstructor<MmWaveMacPduTag> ();
  return tid;
}

TypeId
MmWaveMacPduTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MmWaveMacPduTag::GetSerializedSize (void) const
{
  return 8 + 1 + 1;
}

void
MmWaveMacPduTag::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_sfnSf.GetEncoding ());
  i.WriteU8 (m_symStart);
  i.WriteU8 (m_numSym);
}

void
MmWaveMacPduTag::Deserialize (TagBuffer i)
{
  uint16_t v = i.ReadU64 ();
  m_sfnSf.FromEncoding (v);

  m_symStart = (uint8_t)i.ReadU8 ();
  m_numSym = (uint8_t)i.ReadU8 ();
}

void
MmWaveMacPduTag::Print (std::ostream &os) const
{
  os << m_sfnSf << " " << +m_symStart << " " << +m_numSym;
}

} // namespace ns3

