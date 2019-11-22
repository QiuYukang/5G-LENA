/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 CTTC
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
#include <ns3/mmwave-enb-phy.h>
#include <ns3/node.h>
#include <ns3/nr-ch-access-manager.h>
#include <ns3/multi-model-spectrum-channel.h>

/**
 * \file nr-lte-pattern-generation
 * \ingroup test
 * \brief Unit-testing for the LTE/NR TDD pattern
 *
 * The test creates a fake MAC that checks if, when PHY calls the DL/UL slot
 * allocations, it does it for the right slot in pattern.
 *
 * In other words, if the PHY calls the UL slot allocation for a slot that
 * should be DL, the test will fail.
 */
namespace ns3 {

class TestEnbMac : public MmWaveEnbMac
{
public:
  static TypeId GetTypeId (void);
  TestEnbMac (const std::vector<LteNrTddSlotType> &pattern,
              const Ptr<MmWavePhyMacCommon> &config);
  virtual ~TestEnbMac (void) override;
  virtual void DoSlotDlIndication (const SfnSf &sfnSf, LteNrTddSlotType type) override;
  virtual void DoSlotUlIndication (const SfnSf &sfnSf, LteNrTddSlotType type) override;
  virtual void SetCurrentSfn (const SfnSf &sfn) override;

private:
  std::vector<LteNrTddSlotType> m_pattern;
  Ptr<MmWavePhyMacCommon> m_config;
  std::set <uint32_t> m_slotCreated;
  uint32_t m_totalSlotToCreate {0};
};

NS_OBJECT_ENSURE_REGISTERED (TestEnbMac);

TypeId
TestEnbMac::GetTypeId()
{
  static TypeId tid = TypeId ("ns3::TestEnbMac")
    .SetParent<MmWaveEnbMac> ()
    //.AddConstructor<TestEnbMac> ()
  ;
  return tid;
}

TestEnbMac::TestEnbMac (const std::vector<LteNrTddSlotType> &pattern,
                        const Ptr<MmWavePhyMacCommon> &config)
  : MmWaveEnbMac(),
    m_pattern (pattern),
    m_config (config)
{
  for (const auto & v : m_pattern)
    {
      switch (v) {
        case LteNrTddSlotType::F:
          m_totalSlotToCreate += 2; // But since we are using std::set,
                                    // duplicated slots will not be counted
         break;
         case LteNrTddSlotType::DL:
          m_totalSlotToCreate += 1;
         break;
         case LteNrTddSlotType::UL:
          m_totalSlotToCreate += 1;
         break;
         case LteNrTddSlotType::S:
          m_totalSlotToCreate += 1;
         break;
        }
    }
}

TestEnbMac::~TestEnbMac ()
{
  NS_ASSERT_MSG (m_slotCreated.size () == m_pattern.size (),
                 "The number of created slot (" << m_slotCreated.size () <<
                 ") is not equal to the pattern size " << m_pattern.size () <<
                 ", we have to create " << m_totalSlotToCreate << " slots");
}

void
TestEnbMac::DoSlotDlIndication (const SfnSf &sfnSf, LteNrTddSlotType type)
{
  uint32_t pos = sfnSf.Normalize (m_config->GetSlotsPerSubframe (), m_config->GetSubframesPerFrame ());
  pos = pos % m_pattern.size ();

  NS_ASSERT (type == LteNrTddSlotType::DL || type == LteNrTddSlotType::S || type == LteNrTddSlotType::F);
  NS_ASSERT_MSG (m_pattern[pos] == LteNrTddSlotType::DL ||
                 m_pattern[pos] == LteNrTddSlotType::S  ||
                 m_pattern[pos] == LteNrTddSlotType::F,
                 "MAC called to generate a DL slot, but in the pattern there is " << m_pattern[pos]);

  m_slotCreated.insert (pos);

  MmWaveEnbMac::DoSlotDlIndication (sfnSf, type);
}

void
TestEnbMac::DoSlotUlIndication (const SfnSf &sfnSf, LteNrTddSlotType type)
{
  uint32_t pos = sfnSf.Normalize (m_config->GetSlotsPerSubframe (), m_config->GetSubframesPerFrame ());
  pos = pos % m_pattern.size ();

  NS_ASSERT (type == LteNrTddSlotType::UL || type == LteNrTddSlotType::S || type == LteNrTddSlotType::F);
  NS_ASSERT_MSG (m_pattern[pos] == LteNrTddSlotType::UL ||
                 m_pattern[pos] == LteNrTddSlotType::F,
                 "MAC called to generate a UL slot, but in the pattern there is " << m_pattern[pos]);

  m_slotCreated.insert (pos);

  MmWaveEnbMac::DoSlotUlIndication (sfnSf, type);
}

void
TestEnbMac::SetCurrentSfn (const SfnSf &sfnSf)
{
  MmWaveEnbMac::SetCurrentSfn (sfnSf);
}

/**
 * \brief TestCase for the PHY TDD Patterns
 */
class LtePhyPatternTestCase : public TestCase
{
public:
  /**
   * \brief Create LtePatternTestCase
   * \param name Name of the test
   */
  LtePhyPatternTestCase (const std::vector<LteNrTddSlotType> &pattern, const std::string &name)
    : TestCase (name), m_pattern (pattern)
  {}

private:
  virtual void DoRun (void) override;
  void Print (const std::string &msg1, const std::string& msg2,
              const std::map<uint32_t, std::vector<uint32_t> > &str);
  void StartSimu ();
  Ptr<MmWaveEnbMac> CreateMac (const Ptr<MmWavePhyMacCommon> &config, const Ptr<MmWaveMacScheduler> &sched) const;
  Ptr<MmWaveEnbPhy> CreatePhy (const Ptr<MmWavePhyMacCommon> &config, const Ptr<MmWaveEnbMac> &mac) const;

  bool m_verbose = true;
  Ptr<MmWaveEnbPhy> m_phy;
  std::vector<LteNrTddSlotType> m_pattern;
};

static void
PerformBeamforming (const Ptr<NetDevice> &a, const Ptr<NetDevice> &b)
{
  NS_UNUSED (a);
  NS_UNUSED (b);
}

void
LtePhyPatternTestCase::DoRun ()
{
  Ptr<MmWavePhyMacCommon> configParams = CreateObject<MmWavePhyMacCommon> ();
  configParams->SetNumerology (0); // can it change?

  ObjectFactory schedFactory;
  schedFactory.SetTypeId (configParams->GetMacSchedType ());
  Ptr<MmWaveMacScheduler> sched = DynamicCast<MmWaveMacScheduler> (schedFactory.Create ());

  auto mac = CreateMac (configParams, sched);
  m_phy = CreatePhy (configParams, mac);

  m_phy->SetTddPattern (m_pattern);

  // Finishing initialization
  m_phy->SetPhySapUser (mac->GetPhySapUser());
  m_phy->Initialize ();
  mac->SetPhySapProvider (m_phy->GetPhySapProvider());
  mac->Initialize ();

  StartSimu ();
}

void
LtePhyPatternTestCase::StartSimu()
{
  Simulator::Stop (MilliSeconds (200));
  Simulator::Run ();
  Simulator::Destroy ();
}

Ptr<MmWaveEnbPhy>
LtePhyPatternTestCase::CreatePhy (const Ptr<MmWavePhyMacCommon> &config, const Ptr<MmWaveEnbMac> &mac) const
{
  static Node n;
  Ptr<MmWaveEnbPhy> phy;
  Ptr<MmWaveSpectrumPhy> channelPhy = CreateObject<MmWaveSpectrumPhy> ();

  phy = CreateObject<MmWaveEnbPhy> (channelPhy, &n);

  // PHY <--> Beamforming
  MmWaveEnbPhy::PerformBeamformingFn beamformingFn;
  beamformingFn = std::bind (&PerformBeamforming, std::placeholders::_1, std::placeholders::_2);
  phy->SetPerformBeamformingFn (beamformingFn);

  // PHY <--> CAM
  Ptr<NrChAccessManager> cam = DynamicCast<NrChAccessManager> (CreateObject<NrAlwaysOnAccessManager> ());
  cam->SetNrSpectrumPhy (channelPhy);
  cam->SetNrEnbMac (mac);
  phy->SetCam (cam);

  // PHY <--> HARQ
  Ptr<MmWaveHarqPhy> harq = Create<MmWaveHarqPhy> (20);
  channelPhy->SetHarqPhyModule (harq);

  phy->SetConfigurationParameters (config);
  return phy;
}

Ptr<MmWaveEnbMac>
LtePhyPatternTestCase::CreateMac(const Ptr<MmWavePhyMacCommon> &config,
                                 const Ptr<MmWaveMacScheduler> &sched) const
{
  Ptr<MmWaveEnbMac> mac = CreateObject<TestEnbMac> (m_pattern, config);
  mac->SetConfigurationParameters (config);

  sched->ConfigureCommonParameters (config);

  sched->SetMacSchedSapUser (mac->GetMmWaveMacSchedSapUser ());
  sched->SetMacCschedSapUser (mac->GetMmWaveMacCschedSapUser ());

  mac->SetMmWaveMacSchedSapProvider(sched->GetMacSchedSapProvider ());
  mac->SetMmWaveMacCschedSapProvider(sched->GetMacCschedSapProvider ());

  return mac;
}

void
LtePhyPatternTestCase::Print(const std::string &msg1, const std::string& msg2,
                          const std::map<uint32_t, std::vector<uint32_t> > &str)
{
  for (const auto & v : str)
    {
      for (const auto & i : v.second)
        {
          std::cout << msg1 << i << msg2 << v.first << std::endl;
        }
    }
}



class NrLtePatternsTestSuite : public TestSuite
{
public:
  NrLtePatternsTestSuite () : TestSuite ("nr-phy-patterns", UNIT)
    {

      auto one = {LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  };

      AddTestCase(new LtePhyPatternTestCase (one, "LTE TDD Pattern 1 test"), QUICK);

      auto two = {LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                 };
      AddTestCase(new LtePhyPatternTestCase (two, "LTE TDD Pattern 2 test"), QUICK);

      auto three = {LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                 };
      AddTestCase(new LtePhyPatternTestCase (three, "LTE TDD Pattern 3 test"), QUICK);

      auto four = {LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                 };
      AddTestCase(new LtePhyPatternTestCase (four, "LTE TDD Pattern 4 test"), QUICK);

      auto five = {LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::DL,
                 };
      AddTestCase(new LtePhyPatternTestCase (five, "LTE TDD Pattern 5 test"), QUICK);

      auto six = {LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                 };
      AddTestCase(new LtePhyPatternTestCase (six, "LTE TDD Pattern 6 test"), QUICK);

      auto zero = {LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::DL,
                  LteNrTddSlotType::S,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                  LteNrTddSlotType::UL,
                 };
      AddTestCase(new LtePhyPatternTestCase (zero, "LTE TDD Pattern 0 test"), QUICK);

      auto nr = {LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                  LteNrTddSlotType::F,
                 };
      AddTestCase(new LtePhyPatternTestCase (nr, "LTE TDD Pattern NR test"), QUICK);

    }
};

static NrLtePatternsTestSuite nrLtePatternsTestSuite;

//!< Pattern test suite

}; // namespace ns3
