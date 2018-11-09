/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Biljana Bojovic <bbojovic@cttc.cat>
 *
 */

#ifndef BWP_MANAGER_H
#define BWP_MANAGER_H

#include <ns3/no-op-component-carrier-manager.h>
#include <ns3/lte-ccm-rrc-sap.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/lte-rlc.h>
#include <ns3/eps-bearer.h>
#include <unordered_map>

namespace ns3 {
class UeManager;
class LteCcmRrcSapProvider;

/**
 * \brief Bandwidth part manager that coordinates traffic over different bandwidth parts.
 */
class BwpManager : public RrComponentCarrierManager
{
public:
  BwpManager ();
  virtual ~BwpManager () override;
  static TypeId GetTypeId ();

  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetConvVoiceBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::GBR_CONV_VOICE] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetConvVideoBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::GBR_CONV_VIDEO] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetGamingBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::GBR_GAMING] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNonConvVideoBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::GBR_NON_CONV_VIDEO] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcPttBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::GBR_MC_PUSH_TO_TALK] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNmcPttBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::GBR_NMC_PUSH_TO_TALK] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcVideoBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::GBR_MC_VIDEO] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetGbrV2xBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::GBR_V2X] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetImsBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_IMS] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpOpBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_OPERATOR] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoGamingBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_VOICE_VIDEO_GAMING] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpPremiumBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_PREMIUM] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpDefaultBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_DEFAULT] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcDelaySignalBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_MC_DELAY_SIGNAL] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcDataBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_MC_DATA] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNgbrV2xBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_V2X] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetLowLatEmbbBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::NGBR_LOW_LAT_EMBB] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetDiscreteAutSmallBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::DGBR_DISCRETE_AUT_SMALL] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetDiscreteAutLargeBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::DGBR_DISCRETE_AUT_LARGE] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetItsBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::DGBR_ITS] = bwpIndex; }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetElectricityBwp (uint8_t bwpIndex)
  { m_qciToBwpMap[EpsBearer::DGBR_ELECTRICITY] = bwpIndex; }

protected:
  // Inherited methods
  virtual void DoInitialize (void) override;

  /*
   * \brief This function contains most of the BwpManager logic.
   */
  virtual void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params) override;

  /*
   * \brief Intercepts function calls from MAC of component carriers when it notifies RLC
   * of transmission opportunities. This function decides id the transmission opportunity
   * will be forwarded to the RLC.
   */
  virtual void DoNotifyTxOpportunity (LteMacSapUser::TxOpportunityParameters txOpParams) override;

  /**
   * \brief Forwards uplink BSR to CCM, called by MAC through CCM SAP interface.
   * \param bsr the BSR
   * \param componentCarrierId the component carrier ID
   */
  virtual void DoUlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId) override;

  /**
   * \brief Forward SR to the right MAC instance through CCM SAP interface
   * \param rnti RNTI of the UE that requested the SR
   * \param componentCarrierId the component carrier ID which received the SR
   */
  virtual void DoUlReceiveSr (uint16_t rnti, uint8_t componentCarrierId) override;

  /*
   * \brief Overload DoSetupBadaRadioBearer to connect directly to Rlc retransmission buffer size.
   */
  virtual std::vector<LteCcmRrcSapProvider::LcsConfig> DoSetupDataRadioBearer (EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, LteMacSapUser* msu) override;

private:
  /*
   * \brief Checks if the flow is is GBR.
   */
  bool IsGbr (LteMacSapProvider::ReportBufferStatusParameters params);

  /**
   * \brief Map between QCI and BWP
   */
  std::unordered_map <uint8_t, uint8_t> m_qciToBwpMap;
};

} // end of namespace ns3

#endif /* BWP_MANAGER_H */
