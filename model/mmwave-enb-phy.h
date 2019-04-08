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


#ifndef SRC_MMWAVE_MODEL_MMWAVE_ENB_PHY_H_
#define SRC_MMWAVE_MODEL_MMWAVE_ENB_PHY_H_


#include "mmwave-phy.h"
#include "mmwave-phy-mac-common.h"
#include "mmwave-control-messages.h"
#include <ns3/lte-enb-phy-sap.h>
#include <ns3/lte-enb-cphy-sap.h>
#include <ns3/mmwave-harq-phy.h>
#include <functional>

namespace ns3 {

class PacketBurst;
class MmWaveNetDevice;
class MmWaveUePhy;
class MmWaveEnbMac;

class MmWaveEnbPhy : public MmWavePhy
{
  friend class MemberLteEnbCphySapProvider<MmWaveEnbPhy>;
public:
  MmWaveEnbPhy ();

  MmWaveEnbPhy (Ptr<MmWaveSpectrumPhy>, Ptr<MmWaveSpectrumPhy>, const Ptr<Node> &);
  virtual ~MmWaveEnbPhy () override;

  static TypeId GetTypeId (void);
  virtual void DoInitialize (void) override;
  virtual void DoDispose (void) override;

  void SetmmWaveEnbCphySapUser (LteEnbCphySapUser* s);
  LteEnbCphySapProvider* GetmmWaveEnbCphySapProvider ();

  AntennaArrayModel::BeamId GetBeamId (uint8_t rnti) const override;

  void SetTxPower (double pow);
  double GetTxPower () const;

  void SetNoiseFigure (double pf);
  double GetNoiseFigure () const;

  void CalcChannelQualityForUe (std::vector <double> sinr, Ptr<MmWaveSpectrumPhy> ue);
  /**
   * \brief Set the Tx power spectral density based on the RB index vector
   * \param rbIndexVector vector of the index of the RB (in SpectrumValue array)
   * in which there is a transmission
   */
  void SetSubChannels (const std::vector<int> &rbIndexVector);

  void StartSlot (void);
  void EndSlot (void);

  void StartVarTti (void);
  void EndVarTti (void);


  void SendDataChannels (Ptr<PacketBurst> pb, Time varTtiPeriod, VarTtiAllocInfo& varTtiInfo);

  void SendCtrlChannels (const std::list<Ptr<MmWaveControlMessage> > & ctrlMsg,
                         const Time &varTtiPeriod);

  virtual Ptr<MmWaveSpectrumPhy> GetDlSpectrumPhy () const override;

  Ptr<MmWaveSpectrumPhy> GetUlSpectrumPhy () const;

  bool AddUePhy (uint64_t imsi, Ptr<NetDevice> ueDevice);

  void PhyDataPacketReceived (Ptr<Packet> p);

  void GenerateDataCqiReport (const SpectrumValue& sinr);

  void PhyCtrlMessagesReceived (std::list<Ptr<MmWaveControlMessage> > msgList);

  int8_t DoGetReferenceSignalPower () const;

  void SetPhySapUser (MmWaveEnbPhySapUser* ptr);

  void SetHarqPhyModule (Ptr<MmWaveHarqPhy> harq);

  void ReceiveUlHarqFeedback (UlHarqInfo mes);

  /**
   * \brief Signature for a "PerformBeamforming" function
   */
  typedef std::function<void (const Ptr<NetDevice> &a, const Ptr<NetDevice> &b)> PerformBeamformingFn;

  /**
   * \brief Install the function to perform a beamforming between two devices
   * \param fn Function to install
   */
  void SetPerformBeamformingFn (const PerformBeamformingFn &fn);

private:
  std::list <Ptr<MmWaveControlMessage> > RetrieveMsgsFromDCIs (const SfnSf &sfn);

  bool AddUePhy (uint16_t rnti);
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
  void QueueUlTbAlloc (TbAllocInfo tbAllocInfo);
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

  std::list<TbAllocInfo> DequeueUlTbAlloc ();

  uint8_t m_currSfNumVarTtis;

  uint32_t m_numRbg;

  std::set <uint64_t> m_ueAttached;

  uint8_t m_prevVarTti;   // 1->UL 0->DL 2->Unspecified

  VarTtiAllocInfo::TddMode m_prevVarTtiDir;

  std::vector< Ptr<NetDevice> > m_deviceMap;

  MmWaveEnbPhySapUser* m_phySapUser;

  LteEnbCphySapProvider* m_enbCphySapProvider;

  LteEnbCphySapUser* m_enbCphySapUser;

  LteRrcSap::SystemInformationBlockType1 m_sib1;

  std::set <uint16_t> m_ueAttachedRnti;

  Ptr<MmWaveHarqPhy> m_harqPhyModule;

  Time m_lastSlotStart;

  uint8_t m_currSymStart;

  TracedCallback< uint64_t, SpectrumValue&, SpectrumValue& > m_ulSinrTrace;

  std::unordered_map<uint8_t, std::vector<uint8_t> > m_rbgAllocationPerSym;  //!< RBG allocation in each sym

  bool m_performBeamforming {true}; //!< True when we have to do beamforming. Default to true or we will not perform beamforming the first time..
  Time m_beamformingPeriodicity; //!< Periodicity of beamforming (0 for never)
  EventId m_beamformingTimer;    //!< Beamforming timer
  PerformBeamformingFn m_doBeamforming; //!< Beamforming function

  std::list <Ptr<MmWaveControlMessage> > m_ctrlMsgs; //!< DL CTRL messages to be sent
};

}


#endif /* SRC_MMWAVE_MODEL_MMWAVE_ENB_PHY_H_ */
