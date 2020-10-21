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
  NrSlSciF1aHeader sciF1a;
  sciF1a.SetPriority (1);
  sciF1a.SetMcs (12);
  sciF1a.SetSciStage2Format (NrSlSciF1aHeader::SciFormat2A);
  sciF1a.SetSlResourceReservePeriod (200);
  sciF1a.SetTotalSubChannels (1);
  sciF1a.SetIndexStartSubChannel (0);
  sciF1a.SetLengthSubChannel (1);
  sciF1a.SetSlMaxNumPerReserve (1);

  uint16_t sizeSciF1A = 1 + 1 + 1 + 2 + 2 + 1 + 1 + 1; // 10 bytes

  AddTestCase (new NrSlSciF1aTestCase (sciF1a, sizeSciF1A));

  //Test including the mandatory fields and 1 optional field
  sciF1a.SetSlMaxNumPerReserve (2);
  sciF1a.SetGapReTx1 (2);

  sizeSciF1A = sizeSciF1A + 1; // 11 bytes

  AddTestCase (new NrSlSciF1aTestCase (sciF1a, sizeSciF1A));

  //Test including the mandatory fields and 2 optional fields
  sciF1a.SetSlMaxNumPerReserve (3);
  sciF1a.SetGapReTx1 (2);
  sciF1a.SetGapReTx2 (3);

  sizeSciF1A = sizeSciF1A + 1; // 12 bytes

  AddTestCase (new NrSlSciF1aTestCase (sciF1a, sizeSciF1A));

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
 * Test Case SCI Format 1A
 */

std::string
NrSlSciF1aTestCase::BuildNameString (const NrSlSciF1aHeader &sciF1a, uint16_t expectedHeaderSize)
{
  std::ostringstream oss;

  oss << " Checked SCI format 1A : Priority " << +sciF1a.GetPriority ();
  oss << " MCS " << +sciF1a.GetMcs ();
  oss << " Resource reservation period " << +sciF1a.GetSlResourceReservePeriod ();
  oss << " Total number of Subchannels " << +sciF1a.GetTotalSubChannels ();
  oss << " Index starting Subchannel " << +sciF1a.GetIndexStartSubChannel ();
  oss << " Total number of allocated Subchannels " << +sciF1a.GetLengthSubChannel ();
  oss << " Maximum number of reservations " << +sciF1a.GetSlMaxNumPerReserve ();
  oss << " First retransmission gap in slots " << +sciF1a.GetGapReTx1 ();
  oss << " Second retransmission gap in slots " << +sciF1a.GetGapReTx2 () << "\n";
  return oss.str ();
}

NrSlSciF1aTestCase::NrSlSciF1aTestCase (NrSlSciF1aHeader sciF1a, uint16_t expectedHeaderSize)
  : TestCase (BuildNameString (sciF1a, expectedHeaderSize))
{
  NS_LOG_FUNCTION (this << GetName ());
  m_sciF1a = sciF1a;
  m_expectedHeaderSize = expectedHeaderSize;
}


NrSlSciF1aTestCase::~NrSlSciF1aTestCase ()
{
  NS_LOG_FUNCTION (this << GetName ());
}


void
NrSlSciF1aTestCase::DoRun ()
{
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (m_sciF1a);

  //deserialized
  NrSlSciF1aHeader deSerSciF1a;
  p->RemoveHeader (deSerSciF1a);

  NS_TEST_ASSERT_MSG_EQ (deSerSciF1a, m_sciF1a,
                         "SCI format 1A deserialized version is different than the one we serialized");
  NS_TEST_ASSERT_MSG_EQ (deSerSciF1a.GetSerializedSize (), m_expectedHeaderSize,
                         "SCI format 1A header size is different than the expected size in bytes");

} // end of void NrSlSciF1aTestCase::DoRun ()



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

