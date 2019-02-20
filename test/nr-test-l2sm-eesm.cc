/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
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
#include <ns3/test.h>
#include <ns3/nr-eesm-error-model.h>
#include <ns3/enum.h>
/**
 * \file nr-test-l2sm-eesm.cc
 * \ingroup test
 * \brief Unit-testing the new EESM-based error model.
 *
 */
namespace ns3 {

/**
 * \brief NrL2smEesm testcase
 */
class NrL2smEesmTestCase : public TestCase
{
public:
  NrL2smEesmTestCase (const std::string &name) : TestCase (name) { }

  /**
   * \brief Destroy the object instance
   */
  virtual ~NrL2smEesmTestCase () override {}

private:
  virtual void DoRun (void) override;

  void TestMappingSinrBler1 (const Ptr<NrEesmErrorModel> &em);
  void TestMappingSinrBler2 (const Ptr<NrEesmErrorModel> &em);
  void TestBgType1 (const Ptr<NrEesmErrorModel> &em);
  void TestBgType2 (const Ptr<NrEesmErrorModel> &em);

  void TestTable1();
  void TestTable2();
};

void
NrL2smEesmTestCase::TestBgType1 (const Ptr<NrEesmErrorModel> &em)
{
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 18), NrEesmErrorModel::SECOND,
                         "TestBgType1-a: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3900, 18), NrEesmErrorModel::FIRST,
                         "TestBgType1-b: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (200, 18), NrEesmErrorModel::SECOND,
                         "TestBgType1-c: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (4000, 0), NrEesmErrorModel::SECOND,
                         "TestBgType1-d: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 28), NrEesmErrorModel::FIRST,
                         "TestBgType1-e: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 2), NrEesmErrorModel::SECOND,
                         "TestBgType2-f: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 16), NrEesmErrorModel::SECOND,
                         "TestBgType2-g: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3900, 14), NrEesmErrorModel::FIRST,
                         "TestBgType2-h: The calculated value differs from the 3GPP base graph selection algorithm.");
}

void
NrL2smEesmTestCase::TestBgType2 (const Ptr<NrEesmErrorModel> &em)
{
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 18), NrEesmErrorModel::FIRST,
                         "TestBgType2-a: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3900, 18), NrEesmErrorModel::FIRST,
                         "TestBgType2-b: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (200, 18), NrEesmErrorModel::SECOND,
                         "TestBgType2-c: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (4000, 0), NrEesmErrorModel::SECOND,
                         "TestBgType2-d: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 27), NrEesmErrorModel::FIRST,
                         "TestBgType2-e: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 2), NrEesmErrorModel::SECOND,
                         "TestBgType2-f: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3200, 16), NrEesmErrorModel::FIRST,
                         "TestBgType2-g: The calculated value differs from the 3GPP base graph selection algorithm.");
  NS_TEST_ASSERT_MSG_EQ (em->GetBaseGraphType (3900, 14), NrEesmErrorModel::FIRST,
                         "TestBgType2-h: The calculated value differs from the 3GPP base graph selection algorithm.");
}

typedef std::tuple<double, uint8_t, uint32_t, double> MappingTable;

static std::vector<MappingTable> resultTable1 = {
  // sinr (lineal), mcs, cbsize, result
  MappingTable { 19.95, 18, 3200, 0.0036 },    // sinr 13 db
  MappingTable { 15.84, 18, 3200, 0.964962 },  // sinr 12 db
  MappingTable { 10,    18, 3200, 1.00 },      // sinr 10 db
  MappingTable { 19.95, 18, 1750, 0.0015 },      // sinr 13 db
  MappingTable { 15.84, 18, 1750, 0.744913 },    // sinr 12 db
  MappingTable { 10,    18, 1750, 1.00 },      // sinr 10 db
  MappingTable { 19.95, 18, 3500, 0.0038 },    // sinr 13 db
  MappingTable { 15.84, 18, 3500, 0.967803 },  // sinr 12 db
  MappingTable { 10,    18, 3500, 1.00 },      // sinr 10 db

  MappingTable { 8.9125, 14, 3900, 0.0222 },    // sinr 9.5db
  MappingTable { 7.9433, 14, 3900, 0.961174 },  // sinr 9 db
  MappingTable { 6.3095, 14, 3900, 1.00 },      // sinr 8 db
  MappingTable { 8.9125, 14, 6300, 0.0161 },    // sinr 9.5db
  MappingTable { 7.9433, 14, 6300, 0.992308 },  // sinr 9 db
  MappingTable { 6.3095, 14, 6300, 1.00 }       // sinr 8 db

};
static std::vector<MappingTable> resultTable2 = {
  // sinr (lineal), mcs, cbsize, result
  MappingTable { 19.95, 11, 3200, 0.0036 },    // sinr 13 db
  MappingTable { 15.84, 11, 3200, 0.964962 },    // sinr 12 db
  MappingTable { 10,    11, 3200, 1.00 },      // sinr 10 db
  MappingTable { 19.95, 11, 1750, 0.0015 },    // sinr 13 db
  MappingTable { 15.84, 11, 1750, 0.744913 },    // sinr 12 db
  MappingTable { 10,    11, 1750, 1.00 },      // sinr 10 db
  MappingTable { 19.95, 11, 3500, 0.0038 },      // sinr 13 db
  MappingTable { 15.84, 11, 3500, 0.967803 },    // sinr 12 db
  MappingTable { 10,    11, 3500, 1.00 },      // sinr 10 db

  MappingTable { 8.9125, 8, 3900, 0.0222 },    // sinr 9.5db
  MappingTable { 7.9433, 8, 3900, 0.961174 },  // sinr 9 db
  MappingTable { 6.3095, 8, 3900, 1.00 },      // sinr 8 db
  MappingTable { 8.9125, 8, 6300, 0.0161 },    // sinr 9.5db
  MappingTable { 7.9433, 8, 6300, 0.992308 },  // sinr 9 db
  MappingTable { 6.3095, 8, 6300, 1.00 }       // sinr 8 db

};

void
NrL2smEesmTestCase::TestMappingSinrBler1 (const Ptr<NrEesmErrorModel> &em)
{
  for (auto result : resultTable1)
    {
      NS_TEST_ASSERT_MSG_EQ (em->MappingSinrBler(std::get<0> (result),
                                                 std::get<1> (result),
                                                 std::get<2> (result)),
                                                 std::get<3> (result),
                             "TestMappingSinrBler1: The calculated value differs from "
                             " the SINR-BLER table. SINR=" << std::get<0> (result) <<
                             " MCS " << static_cast<uint32_t> (std::get<1> (result)) <<
                             " CBS " << std::get<2> (result));
    }

}
void
NrL2smEesmTestCase::TestMappingSinrBler2 (const Ptr<NrEesmErrorModel> &em)
{
  for (auto result : resultTable2)
    {
      NS_TEST_ASSERT_MSG_EQ (em->MappingSinrBler(std::get<0> (result),
                                                 std::get<1> (result),
                                                 std::get<2> (result)),
                                                 std::get<3> (result),
                             "TestMappingSinrBler2: The calculated value differs from "
                             " the SINR-BLER table. SINR=" << std::get<0> (result) <<
                             " MCS " << static_cast<uint32_t> (std::get<1> (result)) <<
                             " CBS " << std::get<2> (result));
    }

}
void
NrL2smEesmTestCase::TestTable1()
{
  // Create an object of type NrEesmErrorModel
  Ptr<NrEesmErrorModel> em = CreateObject <NrEesmErrorModel> ();
  // Set attribute of MCS table to be used
  em->SetAttribute("McsTable", EnumValue (NrEesmErrorModel::McsTable1));

  // Test here the functions:
  TestBgType1 (em);
  TestMappingSinrBler1 (em);
}

void
NrL2smEesmTestCase::TestTable2()
{
  // Create an object of type NrEesmErrorModel
  Ptr<NrEesmErrorModel> em = CreateObject <NrEesmErrorModel> ();
  // Set attribute of MCS table to be used
  em->SetAttribute("McsTable", EnumValue (NrEesmErrorModel::McsTable2));

  // Test here the functions:
  TestBgType2 (em);
  TestMappingSinrBler2 (em);
}

void
NrL2smEesmTestCase::DoRun()
{
  TestTable1 ();
  TestTable2 ();
}

class NrTestL2smEesm : public TestSuite
{
public:
  NrTestL2smEesm () : TestSuite ("Nr-test-l2sm-eesm", UNIT)
    {
      AddTestCase(new NrL2smEesmTestCase ("First test"), QUICK);
    }
};

static NrTestL2smEesm NrTestL2smEesmTestSuite; //!< Nr test suite

}; // namespace ns3
