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

#include "test-nr-sl-sci-headers.h"
#include "ns3/core-module.h"
#include <ns3/packet.h>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestNrSlSciHeaders");

/*
 * Test Suite
 */
NrSlSciHeadersTestSuite::NrSlSciHeadersTestSuite ()
  : TestSuite ("nr-sl-sci-headers", SYSTEM)
{

  //Test only including the mandatory fields
  NrSlSciF01Header sciF01;
  sciF01.SetPriority (1);
  sciF01.SetMcs (12);
  sciF01.SetSlResourceReservePeriod (200);
  sciF01.SetTotalSubChannels (1);
  sciF01.SetIndexStartSubChannel (0);
  sciF01.SetLengthSubChannel (1);
  sciF01.SetSlMaxNumPerReserve (1);

  uint16_t sizeSciF01 = 1 + 1 + 2 + 2 + 1 + 1 + 1; // 9 bytes

  AddTestCase (new NrSlSciF01TestCase (sciF01, sizeSciF01));

  //Test including the mandatory fields and 1 optional field
  //indexStartSubChannel and lengthSubChannel
  sciF01.SetSlMaxNumPerReserve (2);
  sciF01.SetGapReTx1 (2);

  sizeSciF01 = sizeSciF01 + 1; // 10 bytes

  AddTestCase (new NrSlSciF01TestCase (sciF01, sizeSciF01));

  //Test including the mandatory fields and 2 optional fields
  sciF01.SetSlMaxNumPerReserve (3);
  sciF01.SetGapReTx1 (2);
  sciF01.SetGapReTx2 (3);

  sizeSciF01 = sizeSciF01 + 1; // 11 bytes

  AddTestCase (new NrSlSciF01TestCase (sciF01, sizeSciF01));

  //SCI Format 02 tests
  uint16_t sizeSciF02 = 8; //8 bytes fixed
  NrSlSciF02Header sciF02;

  sciF02.SetHarqId (5);
  sciF02.SetNdi (1);
  sciF02.SetRv (0);
  sciF02.SetSrcId (1);
  sciF02.SetDstId (255);

  //Test only including the mandatory fields
  AddTestCase (new NrSlSciF02TestCase (sciF02, sizeSciF02));

  //Test including the optional fields
  sciF02.SetCsiReq (1);
  sciF02.SetZoneId (200);
  sciF02.SetCommRange (10);
  AddTestCase (new NrSlSciF02TestCase (sciF02, sizeSciF02));

} // end of LteRadioLinkFailureTestSuite::LteRadioLinkFailureTestSuite ()


static NrSlSciHeadersTestSuite g_nrSlSciHeadersTestSuite;

/*
 * Test Case SCI Format 01
 */

std::string
NrSlSciF01TestCase::BuildNameString (const NrSlSciF01Header &sciF01, uint16_t expectedHeaderSize)
{
  std::ostringstream oss;

  oss << " Checked SCI format 01 : Priority " << +sciF01.GetPriority ();
  oss << " MCS " << +sciF01.GetMcs ();
  oss << " Resource reservation period " << +sciF01.GetSlResourceReservePeriod ();
  oss << " Total number of Subchannels " << +sciF01.GetTotalSubChannels ();
  oss << " Index starting Subchannel " << +sciF01.GetIndexStartSubChannel ();
  oss << " Total number of allocated Subchannels " << +sciF01.GetLengthSubChannel ();
  oss << " Maximum number of reservations " << +sciF01.GetSlMaxNumPerReserve ();
  oss << " First retransmission gap in slots " << +sciF01.GetGapReTx1 ();
  oss << " Second retransmission gap in slots " << +sciF01.GetGapReTx2 () << "\n";
  return oss.str ();
}

NrSlSciF01TestCase::NrSlSciF01TestCase (NrSlSciF01Header sciF01, uint16_t expectedHeaderSize)
  : TestCase (BuildNameString (sciF01, expectedHeaderSize))
{
  NS_LOG_FUNCTION (this << GetName ());
  m_sciF01 = sciF01;
  m_expectedHeaderSize = expectedHeaderSize;
}


NrSlSciF01TestCase::~NrSlSciF01TestCase ()
{
  NS_LOG_FUNCTION (this << GetName ());
}


void
NrSlSciF01TestCase::DoRun ()
{
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (m_sciF01);

  //deserialized
  NrSlSciF01Header deSerSciF01;
  p->RemoveHeader (deSerSciF01);

  NS_TEST_ASSERT_MSG_EQ (deSerSciF01, m_sciF01,
                         "SCI format 01 deserialized version is different than the one we serialized");
  NS_TEST_ASSERT_MSG_EQ (deSerSciF01.GetSerializedSize (), m_expectedHeaderSize,
                         "SCI format 01 header size is different than the expected size in bytes");

} // end of void NrSlSciF01TestCase::DoRun ()



/*
 * Test Case SCI Format 02
 */

std::string
NrSlSciF02TestCase::BuildNameString (const NrSlSciF02Header &sciF02, uint16_t expectedHeaderSize)
{
  std::ostringstream oss;

  oss << " Checked SCI format 02 : HARQ process id " << +sciF02.GetHarqId ();
  oss << " New data indicator " << +sciF02.GetNdi ();
  oss << " Redundancy version " << +sciF02.GetRv ();
  oss << " Source layer 2 Id " << +sciF02.GetSrcId ();
  oss << " Destination layer 2 id " << sciF02.GetDstId ();
  oss << " Channel state information request " << +sciF02.GetCsiReq ();
  oss << " Zone id " << sciF02.GetZoneId ()<< "\n";
  oss << " Communication range requirement " << +sciF02.GetCommRange () << "\n";
  return oss.str ();
}

NrSlSciF02TestCase::NrSlSciF02TestCase (NrSlSciF02Header sciF02, uint16_t expectedHeaderSize)
  : TestCase (BuildNameString (sciF02, expectedHeaderSize))
{
  NS_LOG_FUNCTION (this << GetName ());
  m_sciF02 = sciF02;
  m_expectedHeaderSize = expectedHeaderSize;
}


NrSlSciF02TestCase::~NrSlSciF02TestCase ()
{
  NS_LOG_FUNCTION (this << GetName ());
}


void
NrSlSciF02TestCase::DoRun ()
{
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (m_sciF02);

  //deserialized
  NrSlSciF02Header deSerSciF02;
  p->RemoveHeader (deSerSciF02);

  NS_TEST_ASSERT_MSG_EQ (deSerSciF02, m_sciF02,
                         "SCI format 02 deserialized version is different than the one we serialized");
  NS_TEST_ASSERT_MSG_EQ (deSerSciF02.GetSerializedSize (), m_expectedHeaderSize,
                         "SCI format 02 header size is different than the expected size in bytes");

} // end of void NrSlSciF02TestCase::DoRun ()

