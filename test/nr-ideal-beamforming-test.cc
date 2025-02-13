// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/nr-channel-helper.h"
#include "ns3/nr-epc-helper.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-ue-net-device.h"
#include "ns3/nr-ue-phy.h"
#include "ns3/test.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/uinteger.h"

/**
 * @file nr-ideal-beamforming-test
 * @ingroup test
 *
 * @brief Tests ideal beamforming
 */
namespace ns3
{
class BeamformingTestCase : public TestCase
{
  public:
    BeamformingTestCase(std::string testName,
                        std::string beamformingName,
                        int columns,
                        int rows,
                        Vector3D coord,
                        double sector,
                        double elevation,
                        int oversampling = 1)
        : TestCase(testName),
          m_beamformingName(beamformingName),
          m_numAntennaColumns(columns),
          m_numAntennaRows(rows),
          m_coord(coord),
          m_expectedSector(sector),
          m_expectedElevation(elevation),
          m_oversampling(oversampling)
    {
    }

  private:
    void DoRun() override;
    std::string m_beamformingName;
    int m_numAntennaColumns = 1;
    int m_numAntennaRows = 1;
    Vector3D m_coord{0.0, 0.0, 0.0};
    double m_expectedSector = 0.0;
    double m_expectedElevation = 0.0;
    int m_oversampling = 1;
};

void
BeamformingTestCase::DoRun()
{
    // Put very short channel coherence period make sure we get the channel updated after every
    // movement
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(NanoSeconds(1)));

    NrHelper::AntennaParams apUe;
    NrHelper::AntennaParams apGnb;
    apUe.antennaElem = "ns3::ThreeGppAntennaModel";
    apUe.nAntCols = 8;
    apUe.nAntRows = 8;
    apUe.nHorizPorts = 1;
    apUe.nVertPorts = 1;
    apUe.isDualPolarized = false;
    apGnb.antennaElem = "ns3::ThreeGppAntennaModel";
    apGnb.nAntCols = m_numAntennaColumns;
    apGnb.nAntRows = m_numAntennaRows;
    apGnb.nHorizPorts = 1;
    apGnb.nVertPorts = 1;
    apGnb.isDualPolarized = false;
    double downtiltAngleGnb = 0;

    // The polarization slant angle in degrees in case of x-polarized
    double polSlantAngleGnb = 0.0;
    double polSlantAngleUe = 0.0;
    // The bearing angles in degrees
    double bearingAngleGnb = 0.0;
    double bearingAngleUe = 180.0;

    // Other simulation scenario parameters
    Time simTime = MilliSeconds(1);
    double centralFrequency = 3.5e9;
    double bandwidth = 10e6;
    double txPowerGnb = 23; // dBm
    double txPowerUe = 23;  // dBm
    std::string scheduler = "ns3::NrMacSchedulerTdmaRR";
    std::string beamformingMethod = "ns3::" + m_beamformingName;

    // convert angle values into radians
    apUe.bearingAngle = bearingAngleUe * (M_PI / 180);
    apUe.polSlantAngle = polSlantAngleUe * (M_PI / 180);
    apGnb.bearingAngle = bearingAngleGnb * (M_PI / 180);
    apGnb.polSlantAngle = polSlantAngleGnb * (M_PI / 180);

    NodeContainer gnbContainer;
    gnbContainer.Create(1);
    NodeContainer ueContainer;
    ueContainer.Create(1);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 25.0));
    positionAlloc->Add(Vector(100, 0.0, 1.5));
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(gnbContainer.Get(0));
    mobility.Install(ueContainer.Get(0));

    /**
     * Create the NR helpers that will be used to create and setup NR devices, spectrum, ...
     */
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);

    // Set the channel using the scenario, condition and channel model
    // then enable shadowing
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    channelHelper->ConfigureFactories("UMa", "LOS", "ThreeGpp");
    channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

    // Create and set the channel with the band
    CcBwpCreator ccBwpCreator;
    CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency, bandwidth, 1);
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    channelHelper->AssignChannelsToBands({band});
    /**
     * Configure NrHelper, prepare most of the parameters that will be used in the simulation.
     */
    nrHelper->SetSchedulerTypeId(TypeId::LookupByName(scheduler));
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(TypeId::LookupByName(beamformingMethod)));
    Config::SetDefault("ns3::CellScanBeamforming::OversamplingFactor",
                       UintegerValue(m_oversampling));
    nrHelper->SetupGnbAntennas(apGnb);
    nrHelper->SetGnbAntennaAttribute("DowntiltAngle", DoubleValue(downtiltAngleGnb * M_PI / 180.0));
    nrHelper->SetupUeAntennas(apUe);

    nrHelper->SetGnbPhyAttribute("Numerology", UintegerValue(0));
    nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(txPowerGnb));
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(txPowerUe));

    /**
     * Initialize channel and pathloss, plus other things inside band.
     */
    BandwidthPartInfoPtrVector allBwps;
    allBwps = CcBwpCreator::GetAllBwps({band});

    /**
     * Finally, create the gNB and the UE device.
     */
    NetDeviceContainer gnbNetDevNc = nrHelper->InstallGnbDevice(gnbContainer, allBwps);
    NetDeviceContainer ueNetDevNc = nrHelper->InstallUeDevice(ueContainer, allBwps);

    nrHelper->AttachToGnb(ueNetDevNc.Get(0), gnbNetDevNc.Get(0));

    Ptr<NrGnbNetDevice> gnbNetDev = DynamicCast<NrGnbNetDevice>(gnbNetDevNc.Get(0));
    Ptr<NrUeNetDevice> ueNetDev = DynamicCast<NrUeNetDevice>(ueNetDevNc.Get(0));

    /*
     * clang-format off
     *
     * We move the UE in the shape of a cube in front of the gNB,
     * so that we can check if the beam changes as expected
     *                       (10, 200, 50)     (100, 200, 50)
     *                /    /      xxxxxxxxxxxxx
     *               /   /       x|          xx
     *              /  /   /    x |         x x
     *             / /  /      x  |        x  x
     *        gNB  -----      xxxxxxxxxxxxx   x
     *      (0,0)  \ \  \     x   --------x---x (100, 200, 0)
     *              \  \   \  x  /        x   x
     *               \   \    x /         x  x
     *                \    \  x/          x x
     *                        xxxxxxxxxxxxx
     *                (10, -200, 0)     (100, -200, 0)
     * clang-format on
     */
    Simulator::Schedule(NanoSeconds(2), [=, this]() {
        auto mm = ueNetDev->GetNode()->GetObject<MobilityModel>();
        auto rnti = DynamicCast<NrUePhy>(ueNetDev->GetPhy(0))->GetRnti();
        mm->SetPosition(m_coord);
        idealBeamformingHelper->AddBeamformingTask(gnbNetDev, ueNetDev);
        auto beamId = gnbNetDev->GetPhy(0)->GetBeamId(rnti);
        NS_TEST_ASSERT_MSG_EQ(beamId.GetSector(),
                              m_expectedSector,
                              "Unexpected sector for UE at " << m_coord);
        NS_TEST_ASSERT_MSG_EQ(beamId.GetElevation(),
                              m_expectedElevation,
                              "Unexpected elevation for UE at " << m_coord);
    });
    Simulator::Stop(simTime);
    Simulator::Run();
    Simulator::Destroy();
}

class TestNrIdealBeamforming : public TestSuite
{
  public:
    TestNrIdealBeamforming()
        : TestSuite("nr-ideal-beamforming-test", Type::SYSTEM)
    {
        for (auto [coord, expCellScan] :
             std::vector<std::tuple<Vector3D, std::pair<double, double>>>{
                 // clang-format off
                 /**
                  * In this first block we check UE below gNB (pointing at horizon)
                  * Scanning from left to right (Y-axis),
                  * then foreground to background (X-axis)
                  *
                  * gNB > antenna is horizontal
                  * |  \
                  * |    \            1 4 7
                  * |      \         2 5 8
                  * |        \ UE   3 6 9
                  */
                 // (UE coordinate)  (sector  elevation)
                 {{ 10, -200, 0.0}, {0, 135}},
                 {{ 10, -150, 0.0}, {0, 135}},
                 {{ 10, -120, 0.0}, {0, 135}},
                 {{ 10, -100, 0.0}, {0, 135}},
                 {{ 10,  -50, 0.0}, {1, 135}},
                 {{ 10,    0, 0.0}, {2, 135}},
                 {{ 10,   20, 0.0}, {3, 135}},
                 {{ 10,  100, 0.0}, {0, 135}},
                 {{ 10,  200, 0.0}, {0, 135}},

                 /**
                  * Same height as gNB (Z-axis)
                  * gNB > ------------- UE   1 4 7
                  * |                       2 5 8
                  * |                      3 6 9
                  * |
                  * |
                  */
                 // (UE coordinate)  (sector  elevation)
                 {{100, -200, 25.0}, {0, 135}},
                 {{100, -100, 25.0}, {0, 135}},
                 {{100,  -50, 25.0}, {1,  45}},
                 {{100,    0, 25.0}, {2, 135}},
                 {{100,   50, 25.0}, {3,  45}},
                 {{100,  100, 25.0}, {3, 135}},
                 {{100,  200, 25.0}, {0, 135}},

                 /**
                  * Pointing above gNB (Z-axis)
                  *            _-- UE  1 4 7
                  *        _--       2 5 8
                  * gNB >          3 6 9
                  * |
                  * |
                  * |
                  * |
                  */
                 // (UE coordinate)  (sector  elevation)
                 {{ 10, -200, 50.0}, {0, 45}},
                 {{ 10,    0, 50.0}, {2, 45}},
                 {{ 10,  200, 50.0}, {0, 45}},
                 {{100, -200, 50.0}, {0, 45}},
                 {{100,    0, 50.0}, {2, 45}},
                 {{100,  200, 50.0}, {3, 45}},
                 // clang-format on
             })
        {
            std::string beamformingName{"CellScanBeamforming"};
            std::stringstream ss;
            int cols = 4;
            int rows = 2;
            ss << beamformingName << " with " << cols << "x" << rows << " antenna at " << coord;
            AddTestCase(new BeamformingTestCase(ss.str(),
                                                beamformingName,
                                                cols,
                                                rows,
                                                coord,
                                                expCellScan.first,
                                                expCellScan.second),
                        TestCase::Duration::QUICK);
        }

        // In the previous block we already checked if CellScan work
        // But that is only valid for 4x2 array with no oversampling
        // Let's then test oversampling
        struct TestParams
        {
            Vector3D coord;
            int cols;
            int rows;
            int oversamp;
            double expectedSector;
            double expectedElevation;
        };

        for (auto testParams : std::vector<TestParams>{
                 // clang-format off
               {{  10,    0, 25.0}, 1, 1, 1,  0,  90},
               {{  10,    0, 25.0}, 1, 1, 2,  0,  90},
               {{  10,    0, 25.0}, 2, 1, 1,  1,  90},
               {{ 100, -200, 25.0}, 2, 1, 2,  0,  90},
               {{ 100, -100, 25.0}, 2, 1, 2,  0,  90},
               {{ 100,  -50, 25.0}, 2, 1, 2,  0,  90},
               {{ 100,  -25, 25.0}, 2, 1, 2,  2,  90},
               {{ 100,    0, 25.0}, 2, 1, 2,  2,  90},
               {{ 100,  150, 25.0}, 8, 8, 1,  7,  99},
               {{ 100,  150, 25.0}, 8, 4, 2, 12,  99},
               {{ 100,  150, 25.0}, 8, 2, 4, 24,  99},
               {{ 100,  150, 25.0}, 4, 8, 2,  6,  93.5},
               {{ 100,  150, 25.0}, 2, 8, 4,  4,  87.5},
                 // clang-format on
             })
        {
            std::string beamformingName = "CellScanBeamforming";
            std::stringstream ss;
            ss << beamformingName << " with " << testParams.cols << "x" << testParams.rows << "x"
               << testParams.oversamp << " antenna at " << testParams.coord;
            AddTestCase(new BeamformingTestCase(ss.str(),
                                                beamformingName,
                                                testParams.cols,
                                                testParams.rows,
                                                testParams.coord,
                                                testParams.expectedSector,
                                                testParams.expectedElevation,
                                                testParams.oversamp),
                        TestCase::Duration::QUICK);
        }
    }
};

static TestNrIdealBeamforming g_testNrIdealBeamforming; //!< ideal beamforming test suite

} // namespace ns3
