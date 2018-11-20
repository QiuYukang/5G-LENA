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
 *                        Sourjya Dutta <sdutta@nyu.edu>
 *                        Russell Ford <russell.ford@nyu.edu>
 *                        Menglei Zhang <menglei@nyu.edu>
 */



#ifndef SRC_MMWAVE_MODEL_MMWAVE_PHY_H_
#define SRC_MMWAVE_MODEL_MMWAVE_PHY_H_

#include <ns3/spectrum-value.h>
#include <ns3/mobility-model.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-signal-parameters.h>
#include <ns3/spectrum-interference.h>
#include <ns3/generic-phy.h>
#include <ns3/antenna-array-model.h>
#include "mmwave-phy-mac-common.h"
#include "mmwave-spectrum-phy.h"
#include "mmwave-phy-sap.h"
#include <string>
#include <map>

namespace ns3 {

class MmWaveNetDevice;
class MmWaveControlMessage;

class MmWavePhy : public Object
{
public:
  MmWavePhy ();

  MmWavePhy (Ptr<MmWaveSpectrumPhy> dlChannelPhy, Ptr<MmWaveSpectrumPhy> ulChannelPhy);

  virtual ~MmWavePhy ();

  static TypeId GetTypeId (void);

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

  void SetDevice (Ptr<MmWaveNetDevice> d);

  Ptr<MmWaveNetDevice> GetDevice ();

  void SetChannel (Ptr<SpectrumChannel> c);

  /**
   * \brief Create Tx Power Spectral Density
   * \param rbIndexVector vector of the index of the RB (in SpectrumValue array)
   * in which there is a transmission
   * \return A SpectrumValue array representing the TX Power Spectral Density
   * in W/Hz for each Resource Block, in which each array value
   * is updated to a particular value if the correspond RB index was inside the rbIndexVector,
   * or is left untouched otherwise.
   * \see MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity
   */
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensity (const std::vector<int> &rbIndexVector) const = 0;

  void DoDispose ();

  virtual void DoInitialize (void);

  /*	*
   * \returns transmission time interval

      double GetTti (void) const;*/

  void DoSetCellId (uint16_t cellId);


  void SetNoiseFigure (double nf);
  double GetNoiseFigure (void) const;

  void SetControlMessage (Ptr<MmWaveControlMessage> m);
  std::list<Ptr<MmWaveControlMessage> > GetControlMessages (void);

  virtual void SetMacPdu (Ptr<Packet> pb);

  virtual void SendRachPreamble (uint32_t PreambleId, uint32_t Rnti);


  //	virtual Ptr<PacketBurst> GetPacketBurst (void);
  virtual Ptr<PacketBurst> GetPacketBurst (SfnSf);


  /**
   * \brief Get the component carrier ID
   * \return the component carrier ID
   *
   * Take the value from PhyMacCommon. If it's not set, then return 777.
   */
  uint32_t GetCcId () const;
  void SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig);
  Ptr<MmWavePhyMacCommon> GetConfigurationParameters (void) const;

  MmWavePhySapProvider* GetPhySapProvider ();
  //	void SetPhySapUser (MmWavePhySapUser* ptr);

  void UpdateCurrentAllocationAndSchedule (uint32_t frame, uint32_t sf);

  void SetSlotAllocInfo (const SlotAllocInfo &slotAllocInfo);

  bool SlotExists (const SfnSf &retVal) const;

  SlotAllocInfo GetSlotAllocInfo (const SfnSf &sfnsf);
  /**
   * \brief Peek the SlotAllocInfo at the SfnSf specified
   * \param sfnsf (existing) SfnSf to look for
   * \return a reference to the SlotAllocInfo
   *
   * The method will assert if sfnsf does not exits (please check with SlotExists())
   */
  SlotAllocInfo & PeekSlotAllocInfo (const SfnSf & sfnsf);

  virtual AntennaArrayModel::BeamId GetBeamId (uint8_t rnti) const = 0;

  /**
  * Set the component carrier ID
  *
  * \param index the component carrier ID index
  */
  void SetComponentCarrierId (uint8_t index);

  /**
  * Get the component carrier ID
  *
  * \returns the component carrier ID index
  */
  uint8_t GetComponentCarrierId ();

protected:
  Ptr<MmWaveNetDevice> m_netDevice;
  Ptr<MmWaveSpectrumPhy> m_spectrumPhy;
  Ptr<MmWaveSpectrumPhy> m_downlinkSpectrumPhy;
  Ptr<MmWaveSpectrumPhy> m_uplinkSpectrumPhy;

  double m_txPower;
  double m_noiseFigure;

  uint16_t m_cellId;

  Ptr<MmWavePhyMacCommon> m_phyMacConfig;

  std::map<uint64_t, Ptr<PacketBurst> > m_packetBurstMap;
  std::vector<std::list<Ptr<MmWaveControlMessage> > > m_controlMessageQueue;

  TddVarTtiTypeList m_currTddMap;
  //	std::list<SlotAllocInfo> m_slotAllocInfoList;
  SlotAllocInfo m_currSlotAllocInfo;

  //std::vector<SlotAllocInfo> m_slotAllocInfo;	// maps subframe num to allocation info


  uint16_t m_frameNum;
  uint8_t m_subframeNum;
  uint8_t m_slotNum;
  uint8_t m_varTtiNum;

  std::map<uint32_t, TddVarTtiTypeList> m_tddPatternForVarTtiMap;

  std::map<uint32_t, SlotAllocInfo> m_varTtiAllocInfoMap;

  MmWavePhySapProvider* m_phySapProvider;

  uint32_t m_raPreambleId;

  bool m_slotAllocInfoUpdated;

private:
  std::map <SfnSf,SlotAllocInfo> m_slotAllocInfo;   // maps subframe num to allocation info

  /// component carrier Id used to address sap
  uint8_t m_componentCarrierId;

};

}


#endif /* SRC_MMWAVE_MODEL_MMWAVE_PHY_H_ */
