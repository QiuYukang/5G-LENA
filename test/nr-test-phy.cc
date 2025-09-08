// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/nr-phy.h"
#include "ns3/test.h"

#include <iomanip>

/**
 * @ingroup nr-test
 * @file nr-test-phy.h
 *
 * @brief This test suite contains tests for NrPhy, NrUePhy and NrGnbPhy.
 */

namespace ns3
{
/**
 * @brief Test case for evaluating ARFCN (Absolute Radio Frequency Channel Number) conversions
 *        with respect to specified input frequency, expected ARFCN, and expected output frequency.
 *
 * The test expects an input frequency value, along with expected ARFCN and
 * output frequency values as references against which the computed results are validated.
 */
class NrArfcnTestCase : public TestCase
{
  public:
    /** Constructor. */
    NrArfcnTestCase(double inputFreq, uint32_t outputArfcn, double outputFreq);

  private:
    /**
     * @brief Run test case
     */
    void DoRun() override;
    double m_inputFreq;     //!< Reference input value
    uint32_t m_outputArfcn; //!< Reference ARFCN value
    double m_outputFreq;    //!< Reference output frequency value (may be different, because ARFCN
                            //!< cannot represent all frequencies)
};

NrArfcnTestCase::NrArfcnTestCase(double inputFreq, uint32_t outputArfcn, double outputFreq)
    : TestCase("Arfcn test: inputFreq " + std::to_string(inputFreq) + ", outputArfcn " +
               std::to_string(outputArfcn) + ", outputFreq " + std::to_string(outputFreq)),
      m_inputFreq(inputFreq),
      m_outputArfcn(outputArfcn),
      m_outputFreq(outputFreq)
{
}

void
NrArfcnTestCase::DoRun()
{
    uint32_t convertedFreqToArfcn = NrPhy::FrequencyHzToArfcn(m_inputFreq);
    double convertedFreqFromArfcn = NrPhy::ArfcnToFrequencyHz(convertedFreqToArfcn);
    uint32_t convertedFreqToArfcnFromArfcn = NrPhy::FrequencyHzToArfcn(convertedFreqFromArfcn);
    NS_TEST_EXPECT_MSG_EQ(convertedFreqToArfcn,
                          m_outputArfcn,
                          "Expected matching ARFCN " << std::fixed << m_outputArfcn << ", got "
                                                     << convertedFreqToArfcn);
    NS_TEST_EXPECT_MSG_EQ(convertedFreqFromArfcn,
                          m_outputFreq,
                          "Expected matching frequency " << std::fixed << m_outputFreq << ", got "
                                                         << convertedFreqFromArfcn);
    NS_TEST_EXPECT_MSG_EQ(convertedFreqToArfcnFromArfcn,
                          m_outputArfcn,
                          "Expected matching ARFCN " << std::fixed << m_outputArfcn << ", got "
                                                     << convertedFreqToArfcnFromArfcn);
}

/**
 * @ingroup test
 * The test suite that runs different test cases to test NrPhy.
 */
class NrPhyTestSuite : public TestSuite
{
  public:
    /** Constructor. */
    NrPhyTestSuite();
};

NrPhyTestSuite::NrPhyTestSuite()
    : TestSuite("nr-test-phy")
{
    // clang-format off
    std::vector<std::tuple<double, uint32_t, double>> arfcnParams {
      {         1e9,   200000,   1000000000},
      {         2e9,   400000,   2000000000},
      {         3e9,   600000,   3000000000},
      {         4e9,   666666,   3999990000},
      {         5e9,   733333,   4999995000},
      {         6e9,   800000,   6000000000},
      {         7e9,   866666,   6999990000},
      {         9e9,  1000000,   9000000000},
      {        10e9,  1066666,   9999990000},
      {       100e9,  3279166, 100000000000},
      {114250000000, 62654166, 114250000000},
    };
    // clang-format on
    for (auto& [inputFreq, outputArfcn, outputFreq] : arfcnParams)
    {
        AddTestCase(new NrArfcnTestCase(inputFreq, outputArfcn, outputFreq), Duration::QUICK);
    }
}

// Allocate an instance of this TestSuite
static NrPhyTestSuite g_nrPhyTestSuite;

} // namespace ns3
