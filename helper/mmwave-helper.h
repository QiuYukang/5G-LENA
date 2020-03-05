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


#ifndef MMWAVE_HELPER_H
#define MMWAVE_HELPER_H

#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/eps-bearer.h>
#include <ns3/object-factory.h>
#include <ns3/mmwave-bearer-stats-connector.h>
#include <ns3/mmwave-control-messages.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/mmwave-spectrum-phy.h>
#include "ideal-beamforming-helper.h"
#include "cc-bwp-helper.h"

namespace ns3 {

class MmWaveUePhy;
class MmWaveEnbPhy;
class SpectrumChannel;
class MmWaveSpectrumValueHelper;
class MmWaveEnbMac;
class EpcHelper;
class EpcTft;
class MmWaveBearerStatsCalculator;
class MmwaveMacRxTrace;
class MmWavePhyRxTrace;
class MmWavePhyMacCommon;
class ComponentCarrierEnb;
class ComponentCarrier;
class MmWaveMacScheduler;
class MmWaveEnbNetDevice;
class MmWaveUeNetDevice;
class MmWaveUeMac;

class MmWaveHelper : public Object
{

public:
  MmWaveHelper (void);
  virtual ~MmWaveHelper (void);

  static TypeId GetTypeId (void);

  /**
   * @brief InstallUeDevice
   * @param c
   * @param allBwps
   * @return
   *
   * The position on the vector of bwp will be the bwp id
   */
  NetDeviceContainer InstallUeDevice (const NodeContainer &c,
                                      const std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> &allBwps);
  NetDeviceContainer InstallGnbDevice (const NodeContainer &c,
                                       const std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> allBwps);

  void ConfigureCarriers (std::map<uint8_t, Ptr<ComponentCarrierEnb> > ccPhyConf);

  /**
   * \brief Get the number of configured BWP for a specific GNB NetDevice
   * \param gnbDevice The GNB NetDevice
   * \return the number of BWP installed, or 0 if there are errors
   */
  static uint32_t GetNumberBwp (const Ptr<const NetDevice> &gnbDevice);
  /**
   * \brief Get a pointer to the PHY of the GNB at the specified BWP
   * \param gnbDevice The GNB NetDevice
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the PHY layer of the GNB, or nullptr if there are errors
   */
  static Ptr<MmWaveEnbPhy> GetEnbPhy (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex);
  /**
   * \brief Get a pointer to the MAC of the GNB at the specified BWP
   * \param gnbDevice The GNB NetDevice
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the MAC layer of the GNB, or nullptr if there are errors
   */
  static Ptr<MmWaveEnbMac> GetEnbMac (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex);

  /**
   * This method is used to send the ComponentCarrier map created with CcHelper
   * to the helper, the structure will be used within InstallSingleEnbDevice
   *
   * \param ccmap the component carrier map
   */
  void SetCcPhyParams (const std::map<uint8_t, ComponentCarrier> &ccmap);

  void AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices);
  void AttachToEnb (const Ptr<NetDevice> &ueDevice, const Ptr<NetDevice> &gnbDevice);

  void EnableTraces ();

  void ActivateDataRadioBearer (NetDeviceContainer ueDevices, EpsBearer bearer);
  void ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer);
  void SetEpcHelper (Ptr<EpcHelper> epcHelper);
  void SetIdealBeamformingHelper (Ptr<IdealBeamformingHelper> idealBeamformingHelper);

  void SetHarqEnabled (bool harqEnabled);
  bool GetHarqEnabled ();
  void SetSnrTest (bool snrTest);
  bool GetSnrTest ();

  /**
   * \brief Initialize the bandwidth parts by creating and configuring the channel
   * models, if they are not already initialized.
   *
   * If the models are already set (i.e., the pointers are not null) the helper
   * will not touch anything.
   *
   * \param band the band representation
   */
  void InitializeOperationBand (OperationBandInfo *band) const;

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

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUeMacAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbMacAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbSpectrumAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUeSpectrumAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUeChannelAccessManagerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbChannelAccessManagerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetSchedulerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUePhyAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbPhyAttribute (const std::string &n, const AttributeValue &v);

  void SetMmWavePhyMacCommonAttribute (const std::string &n, const AttributeValue &v);

  void SetUeChannelAccessManagerTypeId (const TypeId &typeId);

  void SetGnbChannelAccessManagerTypeId (const TypeId &typeId);

  void SetSchedulerTypeId (const TypeId &typeId);

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

  Ptr<MmWaveEnbPhy> CreateGnbPhy (const Ptr<Node> &n,
                                  const Ptr<MmWavePhyMacCommon> &phyMacCommon,
                                  const Ptr<SpectrumChannel> &c,
                                  const Ptr<ThreeGppSpectrumPropagationLossModel> &gppChannel,
                                  const Ptr<MmWaveEnbNetDevice> &dev, uint16_t cellId,
                                  const MmWaveSpectrumPhy::MmWavePhyRxCtrlEndOkCallback &phyEndCtrlCallback);
  Ptr<MmWaveMacScheduler> CreateGnbSched (const Ptr<MmWavePhyMacCommon>& conf);
  Ptr<MmWaveEnbMac> CreateGnbMac (const Ptr<MmWavePhyMacCommon>& conf);

  Ptr<MmWaveUeMac> CreateUeMac () const;
  Ptr<MmWaveUePhy> CreateUePhy (const Ptr<Node> &n, const Ptr<SpectrumChannel> &c,
                                const Ptr<ThreeGppSpectrumPropagationLossModel> &gppChannel,
                                const Ptr<MmWaveUeNetDevice> &dev,
                                const MmWaveSpectrumPhy::MmWavePhyDlHarqFeedbackCallback &dlHarqCallback,
                                const MmWaveSpectrumPhy::MmWavePhyRxCtrlEndOkCallback &phyRxCtrlCallback);

  Ptr<NetDevice> InstallSingleUeDevice (const Ptr<Node> &n,
                                        const std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> allBwps);
  Ptr<NetDevice> InstallSingleGnbDevice (const Ptr<Node> &n,
                                         const std::vector<std::reference_wrapper<BandwidthPartInfoPtr>> allBwps);
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

  ObjectFactory m_enbNetDeviceFactory;  //!< NetDevice factory for gnb
  ObjectFactory m_ueNetDeviceFactory;   //!< NetDevice factory for ue
  ObjectFactory m_channelFactory;       //!< Channel factory
  ObjectFactory m_phyMacCommonFactory;  //!< PhyMacCommon factory
  ObjectFactory m_ueMacFactory;         //!< UE MAC factory
  ObjectFactory m_gnbMacFactory;        //!< GNB MAC factory
  ObjectFactory m_ueSpectrumFactory;    //!< UE Spectrum factory
  ObjectFactory m_gnbSpectrumFactory;   //!< GNB spectrum factory
  ObjectFactory m_uePhyFactory;         //!< UE PHY factory
  ObjectFactory m_gnbPhyFactory;        //!< GNB PHY factory
  ObjectFactory m_ueChannelAccessManagerFactory; //!< UE Channel access manager factory
  ObjectFactory m_gnbChannelAccessManagerFactory; //!< GNB Channel access manager factory
  ObjectFactory m_schedFactory;         //!< Scheduler factory


  uint64_t m_imsiCounter;
  uint16_t m_cellIdCounter;

  Ptr<MmWavePhyRxTrace> m_phyStats;
  Ptr<MmwaveMacRxTrace> m_macStats;

  Ptr<EpcHelper> m_epcHelper;
  Ptr<IdealBeamformingHelper> m_idealBeamformingHelper {nullptr};

  bool m_harqEnabled;
  bool m_snrTest;

  Ptr<MmWaveBearerStatsCalculator> m_rlcStats;
  Ptr<MmWaveBearerStatsCalculator> m_pdcpStats;
  MmWaveBearerStatsConnector m_radioBearerStatsConnector;

  /**
   * This contains all the information about each component carrier
   */
  std::map<uint8_t, ComponentCarrier> m_componentCarrierPhyParams;
};

}

#endif /* MMWAVE_HELPER_H */

