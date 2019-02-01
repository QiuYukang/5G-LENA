/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2017 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *
 *   Author: Biljana Bojovic <bbojovic@cttc.es>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include <ns3/buildings-helper.h>
#include "ns3/log.h"
#include <ns3/buildings-module.h>
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/eps-bearer-tag.h"

using namespace ns3;

/**
 * \ingroup examples
 * \file cttc-3gpp-channel-simple-ran.cc
 * \brief Simple RAN
 *
 * This example describes how to setup a simulation using the 3GPP channel model
 * from TR 38.900. This example consists of a simple topology of 1 UE and 1 gNb,
 * and only NR RAN part is simulated. A packet is created and directly sent to
 * gNb device by SendPacket function. Then several functions are connected to
 * PDCP and RLC traces and the delay is printed.
 */


/**
 * \brief Global variable used to configure the numerology. It is accessible as "--numerology" from CommandLine.
 */
static ns3::GlobalValue g_numerology ("numerology",
                                      "The default 3GPP NR numerology to be used",
                                      ns3::UintegerValue (0),
                                      ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the bandwidth for packet size. This value is expressed in bytes. It is accessible as "--packetSize" from CommandLine.
 */

static ns3::GlobalValue g_udpInterval ("packetSize",
                                      "packet size in bytes",
                                      ns3::UintegerValue (1000),
                                      ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global boolean variable used to configure whether the UE performs the uplink traffic. It is accessible as "--isUplink" from CommandLine.
 */
static ns3::GlobalValue g_isUplink ("isUplink",
                                 "whether to perform uplink",
                                 ns3::BooleanValue (false),
                                 ns3::MakeBooleanChecker());


/**
 * Function creates a single packet and directly calls the function send
 * of a device to send the packet to the destination address.
 * @param device Device that will send the packet to the destination address.
 * @param addr Destination address for a packet.
 */
static void SendPacket (Ptr<NetDevice> device, Address& addr)
{
  UintegerValue uintegerValue;
  GlobalValue::GetValueByName("packetSize", uintegerValue); // use optional NLOS equation
  uint16_t packetSize = uintegerValue.Get();

  Ptr<Packet> pkt = Create<Packet> (packetSize);
  Ipv4Header ipv4Header;
  ipv4Header.SetProtocol(UdpL4Protocol::PROT_NUMBER);
  pkt->AddHeader(ipv4Header);
  EpsBearerTag tag (1, 1);
  pkt->AddPacketTag (tag);
  device->Send (pkt, addr, Ipv4L3Protocol::PROT_NUMBER);
}

/**
 * Function that prints out PDCP delay. This function is designed as a callback
 * for PDCP trace source.
 * @param path The path that matches the trace source
 * @param rnti RNTI of UE
 * @param lcid logical channel id
 * @param bytes PDCP PDU size in bytes
 * @param pdcpDelay PDCP delay
 */
void
RxPdcpPDU (std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t pdcpDelay)
{
  std::cout<<"\n Packet PDCP delay:"<<pdcpDelay<<"\n";
}

/**
 * Function that prints out RLC statistics, such as RNTI, lcId, RLC PDU size,
 * delay. This function is designed as a callback
 * for RLC trace source.
 * @param path The path that matches the trace source
 * @param rnti RNTI of UE
 * @param lcid logical channel id
 * @param bytes RLC PDU size in bytes
 * @param rlcDelay RLC PDU delay
 */
void
RxRlcPDU (std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t rlcDelay)
{
  std::cout<<"\n\n Data received at RLC layer at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti:"<<rnti<<std::endl;
  std::cout<<"\n lcid:"<<(unsigned)lcid<<std::endl;
  std::cout<<"\n bytes :"<< bytes<<std::endl;
  std::cout<<"\n delay :"<< rlcDelay<<std::endl;
}

/**
 * Function that connects PDCP and RLC traces to the corresponding trace sources.
 */
void
ConnectPdcpRlcTraces ()
{
  Config::Connect ("/NodeList/1/DeviceList/0/LteUeRrc/DataRadioBearerMap/1/LtePdcp/RxPDU",
                      MakeCallback (&RxPdcpPDU));

  Config::Connect ("/NodeList/1/DeviceList/0/LteUeRrc/DataRadioBearerMap/1/LteRlc/RxPDU",
                      MakeCallback (&RxRlcPDU));
}

/**
 * Function that connects UL PDCP and RLC traces to the corresponding trace sources.
 */
void
ConnectUlPdcpRlcTraces ()
{
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LtePdcp/RxPDU",
                      MakeCallback (&RxPdcpPDU));

  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LteRlc/RxPDU",
                      MakeCallback (&RxRlcPDU));
}

int 
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  // parse again so you can override input file default values via command line
  cmd.Parse (argc, argv);
  Time sendPacketTime = Seconds(0.4);

  UintegerValue uintegerValue;
  GlobalValue::GetValueByName("numerology", uintegerValue); // numerology to use
  uint16_t numerology = uintegerValue.Get();

  BooleanValue boolValue;
  GlobalValue::GetValueByName("isUplink", boolValue); // use uplink
  bool uplink = boolValue.Get();

  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Frequency", DoubleValue(28e9));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::CenterFreq", DoubleValue(28e9));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::Bandwidth", DoubleValue(400e6));
  Config::SetDefault ("ns3::MmWavePhyMacCommon::Numerology", UintegerValue(numerology));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(false));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("l"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon"));

  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::FixedMcsDl", BooleanValue (true));
  Config::SetDefault("ns3::MmWaveMacSchedulerNs3::McsDefaultDl", UintegerValue (28));

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

  if (uplink)
    Simulator::Schedule (sendPacketTime, &SendPacket, ueNetDev.Get(0), enbNetDev.Get(0)->GetAddress());
  else
    Simulator::Schedule (sendPacketTime, &SendPacket, enbNetDev.Get(0), ueNetDev.Get(0)->GetAddress());

  // attach UEs to the closest eNB
  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  if (uplink)
    {
      std::cout<<"\n Sending data in uplink."<<std::endl;
      Simulator::Schedule(Seconds(0.2), &ConnectUlPdcpRlcTraces);
    }
  else
    {
      std::cout<<"\n Sending data in downlink."<<std::endl;
      Simulator::Schedule(Seconds(0.2), &ConnectPdcpRlcTraces);
    }

  mmWaveHelper->EnableTraces();

  Simulator::Stop (Seconds (1));
  Simulator::Run ();
  Simulator::Destroy ();
}


