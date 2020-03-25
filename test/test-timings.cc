/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */

#include "ns3/nr-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/log.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/eps-bearer-tag.h"

#include <unordered_map>

// An essential include is test.h
#include <ns3/test.h>

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;

/**
  * \file test-timings.cc
  * \ingroup test
  * \brief Check each numerology timings
  */

static uint32_t packetSize = 40;

static const std::unordered_map<MmWaveControlMessage::messageType, bool> messageLog =
{
  { MmWaveControlMessage::messageType::DCI,      false },
  { MmWaveControlMessage::messageType::DCI_TDMA, false },
  { MmWaveControlMessage::messageType::DL_CQI,   false },
  { MmWaveControlMessage::messageType::MIB,      false },
  { MmWaveControlMessage::messageType::SIB1,     false },
  { MmWaveControlMessage::messageType::RACH_PREAMBLE, false },
  { MmWaveControlMessage::messageType::RAR,      false },
  { MmWaveControlMessage::messageType::BSR,      false },
  { MmWaveControlMessage::messageType::DL_HARQ,  false },
  { MmWaveControlMessage::messageType::SR,       false },
};

typedef std::unordered_map<MmWaveControlMessage::messageType, uint64_t> TypeToResult;
typedef std::unordered_map<uint32_t, TypeToResult> NumerologyToType;

class NrTimingsTest : public TestCase
{
public:
  NrTimingsTest (const std::string &name, uint32_t numerology, bool verbose);
  virtual ~NrTimingsTest ();

private:
  virtual void DoRun (void);
  void EnbPhyTx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg);
  void EnbPhyRx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg);

  void EnbMacTx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg);
  void EnbMacRx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg);

  void UePhyTx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg);
  void UePhyRx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg);

  void UeMacTx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg);
  void UeMacRx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg);

  uint32_t m_numerology;
  bool verbose {false};
};

NrTimingsTest::NrTimingsTest (const std::string &name, uint32_t numerology, bool verbose)
  : TestCase (name),
    m_numerology (numerology),
    verbose (verbose)
{
}

NrTimingsTest::~NrTimingsTest ()
{
}

static void
SendPacket (const Ptr<NetDevice> &device, const Address& addr)
{
  Ipv4Header header;
  NS_ASSERT (packetSize > header.GetSerializedSize ());
  Ptr<Packet> pkt = Create<Packet> (packetSize - header.GetSerializedSize ());
  header.SetProtocol (0x06);
  EpsBearerTag tag (1, 1);
  pkt->AddPacketTag (tag);
  pkt->AddHeader (header);
  device->Send (pkt, addr, Ipv4L3Protocol::PROT_NUMBER);
}

static const std::unordered_map <MmWaveControlMessage::messageType, std::string> TYPE_TO_STRING =
{
  { MmWaveControlMessage::messageType::DCI,      "DCI" },
  { MmWaveControlMessage::messageType::DCI_TDMA, "DCI_TDMA" },
  { MmWaveControlMessage::messageType::DL_CQI,   "DL_CQI" },
  { MmWaveControlMessage::messageType::MIB,      "MIB" },
  { MmWaveControlMessage::messageType::SIB1,     "SIB1" },
  { MmWaveControlMessage::messageType::RACH_PREAMBLE, "RACH_PREAMBLE" },
  { MmWaveControlMessage::messageType::RAR,      "RAR" },
  { MmWaveControlMessage::messageType::BSR,      "BSR" },
  { MmWaveControlMessage::messageType::DL_HARQ,  "DL_HARQ" },
  { MmWaveControlMessage::messageType::SR,       "SR" },
};

void
NrTimingsTest::EnbPhyTx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  NS_UNUSED (rnti);
  NS_UNUSED (ccId);
  static NumerologyToType res =
  {
    {
      4, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 4, 4).Normalize () } },
    },
    {
      3, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 4, 3).Normalize () } },
    },
    {
      2, { { MmWaveControlMessage::RAR, SfnSf (1, 7, 0, 2).Normalize () } },
    },
    {
      1, { { MmWaveControlMessage::RAR, SfnSf (1, 8, 0, 1).Normalize () } },
    },
    {
      0, { { MmWaveControlMessage::RAR, SfnSf (2, 0, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize() << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize(),
              "The message type " << TYPE_TO_STRING.at (msg->GetMessageType()) <<
              " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
              sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::EnbPhyRx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  NS_UNUSED (rnti);
  NS_UNUSED (ccId);

  static NumerologyToType res =
  {
    {
      4, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 4).Normalize () } },
    },
    {
      3, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 3).Normalize () } },
    },
    {
      2, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 2).Normalize () } },
    },
    {
      1, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 1).Normalize () } },
    },
    {
      0, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 7, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize() << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize(),
              "The message type " << TYPE_TO_STRING.at (msg->GetMessageType()) <<
              " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
              sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::EnbMacTx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  NS_UNUSED (rnti);
  NS_UNUSED (ccId);

  static NumerologyToType res =
  {
    {
      4, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 2, 4).Normalize () } },
    },
    {
      3, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 2, 3).Normalize () } },
    },
    {
      2, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 2, 2).Normalize () } },
    },
    {
      1, { { MmWaveControlMessage::RAR, SfnSf (1, 7, 0, 1).Normalize () } },
    },
    {
      0, { { MmWaveControlMessage::RAR, SfnSf (1, 8, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize() << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize(),
              "The message type " << TYPE_TO_STRING.at (msg->GetMessageType()) <<
              " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
              sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::EnbMacRx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  NS_UNUSED (rnti);
  NS_UNUSED (ccId);

  static NumerologyToType res =
  {
    {
      4, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 4).Normalize () } },
    },
    {
      3, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 3).Normalize () } },
    },
    {
      2, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 2).Normalize () } },
    },
    {
      1, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 1).Normalize () } },
    },
    {
      0, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 7, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize() << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize(),
              "The message type " << TYPE_TO_STRING.at (msg->GetMessageType()) <<
              " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
              sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " not found in the result map;" << std::endl;
    }
}

// UE

void
NrTimingsTest::UePhyTx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  NS_UNUSED (rnti);
  NS_UNUSED (ccId);

  static NumerologyToType res =
  {
    {
      4, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 4).Normalize () } },
    },
    {
      3, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 3).Normalize () } },
    },
    {
      2, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 2).Normalize () } },
    },
    {
      1, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 1).Normalize () } },
    },
    {
      0, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 7, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize() << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize(),
              "The message type " << TYPE_TO_STRING.at (msg->GetMessageType()) <<
              " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
              sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::UePhyRx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  NS_UNUSED (rnti);
  NS_UNUSED (ccId);

  static NumerologyToType res =
  {
    {
      4, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 5, 4).Normalize () } },
    },
    {
      3, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 5, 3).Normalize () } },
    },
    {
      2, { { MmWaveControlMessage::RAR, SfnSf (1, 7, 1, 2).Normalize () } },
    },
    {
      1, { { MmWaveControlMessage::RAR, SfnSf (1, 8, 1, 1).Normalize () } },
    },
    {
      0, { { MmWaveControlMessage::RAR, SfnSf (2, 1, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize() << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize(),
              "The message type " << TYPE_TO_STRING.at (msg->GetMessageType()) <<
              " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
              sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::UeMacTx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  NS_UNUSED (rnti);
  NS_UNUSED (ccId);

  static NumerologyToType res =
  {
    {
      4, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 4).Normalize () } },
    },
    {
      3, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 3).Normalize () } },
    },
    {
      2, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 2).Normalize () } },
    },
    {
      1, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 1).Normalize () } },
    },
    {
      0, { { MmWaveControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize() << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize(),
              "The message type " << TYPE_TO_STRING.at (msg->GetMessageType()) <<
              " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
              sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::UeMacRx (SfnSf sfn, uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  NS_UNUSED (rnti);
  NS_UNUSED (ccId);

  static NumerologyToType res =
  {
    {
      4, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 5, 4).Normalize () } },
    },
    {
      3, { { MmWaveControlMessage::RAR, SfnSf (1, 6, 5, 3).Normalize () } },
    },
    {
      2, { { MmWaveControlMessage::RAR, SfnSf (1, 7, 1, 2).Normalize () } },
    },
    {
      1, { { MmWaveControlMessage::RAR, SfnSf (1, 8, 1, 1).Normalize () } },
    },
    {
      0, { { MmWaveControlMessage::RAR, SfnSf (2, 1, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize() << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize(),
              "The message type " << TYPE_TO_STRING.at (msg->GetMessageType()) <<
              " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
              sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType())
                << " not found in the result map;" << std::endl;
    }
}

// Ugly pre-processor macro, to speed up writing. The best way would be to use
// static functions... so please forget them, and remember that they work
// only here in the DoRun function, as it is all hard-coded
#define GET_ENB_PHY(X, Y) mmWaveHelper->GetEnbPhy (enbNetDev.Get (X), Y)
#define GET_ENB_MAC(X, Y) mmWaveHelper->GetEnbMac (enbNetDev.Get (X), Y)

#define GET_UE_PHY(X, Y) mmWaveHelper->GetUePhy (ueNetDev.Get (X), Y)
#define GET_UE_MAC(X, Y) mmWaveHelper->GetUeMac (ueNetDev.Get (X), Y)

void
NrTimingsTest::DoRun (void)
{
  ns3::SeedManager::SetRun(5);

  Ptr<Node> ueNode = CreateObject<Node> ();
  Ptr<Node> gNbNode = CreateObject<Node> ();

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (gNbNode);
  mobility.Install (ueNode);
  gNbNode->GetObject<MobilityModel>()->SetPosition (Vector(0.0, 0.0, 10));
  ueNode->GetObject<MobilityModel> ()->SetPosition (Vector (0, 10 , 1.5));


  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();

  // Put the pointers inside mmWaveHelper
  mmWaveHelper->SetIdealBeamformingHelper (idealBeamformingHelper);
  mmWaveHelper->SetEpcHelper (epcHelper);

  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;  // in this example, both bands have a single CC

  CcBwpCreator::SimpleOperationBandConf bandConf1 (28e9, 100e6, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);

  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);


  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds(0)));
  mmWaveHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  mmWaveHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  mmWaveHelper->InitializeOperationBand (&band1);

  allBwps = CcBwpCreator::GetAllBwps ({band1});
  // Beamforming method
  idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for all the UEs
  mmWaveHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  mmWaveHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  mmWaveHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (true));

  // Antennas for all the gNbs
  mmWaveHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  mmWaveHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  mmWaveHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (true));

  NetDeviceContainer enbNetDev = mmWaveHelper->InstallGnbDevice (gNbNode, allBwps);
  NetDeviceContainer ueNetDev = mmWaveHelper->InstallUeDevice (ueNode, allBwps);

  mmWaveHelper->GetEnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (m_numerology));

  GET_ENB_PHY(0,0)->TraceConnectWithoutContext ("EnbPhyTxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::EnbPhyTx, this));
  GET_ENB_PHY(0,0)->TraceConnectWithoutContext ("EnbPhyRxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::EnbPhyRx, this));

  GET_ENB_MAC(0,0)->TraceConnectWithoutContext ("EnbMacTxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::EnbMacTx, this));
  GET_ENB_MAC(0,0)->TraceConnectWithoutContext ("EnbMacRxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::EnbMacRx, this));

  GET_UE_PHY(0,0)->TraceConnectWithoutContext ("UePhyTxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::UePhyTx, this));
  GET_UE_PHY(0,0)->TraceConnectWithoutContext ("UePhyRxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::UePhyRx, this));

  GET_UE_MAC(0,0)->TraceConnectWithoutContext ("UeMacTxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::UeMacTx, this));
  GET_UE_MAC(0,0)->TraceConnectWithoutContext ("UeMacRxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::UeMacRx, this));

  // When all the configuration is done, explicitly call UpdateConfig ()

  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
    {
      DynamicCast<MmWaveEnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<MmWaveUeNetDevice> (*it)->UpdateConfig ();
    }

  InternetStackHelper internet;
  internet.Install (ueNode);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // DL at 0.4
  Simulator::Schedule (MilliSeconds (400), &SendPacket, enbNetDev.Get(0), ueNetDev.Get(0)->GetAddress ());

  // UL at 0.8
  Simulator::Schedule (MilliSeconds (800), &SendPacket, ueNetDev.Get(0), enbNetDev.Get(0)->GetAddress ());

  Simulator::Stop (MilliSeconds (1200));

  if (verbose)
    {
      std::cerr << "Executing test for numerology " << m_numerology << std::endl;
    }
  Simulator::Run ();
  Simulator::Destroy ();
}


class NrTimingsTestSuite : public TestSuite
{
public:
  NrTimingsTestSuite ();
};

NrTimingsTestSuite::NrTimingsTestSuite ()
  : TestSuite ("test-timings", SYSTEM)
{
  AddTestCase (new NrTimingsTest ("num=4", 4, false), TestCase::QUICK);
  AddTestCase (new NrTimingsTest ("num=3", 3, false), TestCase::QUICK);
  AddTestCase (new NrTimingsTest ("num=2", 2, false), TestCase::QUICK);
  AddTestCase (new NrTimingsTest ("num=1", 1, false), TestCase::QUICK);
  AddTestCase (new NrTimingsTest ("num=0", 0, false), TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NrTimingsTestSuite nrTimingsTestSuite;

