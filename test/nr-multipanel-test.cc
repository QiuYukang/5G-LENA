// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/antenna-module.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/nr-module.h"
#include "ns3/spectrum-model.h"
#include "ns3/test.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/three-gpp-propagation-loss-model.h"
#include "ns3/three-gpp-spectrum-propagation-loss-model.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrMultipanelTest");

/**
 * @ingroup test
 * @file nr-multipanel-test.cc
 *
 * @brief
 */
class NrMultipanelTestCase : public TestCase
{
  public:
    NrMultipanelTestCase(uint8_t panel, uint8_t uePorts, uint8_t numerology);
    ~NrMultipanelTestCase() override;

  private:
    void DoRun() override;
    uint8_t m_panel = 0;
    uint8_t m_uePorts = 0;
    uint8_t m_numerology = 0;
};

/**
 * TestCase
 */
NrMultipanelTestCase::NrMultipanelTestCase(uint8_t panel, uint8_t uePorts, uint8_t numerology)
    : TestCase("Test if 4-panel UE, with " + std::to_string(uePorts) +
               " ports each, correctly attaches with panel " + std::to_string(panel) +
               " with numerology " + std::to_string(numerology)),
      m_panel(panel),
      m_uePorts(uePorts),
      m_numerology(numerology)
{
}

NrMultipanelTestCase::~NrMultipanelTestCase()
{
}

void
NrMultipanelTestCase::DoRun()
{
    NrHelper::AntennaParams apUe;
    NrHelper::AntennaParams apGnb;
    apUe.antennaElem = "ns3::ThreeGppAntennaModel";
    apUe.nAntCols = 8;
    apUe.nAntRows = 2;
    apUe.nHorizPorts = m_uePorts;
    apUe.nVertPorts = 1;
    apUe.isDualPolarized = false;
    apGnb.antennaElem = "ns3::ThreeGppAntennaModel";
    apGnb.nAntCols = 16;
    apGnb.nAntRows = 8;
    apGnb.nHorizPorts = 1;
    apGnb.nVertPorts = 1;
    apGnb.isDualPolarized = false;
    apGnb.downtiltAngle = 0;
    apUe.bearingAngle = 0 * (M_PI / 180);
    apUe.polSlantAngle = 90.0 * (M_PI / 180);
    apGnb.bearingAngle = 0.0 * (M_PI / 180);
    apGnb.polSlantAngle = 0.0 * (M_PI / 180);

    double centralFrequency = 3.5e9;
    double bandwidth = 20e6;
    double txPowerGnb = 23; // dBm
    double txPowerUe = 23;  // dBm

    NS_ABORT_IF(centralFrequency < 0.5e9 && centralFrequency > 100e9);

    NodeContainer gnbContainer;
    gnbContainer.Create(4);
    NodeContainer ueContainer;
    ueContainer.Create(1);

    /**
     * The test topology is the following:
     * UE0 has 4 antenna panels. We have 4 possible gNBs for it to attach.
     * We increase the power of the gNB we want it to attach. We run maximum RSRP attachment.
     * We check if the desired gNB was selected, and if the panel we wanted to see used was indeed
     * used.
     * clang-format off
     *                    gNB1 : (100, 100, 25.0) : bearingAngle=270
     *                         :
     *                         :
     * gNB2....................UE0..................gNB0
     *    (0.0, 0.0, 25.0)     : (100, 0.0, 1.5)      (200,0.0, 25.0)
     *    bearingAngle=0       : bearingAngle=X       bearingAngle=180
     *                         :
     *                         :
     *                         gNB3
     *                            (100, -100, 25.0)
     *                            bearingAngle=90
     *
     * clang-format on
     */
    MobilityHelper gnbMobility;
    gnbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> gnbPositionAlloc = CreateObject<ListPositionAllocator>();
    gnbPositionAlloc->Add(Vector(200.0, 0.0, 1.5));
    gnbPositionAlloc->Add(Vector(100.0, 100.0, 1.5));
    gnbPositionAlloc->Add(Vector(0.0, 0.0, 1.5));
    gnbPositionAlloc->Add(Vector(100.0, -100.0, 1.5));

    MobilityHelper ueMobility;
    ueMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator>();
    uePositionAlloc->Add(Vector(100, 0.0, 1.5));
    gnbMobility.SetPositionAllocator(gnbPositionAlloc);
    ueMobility.SetPositionAllocator(uePositionAlloc);
    gnbMobility.Install(gnbContainer);
    ueMobility.Install(ueContainer);

    /**
     * Create the NR helpers that will be used to create and setup NR devices, spectrum, ...
     */
    Ptr<NrPointToPointEpcHelper> nrEpcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(DirectPathBeamforming::GetTypeId()));

    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    nrHelper->SetEpcHelper(nrEpcHelper);
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1;
    CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency, bandwidth, numCcPerBand);
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    // Create the channel helper
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    // Set the channel using the scenario and user input
    channelHelper->ConfigureFactories("UMa", "LOS", "ThreeGpp");
    channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
    // Create and set the channel with the band
    channelHelper->AssignChannelsToBands({band});

    nrHelper->SetupGnbAntennas(apGnb);
    nrHelper->SetupUeAntennas(apUe);
    nrHelper->SetGnbPhyAttribute("Numerology", UintegerValue(m_numerology));
    nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(txPowerGnb));
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(txPowerUe));

    BandwidthPartInfoPtrVector allBwps;
    allBwps = CcBwpCreator::GetAllBwps({band});

    /**
     * Finally, create the gNB and the UE device.
     */
    NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice(gnbContainer, allBwps);
    nrHelper->SetUeSpectrumAttribute("NumAntennaPanel", UintegerValue(4));
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueContainer, allBwps);

    // Point all gNB antennas to center
    for (size_t i = 0; i < gnbNetDev.GetN(); i++)
    {
        auto ant = NrHelper::GetGnbPhy(gnbNetDev.Get(i), 0)
                       ->GetSpectrumPhy()
                       ->GetAntenna()
                       ->GetObject<UniformPlanarArray>();
        NS_ASSERT_MSG(ant, "Antenna is not of UniformPlanarArray type");
        ant->SetAlpha(M_PI + ((i * 90 * M_PI) / 180)); // Bearing angleM_PI_4 / 2 +
        gnbNetDev.Get(i)
            ->GetObject<NrGnbNetDevice>()
            ->GetPhy(0)
            ->GetSpectrumPhy()
            ->GetBeamManager()
            ->ChangeBeamformingVector(ueNetDev.Get(0));
    }

    // Increase power of the gNB we want to respective panel from UE to attach
    auto gnb = DynamicCast<NrGnbNetDevice>(gnbNetDev.Get(m_panel));
    gnb->GetPhy(0)->SetTxPower(40);

    // Create the Internet and install the IP stack on the UEs
    InternetStackHelper internet;
    internet.Install(ueContainer);
    nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));

    // Perform initial attachment
    nrHelper->AttachToMaxRsrpGnb(ueNetDev, gnbNetDev);

    Simulator::Stop(Seconds(1));
    std::cout << "Test if 4-panel UE, with " + std::to_string(m_uePorts) +
                     " ports each, correctly attaches with panel " + std::to_string(m_panel) +
                     " with numerology " + std::to_string(m_numerology)
              << std::endl;
    Simulator::Run();

    // Check UE was actually attached the gNB we wanted and using the correct panel
    auto ueDev = DynamicCast<NrUeNetDevice>(ueNetDev.Get(0));
    auto uePhy = ueDev->GetPhy(0)->GetSpectrumPhy();
    auto activePanel = DynamicCast<UniformPlanarArray>(uePhy->GetAntenna());
    auto targetPanel = DynamicCast<UniformPlanarArray>(uePhy->GetPanelByIndex(m_panel));
    NS_ASSERT_MSG(activePanel, "ActivePanel should be a valid UPA");
    NS_ASSERT_MSG(targetPanel, "TargetPanel should be a valid UPA");
    NS_TEST_EXPECT_MSG_EQ(targetPanel->GetId(),
                          activePanel->GetId(),
                          "Active panel should match gNB" << std::to_string(m_panel)
                                                          << " with increased power");

    // Clean simulator for next run, where the next UE panel shall be selected
    Simulator::Destroy();
}

/**
 * TestSuite
 */
class NrMultipanelTestSuite : public TestSuite
{
  public:
    NrMultipanelTestSuite();
};

NrMultipanelTestSuite::NrMultipanelTestSuite()
    : TestSuite("nr-multipanel-test", Type::SYSTEM)
{
    for (int numerology : {0, 1, 2})
    {
        for (int ports : {1, 2, 4})
        {
            for (int cellToPanel = 0; cellToPanel < 4; cellToPanel++)
            {
                AddTestCase(new NrMultipanelTestCase(cellToPanel, ports, numerology),
                            numerology == 1 ? Duration::QUICK : Duration::EXTENSIVE);
            }
        }
    }
}

static NrMultipanelTestSuite nrTestSuite;

} // namespace ns3
