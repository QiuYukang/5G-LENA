/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
 *  
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *  
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *  
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 *   Author: Biljana Bojovic <bbojovic@cttc.es>
 */

#include "ns3/mmwave-helper.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include "ns3/log.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/eps-bearer-tag.h"

// An essential include is test.h
#include "ns3/test.h"

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;


/**
  * \file mmwave-test-numerology-delay.cc
  * \ingroup test
  * \brief Check each numerology delay
  *
  * In this test case we want to observe delays of a single UDP packet, and to track its
  * eNB processing time, air time, UE time depending on the numerology.
  */

static uint32_t packetSize = 1000;

class MmwaveTestNumerologyDelayCase1 : public TestCase
{
public:
  MmwaveTestNumerologyDelayCase1 (std::string name, uint32_t numerology);
  virtual ~MmwaveTestNumerologyDelayCase1 ();
  void DlScheduling (uint32_t frameNo, uint32_t subframeNo, uint32_t slotNum, uint32_t tbSize, uint32_t mcs, uint32_t rnti, uint8_t componentCarrierId);
  void DlSpectrumUeEndRx (RxPacketTraceParams params);
  void DlSpectrumEnbStartTx (EnbPhyPacketCountParameter params);
  void TxRlcPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes);
  void TxPdcpPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes);
  void RxRlcPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay);
  void RxPdcpPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay);

private:
  virtual void DoRun (void);

  uint32_t m_numerology;
  Ptr<MmWavePhyMacCommon> m_mmWavePhyMacCommon;
  Time m_sendPacketTime;
  uint32_t m_numSym;
  bool m_firstMacPdu;
  bool m_firstDlTransmission;
  bool m_firstDlReception;
  bool m_firstRxPlcPDU;
  Time m_lastDlReceptionFinished;
  uint32_t m_slotsCounter;
  uint32_t m_totalNumberOfSymbols;
  uint32_t m_firstMacPduMcs;
};

// Add some help text to this case to describe what it is intended to test
MmwaveTestNumerologyDelayCase1::MmwaveTestNumerologyDelayCase1 (std::string name, uint32_t numerology)
: TestCase (name)
{
  m_numerology = numerology;
  m_firstMacPdu = true;
  m_firstDlTransmission = true;
  m_firstDlReception = true;
  m_firstRxPlcPDU = true;
  m_lastDlReceptionFinished = Seconds (0);
  m_slotsCounter = 0;
  m_firstMacPduMcs = 0;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
MmwaveTestNumerologyDelayCase1::~MmwaveTestNumerologyDelayCase1 ()
{
}


void
LteTestDlSchedCallback (MmwaveTestNumerologyDelayCase1 *testcase, std::string path,
                                uint32_t frameNo, uint32_t subframeNo,
                                uint32_t slotNum, uint32_t tbSize, uint32_t mcs, uint32_t rnti, uint8_t componentCarrierId)
{
  testcase->DlScheduling (frameNo, subframeNo, slotNum, tbSize, mcs, rnti, componentCarrierId);
}

void
LteTestRxPacketUeCallback (MmwaveTestNumerologyDelayCase1 *testcase, std::string path,
                                   RxPacketTraceParams rxParams)
{

  testcase->DlSpectrumUeEndRx (rxParams);
}

void
LteTestTxPacketEnbCallback (MmwaveTestNumerologyDelayCase1 *testcase, std::string path,
                                    EnbPhyPacketCountParameter params)
{

  testcase->DlSpectrumEnbStartTx (params);
}

void
LteTestTxRlcPDUCallback (MmwaveTestNumerologyDelayCase1 *testcase, std::string path,
                         uint16_t rnti, uint8_t lcid, uint32_t bytes)
{

  testcase->TxRlcPDU ( rnti, lcid, bytes);
}

void
LteTestTxPdcpPDUCallback (MmwaveTestNumerologyDelayCase1 *testcase, std::string path,
                         uint16_t rnti, uint8_t lcid, uint32_t bytes)
{
  testcase->TxPdcpPDU (rnti, lcid, bytes);
}


void
LteTestRxRlcPDUCallback (MmwaveTestNumerologyDelayCase1 *testcase, std::string path,
                         uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay)
{

  testcase->RxRlcPDU ( rnti, lcid, bytes, delay);
}

void
LteTestRxPdcpPDUCallback (MmwaveTestNumerologyDelayCase1 *testcase, std::string path,
                          uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay)
{
  testcase->RxPdcpPDU (rnti, lcid, bytes, delay);
}


void
ConnectRlcPdcpTraces (MmwaveTestNumerologyDelayCase1 *testcase)
{

  Config::Connect ("/NodeList/0/DeviceList/0/LteEnbRrc/UeMap/1/DataRadioBearerMap/1/LteRlc/TxPDU",
                       MakeBoundCallback (&LteTestTxRlcPDUCallback, testcase));

  Config::Connect ("/NodeList/0/DeviceList/0/LteEnbRrc/UeMap/1/DataRadioBearerMap/1/LtePdcp/TxPDU",
                       MakeBoundCallback (&LteTestTxPdcpPDUCallback, testcase));

  Config::Connect ("/NodeList/1/DeviceList/0/LteUeRrc/DataRadioBearerMap/1/LteRlc/RxPDU",
                         MakeBoundCallback (&LteTestRxRlcPDUCallback, testcase));

  Config::Connect ("/NodeList/1/DeviceList/0/LteUeRrc/DataRadioBearerMap/1/LtePdcp/RxPDU",
                       MakeBoundCallback (&LteTestRxPdcpPDUCallback, testcase));
}

static void SendPacket (Ptr<NetDevice> device, Address& addr)
{
  Ptr<Packet> pkt = Create<Packet> (packetSize);
  EpsBearerTag tag (1, 1);
  pkt->AddPacketTag (tag);
  device->Send (pkt, addr, Ipv4L3Protocol::PROT_NUMBER);
}

void
MmwaveTestNumerologyDelayCase1::DoRun (void)
{
  m_sendPacketTime = MilliSeconds(400);

  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Frequency", DoubleValue(28e9));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue(28e9));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::Bandwidth", DoubleValue(400e6));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::Numerology", UintegerValue(m_numerology));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(false));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("n"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));

  Config::SetDefault ("ns3::MmWaveMacSchedulerNs3::FixedMcsDl", BooleanValue(true));
  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::McsDefaultDl", UintegerValue (1));

  ns3::SeedManager::SetRun(5);

  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();
  mmWaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
  mmWaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));
  Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmWaveHelper->SetEpcHelper (epcHelper);

  Ptr<Node> ueNode = CreateObject<Node> ();
  Ptr<Node> gNbNode = CreateObject<Node> ();

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (gNbNode);
  mobility.Install (ueNode);
  gNbNode->GetObject<MobilityModel>()->SetPosition (Vector(0.0, 0.0, 10));
  ueNode->GetObject<MobilityModel> ()->SetPosition (Vector (0, 10 , 1.5));

  NetDeviceContainer enbNetDev = mmWaveHelper->InstallEnbDevice (gNbNode);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNode);

  InternetStackHelper internet;
  internet.Install (ueNode);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  Simulator::Schedule (m_sendPacketTime, &SendPacket, enbNetDev.Get(0), ueNetDev.Get(0)->GetAddress());

  // attach UEs to the closest eNB
  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  m_mmWavePhyMacCommon =  CreateObject<MmWavePhyMacCommon>();
  m_mmWavePhyMacCommon->DoInitialize();

  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveEnbMac/DlScheduling",
                      MakeBoundCallback (&LteTestDlSchedCallback, this));

  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUePhy/DlSpectrumPhy/RxPacketTraceUe",
                       MakeBoundCallback (&LteTestRxPacketUeCallback, this));

  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveEnbPhy/DlSpectrumPhy/TxPacketTraceEnb",
                       MakeBoundCallback (&LteTestTxPacketEnbCallback, this));

  Simulator::Schedule(MilliSeconds(200), &ConnectRlcPdcpTraces, this);

  mmWaveHelper->EnableTraces();

  Simulator::Stop (MilliSeconds (1000));
  Simulator::Run ();
  Simulator::Destroy ();
}



void
MmwaveTestNumerologyDelayCase1::TxPdcpPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes)
{
/*  std::cout<<"\n\n Packet transmitted by gNB PDCP at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti :"<<rnti<<std::endl;
  std::cout<<"\n lcid :"<<(unsigned) lcid<<std::endl;
  std::cout<<"\n bytes :"<<bytes<<std::endl;*/

  NS_TEST_ASSERT_MSG_EQ (Simulator::Now(), m_sendPacketTime, "There should not be delay between packet being sent and being scheduled by the gNb PDCP.");
}

void
MmwaveTestNumerologyDelayCase1::TxRlcPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes)
{
/*  std::cout<<"\n\n Packet transmitted by gNB RLC at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti:"<<rnti<<std::endl;
  std::cout<<"\n lcid:"<<rnti<<(unsigned)lcid;
  std::cout<<"\n no of bytes :"<<bytes<<std::endl;*/

  NS_TEST_ASSERT_MSG_EQ (Simulator::Now(), m_sendPacketTime, "There should not be delay between packet being sent and being transmited by the gNb RLC.");
}

void
MmwaveTestNumerologyDelayCase1::DlScheduling (uint32_t frameNo, uint32_t subframeNo,
                                              uint32_t slotNum, uint32_t tbSize,
                                              uint32_t mcs, uint32_t rnti, uint8_t componentCarrierId)
{
/*  std::cout<<"\n\n\n MAC sends PDU to PHY at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n MCS :"<<mcs<<std::endl;
  std::cout<<"\n rnti :"<<rnti<<std::endl;
  std::cout<<"\n frameNo :"<<slotNum<<std::endl;
  std::cout<<"\n subframeNo :"<<slotNum<<std::endl;
  std::cout<<"\n slotNo :"<<slotNum<<std::endl;*/

  if (m_firstMacPdu)
    {
      NS_TEST_ASSERT_MSG_EQ (Simulator::Now(), m_sendPacketTime, "There should not be delay between packet being sent and being scheduled by the MAC.");
      m_firstMacPdu = false;
      m_firstMacPduMcs = mcs;
    }
  m_slotsCounter++;
}

void
MmwaveTestNumerologyDelayCase1::DlSpectrumEnbStartTx (EnbPhyPacketCountParameter params)
{
/*  std::cout<<"\n\n Started transmission at eNb PHY at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n cell id :"<<params.m_cellId<<std::endl;
  std::cout<<"\n no of bytes :"<<(unsigned)params.m_noBytes<<std::endl;
  std::cout<<"\n subframe no:"<<params.m_subframeno<<std::endl;*/
  Time delay = m_mmWavePhyMacCommon->GetL1L2CtrlLatency() * m_mmWavePhyMacCommon->GetSlotPeriod();
  Time ctrlDuration = m_mmWavePhyMacCommon->GetSymbolPeriod();
  // first there is L1L2 processing delay
  // the, before it start the transmission of the DATA symbol, there is 1 DL CTRL symbol
  // and then we are here already in the following nano second

  if (m_firstDlTransmission)
    {
      NS_TEST_ASSERT_MSG_EQ (Simulator::Now(), m_sendPacketTime + delay + ctrlDuration + NanoSeconds(1),
                         "The delay between packet scheduled by the MAC and being transmitted should be L1L2 delay, plus the duration of the control.");
      m_firstDlTransmission = false;
    }
}

void
MmwaveTestNumerologyDelayCase1::DlSpectrumUeEndRx (RxPacketTraceParams params)
{
/*  std::cout<<"\n\n Finished reception at UE PHY at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n MCS :"<<params.m_mcs<<std::endl;
  std::cout<<"\n slot :"<<(unsigned int)params.m_slotNum<<std::endl;
  std::cout<<"\n rnti:"<<params.m_rnti<<std::endl;*/

  Time delay = m_mmWavePhyMacCommon->GetL1L2CtrlLatency() * m_mmWavePhyMacCommon->GetSlotPeriod ();
  Time ctrlDuration = m_mmWavePhyMacCommon->GetSymbolPeriod ();
  Time dataDuration = (m_mmWavePhyMacCommon->GetSymbolPeriod () * params.m_numSym) - NanoSeconds (1);

/*  std::cout<<"\n symbol duration:"<<  Seconds (m_mmWavePhyMacCommon->GetSymbolPeriod());
  std::cout<<"\n symbols:" << (unsigned) params.m_numSym;
  std::cout<<"\n my calculation:"<<m_sendPacketTime + delay + ctrlDuration + dataDuration;*/

  if (m_firstDlReception)
    {
       NS_TEST_ASSERT_MSG_EQ (Simulator::Now(), m_sendPacketTime + delay + ctrlDuration + dataDuration,
                           "The duration of the transmission of the packet is not correct");
       m_firstDlReception = false;
       m_numSym = params.m_numSym;
    }

  m_lastDlReceptionFinished = Simulator::Now();
  m_totalNumberOfSymbols += params.m_numSym;
}

void
MmwaveTestNumerologyDelayCase1::RxRlcPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t rlcDelay)
{
/*  std::cout<<"\n\n Packet received by UE RLC at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti:"<<rnti<<std::endl;
  std::cout<<"\n lcid:"<<(unsigned)lcid<<std::endl;
  std::cout<<"\n bytes :"<< bytes<<std::endl;
  std::cout<<"\n delay :"<< rlcDelay<<std::endl;*/

  Time delay = m_mmWavePhyMacCommon->GetL1L2CtrlLatency() * m_mmWavePhyMacCommon->GetSlotPeriod();
  Time ctrlDuration = m_mmWavePhyMacCommon->GetSymbolPeriod();
  Time dataDuration = (m_mmWavePhyMacCommon->GetSymbolPeriod() * m_numSym) - NanoSeconds(1);
  Time tbDecodeDelay = MicroSeconds(m_mmWavePhyMacCommon->GetTbDecodeLatency());

  if (m_firstRxPlcPDU)
    {
      NS_TEST_ASSERT_MSG_EQ (Simulator::Now(), m_sendPacketTime + delay + ctrlDuration + dataDuration + tbDecodeDelay,
                           "The duration of the reception by RLC is not correct.");
      m_firstRxPlcPDU = false;
    }
}

void
MmwaveTestNumerologyDelayCase1::RxPdcpPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t pdcpDelay)
{
/*  std::cout<<"\n\n Packet received by UE PDCP at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti :"<<rnti<<std::endl;
  std::cout<<"\n lcid:"<<(unsigned)lcid<<std::endl;
  std::cout<<"\n bytes :"<< bytes<<std::endl;
  std::cout<<"\n delay :"<<pdcpDelay<<std::endl;*/

  Time delay = m_mmWavePhyMacCommon->GetL1L2CtrlLatency() * m_mmWavePhyMacCommon->GetSlotPeriod();
  Time ctrlDuration = m_mmWavePhyMacCommon->GetSymbolPeriod();
  Time dataDuration = (m_mmWavePhyMacCommon->GetSymbolPeriod() * m_numSym) - NanoSeconds(1);
  Time tbDecodeDelay = MicroSeconds(m_mmWavePhyMacCommon->GetTbDecodeLatency());

  NS_TEST_ASSERT_MSG_EQ (Simulator::Now(), m_lastDlReceptionFinished + tbDecodeDelay,
                           "The duration of the reception by PDCP is not correct.");

  std::cout<<"\n Numerology:"<<m_numerology<<"\t Packet of :" <<packetSize<<" bytes\t#Slots:"
      <<m_slotsCounter <<"\t#Symbols:"<<m_totalNumberOfSymbols<<"\tPacket PDCP delay:"<<pdcpDelay
      <<"\tRLC delay of first PDU:"<<delay + ctrlDuration + dataDuration + tbDecodeDelay
      <<"\tMCS of the first PDU:"<<m_firstMacPduMcs;
}


// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class MmwaveTestNumerologyDelayTestSuite : public TestSuite
{
public:
  MmwaveTestNumerologyDelayTestSuite ();
};

MmwaveTestNumerologyDelayTestSuite::MmwaveTestNumerologyDelayTestSuite ()
: TestSuite ("mmwave-test-numerology-delay", SYSTEM)
{
   AddTestCase (new MmwaveTestNumerologyDelayCase1 ("num=0", 0), TestCase::QUICK);
   AddTestCase (new MmwaveTestNumerologyDelayCase1 ("num=1", 1), TestCase::QUICK);
   AddTestCase (new MmwaveTestNumerologyDelayCase1 ("num=2", 2), TestCase::QUICK);
   AddTestCase (new MmwaveTestNumerologyDelayCase1 ("num=3", 3), TestCase::QUICK);
   AddTestCase (new MmwaveTestNumerologyDelayCase1 ("num=4", 4), TestCase::QUICK);
   AddTestCase (new MmwaveTestNumerologyDelayCase1 ("num=5", 5), TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static MmwaveTestNumerologyDelayTestSuite mmwaveTestSuite;

