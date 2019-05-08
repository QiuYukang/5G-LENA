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

#ifndef MMWAVE_UE_MAC_H
#define MMWAVE_UE_MAC_H


#include "mmwave-mac.h"
#include "mmwave-phy-mac-common.h"
#include "mmwave-phy-sap.h"
#include <ns3/lte-ue-cmac-sap.h>
#include <ns3/lte-ccm-mac-sap.h>

namespace ns3 {

class MmWaveUeMac : public Object
{
  friend class UeMemberMmWaveUeCmacSapProvider;
  friend class UeMemberMmWaveMacSapProvider;
  friend class MacUeMemberPhySapUser;

public:
  /**
   * \brief Get the Type id
   * @return the type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief MmWaveUeMac constructor
   */
  MmWaveUeMac (void);
  /**
    * \brief Deconstructor
    */
  ~MmWaveUeMac (void) override;

  /**
   * \brief Set the C MAC SAP user (AKA the RRC representation for the MAC)
   * \param s the SAP pointer
   */
  void  SetUeCmacSapUser (LteUeCmacSapUser* s);

  /**
   * \brief Get the C MAC SAP provider (AKA the MAC representation for the RRC)
   * \return  C MAC SAP provider (AKA the MAC representation for the RRC)
   */
  LteUeCmacSapProvider*  GetUeCmacSapProvider (void);

  /**
   * \brief Get the Mac SAP provider (AKA the MAC representation for the RLC)
   * \return the Mac SAP provider (AKA the MAC representation for the RLC)
   */
  LteMacSapProvider*  GetUeMacSapProvider (void);

  /**
   * \brief Get the PHY SAP User (AKA the MAC representation for the PHY)
   * \return the PHY SAP User (AKA the MAC representation for the PHY)
   */
  MmWaveUePhySapUser* GetPhySapUser ();

  /**
   * \brief Set PHY SAP provider (AKA the PHY representation for the MAC)
   * \param ptr the PHY SAP provider (AKA the PHY representation for the MAC)
   */
  void SetPhySapProvider (MmWavePhySapProvider* ptr);

  /**
   * \brief Set the configuration parameter for the mac
   *
   * Done only once in the helper (no handover)
   *
   * \param ptrConfig Config
   */
  void SetConfigurationParameters (const Ptr<MmWavePhyMacCommon> &ptrConfig);

  /**
   *  TracedCallback signature for Ue Mac Received Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* RxedUeMacCtrlMsgsTracedCallback)
    (const SfnSf sfnSf, const uint16_t rnti, Ptr<MmWaveControlMessage>);

  /**
   *  TracedCallback signature for Ue Mac Transmitted Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* TxedUeMacCtrlMsgsTracedCallback)
    (const SfnSf sfnSf, const uint16_t rnti, Ptr<MmWaveControlMessage>);



protected:
  // From object
  virtual void DoDispose (void) override;

private:
  void RecvRaResponse (BuildRarListElement_s raResponse);
  void SetRnti (uint16_t);
  void DoSlotIndication (SfnSf sfn);

  /**
   * \brief Get the total size of the RLC buffers.
   * \return The number of bytes that are in the RLC buffers
   */
  uint32_t GetTotalBufSize () const __attribute__((warn_unused_result));

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

private:
  Ptr<MmWavePhyMacCommon> m_phyMacConfig;

  LteUeCmacSapUser* m_cmacSapUser;
  LteUeCmacSapProvider* m_cmacSapProvider;
  MmWavePhySapProvider* m_phySapProvider;
  MmWaveUePhySapUser* m_phySapUser;
  LteMacSapProvider* m_macSapProvider;

  uint16_t m_frameNum {0};
  uint8_t m_subframeNum {0};
  uint16_t m_slotNum {0};
  uint8_t m_varTtiNum {0};

  std::map<uint32_t, struct MacPduInfo> m_macPduMap;

  std::map <uint8_t, LteMacSapProvider::ReportBufferStatusParameters> m_ulBsrReceived;   // BSR received from RLC (the last one)

  /**
   * \brief States for the SR/BSR mechanism.
   *
   * The SR/BSR mechanism is based on a variable in which
   * it is saved the state (INACTIVE/ACTIVE).
   *
   * The machine is starting from the INACTIVE state. When the RLC notifies
   * to MAC that there are new bytes in its queue (DoReportBufferStatus()),
   * if the machine is in INACTIVE state, it enters the ACTIVE state.
   * Entering the ACTIVE state means to send a SR, which is enqueued in the PHY layer.
   * It will suffer slots of CTRL latency. If the state is already ACTIVE, then
   * the BSR can be sent in the same slot as data. It means that the MAC prepares
   * together the data and the BSR.
   *
   * If the BSR is not sent (we don't have any data in the queue) and we don't
   * have any more reserved space to send BSR, then the state goes back to the
   * INACTIVE state.
   */
  enum SrBsrMachine : uint8_t
  {
    INACTIVE,    //!< no SR nor BSR.. initial state
    TO_SEND,     //!< We have to send the BSR when possible
    ACTIVE       //!< SR or BSR sent; now the source of information is the vector m_bsrReservedSpace
  };
  SrBsrMachine m_srState {INACTIVE};       //!< Current state for the SR/BSR machine.

  Ptr<UniformRandomVariable> m_raPreambleUniformVariable;
  uint8_t m_raPreambleId {0};
  uint8_t m_raRnti {0};

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
  uint16_t m_rnti {0};

  bool m_waitingForRaResponse {true};
  static uint8_t g_raPreambleId;

  /**
   * Trace information regarding Ue MAC Received Control Messages
   * Frame number, Subframe number, slot, VarTtti, rnti,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, Ptr<MmWaveControlMessage>> m_macRxedCtrlMsgsTrace;

  /**
   * Trace information regarding Ue MAC Transmitted Control Messages
   * Frame number, Subframe number, slot, VarTtti, rnti,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, Ptr<MmWaveControlMessage>> m_macTxedCtrlMsgsTrace;
};

}


#endif /* MMWAVE_UE_MAC_H */
