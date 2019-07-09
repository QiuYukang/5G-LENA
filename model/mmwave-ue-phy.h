/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015 NYU WIRELESS, Tandon School of Engineering, New York University
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

#ifndef MMWAVE_UE_PHY_H
#define MMWAVE_UE_PHY_H

#include <ns3/mmwave-phy.h>
#include "nr-amc.h"
#include <ns3/lte-ue-phy-sap.h>
#include <ns3/lte-ue-cphy-sap.h>
#include <ns3/mmwave-harq-phy.h>
#include <ns3/antenna-array-3gpp-model.h>

namespace ns3 {

class NrChAccessManager;

class MmWaveUePhy : public MmWavePhy
{
  friend class UeMemberLteUePhySapProvider;
  friend class MemberLteUeCphySapProvider<MmWaveUePhy>;

public:
  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoInitialize (void) override; // Public because it's called by hand,
                                             // and not by aggregation, in MmWaveNetDevice

  /**
   * \brief MmWaveUePhy default constructor. Is there for ns-3 object system, but should not be used.
   */
  MmWaveUePhy ();

  /**
   * \brief MmWaveUePhy real constructor
   * \param channelPhy spectrum phy
   * \param n Pointer to the node owning this instance
   *
   * Usually called by the helper. It starts the event loop for the ue.
   */
  MmWaveUePhy (Ptr<MmWaveSpectrumPhy> channelPhy, const Ptr<Node> &n);

  /**
   * \brief ~MmWaveUePhy
   */
  virtual ~MmWaveUePhy () override;

  /**
   * \brief Retrieve the pointer for the C PHY SAP provider (AKA the PHY interface towards the RRC)
   * \return the C PHY SAP pointer
   */
  LteUeCphySapProvider* GetUeCphySapProvider () __attribute__((warn_unused_result));

  /**
   * \brief Install ue C PHY SAP user (AKA the PHY interface towards the RRC)
   * \param s the C PHY SAP user pointer to install
   */
  void SetUeCphySapUser (LteUeCphySapUser* s);

  /**
   * \brief Install the PHY sap user (AKA the UE MAC)
   *
   * \param ptr the PHY SAP user pointer to install
   */
  void SetPhySapUser (MmWaveUePhySapUser* ptr);

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
   * \brief Register the UE to a certain Enb
   *
   * Install the configuration parameters in the UE. At the moment, the code
   * does not reconfigure itself when the PhyMacCommon parameters change,
   * so you can call this function only one (therefore, no handoff)
   *
   * \param cellId the CELL ID of the ENB
   * \param config the ENB configuration
   */
  void RegisterToEnb (uint16_t cellId, Ptr<MmWavePhyMacCommon> config);

  /**
   * \brief Retrieve the SpectrumPhy pointer
   *
   * As this function is used mainly to get traced values out of Spectrum,
   * it should be removed and the traces connected (and redirected) here.
   * \return A pointer to the SpectrumPhy of this UE
   */
  virtual Ptr<MmWaveSpectrumPhy> GetSpectrumPhy () const override __attribute__((warn_unused_result));

  /**
   * \brief Receive a list of CTRL messages
   *
   * Connected by the helper to a callback of the spectrum.
   *
   * \param msgList message list
   */
  void PhyCtrlMessagesReceived (const std::list<Ptr<MmWaveControlMessage> > &msgList);

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
  void GenerateDlCqiReport (const SpectrumValue& sinr);

  /**
   * \brief Get the current RNTI of the user
   *
   * \return the current RNTI of the user
   */
  uint16_t GetRnti () __attribute__((warn_unused_result));

  /**
   * \brief Receive the HARQ feedback on the transmission
   *
   * Connected by the helper to a spectrum phy callback
   *
   * \param m the HARQ feedback
   */
  void ReceiveLteDlHarqFeedback (const DlHarqInfo &m);

  /**
   *  TracedCallback signature for Ue Phy Received Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] ccId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* RxedUePhyCtrlMsgsTracedCallback)
      (const SfnSf sfnSf, const uint16_t rnti, const uint8_t ccId, Ptr<MmWaveControlMessage>);

  /**
   *  TracedCallback signature for Ue Phy Transmitted Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] ccId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* TxedUePhyCtrlMsgsTracedCallback)
      (const SfnSf sfnSf, const uint16_t rnti, const uint8_t ccId, Ptr<MmWaveControlMessage>);

  /**
   * \brief Set the channel access manager interface for this instance of the PHY
   * \param s the pointer to the interface
   */
  void SetCam (const Ptr<NrChAccessManager> &cam);

  // From mmwave phy. Not used in the UE
  virtual AntennaArrayModel::BeamId GetBeamId (uint16_t rnti) const override;

protected:
  // From object
  virtual void DoDispose (void) override;

private:
  /**
   * \brief Channel access granted, invoked after the LBT
   *
   * \param time Time of the grant
   */
  void ChannelAccessGranted (const Time &time);

  /**
   * \brief Channel access denied
   */
  void ChannelAccessDenied ();

  /**
   * \brief RequestAccess
   */
  void RequestAccess ();

  /**
   * \brief Forward the received RAR to the MAC
   * \param rarMsg RAR message
   */
  void DoReceiveRar (Ptr<MmWaveRarMessage> rarMsg);
  /**
   * \brief Create a DlCqiFeedback message
   * \param sinr the SINR value
   * \return a CTRL message with the CQI feedback
   */
  Ptr<MmWaveDlCqiMessage> CreateDlCqiFeedbackMessage (const SpectrumValue& sinr) __attribute__((warn_unused_result));
  /**
   * \brief Receive DL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of DL CTRL will end
   */
  Time DlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));
  /**
   * \brief Transmit UL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of UL CTRL will end
   */
  Time UlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Receive DL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of DL data will end
   */
  Time DlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Transmit UL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of UL data will end
   */
  Time UlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Try to perform an lbt before UL CTRL
   *
   * This function should be called after we receive the DL_DCI for the slot,
   * and then checks if we can re-use the channel through shared MCOT. Otherwise,
   * schedule an LBT before the transmission of the UL CTRL.
   */
  void TryToPerformLbt ();

  void StartSlot (uint16_t frameNum, uint8_t subframeNum, uint16_t slotNum);
  void StartVarTti ();
  void EndVarTti ();
  void SetSubChannelsForTransmission (std::vector <int> mask);
  void DoSendControlMessage (Ptr<MmWaveControlMessage> msg);
  void SendDataChannels (Ptr<PacketBurst> pb, std::list<Ptr<MmWaveControlMessage> > ctrlMsg, Time duration, uint8_t slotInd);
  void SendCtrlChannels (std::list<Ptr<MmWaveControlMessage> > ctrlMsg, Time prd);

  // SAP methods
  void DoReset ();
  void DoStartCellSearch (uint16_t dlEarfcn);
  void DoSynchronizeWithEnb (uint16_t cellId);
  void DoSynchronizeWithEnb (uint16_t cellId, uint16_t dlEarfcn);
  void DoSetPa (double pa);
  /**
   * \param rsrpFilterCoefficient value. Determines the strength of
   * smoothing effect induced by layer 3 filtering of RSRP
   * used for uplink power control in all attached UE.
   * If equals to 0, no layer 3 filtering is applicable.
   */
  void DoSetRsrpFilterCoefficient (uint8_t rsrpFilterCoefficient);
  void DoSetDlBandwidth (uint8_t ulBandwidth);
  void DoConfigureUplink (uint16_t ulEarfcn, uint8_t ulBandwidth);
  void DoConfigureReferenceSignalPower (int8_t referenceSignalPower);
  void DoSetRnti (uint16_t rnti);
  void DoSetTransmissionMode (uint8_t txMode);
  void DoSetSrsConfigurationIndex (uint16_t srcCi);

private:
  MmWaveUePhySapUser* m_phySapUser;             //!< SAP pointer
  LteUeCphySapProvider* m_ueCphySapProvider;    //!< SAP pointer
  LteUeCphySapUser* m_ueCphySapUser;            //!< SAP pointer

  Ptr<NrAmc> m_amc;  //!< AMC model used to compute the CQI feedback

  Time m_wbCqiPeriod;       /**< Wideband Periodic CQI: 2, 5, 10, 16, 20, 32, 40, 64, 80 or 160 ms */
  Time m_wbCqiLast;
  Time m_lastSlotStart; //!< Time of the last slot start

  bool m_ulConfigured {false};     //!< Flag to indicate if RRC configured the UL
  bool m_receptionEnabled {false}; //!< Flag to indicate if we are currently receiveing data
  uint16_t m_rnti {0};             //!< Current RNTI of the user
  uint32_t m_currTbs {0};          //!< Current TBS of the receiveing DL data (used to compute the feedback)

  /**
   * \brief Status of the channel for the PHY
   */
  enum ChannelStatus
  {
    NONE,        //!< The PHY doesn't know the channel status
    REQUESTED,   //!< The PHY requested channel access
    GRANTED      //!< The PHY has the channel, it can transmit
  };

  ChannelStatus m_channelStatus {NONE}; //!< The channel status
  Ptr<NrChAccessManager> m_cam; //!< Channel Access Manager
  Time m_lbtThresholdForCtrl; //!< Threshold for LBT before the UL CTRL
  bool m_tryToPerformLbt {false}; //!< Boolean value set in DlCtrl() method

  TracedCallback< uint64_t, SpectrumValue&, SpectrumValue& > m_reportCurrentCellRsrpSinrTrace; //!< Report the rsrp
  TracedCallback<uint64_t, uint64_t> m_reportUlTbSize; //!< Report the UL TBS
  TracedCallback<uint64_t, uint64_t> m_reportDlTbSize; //!< Report the DL TBS

  /**
   * Trace information regarding Ue PHY Received Control Messages
   * Frame number, Subframe number, slot, VarTtti, rnti, ccId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint8_t, Ptr<const MmWaveControlMessage>> m_phyRxedCtrlMsgsTrace;

  /**
   * Trace information regarding Ue PHY Transmitted Control Messages
   * Frame number, Subframe number, slot, VarTtti, rnti, ccId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint8_t, Ptr<const MmWaveControlMessage>> m_phyTxedCtrlMsgsTrace;
};

}

#endif /* MMWAVE_UE_PHY_H */
