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
#ifndef MMWAVE_MAC_H
#define MMWAVE_MAC_H

#include "mmwave-mac-pdu-header.h"
#include "mmwave-mac-pdu-tag.h"

namespace ns3 {

struct MacPduInfo
{
  MacPduInfo (SfnSf sfn, uint32_t size, uint8_t numRlcPdu) :
    m_sfnSf (sfn), m_size (size), m_numRlcPdu (numRlcPdu)
  {
    m_pdu = Create<Packet> ();
    m_macHeader = MmWaveMacPduHeader ();
    MmWaveMacPduTag tag (sfn);
    m_pdu->AddPacketTag (tag);
  }

  MacPduInfo (SfnSf sfn, uint32_t size, uint8_t numRlcPdu, DciInfoElementTdma dci) :
    m_sfnSf (sfn), m_size (size), m_numRlcPdu (numRlcPdu)
  {
    m_pdu = Create<Packet> ();
    m_macHeader = MmWaveMacPduHeader ();
    MmWaveMacPduTag tag (sfn, dci.m_symStart, dci.m_numSym);
    m_pdu->AddPacketTag (tag);
  }

  SfnSf m_sfnSf;
  uint32_t m_size;
  uint8_t m_numRlcPdu;
  Ptr<Packet> m_pdu;
  MmWaveMacPduHeader m_macHeader;
};

} // namespace ns3
#endif // MMWAVE_MAC_H
