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


#include "nr-mac.h"
#include "nr-phy-mac-common.h"

#include <ns3/lte-ue-cmac-sap.h>
#include <ns3/lte-ccm-mac-sap.h>
#include <ns3/traced-callback.h>

#include <ns3/nr-sl-mac-sap.h>
#include <ns3/nr-sl-ue-phy-sap.h>
#include <ns3/nr-sl-ue-cmac-sap.h>
#include <map>
#include <unordered_map>

namespace ns3 {

class NrUePhySapUser;
class NrPhySapProvider;
class NrControlMessage;
class UniformRandomVariable;
class PacketBurst;
class NrUlDciMessage;
class NrAmc;

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
  //NR Sidelink
  /// let the forwarder class access the protected and private members
  friend class MemberNrSlMacSapProvider<NrUeMac>;
  friend class MemberNrSlUeCmacSapProvider<NrUeMac>;
  friend class MemberNrSlUePhySapUser<NrUeMac>;

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
   */
  void SetNumHarqProcess (uint8_t numHarqProcesses);

  /**
   * \return number of HARQ processes
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
  void DoReceiveControlMessage  (Ptr<NrControlMessage> msg);
  //void DoNotifyHarqDeliveryFailure (uint8_t harqId);

  // forwarded from UE CMAC SAP
  void DoConfigureRach (LteUeCmacSapProvider::RachConfig rc);
  void DoStartContentionBasedRandomAccessProcedure ();
  void DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t rapId, uint8_t prachMask);
  void AddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu);
  void DoRemoveLc (uint8_t lcId);
  void DoReset ();
  /**
   * \brief Notify MAC about the successful RRC connection
   * establishment.
   */
  void DoNotifyConnectionSuccessful ();
  /**
   * \brief Set IMSI
   *
   * \param imsi the IMSI of the UE
   */
  void DoSetImsi (uint64_t imsi);

  void RandomlySelectAndSendRaPreamble ();
  void SendRaPreamble (bool contention);
  void SendReportBufferStatus (void);
  void RefreshHarqProcessesPacketBuffer (void);

  std::map<uint32_t, struct MacPduInfo>::iterator AddToMacPduMap (const std::shared_ptr<DciInfoElementTdma> & dci,
                                                                  unsigned activeLcs, const SfnSf &ulSfn);

private:
  void ProcessUlDci (const Ptr<NrUlDciMessage> &dciMsg);

private:

  LteUeCmacSapUser* m_cmacSapUser {nullptr};
  LteUeCmacSapProvider* m_cmacSapProvider {nullptr};
  NrPhySapProvider* m_phySapProvider {nullptr};
  NrUePhySapUser* m_phySapUser {nullptr};
  LteMacSapProvider* m_macSapProvider {nullptr};

  SfnSf m_currentSlot;

  uint8_t m_numHarqProcess {20}; //!< number of HARQ processes

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
  uint64_t m_imsi {0}; ///< IMSI

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

  //NR SL
public:
  /**
   * \brief Get the NR Sidelik MAC SAP offered by MAC to RLC
   *
   * \return the NR Sidelik MAC SAP provider interface offered by
   *          MAC to RLC
   */
  NrSlMacSapProvider* GetNrSlMacSapProvider ();

  /**
   * \brief Set the NR Sidelik MAC SAP offered by this RLC
   *
   * \param s the NR Sidelik MAC SAP user interface offered to the
   *          MAC by RLC
   */
   void SetNrSlMacSapUser (NrSlMacSapUser* s);

   /**
    * \brief Get the NR Sidelik UE Control MAC SAP offered by MAC to RRC
    *
    * \return the NR Sidelik UE Control MAC SAP provider interface offered by
    *         MAC to RRC
    */
   NrSlUeCmacSapProvider* GetNrSlUeCmacSapProvider ();

   /**
    * \brief Set the NR Sidelik UE Control MAC SAP offered by RRC to MAC
    *
    * \param s the NR Sidelik UE Control MAC SAP user interface offered to the
    *          MAC by RRC
    */
    void SetNrSlUeCmacSapUser (NrSlUeCmacSapUser* s);

    /**
     * \brief Get the NR Sidelik UE PHY SAP offered by UE MAC to UE PHY
     *
     * \return the NR Sidelik UE PHY SAP user interface offered by
     *         UE MAC to UE PHY
     */
    NrSlUePhySapUser* GetNrSlUePhySapUser ();

    /**
     * \brief Set the NR Sidelik UE PHY SAP offered by UE PHY to UE MAC
     *
     * \param s the NR Sidelik UE PHY SAP provider interface offered to the
     *          UE MAC by UE PHY
     */
     void SetNrSlUePhySapProvider (NrSlUePhySapProvider* s);

  /**
   * \brief Set the sidelink AMC model
   *
   * Usually it will be set by the NrSlHelper
   *
   * \param slAmc The AMC to be used for sidelink
   */
  void SetSlAmcModel (const Ptr<NrAmc> &slAmc);

protected:
  // forwarded from NR SL UE MAC SAP Provider
  /**
   * \brief send an NR SL RLC PDU to the MAC for transmission. This method is
   * to be called as a response to NrSlMacSapUser::NotifyNrSlTxOpportunity
   *
   * \param params NrSlRlcPduParameters
   */
  void DoTransmitNrSlRlcPdu (const NrSlMacSapProvider::NrSlRlcPduParameters &params);
  /**
   * \brief Report the RLC buffer status to the MAC
   *
   * \param params NrSlReportBufferStatusParameters
   */
  void DoReportNrSlBufferStatus (const NrSlMacSapProvider::NrSlReportBufferStatusParameters &params);

  // forwarded from UE CMAC SAP
  /**
   * \brief Adds a new Logical Channel (LC) used for Sidelink
   *
   * \param slLcInfo The sidelink LC info
   * \param msu The corresponding LteMacSapUser
   */
  void DoAddNrSlLc (const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo &slLcInfo, NrSlMacSapUser* msu);
  /**
   * \brief Remove an existing NR Sidelink Logical Channel for a UE in the LteUeComponentCarrierManager
   *
   * \param slLcId is the Sidelink Logical Channel Id
   * \param srcL2Id is the Source L2 ID
   * \param dstL2Id is the Destination L2 ID
   */
  void DoRemoveNrSlLc (uint8_t slLcId, uint32_t srcL2Id, uint32_t dstL2Id);
  /**
   * \brief Reset Nr Sidelink LC map
   *
   */
  void DoResetNrSlLcMap ();

private:
  /// Sidelink Communication related variables
  struct SidelinkLcIdentifier
  {
    uint8_t lcId; ///< Sidelink LCID
    uint32_t srcL2Id; ///< Source L2 ID
    uint32_t dstL2Id; ///< Destination L2 ID
  };

  /**
   * \brief Less than operator
   *
   * \param l first SidelinkLcIdentifier
   * \param r second SidelinkLcIdentifier
   * \returns true if first SidelinkLcIdentifier parameter values are less than the second SidelinkLcIdentifier parameters"
   */
  friend bool operator < (const SidelinkLcIdentifier &l, const SidelinkLcIdentifier &r)
  {
    return l.lcId < r.lcId || (l.lcId == r.lcId && l.srcL2Id < r.srcL2Id) || (l.lcId == r.lcId && l.srcL2Id == r.srcL2Id && l.dstL2Id < r.dstL2Id);
  }

  struct SlLcInfoUeMac
  {
    NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo lcInfo;
    NrSlMacSapUser* macSapUser;
  };
  Ptr<const NrAmc> m_slAmc {nullptr};  //!< AMC model used to compute SL Transport block size
  std::map <SidelinkLcIdentifier, SlLcInfoUeMac> m_nrSlLcInfoMap; ///< Sidelink logical channel info map
  NrSlMacSapProvider* m_nrSlMacSapProvider; //!< SAP interface to receive calls from the UE RLC instance
  NrSlMacSapUser* m_nrSlMacSapUser {nullptr}; //!< SAP interface to call the methods of UE RLC instance
  NrSlUeCmacSapProvider* m_nrSlUeCmacSapProvider; //!< Control SAP interface to receive calls from the UE RRC instance
  NrSlUeCmacSapUser* m_nrSlUeCmacSapUser {nullptr}; //!< Control SAP interface to call the methods of UE RRC instance
  NrSlUePhySapProvider* m_nrSlUePhySapProvider {nullptr}; //!< SAP interface to call the methods of UE PHY instance
  NrSlUePhySapUser* m_nrSlUePhySapUser; //!< SAP interface to receive calls from the UE PHY instance
};

}


#endif /* NR_UE_MAC_H */
