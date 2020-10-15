/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "ns3/test.h"
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/callback.h>
#include <ns3/config.h>
#include <ns3/string.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include <ns3/pointer.h>
#include <ns3/integer.h>
#include <ns3/mobility-helper.h>
#include <ns3/spectrum-value.h>
#include <ns3/ff-mac-scheduler.h>
#include <ns3/nr-module.h>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("NrUplinkPowerControlTestCase");

/**
 * \file nr-test-uplink-power-control.cc
 * \ingroup test
 *
 * \brief
 *
 */
class NrUplinkPowerControlTestSuite : public TestSuite
{
public:
  NrUplinkPowerControlTestSuite ();
};


/**
 * \file nr-test-uplink-power-control.cc
 * \ingroup test
 *
 * \brief NR uplink power control test case. Tests PUSCH and PUCCH
 * power control adaptation. Move UE to different positions and check
 * whether the power for is adjusted as expected (open loop, closed loop
 * absolute/accumulated mode).
 */
class NrUplinkPowerControlTestCase : public TestCase
{
public:
  /**
   * Constructor
   * \param name the test case name
   */
  NrUplinkPowerControlTestCase (std::string name, bool openLoop, bool accumulatedMode);
  /**
   * Destructor
   */
  virtual ~NrUplinkPowerControlTestCase ();

  /**
   * Function that moves the UE to a different position
   *
   * \param distance a new distance to be set between UE and gNB
   * \param expectedPuschTxPower the expected PUSCH transmit power after moving to a new position
   * \param expectedPucchTxPower the expected PUCCH transmit power after moving to a new position
   */
  void MoveUe (uint32_t distance, double expectedPuschTxPower, double expectedPucchTxPower);

  /**
   * PUSCH transmit power trace function
   * \param cellId the cell ID
   * \param rnti the RNTI
   * \param txPower the transmit power
   */
  void PuschTxPowerTrace (uint16_t cellId, uint16_t rnti, double txPower);
  /**
   * PUCCH transmit power trace function
   * \param cellId the cell ID
   * \param rnti the RNTI
   * \param txPower the transmit power
   */
  void PucchTxPowerTrace (uint16_t cellId, uint16_t rnti, double txPower);

protected:
  virtual void DoRun (void);

  Ptr<MobilityModel> m_ueMobility; //!< UE mobility model
  Ptr<NrUePowerControl> m_ueUpc; //!< UE power control
  Time m_movingTime; //!< moving time
  double m_expectedPuschTxPower {0.0}; //!< expected PUSCH transmit power
  double m_expectedPucchTxPower {0.0}; //!< expected PUCCH transmit power
  bool m_closedLoop {true}; //!< Indicates whether open or closed loops is being used
  bool m_accumulatedMode {true}; //!< if closed loop is configured indicates which TPC mode will be used for the closed loop power control
  bool m_puschTxPowerTraceFired {true}; //! indicator if the trace that calls the test function got executed
  bool m_pucchTxPowerTraceFired {true}; //! indicator if the trace that calls the test function got executed
};

/**
 * TestSuite
 */
NrUplinkPowerControlTestSuite::NrUplinkPowerControlTestSuite ()
  : TestSuite ("nr-test-uplink-power-control", SYSTEM)
{
  //LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_DEBUG);
  //LogComponentEnable ("NrUplinkPowerControlTestSuite", logLevel);
  NS_LOG_INFO ("Creating NrUplinkPowerControlTestSuite");
  AddTestCase (new NrUplinkPowerControlTestCase ("OpenLoopPowerControlTest", true, false), TestCase::QUICK);
  AddTestCase (new NrUplinkPowerControlTestCase ("ClosedLoopPowerControlAbsoluteModeTest", false, false), TestCase::QUICK);
  AddTestCase (new NrUplinkPowerControlTestCase ("ClosedLoopPowerControlAccumulatedModeTest", false, true), TestCase::QUICK);
}

static NrUplinkPowerControlTestSuite lteUplinkPowerControlTestSuite;

/**
 * TestCase Data
 */
void
PuschTxPowerReport (NrUplinkPowerControlTestCase *testcase,
                          uint16_t cellId, uint16_t rnti, double txPower)
{
  testcase->PuschTxPowerTrace (cellId, rnti, txPower);
}

void
PucchTxPowerReport (NrUplinkPowerControlTestCase *testcase,
                          uint16_t cellId, uint16_t rnti, double txPower)
{
  testcase->PucchTxPowerTrace (cellId, rnti, txPower);
}

NrUplinkPowerControlTestCase::NrUplinkPowerControlTestCase (std::string name, bool openLoop, bool accumulatedMode)
  : TestCase (name)
{
  NS_LOG_INFO ("Creating NrUplinkPowerControlTestCase");
  m_closedLoop = openLoop;
  m_accumulatedMode = accumulatedMode; // if closed loope configures indicates which TPC mode will be used for the closed loop power control
}

NrUplinkPowerControlTestCase::~NrUplinkPowerControlTestCase ()
{
}

void
NrUplinkPowerControlTestCase::MoveUe (uint32_t distance, double expectedPuschTxPower, double expectedPucchTxPower)
{
  NS_LOG_FUNCTION (this);

  //NS_TEST_ASSERT_MSG_EQ (m_pucchTxPowerTraceFired, true, "Power trace for PUCCH did not get triggered. Test check for PUCCH did not executed as expected. ");
  m_pucchTxPowerTraceFired = false; // reset
  NS_TEST_ASSERT_MSG_EQ (m_puschTxPowerTraceFired, true, "Power trace for PUSCH did not get triggered. Test check did PUSCH not executed as expected. ");
  m_puschTxPowerTraceFired = false; // reset
  Vector newPosition = m_ueMobility->GetPosition();
  newPosition.x = distance;
  m_ueMobility->SetPosition (newPosition);
  NS_LOG_DEBUG ("Move UE to : "<<m_ueMobility->GetPosition());
  m_movingTime = Simulator::Now ();
  m_expectedPuschTxPower = expectedPuschTxPower;
  m_expectedPucchTxPower = expectedPucchTxPower;
}

void
NrUplinkPowerControlTestCase::PuschTxPowerTrace (uint16_t cellId, uint16_t rnti, double txPower)
{
  NS_LOG_FUNCTION (this);
  m_puschTxPowerTraceFired = true;
  NS_LOG_DEBUG ("PuschTxPower for CellId: " << cellId << " RNTI: " << rnti << " PuschTxPower: " << txPower);
  //wait because of RSRP filtering
  if ( (Simulator::Now () - m_movingTime ) < MilliSeconds (50))
    {
      return;
    }
  NS_TEST_ASSERT_MSG_EQ_TOL (txPower, m_expectedPuschTxPower, 0.01, "Wrong Pusch Tx Power");
}

void
NrUplinkPowerControlTestCase::PucchTxPowerTrace (uint16_t cellId, uint16_t rnti, double txPower)
{
  NS_LOG_FUNCTION (this);
  m_pucchTxPowerTraceFired = true;
  NS_LOG_DEBUG ("PucchTxPower : CellId: " << cellId << " RNTI: " << rnti << " PuschTxPower: " << txPower);
  //wait because of RSRP filtering
  if ( (Simulator::Now () - m_movingTime ) < MilliSeconds (50))
    {
      return;
    }

  NS_TEST_ASSERT_MSG_EQ_TOL (txPower, m_expectedPucchTxPower, 0.01, "Wrong Pucch Tx Power");
}

void
NrUplinkPowerControlTestCase::DoRun (void)
{
  std::string scenario = "UMi-StreetCanyon"; //scenario
  double frequency = 2e9; // central frequency
  double bandwidth = 20e6; //bandwidth
  double hBS = 10, hUT = 1.5; //base station antenna height in meters, and user antenna height in meters
  double gNBTxPower = 30, ueTxPower = 10; // txPower
  enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::UMi_StreetCanyon_LoS;
  uint16_t numerology = 0; // numerology to be used
  uint16_t numCcPerBand = 1; // number of component carrier in the assigned band

  Config::Reset ();

  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (true));
  Config::SetDefault ("ns3::LteUePowerControl::ClosedLoop", BooleanValue (m_closedLoop));
  Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (m_accumulatedMode));
  Config::SetDefault ("ns3::LteUePowerControl::PoNominalPusch", IntegerValue (-90));
  Config::SetDefault ("ns3::LteUePowerControl::PsrsOffset", IntegerValue (9));

  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject <IdealBeamformingHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  double distance = 0;

  // Create Nodes: eNodeB and UE
  NodeContainer gnbNodes;
  NodeContainer ueNodes;
  gnbNodes.Create (1);
  ueNodes.Create (1);
  NodeContainer allNodes = NodeContainer (gnbNodes, ueNodes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, hBS)); // gNB
  positionAlloc->Add (Vector (distance, 0.0, hUT));  // UE

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);
  m_ueMobility = ueNodes.Get (0)->GetObject<MobilityModel> ();

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (numerology));
  nrHelper->SetGnbPhyAttribute ("TxPower", DoubleValue (gNBTxPower));
  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (ueTxPower));
  nrHelper->SetUePhyAttribute ("EnableUplinkPowerControl", BooleanValue (true));

  CcBwpCreator::SimpleOperationBandConf bandConf (frequency, bandwidth, numCcPerBand, scenarioEnum);
  CcBwpCreator ccBwpCreator;
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
  //Initialize channel and pathloss, plus other things inside band.
  nrHelper->InitializeOperationBand (&band);
  BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps ({band});

  // Configure ideal beamforming method
  idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Antennas for the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (true));

  // Antennas for the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (true));

  enbDevs = nrHelper->InstallGnbDevice (gnbNodes, allBwps);
  ueDevs = nrHelper->InstallUeDevice (ueNodes, allBwps);

  Ptr<NrUePhy> uePhy = nrHelper->GetUePhy (ueDevs.Get(0), 0);

  m_ueUpc = uePhy->GetUplinkPowerControl ();

  m_ueUpc->TraceConnectWithoutContext ("ReportPuschTxPower",
                                       MakeBoundCallback (&PuschTxPowerReport, this));
  m_ueUpc->TraceConnectWithoutContext ("ReportPucchTxPower",
                                       MakeBoundCallback (&PucchTxPowerReport, this));

  // Attach a UE to a eNB
  nrHelper->AttachToEnb (ueDevs.Get(0), enbDevs.Get (0));

  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  nrHelper->ActivateDataRadioBearer (ueDevs, bearer);

  //Changing UE position

  if (!m_closedLoop)
    {
      Simulator::Schedule (MilliSeconds (0),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 0, -40, -40 );
      Simulator::Schedule (MilliSeconds (200),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 200, 8.9745, 8.9745 );
      Simulator::Schedule (MilliSeconds (300),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 400, 14.9951, 14.9951 );
      Simulator::Schedule (MilliSeconds (400),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 600, 18.5169, 18.5169 );
      Simulator::Schedule (MilliSeconds (500),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 800, 21.0157, 21.0157 );
      Simulator::Schedule (MilliSeconds (600),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 22.9539, 22.9539 );
      Simulator::Schedule (MilliSeconds (700),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 1200, 23, 10 );
      Simulator::Schedule (MilliSeconds (800),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 400, 14.9951, 14.9951 );
      Simulator::Schedule (MilliSeconds (900),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 800, 21.0157, 21.0157 );
      Simulator::Schedule (MilliSeconds (1000),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 0, -40, -40 );
      Simulator::Schedule (MilliSeconds (1100),
                       &NrUplinkPowerControlTestCase::MoveUe, this, 100, 2.9539, 2.9539 );
    }
  else
    {
      if (m_accumulatedMode)
        {
          Simulator::Schedule (MilliSeconds (0),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 0, -40, -40 );
          Simulator::Schedule (MilliSeconds (200),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 200, 8.9745, 8.9745 );
          Simulator::Schedule (MilliSeconds (300),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 400, 14.9951, 14.9951 );
          Simulator::Schedule (MilliSeconds (400),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 600, 18.5169, 18.5169 );
          Simulator::Schedule (MilliSeconds (500),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 800, 21.0157, 21.0157 );
          Simulator::Schedule (MilliSeconds (600),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 22.9539, 22.9539 );
          Simulator::Schedule (MilliSeconds (700),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 1200, 23, 10 );
          Simulator::Schedule (MilliSeconds (800),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 400, 14.9951, 14.9951 );
          Simulator::Schedule (MilliSeconds (900),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 800, 21.0157, 21.0157 );
          Simulator::Schedule (MilliSeconds (1000),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 0, -40, -40 );
          Simulator::Schedule (MilliSeconds (1100),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 100, 2.9539, 2.9539 );
        }
      else
        {
          Simulator::Schedule (MilliSeconds (0),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 0, -40, -40 );
          Simulator::Schedule (MilliSeconds (200),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 200, 8.9745, 8.9745 );
          Simulator::Schedule (MilliSeconds (300),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 400, 14.9951, 14.9951 );
          Simulator::Schedule (MilliSeconds (400),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 600, 18.5169, 18.5169 );
          Simulator::Schedule (MilliSeconds (500),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 800, 21.0157, 21.0157 );
          Simulator::Schedule (MilliSeconds (600),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 22.9539, 22.9539 );
          Simulator::Schedule (MilliSeconds (700),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 1200, 23, 10 );
          Simulator::Schedule (MilliSeconds (800),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 400, 14.9951, 14.9951 );
          Simulator::Schedule (MilliSeconds (900),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 800, 21.0157, 21.0157 );
          Simulator::Schedule (MilliSeconds (1000),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 0, -40, -40 );
          Simulator::Schedule (MilliSeconds (1100),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 100, 2.9539, 2.9539 );
        }
    }

  Simulator::Stop (Seconds (1.200));
  Simulator::Run ();

  Simulator::Destroy ();
}
