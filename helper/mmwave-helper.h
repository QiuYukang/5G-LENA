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
#include <algorithm>

namespace ns3 {

/* ... */
class MmWaveUePhy;
class MmWaveEnbPhy;
class SpectrumChannel;
class SpectrumpropagationLossModel;
class MmWaveSpectrumValueHelper;
class PropagationLossModel;


/**
 * Bandwidth part configuration element
 */
struct ComponentCarrierBandwidthPartElement
{
  uint8_t m_bwpId;            //<! BWP id
  uint8_t m_numerology;       //<! BWP numerology: 0,1,2,3,4
  double m_centralFrequency;  //<! BWP central frequency
  double m_lowerFrequency;    //<! BWP lower frequency
  double m_higherFrequency;   //<! BWP higher frequency
  uint32_t m_bandwidth;       //<! BWP bandwidth
};


enum ComponentCarrierState
{
  PRIMARY,
  SECONDARY
};

/**
 * Component carrier configuration element
 */
struct ComponentCarrierInfo
{
  uint8_t m_ccId {0};          //<! CC id
  uint8_t m_numBwps {0};       //<! Number of BWP in the carrier
  uint8_t m_activeBwp;         //<! Active BWP index
  double m_centralFrequency;   //<! BWP central frequency
  double m_lowerFrequency;     //<! BWP lower frequency
  double m_higherFrequency;    //<! BWP higher frequency
  uint32_t m_bandwidth;        //<! BWP bandwidth
  ComponentCarrierState m_primaryCc {PRIMARY};  //<! Primary or secondary CC
  std::map<uint8_t, ComponentCarrierBandwidthPartElement> m_bwp;  //<! Space for BWP

  /**
   * \brief Adds a BWP to the carrier
   * \param bwp Description of the BWP
   */
  void AddBwp (const ComponentCarrierBandwidthPartElement & bwp);
  void AddBwp (uint8_t bwdId, const ComponentCarrierBandwidthPartElement & bwp);
};


/**
 * Carrier aggregation contiguous allocation mode within an operation band
 */
enum ContiguousMode
{
  CONTIGUOUS,
  NON_CONTIGUOUS
};


/*
 * Upper limits of the number of component carriers used for
 * Carrier Aggregation depends on the CC contiguousness.
 * Eventually, the number of CCs may depend on the operation frequency
 */
const uint8_t MAX_CC_INTRA_BAND = 8;  //<! Up to 8 CCs can be aggregated in the same operation band
const uint8_t MAX_CC_INTER_BAND = 16; //<! The maximum number of aggregated CCs is 16 in NR Rel. 16 (in more than one operation band)

/**
 * \brief The operation band information structure
 */
struct OperationBandInfo
{
  uint8_t m_bandId {0};       //<! Operation band id
  double m_centralFrequency;  //<! Operation band central frequency
  double m_lowerFrequency;    //<! Operation band lower frequency
  double m_higherFrequency;   //<! Operation band higher frequency
  uint32_t m_bandwidth;       //<! Operation band bandwidth
  uint8_t m_numCarriers {0};  //<! Number of configured carriers in the operation band
  ContiguousMode m_contiguousCc {CONTIGUOUS};  //<! CA intra-band contiguousness
  std::map<uint8_t, ComponentCarrierInfo> m_cc;

  void AddCc (const ComponentCarrierInfo &cc);
  void AddCc (uint8_t ccId, const ComponentCarrierInfo &cc);
};



/**
 * \brief Manages the correct creation of operation bands, component carriers and bandwidth parts
 */
class ComponentCarrierBandwidthPartCreator
{
public:
  ComponentCarrierBandwidthPartCreator ();
  ComponentCarrierBandwidthPartCreator (uint8_t maxNumBands);
  virtual ~ComponentCarrierBandwidthPartCreator ();

  ComponentCarrierBandwidthPartCreator& operator= (const ns3::ComponentCarrierBandwidthPartCreator&);


  /**
   * \brief Creates an operation band by splitting the available bandwidth into numCCs equally-large contiguous carriers
   * \param centralFrequency Central operation frequency in Hz
   * \param operationBandwidth Operation band bandwidth
   * \param numCCs Number of contiguous CC
   */
  void CreateOperationBandContiguousCc (double centralFrequency, uint32_t operationBandwidth, uint8_t numCCs);

  /**
   * \brief Creates an operation band with the desired central frequency and bandwidth with no CC information
   * \param centralFrequency The central frequency of the operation band
   * \param operationBandwidth The operation band bandwidth
   */
  OperationBandInfo CreateEmptyOperationBand (double centralFrequency, uint32_t operationBandwidth);

  /**
   * \brief Creates an operation band with the desired central frequency and bandwidth, with a single CC occupying the whole operation band
   * \param centralFrequency The central frequency of the operation band
   * \param operationBandwidth The operation band bandwidth
   */
  OperationBandInfo CreateOperationBand (double centralFrequency, uint32_t operationBandwidth);

  /**
   * \brief Adds the operation band to the class
   * \param band Description of the operation band
   */
  void AddOperationBand (const OperationBandInfo &bandInfo);

  /**
   * \brief Performs some validation checks on the provided operation band configuration and its child CC and BWP structures
   * \param band Operation band parameters
   */
  void ValidateOperationBand (OperationBandInfo &band);

  /**
   * \brief Checks the consistency of BWP within the carrier
   * \note Simulation will stop if a bad configuration is found
   * \param cc Component Carrier definition
   */
  void CheckBwpsInCc (const ComponentCarrierInfo &cc);

  /**
   * \brief Validates the CA/BWP configuration
   */
  void ValidateCaBwpConfiguration ();

  /**
   * \brief Determines whether the CCs in the band are contiguous or not based on a intra-band CC frequency offset
   * \param band Operation band information structure
   * \param freqSeparation Maximum separation between contiguous CCs in Hz
   * \returns Flag indicating contiguous or non-contiguous CCs for the given band
   */
  ContiguousMode GetCcContiguousnessState (OperationBandInfo &band, uint32_t freqSeparation);

  /**
   * \brief Gets the ComponentCarrierBandwidthPartElement struct of the active BWP of the primary CC
   * \return The active BWP information object
   */
  ComponentCarrierBandwidthPartElement GetActiveBwpInfo ();

  /**
   * \brief Gets the ComponentCarrierBandwidthPartElement struct of the active BWP of the provided band and carrier index
   * \param bandIndex Operation band id
   * \param ccIndex Component carrier id
   * \return The active BWP information object
   */
  ComponentCarrierBandwidthPartElement GetActiveBwpInfo (uint8_t bandIndex, uint8_t ccIndex);

  /**
   * \brief Gets the ComponentCarrierInfo struct associated to the provided operation band and carrier indices
   * \param bandIndex Operation band id
   * \param ccIndex Component carrier id
   * \return The desired CC information object
   */
  ComponentCarrierInfo GetComponentCarrier (uint8_t bandId, uint8_t ccId);

  /**
   * \brief Returns the aggregated bandwidth in the CA configuration, considering the active BWP
   * \return Aggregated bandwidth in Hz
   */
  uint32_t GetAggregatedBandwidth ();

  /**
   * \brief Iterates along the operation bands and returns the active BWP of the given CC id
   * \param ccId CC id (position in the operation band's internal vector of CCs)
   * \return Aggregated bandwidth in Hz
   */
  uint32_t GetCarrierBandwidth (uint8_t ccId);

  /**
   * \brief Returns the active BWP of the given CC in the given operation band
   * \param bandId Operation band id (position in the class internal vector of operation bands)
   * \param ccId CC id (position in the operation band's internal vector of CCs)
   * \return Aggregated bandwidth in Hz
   */
  uint32_t GetCarrierBandwidth (uint8_t bandId, uint8_t ccId);

  /**
   * \brief Change the active BWP of a given UE operating in the given ccId
   * \param bandId Operation band id containing the CC/BWP to set to active
   * \param ccId Component carrier id which BWP is to set to active
   * \param activeBwpId BWP id to set to active
   */
  void ChangeActiveBwp (uint8_t bandId, uint8_t ccId, uint8_t activeBwpId);


private:
  uint32_t m_id {0};       //!< UE/flow/bearer id
  uint8_t m_maxBands {1};  //!< Limit the number of operation bands
  uint8_t m_numBands {0};  //!< Number of current operation bands. It must be smaller or equal than m_maxBands
  uint8_t m_numBwps {0};   //!< Number of BWP created. Consider removing
  uint8_t m_numCcs {0};    //!< Number of Component Carriers created
  std::vector<OperationBandInfo> m_bands;  //!< Vector to the operation band information elements

};


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
  std::vector<LteNrTddSlotType> m_pattern {LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F,
                                           LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F,
                                           LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F};
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

