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

#include "mmwave-helper.h"
#include <ns3/lte-rrc-protocol-ideal.h>
#include <ns3/lte-rrc-protocol-real.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/epc-ue-nas.h>
#include <ns3/names.h>
#include <ns3/mmwave-rrc-protocol-ideal.h>
#include <ns3/mmwave-enb-mac.h>
#include <ns3/mmwave-enb-phy.h>
#include <ns3/mmwave-ue-phy.h>
#include <ns3/mmwave-ue-mac.h>
#include <ns3/mmwave-enb-net-device.h>
#include <ns3/mmwave-ue-net-device.h>
#include <ns3/nr-ch-access-manager.h>
#include <ns3/bandwidth-part-gnb.h>
#include <ns3/bwp-manager-gnb.h>
#include <ns3/bwp-manager-ue.h>
#include <ns3/mmwave-rrc-protocol-ideal.h>
#include <ns3/epc-helper.h>
#include <ns3/epc-enb-application.h>
#include <ns3/epc-x2.h>
#include <ns3/mmwave-phy-rx-trace.h>
#include <ns3/mmwave-mac-rx-trace.h>
#include <ns3/mmwave-bearer-stats-calculator.h>
#include <ns3/bandwidth-part-ue.h>
#include <ns3/beam-manager.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/mmwave-mac-scheduler-tdma-rr.h>

#include <algorithm>

namespace ns3 {

/* ... */
NS_LOG_COMPONENT_DEFINE ("MmWaveHelper");

NS_OBJECT_ENSURE_REGISTERED (MmWaveHelper);

MmWaveHelper::MmWaveHelper (void)
  : m_imsiCounter (0),
  m_cellIdCounter {1},
  m_harqEnabled (false),
  m_snrTest (false)
{
  NS_LOG_FUNCTION (this);
  m_channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());
  m_enbNetDeviceFactory.SetTypeId (MmWaveEnbNetDevice::GetTypeId ());
  m_ueNetDeviceFactory.SetTypeId (MmWaveUeNetDevice::GetTypeId ());
  m_ueMacFactory.SetTypeId (MmWaveUeMac::GetTypeId ());
  m_gnbMacFactory.SetTypeId (MmWaveEnbMac::GetTypeId ());
  m_ueSpectrumFactory.SetTypeId (MmWaveSpectrumPhy::GetTypeId ());
  m_gnbSpectrumFactory.SetTypeId (MmWaveSpectrumPhy::GetTypeId ());
  m_uePhyFactory.SetTypeId (MmWaveUePhy::GetTypeId ());
  m_gnbPhyFactory.SetTypeId (MmWaveEnbPhy::GetTypeId ());
  m_ueChannelAccessManagerFactory.SetTypeId (NrAlwaysOnAccessManager::GetTypeId ());
  m_gnbChannelAccessManagerFactory.SetTypeId (NrAlwaysOnAccessManager::GetTypeId ());
  m_schedFactory.SetTypeId (MmWaveMacSchedulerTdmaRR::GetTypeId ());
  m_phyMacCommonFactory.SetTypeId (MmWavePhyMacCommon::GetTypeId ());
  m_ueAntennaFactory.SetTypeId (ThreeGppAntennaArrayModel::GetTypeId ());
  m_gnbAntennaFactory.SetTypeId (ThreeGppAntennaArrayModel::GetTypeId ());

  Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));

  m_phyStats = CreateObject<MmWavePhyRxTrace> ();
}

MmWaveHelper::~MmWaveHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId
MmWaveHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::MmWaveHelper")
    .SetParent<Object> ()
    .AddConstructor<MmWaveHelper> ()
    .AddAttribute ("HarqEnabled",
                   "Enable Hybrid ARQ",
                   BooleanValue (true),
                   MakeBooleanAccessor (&MmWaveHelper::m_harqEnabled),
                   MakeBooleanChecker ())
    ;
  return tid;
}

typedef std::function<void (ObjectFactory *, ObjectFactory *)> InitPathLossFn;

static void
InitRma (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppRmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppRmaChannelConditionModel::GetTypeId ());
}

static void
InitUma (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmaPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppUmaChannelConditionModel::GetTypeId ());
}

static void
InitUmi (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppUmiStreetCanyonPropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppUmiStreetCanyonChannelConditionModel::GetTypeId ());
}

static void
InitIndoorOpen (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppIndoorOpenOfficeChannelConditionModel::GetTypeId ());
}

static void
InitIndoorMixed (ObjectFactory *pathlossModelFactory, ObjectFactory *channelConditionModelFactory)
{
  pathlossModelFactory->SetTypeId (ThreeGppIndoorOfficePropagationLossModel::GetTypeId ());
  channelConditionModelFactory->SetTypeId (ThreeGppIndoorMixedOfficeChannelConditionModel::GetTypeId ());
}

void
MmWaveHelper::InitializeOperationBand (OperationBandInfo *band) const
{
  NS_LOG_FUNCTION (this);

  static std::unordered_map<BandwidthPartInfo::Scenario, InitPathLossFn> initLookupTable
  {
    {BandwidthPartInfo::RMa, std::bind (&InitRma, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMa, std::bind (&InitUma, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::UMi_StreetCanyon, std::bind (&InitUmi, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::InH_OfficeOpen, std::bind (&InitIndoorOpen, std::placeholders::_1, std::placeholders::_2)},
    {BandwidthPartInfo::InH_OfficeMixed, std::bind (&InitIndoorMixed, std::placeholders::_1, std::placeholders::_2)},
  };

  ObjectFactory channelConditionModelFactory;
  ObjectFactory spectrumPropagationFactory;
  ObjectFactory pathlossModelFactory;
  ObjectFactory channelFactory;

  spectrumPropagationFactory.SetTypeId (ThreeGppSpectrumPropagationLossModel::GetTypeId ());
  channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());

  // Iterate over all CCs, and instantiate the channel and propagation model
  for (const auto & cc : band->m_cc)
    {
      for (const auto & bwp : cc->m_bwp)
        {
          if (! (bwp->m_channel == nullptr && bwp->m_propagation == nullptr && bwp->m_3gppChannel == nullptr))
            {
              continue;
            }

          // Initialize the type ID of the factories by calling the relevant
          // static function defined above and stored inside the lookup table
          initLookupTable.at (bwp->m_scenario) (&pathlossModelFactory, &channelConditionModelFactory);

          Ptr<ChannelConditionModel> channelConditionModel  = channelConditionModelFactory.Create<ChannelConditionModel>();

          bwp->m_propagation = pathlossModelFactory.Create <ThreeGppPropagationLossModel> ();
          bwp->m_propagation->SetAttributeFailSafe ("Frequency", DoubleValue (bwp->m_centralFrequency));
          bwp->m_propagation->SetChannelConditionModel (channelConditionModel);

          bwp->m_3gppChannel = spectrumPropagationFactory.Create<ThreeGppSpectrumPropagationLossModel>();
          bwp->m_3gppChannel->SetFrequency (bwp->m_centralFrequency);
          bwp->m_3gppChannel->SetScenario (bwp->GetScenario ());
          bwp->m_3gppChannel->SetChannelConditionModel (channelConditionModel);

          bwp->m_channel = m_channelFactory.Create<SpectrumChannel> ();
          bwp->m_channel->AddPropagationLossModel (bwp->m_propagation);
          bwp->m_channel->AddSpectrumPropagationLossModel (bwp->m_3gppChannel);
        }
    }
}

uint32_t
MmWaveHelper::GetNumberBwp (const Ptr<const NetDevice> &gnbDevice)
{
  NS_LOG_FUNCTION (gnbDevice);
  Ptr<const MmWaveEnbNetDevice> netDevice = DynamicCast<const MmWaveEnbNetDevice> (gnbDevice);
  if (netDevice == nullptr)
    {
      return 0;
    }
  return netDevice->GetCcMapSize ();
}

Ptr<MmWaveEnbPhy>
MmWaveHelper::GetEnbPhy (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex)
{
  NS_LOG_FUNCTION (gnbDevice << bwpIndex);
  NS_ASSERT(bwpIndex < UINT8_MAX);
  Ptr<MmWaveEnbNetDevice> netDevice = DynamicCast<MmWaveEnbNetDevice> (gnbDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }
  return netDevice->GetPhy (static_cast<uint8_t> (bwpIndex));
}

Ptr<MmWaveEnbMac>
MmWaveHelper::GetEnbMac (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex)
{
  NS_LOG_FUNCTION (gnbDevice << bwpIndex);
  NS_ASSERT(bwpIndex < UINT8_MAX);
  Ptr<MmWaveEnbNetDevice> netDevice = DynamicCast<MmWaveEnbNetDevice> (gnbDevice);
  if (netDevice == nullptr)
    {
      return nullptr;
    }
  return netDevice->GetMac (static_cast<uint8_t> (bwpIndex));
}

void
MmWaveHelper::SetHarqEnabled (bool harqEnabled)
{
  m_harqEnabled = harqEnabled;
}

bool
MmWaveHelper::GetHarqEnabled ()
{
  return m_harqEnabled;
}

void
MmWaveHelper::SetSnrTest (bool snrTest)
{
  m_snrTest = snrTest;
}

bool
MmWaveHelper::GetSnrTest ()
{
  return m_snrTest;
}

NetDeviceContainer
MmWaveHelper::InstallUeDevice (const NodeContainer &c,
                               const std::vector<std::reference_wrapper<BandwidthPartInfoPtr> > &allBwps)
{
  NS_LOG_FUNCTION (this);
  Initialize ();    // Run DoInitialize (), if necessary
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleUeDevice (node, allBwps);
      device->SetAddress (Mac48Address::Allocate ());
      devices.Add (device);
    }
  return devices;

}

NetDeviceContainer
MmWaveHelper::InstallGnbDevice (const NodeContainer & c,
                                const std::vector<std::reference_wrapper<BandwidthPartInfoPtr> > allBwps)
{
  NS_LOG_FUNCTION (this);
  Initialize ();    // Run DoInitialize (), if necessary
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleGnbDevice (node, allBwps);
      device->SetAddress (Mac48Address::Allocate ());
      devices.Add (device);
    }
  return devices;
}

Ptr<MmWaveUeMac>
MmWaveHelper::CreateUeMac () const
{
  NS_LOG_FUNCTION (this);
  Ptr<MmWaveUeMac> mac = m_ueMacFactory.Create <MmWaveUeMac> ();
  return mac;
}

Ptr<MmWaveUePhy>
MmWaveHelper::CreateUePhy (const Ptr<Node> &n, const Ptr<SpectrumChannel> &c,
                           const Ptr<ThreeGppSpectrumPropagationLossModel> &gppChannel,
                           const Ptr<MmWaveUeNetDevice> &dev,
                           const MmWaveSpectrumPhy::MmWavePhyDlHarqFeedbackCallback &dlHarqCallback,
                           const MmWaveSpectrumPhy::MmWavePhyRxCtrlEndOkCallback &phyRxCtrlCallback)
{
  NS_LOG_FUNCTION (this);

  Ptr<MmWaveSpectrumPhy> channelPhy = m_ueSpectrumFactory.Create <MmWaveSpectrumPhy> ();
  Ptr<MmWaveUePhy> phy = m_uePhyFactory.Create <MmWaveUePhy> ();
  Ptr<MmWaveHarqPhy> harq = Create<MmWaveHarqPhy> ();

  Ptr<ThreeGppAntennaArrayModel> antenna = m_ueAntennaFactory.Create <ThreeGppAntennaArrayModel> ();

  phy->InstallCentralFrequency (gppChannel->GetFrequency ());
  phy->InstallSpectrumPhy (channelPhy);
  phy->InstallAntenna (antenna);
  phy->StartEventLoop (n->GetId (), SfnSf (0, 0, 0, 0));

  Ptr<NrChAccessManager> cam = DynamicCast<NrChAccessManager> (m_ueChannelAccessManagerFactory.Create ());
  cam->SetNrSpectrumPhy (channelPhy);
  phy->SetCam (cam);

  channelPhy->SetHarqPhyModule (harq);

  Ptr<mmWaveChunkProcessor> pData = Create<mmWaveChunkProcessor> ();
  pData->AddCallback (MakeCallback (&MmWaveUePhy::GenerateDlCqiReport, phy));
  pData->AddCallback (MakeCallback (&MmWaveSpectrumPhy::UpdateSinrPerceived, channelPhy));
  channelPhy->AddDataSinrChunkProcessor (pData);

  if (m_harqEnabled)
    {
      channelPhy->SetPhyDlHarqFeedbackCallback (dlHarqCallback);
    }

  channelPhy->SetChannel (c);

  Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
  NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling MmWaveHelper::InstallUeDevice ()");
  channelPhy->SetMobility (mm);

  channelPhy->SetPhyRxDataEndOkCallback (MakeCallback (&MmWaveUePhy::PhyDataPacketReceived, phy));
  channelPhy->SetPhyRxCtrlEndOkCallback (phyRxCtrlCallback);

  // TODO: If antenna changes, we are fucked!
  gppChannel->AddDevice (dev,  phy->GetSpectrumPhy()->GetAntennaArray());

  return phy;
}

Ptr<NetDevice>
MmWaveHelper::InstallSingleUeDevice (const Ptr<Node> &n,
                                     const std::vector<std::reference_wrapper<BandwidthPartInfoPtr> > allBwps)
{
  NS_LOG_FUNCTION (this);

  Ptr<MmWaveUeNetDevice> dev = m_ueNetDeviceFactory.Create<MmWaveUeNetDevice> ();
  std::map<uint8_t, Ptr<BandwidthPartUe> > ueCcMap;

  // Create, for each ue, its component carriers
  for (uint32_t ccId = 0; ccId < allBwps.size (); ++ccId)
    {
      Ptr <BandwidthPartUe> cc =  CreateObject<BandwidthPartUe> ();
      cc->SetUlBandwidth (allBwps[ccId].get()->m_channelBandwidth);
      cc->SetDlBandwidth (allBwps[ccId].get()->m_channelBandwidth);
      cc->SetDlEarfcn (0); // Used for nothing..
      cc->SetUlEarfcn (0); // Used for nothing..

      if (ccId == 0)
        {
          cc->SetAsPrimary (true);
        }
      else
        {
          cc->SetAsPrimary (false);
        }

      auto mac = CreateUeMac ();
      cc->SetMac (mac);

      auto phy = CreateUePhy (n, allBwps[ccId].get()->m_channel, allBwps[ccId].get ()->m_3gppChannel,
                              dev, MakeCallback (&MmWaveUeNetDevice::EnqueueDlHarqFeedback, dev),
                              std::bind (&MmWaveUeNetDevice::RouteIngoingCtrlMsgs, dev,
                                         std::placeholders::_1, ccId));
      phy->SetDevice (dev);
      phy->GetSpectrumPhy ()->SetDevice (dev);
      cc->SetPhy (phy);

      ueCcMap.insert (std::make_pair (ccId, cc));
    }

  Ptr<LteUeComponentCarrierManager> ccmUe = DynamicCast<LteUeComponentCarrierManager> (CreateObject <BwpManagerUe> ());

  Ptr<LteUeRrc> rrc = CreateObject<LteUeRrc> ();
  rrc->m_numberOfComponentCarriers = ueCcMap.size ();
  // run intializeSap to create the proper number of sap provider/users
  rrc->InitializeSap ();
  rrc->SetLteMacSapProvider (ccmUe->GetLteMacSapProvider ());
  // setting ComponentCarrierManager SAP
  rrc->SetLteCcmRrcSapProvider (ccmUe->GetLteCcmRrcSapProvider ());
  ccmUe->SetLteCcmRrcSapUser (rrc->GetLteCcmRrcSapUser ());
  ccmUe->SetNumberOfComponentCarriers (ueCcMap.size ());

  bool useIdealRrc = true;
  if (useIdealRrc)
    {
      Ptr<mmWaveUeRrcProtocolIdeal> rrcProtocol = CreateObject<mmWaveUeRrcProtocolIdeal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetLteUeRrcSapProvider (rrc->GetLteUeRrcSapProvider ());
      rrc->SetLteUeRrcSapUser (rrcProtocol->GetLteUeRrcSapUser ());
    }
  else
    {
      Ptr<LteUeRrcProtocolReal> rrcProtocol = CreateObject<LteUeRrcProtocolReal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetLteUeRrcSapProvider (rrc->GetLteUeRrcSapProvider ());
      rrc->SetLteUeRrcSapUser (rrcProtocol->GetLteUeRrcSapUser ());
    }

  if (m_epcHelper != nullptr)
    {
      rrc->SetUseRlcSm (false);
    }
  else
    {
      rrc->SetUseRlcSm (true);
    }
  Ptr<EpcUeNas> nas = CreateObject<EpcUeNas> ();

  nas->SetAsSapProvider (rrc->GetAsSapProvider ());
  nas->SetDevice (dev);
  nas->SetForwardUpCallback (MakeCallback (&MmWaveUeNetDevice::Receive, dev));

  rrc->SetAsSapUser (nas->GetAsSapUser ());

  for (auto it = ueCcMap.begin (); it != ueCcMap.end (); ++it)
    {
      rrc->SetLteUeCmacSapProvider (it->second->GetMac ()->GetUeCmacSapProvider (), it->first);
      it->second->GetMac ()->SetUeCmacSapUser (rrc->GetLteUeCmacSapUser (it->first));

      it->second->GetPhy ()->SetUeCphySapUser (rrc->GetLteUeCphySapUser ());
      rrc->SetLteUeCphySapProvider (it->second->GetPhy ()->GetUeCphySapProvider (), it->first);

      it->second->GetPhy ()->SetPhySapUser (it->second->GetMac ()->GetPhySapUser ());
      it->second->GetMac ()->SetPhySapProvider (it->second->GetPhy ()->GetPhySapProvider ());

      bool ccmTest = ccmUe->SetComponentCarrierMacSapProviders (it->first,
                                                                it->second->GetMac ()->GetUeMacSapProvider ());

      if (ccmTest == false)
        {
          NS_FATAL_ERROR ("Error in SetComponentCarrierMacSapProviders");
        }
    }

  NS_ABORT_MSG_IF (m_imsiCounter >= 0xFFFFFFFF, "max num UEs exceeded");
  uint64_t imsi = ++m_imsiCounter;

  dev->SetNode (n);
  dev->SetAttribute ("Imsi", UintegerValue (imsi));
  dev->SetCcMap (ueCcMap);
  dev->SetAttribute ("mmWaveUeRrc", PointerValue (rrc));
  dev->SetAttribute ("EpcUeNas", PointerValue (nas));
  dev->SetAttribute ("LteUeComponentCarrierManager", PointerValue (ccmUe));

  n->AddDevice (dev);


  if (m_epcHelper != nullptr)
    {
      m_epcHelper->AddUe (dev, dev->GetImsi ());
    }

  dev->Initialize ();

  return dev;
}

Ptr<MmWaveEnbPhy>
MmWaveHelper::CreateGnbPhy (const Ptr<Node> &n,
                            const Ptr<MmWavePhyMacCommon> &phyMacCommon,
                            const Ptr<SpectrumChannel> &c, const Ptr<ThreeGppSpectrumPropagationLossModel> &gppChannel,
                            const Ptr<MmWaveEnbNetDevice> &dev, uint16_t cellId,
                            const MmWaveSpectrumPhy::MmWavePhyRxCtrlEndOkCallback &phyEndCtrlCallback)
{
  NS_LOG_FUNCTION (this);

  Ptr<MmWaveSpectrumPhy> channelPhy = m_gnbSpectrumFactory.Create <MmWaveSpectrumPhy> ();
  Ptr<MmWaveEnbPhy> phy = m_gnbPhyFactory.Create <MmWaveEnbPhy> ();
  Ptr<ThreeGppAntennaArrayModel> antenna = m_gnbAntennaFactory.Create <ThreeGppAntennaArrayModel> ();

  phy->InstallCentralFrequency (gppChannel->GetFrequency ());
  phy->InstallSpectrumPhy (channelPhy);
  phy->InstallAntenna (antenna);

  phy->StartEventLoop (n->GetId (), SfnSf (0, 0, 0, 0));

  // PHY <--> CAM
  Ptr<NrChAccessManager> cam = DynamicCast<NrChAccessManager> (m_gnbChannelAccessManagerFactory.Create ());
  cam->SetNrSpectrumPhy (channelPhy);
  phy->SetCam (cam);

  Ptr<MmWaveHarqPhy> harq = Create<MmWaveHarqPhy> ();
  harq->SetHarqNum(phyMacCommon->GetNumHarqProcess ());
  channelPhy->SetHarqPhyModule (harq);

  Ptr<mmWaveChunkProcessor> pData = Create<mmWaveChunkProcessor> ();
  if (!m_snrTest)
    {
      pData->AddCallback (MakeCallback (&MmWaveEnbPhy::GenerateDataCqiReport, phy));
      pData->AddCallback (MakeCallback (&MmWaveSpectrumPhy::UpdateSinrPerceived, channelPhy));
    }
  channelPhy->AddDataSinrChunkProcessor (pData);

  phy->SetConfigurationParameters (phyMacCommon);
  phy->SetDevice (dev);

  channelPhy->SetChannel (c);

  Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
  NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling MmWaveHelper::InstallEnbDevice ()");
  channelPhy->SetMobility (mm);

  channelPhy->SetDevice (dev);
  channelPhy->SetCellId (cellId);
  channelPhy->SetPhyRxDataEndOkCallback (MakeCallback (&MmWaveEnbPhy::PhyDataPacketReceived, phy));
  channelPhy->SetPhyRxCtrlEndOkCallback (phyEndCtrlCallback);
  channelPhy->SetPhyUlHarqFeedbackCallback (MakeCallback (&MmWaveEnbPhy::ReportUlHarqFeedback, phy));

  phy->Initialize ();

  c->AddRx (channelPhy);
  // TODO: NOTE: if changing the Antenna Array, this will broke
  gppChannel->AddDevice (dev, phy->GetSpectrumPhy()->GetAntennaArray());

  return phy;
}

Ptr<MmWaveEnbMac>
MmWaveHelper::CreateGnbMac (const Ptr<MmWavePhyMacCommon>& conf)
{
  NS_LOG_FUNCTION (this);

  Ptr<MmWaveEnbMac> mac = m_gnbMacFactory.Create <MmWaveEnbMac> ();
  mac->SetConfigurationParameters (conf);
  return mac;
}

Ptr<MmWaveMacScheduler>
MmWaveHelper::CreateGnbSched (const Ptr<MmWavePhyMacCommon>& conf)
{
  NS_LOG_FUNCTION (this);

  Ptr<MmWaveMacScheduler> sched = m_schedFactory.Create <MmWaveMacScheduler> ();
  sched->ConfigureCommonParameters (conf);
  return sched;
}

Ptr<NetDevice>
MmWaveHelper::InstallSingleGnbDevice (const Ptr<Node> &n,
                                      const std::vector<std::reference_wrapper<BandwidthPartInfoPtr> > allBwps)
{
  NS_ABORT_MSG_IF (m_cellIdCounter == 65535, "max num eNBs exceeded");

  Ptr<MmWaveEnbNetDevice> dev = m_enbNetDeviceFactory.Create<MmWaveEnbNetDevice> ();
  Ptr<MmWavePhyMacCommon> phyMacCommon = m_phyMacCommonFactory.Create <MmWavePhyMacCommon> ();

  // create component carrier map for this eNb device
  std::map<uint8_t,Ptr<BandwidthPartGnb> > ccMap;

  for (uint32_t ccId = 0; ccId < allBwps.size (); ++ccId)
    {
      Ptr <BandwidthPartGnb> cc =  CreateObject<BandwidthPartGnb> ();
      cc->SetUlBandwidth (allBwps[ccId].get()->m_channelBandwidth);
      cc->SetDlBandwidth (allBwps[ccId].get()->m_channelBandwidth);
      cc->SetDlEarfcn (0); // Argh... handover not working
      cc->SetUlEarfcn (0); // Argh... handover not working
      cc->SetCellId (m_cellIdCounter++);

      if (ccId == 0)
        {
          cc->SetAsPrimary (true);
        }
      else
        {
          cc->SetAsPrimary (false);
        }

      auto phy = CreateGnbPhy (n, phyMacCommon, allBwps[ccId].get()->m_channel,
                               allBwps[ccId].get()->m_3gppChannel, dev, cc->GetCellId (),
                               std::bind (&MmWaveEnbNetDevice::RouteIngoingCtrlMsgs,
                                          dev, std::placeholders::_1, ccId));
      cc->SetPhy (phy);

      auto mac = CreateGnbMac (phyMacCommon);
      cc->SetMac (mac);
      phy->GetCam ()->SetNrEnbMac (mac);

      auto sched = CreateGnbSched (phyMacCommon);
      cc->SetMmWaveMacScheduler (sched);

      ccMap.insert (std::make_pair (ccId, cc));
    }

  Ptr<LteEnbRrc> rrc = CreateObject<LteEnbRrc> ();
  Ptr<LteEnbComponentCarrierManager> ccmEnbManager = DynamicCast<LteEnbComponentCarrierManager> (CreateObject<BwpManagerGnb> ());

  // Convert Enb carrier map to only PhyConf map
  // we want to make RRC to be generic, to be able to work with any type of carriers, not only strictly LTE carriers
  std::map < uint8_t, Ptr<ComponentCarrierBaseStation> > ccPhyConfMap;
  for (const auto &i : ccMap)
    {
      Ptr<ComponentCarrierBaseStation> c = i.second;
      ccPhyConfMap.insert (std::make_pair (i.first,c));
    }

  //ComponentCarrierManager SAP
  rrc->SetLteCcmRrcSapProvider (ccmEnbManager->GetLteCcmRrcSapProvider ());
  ccmEnbManager->SetLteCcmRrcSapUser (rrc->GetLteCcmRrcSapUser ());
  // Set number of component carriers. Note: eNB CCM would also set the
  // number of component carriers in eNB RRC

  ccmEnbManager->SetNumberOfComponentCarriers (ccMap.size ());
  rrc->ConfigureCarriers (ccPhyConfMap);

  //mmwave module currently uses only RRC ideal mode
  bool useIdealRrc = true;

  if (useIdealRrc)
    {
      Ptr<MmWaveEnbRrcProtocolIdeal> rrcProtocol = CreateObject<MmWaveEnbRrcProtocolIdeal> ();
      rrcProtocol->SetLteEnbRrcSapProvider (rrc->GetLteEnbRrcSapProvider ());
      rrc->SetLteEnbRrcSapUser (rrcProtocol->GetLteEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
    }
  else
    {
      Ptr<LteEnbRrcProtocolReal> rrcProtocol = CreateObject<LteEnbRrcProtocolReal> ();
      rrcProtocol->SetLteEnbRrcSapProvider (rrc->GetLteEnbRrcSapProvider ());
      rrc->SetLteEnbRrcSapUser (rrcProtocol->GetLteEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
    }

  if (m_epcHelper != nullptr)
    {
      EnumValue epsBearerToRlcMapping;
      rrc->GetAttribute ("EpsBearerToRlcMapping", epsBearerToRlcMapping);
      // it does not make sense to use RLC/SM when also using the EPC
      if (epsBearerToRlcMapping.Get () == LteEnbRrc::RLC_SM_ALWAYS)
        {
          rrc->SetAttribute ("EpsBearerToRlcMapping", EnumValue (LteEnbRrc::RLC_UM_ALWAYS));
        }
    }

  // This RRC attribute is used to connect each new RLC instance with the MAC layer
  // (for function such as TransmitPdu, ReportBufferStatusReport).
  // Since in this new architecture, the component carrier manager acts a proxy, it
  // will have its own LteMacSapProvider interface, RLC will see it as through original MAC
  // interface LteMacSapProvider, but the function call will go now through LteEnbComponentCarrierManager
  // instance that needs to implement functions of this interface, and its task will be to
  // forward these calls to the specific MAC of some of the instances of component carriers. This
  // decision will depend on the specific implementation of the component carrier manager.
  rrc->SetLteMacSapProvider (ccmEnbManager->GetLteMacSapProvider ());
  rrc->SetForwardUpCallback (MakeCallback (&MmWaveEnbNetDevice::Receive, dev));

  for (auto it = ccMap.begin (); it != ccMap.end (); ++it)
    {
      it->second->GetPhy ()->SetEnbCphySapUser (rrc->GetLteEnbCphySapUser (it->first));
      rrc->SetLteEnbCphySapProvider (it->second->GetPhy ()->GetEnbCphySapProvider (), it->first);

      rrc->SetLteEnbCmacSapProvider (it->second->GetMac ()->GetEnbCmacSapProvider (),it->first );
      it->second->GetMac ()->SetEnbCmacSapUser (rrc->GetLteEnbCmacSapUser (it->first));

      // PHY <--> MAC SAP
      it->second->GetPhy ()->SetPhySapUser (it->second->GetMac ()->GetPhySapUser ());
      it->second->GetMac ()->SetPhySapProvider (it->second->GetPhy ()->GetPhySapProvider ());
      // PHY <--> MAC SAP END

      //Scheduler SAP
      it->second->GetMac ()->SetMmWaveMacSchedSapProvider (it->second->GetMmWaveMacScheduler ()->GetMacSchedSapProvider ());
      it->second->GetMac ()->SetMmWaveMacCschedSapProvider (it->second->GetMmWaveMacScheduler ()->GetMacCschedSapProvider ());

      it->second->GetMmWaveMacScheduler ()->SetMacSchedSapUser (it->second->GetMac ()->GetMmWaveMacSchedSapUser ());
      it->second->GetMmWaveMacScheduler ()->SetMacCschedSapUser (it->second->GetMac ()->GetMmWaveMacCschedSapUser ());
      // Scheduler SAP END

      it->second->GetMac ()->SetLteCcmMacSapUser (ccmEnbManager->GetLteCcmMacSapUser ());
      ccmEnbManager->SetCcmMacSapProviders (it->first, it->second->GetMac ()->GetLteCcmMacSapProvider ());

      // insert the pointer to the LteMacSapProvider interface of the MAC layer of the specific component carrier
      ccmEnbManager->SetMacSapProvider (it->first, it->second->GetMac ()->GetMacSapProvider ());
    }

  dev->SetNode (n);
  dev->SetAttribute ("LteEnbComponentCarrierManager", PointerValue (ccmEnbManager));
  dev->SetCcMap (ccMap);
  dev->SetAttribute ("LteEnbRrc", PointerValue (rrc));
  dev->Initialize ();

  n->AddDevice (dev);

  if (m_epcHelper != nullptr)
    {
      NS_LOG_INFO ("adding this eNB to the EPC");
      m_epcHelper->AddEnb (n, dev, dev->GetCellId ());
      Ptr<EpcEnbApplication> enbApp = n->GetApplication (0)->GetObject<EpcEnbApplication> ();
      NS_ASSERT_MSG (enbApp != nullptr, "cannot retrieve EpcEnbApplication");

      // S1 SAPs
      rrc->SetS1SapProvider (enbApp->GetS1SapProvider ());
      enbApp->SetS1SapUser (rrc->GetS1SapUser ());

      // X2 SAPs
      Ptr<EpcX2> x2 = n->GetObject<EpcX2> ();
      x2->SetEpcX2SapUser (rrc->GetEpcX2SapUser ());
      rrc->SetEpcX2SapProvider (x2->GetEpcX2SapProvider ());
    }

  return dev;
}

void
MmWaveHelper::AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);

  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); i++)
    {
      AttachToClosestEnb (*i, enbDevices);
    }
}

void
MmWaveHelper::AttachToClosestEnb (Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (enbDevices.GetN () > 0, "empty enb device container");
  Vector uepos = ueDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  double minDistance = std::numeric_limits<double>::infinity ();
  Ptr<NetDevice> closestEnbDevice;
  for (NetDeviceContainer::Iterator i = enbDevices.Begin (); i != enbDevices.End (); ++i)
    {
      Vector enbpos = (*i)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
      double distance = CalculateDistance (uepos, enbpos);
      if (distance < minDistance)
        {
          minDistance = distance;
          closestEnbDevice = *i;
        }
    }
  NS_ASSERT (closestEnbDevice != 0);

  AttachToEnb (ueDevice, closestEnbDevice);
}


void
MmWaveHelper::AttachToEnb (const Ptr<NetDevice> &ueDevice,
                           const Ptr<NetDevice> &gnbDevice)
{
  Ptr<MmWaveEnbNetDevice> enbNetDev = gnbDevice->GetObject<MmWaveEnbNetDevice> ();
  Ptr<MmWaveUeNetDevice> ueNetDev = ueDevice->GetObject<MmWaveUeNetDevice> ();

  NS_ABORT_IF (enbNetDev == nullptr || ueNetDev == nullptr);

  for (uint32_t i = 0; i < enbNetDev->GetCcMapSize (); ++i)
    {
      Ptr<MmWavePhyMacCommon> configParams = enbNetDev->GetPhy (i)->GetConfigurationParameters ();
      enbNetDev->GetPhy(i)->RegisterUe (ueNetDev->GetImsi (), ueNetDev);
      ueNetDev->GetPhy (i)->RegisterToEnb (enbNetDev->GetCellId (i), configParams);
      ueNetDev->GetPhy (i)->SetNumRbPerRbg (enbNetDev->GetMac(i)->GetNumRbPerRbg());
      ueNetDev->GetCcMap()[i]->GetMac()->SetConfigurationParameters (configParams);
      Ptr<EpcUeNas> ueNas = ueNetDev->GetNas ();
      ueNas->Connect (enbNetDev->GetCellId (i), enbNetDev->GetEarfcn (i));
    }

  if (m_epcHelper != nullptr)
    {
      // activate default EPS bearer
      m_epcHelper->ActivateEpsBearer (ueDevice, ueNetDev->GetImsi (), EpcTft::Default (), EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
    }

  // tricks needed for the simplified LTE-only simulations
  //if (m_epcHelper == 0)
  //{
  ueNetDev->SetTargetEnb (enbNetDev);
  //}

  if (m_idealBeamformingHelper != nullptr)
    {
      m_idealBeamformingHelper->AddBeamformingTask (enbNetDev, ueNetDev);
    }
}



uint8_t
MmWaveHelper::ActivateDedicatedEpsBearer (NetDeviceContainer ueDevices, EpsBearer bearer, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      uint8_t bearerId = ActivateDedicatedEpsBearer (*i, bearer, tft);
      return bearerId;
    }
  return 0;
}


uint8_t
MmWaveHelper::ActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_epcHelper != 0, "dedicated EPS bearers cannot be set up when the EPC is not used");

  uint64_t imsi = ueDevice->GetObject<MmWaveUeNetDevice> ()->GetImsi ();
  uint8_t bearerId = m_epcHelper->ActivateEpsBearer (ueDevice, imsi, tft, bearer);
  return bearerId;
}

void
MmWaveHelper::DeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice,Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);
  NS_ASSERT_MSG (m_epcHelper != nullptr, "Dedicated EPS bearers cannot be de-activated when the EPC is not used");
  NS_ASSERT_MSG (bearerId != 1, "Default bearer cannot be de-activated until and unless and UE is released");

  DoDeActivateDedicatedEpsBearer (ueDevice, enbDevice, bearerId);
}

void
MmWaveHelper::SetUeMacAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueMacFactory.Set (n, v);
}

void
MmWaveHelper::SetGnbMacAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbMacFactory.Set (n, v);
}

void
MmWaveHelper::SetGnbSpectrumAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbSpectrumFactory.Set (n, v);
}

void
MmWaveHelper::SetUeSpectrumAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueSpectrumFactory.Set (n, v);
}

void
MmWaveHelper::SetUeChannelAccessManagerAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueChannelAccessManagerFactory.Set (n, v);
}

void
MmWaveHelper::SetGnbChannelAccessManagerAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbChannelAccessManagerFactory.Set (n, v);
}

void
MmWaveHelper::SetSchedulerAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_schedFactory.Set (n, v);
}

void
MmWaveHelper::SetUePhyAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_uePhyFactory.Set (n, v);
}

void
MmWaveHelper::SetGnbPhyAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbPhyFactory.Set (n, v);
}

void
MmWaveHelper::SetMmWavePhyMacCommonAttribute(const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_phyMacCommonFactory.Set (n, v);
}

void
MmWaveHelper::SetUeAntennaAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueAntennaFactory.Set (n, v);
}

void
MmWaveHelper::SetGnbAntennaAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_gnbAntennaFactory.Set (n, v);
}

void
MmWaveHelper::SetUeChannelAccessManagerTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_ueChannelAccessManagerFactory.SetTypeId (typeId);
}

void
MmWaveHelper::SetGnbChannelAccessManagerTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_gnbChannelAccessManagerFactory.SetTypeId (typeId);
}

void
MmWaveHelper::SetSchedulerTypeId (const TypeId &typeId)
{
  NS_LOG_FUNCTION (this);
  m_schedFactory.SetTypeId (typeId);
}

void
MmWaveHelper::DoDeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);

  //Extract IMSI and rnti
  uint64_t imsi = ueDevice->GetObject<MmWaveUeNetDevice> ()->GetImsi ();
  uint16_t rnti = ueDevice->GetObject<MmWaveUeNetDevice> ()->GetRrc ()->GetRnti ();


  Ptr<LteEnbRrc> enbRrc = enbDevice->GetObject<MmWaveEnbNetDevice> ()->GetRrc ();

  enbRrc->DoSendReleaseDataRadioBearer (imsi,rnti,bearerId);
}


void
MmWaveHelper::SetEpcHelper (Ptr<EpcHelper> epcHelper)
{
  m_epcHelper = epcHelper;
}

void
MmWaveHelper::SetIdealBeamformingHelper (Ptr<IdealBeamformingHelper> idealBeamformingHelper)
{
  m_idealBeamformingHelper = idealBeamformingHelper;
  m_idealBeamformingHelper->Initialize();
}

class MmWaveDrbActivator : public SimpleRefCount<MmWaveDrbActivator>
{
public:
  MmWaveDrbActivator (Ptr<NetDevice> ueDevice, EpsBearer bearer);
  static void ActivateCallback (Ptr<MmWaveDrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti);
  void ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti);
private:
  bool m_active;
  Ptr<NetDevice> m_ueDevice;
  EpsBearer m_bearer;
  uint64_t m_imsi;
};

MmWaveDrbActivator::MmWaveDrbActivator (Ptr<NetDevice> ueDevice, EpsBearer bearer)
  : m_active (false),
  m_ueDevice (ueDevice),
  m_bearer (bearer),
  m_imsi (m_ueDevice->GetObject< MmWaveUeNetDevice> ()->GetImsi ())
{
}

void
MmWaveDrbActivator::ActivateCallback (Ptr<MmWaveDrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (a << context << imsi << cellId << rnti);
  a->ActivateDrb (imsi, cellId, rnti);
}

void
MmWaveDrbActivator::ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{

  NS_LOG_FUNCTION (this << imsi << cellId << rnti << m_active);
  if ((!m_active) && (imsi == m_imsi))
    {
      Ptr<LteUeRrc> ueRrc = m_ueDevice->GetObject<MmWaveUeNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetState () == LteUeRrc::CONNECTED_NORMALLY);
      uint16_t rnti = ueRrc->GetRnti ();
      Ptr<MmWaveEnbNetDevice> enbLteDevice = m_ueDevice->GetObject<MmWaveUeNetDevice> ()->GetTargetEnb ();
      Ptr<LteEnbRrc> enbRrc = enbLteDevice->GetObject<MmWaveEnbNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetCellId () == enbLteDevice->GetCellId ());
      Ptr<UeManager> ueManager = enbRrc->GetUeManager (rnti);
      NS_ASSERT (ueManager->GetState () == UeManager::CONNECTED_NORMALLY
                 || ueManager->GetState () == UeManager::CONNECTION_RECONFIGURATION);
      EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params;
      params.rnti = rnti;
      params.bearer = m_bearer;
      params.bearerId = 0;
      params.gtpTeid = 0;   // don't care
      enbRrc->GetS1SapUser ()->DataRadioBearerSetupRequest (params);
      m_active = true;
    }
}
void
MmWaveHelper::ActivateDataRadioBearer (NetDeviceContainer ueDevices, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      ActivateDataRadioBearer (*i, bearer);
    }
}
void
MmWaveHelper::ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice);
  NS_ASSERT_MSG (m_epcHelper == 0, "this method must not be used when the EPC is being used");

  // Normally it is the EPC that takes care of activating DRBs
  // when the UE gets connected. When the EPC is not used, we achieve
  // the same behavior by hooking a dedicated DRB activation function
  // to the Enb RRC Connection Established trace source


  Ptr<MmWaveEnbNetDevice> enbmmWaveDevice = ueDevice->GetObject<MmWaveUeNetDevice> ()->GetTargetEnb ();

  std::ostringstream path;
  path << "/NodeList/" << enbmmWaveDevice->GetNode ()->GetId ()
       << "/DeviceList/" << enbmmWaveDevice->GetIfIndex ()
       << "/LteEnbRrc/ConnectionEstablished";
  Ptr<MmWaveDrbActivator> arg = Create<MmWaveDrbActivator> (ueDevice, bearer);
  Config::Connect (path.str (), MakeBoundCallback (&MmWaveDrbActivator::ActivateCallback, arg));
}


void
MmWaveHelper::EnableTraces (void)
{
  EnableDlPhyTrace ();
  EnableUlPhyTrace ();
  //EnableEnbPacketCountTrace ();
  //EnableUePacketCountTrace ();
  //EnableTransportBlockTrace ();
  EnableRlcTraces ();
  EnablePdcpTraces ();
  EnableEnbPhyCtrlMsgsTraces ();
  EnableUePhyCtrlMsgsTraces ();
  EnableEnbMacCtrlMsgsTraces ();
  EnableUeMacCtrlMsgsTraces ();
}

void
MmWaveHelper::EnableDlPhyTrace (void)
{
  //NS_LOG_FUNCTION_NOARGS ();
  //Config::Connect ("/NodeList/*/DeviceList/*/MmWaveUePhy/ReportCurrentCellRsrpSinr",
  //  MakeBoundCallback (&MmWavePhyRxTrace::ReportCurrentCellRsrpSinrCallback, m_phyStats));

  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUePhy/SpectrumPhy/RxPacketTraceUe",
                   MakeBoundCallback (&MmWavePhyRxTrace::RxPacketTraceUeCallback, m_phyStats));
}

void
MmWaveHelper::EnableEnbPhyCtrlMsgsTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveEnbPhy/EnbPhyRxedCtrlMsgsTrace",
                   MakeBoundCallback (&MmWavePhyRxTrace::RxedEnbPhyCtrlMsgsCallback, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveEnbPhy/EnbPhyTxedCtrlMsgsTrace",
                   MakeBoundCallback (&MmWavePhyRxTrace::TxedEnbPhyCtrlMsgsCallback, m_phyStats));
}

void
MmWaveHelper::EnableEnbMacCtrlMsgsTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveEnbMac/EnbMacRxedCtrlMsgsTrace",
                   MakeBoundCallback (&MmwaveMacRxTrace::RxedEnbMacCtrlMsgsCallback, m_macStats));

  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveEnbMac/EnbMacTxedCtrlMsgsTrace",
                   MakeBoundCallback (&MmwaveMacRxTrace::TxedEnbMacCtrlMsgsCallback, m_macStats));
}

void
MmWaveHelper::EnableUePhyCtrlMsgsTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUePhy/UePhyRxedCtrlMsgsTrace",
                   MakeBoundCallback (&MmWavePhyRxTrace::RxedUePhyCtrlMsgsCallback, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUePhy/UePhyTxedCtrlMsgsTrace",
                   MakeBoundCallback (&MmWavePhyRxTrace::TxedUePhyCtrlMsgsCallback, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUePhy/UePhyRxedDlDciTrace",
                   MakeBoundCallback (&MmWavePhyRxTrace::RxedUePhyDlDciCallback, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUePhy/UePhyTxedHarqFeedbackTrace",
                   MakeBoundCallback (&MmWavePhyRxTrace::TxedUePhyHarqFeedbackCallback, m_phyStats));
}

void
MmWaveHelper::EnableUeMacCtrlMsgsTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUeMac/UeMacRxedCtrlMsgsTrace",
                   MakeBoundCallback (&MmwaveMacRxTrace::RxedUeMacCtrlMsgsCallback, m_macStats));
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUeMac/UeMacTxedCtrlMsgsTrace",
                   MakeBoundCallback (&MmwaveMacRxTrace::TxedUeMacCtrlMsgsCallback, m_macStats));
}

void
MmWaveHelper::EnableUlPhyTrace (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveEnbPhy/SpectrumPhy/RxPacketTraceEnb",
                   MakeBoundCallback (&MmWavePhyRxTrace::RxPacketTraceEnbCallback, m_phyStats));
}

void
MmWaveHelper::EnableEnbPacketCountTrace ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMap/*/MmWaveEnbPhy/SpectrumPhy/ReportEnbTxRxPacketCount",
                   MakeBoundCallback (&MmWavePhyRxTrace::ReportPacketCountEnbCallback, m_phyStats));

}

void
MmWaveHelper::EnableUePacketCountTrace ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUePhy/SpectrumPhy/ReportUeTxRxPacketCount",
                   MakeBoundCallback (&MmWavePhyRxTrace::ReportPacketCountUeCallback, m_phyStats));

}

void
MmWaveHelper::EnableTransportBlockTrace ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/MmWaveUePhy/ReportDownlinkTbSize",
                   MakeBoundCallback (&MmWavePhyRxTrace::ReportDownLinkTBSize, m_phyStats));
}


void
MmWaveHelper::EnableRlcTraces (void)
{
  NS_ASSERT_MSG (m_rlcStats == 0, "please make sure that MmWaveHelper::EnableRlcTraces is called at most once");
  m_rlcStats = CreateObject<MmWaveBearerStatsCalculator> ("RLC");
  m_radioBearerStatsConnector.EnableRlcStats (m_rlcStats);
}

Ptr<MmWaveBearerStatsCalculator>
MmWaveHelper::GetRlcStats (void)
{
  return m_rlcStats;
}

void
MmWaveHelper::EnablePdcpTraces (void)
{
  NS_ASSERT_MSG (m_pdcpStats == 0, "please make sure that MmWaveHelper::EnablePdcpTraces is called at most once");
  m_pdcpStats = CreateObject<MmWaveBearerStatsCalculator> ("PDCP");
  m_radioBearerStatsConnector.EnablePdcpStats (m_pdcpStats);
}

Ptr<MmWaveBearerStatsCalculator>
MmWaveHelper::GetPdcpStats (void)
{
  return m_pdcpStats;
}

} // namespace ns3

