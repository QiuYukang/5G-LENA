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

#ifndef SRC_MMWAVE_MODEL_MMWAVE_PHY_H_
#define SRC_MMWAVE_MODEL_MMWAVE_PHY_H_

#include "mmwave-phy-sap.h"

namespace ns3 {

class MmWaveNetDevice;
class MmWaveControlMessage;
class MmWavePhyMacCommon;
class MmWaveSpectrumPhy;
class AntennaArrayBasicModel;
class MmWavePhyMacCommon;
class BeamManager;
class ThreeGppAntennaArrayModel;

class MmWavePhy : public Object
{
public:
  MmWavePhy ();

  MmWavePhy (Ptr<MmWaveSpectrumPhy> channelPhy);

  virtual ~MmWavePhy () override;

  static TypeId GetTypeId (void);

  // Called by SAP
  /**
   * \brief Enqueue a ctrl message, keeping in consideration L1L2CtrlDelay
   * \param m the message to enqueue
   */
  void EnqueueCtrlMessage (const Ptr<MmWaveControlMessage> &m);

  void SetMacPdu (Ptr<Packet> pb);
  void SendRachPreamble (uint32_t PreambleId, uint32_t Rnti);

  /**
   * \brief Store the slot allocation info
   * \param slotAllocInfo the allocation to store
   *
   * This method expect that the sfn of the allocation will match the sfn
   * when the allocation will be retrieved.
   */
  void PushBackSlotAllocInfo (const SlotAllocInfo &slotAllocInfo);

  /**
   * \brief Notify PHY about the successful RRC connection
   * establishment.
   */
  void NotifyConnectionSuccessful ();

  virtual BeamId GetBeamId (uint16_t rnti) const = 0;

  Ptr<BeamManager> GetBeamManager ();

  // Attributes
  /**
   * \return The antena array that is being used by this PHY
   */
  Ptr<ThreeGppAntennaArrayModel> GetAntennaArray () const;

  // Installation / Helpers
  MmWavePhySapProvider* GetPhySapProvider ();

  void SetDevice (Ptr<MmWaveNetDevice> d);

  Ptr<MmWavePhyMacCommon> GetConfigurationParameters (void) const;

  virtual Ptr<MmWaveSpectrumPhy> GetSpectrumPhy () const = 0;

  // SAP
  void DoSetCellId (uint16_t cellId);

protected:

  virtual void DoDispose () override;

  /**
   * \brief Transform a MAC-made vector of RBG to a PHY-ready vector of SINR indices
   * \param rbgBitmask Bitmask which indicates with 1 the RBG in which there is a transmission,
   * with 0 a RBG in which there is not a transmission
   * \return a vector of indices.
   *
   * Example (4 RB per RBG, 4 total RBG assignable):
   * rbgBitmask = <0,1,1,0>
   * output = <4,5,6,7,8,9,10,11>
   *
   * (the rbgBitmask expressed as rbBitmask would be:
   * <0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0> , and therefore the places in which there
   * is a 1 are from the 4th to the 11th, and that is reflected in the output)
   */
  std::vector<int> FromRBGBitmaskToRBAssignment (const std::vector<uint8_t> rbgBitmask) const;

  void InstallBeamManager ();

  Ptr<PacketBurst> GetPacketBurst (SfnSf);

  /**
   * \brief Create Noise Power Spectral density
   * \return A SpectrumValue array with fixed size, in which a value is
   * update to a particular value of the noise
   */
  Ptr<SpectrumValue> GetNoisePowerSpectralDensity ();

  /**
   * Create Tx Power Spectral Density
   * \param rbIndexVector vector of the index of the RB (in SpectrumValue array)
   * in which there is a transmission
   * \return A SpectrumValue array with fixed size, in which each value
   * is updated to a particular value if the correspond RB index was inside the rbIndexVector,
   * or is left untouched otherwise.
   * \see MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity
   */
  Ptr<SpectrumValue> GetTxPowerSpectralDensity (const std::vector<int> &rbIndexVector) const;

  /**
   * \brief Get the component carrier ID
   * \return the component carrier ID
   *
   * Take the value from PhyMacCommon. If it's not set, then return 777.
   */
  uint32_t GetCcId () const;

  /**
   * \brief Store the slot allocation info at the front
   * \param slotAllocInfo the allocation to store
   *
   * Increase the sfn of all allocations to be chronologically "in order".
   */
  void PushFrontSlotAllocInfo (const SfnSf &newSfnSf, const SlotAllocInfo &slotAllocInfo);

  /**
   * \brief Check if the SlotAllocationInfo for that slot exists
   * \param sfnsf slot to check
   * \return true if the allocation exists
   */
  bool SlotAllocInfoExists (const SfnSf &sfnsf) const;

  /**
   * \brief Get the head for the slot allocation info, and delete it from the
   * internal list
   * \return the Slot allocation info head
   */
  SlotAllocInfo RetrieveSlotAllocInfo ();

  /**
   * \brief Get the SlotAllocationInfo for the specified slot, and delete it
   * from the internal list
   *
   * \param sfnsf slot specified
   * \return the SlotAllocationInfo
   */
  SlotAllocInfo RetrieveSlotAllocInfo (const SfnSf &sfnsf);

  /**
   * \brief Peek the SlotAllocInfo at the SfnSf specified
   * \param sfnsf (existing) SfnSf to look for
   * \return a reference to the SlotAllocInfo
   *
   * The method will assert if sfnsf does not exits (please check with SlotExists())
   */
  SlotAllocInfo & PeekSlotAllocInfo (const SfnSf & sfnsf);

  /**
   * \brief Retrieve the size of the SlotAllocInfo list
   * \return the allocation list size
   */
  size_t SlotAllocInfoSize () const;

  /**
   * \brief Check if there are no control messages queued for this slot
   * \return true if there are no control messages queued for this slot
   */
  bool IsCtrlMsgListEmpty () const;

  /**
   * \brief Enqueue a CTRL message without considering L1L2CtrlLatency
   * \param msg The message to enqueue
   */
  void EnqueueCtrlMsgNow (const Ptr<MmWaveControlMessage> &msg);

  /**
   * @brief Enqueue a CTRL message without considering L1L2CtrlLatency
   * @param listOfMsgs The list of messages to enqueue
   */
  void EnqueueCtrlMsgNow (const std::list<Ptr<MmWaveControlMessage> > &listOfMsgs);

  /**
   * \brief Initialize the message list
   */
  void InitializeMessageList ();
  /**
   * \brief Extract and return the message list that is at the beginning of the queue
   * \return a list of control messages that are meant to be sent in the current slot
   */
  virtual std::list<Ptr<MmWaveControlMessage> > PopCurrentSlotCtrlMsgs (void);

protected:
  Ptr<MmWaveNetDevice> m_netDevice;
  Ptr<MmWaveSpectrumPhy> m_spectrumPhy;

  double m_txPower {0.0};
  double m_noiseFigure {0.0};

  uint16_t m_cellId {0};

  Ptr<MmWavePhyMacCommon> m_phyMacConfig;

  std::unordered_map<uint64_t, Ptr<PacketBurst> > m_packetBurstMap;

  SlotAllocInfo m_currSlotAllocInfo;
  uint16_t m_frameNum {0};
  uint8_t m_subframeNum {0};
  uint8_t m_slotNum {0};
  uint8_t m_varTtiNum {0};

  MmWavePhySapProvider* m_phySapProvider;

  uint32_t m_raPreambleId {0};
  bool m_isConnected {false}; ///< set when UE RRC is in CONNECTED_NORMALLY state
  Ptr<BeamManager> m_beamManager; //!< TODO

private:
  std::list<SlotAllocInfo> m_slotAllocInfo; //!< slot allocation info list
  std::vector<std::list<Ptr<MmWaveControlMessage>>> m_controlMessageQueue; //!< CTRL message queue

public:
  bool m_areIsotropicElements {false}; //!< Whether to use isotropic antenna elements, default is false which means that 3gpp antenna elements will be used
  uint8_t m_antennaNumDim1 {0};
  uint8_t m_antennaNumDim2 {0};
};

}


#endif /* SRC_MMWAVE_MODEL_MMWAVE_PHY_H_ */
