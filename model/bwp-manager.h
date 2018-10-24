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
#include <map>

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
  virtual ~BwpManager ();
  static TypeId GetTypeId ();

protected:
  // Inherited methods
  virtual void DoInitialize (void);

  /*
   * \brief This function contains most of the BwpManager logic.
   */
  virtual void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params);

  /*
   * \brief Intercepts function calls from MAC of component carriers when it notifies RLC
   * of transmission opportunities. This function decides id the transmission opportunity
   * will be forwarded to the RLC.
   */
  virtual void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);

  /**
   * \brief Forwards uplink BSR to CCM, called by MAC through CCM SAP interface.
   * \param bsr the BSR
   * \param componentCarrierId the component carrier ID
   */
  virtual void DoUlReceiveMacCe (MacCeListElement_s bsr, uint8_t componentCarrierId);

  /*
   * \brief Overload DoSetupBadaRadioBearer to connect directly to Rlc retransmission buffer size.
   */
  virtual std::vector<LteCcmRrcSapProvider::LcsConfig> DoSetupDataRadioBearer (EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, LteMacSapUser* msu);

private:
  /*
   * \brief Checks if the flow is is GBR.
   */
  bool IsGbr (LteMacSapProvider::ReportBufferStatusParameters params);

  uint8_t m_gbr_conv_voice_bwp;
  uint8_t m_gbr_conv_video_bwp;
  uint8_t m_gbr_gaming_bwp;
  uint8_t m_gbr_non_conv_video_bwp;
  uint8_t m_ngbr_ims_bwp;
  uint8_t m_ngbr_video_tcp_operator_bwp;
  uint8_t m_ngbr_voice_video_gaming_bwp;
  uint8_t m_ngbr_video_tcp_premium_bwp;
  uint8_t m_ngbr_video_tcp_default_bwp;
  uint8_t m_gbr_ultra_low_lat_bwp;

};
} // end of namespace ns3

#endif /* BWP_MANAGER_H */
