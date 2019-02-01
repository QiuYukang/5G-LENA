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
#include <ns3/object-factory.h>
#include <ns3/mmwave-mac-scheduler-ns3.h>
#include <ns3/mmwave-mac-sched-sap.h>

/**
 * \file mmwave-test-sched.cc
 * \ingroup test
 * \brief Unit-testing the Scheduler interface class.
 *
 * TODO
 */
namespace ns3 {

/**
 * \brief The TestCschedSapUser class
 *
 * This class doesn't do absolutely nothing. Thank you for the attention.
 */
class TestCschedSapUser : public MmWaveMacCschedSapUser
{
public:
  TestCschedSapUser () : MmWaveMacCschedSapUser () { }
  virtual void CschedCellConfigCnf (const struct CschedCellConfigCnfParameters& params) override
  { NS_UNUSED(params); }

  virtual void CschedUeConfigCnf (const struct CschedUeConfigCnfParameters& params) override
  { NS_UNUSED(params); }

  virtual void CschedLcConfigCnf (const struct CschedLcConfigCnfParameters& params) override
  { NS_UNUSED(params); }

  virtual void CschedLcReleaseCnf (const struct CschedLcReleaseCnfParameters& params) override
  { NS_UNUSED(params); }

  virtual void CschedUeReleaseCnf (const struct CschedUeReleaseCnfParameters& params) override
  { NS_UNUSED(params); }

  virtual void CschedUeConfigUpdateInd (const struct CschedUeConfigUpdateIndParameters& params) override
  { NS_UNUSED(params); }

  virtual void CschedCellConfigUpdateInd (const struct CschedCellConfigUpdateIndParameters& params) override
  { NS_UNUSED(params); }
};

class TestSchedSapUser;
/**
 * \brief TestSched testcase
 */
class MmWaveSchedGeneralTestCase : public TestCase
{
public:
  /**
   * \brief Create MmWaveSchedGeneralTestCase
   * \param scheduler Scheduler to test
   * \param name Name of the test
   */
  MmWaveSchedGeneralTestCase (const std::string &scheduler, const std::string &name)
    : TestCase (name),
      m_scheduler (scheduler) {}

  /**
   * \brief Destroy the object instance
   */
  virtual ~MmWaveSchedGeneralTestCase () override {}

  void SchedConfigInd (const struct MmWaveMacSchedSapUser::SchedConfigIndParameters& params);

protected:
  void TestSAPInterface (const Ptr<MmWaveMacScheduler> &sched);
  void TestAddingRemovingUsersNoData (const Ptr<MmWaveMacSchedulerNs3> &sched);
  void TestSchedNewData (const Ptr<MmWaveMacSchedulerNs3> &sched);
  void TestSchedNewDlData (const Ptr<MmWaveMacSchedulerNs3> &sched);
  void TestSchedNewUlData (const Ptr<MmWaveMacSchedulerNs3> &sched);
  void TestSchedNewDlUlData (const Ptr<MmWaveMacSchedulerNs3> &sched);

protected:
  void AddOneUser (uint16_t rnti, const Ptr<MmWaveMacSchedulerNs3> &sched);
  void TestingRemovingUsers (const Ptr<MmWaveMacSchedulerNs3> &sched);
  void TestingAddingUsers (const Ptr<MmWaveMacSchedulerNs3> &sched);
  void LcConfigFor (uint16_t rnti, uint32_t bytes, const Ptr<MmWaveMacSchedulerNs3> &sched);

private:
  virtual void DoRun (void) override;

  std::string m_scheduler            {}; //!< Type of the scheduler
  TestCschedSapUser *m_cSchedSapUser {nullptr};
  TestSchedSapUser *m_schedSapUser   {nullptr};
};

class TestSchedSapUser : public MmWaveMacSchedSapUser
{
public:
  TestSchedSapUser (MmWaveSchedGeneralTestCase *testCase)
    : MmWaveMacSchedSapUser (),
      m_testCase (testCase)
  { }

  virtual void SchedConfigInd (const struct SchedConfigIndParameters& params) override
  {
    m_testCase->SchedConfigInd (params);
  }
private:
  MmWaveSchedGeneralTestCase *m_testCase;
};

void
MmWaveSchedGeneralTestCase::TestSAPInterface (const Ptr<MmWaveMacScheduler> &sched)
{
  NS_ABORT_IF (sched->GetMacSchedSapProvider() == nullptr);
  NS_ABORT_IF (sched->GetMacCschedSapProvider () == nullptr);
  sched->SetMacCschedSapUser (m_cSchedSapUser);
  sched->SetMacSchedSapUser(m_schedSapUser);
}

void
MmWaveSchedGeneralTestCase::SchedConfigInd (const struct MmWaveMacSchedSapUser::SchedConfigIndParameters& params)
{
  NS_UNUSED(params);
}

void
MmWaveSchedGeneralTestCase::AddOneUser(uint16_t rnti, const Ptr<MmWaveMacSchedulerNs3> &sched)
{
  MmWaveMacCschedSapProvider::CschedUeConfigReqParameters params;
  params.m_rnti = rnti;
  params.m_beamId = AntennaArrayModel::BeamId (8, 120.0);
  sched->DoCschedUeConfigReq (params);
}

void
MmWaveSchedGeneralTestCase::TestingAddingUsers (const Ptr<MmWaveMacSchedulerNs3> &sched)
{
  for (uint16_t i = 0; i < 80; ++i)
  {
    AddOneUser (i, sched);
    NS_TEST_ASSERT_MSG_EQ(sched->m_ueMap.size(),
                          static_cast<uint32_t> (i+1), "UE not saved in the map");
  }
}

void
MmWaveSchedGeneralTestCase::TestingRemovingUsers (const Ptr<MmWaveMacSchedulerNs3> &sched)
{
  for (uint16_t i = 80; i > 0; --i)
  {
    MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters params;
    params.m_rnti = i - 1;
    sched->DoCschedUeReleaseReq (params);
    NS_TEST_ASSERT_MSG_EQ (sched->m_ueMap.size (),
                           static_cast<uint32_t> (i - 1), "UE released from the map. Map size " <<
                           sched->m_ueMap.size () << " counter " << i);
  }
}

void
MmWaveSchedGeneralTestCase::TestAddingRemovingUsersNoData (const Ptr<MmWaveMacSchedulerNs3> &sched)
{
  NS_TEST_ASSERT_MSG_EQ(sched->m_ueMap.size(), 0, "some UE are in the map");
  TestingAddingUsers (sched);
  TestingRemovingUsers(sched);
  NS_TEST_ASSERT_MSG_EQ(sched->m_ueMap.size(), 0, sched->m_ueMap.size() << " UEs are still in the map");
}

void
MmWaveSchedGeneralTestCase::TestSchedNewData (const Ptr<MmWaveMacSchedulerNs3> &sched)
{
  TestSchedNewDlData(sched);
  TestSchedNewUlData(sched);
  TestSchedNewDlUlData(sched);
}

void
MmWaveSchedGeneralTestCase::LcConfigFor (uint16_t rnti, uint32_t bytes,
                                         const Ptr<MmWaveMacSchedulerNs3> &sched)
{
  MmWaveMacCschedSapProvider::CschedLcConfigReqParameters params;
  LogicalChannelConfigListElement_s lc;
  params.m_rnti = rnti;
  params.m_reconfigureFlag = false;
  params.m_logicalChannelConfigList.emplace_back (lc);

  sched->DoCschedLcConfigReq (params);
}

void
MmWaveSchedGeneralTestCase::TestSchedNewDlData (const Ptr<MmWaveMacSchedulerNs3> &sched)
{
  // Add 80 users
  TestingAddingUsers (sched);
}

void
MmWaveSchedGeneralTestCase::TestSchedNewUlData (const Ptr<MmWaveMacSchedulerNs3> &sched)
{

}

void
MmWaveSchedGeneralTestCase::TestSchedNewDlUlData (const Ptr<MmWaveMacSchedulerNs3> &sched)
{

}

void
MmWaveSchedGeneralTestCase::DoRun()
{
  m_cSchedSapUser = new TestCschedSapUser ();
  m_schedSapUser = new TestSchedSapUser (this);
  Ptr<MmWavePhyMacCommon> phyMacConfig = CreateObject<MmWavePhyMacCommon> ();
  phyMacConfig->SetNumerology (0); // Doesn't matter, for the moment, what numerology

  ObjectFactory factory;
  factory.SetTypeId(m_scheduler);
  Ptr<MmWaveMacSchedulerNs3> sched = DynamicCast<MmWaveMacSchedulerNs3> (factory.Create());
  sched->ConfigureCommonParameters (phyMacConfig);
  NS_ABORT_MSG_IF(sched == nullptr, "Can't create a MmWaveMacSchedulerNs3 from type " + m_scheduler);

  TestSAPInterface (sched);
  TestAddingRemovingUsersNoData(sched);
  TestSchedNewData (sched);

  delete m_cSchedSapUser;
  delete m_schedSapUser;
}

class MmwaveTestSchedSuite : public TestSuite
{
public:
  MmwaveTestSchedSuite () : TestSuite ("mmwave-test-sched", SYSTEM)
    {
      AddTestCase(new MmWaveSchedGeneralTestCase ("ns3::MmWaveMacSchedulerTdmaRR", "TdmaRR test"), QUICK);
      AddTestCase(new MmWaveSchedGeneralTestCase ("ns3::MmWaveMacSchedulerTdmaPF", "TdmaPF test"), QUICK);
      AddTestCase(new MmWaveSchedGeneralTestCase ("ns3::MmWaveMacSchedulerOfdmaRR", "OfdmaRR test"), QUICK);
      AddTestCase(new MmWaveSchedGeneralTestCase ("ns3::MmWaveMacSchedulerOfdmaPF", "OfdmaPF test"), QUICK);
    }
};

static MmwaveTestSchedSuite mmwaveSchedTestSuite; //!< Mmwave scheduler test suite

}; // namespace ns3
