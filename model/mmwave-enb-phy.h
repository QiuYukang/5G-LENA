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
 */
#ifndef MMWAVE_ENB_PHY_H
#define MMWAVE_ENB_PHY_H

#include "mmwave-phy.h"
#include "mmwave-phy-mac-common.h"
#include "mmwave-control-messages.h"
#include <ns3/lte-enb-phy-sap.h>
#include <ns3/lte-enb-cphy-sap.h>
#include <ns3/mmwave-harq-phy.h>
#include <functional>
#include "ns3/ideal-beamforming-algorithm.h"

namespace ns3 {

class PacketBurst;
class MmWaveNetDevice;
class MmWaveUePhy;
class MmWaveEnbMac;
class NrChAccessManager;
class BeamManager;

/**
 * \brief The GNB PHY class
 *
 * To initialize it, you must call also SetSpectrumPhy and StartEventLoop.
 *
 * \see SetSpectrumPhy
 * \see StartEventLoop
 */
class MmWaveEnbPhy : public MmWavePhy
{
  friend class MemberLteEnbCphySapProvider<MmWaveEnbPhy>;
  friend class MmWaveMemberPhySapProvider;
public:
  /**
   * \brief Get Type id
   * \return the type id of the MmWaveEnbPhy
   */
  static TypeId GetTypeId (void);

  /**
   * \brief MmWaveEnbPhy constructor. Please use the other one.
   */
  MmWaveEnbPhy ();

  /**
   * \brief MmWaveEnbPhy real constructor. Start the event loop for the gnb.
   */
  MmWaveEnbPhy (Ptr<MmWaveSpectrumPhy>, const Ptr<Node> &);

  /**
   * \brief ~MmWaveEnbPhy
   */
  virtual ~MmWaveEnbPhy () override;

  /**
   * \brief Set the configuration parameters for the gnb
   * \param phyMacCommon configuration parameters
   */
  void SetConfigurationParameters (const Ptr<MmWavePhyMacCommon> &phyMacCommon);

  /**
   * \brief Set the C PHY SAP user
   * \param s the C PHY SAP user
   */
  void SetEnbCphySapUser (LteEnbCphySapUser* s);
  /**
   * \brief Get the C PHY SAP provider
   * \return the C PHY SAP provider pointer
   */
  LteEnbCphySapProvider* GetEnbCphySapProvider ();

  /**
    * \brief: Set the minimum processing delay (in slots)
    * to decode DL DCI and decode DL data
    */
   void SetN0Delay (uint32_t delay);

   /**
    * \brief: Set the minimum processing delay (in slots)
    * to decode DL Data and send Harq feedback
    *
    * Please note that in the current implementation N1
    * must be equal or larger than 1 (N1 >= 1)
    */
   void SetN1Delay (uint32_t delay);

   /**
    * \brief: Set the minimum processing delay (in slots)
    * to decode UL DCI and prepare UL data
    *
    * Please note that in the current implementation N2
    * must be equal or larger than 1 (N2 >= 1)
    */
   void SetN2Delay (uint32_t delay);

  /**
   * \brief: Get the minimum processing delay (in slots)
   * to decode DL DCI and decode DL Data
   */
  uint32_t GetN0Delay (void) const;

  /**
   * \brief: Get the minimum processing delay (in slots)
   * to decode DL Data and send Harq feedback
   */
  uint32_t GetN1Delay (void) const;

  /**
   * \brief: Get the minimum processing delay (in slots)
   * to decode UL DCI and prepare UL data
   */
  uint32_t GetN2Delay (void) const;

  /**
   * \brief Get the BeamId for the selected user
   * \param rnti the selected user
   * \return the beam id of the user
   */
  BeamId GetBeamId (uint16_t rnti) const override;

  /**
   * \brief Set the channel access manager interface for this instance of the PHY
   * \param s the pointer to the interface
   */
  void SetCam (const Ptr<NrChAccessManager> &s);

  /**
   * \brief Get the channel access manager for the PHY
   * \return the CAM of the PHY
   */
  Ptr<NrChAccessManager> GetCam () const;

  /**
   * \brief Set the transmission power for the UE
   *
   * Please note that there is also an attribute ("MmWaveUePhy::TxPower")
   * \param pow power
   */
  void SetTxPower (double pow);

  /**
   * \brief Retrieve the TX power of the UE
   *
   * Please note that there is also an attribute ("MmWaveUePhy::TxPower")
   * \return the TX power of the UE
   */
  double GetTxPower () const __attribute__((warn_unused_result));

  /**
   * \brief Set the Tx power spectral density based on the RB index vector
   * \param rbIndexVector vector of the index of the RB (in SpectrumValue array)
   * in which there is a transmission
   */
  void SetSubChannels (const std::vector<int> &rbIndexVector);

  /**
   * \brief Add the UE to the list of this gnb UEs.
   *
   * Usually called by the helper when a UE register to this gnb.
   * \param imsi IMSI of the device
   * \param ueDevice Device
   * \return
   */
  bool RegisterUe (uint64_t imsi, const Ptr<MmWaveUeNetDevice> &ueDevice);

  /**
   * \brief Receive a PHY data packet
   *
   * Connected by the helper to a callback of the spectrum.
   *
   * \param p Received packet
   */
  void PhyDataPacketReceived (const Ptr<Packet> &p);

  /**
   * \brief Generate a DL CQI report
   *
   * Connected by the helper to a callback in mmWaveChunkProcessor.
   *
   * \param sinr the SINR
   */
  void GenerateDataCqiReport (const SpectrumValue& sinr);

  /**
   * \brief Receive a list of CTRL messages
   *
   * Connected by the helper to a callback of the spectrum.
   *
   * \param msgList message list
   */
  void PhyCtrlMessagesReceived (const Ptr<MmWaveControlMessage> &msg);

  /**
   * \brief Get the power of the enb
   * \return the power
   */
  int8_t DoGetReferenceSignalPower () const;

  /**
   * \brief Install the PHY SAP user (which is in this case the MAC)
   *
   * \param ptr the PHY SAP user pointer to install
   */
  void SetPhySapUser (MmWaveEnbPhySapUser* ptr);

  /**
   * \brief Get the HARQ feedback from MmWaveSpectrumPhy
   * and forward it to the scheduler
   *
   * Connected by the helper to a spectrum phy callback
   *
   * \param m the HARQ feedback
   */
  void ReportUlHarqFeedback (const UlHarqInfo &mes);

  void SetPattern (const std::string &pattern);

  std::string GetPattern() const;

  /**
   * \brief Start the ue Event Loop
   * \param nodeId the UE nodeId
   * \param startSlot the slot number from which the UE has to start (must be in sync with gnb)
   */
  virtual void ScheduleStartEventLoop (uint32_t nodeId, uint16_t frame, uint8_t subframe, uint16_t slot) override;

  /**
   *  TracedCallback signature for Received Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* RxedEnbPhyCtrlMsgsTracedCallback)
      (const SfnSf sfn, const uint16_t rnti, const uint8_t bwpId, Ptr<MmWaveControlMessage>);

  /**
   *  TracedCallback signature for Transmitted Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* TxedEnbPhyCtrlMsgsTracedCallback)
      (const SfnSf sfn, const uint16_t rnti, const uint8_t bwpId, Ptr<MmWaveControlMessage>);

  uint32_t GetNumRbPerRbg () const override;
  uint32_t GetChannelBandwidth () const override;

private:
  /**
   * \brief Set the current slot pattern (better to call it only once..)
   * \param pattern the pattern
   *
   * It does not support dynamic change of pattern during the simulation
   */
  void SetTddPattern (const std::vector<LteNrTddSlotType> &pattern);

  void StartSlot (const SfnSf &startSlot);
  void EndSlot (void);

  void StartVarTti (const std::shared_ptr<DciInfoElementTdma> &dci);
  void EndVarTti (const std::shared_ptr<DciInfoElementTdma> &lastDci);

  void SendDataChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod,
                         const std::shared_ptr<DciInfoElementTdma> &dci);

  void SendCtrlChannels (const Time &varTtiPeriod);

  std::list <Ptr<MmWaveControlMessage>> RetrieveMsgsFromDCIs (const SfnSf &sfn) __attribute__((warn_unused_result));

  /**
   * \brief Channel access granted, invoked after the LBT
   *
   * \param time Time of the grant
   */
  void ChannelAccessGranted (const Time &time);

  /**
   * \brief Channel access lost, the grant has expired or the LBT denied the access
   */
  void ChannelAccessLost ();

  /**
   * \brief Transmit DL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of DL CTRL will end
   */
  Time DlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));
  /**
   * \brief Receive UL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of UL CTRL will end
   */
  Time UlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Transmit DL data and return the time at which the transmission will end
   * \param varTtiInfo the current varTti
   * \return the time at which the transmission of DL data will end
   */
  Time DlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Receive UL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of UL data will end
   */
  Time UlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Queue a MIB message, to be sent (hopefully) in this slot
   */
  void QueueMib ();
  /**
   * \brief Queue a SIB message, to be sent (hopefully) in this slot
   */
  void QueueSib ();

  /**
   * \brief Effectively start the slot, as we have the channel.
   */
  void DoStartSlot ();

  // LteEnbCphySapProvider forwarded methods
  void DoSetBandwidth (uint16_t ulBandwidth, uint16_t dlBandwidth);
  void DoSetEarfcn (uint16_t dlEarfcn, uint16_t ulEarfcn);
  void DoAddUe (uint16_t rnti);
  void DoRemoveUe (uint16_t rnti);
  void DoSetPa (uint16_t rnti, double pa);
  void DoSetTransmissionMode (uint16_t  rnti, uint8_t txMode);
  void DoSetSrsConfigurationIndex (uint16_t  rnti, uint16_t srcCi);
  void DoSetMasterInformationBlock (LteRrcSap::MasterInformationBlock mib);
  void DoSetSystemInformationBlockType1 (LteRrcSap::SystemInformationBlockType1 sib1);
  void DoSetBandwidth (uint16_t Bandwidth );
  void DoSetEarfcn (uint16_t Earfcn );

  /**
   * \brief Store the RBG allocation in the symStart, rbg map.
   * \param dci DCI
   *
   * The map will be used to change the subchannels each time the beam is changed.
   */
  void StoreRBGAllocation (const std::shared_ptr<DciInfoElementTdma> &dci);

  /**
   * \brief Generate the generate/send DCI structures from a pattern
   * \param pattern The pattern to analyze
   * \param toSendDl The structure toSendDl to fill
   * \param toSendUl The structure toSendUl to fill
   * \param generateDl The structure generateDl to fill
   * \param generateUl The structure generateUl to fill
   * \param dlHarqfbPosition The structure dlHarqfbPosition to fill
   * \param n0 N0 parameter
   * \param n2 N2 parameter
   * \param n1 N1 parameter
   * \param l1l2CtrlLatency L1L2CtrlLatency of the system
   */
  static void GenerateStructuresFromPattern (const std::vector<LteNrTddSlotType> &pattern,
                                             std::map<uint32_t, std::vector<uint32_t> > *toSendDl,
                                             std::map<uint32_t, std::vector<uint32_t> > *toSendUl,
                                             std::map<uint32_t, std::vector<uint32_t> > *generateDl,
                                             std::map<uint32_t, std::vector<uint32_t> > *generateUl,
                                             std::map<uint32_t, uint32_t> *dlHarqfbPosition,
                                             uint32_t n0, uint32_t n2, uint32_t n1, uint32_t l1l2CtrlLatency);

  /**
   * \brief Call MAC for retrieve the slot indication. Currently calls UL and DL.
   * \param currentSlot Current slot
   */
  void CallMacForSlotIndication (const SfnSf &currentSlot);

  /**
   * \brief Retrieve a DCI list for the allocation passed as parameter
   * \param alloc The allocation we are searching in
   * \param format The format of the DCI (UL or DL)
   * \param kDelay The K0 or K2 delay
   * \return A list of control messages that can be sent
   *
   * PS: This function ignores CTRL allocations.
   */
  std::list <Ptr<MmWaveControlMessage>>
  RetrieveDciFromAllocation (const SlotAllocInfo &alloc,
                             const DciInfoElementTdma::DciFormat &format,
                             uint32_t kDelay, uint32_t k1Delay);

  /**
   * \brief Insert a fake DL allocation in the allocation list
   * \param sfnSf The sfnSf to which we need a fake allocation
   *
   * Usually called at the beginning of the simulation to fill
   * the slot allocation queue until the generation take place
   */
  void PushDlAllocation (const SfnSf &sfnSf) const;

  /**
   * \brief Insert a fake UL allocation in the allocation list
   * \param sfnSf The sfnSf to which we need a fake allocation
   *
   * Usually called at the beginning of the simulation to fill
   * the slot allocation queue until the generation take place
   */
  void PushUlAllocation (const SfnSf &sfnSf) const;

  void StartEventLoop (uint16_t frame, uint8_t subframe, uint16_t slot);

private:
  MmWaveEnbPhySapUser* m_phySapUser {nullptr};           //!< MAC SAP user pointer, MAC is user of services of PHY, implements e.g. ReceiveRachPreamble
  LteEnbCphySapProvider* m_enbCphySapProvider {nullptr}; //!< PHY SAP provider pointer, PHY provides control services to RRC, RRC can call e.g SetBandwidth
  LteEnbCphySapUser* m_enbCphySapUser {nullptr};         //!< PHY CSAP user pointer, RRC can receive control information by PHY, currently configured but not used

  std::set <uint64_t> m_ueAttached; //!< Set of attached UE (by IMSI)
  std::set <uint16_t> m_ueAttachedRnti; //!< Set of attached UE (by RNTI)
  std::vector< Ptr<MmWaveUeNetDevice> > m_deviceMap; //!< Vector of UE devices

  LteRrcSap::SystemInformationBlockType1 m_sib1; //!< SIB1 message
  Time m_lastSlotStart; //!< Time at which the last slot started
  uint8_t m_currSymStart {0}; //!< Symbol at which the current allocation started
  std::unordered_map<uint8_t, std::vector<uint8_t> > m_rbgAllocationPerSym;  //!< RBG allocation in each sym

  TracedCallback< uint64_t, SpectrumValue&, SpectrumValue& > m_ulSinrTrace; //!< SINR trace

  /**
   * Trace information regarding Received Control Messages
   * Frame number, Subframe number, slot, VarTtti, rnti, bwpId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint8_t, Ptr<const MmWaveControlMessage>> m_phyRxedCtrlMsgsTrace;

  /**
   * Trace information regarding Transmitted Control Messages
   * Frame number, Subframe number, slot, VarTtti, rnti, bwpId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint8_t, Ptr<const MmWaveControlMessage>> m_phyTxedCtrlMsgsTrace;

  std::vector<LteNrTddSlotType> m_tddPattern = { F, F, F, F, F, F, F, F, F, F}; //!< Per-slot pattern

  std::map<uint32_t, std::vector<uint32_t>> m_toSendDl; //!< Map that indicates, for each slot, what DL DCI we have to send
  std::map<uint32_t, std::vector<uint32_t>> m_toSendUl; //!< Map that indicates, for each slot, what UL DCI we have to send
  std::map<uint32_t, std::vector<uint32_t>> m_generateUl; //!< Map that indicates, for each slot, what UL DCI we have to generate
  std::map<uint32_t, std::vector<uint32_t>> m_generateDl; //!< Map that indicates, for each slot, what DL DCI we have to generate

  std::map<uint32_t, uint32_t> m_dlHarqfbPosition; //!< Map that indicates, for each DL slot, where the UE has to send the Harq Feedback

  /**
   * \brief Status of the channel for the PHY
   */
  enum ChannelStatus
  {
    NONE,        //!< The PHY doesn't know the channel status
    REQUESTED,   //!< The PHY requested channel access
    GRANTED,      //!< The PHY has the channel, it can transmit
    TO_LOSE
  };

  ChannelStatus m_channelStatus {NONE}; //!< The channel status
  EventId m_channelLostTimer; //!< Timer that, when expires, indicates that the channel is lost

  Ptr<NrChAccessManager> m_cam; //!< Channel Access Manager

  friend class LtePatternTestCase;

  uint32_t m_n0Delay {0}; //!< minimum processing delay (in slots) needed to decode DL DCI and decode DL data (UE side)
  uint32_t m_n1Delay {0}; //!< minimum processing delay (in slots) from the end of DL Data reception to the earliest possible start of the corresponding ACK/NACK transmission (UE side)
  uint32_t m_n2Delay {0}; //!< minimum processing delay (in slots) needed to decode UL DCI and prepare UL data (UE side)

  uint16_t m_channelBandwidth {200};  //!< Value in kHz * 100. Set by RRC. Default to 20 MHz

  SfnSf m_currentSlot;
};

}


#endif /* MMWAVE_ENB_PHY_H */
