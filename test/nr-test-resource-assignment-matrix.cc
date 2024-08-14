// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/resource-assignment-matrix.h"
#include "ns3/test.h"

#include <algorithm>

/**
 * @file nr-test-resource-assignment-matrix.cc
 * @ingroup test
 *
 * @brief The test creates resource assignment matrices to check its behaviour.
 */
namespace ns3
{

/**
 * @brief TestCase for the resource assignment matrix
 */
class NrResourceAssignmentMatrixTestCase : public TestCase
{
  public:
    /**
     * @brief Create NrPatternTestCase
     * @param name Name of the test
     */
    NrResourceAssignmentMatrixTestCase(std::vector<bool> notchingMask, uint8_t symbols, bool ofdma)
        : TestCase("NrResourceAssignmentMatrixTestCase"),
          m_notchingMask(notchingMask),
          m_symbols(symbols),
          m_ofdma(ofdma)
    {
    }

  private:
    void DoRun() override;
    std::vector<bool> m_notchingMask;
    uint8_t m_symbols;
    bool m_ofdma;
};

void
NrResourceAssignmentMatrixTestCase::DoRun()
{
    auto rm = ResourceAssignmentMatrix(m_notchingMask, m_symbols);

    size_t numAssignableRbgs = std::count(m_notchingMask.begin(), m_notchingMask.end(), true);
    size_t numUnassignableResources = (m_notchingMask.size() - numAssignableRbgs) * m_symbols;

    NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                          0 + numUnassignableResources,
                          "Number of assigned resources is incorrect");
    NS_TEST_EXPECT_MSG_EQ(rm.GetFreeResourcesTotal(),
                          3 * numAssignableRbgs,
                          "Number of free resources is incorrect");

    if (m_ofdma)
    {
        if (m_notchingMask == std::vector<bool>{false, false, false})
        {
            // First test OFDMA allocation
            rm.AssignBeamIdToSymbols({1, 0}, 0);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA,
                                              0,
                                              0,
                                              0);
            NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                                  1,
                                  "Number of assigned resources is incorrect");
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::UL_DATA,
                                              1,
                                              1,
                                              0);
            NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                                  2,
                                  "Number of assigned resources is incorrect");
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::HARQ,
                                              1,
                                              2,
                                              0);
            NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                                  3,
                                  "Number of assigned resources is incorrect");

            rm.AssignBeamIdToSymbols({1, 0}, 1);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::PBSCH_DMRS,
                                              1,
                                              0,
                                              1);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::PDSCH_DMRS,
                                              1,
                                              1,
                                              1);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::PUSCH_DMRS,
                                              1,
                                              2,
                                              1);

            rm.AssignBeamIdToSymbols({2, 0}, 2);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::CSI_RS,
                                              1,
                                              0,
                                              2);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::PTRS,
                                              1,
                                              1,
                                              2);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::TRS, 1, 2, 2);

            // Should assert due to duplicate resource assignment
            // rm.AssignOfdmaRbgDuringSymbolToUe(1, 0, 0);

            // Should assert due to out-of-bounds checks
            // rm.AssignBeamIdToSymbols(1, -1);
            // rm.AssignBeamIdToSymbols(1, 3);
            // rm.AssignOfdmaRbgDuringSymbolToUe(2, 3, 0);
            // rm.AssignOfdmaRbgDuringSymbolToUe(2, -1, 0);
            // rm.AssignOfdmaRbgDuringSymbolToUe(2, 0, 3);
            // rm.AssignOfdmaRbgDuringSymbolToUe(2, 0, -1);

            NS_TEST_EXPECT_MSG_EQ(rm.GetNumAssignedResourcesToUe(0),
                                  1,
                                  "Number of assigned resources is incorrect");
            NS_TEST_EXPECT_MSG_EQ(rm.GetNumAssignedResourcesToUe(1),
                                  8,
                                  "Number of assigned resources is incorrect");
        }
        else if (m_notchingMask == std::vector<bool>{false, true, false})
        {
            rm.AssignBeamIdToSymbols({1, 0}, 0);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA,
                                              0,
                                              0,
                                              0);
            NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                                  1 + numUnassignableResources,
                                  "Number of assigned resources is incorrect");
            // rm.AssignOfdmaRbgDuringSymbolToUe(1, 1, 0); // Should assert due to notching
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA,
                                              1,
                                              2,
                                              0);
            NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                                  2 + numUnassignableResources,
                                  "Number of assigned resources is incorrect");

            rm.AssignBeamIdToSymbols({2, 0}, 1);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA,
                                              1,
                                              0,
                                              1);
            NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                                  3 + numUnassignableResources,
                                  "Number of assigned resources is incorrect");
            // rm.AssignOfdmaRbgDuringSymbolToUe(1, 1, 1); // Should assert due to notching
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA,
                                              1,
                                              2,
                                              1);
            NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                                  4 + numUnassignableResources,
                                  "Number of assigned resources is incorrect");

            rm.AssignBeamIdToSymbols({3, 0}, 2);
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA,
                                              1,
                                              0,
                                              2);
            // rm.AssignOfdmaRbgDuringSymbolToUe(1, 1, 2); // Should assert due to notching
            rm.AssignOfdmaRbgDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA,
                                              1,
                                              2,
                                              2);

            NS_TEST_EXPECT_MSG_EQ(rm.GetNumAssignedResourcesToUe(0),
                                  1,
                                  "Number of assigned resources is incorrect");
            NS_TEST_EXPECT_MSG_EQ(rm.GetNumAssignedResourcesToUe(1),
                                  5,
                                  "Number of assigned resources is incorrect");

            NS_TEST_EXPECT_MSG_EQ(rm.GetNumAssignedResourcesToUe(0),
                                  rm.GetAssignedResourcesToUe(0).size(),
                                  "Mismatching number of assigned resources");
            NS_TEST_EXPECT_MSG_EQ(rm.GetNumAssignedResourcesToUe(1),
                                  rm.GetAssignedResourcesToUe(1).size(),
                                  "Mismatching number of assigned resources");
        }
    }
    else
    {
        // Then test TDMA allocation
        rm.AssignTdmaChannelDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA, 0, 0);
        NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                              numAssignableRbgs + numUnassignableResources,
                              "Number of assigned resources is incorrect");
        NS_TEST_EXPECT_MSG_EQ(rm.GetFreeResourcesTotal(),
                              2 * numAssignableRbgs,
                              "Number of free resources is incorrect");

        rm.AssignTdmaChannelDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA, 2, 1);
        NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                              2 * numAssignableRbgs + numUnassignableResources,
                              "Number of assigned resources is incorrect");
        NS_TEST_EXPECT_MSG_EQ(rm.GetFreeResourcesTotal(),
                              numAssignableRbgs,
                              "Number of assigned free is incorrect");

        rm.AssignTdmaChannelDuringSymbolToUe(ResourceAssignmentMatrix::ResourceType::DL_DATA, 1, 2);
        NS_TEST_EXPECT_MSG_EQ(rm.GetAssignedResourcesTotal(),
                              3 * numAssignableRbgs + numUnassignableResources,
                              "Number of assigned resources is incorrect");
        NS_TEST_EXPECT_MSG_EQ(rm.GetFreeResourcesTotal(),
                              0,
                              "Number of free resources is incorrect");

        NS_TEST_EXPECT_MSG_EQ(rm.GetNumAssignedResourcesToUe(0),
                              rm.GetAssignedResourcesToUe(0).size(),
                              "Mismatching number of assigned resources");
        NS_TEST_EXPECT_MSG_EQ(rm.GetNumAssignedResourcesToUe(1),
                              rm.GetAssignedResourcesToUe(1).size(),
                              "Mismatching number of assigned resources");
    }
}

class NrResourceAssignmentMatrixTestSuite : public TestSuite
{
  public:
    NrResourceAssignmentMatrixTestSuite()
        : TestSuite("nr-resource-assignment-matrix", Type::UNIT)
    {
        for (auto ofdma : {true, false})
        {
            AddTestCase(new NrResourceAssignmentMatrixTestCase({true, true, true}, 3, ofdma),
                        Duration::QUICK);
            AddTestCase(new NrResourceAssignmentMatrixTestCase({true, false, true}, 3, ofdma),
                        Duration::QUICK);
        }
    }
};

static NrResourceAssignmentMatrixTestSuite g_nrResourceAssignmentMatrixTestSuite;

//!< Pattern test suite

} // namespace ns3
