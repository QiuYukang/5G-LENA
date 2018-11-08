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
 *             Sourjya Dutta <sdutta@nyu.edu>
 *             Russell Ford <russell.ford@nyu.edu>
 *            Menglei Zhang <menglei@nyu.edu>
 */


#ifndef SRC_MMWAVE_MODEL_MMWAVE_ENB_PHY_H_
#define SRC_MMWAVE_MODEL_MMWAVE_ENB_PHY_H_


#include "mmwave-phy.h"
#include "mmwave-phy-mac-common.h"
#include "mmwave-control-messages.h"
#include <ns3/lte-enb-phy-sap.h>
#include <ns3/lte-enb-cphy-sap.h>
#include <ns3/mmwave-harq-phy.h>

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
   * \brief Create Tx Power Spectral Density
   * \param rbIndexVector vector of the index of the RB (in SpectrumValue array)
   * in which there is a transmission
   * \return A SpectrumValue array with fixed size, in which each value
   * is updated to a particular value if the correspond RB index was inside the rbIndexVector,
   * or is left untouched otherwise.
   * \see MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity
   */
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensity (const std::vector<int> &rbIndexVector) const override;

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

  void SendCtrlChannels (std::list<Ptr<MmWaveControlMessage> > ctrlMsg, Time varTtiPeriod);

  Ptr<MmWaveSpectrumPhy> GetDlSpectrumPhy () const;
  Ptr<MmWaveSpectrumPhy> GetUlSpectrumPhy () const;

  /**virtual void SendIdealControlMessage(Ptr<IdealControlMessage> msg);
virtual void ReceiveIdealControlMessage(Ptr<IdealControlMessage> msg)**/

  bool AddUePhy (uint64_t imsi, Ptr<NetDevice> ueDevice);

  // void SetMacPdu (Ptr<Packet> pb);

  void PhyDataPacketReceived (Ptr<Packet> p);

  void GenerateDataCqiReport (const SpectrumValue& sinr);

  void PhyCtrlMessagesReceived (std::list<Ptr<MmWaveControlMessage> > msgList);

  int8_t DoGetReferenceSignalPower () const;

  void SetPhySapUser (MmWaveEnbPhySapUser* ptr);

  void SetHarqPhyModule (Ptr<MmWaveHarqPhy> harq);

  void ReceiveUlHarqFeedback (UlHarqInfo mes);


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
};

}


#endif /* SRC_MMWAVE_MODEL_MMWAVE_ENB_PHY_H_ */
