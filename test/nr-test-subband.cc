// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/basic-data-calculators.h"
#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/nr-amc.h"
#include "ns3/nr-channel-helper.h"
#include "ns3/nr-eps-bearer.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-pm-search-full.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/nr-ue-net-device.h"
#include "ns3/nr-ue-phy.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/test.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"

#include <iostream>
#include <map>
#include <numeric>
#include <string>

/**
 * @file nr-test-subband.cc
 * @ingroup test
 *
 * @brief Unit testing for sub-band downsampling and upsampling.
 */
namespace ns3
{

std::string
GetSubbandTestCaseName(const std::string& testType,
                       size_t matrixSize,
                       size_t subbandSize,
                       const std::string& technique,
                       bool enforce)
{
    std::stringstream ss;
    ss << testType << ", mat=" << matrixSize << "x" << matrixSize << ", sb=" << subbandSize
       << ", enforceSbSize=" << (enforce ? "yes" : "no") << ", downsamplingTechnique=" << technique;
    return ss.str();
}

class SubbandDimensionsTestCase : public TestCase
{
  public:
    SubbandDimensionsTestCase(NrIntfNormChanMat matrix,
                              size_t sbSize,
                              const std::string& technique,
                              bool enforce)
        : TestCase(GetSubbandTestCaseName("Check dimensions",
                                          matrix.GetNumPages(),
                                          sbSize,
                                          technique,
                                          enforce)),
          m_matrix(matrix),
          m_subbandSize(sbSize),
          m_technique(technique),
          m_enforce(enforce)
    {
    }

  private:
    void DoRun() override;

    NrIntfNormChanMat m_matrix;
    size_t m_subbandSize;
    std::string m_technique;
    bool m_enforce;
};

void
SubbandDimensionsTestCase::DoRun()
{
    size_t prbs = m_matrix.GetNumPages();

    // If sub-band size is bigger than bandwidth, skip nonsensical case
    if (m_subbandSize > prbs)
    {
        return;
    }

    // If enforcing 3GPP sub-band sizes, skip unsupported cases
    if (m_enforce)
    {
        switch (m_subbandSize)
        {
        case 1:
            if (prbs >= 24)
            {
                return;
            }
            break;
        case 4:
            if (prbs < 24 || prbs > 72)
            {
                return;
            }
            break;
        case 8:
            if (prbs < 73 || prbs > 144)
            {
                return;
            }
            break;
        case 16:
            if (prbs < 145 || prbs > 275)
            {
                return;
            }
            break;
        case 32:
            if (prbs < 275)
            {
                return;
            }
            break;
        default:
            // Early exit for non-compliant 3GPP sub-band sizes
            return;
        }
    }

    auto pm = CreateObject<NrPmSearchFull>();
    pm->SetAttribute("SubbandSize", UintegerValue(m_subbandSize));
    pm->SetAttribute("EnforceSubbandSize", BooleanValue(m_enforce));
    pm->SetAttribute("DownsamplingTechnique", StringValue(m_technique));

    // Downsample and check if dimensions match
    // Contents are checked in SubbandContentsTestCase
    auto down = pm->SubbandDownsampling(m_matrix);
    NS_TEST_EXPECT_MSG_EQ(m_matrix.GetNumCols(), down.GetNumCols(), "Cols must match");
    NS_TEST_EXPECT_MSG_EQ(m_matrix.GetNumRows(), down.GetNumRows(), "Rows must match");

    size_t expectedSubbands =
        prbs / m_subbandSize + ((prbs % m_subbandSize && prbs > m_subbandSize) ? 1 : 0);
    NS_TEST_EXPECT_MSG_EQ(expectedSubbands,
                          down.GetNumPages(),
                          "Pages must match after downsampling");

    // Downsample and check if dimensions match
    auto up = pm->SubbandUpsampling(down, prbs);
    NS_TEST_EXPECT_MSG_EQ(m_matrix.GetNumCols(), up.GetNumCols(), "Cols must match");
    NS_TEST_EXPECT_MSG_EQ(m_matrix.GetNumRows(), up.GetNumRows(), "Rows must match");
    NS_TEST_EXPECT_MSG_EQ(prbs, up.GetNumPages(), "Pages must match after upsampling");
}

class SubbandContentsTestCase : public TestCase
{
  public:
    SubbandContentsTestCase(Ptr<NrIntfNormChanMat> input,
                            Ptr<NrIntfNormChanMat> reference,
                            size_t sbSize,
                            const std::string& technique,
                            bool enforce)
        : TestCase(GetSubbandTestCaseName("Check Contents",
                                          input->GetNumPages(),
                                          sbSize,
                                          technique,
                                          enforce)),
          m_input(input),
          m_reference(reference),
          m_sbSize(sbSize),
          m_technique(technique),
          m_enforce(enforce)
    {
    }

  private:
    void DoRun() override;

    Ptr<NrIntfNormChanMat> m_input;
    Ptr<NrIntfNormChanMat> m_reference;
    size_t m_sbSize;
    std::string m_technique;
    bool m_enforce;
};

void
SubbandContentsTestCase::DoRun()
{
    size_t prbs = m_input->GetNumPages();

    auto pm = CreateObject<NrPmSearchFull>();
    pm->SetAttribute("SubbandSize", UintegerValue(m_sbSize));
    pm->SetAttribute("EnforceSubbandSize", BooleanValue(m_enforce));
    pm->SetAttribute("DownsamplingTechnique", StringValue(m_technique));

    // Downsample first
    auto down = pm->SubbandDownsampling(*m_input);
    NS_TEST_EXPECT_MSG_EQ(m_input->GetNumCols(), down.GetNumCols(), "Cols must match");
    NS_TEST_EXPECT_MSG_EQ(m_input->GetNumRows(), down.GetNumRows(), "Rows must match");

    size_t expectedSubbands = prbs / m_sbSize + ((prbs % m_sbSize > 0) && (prbs > m_sbSize));
    NS_TEST_EXPECT_MSG_EQ(expectedSubbands,
                          down.GetNumPages(),
                          "Pages must match after downsampling");

    // Then upsample and check if contents match
    auto up = pm->SubbandUpsampling(down, prbs);
    NS_TEST_EXPECT_MSG_EQ(m_reference->GetNumCols(), up.GetNumCols(), "Cols must match");
    NS_TEST_EXPECT_MSG_EQ(m_reference->GetNumRows(), up.GetNumRows(), "Rows must match");
    NS_TEST_EXPECT_MSG_EQ(prbs, up.GetNumPages(), "Pages must match after upsampling");

    if (m_technique == "RandomPRB")
    {
        // RandomPRB downsampling must be tested separately not for equality with a reference
        // matrix, but range of values from sub-bands
        size_t lastFullSbStart = (prbs / m_sbSize) * m_sbSize;
        for (size_t i = 0; i < up.GetNumPages(); i++)
        {
            auto value = up.Elem(0, 0, i).real();
            auto sb = i / m_sbSize;
            auto lowerBound = sb * m_sbSize;
            auto upperBound = (sb + 1) * m_sbSize; // For full sub-bands
            if (i >= lastFullSbStart)
            {
                upperBound = sb * m_sbSize + prbs % m_sbSize; // For incomplete sub-bands
            }
            NS_TEST_ASSERT_MSG_GT_OR_EQ(value, lowerBound, "Value must be >= lower PRB");
            NS_TEST_ASSERT_MSG_LT(value, upperBound, "Value must be < upper PRB");
        }
        return;
    }

    NS_TEST_EXPECT_MSG_EQ(up, *m_reference, "Upsampled matrix must match reference");
}

std::tuple<size_t, size_t, size_t>
GetNumSbsAndLastSbInfo(size_t prbs, size_t sbSize)
{
    size_t lastSbSize = prbs % sbSize;
    size_t numSbs = prbs / sbSize;
    size_t lastSbStartingPrb = prbs - sbSize;
    if (lastSbSize == 0)
    {
        lastSbSize = sbSize;
    }
    else
    {
        numSbs += 1;
        lastSbStartingPrb = prbs - lastSbSize;
    }
    return std::make_tuple(numSbs, lastSbSize, lastSbStartingPrb);
}

class TestSubband : public TestSuite
{
  public:
    TestSubband()
        : TestSuite("nr-test-subband", Type::UNIT)
    {
        for (auto prbs : {1, 5, 10, 25, 32, 56, 114, 128, 225, 250, 256, 264, 300})
        {
            NrIntfNormChanMat mat(ComplexMatrixArray(10, 10, prbs));
            for (auto sbSize : {1, 2, 3, 4, 5, 7, 8, 16, 20, 31, 32})
            {
                for (bool enforce : {false, true})
                {
                    for (const auto* technique : {"FirstPRB", "RandomPRB", "AveragePRB"})
                    {
                        AddTestCase(new SubbandDimensionsTestCase(mat, sbSize, technique, enforce),
                                    Duration::QUICK);
                    }
                }
            }
        }

        // Now we test for contents of downsampling techniques
        for (size_t prbs : {128, 146})
        {
            auto input = Create<NrIntfNormChanMat>(ComplexMatrixArray(2, 2, prbs));
            // Each page will have all elements with same value (number of page)
            for (size_t i = 0; i < prbs; ++i)
            {
                std::fill_n(input->GetPagePtr(i), 4, i);
            }

            // Now we generate the reference matrix for each case
            for (size_t sbSize : {1, 4, 8, 16, 32})
            {
                // FirstPRB
                auto firstRef = Create<NrIntfNormChanMat>(ComplexMatrixArray(2, 2, prbs));
                for (size_t i = 0; i < prbs; ++i)
                {
                    std::fill_n(firstRef->GetPagePtr(i), 4, (i / sbSize) * sbSize);
                }

                AddTestCase(new SubbandContentsTestCase(input, firstRef, sbSize, "FirstPRB", false),
                            Duration::QUICK);

                // AveragePRB
                auto avgRef = Create<NrIntfNormChanMat>(ComplexMatrixArray(2, 2, prbs));
                auto [num, lastSize, lastStart] = GetNumSbsAndLastSbInfo(prbs, sbSize);
                for (size_t i = 0; i < prbs; i += sbSize)
                {
                    auto* ptr = avgRef->GetPagePtr(i);
                    size_t len = (i != lastStart) ? sbSize : lastSize;
                    std::vector<std::complex<double>> vals(len);
                    std::iota(vals.begin(), vals.end(), i);
                    auto avg = std::reduce(vals.begin(), vals.end()) / double(vals.size());
                    std::fill_n(ptr, 4 * len, avg);
                }

                AddTestCase(new SubbandContentsTestCase(input, avgRef, sbSize, "AveragePRB", false),
                            Duration::QUICK);

                // RandomPRB
                // Different from previous, we pass an empty reference matrix
                // On the test, instead of ensuring equality, we check if
                // random PRBs are within the expected range per sub-band
                auto randomRef = Create<NrIntfNormChanMat>(ComplexMatrixArray(2, 2, prbs));
                AddTestCase(
                    new SubbandContentsTestCase(input, randomRef, sbSize, "RandomPRB", false),
                    Duration::QUICK);
            }
        }
    }
};

static TestSubband g_TestSubband;

} // namespace ns3
