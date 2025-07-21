// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/config.h"
#include "ns3/nr-mac-sched-sap.h"
#include "ns3/nr-mac-scheduler-ns3.h"
#include "ns3/nr-mac-scheduler-ofdma-rr.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-spectrum-phy.h"
#include "ns3/object-factory.h"
#include "ns3/test.h"

/**
 * @file nr-test-sched-harq.cc
 * @ingroup test
 *
 * @brief This file contain tests for the round-robin nature of nr-mac-scheduler-harq-rr.
 * It also tests if allocations are properly consolidated to use less symbols,
 * and maintain or increase MCS, in order to increase the change of successful decoding.
 *
 */
namespace ns3
{
class TestCschedSapUserHarq : public NrMacCschedSapUser
{
  public:
    TestCschedSapUserHarq()
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

class TestSchedSapUserHarq : public NrMacSchedSapUser
{
  public:
    TestSchedSapUserHarq(
        std::function<void(const struct SchedConfigIndParameters&)> schedConfigIndCallback =
            [](auto params) {},
        std::function<uint32_t(void)> symbolsPerSlotCallback = []() { return 14; })
        : NrMacSchedSapUser(),
          m_schedConfigIndCallback(schedConfigIndCallback),
          m_symbolsPerSlotCallback(symbolsPerSlotCallback)
    {
    }

    void SchedConfigInd(const NrMacSchedSapUser::SchedConfigIndParameters& params) override
    {
        if (m_schedConfigIndCallback)
        {
            m_schedConfigIndCallback(params);
        }
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
        return m_symbolsPerSlotCallback();
    }

    Time GetSlotPeriod() const override
    {
        return MilliSeconds(1);
    }

    void BuildRarList(SlotAllocInfo& allocInfo) override
    {
    }

  private:
    std::function<void(const NrMacSchedSapUser::SchedConfigIndParameters&)>
        m_schedConfigIndCallback;
    std::function<uint32_t()> m_symbolsPerSlotCallback;
};

/**
 * @brief TestSched testcase
 */
class NrTestMacSchedulerHarqRrReshape : public TestCase
{
  public:
    /**
     * @brief Create NrSchedGeneralTestCase
     * @param scheduler Scheduler to test
     * @param name Name of the test
     */
    NrTestMacSchedulerHarqRrReshape(std::vector<DciInfoElementTdma> dcis,
                                    uint8_t startingSymbol,
                                    uint8_t numSymbols,
                                    const std::string& name)
        : TestCase(name),
          m_dcis(dcis),
          m_startingSymbol(startingSymbol),
          m_numSymbols(numSymbols)
    {
    }

  protected:
    void DoRun() override;
    const std::vector<DciInfoElementTdma> m_dcis;
    const uint8_t m_startingSymbol;
    const uint8_t m_numSymbols;
};

void
NrTestMacSchedulerHarqRrReshape::DoRun()
{
    // Prepare common settings for both TDMA and OFDMA schedulers
    auto cellConfig = NrMacCschedSapProvider::CschedCellConfigReqParameters();
    cellConfig.m_dlBandwidth = 10; // 10 RBGs
    cellConfig.m_ulBandwidth = 10;

    std::vector<NrMacCschedSapProvider::CschedUeConfigReqParameters> ueConfig;
    for (const auto& dci : m_dcis)
    {
        NrMacCschedSapProvider::CschedUeConfigReqParameters config{};
        config.m_rnti = dci.m_rnti;
        config.m_transmissionMode = 0;
        config.m_beamId = BeamId(dci.m_rnti / 5, 0);
        ueConfig.push_back(config);
    }

    auto* schedSapUser = new TestSchedSapUserHarq();
    auto* cschedSapUser = new TestCschedSapUserHarq();

    for (bool isTdma : {true, false})
    {
        Ptr<NrMacSchedulerNs3> scheduler;
        if (isTdma)
        {
            scheduler = CreateObject<NrMacSchedulerTdmaRR>();
        }
        else
        {
            scheduler = CreateObject<NrMacSchedulerOfdmaRR>();
        }
        scheduler->SetMacSchedSapUser(schedSapUser);
        scheduler->SetMacCschedSapUser(cschedSapUser);
        scheduler->DoCschedCellConfigReq(cellConfig);
        for (const auto& ueConf : ueConfig)
        {
            scheduler->DoCschedUeConfigReq(ueConf);
        }
        const bool isDl = true;
        std::vector<bool> bitmask(10, true);
        auto startingSymbol = m_startingSymbol;
        auto numSymbols = m_numSymbols;
        auto reshapedDcisTdma =
            scheduler->ReshapeAllocation(m_dcis, startingSymbol, numSymbols, bitmask, isDl);

        // Check if we went above number of available symbols
        auto reshapedAllocatedSymbols = 0;
        if (!reshapedDcisTdma.empty())
        {
            auto smallestStartSymbol = std::numeric_limits<uint8_t>::max();
            auto largestFinalSymbol = m_startingSymbol;
            for (auto& dci : reshapedDcisTdma)
            {
                if (smallestStartSymbol > dci.m_symStart)
                {
                    smallestStartSymbol = dci.m_symStart;
                }
                if (largestFinalSymbol < (dci.m_symStart + dci.m_numSym))
                {
                    largestFinalSymbol = dci.m_symStart + dci.m_numSym;
                }
            }
            NS_ASSERT_MSG(smallestStartSymbol != std::numeric_limits<uint8_t>::max(),
                          "There must have been a valid starting symbol");
            reshapedAllocatedSymbols = largestFinalSymbol - smallestStartSymbol;
        }
        NS_TEST_ASSERT_MSG_LT_OR_EQ(
            reshapedAllocatedSymbols,
            +m_numSymbols,
            (isTdma ? "TDMA" : "OFDMA")
                << ": Reshaped unexpectedly into more symbols than available");

        // If the test case has no symbols, do not continue which checks, because the one above
        // should suffice
        if (m_numSymbols == 0)
        {
            continue;
        }

        // If there is no reshaped DCI, we do not continue checks
        // (temporary until reshape can handle multiple DCIs, and later multiple beams)
        if (reshapedDcisTdma.empty())
        {
            continue;
        }

        // Test we haven't changed what we are not supposed to change
        for (auto& reshapedDci : reshapedDcisTdma)
        {
            const auto originalDci =
                std::find_if(m_dcis.begin(), m_dcis.end(), [=](DciInfoElementTdma a) {
                    return reshapedDci.m_rnti == a.m_rnti;
                });
            NS_TEST_EXPECT_MSG_EQ((originalDci != m_dcis.end()),
                                  true,
                                  "Reshaped allocation changed DCI RNTI");
            NS_TEST_ASSERT_MSG_EQ(reshapedDci.m_format,
                                  originalDci->m_format,
                                  "Reshaped allocation changed DCI format");
            NS_TEST_ASSERT_MSG_EQ(+reshapedDci.m_mcs,
                                  +originalDci->m_mcs,
                                  "Reshaped allocation changed DCI MCS");
            NS_TEST_ASSERT_MSG_EQ(+reshapedDci.m_rank,
                                  +originalDci->m_rank,
                                  "Reshaped allocation changed DCI rank");
            NS_TEST_ASSERT_MSG_EQ(reshapedDci.m_precMats,
                                  originalDci->m_precMats,
                                  "Reshaped allocation changed DCI Precoding matrices");
            NS_TEST_ASSERT_MSG_EQ(reshapedDci.m_tbSize,
                                  originalDci->m_tbSize,
                                  "Reshaped allocation changed DCI TBS");
            NS_TEST_ASSERT_MSG_EQ(reshapedDci.m_ndi,
                                  originalDci->m_ndi,
                                  "Reshaped allocation changed DCI NDI");
            NS_TEST_ASSERT_MSG_EQ(reshapedDci.m_rv,
                                  originalDci->m_rv,
                                  "Reshaped allocation changed DCI HARQ RV");
            NS_TEST_ASSERT_MSG_EQ(reshapedDci.m_type,
                                  originalDci->m_type,
                                  "Reshaped allocation changed DCI type");
            NS_TEST_ASSERT_MSG_EQ(reshapedDci.m_bwpIndex,
                                  originalDci->m_bwpIndex,
                                  "Reshaped allocation changed DCI BWP index");
            NS_TEST_ASSERT_MSG_EQ(reshapedDci.m_tpc,
                                  originalDci->m_tpc,
                                  "Reshaped allocation changed DCI TPC");

            // Test if we changed what we are supposed to change
            if (isTdma)
            {
                NS_TEST_ASSERT_MSG_GT_OR_EQ(
                    +std::count(reshapedDci.m_rbgBitmask.begin(),
                                reshapedDci.m_rbgBitmask.end(),
                                true),
                    +std::count(originalDci->m_rbgBitmask.begin(),
                                originalDci->m_rbgBitmask.end(),
                                true),
                    "Reshaped TDMA allocation unexpectedly has less RBGs than the original");
                NS_TEST_ASSERT_MSG_LT_OR_EQ(
                    +reshapedDci.m_numSym,
                    +originalDci->m_numSym,
                    "Reshaped TDMA allocation unexpectedly has more symbols than the original");
                NS_TEST_ASSERT_MSG_GT_OR_EQ(
                    reshapedDci.m_numSym * std::count(reshapedDci.m_rbgBitmask.begin(),
                                                      reshapedDci.m_rbgBitmask.end(),
                                                      true),
                    originalDci->m_numSym * std::count(originalDci->m_rbgBitmask.begin(),
                                                       originalDci->m_rbgBitmask.end(),
                                                       true),
                    "Reshaped TDMA allocation unexpectedly has less resources than the original");
            }
            else
            {
            }
        }
    }
    delete schedSapUser;
    delete cschedSapUser;
}

class NrTestMacSchedulerHarqRrScheduleDlHarq : public NrTestMacSchedulerHarqRrReshape
{
  public:
    /**
     * @brief Create NrSchedGeneralTestCase
     * @param scheduler Scheduler to test
     * @param name Name of the test
     */
    NrTestMacSchedulerHarqRrScheduleDlHarq(std::vector<DciInfoElementTdma> dcis,
                                           uint8_t startingSymbol,
                                           uint8_t numSymbols,
                                           const std::string& name)
        : NrTestMacSchedulerHarqRrReshape(dcis, startingSymbol, numSymbols, name)
    {
    }

    void CheckSchedule(const NrMacSchedSapUser::SchedConfigIndParameters& params);

  protected:
    void DoRun() override;
    bool m_testingTdma{false};
};

void
NrTestMacSchedulerHarqRrScheduleDlHarq::CheckSchedule(
    const NrMacSchedSapUser::SchedConfigIndParameters& params)
{
    // Retrieve resulting scheduled HARQ DCIs
    std::vector<DciInfoElementTdma> resultingDcis;
    for (const auto& varTtiAllocInfo : params.m_slotAllocInfo.m_varTtiAllocInfo)
    {
        // We only want data, no control DCIs
        if (varTtiAllocInfo.m_dci->m_type == DciInfoElementTdma::DATA)
        {
            resultingDcis.push_back(*varTtiAllocInfo.m_dci);
        }
    }

    if (m_testingTdma)
    {
        // TDMA should minimize symbols used by HARQ allocations, to fit more HARQ retransmissions
        // in a slot
        for (auto& resultingDci : resultingDcis)
        {
            auto& originalDci =
                *std::find_if(m_dcis.begin(), m_dcis.end(), [resultingDci](auto& a) {
                    return a.m_rnti == resultingDci.m_rnti;
                });
            NS_TEST_EXPECT_MSG_LT_OR_EQ(+resultingDci.m_numSym,
                                        +originalDci.m_numSym,
                                        "Number of symbols for TDMA should be same or smaller");

            auto originalRbgs =
                std::count(originalDci.m_rbgBitmask.begin(), originalDci.m_rbgBitmask.end(), true);
            auto resultingRbgs = std::count(resultingDci.m_rbgBitmask.begin(),
                                            resultingDci.m_rbgBitmask.end(),
                                            true);
            NS_TEST_ASSERT_MSG_EQ(resultingRbgs * resultingDci.m_numSym,
                                  originalRbgs * originalDci.m_numSym,
                                  "Number of allocated resources should not change (error model "
                                  "assumes it remains constant)");
        }
    }
    // Testing OFDMA
    else
    {
        // OFDMA should maximize symbols used by HARQ retransmissions, in order to make better use
        // of RBGs for other retransmissions. However, the total number of resources of
        // retransmissions of a symbol should use the least amount of symbols possible, to (if
        // possible) have more beams in a given slot.
    }
}

void
NrTestMacSchedulerHarqRrScheduleDlHarq::DoRun()
{
    Config::SetDefault("ns3::NrMacSchedulerHarqRr::ConsolidateHarqRetx", BooleanValue(true));

    // Prepare common settings for both TDMA and OFDMA schedulers
    auto cellConfig = NrMacCschedSapProvider::CschedCellConfigReqParameters();
    cellConfig.m_dlBandwidth = 10; // 10 RBGs
    cellConfig.m_ulBandwidth = 10;

    std::vector<NrMacCschedSapProvider::CschedUeConfigReqParameters> ueConfig;
    for (const auto& dci : m_dcis)
    {
        NrMacCschedSapProvider::CschedUeConfigReqParameters config{};
        config.m_rnti = dci.m_rnti;
        config.m_transmissionMode = 0;
        config.m_beamId = BeamId(dci.m_rnti / 5, 0);
        ueConfig.push_back(config);
    }

    auto* schedSapUser = new TestSchedSapUserHarq([this](auto params) { CheckSchedule(params); },
                                                  [this]() { return m_numSymbols; });
    auto* cschedSapUser = new TestCschedSapUserHarq();

    Ptr<NrMacSchedulerNs3> sched;

    // Instead of using of reshaping straight from scheduler,
    // reproduce the conditions to call it via the scheduler->HARQ scheduler->reshape
    for (bool isTdma : {true, false})
    {
        // Create scheduler
        if (isTdma)
        {
            sched = CreateObject<NrMacSchedulerTdmaRR>();
        }
        else
        {
            sched = CreateObject<NrMacSchedulerOfdmaRR>();
        }
        sched->InstallDlAmc(CreateObject<NrAmc>());
        sched->InstallUlAmc(CreateObject<NrAmc>());

        // Configure scheduler
        sched->SetMacSchedSapUser(schedSapUser);
        sched->SetMacCschedSapUser(cschedSapUser);
        sched->DoCschedCellConfigReq(cellConfig);
        for (const auto& ueConf : ueConfig)
        {
            sched->DoCschedUeConfigReq(ueConf);
        }

        // Set starting symbol
        sched->SetDlCtrlSyms(m_startingSymbol);

        // Create scheduler parameters
        NrMacSchedSapProvider::SchedDlTriggerReqParameters paramsDlTrigger;
        paramsDlTrigger.m_snfSf = SfnSf(0, 0, 0, 0);
        paramsDlTrigger.m_slotType = LteNrTddSlotType::DL;
        paramsDlTrigger.m_dlHarqInfoList = {};

        // Activate harq processes and populate harq info list
        for (const auto& dci : m_dcis)
        {
            auto& ueInfo = sched->m_ueMap.find(dci.m_rnti)->second;
            auto& harqProcess = ueInfo->m_dlHarq.Find(dci.m_rnti)->second;
            harqProcess.m_dciElement = std::make_shared<DciInfoElementTdma>(dci);
            harqProcess.m_active = true;

            DlHarqInfo harqInfo;
            harqInfo.m_harqStatus = DlHarqInfo::NACK;
            harqInfo.m_numRetx = 0;
            harqInfo.m_rnti = dci.m_rnti;
            harqInfo.m_harqProcessId = dci.m_rnti;
            harqInfo.m_bwpIndex = 0;
            paramsDlTrigger.m_dlHarqInfoList.push_back(harqInfo);
        }

        // Indicate CheckSchedule should check for TDMA or OFDMA
        m_testingTdma = isTdma;

        // Call ScheduleDl
        sched->DoSchedDlTriggerReq(paramsDlTrigger);
    }
    delete schedSapUser;
    delete cschedSapUser;
}

class NrTestSchedHarqSuite : public TestSuite
{
  public:
    NrTestSchedHarqSuite()
        : TestSuite("nr-test-sched-harq", Type::UNIT)
    {
        // clang-format off
        using DIET = DciInfoElementTdma;
        std::vector<DciInfoElementTdma> dcis{
            // beam 0
            // rnti,   format, startSym, numSym, mcs, rank, precmat, tbs, ndi, rv,       type, bwp, tpc
            {     0, DIET::DL,        0,      4,  10,    4,      {}, 800,   0,  0, DIET::DATA,   0,   0},
            {     1, DIET::DL,        1,      7,  17,    1,      {}, 200,   0,  0, DIET::DATA,   0,   0},
            {     2, DIET::DL,        2,      1,  13,    3,      {}, 600,   0,  0, DIET::DATA,   0,   0},
            {     3, DIET::DL,        3,      9,  10,    2,      {}, 400,   0,  0, DIET::DATA,   0,   0},
            {     4, DIET::DL,        4,      2,   6,    4,      {}, 800,   0,  0, DIET::DATA,   0,   0},
            // beam 1
            {     5, DIET::DL,        5,      5,  20,    4,      {}, 800,   0,  0, DIET::DATA,   0,   0},
            {     6, DIET::DL,        6,      3,  10,    1,      {}, 200,   0,  0, DIET::DATA,   0,   0},
            {     7, DIET::DL,        7,      8,   1,    1,      {}, 200,   0,  0, DIET::DATA,   0,   0},
            {     8, DIET::DL,        8,      1,   7,    3,      {}, 600,   0,  0, DIET::DATA,   0,   0},
            {     9, DIET::DL,        9,      2,  19,    2,      {}, 400,   0,  0, DIET::DATA,   0,   0}
        };
        dcis.at(0).m_rbgBitmask = { true,  true, false, false, false,  true, false, false,  true,  true};
        dcis.at(1).m_rbgBitmask = {false,  true,  true, false,  true,  true,  true, false,  true, false};
        dcis.at(2).m_rbgBitmask = {false, false,  true, false, false,  true,  true,  true,  true,  true};
        dcis.at(3).m_rbgBitmask = { true, false,  true, false,  true, false,  true, false,  true, false};
        dcis.at(4).m_rbgBitmask = {false,  true, false,  true, false,  true, false,  true, false,  true};
        dcis.at(5).m_rbgBitmask = { true, false,  true,  true, false, false, false, false, false,  true};
        dcis.at(6).m_rbgBitmask = { true, false, false,  true, false,  true, false, false, false,  true};
        dcis.at(7).m_rbgBitmask = { true, false, false, false,  true,  true,  true, false, false,  true};
        dcis.at(8).m_rbgBitmask = { true, false, false, false, false,  true,  true,  true, false,  true};
        dcis.at(9).m_rbgBitmask = { true,  true, false,  true, false,  true,  true, false, false, false};
        dcis.at(0).m_harqProcess = 0;
        dcis.at(1).m_harqProcess = 1;
        dcis.at(2).m_harqProcess = 2;
        dcis.at(3).m_harqProcess = 3;
        dcis.at(4).m_harqProcess = 4;
        dcis.at(5).m_harqProcess = 5;
        dcis.at(6).m_harqProcess = 6;
        dcis.at(7).m_harqProcess = 7;
        dcis.at(8).m_harqProcess = 8;
        dcis.at(9).m_harqProcess = 9;
        // clang-format on
        for (auto [startSym, numSym] : std::vector<std::pair<uint8_t, uint8_t>>{
                 {0, 0},
                 {0, 13},
                 {6, 14},
                 {0, 1},
                 {1, 13},
             })
        {
            // Test reshaping alone
            AddTestCase(new NrTestMacSchedulerHarqRrReshape(
                            std::vector<DIET>(dcis.begin(), dcis.begin() + 4),
                            startSym,
                            numSym,
                            "Reshape: Beam   0, startSym " + std::to_string(startSym) +
                                ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrReshape(
                            std::vector<DIET>(dcis.begin() + 6, dcis.end()),
                            startSym,
                            numSym,
                            "Reshape: Beam   1, startSym " + std::to_string(startSym) +
                                ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrReshape(
                            std::vector<DIET>(dcis.begin() + 3, dcis.begin() + 7),
                            startSym,
                            numSym,
                            "Reshape: Beam 0+1, startSym " + std::to_string(startSym) +
                                ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            // Test reshaping via scheduler
            AddTestCase(new NrTestMacSchedulerHarqRrScheduleDlHarq(
                            std::vector<DIET>(dcis.begin(), dcis.begin() + 1),
                            startSym,
                            numSym,
                            "Reshape with scheduler: Beam   0, startSym " +
                                std::to_string(startSym) + ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrScheduleDlHarq(
                            std::vector<DIET>(dcis.begin() + 1, dcis.begin() + 2),
                            startSym,
                            numSym,
                            "Reshape with scheduler: Beam   0, startSym " +
                                std::to_string(startSym) + ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrScheduleDlHarq(
                            std::vector<DIET>(dcis.begin() + 2, dcis.begin() + 3),
                            startSym,
                            numSym,
                            "Reshape with scheduler: Beam   0, startSym " +
                                std::to_string(startSym) + ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrScheduleDlHarq(
                            std::vector<DIET>(dcis.begin() + 3, dcis.begin() + 4),
                            startSym,
                            numSym,
                            "Reshape with scheduler: Beam   0, startSym " +
                                std::to_string(startSym) + ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrScheduleDlHarq(
                            std::vector<DIET>(dcis.begin(), dcis.begin() + 4),
                            startSym,
                            numSym,
                            "Reshape with scheduler: Beam   0, startSym " +
                                std::to_string(startSym) + ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrScheduleDlHarq(
                            std::vector<DIET>(dcis.begin() + 4, dcis.begin() + 6),
                            startSym,
                            numSym,
                            "Reshape with scheduler: Beam 0+1, startSym " +
                                std::to_string(startSym) + ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrScheduleDlHarq(
                            std::vector<DIET>(dcis.begin() + 3, dcis.begin() + 7),
                            startSym,
                            numSym,
                            "Reshape with scheduler: Beam 0+1, startSym " +
                                std::to_string(startSym) + ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
            AddTestCase(new NrTestMacSchedulerHarqRrScheduleDlHarq(
                            std::vector<DIET>(dcis.begin() + 2, dcis.begin() + 8),
                            startSym,
                            numSym,
                            "Reshape with scheduler: Beam 0+1, startSym " +
                                std::to_string(startSym) + ", numSym " + std::to_string(numSym)),
                        Duration::QUICK);
        }
    }
};

static NrTestSchedHarqSuite nrSchedHarqTestSuite; //!< NR HARQ scheduler test suite

} // namespace ns3
