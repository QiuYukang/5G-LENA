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

namespace ns3 {

class MmWaveEnbPhy : public MmWavePhy
{
  friend class MemberLteEnbCphySapProvider<MmWaveEnbPhy>;
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
  MmWaveEnbPhy (Ptr<MmWaveSpectrumPhy>, Ptr<MmWaveSpectrumPhy>, const Ptr<Node> &);

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
   * \brief Get the BeamId for the selected user
   * \param rnti the selected user
   * \return the beam id of the user
   */
  AntennaArrayModel::BeamId GetBeamId (uint16_t rnti) const override;

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
   * \brief Retrieve the DlSpectrumPhy pointer
   *
   * As this function is used mainly to get traced values out of DlSpectrum,
   * it should be removed and the traces connected (and redirected) here.
   * \return A pointer to the DlSpectrumPhy of this UE
   */
  virtual Ptr<MmWaveSpectrumPhy> GetDlSpectrumPhy () const override __attribute__((warn_unused_result));

  /**
   * \brief Add the UE to the list of this gnb UEs.
   *
   * Usually called by the helper when a UE register to this gnb.
   * \param imsi IMSI of the device
   * \param ueDevice Device
   * \return
   */
  bool RegisterUe (uint64_t imsi, const Ptr<NetDevice> &ueDevice);

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
  void PhyCtrlMessagesReceived (const std::list<Ptr<MmWaveControlMessage> > &msgList);

  /**
   * \brief Get the power of the enb
   * \return the power
   */
  int8_t DoGetReferenceSignalPower () const;

  /**
   * \brief Install the PHY sap user (AKA the UE MAC)
   *
   * \param ptr the PHY SAP user pointer to install
   */
  void SetPhySapUser (MmWaveEnbPhySapUser* ptr);

  /**
   * \brief Receive the HARQ feedback on the transmission
   *
   * Connected by the helper to a spectrum phy callback
   *
   * \param m the HARQ feedback
   */
  void ReceiveUlHarqFeedback (const UlHarqInfo &mes);

  /**
   * \brief Signature for a "PerformBeamforming" function
   */
  typedef std::function<void (const Ptr<NetDevice> &a, const Ptr<NetDevice> &b)> PerformBeamformingFn;

  /**
   * \brief Install the function to perform a beamforming between two devices
   *
   * Usually done by the helper
   * \param fn Function to install
   */
  void SetPerformBeamformingFn (const PerformBeamformingFn &fn);

protected:
  // From object
  virtual void DoDispose (void) override;
  virtual void DoInitialize (void) override;

private:
  void StartSlot (uint16_t frameNum, uint8_t sfNum, uint16_t slotNum);
  void EndSlot (void);

  void StartVarTti (void);
  void EndVarTti (void);

  void SendDataChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod,
                         const VarTtiAllocInfo &varTtiInfo);

  void SendCtrlChannels (std::list<Ptr<MmWaveControlMessage> > *ctrlMsgs,
                         const Time &varTtiPeriod);

  std::list <Ptr<MmWaveControlMessage>> RetrieveMsgsFromDCIs (const SfnSf &sfn) __attribute__((warn_unused_result));

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
  Time DlData (const VarTtiAllocInfo &varTtiInfo) __attribute__((warn_unused_result));

  /**
   * \brief Receive UL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of UL data will end
   */
  Time UlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Store the RBG allocation in the symStart, rbg map.
   * \param dci DCI
   *
   * The map will be used to change the subchannels each time the beam is changed.
   */
  void StoreRBGAllocation (const std::shared_ptr<DciInfoElementTdma> &dci);

  /**
   * \brief The beamforming timer has expired; at the next slot, perform beamforming.
   *
   * This function just set to true a boolean variable that will be checked in
   * StartVarTti().
   */
  void ExpireBeamformingTimer ();

  // LteEnbCphySapProvider forwarded methods
  void DoSetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth);
  void DoSetEarfcn (uint16_t dlEarfcn, uint16_t ulEarfcn);
  void DoAddUe (uint16_t rnti);
  void DoRemoveUe (uint16_t rnti);
  void DoSetPa (uint16_t rnti, double pa);
  void DoSetTransmissionMode (uint16_t  rnti, uint8_t txMode);
  void DoSetSrsConfigurationIndex (uint16_t  rnti, uint16_t srcCi);
  void DoSetMasterInformationBlock (LteRrcSap::MasterInformationBlock mib);
  void DoSetSystemInformationBlockType1 (LteRrcSap::SystemInformationBlockType1 sib1);
  void DoSetBandwidth (uint8_t Bandwidth );
  void DoSetEarfcn (uint16_t Earfcn );

private:
  MmWaveEnbPhySapUser* m_phySapUser;           //!< SAP pointer
  LteEnbCphySapProvider* m_enbCphySapProvider; //!< SAP pointer
  LteEnbCphySapUser* m_enbCphySapUser;         //!< SAP pointer

  std::set <uint64_t> m_ueAttached; //!< Set of attached UE (by IMSI)
  std::set <uint16_t> m_ueAttachedRnti; //!< Set of attached UE (by RNTI)
  std::vector< Ptr<NetDevice> > m_deviceMap; //!< Vector of UE devices

  LteRrcSap::SystemInformationBlockType1 m_sib1; //!< SIB1 message
  Time m_lastSlotStart; //!< Time at which the last slot started
  uint8_t m_currSymStart {0}; //!< Symbol at which the current allocation started
  std::unordered_map<uint8_t, std::vector<uint8_t> > m_rbgAllocationPerSym;  //!< RBG allocation in each sym

  bool m_performBeamforming {true}; //!< True when we have to do beamforming. Default to true or we will not perform beamforming the first time..
  Time m_beamformingPeriodicity; //!< Periodicity of beamforming (0 for never)
  EventId m_beamformingTimer;    //!< Beamforming timer
  PerformBeamformingFn m_doBeamforming; //!< Beamforming function

  TracedCallback< uint64_t, SpectrumValue&, SpectrumValue& > m_ulSinrTrace; //!< SINR trace
};

}


#endif /* MMWAVE_ENB_PHY_H */
