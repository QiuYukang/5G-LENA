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
 *
 */

#ifndef NR_UE_MAC_H
#define NR_UE_MAC_H

#include "nr-phy-mac-common.h"
#include "nr-mac-pdu-info.h"

#include <ns3/lte-ue-cmac-sap.h>
#include <ns3/lte-ccm-mac-sap.h>
#include <ns3/traced-callback.h>

#include <unordered_map>

namespace ns3 {

class NrUePhySapUser;
class NrPhySapProvider;
class NrControlMessage;
class UniformRandomVariable;
class PacketBurst;
class NrUlDciMessage;

/**
 * \ingroup ue-mac
 * \brief The MAC class for the UE
 *
 * \section ue_mac_general General information
 *
 * \todo fill ue-mac general information doxygen part
 *
 * \section ue_mac_configuration Configuration
 *
 * The user can configure the class using the method NrHelper::SetUeMacAttribute(),
 * or by directly calling `SetAttribute` on the MAC pointer. The list of
 * attributes is reported below, in the Attributes section.
 *
 * \section ue_mac_traces CTRL-trace Traces for CTRL messages
 *
 * The class has two attributes that signals to the eventual listener the
 * transmission or the reception of CTRL messages. One is UeMacRxedCtrlMsgsTrace,
 * and the other is UeMacTxedCtrlMsgsTrace. For what regards the Gnb, you will
 * find more information in the NrGnbPhy class documentation.
 */
class NrUeMac : public Object
{
  friend class UeMemberNrUeCmacSapProvider;
  friend class UeMemberNrMacSapProvider;
  friend class MacUeMemberPhySapUser;

public:
  /**
   * \brief Get the Type id
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrUeMac constructor
   */
  NrUeMac (void);
  /**
    * \brief Deconstructor
    */
  ~NrUeMac (void) override;


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
  NrUePhySapUser* GetPhySapUser ();

  /**
   * \brief Set PHY SAP provider (AKA the PHY representation for the MAC)
   * \param ptr the PHY SAP provider (AKA the PHY representation for the MAC)
   */
  void SetPhySapProvider (NrPhySapProvider* ptr);

  /**
   *  TracedCallback signature for Ue Mac Received Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] nodeId
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* RxedUeMacCtrlMsgsTracedCallback)
    (const SfnSf sfnSf, const uint16_t nodeId, const uint16_t rnti,
     const uint8_t bwpId, Ptr<NrControlMessage>);

  /**
   *  TracedCallback signature for Ue Mac Transmitted Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] nodeId
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* TxedUeMacCtrlMsgsTracedCallback)
    (const SfnSf sfnSf, const uint16_t nodeId, const uint16_t rnti,
     const uint8_t bwpId, Ptr<NrControlMessage>);

  /**
   * \brief Sets the number of HARQ processes
   * \param numHarqProcesses the maximum number of harq processes
   *
   * Called by the helper at the moment of UE attachment
   */
  void SetNumHarqProcess (uint8_t numHarqProcesses);

  /**
   * \return number of HARQ processes
   *
   * Please remember that this number is obtained by the GNB, the UE
   * cannot configure it.
   */
  uint8_t GetNumHarqProcess () const;

protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;
  /**
   * \brief Get the bwp id of this MAC
   * \return the bwp id
   */
  uint16_t GetBwpId () const;

  /**
   * \brief Get the cell id of this MAC
   * \return the cell id
   */
  uint16_t GetCellId () const;

private:
  /**
   * \brief Received a RA response
   * \param raResponse the response
   */
  void RecvRaResponse (BuildRarListElement_s raResponse);
  /**
   * \brief Set the RNTI
   */
  void SetRnti (uint16_t);
  /**
   * \brief Do some work, we begin a new slot
   * \param sfn the new slot
   */
  void DoSlotIndication (const SfnSf &sfn);

  /**
   * \brief Get the total size of the RLC buffers.
   * \return The number of bytes that are in the RLC buffers
   */
  uint32_t GetTotalBufSize () const __attribute__((warn_unused_result));

  /**
   * \brief Send to the PHY a SR
   */
  void SendSR () const;
  /**
   * \brief Called by RLC to transmit a RLC PDU
   * \param params the RLC params
   *
   * Please note that this call is triggered by communicating to the RLC
   * that there is a new transmission opportunity with NotifyTxOpportunity().
   */
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
  void DoReceiveControlMessage  (Ptr<NrControlMessage> msg);
  //void DoNotifyHarqDeliveryFailure (uint8_t harqId);

  // forwarded from UE CMAC SAP
  void DoConfigureRach (LteUeCmacSapProvider::RachConfig rc);
  void DoStartContentionBasedRandomAccessProcedure ();
  void DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t rapId, uint8_t prachMask);
  void AddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu);
  void DoRemoveLc (uint8_t lcId);
  void DoReset ();
  void DoNotifyConnectionSuccessful ();
  void DoSetImsi (uint64_t imsi);

  void RandomlySelectAndSendRaPreamble ();
  void SendRaPreamble (bool contention);
  void SendReportBufferStatus (const SfnSf &dataSfn, uint8_t symStart);
  void RefreshHarqProcessesPacketBuffer (void);

  /**
   * \brief Create a new place in the MAC PDU map
   * \param dci the DCI that triggered the new MAC PDU
   * \param ulSfn the slot at which the data will be sent
   * \return the iterator for the MAC PDU in the map
   */
  std::unordered_map<uint32_t, struct NrMacPduInfo>::iterator
  AddToMacPduMap (const std::shared_ptr<DciInfoElementTdma> & dci, const SfnSf &ulSfn);

  /**
   * \brief Process the received UL DCI
   * \param dciMsg the UL DCI received
   */
  void ProcessUlDci (const Ptr<NrUlDciMessage> &dciMsg);

private:

  LteUeCmacSapUser* m_cmacSapUser {nullptr};
  LteUeCmacSapProvider* m_cmacSapProvider {nullptr};
  NrPhySapProvider* m_phySapProvider {nullptr};
  NrUePhySapUser* m_phySapUser {nullptr};
  LteMacSapProvider* m_macSapProvider {nullptr};

  SfnSf m_currentSlot;  //!< The current slot
  uint8_t m_numHarqProcess {20}; //!< number of HARQ processes
  std::unordered_map<uint32_t, struct NrMacPduInfo> m_macPduMap; //!< HarqId/PDU map

  std::unordered_map <uint8_t, LteMacSapProvider::ReportBufferStatusParameters> m_ulBsrReceived; //!< BSR received from RLC (the last one)

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
  uint8_t m_raPreambleId {0}; //!< The RA Preamble ID
  uint8_t m_raRnti {0};       //!< The RA Rnti
  uint64_t m_imsi {0};        ///< IMSI

  // The HARQ part has to be reviewed
  struct UlHarqProcessInfo
  {
    Ptr<PacketBurst> m_pktBurst;
    // maintain list of LCs contained in this TB
    // used to signal HARQ failure to RLC handlers
    std::vector<uint8_t> m_lcidList;
  };

  //uint8_t m_harqProcessId;
  std::vector < UlHarqProcessInfo > m_miUlHarqProcessesPacket; //!< Packets under trasmission of the UL HARQ processes
  std::vector < uint8_t > m_miUlHarqProcessesPacketTimer;      //!< timer for packet life in the buffer

  struct LcInfo
  {
    LteUeCmacSapProvider::LogicalChannelConfig lcConfig;
    LteMacSapUser* macSapUser;
  };

  std::unordered_map <uint8_t, LcInfo> m_lcInfoMap;
  uint16_t m_rnti {0};

  bool m_waitingForRaResponse {true}; //!< Indicates if we are waiting for a RA response
  static uint8_t g_raPreambleId; //!< Preamble ID, fixed, the UEs will not have any collision

  /**
   * Trace information regarding Ue MAC Received Control Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId, rnti, bwpId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, Ptr<const NrControlMessage>> m_macRxedCtrlMsgsTrace;

  /**
   * Trace information regarding Ue MAC Transmitted Control Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId, rnti, bwpId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, Ptr<const NrControlMessage>> m_macTxedCtrlMsgsTrace;
};

}


#endif /* NR_UE_MAC_H */
