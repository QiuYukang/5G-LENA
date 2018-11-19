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
 *                Sourjya Dutta <sdutta@nyu.edu>
 *                Russell Ford <russell.ford@nyu.edu>
 *                Menglei Zhang <menglei@nyu.edu>
 */



#ifndef SRC_MMWAVE_MODEL_MMWAVE_UE_MAC_H_
#define SRC_MMWAVE_MODEL_MMWAVE_UE_MAC_H_

#include "mmwave-mac.h"
#include <ns3/lte-ue-cmac-sap.h>
#include <ns3/lte-mac-sap.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <queue>

namespace ns3 {
class MmWaveControlMessage;

class MmWaveUeMac : public Object
{
  friend class UeMemberMmWaveUeCmacSapProvider;
  friend class UeMemberMmWaveMacSapProvider;
  friend class MacUeMemberPhySapUser;

public:
  static TypeId GetTypeId (void);

  MmWaveUeMac (void);
  ~MmWaveUeMac (void);
  virtual void DoDispose (void);

  void  SetUeCmacSapUser (LteUeCmacSapUser* s);
  LteUeCmacSapProvider*  GetUeCmacSapProvider (void);
  LteMacSapProvider*  GetUeMacSapProvider (void);

  /**
  * \brief Set the component carried ID
  * \param index the component carrier ID
  */
  void SetComponentCarrierId (uint8_t index);

  void SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig);
  Ptr<MmWavePhyMacCommon> GetConfigurationParameters (void) const;

  void DoSlotIndication (SfnSf sfn);

  MmWaveUePhySapUser* GetPhySapUser ();
  void SetPhySapProvider (MmWavePhySapProvider* ptr);

  void RecvRaResponse (BuildRarListElement_s raResponse);

  virtual void SetRnti (uint16_t);

  /// component carrier Id --> used to address sap
  uint8_t m_componentCarrierId;

private:
  /**
   * \brief Get the total size of the RLC buffers.
   * \return The number of bytes that are in the RLC buffers
   */
  uint32_t GetTotalBufSize () const;

  void SendSR () const;
  void DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params);

  /**
   * \brief Called by CCM
   * \param params the BSR params
   *
   * The CCM is calling this function for all the MAC of the UE. This method
   * will send SR only for CC ID = 0 (BwpManagerGnb will take care of
   * routing the SR to the appropriate MAC).
   *
   * \see DoSlotIndication
   */
  void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params);

  // forwarded from PHY SAP
  void DoReceivePhyPdu (Ptr<Packet> p);
  void DoReceiveControlMessage  (Ptr<MmWaveControlMessage> msg);
  //void DoNotifyHarqDeliveryFailure (uint8_t harqId);

  // forwarded from UE CMAC SAP
  void DoConfigureRach (LteUeCmacSapProvider::RachConfig rc);
  void DoStartContentionBasedRandomAccessProcedure ();
  void DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t rapId, uint8_t prachMask);
  void AddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu);
  void DoRemoveLc (uint8_t lcId);
  void DoReset ();

  void RandomlySelectAndSendRaPreamble ();
  void SendRaPreamble (bool contention);
  void SendReportBufferStatus (void);
  void RefreshHarqProcessesPacketBuffer (void);

  std::map<uint32_t, struct MacPduInfo>::iterator AddToMacPduMap (const std::shared_ptr<DciInfoElementTdma> & dci, unsigned activeLcs);

  Ptr<MmWavePhyMacCommon> m_phyMacConfig;

  LteUeCmacSapUser* m_cmacSapUser;
  LteUeCmacSapProvider* m_cmacSapProvider;

  TddVarTtiTypeList m_DataTxTDDMap;
  SlotAllocInfo m_DataTxAllocationList;

  MmWavePhySapProvider* m_phySapProvider;
  MmWaveUePhySapUser* m_phySapUser;
  LteMacSapProvider* m_macSapProvider;

  uint16_t m_frameNum;
  uint8_t m_subframeNum;
  uint16_t m_slotNum;
  uint8_t m_varTtiNum;

  //uint8_t m_tbUid;
  std::map<uint32_t, struct MacPduInfo> m_macPduMap;

  std::map <uint8_t, LteMacSapProvider::ReportBufferStatusParameters> m_ulBsrReceived;   // BSR received from RLC (the last one)

  /**
   * \brief States for the SR/BSR mechanism.
   *
   * The SR/BSR mechanism is composed by two components: a variable in which
   * it is saved the state (INACTIVE/ACTIVE) and m_bsrReservedSpace, a queue
   * of Sfnsf (frame - subframe - slot - symbol start) in which is possible to
   * send a BSR message.
   *
   * The machine is starting from the INACTIVE state. When the RLC notifies
   * to MAC that there are new bytes in its queue (DoReportBufferStatus()),
   * if the machine is in INACTIVE state, it enters the ACTIVE state.
   * Entering the ACTIVE state means to send a SR, which will be sent in UL CTRL
   * symbol of the same slot the data is received. Being already in the ACTIVE
   * state means that the BSR will be sent only in the reserved Sfnsf,
   * that are maintained as queue in the variable m_bsrReservedSpace.
   *
   * When the UE receive a message of type UL_BSR_GRANT to himself, then
   * the current slot is saved in the m_bsrReservedSpace
   * queue. It means that the UE, in the current slot, will send a BSR
   * in the UL CTRL symbol.
   *
   * When the UE receive a DCI UL grant, it saves the Sfnsf in which it will
   * send the data (depending on the K2) as allowed in the m_bsrReservedSpace
   * queue. In this way, along with the data, a BSR will be sent.
   *
   * At the start of each VarTti (each different DCI) the PHY is calling
   * DoSlotIndication. In this function, if the Sfnsf at the front of the Sfnsf
   * queue is true, then the BSR is sent only if we have data in the queue.
   *
   * If the BSR is not sent (we don't have any data in the queue) and we don't
   * have any more reserved space to send BSR, then the state goes back to the
   * INACTIVE state.
   */
  enum SrBsrMachine : uint8_t
  {
    INACTIVE,    //!< no SR nor BSR.. initial state
    ACTIVE       //!< SR or BSR sent; now the source of information is the vector m_bsrReservedSpace
  };
  std::queue<SfnSf> m_bsrReservedSpace;    //!< The SfnSf(s) in which the UE has reserved space for BSR
  SrBsrMachine m_srState {INACTIVE};       //!< Current state for the SR/BSR machine.
  bool m_stdBsr {false};                   //!< 3GPP std BSR/SR or old method.

  Ptr<UniformRandomVariable> m_raPreambleUniformVariable;
  uint8_t m_raPreambleId;
  uint8_t m_raRnti;

  struct UlHarqProcessInfo
  {
    Ptr<PacketBurst> m_pktBurst;
    // maintain list of LCs contained in this TB
    // used to signal HARQ failure to RLC handlers
    std::vector<uint8_t> m_lcidList;
  };

  //uint8_t m_harqProcessId;
  std::vector < UlHarqProcessInfo > m_miUlHarqProcessesPacket;   // Packets under trasmission of the UL HARQ processes
  std::vector < uint8_t > m_miUlHarqProcessesPacketTimer;   // timer for packet life in the buffer

  struct LcInfo
  {
    LteUeCmacSapProvider::LogicalChannelConfig lcConfig;
    LteMacSapUser* macSapUser;
  };

  std::map <uint8_t, LcInfo> m_lcInfoMap;
  uint16_t m_rnti;

  bool m_waitingForRaResponse;
  static uint8_t g_raPreambleId;

};

}


#endif /* SRC_MMWAVE_MODEL_MMWAVE_UE_MAC_H_ */
