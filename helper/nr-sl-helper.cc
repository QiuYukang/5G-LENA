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

#include "nr-sl-helper.h"
#include <ns3/nr-sl-comm-resource-pool-factory.h>
#include <ns3/nr-sl-comm-preconfig-resource-pool-factory.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/nr-sl-ue-rrc.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/nr-ue-mac.h>
#include <ns3/nr-amc.h>
#include <ns3/nr-spectrum-phy.h>



#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/abort.h>

#include <ns3/pointer.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlHelper");

NS_OBJECT_ENSURE_REGISTERED (NrSlHelper);

NrSlHelper::NrSlHelper (void)

{
  NS_LOG_FUNCTION (this);
  m_ueSlAmcFactory.SetTypeId (NrAmc::GetTypeId ());
  //m_channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());
 // m_enbNetDeviceFactory.SetTypeId (NrEnbNetDevice::GetTypeId ());
 // m_ueNetDeviceFactory.SetTypeId (NrUeNetDevice::GetTypeId ());

  //Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));
}

NrSlHelper::~NrSlHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrSlHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrSlHelper")
    .SetParent<Object> ()
    .SetGroupName("nr")
    .AddConstructor<NrSlHelper> ();
  return tid;
}

void
NrSlHelper::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  //m_slResoPoolFactory = nullptr;
  //m_slPreConfigResoPoolFactory = nullptr;
  Object::DoDispose ();
}
/*
void
NrSlHelper::SetSlPoolFactory (Ptr<NrSlResourcePoolFactory> poolFactory)
{
  NS_LOG_FUNCTION (this);
  m_slResoPoolFactory = poolFactory;
}

void
NrSlHelper::SetSlPreConfigPoolFactory (Ptr<NrSlPreconfigResourcePoolFactory> preconfigPoolFactory)
{
  NS_LOG_FUNCTION (this);
  m_slPreConfigResoPoolFactory = preconfigPoolFactory;
}
*/

void
NrSlHelper::SetSlErrorModel(const std::string &errorModelTypeId)
{
  NS_LOG_FUNCTION (this);

  SetUeSlAmcAttribute ("ErrorModelType", TypeIdValue (TypeId::LookupByName (errorModelTypeId)));
}

void
NrSlHelper::SetUeSlAmcAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueSlAmcFactory.Set (n, v);
}

Ptr<NrAmc>
NrSlHelper::CreateUeSlAmc () const
{
  NS_LOG_FUNCTION (this);

  Ptr<NrAmc> slAmc = m_ueSlAmcFactory.Create <NrAmc> ();
  return slAmc;
}

void
NrSlHelper::InstallNrSlPreConfiguration (NetDeviceContainer c, const LteRrcSap::SidelinkPreconfigNr preConfig)
{
  NS_LOG_FUNCTION (this);

  struct LteRrcSap::SlFreqConfigCommonNr slFreqConfigCommonNr = preConfig.slPreconfigFreqInfoList [0];
  LteRrcSap::SlPreconfigGeneralNr slPreconfigGeneralNr = preConfig.slPreconfigGeneral;

  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<NetDevice> netDev = *i;
      Ptr<NrUeNetDevice> nrUeDev = netDev->GetObject <NrUeNetDevice>();
      Ptr<LteUeRrc> lteUeRrc = nrUeDev->GetRrc ();

      Ptr<NrSlUeRrc> nrSlUeRrc = CreateObject<NrSlUeRrc> ();
      nrSlUeRrc->SetNrSlEnabled (true);
      lteUeRrc->AggregateObject (nrSlUeRrc);
      nrSlUeRrc->SetNrSlUeRrcSapProvider (lteUeRrc->GetNrSlUeRrcSapProvider());
      lteUeRrc->SetNrSlUeRrcSapUser (nrSlUeRrc->GetNrSlUeRrcSapUser());

      nrSlUeRrc->SetNrSlPreconfiguration (preConfig);
      bool ueSlBwpConfigured = ConfigUeParams (nrUeDev, slFreqConfigCommonNr, slPreconfigGeneralNr);
      NS_ABORT_MSG_IF (ueSlBwpConfigured == false, "No SL configuration found for IMSI " << nrUeDev->GetImsi ());
    }
}

bool
NrSlHelper::ConfigUeParams (const Ptr<NrUeNetDevice> &dev,
                            const LteRrcSap::SlFreqConfigCommonNr &freqCommon,
                            const LteRrcSap::SlPreconfigGeneralNr &general)
{
  NS_LOG_FUNCTION (this);
  bool found = false;
  std::string tddPattern = general.slTddConfig.tddPattern;

  for (uint8_t index = 0; index < freqCommon.slBwpList.size (); ++index)
    {
      //configure the parameters if both BWP generic and SL pools are configured.
      if (freqCommon.slBwpList [index].haveSlBwpGeneric && freqCommon.slBwpList [index].haveSlBwpPoolConfigCommonNr)
        {
          NS_LOG_INFO ("Configuring BWP id " << +index << " for SL");
          dev->GetPhy (index)->RegisterSlBwpId (static_cast <uint16_t> (index));
          dev->GetPhy (index)->SetNumerology (freqCommon.slBwpList [index].slBwpGeneric.bwp.numerology);
          dev->GetPhy (index)->SetSymbolsPerSlot (freqCommon.slBwpList [index].slBwpGeneric.bwp.symbolsPerSlots);
          dev->GetPhy (index)->PreConfigSlBandwidth (freqCommon.slBwpList [index].slBwpGeneric.bwp.bandwidth);
          dev->GetPhy (index)->SetNumRbPerRbg (freqCommon.slBwpList [index].slBwpGeneric.bwp.rbPerRbg);
          dev->GetPhy (index)->SetPattern (tddPattern);
          //Error model and UE MAC AMC
          Ptr<NrAmc> slAmc = CreateUeSlAmc ();
          TypeIdValue typeIdValue;
          slAmc->GetAttribute ("ErrorModelType", typeIdValue);
          dev->GetPhy (index)->GetSpectrumPhy()->SetAttribute ("SlErrorModelType", typeIdValue);
          dev->GetMac (index)->SetSlAmcModel (slAmc);
          found = true;
        }
    }

  return found;
}







} // namespace ns3

