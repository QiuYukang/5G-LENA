/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

/**
 *
 * \file cttc-3gpp-channel-simple-fdm.cc
 * \ingroup examples
 * \brief Simple frequency division multiplexing example.
 *
 * This example describes how to setup a simple simulation with the frequency
 * division multiplexing. Simulation example allow configuration of the two
 * bandwidth parts where each is dedicated to different traffic type.
 * The topology is a simple topology that consists of 1 UE and 1 eNB. There
 * is one data bearer active and it will be multiplexed over a specific bandwidth
 * part depending on whether it is configured as low latency traffic.
 *
 * This example can be run from the command line in the following way:
 *
 * ./waf --run cttc-3gpp-channel-simple-fdm
 *
 * Bellow are described the global variables that are accessible through the
 * command line. E.g. the numerology of the BWP 1 can be configured by using
 * as --numerologyBwp1=4, so if use would like to specify this parameter
 * the program would be run in the following way:
 *
 * ./waf --run "cttc-3gpp-channel-simple-fdm --numerologyBwp1=3"
 *
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
 * \brief Global variable used to configure the numerology for BWP 1. It is accessible as "--numerologyBwp1" from CommandLine.
 */
static ns3::GlobalValue g_numerologyBwp1 ("numerologyBwp1",
                                          "The numerology to be used in bandwidth part 1",
                                           ns3::UintegerValue (4),
                                           ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the central system frequency for BWP 1. It is accessible as "--frequencyBwp1" from CommandLine.
 */
static ns3::GlobalValue g_frequencyBwp1 ("frequencyBwp1",
                                         "The system frequency to be used in bandwidth part 1",
                                          ns3::DoubleValue(28.1e9),
                                          ns3::MakeDoubleChecker<double>(6e9,100e9));

/**
 * \brief Global variable used to configure the bandwidth for BWP 1. This value is expressed in Hz.It is accessible as "--bandwidthBwp1" from CommandLine.
 */
static ns3::GlobalValue g_bandwidthBwp1 ("bandwidthBwp1",
                                        "The system bandwidth to be used in bandwidth part 1",
                                         ns3::DoubleValue(100e6),
                                         ns3::MakeDoubleChecker<double>());

/**
 * \brief Global variable used to configure the numerology for BWP 2. It is accessible as "--numerologyBwp2" from CommandLine.
 */
static ns3::GlobalValue g_numerologyBwp2 ("numerologyBwp2",
                                          "The numerology to be used in bandwidth part 2",
                                           ns3::UintegerValue (2),
                                           ns3::MakeUintegerChecker<uint32_t>());

/**
 * \brief Global variable used to configure the central system frequency for BWP 2. It is accessible as "--frequencyBwp2" from CommandLine.
 */
static ns3::GlobalValue g_frequencyBwp2 ("frequencyBwp2",
                                         "The system frequency to be used in bandwidth part 2",
                                          ns3::DoubleValue(28.1e9),
                                          ns3::MakeDoubleChecker<double>(6e9,100e9));

/**
 * \brief Global variable used to configure the bandwidth for BWP 2. This value is expressed in Hz.It is accessible as "--bandwidthBwp2" from CommandLine.
 */
static ns3::GlobalValue g_bandwidthBwp2 ("bandwidthBwp2",
                                         "The system bandwidth to be used in bandwidth part 2",
                                          ns3::DoubleValue(100e6),
                                          ns3::MakeDoubleChecker<double>());

/**
 * \brief Global variable used to configure the packet size. This value is expressed in bytes. It is accessible as "--packetSize" from CommandLine.
 */
static ns3::GlobalValue g_udpPacketSizeUll ("packetSize",
                                            "packet size in bytes",
                                            ns3::UintegerValue (1000),
                                            ns3::MakeUintegerChecker<uint32_t>());

/**
 * \related Global boolean variable used to configure whether the flow is a low latency. It is accessible as "--isUll" from CommandLine.
 */
static ns3::GlobalValue g_isUll ("isUll",
                                 "Whether the flow is a low latency type of traffic.",
                                 ns3::BooleanValue (true),
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
  std::cout<<"\n\n Data received by UE RLC at:"<<Simulator::Now()<<std::endl;
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
  DoubleValue doubleValue;
  GlobalValue::GetValueByName("numerologyBwp1", uintegerValue); // use optional NLOS equation
  uint16_t numerologyBwp1 = uintegerValue.Get();
  GlobalValue::GetValueByName("frequencyBwp1", doubleValue); //
  double frequencyBwp1 = doubleValue.Get();
  GlobalValue::GetValueByName("bandwidthBwp1", doubleValue); //
  double bandwidthBwp1 = doubleValue.Get();
  GlobalValue::GetValueByName("numerologyBwp2", uintegerValue); // use optional NLOS equation
  uint16_t numerologyBwp2 = uintegerValue.Get();
  GlobalValue::GetValueByName("frequencyBwp2", doubleValue); //
  double frequencyBwp2 = doubleValue.Get();
  GlobalValue::GetValueByName("bandwidthBwp2", doubleValue); //
  double bandwidthBwp2 = doubleValue.Get();
  BooleanValue boolValue;
  GlobalValue::GetValueByName("isUll", boolValue); //
  double isUll = boolValue.Get();

  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Frequency", DoubleValue(28e9));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Shadowing", BooleanValue(false));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::ChannelCondition", StringValue("l"));
  Config::SetDefault ("ns3::MmWave3gppPropagationLossModel::Scenario", StringValue("UMi-StreetCanyon"));
  Config::SetDefault ("ns3::MmWaveHelper::NumberOfComponentCarriers", UintegerValue (2));
  Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::NGBR_LOW_LAT_EMBB", UintegerValue (0));
  Config::SetDefault ("ns3::BwpManagerAlgorithmStatic::GBR_CONV_VOICE", UintegerValue (1));
  Config::SetDefault ("ns3::MmWaveHelper::EnbComponentCarrierManager", StringValue ("ns3::BwpManagerGnb"));
  Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));
  Config::SetDefault ("ns3::MmWaveEnbPhy::TxPower", DoubleValue(4));

  Ptr<MmWaveHelper> mmWaveHelper = CreateObject<MmWaveHelper> ();
  mmWaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::MmWave3gppPropagationLossModel"));
  mmWaveHelper->SetAttribute ("ChannelModel", StringValue ("ns3::MmWave3gppChannel"));
  Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  mmWaveHelper->SetEpcHelper (epcHelper);

  Ptr<BandwidthPartsPhyMacConf> bwpConf = CreateObject <BandwidthPartsPhyMacConf> ();

  Ptr<MmWavePhyMacCommon> phyMacCommonBwp1 = CreateObject<MmWavePhyMacCommon>();
  phyMacCommonBwp1->SetCentreFrequency(frequencyBwp1);
  phyMacCommonBwp1->SetBandwidth (bandwidthBwp1);
  phyMacCommonBwp1->SetNumerology(numerologyBwp1);
  phyMacCommonBwp1->SetCcId(0);
  bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp1);

  Ptr<MmWavePhyMacCommon> phyMacCommonBwp2 = CreateObject<MmWavePhyMacCommon>();
  phyMacCommonBwp2->SetCentreFrequency(frequencyBwp2);
  phyMacCommonBwp2->SetBandwidth (bandwidthBwp2);
  phyMacCommonBwp2->SetNumerology(numerologyBwp2);
  phyMacCommonBwp2->SetCcId(1);
  bwpConf->AddBandwidthPartPhyMacConf(phyMacCommonBwp2);

  mmWaveHelper->SetBandwidthPartMap (bwpConf);

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

  Simulator::Schedule (sendPacketTime, &SendPacket, enbNetDev.Get(0), ueNetDev.Get(0)->GetAddress());

  // attach UEs to the closest eNB
  mmWaveHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  enum EpsBearer::Qci q;

  if (isUll)
    {
      q = EpsBearer::NGBR_LOW_LAT_EMBB;
    }
  else
    {
      q = EpsBearer::GBR_CONV_VOICE;
    }

  EpsBearer bearer (q);
  mmWaveHelper->ActivateDataRadioBearer (ueNetDev, bearer);

  Simulator::Schedule(Seconds(0.2), &ConnectPdcpRlcTraces);

  mmWaveHelper->EnableTraces();

  Simulator::Stop (Seconds (1));
  Simulator::Run ();
  Simulator::Destroy ();
}


