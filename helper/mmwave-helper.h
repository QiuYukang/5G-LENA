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
*   Author: Marco Miozzo <marco.miozzo@cttc.es>
*           Nicola Baldo  <nbaldo@cttc.es>
*
*   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
*                         Sourjya Dutta <sdutta@nyu.edu>
*                         Russell Ford <russell.ford@nyu.edu>
*                         Menglei Zhang <menglei@nyu.edu>
*/


#ifndef MMWAVE_HELPER_H
#define MMWAVE_HELPER_H

#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/names.h>
#include <ns3/net-device.h>
#include <ns3/net-device-container.h>
#include <ns3/node.h>
#include <ns3/node-container.h>
#include <ns3/mobility-model.h>
#include <ns3/spectrum-phy.h>
#include <ns3/mmwave-ue-net-device.h>
#include <ns3/mmwave-enb-net-device.h>
#include <ns3/mmwave-phy.h>
#include <ns3/mmwave-ue-phy.h>
#include <ns3/mmwave-enb-phy.h>
#include <ns3/mmwave-spectrum-value-helper.h>
#include <ns3/mmwave-phy-mac-common.h>
#include <ns3/mmwave-rrc-protocol-ideal.h>
#include "mmwave-phy-rx-trace.h"
#include "mmwave-mac-rx-trace.h"
#include <ns3/epc-helper.h>
#include <ns3/epc-ue-nas.h>
#include <ns3/lte-enb-rrc.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/boolean.h>
#include <ns3/epc-helper.h>
#include <ns3/lte-ffr-algorithm.h>
#include <ns3/mmwave-bearer-stats-calculator.h>
#include <ns3/mmwave-bearer-stats-connector.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/mmwave-3gpp-channel.h>
#include <ns3/cc-helper.h>

namespace ns3 {

/* ... */
class MmWaveUePhy;
class MmWaveEnbPhy;
class SpectrumChannel;
class SpectrumpropagationLossModel;
class MmWaveSpectrumValueHelper;
class PropagationLossModel;


class BandwidthPartRepresentation
{
public:
  BandwidthPartRepresentation (uint32_t id, const Ptr<MmWavePhyMacCommon> &phyMacCommon,
                               const Ptr<SpectrumChannel> &channel,
                               const Ptr<PropagationLossModel> &propagation,
                               const Ptr<MmWave3gppChannel> & spectrumPropagation);
  BandwidthPartRepresentation (const BandwidthPartRepresentation & o);
  ~BandwidthPartRepresentation ();

  BandwidthPartRepresentation& operator= (const ns3::BandwidthPartRepresentation&);

  uint32_t m_id {0};
  Ptr<MmWavePhyMacCommon> m_phyMacCommon;
  Ptr<SpectrumChannel> m_channel;
  Ptr<PropagationLossModel> m_propagation;
  Ptr<MmWave3gppChannel> m_3gppChannel;
  std::string m_gnbChannelAccessManagerType {"ns3::NrAlwaysOnAccessManager"}; //!< Channel access manager type for GNB
  std::string m_ueChannelAccessManagerType {"ns3::NrAlwaysOnAccessManager"}; //!< Channel access manager type for UE
};

class MmWaveHelper : public Object
{

public:
  MmWaveHelper (void);
  virtual ~MmWaveHelper (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);

  NetDeviceContainer InstallUeDevice (NodeContainer c);
  NetDeviceContainer InstallEnbDevice (NodeContainer c);

  void ConfigureCarriers (std::map<uint8_t, Ptr<ComponentCarrierEnb> > ccPhyConf);

  void SetPathlossModelType (std::string type);
  void SetChannelModelType (std::string type);

  /**
   * This method is used to send the ComponentCarrier map created with CcHelper
   * to the helper, the structure will be used within InstallSingleEnbDevice
   *
   * \param ccmap the component carrier map
   */
  void SetCcPhyParams (std::map< uint8_t, ComponentCarrier> ccmap);

  void AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices);
  void AttachToEnb (const Ptr<NetDevice> &ueDevice, const Ptr<NetDevice> &gnbDevice);

  void EnableTraces ();

  void SetSchedulerType (std::string type);

  void ActivateDataRadioBearer (NetDeviceContainer ueDevices, EpsBearer bearer);
  void ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer);
  void SetEpcHelper (Ptr<EpcHelper> epcHelper);

  void SetHarqEnabled (bool harqEnabled);
  bool GetHarqEnabled ();
  void SetSnrTest (bool snrTest);
  bool GetSnrTest ();
  Ptr<PropagationLossModel> GetPathLossModel (uint8_t index);
  void AddBandwidthPart (uint32_t id, const BandwidthPartRepresentation &bwpRepr);

  /**
   * Activate a dedicated EPS bearer on a given set of UE devices.
   *
   * \param ueDevices the set of UE devices
   * \param bearer the characteristics of the bearer to be activated
   * \param tft the Traffic Flow Template that identifies the traffic to go on this bearer
   * \returns bearer ID
   */
  uint8_t ActivateDedicatedEpsBearer (NetDeviceContainer ueDevices, EpsBearer bearer, Ptr<EpcTft> tft);

  /**
   * Activate a dedicated EPS bearer on a given UE device.
   *
   * \param ueDevice the UE device
   * \param bearer the characteristics of the bearer to be activated
   * \param tft the Traffic Flow Template that identifies the traffic to go on this bearer.
   * \returns bearer ID
   */
  uint8_t ActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer, Ptr<EpcTft> tft);

  /**
   *  \brief Manually trigger dedicated bearer de-activation at specific simulation time
   *  \param ueDevice the UE on which dedicated bearer to be de-activated must be of the type LteUeNetDevice
   *  \param enbDevice eNB, must be of the type LteEnbNetDevice
   *  \param bearerId Bearer Identity which is to be de-activated
   *
   *  \warning Requires the use of EPC mode. See SetEpcHelper() method.
   */

  void DeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId);

protected:
  /**
   * \brief Initialize things inside the helper.
   *
   * The most important thing is the channel and the propagation loss model
   * for each bandwidth part. If they are not specified by the user through
   * AddBandwidthPart, one will created by default. If the user specifies
   * the channel and the propagation model as bwp configuration, they will be
   * not touched. Otherwise, the models will be created and connected for each
   * bwp.
   */
  virtual void DoInitialize ();

private:
  /**
   *  \brief The actual function to trigger a manual bearer de-activation
   *  \param ueDevice the UE on which bearer to be de-activated must be of the type LteUeNetDevice
   *  \param enbDevice eNB, must be of the type LteEnbNetDevice
   *  \param bearerId Bearer Identity which is to be de-activated
   *
   *  This method is normally scheduled by DeActivateDedicatedEpsBearer() to run at a specific
   *  time when a manual bearer de-activation is desired by the simulation user.
   */
  void DoDeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId);

  Ptr<NetDevice> InstallSingleUeDevice (Ptr<Node> n);
  Ptr<NetDevice> InstallSingleEnbDevice (Ptr<Node> n);
  void AttachToClosestEnb (Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices);
  void EnableDlPhyTrace ();
  void EnableUlPhyTrace ();
  void EnableEnbPacketCountTrace ();
  void EnableUePacketCountTrace ();
  void EnableTransportBlockTrace ();
  void EnableRlcTraces (void);
  void EnableEnbPhyCtrlMsgsTraces (void);
  void EnableUePhyCtrlMsgsTraces (void);
  void EnableEnbMacCtrlMsgsTraces (void);
  void EnableUeMacCtrlMsgsTraces (void);
  Ptr<MmWaveBearerStatsCalculator> GetRlcStats (void);
  void EnablePdcpTraces (void);
  Ptr<MmWaveBearerStatsCalculator> GetPdcpStats (void);

  std::map<uint8_t, ComponentCarrier> GetBandwidthPartMap ();
  
  std::map< uint8_t, Ptr<Object> > m_pathlossModel;
  std::string m_pathlossModelType;
  std::string m_channelModelType;

  ObjectFactory m_enbNetDeviceFactory;
  ObjectFactory m_ueNetDeviceFactory;
  ObjectFactory m_channelFactory;
  ObjectFactory m_pathlossModelFactory;
  ObjectFactory m_phyMacCommonFactory;

  uint64_t m_imsiCounter;
  uint16_t m_cellIdCounter;

  Ptr<MmWavePhyRxTrace> m_phyStats;
  Ptr<MmwaveMacRxTrace> m_macStats;

  ObjectFactory m_ffrAlgorithmFactory;

  Ptr<EpcHelper> m_epcHelper;

  bool m_harqEnabled;
  bool m_snrTest;

  Ptr<MmWaveBearerStatsCalculator> m_rlcStats;
  Ptr<MmWaveBearerStatsCalculator> m_pdcpStats;
  MmWaveBearerStatsConnector m_radioBearerStatsConnector;

  /**
   * The `UseCa` attribute. If true, Carrier Aggregation is enabled.
   * Hence, the helper will expect a valid component carrier map
   * If it is false, the component carrier will be created within the LteHelper
   * this is to maintain the backwards compatibility with user script
   */
  bool m_useCa;

  bool m_initialized {false}; //!< Is helper initialized correctly?

  /**
   * This contains all the information about each component carrier
   */
  std::map<uint8_t, ComponentCarrier> m_componentCarrierPhyParams;

  std::unordered_map<uint32_t, BandwidthPartRepresentation> m_bwpConfiguration;

  /**
   * Number of component carriers that will be installed by default at eNodeB and UE devices.
   */
  uint16_t m_noOfCcs;
  TypeId m_defaultSchedulerType;
};

}

#endif /* MMWAVE_HELPER_H */

