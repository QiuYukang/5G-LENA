// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/nr-mac-sched-sap.h"
#include "ns3/nr-mac-scheduler-ofdma-rr.h"
#include "ns3/nr-mac-scheduler-ofdma.h"
#include "ns3/nr-mac-short-bsr-ce.h"
#include "ns3/object-factory.h"
#include "ns3/test.h"

/**
 * @file nr-test-sched-symbols-per-beam.cc
 * @ingroup test
 *
 * @brief The class tests OFDMA time-domain schedulers (symbols per beam)
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
class NrSchedOfdmaSymbolPerBeamTestCase : public TestCase
{
  public:
    /**
     * @brief Create NrSchedOfdmaSymbolPerBeamTestCase
     * @param scheduler Scheduler to test
     * @param name Name of the test
     */
    NrSchedOfdmaSymbolPerBeamTestCase(std::string symPerBeamPolicyType)
        : TestCase(symPerBeamPolicyType),
          m_symPerBeamPolicyType(symPerBeamPolicyType)
    {
    }

  protected:
    void AddOneUser(uint16_t rnti, uint16_t sector, const Ptr<NrMacSchedulerNs3>& sched);
    void LcConfig(uint16_t rnti, uint64_t bytes, uint8_t cqi, const Ptr<NrMacSchedulerNs3>& sched);
    void SetUserData(uint16_t rnti, uint64_t bytes, const Ptr<NrMacSchedulerNs3>& sched);

  private:
    void DoRun() override;
    std::string m_symPerBeamPolicyType{};
};

void
NrSchedOfdmaSymbolPerBeamTestCase::AddOneUser(uint16_t rnti,
                                              uint16_t sector,
                                              const Ptr<NrMacSchedulerNs3>& sched)
{
    NrMacCschedSapProvider::CschedUeConfigReqParameters params;
    params.m_rnti = rnti;
    params.m_beamId = BeamId(sector, 120.0);
    sched->DoCschedUeConfigReq(params);
}

void
NrSchedOfdmaSymbolPerBeamTestCase::LcConfig(uint16_t rnti,
                                            uint64_t bytes,
                                            uint8_t cqi,
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
    cqiInfo.m_wbCqi = cqi;
    NrMacSchedSapProvider::SchedDlCqiInfoReqParameters schedDlCqiInfoReqParameters{};
    schedDlCqiInfoReqParameters.m_cqiList.push_back(cqiInfo);
    sched->DoSchedDlCqiInfoReq(schedDlCqiInfoReqParameters);
}

void
NrSchedOfdmaSymbolPerBeamTestCase::SetUserData(uint16_t rnti,
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
NrSchedOfdmaSymbolPerBeamTestCase::DoRun()
{
    Ptr<NrMacSchedulerOfdma> scheduler = CreateObject<NrMacSchedulerOfdmaRR>();
    scheduler->SetAttribute("SymPerBeamType", StringValue(m_symPerBeamPolicyType));

    // Create and configure SAPs
    NrMacCschedSapUser* cSchedSapUser = new TestCschedSapUser();
    NrMacSchedSapUser* schedSapUser = new TestSchedSymPerBeamSapUser();
    scheduler->SetMacCschedSapUser(cSchedSapUser);
    scheduler->SetMacSchedSapUser(schedSapUser);

    // Configure bandwidth in RBGs
    NrMacCschedSapProvider::CschedCellConfigReqParameters cellConfigReqParameters{};
    cellConfigReqParameters.m_dlBandwidth = 100;
    cellConfigReqParameters.m_ulBandwidth = 100;
    scheduler->DoCschedCellConfigReq(cellConfigReqParameters);

    // Create and configure dlAmc
    Ptr<NrAmc> dlAmc = CreateObject<NrAmc>();
    scheduler->InstallDlAmc(dlAmc);

    // Active UE and beam map
    NrMacSchedulerNs3::ActiveUeMap activeDl;
    NrMacSchedulerNs3::BeamSymbolMap beamSymbolMap;

    /**********************************************************************************************/
    // Schedule symbols per beams with no users nor beams (no symbol should be scheduled)
    activeDl.clear();
    scheduler->ComputeActiveUe(&activeDl,
                               &NrMacSchedulerUeInfo::GetDlLCG,
                               &NrMacSchedulerUeInfo::GetDlHarqVector,
                               "DL");
    beamSymbolMap = scheduler->GetSymPerBeam(12, activeDl);
    NS_TEST_EXPECT_MSG_EQ(activeDl.empty(), true, "Expected no active beams");
    NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.empty(),
                          true,
                          "Expected no symbols scheduled when there are no beams to schedule");
    /**********************************************************************************************/

    /**********************************************************************************************/
    // Create RNTI 1000, beam in sector 0, with 1 byte of fake data, CQI 15
    AddOneUser(1000, 0, scheduler);
    LcConfig(1000, 1, 15, scheduler);

    // Schedule (all symbols should go to that beam)
    activeDl.clear();
    scheduler->ComputeActiveUe(&activeDl,
                               &NrMacSchedulerUeInfo::GetDlLCG,
                               &NrMacSchedulerUeInfo::GetDlHarqVector,
                               "DL");
    beamSymbolMap = scheduler->GetSymPerBeam(12, activeDl);
    NS_TEST_EXPECT_MSG_EQ(activeDl.size(), 1, "Expected a single active beam");
    NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.size(),
                          1,
                          "Expected all symbols to be scheduled to the unique beam");

    if (m_symPerBeamPolicyType == "PROPORTIONAL_FAIR")
    {
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.begin()->second,
                              1,
                              "Expected a single symbol to be scheduled, since it is more than "
                              "enough for the active UE");
    }
    else
    {
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.begin()->second,
                              12,
                              "Expected all symbols to be schedule to the unique beam");
    }
    /**********************************************************************************************/

    /**********************************************************************************************/
    // Create RNTI 1001, in sector 1, with 100 byte of fake data, CQI 2
    AddOneUser(1001, 1, scheduler);
    LcConfig(1001, 1000000, 2, scheduler);
    // Schedule
    activeDl.clear();
    scheduler->ComputeActiveUe(&activeDl,
                               &NrMacSchedulerUeInfo::GetDlLCG,
                               &NrMacSchedulerUeInfo::GetDlHarqVector,
                               "DL");
    beamSymbolMap = scheduler->GetSymPerBeam(12, activeDl);
    NS_TEST_EXPECT_MSG_EQ(activeDl.size(), 2, "Expected two active beams");
    NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.size(),
                          2,
                          "Expected symbols to be split between two beams");

    if (m_symPerBeamPolicyType == "LOAD_BASED")
    {
        // In load-based, this sector one should have more symbols than sector 0
        NS_TEST_EXPECT_MSG_GT(beamSymbolMap.at({1, 120.0}),
                              beamSymbolMap.at({0, 120.0}),
                              "Expected more symbols for the beam with more load");
    }
    else if (m_symPerBeamPolicyType == "ROUND_ROBIN")
    {
        // In round-robin, beams should have the same +- 1 symbol than the other
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({1, 120.0}),
                              beamSymbolMap.at({0, 120.0}),
                              "Expected symbols to be split equally between two beams");
    }
    else if (m_symPerBeamPolicyType == "PROPORTIONAL_FAIR")
    {
        // In proportional-fair, this second beam should pretty much monopolize symbols
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({0, 120.0}),
                              1,
                              "Expected less symbols for the beam with less load");
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({1, 120.0}),
                              11,
                              "Expected more symbols for the beam with more load");
    }
    else
    {
        NS_ABORT_MSG("Unreachable condition");
    }
    /**********************************************************************************************/

    /**********************************************************************************************/
    // Create RNTI 1002, in sector 0, with same data as 1001 when combined with 1000, CQI 15
    AddOneUser(1002, 0, scheduler);
    LcConfig(1002, 999999, 15, scheduler);
    // Schedule
    activeDl.clear();
    scheduler->ComputeActiveUe(&activeDl,
                               &NrMacSchedulerUeInfo::GetDlLCG,
                               &NrMacSchedulerUeInfo::GetDlHarqVector,
                               "DL");
    beamSymbolMap = scheduler->GetSymPerBeam(12, activeDl);
    NS_TEST_EXPECT_MSG_EQ(activeDl.size(), 2, "Expected two active beams");
    NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.size(),
                          2,
                          "Expected symbols to be split between two beams");

    if (m_symPerBeamPolicyType == "LOAD_BASED" || m_symPerBeamPolicyType == "ROUND_ROBIN")
    {
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({1, 120.0}),
                              beamSymbolMap.at({0, 120.0}),
                              "Expected symbols to be split equally between two beams");
    }
    else if (m_symPerBeamPolicyType == "PROPORTIONAL_FAIR")
    {
        // In proportional-fair, this both beams should divide the symbols, but UEs on beam 0 have
        // CQI 15 while UE on beam 1 has CQI 2
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({0, 120.0}),
                              11,
                              "Expected more symbols initially for the beam with more UEs");
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({1, 120.0}),
                              1,
                              "Expected less symbols initially for the beam with less UEs");

        // Since PF has internal memory keeping track of fairness over time, if we schedule same
        // loads, we should get a different number of symbols per beam
        beamSymbolMap = scheduler->GetSymPerBeam(12, activeDl);
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({0, 120.0}),
                              6,
                              "Expected same number of symbols for fairness");
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({1, 120.0}),
                              6,
                              "Expected same number of symbols for fairness");
    }
    else
    {
        NS_ABORT_MSG("Unreachable condition");
    }
    /**********************************************************************************************/

    /**********************************************************************************************/
    // Create RNTI 1003, in sector 2, same traffic as other beams, CQI 8
    AddOneUser(1003, 2, scheduler);
    LcConfig(1003, 1000000, 8, scheduler);
    // Schedule
    activeDl.clear();
    scheduler->ComputeActiveUe(&activeDl,
                               &NrMacSchedulerUeInfo::GetDlLCG,
                               &NrMacSchedulerUeInfo::GetDlHarqVector,
                               "DL");
    beamSymbolMap = scheduler->GetSymPerBeam(11, activeDl); // schedule less symbols
    if (m_symPerBeamPolicyType == "LOAD_BASED" || m_symPerBeamPolicyType == "ROUND_ROBIN")
    {
        // In load-based and round-robin, beams should have the same +- 1 symbol than the other
        auto a = std::min_element(beamSymbolMap.begin(), beamSymbolMap.end(), [](auto a, auto b) {
            return a.second < a.second;
        });
        auto b = std::max_element(beamSymbolMap.begin(), beamSymbolMap.end(), [](auto a, auto b) {
            return a.second > a.second;
        });
        NS_TEST_EXPECT_MSG_GT_OR_EQ(a->second + 1,
                                    b->second,
                                    "Expected beams to receive about 50% of the symbols each");
    }
    else if (m_symPerBeamPolicyType == "PROPORTIONAL_FAIR")
    {
        // In proportional-fair, last beam should monopolize symbols, since other already had their
        // chance to transmit
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({0, 120.0}),
                              0,
                              "Expected 0 symbols since a new beam was added");
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({1, 120.0}),
                              0,
                              "Expected 0 symbols since a new beam was added");
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({2, 120.0}),
                              11,
                              "Expected 11 symbols to the new beam");

        // Since PF has internal memory keeping track of fairness over time, if we schedule same
        // loads, we should get a different number of symbols per beam
        beamSymbolMap = scheduler->GetSymPerBeam(12, activeDl);
        NS_TEST_EXPECT_MSG_EQ(
            beamSymbolMap.at({0, 120.0}),
            1,
            "Expected 1 symbol for same load as beam 1 and 2, but much higher CQI");
        NS_TEST_EXPECT_MSG_EQ(
            beamSymbolMap.at({1, 120.0}),
            2,
            "Expected 2 symbols for same load as beam 0 and 2, but much lower CQI");
        NS_TEST_EXPECT_MSG_EQ(beamSymbolMap.at({2, 120.0}),
                              9,
                              "Expected 9 symbols for same load as beam 0 and 1, average CQI, but "
                              "lower average throughput");
    }
    else
    {
        NS_ABORT_MSG("Unreachable condition");
    }
    /**********************************************************************************************/

    /**********************************************************************************************/
    if (m_symPerBeamPolicyType == "ROUND_ROBIN")
    {
        // We need to test if the order beams get symbols is correct in round-robin (due to internal
        // memory)
        auto getNextBeam = [&scheduler, &activeDl]() -> BeamId {
            auto symPerBeam = scheduler->GetSymPerBeam(1, activeDl);
            return std::max_element(symPerBeam.begin(),
                                    symPerBeam.end(),
                                    [](auto& a, auto& b) { return a.second < b.second; })
                ->first;
        };

        auto testSamplesStride = [this, &getNextBeam](int stride) {
            std::vector<BeamId> beams(stride * 2);
            for (int i = 0; i < stride * 2; i++)
            {
                beams.at(i) = getNextBeam();
            }
            for (int i = 0; i < stride - 1; i++)
            {
                for (int j = 0; j < stride; j++)
                {
                    if ((j != i) && (j != i + stride))
                    {
                        NS_TEST_EXPECT_MSG_NE(
                            beams[i],
                            beams[j],
                            "Round-robin is not giving symbols to other active beams");
                    }
                }
                NS_TEST_ASSERT_MSG_EQ(beams[i],
                                      beams[i + stride],
                                      "Round-robin is not giving symbols to other active beams");
            }
        };
        // Test if beams loop around
        testSamplesStride(3);

        // Zero-out each user data, and see if beams stop being scheduled
        SetUserData(1000, 0, scheduler);
        activeDl.clear();
        scheduler->ComputeActiveUe(&activeDl,
                                   &NrMacSchedulerUeInfo::GetDlLCG,
                                   &NrMacSchedulerUeInfo::GetDlHarqVector,
                                   "DL");
        testSamplesStride(3);

        // Zero another user in beam 0
        SetUserData(1002, 0, scheduler);
        activeDl.clear();
        scheduler->ComputeActiveUe(&activeDl,
                                   &NrMacSchedulerUeInfo::GetDlLCG,
                                   &NrMacSchedulerUeInfo::GetDlHarqVector,
                                   "DL");
        testSamplesStride(2);

        // Zero another user in beam 1
        SetUserData(1001, 0, scheduler);
        activeDl.clear();
        scheduler->ComputeActiveUe(&activeDl,
                                   &NrMacSchedulerUeInfo::GetDlLCG,
                                   &NrMacSchedulerUeInfo::GetDlHarqVector,
                                   "DL");
        testSamplesStride(1);

        // Then check if they come back in order after adding data
        SetUserData(1001, 100, scheduler);
        activeDl.clear();
        scheduler->ComputeActiveUe(&activeDl,
                                   &NrMacSchedulerUeInfo::GetDlLCG,
                                   &NrMacSchedulerUeInfo::GetDlHarqVector,
                                   "DL");
        testSamplesStride(2);
    }

    /**********************************************************************************************/

    delete schedSapUser;
    delete cSchedSapUser;
}

class NrTestSchedOfdmaSymbolPerBeamSuite : public TestSuite
{
  public:
    NrTestSchedOfdmaSymbolPerBeamSuite()
        : TestSuite("nr-test-sched-ofdma-symbol-per-beam", Type::UNIT)
    {
        AddTestCase(new NrSchedOfdmaSymbolPerBeamTestCase("LOAD_BASED"), Duration::QUICK);
        AddTestCase(new NrSchedOfdmaSymbolPerBeamTestCase("ROUND_ROBIN"), Duration::QUICK);
        AddTestCase(new NrSchedOfdmaSymbolPerBeamTestCase("PROPORTIONAL_FAIR"), Duration::QUICK);
    }
};

/// Test suite to test OFDMA time-domain symbol scheduling
static NrTestSchedOfdmaSymbolPerBeamSuite nrSchedOfdmaSymbolPerBeamTestSuite;

} // namespace ns3
