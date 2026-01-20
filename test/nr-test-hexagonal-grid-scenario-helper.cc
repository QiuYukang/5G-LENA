// Copyright (c) 2026 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/fast-fading-constant-position-mobility-model.h"
#include "ns3/hexagonal-grid-scenario-helper.h"
#include "ns3/resource-assignment-matrix.h"
#include "ns3/test.h"
#include "ns3/vector.h"

/**
 * @file nr-test-hexagonal-grid-scenario-helper.cc
 * @ingroup test
 *
 * @brief The test checks behaviour of hexagonal grid deployment helper.
 */
namespace ns3
{

/**
 * @brief TestCase for the hexagonal grid deployment
 */
class NrHexagonalGridDeploymentTestCase : public TestCase
{
  public:
    /**
     * @brief Create NrPatternTestCase
     */
    NrHexagonalGridDeploymentTestCase()
        : TestCase("NrHexagonalGridDeploymentTestCase")
    {
    }

  private:
    void DoRun() override;
};

void
NrHexagonalGridDeploymentTestCase::DoRun()
{
    auto getHelper = []() -> HexagonalGridScenarioHelper {
        ScenarioParameters scenarioParameters;
        scenarioParameters.m_isd = 200;
        scenarioParameters.m_bsHeight = 25;
        scenarioParameters.m_utHeight = 0;
        scenarioParameters.m_minBsUtDistance = 20;
        scenarioParameters.m_sectorization = HexagonalGridScenarioHelper::SINGLE;

        HexagonalGridScenarioHelper helper;
        helper.SetScenarioParameters(scenarioParameters);
        helper.SetNumRings(0);
        helper.SetUtNumber(10);
        return helper;
    };
    double indoorUeFraction = 0.5;

    // Set indoor and outdoor nodes with 0 speed should result in them being configured with
    // ConstantPositionMobilityModel
    Vector3D indoorSpeed = {0, 0, 0};
    Vector3D outdoorSpeed = {0, 0, 0};
    HexagonalGridScenarioHelper helper = getHelper();
    helper.CreateScenarioWithMobility(indoorSpeed, outdoorSpeed, indoorUeFraction);
    auto ue0Mm = helper.GetUserTerminals().Get(0)->GetObject<MobilityModel>();
    auto ue5Mm = helper.GetUserTerminals().Get(5)->GetObject<MobilityModel>();
    auto ue0MmTypeId = ue0Mm->GetInstanceTypeId();
    auto ue5MmTypeId = ue5Mm->GetInstanceTypeId();
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ue5MmTypeId,
                          "User terminals should have the same mobility model type");
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ConstantPositionMobilityModel::GetTypeId(),
                          "User terminal 0 should have the ConstantPositionMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue5MmTypeId,
                          ConstantPositionMobilityModel::GetTypeId(),
                          "User terminal 5 should have the ConstantPositionMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue0Mm->GetVelocity().GetLength(),
                          0,
                          "User terminal 0 should have zero velocity");
    NS_TEST_ASSERT_MSG_EQ(ue5Mm->GetVelocity().GetLength(),
                          0,
                          "User terminal 5 should have zero velocity");

    // Set indoor and outdoor nodes with 0 and 10m/s speed should result in indoor being configured
    // with ConstantPositionMobilityModel and outdoor as ConstantVelocityPositionModel
    indoorSpeed = {0, 0, 0};
    outdoorSpeed = {10, 0, 0};
    helper = getHelper();
    helper.CreateScenarioWithMobility(indoorSpeed, outdoorSpeed, indoorUeFraction);
    ue0Mm = helper.GetUserTerminals().Get(0)->GetObject<MobilityModel>();
    ue5Mm = helper.GetUserTerminals().Get(5)->GetObject<MobilityModel>();
    ue0MmTypeId = ue0Mm->GetInstanceTypeId();
    ue5MmTypeId = ue5Mm->GetInstanceTypeId();
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ue5MmTypeId,
                          "User terminals should have the same mobility model type");
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ConstantVelocityMobilityModel::GetTypeId(),
                          "User terminal 0 should have the ConstantVelocityMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue5MmTypeId,
                          ConstantVelocityMobilityModel::GetTypeId(),
                          "User terminal 5 should have the ConstantVelocityMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue0Mm->GetVelocity().GetLength(),
                          0,
                          "User terminal 0 should have zero velocity");
    NS_TEST_ASSERT_MSG_EQ(ue5Mm->GetVelocity().GetLength(),
                          10,
                          "User terminal 5 should have non-zero velocity");

    // Set indoor and outdoor nodes with 10 and 0m/s speed should result in outdoor being configured
    // with ConstantPositionMobilityModel and indoor as ConstantVelocityPositionModel
    indoorSpeed = {10, 0, 0};
    outdoorSpeed = {0, 0, 0};
    helper = getHelper();
    helper.CreateScenarioWithMobility(indoorSpeed, outdoorSpeed, indoorUeFraction);
    ue0Mm = helper.GetUserTerminals().Get(0)->GetObject<MobilityModel>();
    ue5Mm = helper.GetUserTerminals().Get(5)->GetObject<MobilityModel>();
    ue0MmTypeId = ue0Mm->GetInstanceTypeId();
    ue5MmTypeId = ue5Mm->GetInstanceTypeId();
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ue5MmTypeId,
                          "User terminals should have the same mobility model type");
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ConstantVelocityMobilityModel::GetTypeId(),
                          "User terminal 0 should have the ConstantVelocityMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue5MmTypeId,
                          ConstantVelocityMobilityModel::GetTypeId(),
                          "User terminal 5 should have the ConstantVelocityMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue0Mm->GetVelocity().GetLength(),
                          10,
                          "User terminal 0 should have non-zero velocity");
    NS_TEST_ASSERT_MSG_EQ(ue5Mm->GetVelocity().GetLength(),
                          0,
                          "User terminal 5 should have zero velocity");

    // Set indoor and outdoor nodes with 10 and 10m/s speed should result in outdoor and indoor
    // being configured as ConstantVelocityPositionModel
    indoorSpeed = {10, 0, 0};
    outdoorSpeed = {10, 0, 0};
    helper = getHelper();
    helper.CreateScenarioWithMobility(indoorSpeed, outdoorSpeed, indoorUeFraction);
    ue0Mm = helper.GetUserTerminals().Get(0)->GetObject<MobilityModel>();
    ue5Mm = helper.GetUserTerminals().Get(5)->GetObject<MobilityModel>();
    ue0MmTypeId = ue0Mm->GetInstanceTypeId();
    ue5MmTypeId = ue5Mm->GetInstanceTypeId();
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ue5MmTypeId,
                          "User terminals should have the same mobility model type");
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ConstantVelocityMobilityModel::GetTypeId(),
                          "User terminal 0 should have the ConstantVelocityMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue5MmTypeId,
                          ConstantVelocityMobilityModel::GetTypeId(),
                          "User terminal 5 should have the ConstantVelocityMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue0Mm->GetVelocity().GetLength(),
                          10,
                          "User terminal 0 should have non-zero velocity");
    NS_TEST_ASSERT_MSG_EQ(ue5Mm->GetVelocity().GetLength(),
                          10,
                          "User terminal 5 should have non-zero velocity");

    // Set indoor and outdoor nodes with 0 and 10m/s speed should result in outdoor and indoor
    // being configured with ns3::FastFadingConstantPositionMobilityModel
    indoorSpeed = {0, 0, 0};
    outdoorSpeed = {10, 0, 0};
    helper = getHelper();
    helper.CreateScenarioWithMobility(indoorSpeed,
                                      outdoorSpeed,
                                      indoorUeFraction,
                                      "ns3::FastFadingConstantPositionMobilityModel");
    ue0Mm = helper.GetUserTerminals().Get(0)->GetObject<MobilityModel>();
    ue5Mm = helper.GetUserTerminals().Get(5)->GetObject<MobilityModel>();
    ue0MmTypeId = ue0Mm->GetInstanceTypeId();
    ue5MmTypeId = ue5Mm->GetInstanceTypeId();
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ue5MmTypeId,
                          "User terminals should have the same mobility model type");
    NS_TEST_ASSERT_MSG_EQ(
        ue0MmTypeId,
        FastFadingConstantPositionMobilityModel::GetTypeId(),
        "User terminal 0 should have the FastFadingConstantPositionMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(
        ue5MmTypeId,
        FastFadingConstantPositionMobilityModel::GetTypeId(),
        "User terminal 5 should have the FastFadingConstantPositionMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue0Mm->GetVelocity().GetLength(),
                          0,
                          "User terminal 0 should have non-zero velocity");
    NS_TEST_ASSERT_MSG_EQ(ue5Mm->GetVelocity().GetLength(),
                          10,
                          "User terminal 5 should have non-zero velocity");

    // Set indoor and outdoor nodes with 10 and 10m/s speed should result in outdoor and indoor
    // being configured with ns3::FastFadingConstantPositionMobilityModel
    indoorSpeed = {10, 0, 0};
    outdoorSpeed = {10, 0, 0};
    helper = getHelper();
    helper.CreateScenarioWithMobility(indoorSpeed,
                                      outdoorSpeed,
                                      indoorUeFraction,
                                      "ns3::FastFadingConstantPositionMobilityModel");
    ue0Mm = helper.GetUserTerminals().Get(0)->GetObject<MobilityModel>();
    ue5Mm = helper.GetUserTerminals().Get(5)->GetObject<MobilityModel>();
    ue0MmTypeId = ue0Mm->GetInstanceTypeId();
    ue5MmTypeId = ue5Mm->GetInstanceTypeId();
    NS_TEST_ASSERT_MSG_EQ(ue0MmTypeId,
                          ue5MmTypeId,
                          "User terminals should have the same mobility model type");
    NS_TEST_ASSERT_MSG_EQ(
        ue0MmTypeId,
        FastFadingConstantPositionMobilityModel::GetTypeId(),
        "User terminal 0 should have the FastFadingConstantPositionMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(
        ue5MmTypeId,
        FastFadingConstantPositionMobilityModel::GetTypeId(),
        "User terminal 5 should have the FastFadingConstantPositionMobilityModel type");
    NS_TEST_ASSERT_MSG_EQ(ue0Mm->GetVelocity().GetLength(),
                          10,
                          "User terminal 0 should have non-zero velocity");
    NS_TEST_ASSERT_MSG_EQ(ue5Mm->GetVelocity().GetLength(),
                          10,
                          "User terminal 5 should have non-zero velocity");
}

class NrHexagonalGridDeploymentTestSuite : public TestSuite
{
  public:
    NrHexagonalGridDeploymentTestSuite()
        : TestSuite("nr-hexagonal-deployment", Type::UNIT)
    {
        AddTestCase(new NrHexagonalGridDeploymentTestCase(), Duration::QUICK);
    }
};

static NrHexagonalGridDeploymentTestSuite g_nrHexagonalGridDeploymentTestSuite;

//!< Pattern test suite

} // namespace ns3
