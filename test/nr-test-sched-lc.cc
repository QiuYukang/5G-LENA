// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/config.h"
#include "ns3/nr-mac-sched-sap.h"
#include "ns3/nr-mac-scheduler-lc-alg.h"
#include "ns3/nr-mac-scheduler-lcg.h"
#include "ns3/nr-spectrum-phy.h"
#include "ns3/object-factory.h"
#include "ns3/test.h"

#include <unordered_map>

/**
 * @file nr-test-sched-lc.cc
 * @ingroup test
 *
 * @brief This file contain tests for the round-robin nature of nr-mac-scheduler-lc-rr.
 * It tests that different logical channels get scheduled the necessary amount of bytes,
 * according to their requirements. And if there are leftover bytes, they are distributed
 * properly, so all bytes in a txop are available for use by LCs.
 *
 */
namespace ns3
{

using LCG = std::unordered_map<uint8_t, LCGPtr>;

class NrTestMacSchedLcRr : public TestCase
{
  public:
    /**
     * @brief Constructor for NrTestMacSchedLcRr.
     *
     * Initializes the test case with the given description, logical channel scheduler type,
     * an allocation map of logical channel groups (LCGs), transport block size (TBS),
     * and the expected assigned bytes for each logical channel group.
     *
     * @param description The description of the test case.
     * @param lcType The type identifier for the logical channel scheduler implementation tested.
     * @param lcgToAllocate A vector representing the LCGs and their associated byte allocations,
     *                      given as pairs containing the LCG ID and its bytes.
     * @param tbs The transport block size (TBS) to be allocated for scheduling.
     * @param assignedBytes A vector containing expected assigned byte allocations per LCG,
     *                      given as pairs of LCG ID and byte count.
     *
     * The constructor performs the following:
     * - Initializes the base TestCase class with the given description.
     * - Stores the specified transport block size and expected assigned bytes.
     * - Configures the logical channel (LC) scheduler factory with the given lcType.
     * - Constructs logical channel group (LCG) structures and logical channels within each LCG
     * using the provided allocation map.
     * - Updates the LCGs with their respective byte sizes for scheduling purposes.
     */
    NrTestMacSchedLcRr(const std::string description,
                       const std::string lcType,
                       std::vector<std::pair<uint8_t, uint32_t>> lcgToAllocate,
                       uint32_t tbs,
                       std::vector<std::pair<uint8_t, uint32_t>> assignedBytes)
        : TestCase(description),
          m_tbSize(tbs),
          m_expectedAssignedBytes(assignedBytes)
    {
        // Set factory of LC scheduler to tested type
        m_lcFactory.SetTypeId(TypeId::LookupByName(lcType));
        // Build LCG structure from vector of pairs
        for (auto [lcgId, lcgBytes] : lcgToAllocate)
        {
            auto lcgEntry = std::make_unique<NrMacSchedulerLCG>(lcgId);
            m_lcg.emplace(lcgId, std::move(lcgEntry));
            nr::LogicalChannelConfigListElement_s config{.m_qci = 5};
            auto lcEntry = std::make_unique<NrMacSchedulerLC>(config);
            lcEntry->m_id = lcgId;
            m_lcg.at(lcgId)->Insert(std::move(lcEntry));
            NrMacSchedSapProvider::SchedDlRlcBufferReqParameters params{};
            params.m_logicalChannelIdentity = lcgId;
            params.m_rlcTransmissionQueueSize = lcgBytes;
            m_lcg.at(lcgId)->UpdateInfo(params);
        }
    }

  protected:
    void DoRun() override;
    ObjectFactory m_lcFactory;
    LCG m_lcg;
    uint32_t m_tbSize;
    std::vector<std::pair<uint8_t, uint32_t>> m_expectedAssignedBytes;
};

void
NrTestMacSchedLcRr::DoRun()
{
    auto schedLc = DynamicCast<NrMacSchedulerLcAlgorithm>(m_lcFactory.Create());
    // Call DL/UL assign bytes functions
    auto assignedBytesDl = schedLc->AssignBytesToDlLC(m_lcg, m_tbSize, MilliSeconds(0));
    auto assignedBytesUl = schedLc->AssignBytesToUlLC(m_lcg, m_tbSize);
    // Check if both allocate the expected number of bytes per LCG
    for (auto assignedBytes : {&assignedBytesDl, &assignedBytesUl})
    {
        uint32_t totalAssignedBytes = 0;
        for (auto [lcgId, lcgBytes] : m_expectedAssignedBytes)
        {
            auto lcgAssigned = std::find_if(assignedBytes->begin(),
                                            assignedBytes->end(),
                                            [lcgId](auto& entry) { return entry.m_lcg == lcgId; });
            if (lcgAssigned != assignedBytes->end())
            {
                NS_TEST_ASSERT_MSG_EQ(lcgAssigned->m_bytes,
                                      lcgBytes,
                                      "Expected " << lcgBytes << " bytes assigned for LCG "
                                                  << +lcgId);
                totalAssignedBytes += lcgAssigned->m_bytes;
            }
            else
            {
                if (m_expectedAssignedBytes.at(lcgId).second != 0)
                {
                    NS_TEST_ASSERT_MSG_EQ(false,
                                          true,
                                          "Expected LCG " << +lcgId << " to be assigned");
                }
            }
        }

        if (!m_expectedAssignedBytes.empty())
        {
            NS_TEST_ASSERT_MSG_EQ(totalAssignedBytes,
                                  m_tbSize,
                                  "Expected all " << m_tbSize << " bytes to be assigned");
        }

        if (m_tbSize == 0)
        {
            NS_TEST_ASSERT_MSG_EQ(totalAssignedBytes, 0, "Expected no LCGs to be assigned");
        }
    }
}

class NrTestSchedLcSuite : public TestSuite
{
  public:
    NrTestSchedLcSuite()
        : TestSuite("nr-test-sched-lc", Type::UNIT)
    {
        // clang-format off
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (), tbs = 0",               "ns3::NrMacSchedulerLcRR",{},                    0, {}                   ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (), tbs = 1",               "ns3::NrMacSchedulerLcRR",{},                    1, {}                   ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:0B), tbs = 0",           "ns3::NrMacSchedulerLcRR",{{1,0}},               0, {}                   ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B), tbs = 0",           "ns3::NrMacSchedulerLcRR",{{1,1}},               0, {}                   ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B), tbs = 1",           "ns3::NrMacSchedulerLcRR",{{1,1}},               1, {{1,1}}              ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B), tbs = 2",           "ns3::NrMacSchedulerLcRR",{{1,1}},               2, {{1,2}}              ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:1B), tbs = 0",      "ns3::NrMacSchedulerLcRR",{{1,1}, {2,1}},        0, {}                   ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:1B), tbs = 1",      "ns3::NrMacSchedulerLcRR",{{1,1}, {2,1}},        1, {{1,1}}              ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:1B), tbs = 2",      "ns3::NrMacSchedulerLcRR",{{1,1}, {2,1}},        2, {{1,1}, {2,1}}       ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:1B), tbs = 3",      "ns3::NrMacSchedulerLcRR",{{1,1}, {2,1}},        3, {{1,2}, {2,1}}       ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:1B), tbs = 4",      "ns3::NrMacSchedulerLcRR",{{1,1}, {2,1}},        4, {{1,2}, {2,2}}       ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 0", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 0, {}                   ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 1", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 1, {{1,1}}              ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 2", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 2, {{1,1}, {2,1}}       ), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 3", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 3, {{1,1}, {2,1}, {3,1}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 4", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 4, {{1,1}, {2,2}, {3,1}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 5", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 5, {{1,1}, {2,2}, {3,2}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 6", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 6, {{1,1}, {2,2}, {3,3}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 7", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 7, {{1,2}, {2,2}, {3,3}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 8", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 8, {{1,2}, {2,3}, {3,3}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:3B), tbs = 9", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,3}}, 9, {{1,2}, {2,3}, {3,4}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:300B), tbs = 300", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,300}}, 300, {{1,1}, {2,2}, {3,297}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:297B), tbs = 309", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,297}}, 309, {{1,4}, {2,5}, {3,300}}), Duration::QUICK);
        AddTestCase(new NrTestMacSchedLcRr("LcRR flows (1:1B,2:2B,3:300B), tbs = 309", "ns3::NrMacSchedulerLcRR",{{1,1}, {2,2}, {3,300}}, 309, {{1,3}, {2,4}, {3,302}}), Duration::QUICK);
        // clang-format on
    }
};

static NrTestSchedLcSuite nrSchedLcTestSuite; //!< NR LC scheduler test suite

} // namespace ns3
