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



#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/abort.h>

#include <ns3/pointer.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>

namespace ns3 {

/* ... */
NS_LOG_COMPONENT_DEFINE ("NrSlHelper");

NS_OBJECT_ENSURE_REGISTERED (NrSlHelper);

NrSlHelper::NrSlHelper (void)

{
  NS_LOG_FUNCTION (this);
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
  m_slResoPoolFactory = nullptr;
  m_slPreConfigResoPoolFactory = nullptr;
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
NrSlHelper::InstallNrSlPreConfiguration (NetDeviceContainer c, const LteRrcSap::SidelinkPreconfigNr preConfig)
{
  NS_LOG_FUNCTION (this);

  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<NetDevice> netDev = *i;
      Ptr<NrUeNetDevice> nrUeDev = netDev->GetObject <NrUeNetDevice>();
      Ptr<NrSlUeRrc> nrSlUeRrc = nrUeDev->GetObject <NrSlUeRrc> ();
      nrSlUeRrc->SetNrSlPreconfiguration (preConfig);



    //  Ptr<NetDevice> device = InstallSingleUeDevice (node);
    //  device->SetAddress (Mac48Address::Allocate ());
    //  devices.Add (device);
    }
 // return devices;

}







} // namespace ns3

