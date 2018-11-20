/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *                Biljana Bojovic <bbojovic@cttc.es>
 */

#ifndef SRC_MMWAVE_MODEL_MMWAVE_ENB_NET_DEVICE_H_
#define SRC_MMWAVE_MODEL_MMWAVE_ENB_NET_DEVICE_H_

#include "mmwave-net-device.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "mmwave-phy.h"
#include "mmwave-enb-phy.h"
#include "mmwave-enb-mac.h"
#include "mmwave-mac-scheduler.h"
#include <vector>
#include <ns3/lte-enb-rrc.h>
#include "ns3/lte-enb-component-carrier-manager.h"
#include "ns3/component-carrier-gnb.h"

namespace ns3 {
/* Add forward declarations here */
class Packet;
class PacketBurst;
class Node;
//class MmWavePhy;
class MmWaveEnbPhy;
class MmWaveEnbMac;


class MmWaveEnbNetDevice : public MmWaveNetDevice
{
public:
  static TypeId
  GetTypeId (void);

  MmWaveEnbNetDevice ();

  virtual ~MmWaveEnbNetDevice (void);

  virtual void DoDispose (void);

  virtual bool DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

  Ptr<MmWaveEnbMac> GetMac (uint8_t index);

  Ptr<MmWaveEnbPhy> GetPhy (uint8_t index);

  uint16_t GetCellId () const;

  uint8_t GetBandwidth () const;

  void SetBandwidth (uint8_t bw);

  void SetEarfcn (uint16_t earfcn);

  uint16_t GetEarfcn () const;

  void SetRrc (Ptr<LteEnbRrc> rrc);

  Ptr<LteEnbRrc> GetRrc (void);

  /**
  * \brief Set the number of antenna elements in the first dimension
  * \param antennaNum the number of antenna elements in the first dimension
  */
  void SetAntennaNumDim1 (uint8_t antennaNum);
  /**
   * \brief Set the number of antenna elements in the second dimension
   * \param antennaNum the number of antenna elements in the second dimension
   */
  void SetAntennaNumDim2 (uint8_t antennaNum);
  /**
  * \brief Returns the total number of antenna elements.
  */
  uint8_t GetAntennaNum () const;
 /**
  * \brief Returns the number of antenna elements in the first dimension.
  */
  uint8_t GetAntennaNumDim1 () const;
 /**
  * \brief Returns the number of antenna elements in the second dimension.
  */
  uint8_t GetAntennaNumDim2 () const;

  void SetCcMap (std::map<uint8_t, Ptr<ComponentCarrierGnb> > ccm);

  /**
   * \brief Get the size of the component carriers map
   * \return the number of cc that we have
   */
  uint32_t GetCcMapSize () const;

protected:
  virtual void
  DoInitialize (void);
  void
  UpdateConfig ();

private:
  Ptr<MmWaveMacScheduler> m_scheduler;

  Ptr<LteEnbRrc> m_rrc;

  uint16_t m_cellId; /* Cell Identifer. To uniquely identify an E-nodeB  */

  uint8_t m_Bandwidth; /* bandwidth in RBs (?) */

  uint16_t m_Earfcn; /* carrier frequency */

  bool m_isConstructed;

  bool m_isConfigured;

  uint8_t m_antennaNumDim1; //!< The number of antenna elements in the first dimension.
  uint8_t m_antennaNumDim2; //!< The number of antenna elements in the first dimension.


  std::map<uint8_t, Ptr<ComponentCarrierGnb> > m_ccMap; /**< ComponentCarrier map */

  Ptr<LteEnbComponentCarrierManager> m_componentCarrierManager; ///< the component carrier manager of this eNb
};

}

#endif /* SRC_MMWAVE_MODEL_MMWAVE_ENB_NET_DEVICE_H_ */
