// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_TEST_CSI_H
#define NR_TEST_CSI_H

#include "ns3/flow-monitor.h"
#include "ns3/nr-mac-scheduler-ue-info.h"
#include "ns3/nr-phy-mac-common.h"
#include "ns3/nr-spectrum-phy.h"
#include "ns3/onoff-application.h"
#include "ns3/test.h"

/**
 * @ingroup nr-test
 * @file nr-test-csi.h
 *
 * @brief This test creates a scenario with up to two gnb/ues, to test if the CSI
 * and sub-band aware scheduler works correctly, avoiding strong narrowband interference
 */

namespace ns3
{

class MobilityModel;

class NrCsiTestCase : public TestCase
{
  public:
    struct NrCsiTestCaseParams
    {
        TestCase::Duration m_duration; //!< duration style of test case
        double m_interfDistance; //!< the distance in meters between the original node pair, and the
        // interfering node pair
        double m_ueGnbDistance; //!< distance between the gNB and its corresponding UE
        std::vector<std::vector<bool>>
            m_interfPattern;       //!< RBGs affected by each of the interferer pairs
        bool m_subbandCqiClamping; //!< Enable 3gpp sub-band CQI clamping (2 bits worth of info on
                                   //!< top of wide-band CQI)
        double m_expectedThrUe0;   //!< Expected throughput of observed UE0
        double m_interfCovMatAlpha;
        uint32_t m_rlcBufferSize;
        bool m_enableCsiFeedback;
        uint32_t m_csiFeedbackFlags;
        NrMacSchedulerUeInfo::McsCsiSource m_mcsCsiSource;
    };

    /** Constructor. */
    NrCsiTestCase(NrCsiTestCaseParams params);

    /** Destructor. */
    ~NrCsiTestCase() override;

    // Definitions of machine states
    enum MachineState
    {
        WAITING_FOR_INTERFERER_TO_START = 0b00000001,
        WAITING_FOR_UP_TO_DATE_INTERFERED_CQI = 0b00000010,
        LINK_ADAPTED_TO_INTERFERENCE = 0b00000100,
        WAITING_FOR_INTERFERER_TO_STOP = 0b00001000,
        WAITING_FOR_UP_TO_DATE_NON_INTERFERED_CQI = 0b00010000,
        LINK_ADAPTED_TO_NO_INTERFERENCE = 0b00100000
    };

    struct CsiState
    {
        uint8_t widebandCqi;
        uint8_t lowbandCqi;
        uint8_t highbandCqi;
        uint8_t rank;
        friend bool operator==(const CsiState& a, const CsiState& b);
        friend bool operator<(const CsiState& a, const CsiState& b);
    };

  private:
    /**
     * @brief Run test case
     */
    void DoRun() override;

    struct NrCsiTestCaseParams m_params; //!< Parameters to configure test case

    // Functions to collect traces
    void UeReception(RxPacketTraceParams params);
    void CsiFeedbackReceived(uint16_t cellId,
                             uint16_t bwpId,
                             const std::shared_ptr<NrMacSchedulerUeInfo>& ueInfo);
    void LogThroughputUe0(Ptr<FlowMonitor> monitor, Time udpAppStartTime);
    static void LogApplicationStateTrampoline(NrCsiTestCase* testCase,
                                              Ptr<OnOffApplication> app,
                                              bool beforeState,
                                              bool afterState);
    void LogApplicationState(Ptr<OnOffApplication> app, bool beforeState, bool afterState);

    double GetSlidingWindowErrorRate();

    /**
     * @brief This is the function that actually checks if the test is working correctly by using a
     * FSM
     */
    void StateMachineStep(MachineState nextState);
    MachineState m_machineState =
        MachineState::WAITING_FOR_INTERFERER_TO_START; //!< Maintains the state of the checker
                                                       //!< machine state
    std::deque<double> m_errorRateHistory;
    // Definitions to store with traces
    enum class InterferenceType
    {
        NO_INTERFERENCE,
        WIDEBAND_INTERFERENCE,
        LOWBAND_INTERFERENCE,
        HIGHBAND_INTERFERENCE
    };

    struct InterfererState
    {
        std::vector<std::pair<Time, bool>> timestampAndState;
        InterferenceType type;
    };

    const InterfererState GetInterfererState() const
    {
        for (auto& interferer : m_interferers)
        {
            if (interferer.first.empty())
            {
                continue;
            }
            return interferer.second;
        }
        return {};
    }

    // Variables to store traces
    std::map<Ptr<OnOffApplication>, std::string> m_interfAppToString;
    std::unordered_map<std::string, InterfererState> m_interferers;
    std::deque<std::pair<Time, CsiState>> m_csiSlidingWindow;
    std::deque<std::pair<Time, bool>> m_tbErrorSlidingWindow;
    std::string m_description;
};

inline std::ostream&
operator<<(std::ostream& os, NrCsiTestCase::MachineState v)
{
    switch (v)
    {
    case NrCsiTestCase::MachineState::WAITING_FOR_INTERFERER_TO_START:
        os << "WAITING_FOR_INTERFERER_TO_START";
        break;
    case NrCsiTestCase::MachineState::WAITING_FOR_UP_TO_DATE_INTERFERED_CQI:
        os << "WAITING_FOR_UP_TO_DATE_INTERFERED_CQI";
        break;
    case NrCsiTestCase::MachineState::LINK_ADAPTED_TO_INTERFERENCE:
        os << "LINK_ADAPTED_TO_INTERFERENCE";
        break;
    case NrCsiTestCase::MachineState::WAITING_FOR_INTERFERER_TO_STOP:
        os << "WAITING_FOR_INTERFERER_TO_STOP";
        break;
    case NrCsiTestCase::MachineState::WAITING_FOR_UP_TO_DATE_NON_INTERFERED_CQI:
        os << "WAITING_FOR_UP_TO_DATE_NON_INTERFERED_CQI";
        break;
    case NrCsiTestCase::MachineState::LINK_ADAPTED_TO_NO_INTERFERENCE:
        os << "LINK_ADAPTED_TO_NO_INTERFERENCE";
        break;
    }
    return os;
}

inline bool
operator==(const NrCsiTestCase::CsiState& a, const NrCsiTestCase::CsiState& b)
{
    return a.rank == b.rank && a.highbandCqi == b.highbandCqi && a.lowbandCqi == b.lowbandCqi;
}

inline bool
operator<(const NrCsiTestCase::CsiState& a, const NrCsiTestCase::CsiState& b)
{
    return (a.rank < b.rank) || ((a.rank == b.rank) && (a.widebandCqi < b.widebandCqi));
}

/**
 * @ingroup test
 * The test suite that runs different test cases to test NrSpectrumPhy.
 */
class NrCsiTestSuite : public TestSuite
{
  public:
    /** Constructor. */
    NrCsiTestSuite();
};

} // namespace ns3

#endif // NR_TEST_CSI_H
