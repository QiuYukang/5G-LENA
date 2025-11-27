// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/config.h"
#include "ns3/nr-mac-sched-sap.h"
#include "ns3/nr-mac-scheduler-ofdma-rr.h"
#include "ns3/nr-mac-scheduler-ofdma.h"
#include "ns3/nr-mac-short-bsr-ce.h"
#include "ns3/object-factory.h"
#include "ns3/test.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <sstream>

/**
 * @file nr-test-sched-ofdma-frequency-domain.cc
 * @ingroup test
 *
 * @brief The class tests OFDMA frequency-domain schedulers
 */
namespace ns3
{

class TestCschedSapUser : public NrMacCschedSapUser
{
  public:
    TestCschedSapUser()
        : NrMacCschedSapUser()
    {
    }

    void CschedCellConfigCnf(
        [[maybe_unused]] const struct CschedCellConfigCnfParameters& params) override
    {
    }

    void CschedUeConfigCnf(
        [[maybe_unused]] const struct CschedUeConfigCnfParameters& params) override
    {
    }

    void CschedLcConfigCnf(
        [[maybe_unused]] const struct CschedLcConfigCnfParameters& params) override
    {
    }

    void CschedLcReleaseCnf(
        [[maybe_unused]] const struct CschedLcReleaseCnfParameters& params) override
    {
    }

    void CschedUeReleaseCnf(
        [[maybe_unused]] const struct CschedUeReleaseCnfParameters& params) override
    {
    }

    void CschedUeConfigUpdateInd(
        [[maybe_unused]] const struct CschedUeConfigUpdateIndParameters& params) override
    {
    }

    void CschedCellConfigUpdateInd(
        [[maybe_unused]] const struct CschedCellConfigUpdateIndParameters& params) override
    {
    }
};

class TestSchedSymPerBeamSapUser : public NrMacSchedSapUser
{
  public:
    TestSchedSymPerBeamSapUser()
        : NrMacSchedSapUser()
    {
    }

    void SchedConfigInd(const struct SchedConfigIndParameters& params) override
    {
    }

    // For the rest, setup some hard-coded values; for the moment, there is
    // no need to have real values here.
    Ptr<const SpectrumModel> GetSpectrumModel() const override
    {
        return nullptr;
    }

    uint32_t GetNumRbPerRbg() const override
    {
        return 1;
    }

    uint8_t GetNumHarqProcess() const override
    {
        return 20;
    }

    uint16_t GetBwpId() const override
    {
        return 0;
    }

    uint16_t GetCellId() const override
    {
        return 0;
    }

    uint32_t GetSymbolsPerSlot() const override
    {
        return 14;
    }

    Time GetSlotPeriod() const override
    {
        return MilliSeconds(1);
    }

    void BuildRarList(SlotAllocInfo& allocInfo) override
    {
    }
};

/**
 * @brief TestSched testcase
 */
class NrSchedOfdmaMcsTestCase : public TestCase
{
  public:
    static std::string GetTestName(std::string mcsCsiSource, std::vector<uint8_t> sbCqis)
    {
        std::stringstream ss;
        ss << "Scheduling with McsCsiSource=" << mcsCsiSource << " and sbCqis=[";
        for (auto cqi : sbCqis)
        {
            ss << std::to_string(cqi) << ",";
        }
        ss << "]";
        return ss.str();
    }

    /**
     * @brief Create NrSchedOfdmaMcsTestCase
     * @param scheduler Scheduler to test
     * @param name Name of the test
     */
    NrSchedOfdmaMcsTestCase(std::string mcsCsiSource,
                            uint32_t tbs,
                            std::set<uint16_t> scheduledRbgSet,
                            std::vector<uint8_t> sbCqis)
        : TestCase(GetTestName(mcsCsiSource, sbCqis)),
          m_mcsCsiSource(mcsCsiSource),
          m_expectedTbSize(tbs),
          m_scheduledRbgSet(scheduledRbgSet),
          m_sbCqis(sbCqis)
    {
    }

  protected:
    void AddOneUser(uint16_t rnti, uint16_t sector, const Ptr<NrMacSchedulerNs3>& sched);
    void LcConfig(uint16_t rnti, uint64_t bytes, const Ptr<NrMacSchedulerNs3>& sched);
    void SetUserData(uint16_t rnti, uint64_t bytes, const Ptr<NrMacSchedulerNs3>& sched);

  private:
    void DoRun() override;
    std::string m_mcsCsiSource{};
    uint32_t m_expectedTbSize{};
    std::set<uint16_t> m_scheduledRbgSet{};
    std::vector<uint8_t> m_sbCqis{};
};

void
NrSchedOfdmaMcsTestCase::AddOneUser(uint16_t rnti,
                                    uint16_t sector,
                                    const Ptr<NrMacSchedulerNs3>& sched)
{
    NrMacCschedSapProvider::CschedUeConfigReqParameters params;
    params.m_rnti = rnti;
    params.m_beamId = BeamId(sector, 120.0);
    sched->DoCschedUeConfigReq(params);
}

void
NrSchedOfdmaMcsTestCase::LcConfig(uint16_t rnti,
                                  uint64_t bytes,
                                  const Ptr<NrMacSchedulerNs3>& sched)
{
    // Create standard LCGs and LCs
    NrMacCschedSapProvider::CschedLcConfigReqParameters params;
    nr::LogicalChannelConfigListElement_s lc;
    params.m_rnti = rnti;
    params.m_reconfigureFlag = false;
    lc.m_direction = nr::LogicalChannelConfigListElement_s::Direction_e::DIR_BOTH;
    lc.m_qosBearerType = nr::LogicalChannelConfigListElement_s::QosBearerType_e::QBT_NON_GBR;
    lc.m_qci = 9;
    for (size_t i = 0; i < 4; i++)
    {
        lc.m_logicalChannelGroup = i;
        lc.m_logicalChannelIdentity = i;
        params.m_logicalChannelConfigList.emplace_back(lc);
    }
    sched->DoCschedLcConfigReq(params);
    SetUserData(rnti, bytes, sched);

    // Set CQI
    struct DlCqiInfo cqiInfo;
    cqiInfo.m_rnti = rnti;
    cqiInfo.m_wbCqi = std::accumulate(m_sbCqis.begin(), m_sbCqis.end(), 0) / m_sbCqis.size();
    cqiInfo.m_sbCqis = m_sbCqis;
    NrMacSchedSapProvider::SchedDlCqiInfoReqParameters schedDlCqiInfoReqParameters{};
    schedDlCqiInfoReqParameters.m_cqiList.push_back(cqiInfo);
    sched->DoSchedDlCqiInfoReq(schedDlCqiInfoReqParameters);
}

void
NrSchedOfdmaMcsTestCase::SetUserData(uint16_t rnti,
                                     uint64_t bytes,
                                     const Ptr<NrMacSchedulerNs3>& sched)
{
    // Notify scheduler of DL data in LC3 buffer
    NrMacSchedSapProvider::SchedDlRlcBufferReqParameters paramsData{};
    paramsData.m_rnti = rnti;
    paramsData.m_logicalChannelIdentity = 3;
    paramsData.m_rlcRetransmissionQueueSize = 0;
    paramsData.m_rlcTransmissionQueueSize = bytes;
    sched->DoSchedDlRlcBufferReq(paramsData);
}

void
NrSchedOfdmaMcsTestCase::DoRun()
{
    Config::SetDefault("ns3::NrPmSearch::SubbandSize", UintegerValue(1));
    Config::SetDefault("ns3::NrMacSchedulerNs3::McsCsiSource", StringValue(m_mcsCsiSource));
    Ptr<NrMacSchedulerOfdma> scheduler = CreateObject<NrMacSchedulerOfdmaRR>();

    // Create and configure SAPs
    NrMacCschedSapUser* cSchedSapUser = new TestCschedSapUser();
    NrMacSchedSapUser* schedSapUser = new TestSchedSymPerBeamSapUser();
    scheduler->SetMacCschedSapUser(cSchedSapUser);
    scheduler->SetMacSchedSapUser(schedSapUser);

    // Configure bandwidth in RBGs
    NrMacCschedSapProvider::CschedCellConfigReqParameters cellConfigReqParameters{};
    cellConfigReqParameters.m_dlBandwidth = 10;
    cellConfigReqParameters.m_ulBandwidth = 10;
    scheduler->DoCschedCellConfigReq(cellConfigReqParameters);

    // Create and configure dlAmc
    Ptr<NrAmc> dlAmc = CreateObject<NrAmc>();
    scheduler->InstallDlAmc(dlAmc);

    // Active UE and beam map
    NrMacSchedulerNs3::ActiveUeMap activeDl;
    NrMacSchedulerNs3::BeamSymbolMap beamSymbolMap;

    // Create RNTI 1000, beam in sector 0, with 10000 bytes of fake data, with WBCQI 9
    AddOneUser(1000, 0, scheduler);
    LcConfig(1000, 10000, scheduler);

    // Schedule (all symbols should go to that beam)
    activeDl.clear();
    auto symAvail = 12;
    scheduler->ComputeActiveUe(&activeDl,
                               &NrMacSchedulerUeInfo::GetDlLCG,
                               &NrMacSchedulerUeInfo::GetDlHarqVector,
                               "DL");
    // Let scheduler schedule RBGs, based on different
    // MCS approximations from WB or sub-band CQI
    auto scheduled = scheduler->AssignDLRBG(symAvail, activeDl);

    // Check if the expected TB size was assigned
    const auto ueInfo = activeDl.begin()->second.front().first;
    NS_TEST_EXPECT_MSG_EQ(ueInfo->m_dlTbSize, m_expectedTbSize, "Wrong TB size");

    // Check if expected RBGs were scheduled
    std::set<uint16_t> scheduledRbgSet(ueInfo->m_dlRBG.begin(), ueInfo->m_dlRBG.end());

    auto computeDifference = [](const std::set<uint16_t>& a, const std::set<uint16_t>& b) {
        std::set<uint16_t> diff;
        std::set_difference(a.begin(),
                            a.end(),
                            b.begin(),
                            b.end(),
                            std::inserter(diff, diff.begin()));
        return diff;
    };

    std::stringstream ss;
    auto missing = computeDifference(m_scheduledRbgSet, scheduledRbgSet);
    auto unexpected = computeDifference(scheduledRbgSet, m_scheduledRbgSet);

    if (!unexpected.empty())
    {
        ss << "Unexpected RBGs scheduled:";
        for (uint16_t rbg : unexpected)
        {
            ss << " " << rbg;
        }
        ss << ".";
    }

    if (!missing.empty())
    {
        ss << "Expected RBGs missing:";
        for (uint16_t rbg : missing)
        {
            ss << " " << rbg;
        }
        ss << ".";
    }

    NS_TEST_EXPECT_MSG_EQ(unexpected.empty() && missing.empty(), true, ss.str());

    delete schedSapUser;
    delete cSchedSapUser;
}

class NrTestSchedOfdmaFrequencyDomainSuite : public TestSuite
{
  public:
    NrTestSchedOfdmaFrequencyDomainSuite()
        : TestSuite("nr-test-sched-ofdma-frequency-domain", Type::UNIT)
    {
        AddTestCase(new NrSchedOfdmaMcsTestCase("WIDEBAND_MCS",
                                                313,
                                                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                                                {4, 5, 6, 7, 8, 9, 10, 11, 12, 13}),
                    Duration::QUICK);
        AddTestCase(new NrSchedOfdmaMcsTestCase("AVG_MCS",
                                                353,
                                                {1, 2, 3, 4, 5, 6, 7, 8, 9},
                                                {4, 5, 6, 7, 8, 9, 10, 11, 12, 13}),
                    Duration::QUICK);
        AddTestCase(new NrSchedOfdmaMcsTestCase("AVG_SPEC_EFF",
                                                353,
                                                {1, 2, 3, 4, 5, 6, 7, 8, 9},
                                                {4, 5, 6, 7, 8, 9, 10, 11, 12, 13}),
                    Duration::QUICK);
        AddTestCase(new NrSchedOfdmaMcsTestCase("WIDEBAND_MCS",
                                                195,
                                                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                                                {15, 15, 15, 1, 1, 1, 15, 1, 1, 1}),
                    Duration::QUICK);
        AddTestCase(new NrSchedOfdmaMcsTestCase("AVG_MCS",
                                                361,
                                                {0, 1, 2, 6},
                                                {15, 15, 15, 1, 1, 1, 15, 1, 1, 1}),
                    Duration::QUICK);
        // clang-format off
        AddTestCase(
            new NrSchedOfdmaMcsTestCase("AVG_MCS",
                                                179,
                                                {0, 1},
                                                {15, 15, 1, 1, 1, 1, 1, 1, 1, 1}),
                    Duration::QUICK);
        AddTestCase(
            new NrSchedOfdmaMcsTestCase("AVG_MCS",
                                                88,
                                                {0},
                                                {15, 1, 1, 1, 1, 1, 1, 1, 1, 1}),
                    Duration::QUICK);
        // clang-format on
        AddTestCase(new NrSchedOfdmaMcsTestCase("AVG_MCS",
                                                30,
                                                {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                                                {8, 1, 1, 1, 1, 1, 1, 1, 1, 1}),
                    Duration::QUICK);
    }
};

/// Test suite to test OFDMA frequency-domain resource scheduling
static NrTestSchedOfdmaFrequencyDomainSuite nrSchedOfdmaFrequencyDomainTestSuite;

} // namespace ns3
