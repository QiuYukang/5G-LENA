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
 */

#ifndef MMWAVE_ENB_MAC_H
#define MMWAVE_ENB_MAC_H

#include "mmwave-mac.h"
#include "mmwave-phy-mac-common.h"
#include "mmwave-mac-sched-sap.h"
#include "mmwave-phy-sap.h"
#include "mmwave-mac-scheduler.h"

#include <ns3/lte-enb-cmac-sap.h>
#include <ns3/lte-mac-sap.h>
#include <ns3/lte-ccm-mac-sap.h>
#include <ns3/lte-mac-sap.h>
#include <ns3/lte-enb-cmac-sap.h>

namespace ns3 {

class MmWaveControlMessage;
class MmWaveRarMessage;

/**
 * \ingroup gnb
 * \brief The MAC class for the gnb
 *
 * \section CTRL-trace Traces for CTRL messages
 *
 * The class has two attributes that signals to the eventual listener the
 * transmission or the reception of CTRL messages. One is EnbMacRxedCtrlMsgsTrace,
 * and the other is EnbMacTxedCtrlMsgsTrace. For what regards the UE, you will
 * find more information in the MmWaveUePhy class documentation.
 */
class MmWaveEnbMac : public Object
{
  friend class MmWaveEnbMacMemberEnbCmacSapProvider;
  friend class MmWaveMacEnbMemberPhySapUser;
  friend class MmWaveMacMemberMacCschedSapUser;
  friend class MmWaveMacMemberMacSchedSapUser;
  friend class EnbMacMemberLteMacSapProvider<MmWaveEnbMac>;
  friend class MemberLteCcmMacSapProvider<MmWaveEnbMac>;

public:
  static TypeId GetTypeId (void);
  MmWaveEnbMac (void);
  virtual ~MmWaveEnbMac (void) override;

  void SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig);
  Ptr<MmWavePhyMacCommon> GetConfigurationParameters (void) const;

  void SetMcs (int mcs);

  void SetForwardUpCallback (Callback <void, Ptr<Packet> > cb);

  MmWaveEnbPhySapUser* GetPhySapUser ();
  void SetPhySapProvider (MmWavePhySapProvider* ptr);

  MmWaveMacSchedSapUser* GetMmWaveMacSchedSapUser ();
  void SetMmWaveMacSchedSapProvider (MmWaveMacSchedSapProvider* ptr);

  MmWaveMacCschedSapUser* GetMmWaveMacCschedSapUser ();
  void SetMmWaveMacCschedSapProvider (MmWaveMacCschedSapProvider* ptr);

  LteMacSapProvider* GetMacSapProvider (void);
  LteEnbCmacSapProvider* GetEnbCmacSapProvider (void);

  void SetEnbCmacSapUser (LteEnbCmacSapUser* s);



  /**
  * \brief Get the eNB-ComponentCarrierManager SAP User
  * \return a pointer to the SAP User of the ComponentCarrierManager
  */
  LteCcmMacSapProvider* GetLteCcmMacSapProvider ();

  /**
  * \brief Set the ComponentCarrierManager SAP user
  * \param s a pointer to the ComponentCarrierManager provider
  */
  void SetLteCcmMacSapUser (LteCcmMacSapUser* s);

  /**
   * \brief A Beam for a user has changed
   * \param beamId new beam ID
   * \param rnti RNTI of the user
   */
  void BeamChangeReport (AntennaArrayModel::BeamId beamId, uint8_t rnti);

  /**
   * TracedCallback signature for DL scheduling events.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * ...
   */
  typedef void (* DlSchedulingTracedCallback)(   uint32_t frameNum, uint32_t subframeNum,
                                                 uint32_t slotNum,
                                                 uint32_t tbSize, uint32_t mcs,
                                                 uint32_t rnti,
                                                 uint8_t componentCarrierId);

  /**
   * TracedCallback signature for SR scheduling events.
   *
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] ccId The component carrier ID of this MAC.
   */
  typedef void (* SrTracedCallback) (const uint8_t ccId, const uint16_t rnti);

  /**
   *  TracedCallback signature for Enb Mac Received Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] ccId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* RxedEnbMacCtrlMsgsTracedCallback)
      (const SfnSf sfn, const uint16_t rnti, const uint8_t ccId, Ptr<MmWaveControlMessage>);

  /**
   *  TracedCallback signature for Enb Mac Transmitted Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] ccId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* TxedEnbMacCtrlMsgsTracedCallback)
      (const SfnSf sfn, const uint16_t rnti, const uint8_t ccId, Ptr<MmWaveControlMessage>);

protected:
  virtual void DoInitialize () override;
  virtual void DoDispose (void) override;

private:
  void ReceiveRachPreamble (uint32_t raId);
  void DoReceiveRachPreamble (uint32_t raId);
  void ReceiveBsrMessage  (MacCeElement bsr);
  void DoReportMacCeToScheduler (MacCeListElement_s bsr);
  /**
   * \brief Called by CCM to inform us that we are the addressee of a SR.
   * \param rnti RNTI that requested to be scheduled
   */
  void DoReportSrToScheduler (uint16_t rnti);
  void DoReceivePhyPdu (Ptr<Packet> p);
  void DoReceiveControlMessage  (Ptr<MmWaveControlMessage> msg);
  void DoSchedConfigIndication (MmWaveMacSchedSapUser::SchedConfigIndParameters ind);
  // forwarded from LteMacSapProvider
  void DoTransmitPdu (LteMacSapProvider::TransmitPduParameters);
  void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters);
  void DoUlCqiReport (MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);
  void DoSlotIndication (SfnSf sfnSf);
  // forwarded from MmWaveMacCchedSapUser
  void DoCschedCellConfigCnf (MmWaveMacCschedSapUser::CschedCellConfigCnfParameters params);
  void DoCschedUeConfigCnf (MmWaveMacCschedSapUser::CschedUeConfigCnfParameters params);
  void DoCschedLcConfigCnf (MmWaveMacCschedSapUser::CschedLcConfigCnfParameters params);
  void DoCschedLcReleaseCnf (MmWaveMacCschedSapUser::CschedLcReleaseCnfParameters params);
  void DoCschedUeReleaseCnf (MmWaveMacCschedSapUser::CschedUeReleaseCnfParameters params);
  void DoCschedUeConfigUpdateInd (MmWaveMacCschedSapUser::CschedUeConfigUpdateIndParameters params);
  void DoCschedCellConfigUpdateInd (MmWaveMacCschedSapUser::CschedCellConfigUpdateIndParameters params);
  // forwarded from LteEnbCmacSapProvider
  void DoConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth);
  void DoAddUe (uint16_t rnti);
  void DoRemoveUe (uint16_t rnti);
  void DoAddLc (LteEnbCmacSapProvider::LcInfo lcinfo, LteMacSapUser* msu);
  void DoReconfigureLc (LteEnbCmacSapProvider::LcInfo lcinfo);
  void DoReleaseLc (uint16_t  rnti, uint8_t lcid);
  void UeUpdateConfigurationReq (LteEnbCmacSapProvider::UeConfig params);
  LteEnbCmacSapProvider::RachConfig DoGetRachConfig ();
  LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue DoAllocateNcRaPreamble (uint16_t rnti);

  void DoDlHarqFeedback (DlHarqInfo params);
  void DoUlHarqFeedback (UlHarqInfo params);

private:
  struct MmWaveDlHarqProcessInfo
  {
    Ptr<PacketBurst> m_pktBurst;
    // maintain list of LCs contained in this TB
    // used to signal HARQ failure to RLC handlers
    std::vector<uint8_t> m_lcidList;
  };

  typedef std::vector < MmWaveDlHarqProcessInfo> MmWaveDlHarqProcessesBuffer_t;
  Ptr<MmWavePhyMacCommon> m_phyMacConfig;

  LteMacSapProvider* m_macSapProvider;
  LteEnbCmacSapProvider* m_cmacSapProvider;
  LteEnbCmacSapUser* m_cmacSapUser;
  MmWavePhySapProvider* m_phySapProvider;
  MmWaveEnbPhySapUser* m_phySapUser;

  MmWaveMacSchedSapProvider* m_macSchedSapProvider;
  MmWaveMacSchedSapUser* m_macSchedSapUser;
  MmWaveMacCschedSapProvider* m_macCschedSapProvider;
  MmWaveMacCschedSapUser* m_macCschedSapUser;

  // Sap For ComponentCarrierManager 'Uplink case'
  LteCcmMacSapProvider* m_ccmMacSapProvider;   ///< CCM MAC SAP provider
  LteCcmMacSapUser* m_ccmMacSapUser;   ///< CCM MAC SAP user


  uint16_t m_frameNum {0};
  uint8_t m_subframeNum {0};
  uint16_t m_slotNum {0};
  uint32_t m_varTtiNum {0};

  std::map<uint32_t, struct MacPduInfo> m_macPduMap;

  Callback <void, Ptr<Packet> > m_forwardUpCallback;

  std::vector <DlCqiInfo> m_dlCqiReceived;
  std::vector <MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters> m_ulCqiReceived;
  std::vector <MacCeElement> m_ulCeReceived;   // CE received (BSR up to now)


  std::map<uint8_t, uint32_t> m_receivedRachPreambleCount;

  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> > m_rlcAttached;

  std::vector <DlHarqInfo> m_dlHarqInfoReceived;   // DL HARQ feedback received
  std::vector <UlHarqInfo> m_ulHarqInfoReceived;   // UL HARQ feedback received
  std::map <uint16_t, MmWaveDlHarqProcessesBuffer_t> m_miDlHarqProcessesPackets;   // Packet under trasmission of the DL HARQ process

  /* That's horribly broken: in the class the DlScheduling attribute refers to
   * the LteEnbMac signature */
  TracedCallback<uint32_t, uint32_t,uint32_t, uint32_t, uint32_t, uint32_t, uint8_t> m_dlScheduling;

  std::list<uint16_t> m_srRntiList; //!< List of RNTI that requested a SR

  std::map<uint8_t, uint32_t> m_rapIdRntiMap; //!< RAPID RNTI map

  TracedCallback<uint8_t, uint16_t> m_srCallback; //!< Callback invoked when a UE requested a SR

  /**
   * Trace information regarding ENB MAC Received Control Messages
   * Frame number, Subframe number, slot, VarTtti, rnti, ccId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint8_t, Ptr<MmWaveControlMessage>> m_macRxedCtrlMsgsTrace;

  /**
   * Trace information regarding ENB MAC Transmitted Control Messages
   * Frame number, Subframe number, slot, VarTtti, rnti, ccId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint8_t, Ptr<MmWaveControlMessage>> m_macTxedCtrlMsgsTrace;
};

}

#endif /* MMWAVE_ENB_MAC_H */
