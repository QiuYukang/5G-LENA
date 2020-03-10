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

#ifndef SRC_MMWAVE_MODEL_MMWAVE_PHY_SAP_H_
#define SRC_MMWAVE_MODEL_MMWAVE_PHY_SAP_H_

#include <ns3/packet-burst.h>
#include <ns3/mmwave-phy-mac-common.h>
#include <ns3/mmwave-mac-sched-sap.h>
#include <ns3/mmwave-control-messages.h>
#include "beam-id.h"

namespace ns3 {

class MmWaveControlMessage;

/* Mac to Phy comm*/
class MmWavePhySapProvider
{
public:
  virtual ~MmWavePhySapProvider ();

  virtual void SendMacPdu (Ptr<Packet> p ) = 0;

  virtual void SendControlMessage (Ptr<MmWaveControlMessage> msg) = 0;

  virtual void SendRachPreamble (uint8_t PreambleId, uint8_t Rnti) = 0;

  virtual void SetSlotAllocInfo (SlotAllocInfo slotAllocInfo) = 0;

  /**
   * \brief Notify PHY about the successful RRC connection
   * establishment.
   */
  virtual void NotifyConnectionSuccessful () = 0;

  /**
   * \brief Get the beam ID from the RNTI specified. Not in any standard.
   * \param rnti RNTI of the user
   * \return Beam ID of the user
   */
  virtual BeamId GetBeamId (uint8_t rnti) const = 0;

  /**
   * \brief Retrieve the spectrum model used by the PHY layer.
   * \return the SpectrumModel
   *
   * It is used to calculate the CQI. In the future, this method may be removed
   * if the CQI calculation is done in the PHY layer, just reporting to MAC
   * its value.
   */
  virtual Ptr<const SpectrumModel> GetSpectrumModel () const = 0;

  /**
   * \return the Bwp id of the PHY
   */
  virtual uint16_t GetBwpId () const = 0;

  /**
   * \return the cell id of the PHY
   */
  virtual uint16_t GetCellId () const = 0;

};

/* This SAP is normally used so that PHY can send to MAC indications
 * and providing to MAC some information. The relationship between MAC and PHY
 * is that PHY is service provider to MAC, and MAC is user.
 * Exceptionally, PHY can also request some information from MAC through this
 * interface, such as e.g. GetNumRbPerRbg.*/
class MmWaveEnbPhySapUser
{
public:
  virtual ~MmWaveEnbPhySapUser ()
  {
  }

  /**
   * Called by the Phy to notify the MAC of the reception of a new PHY-PDU
   *
   * \param p
   */
  virtual void ReceivePhyPdu (Ptr<Packet> p) = 0;

  /**
   * \brief Receive SendLteControlMessage (PDCCH map, CQI feedbacks) using the ideal control channel
   * \param msg the Ideal Control Message to receive
   */
  virtual void ReceiveControlMessage (Ptr<MmWaveControlMessage> msg) = 0;

  /**
   * \brief Set the current Sfn. The state machine has advanced by one slot
   * \param sfn The current sfn
   */
  virtual void SetCurrentSfn(const SfnSf &sfn) = 0;

  /**
   * \brief Trigger MAC layer to generate a DL slot for the SfnSf indicated
   * \param sfn Slot to fill with DL scheduling decisions
   * \param slotType Slot type requested (DL, S, F)
   */
  virtual void SlotDlIndication (const SfnSf &sfn, LteNrTddSlotType slotType) = 0;

  /**
   * \brief Trigger MAC layer to generate an UL slot for the SfnSf indicated
   * \param sfn Slot to fill with UL scheduling decisions
   * \param slotType Slot type requested (UL, S, F)
   */
  virtual void SlotUlIndication (const SfnSf &sfn, LteNrTddSlotType slotType) = 0;

  // We do a DL and then manually add an UL CTRL if it's an S slot.
  // virtual void SlotSIndication (const SfnSf &sfn) = 0;
  // We do UL and then DL to model an F slot.
  // virtual void SlotFIndication (const SfnSf &sfn) = 0;

  /**
   * \brief Returns to MAC level the UL-CQI evaluated
   * \param ulcqi the UL-CQI (see FF MAC API 4.3.29)
   */
  virtual void UlCqiReport (MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi) = 0;

  /**
   * notify the reception of a RACH preamble on the PRACH
   *
   * \param prachId the ID of the preamble
   */
  virtual void ReceiveRachPreamble (uint32_t raId) = 0;

  /**
   * Notify the HARQ on the UL tranmission status
   *
   * \param params
   */
  virtual void UlHarqFeedback (UlHarqInfo params) = 0;

  /**
   * \brief Called by the PHY to notify MAC that beam has changed. Not in any standard
   * \param beamId the new beam ID
   * \param rnti the RNTI of the user
   */
  virtual void BeamChangeReport (BeamId beamId, uint8_t rnti) = 0;

  /**
   * \brief PHY requests information from MAC.
   * While MAC normally act as user of PHY services, in this case
   * exceptionally MAC provides information/service to PHY.
   * \return number of resource block per resource block group
   */
  virtual uint32_t GetNumRbPerRbg () const = 0;
};

class MmWaveUePhySapUser
{
public:
  virtual ~MmWaveUePhySapUser ()
  {
  }

  /**
   * Called by the Phy to notify the MAC of the reception of a new PHY-PDU
   *
   * \param p
   */
  virtual void ReceivePhyPdu (Ptr<Packet> p) = 0;

  /**
   * \brief Receive SendLteControlMessage (PDCCH map, CQI feedbacks) using the ideal control channel
   * \param msg the Ideal Control Message to receive
   */
  virtual void ReceiveControlMessage (Ptr<MmWaveControlMessage> msg) = 0;

  /**
   * \brief Trigger the start from a new slot (input from Phy layer)
   * \param frameNo frame number
   * \param subframeNo subframe number
   */
  virtual void SlotIndication (SfnSf) = 0;

  //virtual void NotifyHarqDeliveryFailure (uint8_t harqId) = 0;
};

}

#endif /* SRC_MMWAVE_MODEL_MMWAVE_PHY_SAP_H_ */
