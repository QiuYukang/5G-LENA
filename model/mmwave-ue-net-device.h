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
*                Biljana Bojovic <bbojovic@cttc.es> added carrier aggregation
*/

#ifndef SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_
#define SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_


#include "mmwave-net-device.h"
#include "mmwave-enb-net-device.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "mmwave-phy.h"
#include "mmwave-ue-mac.h"
#include <ns3/lte-ue-rrc.h>
#include <ns3/epc-ue-nas.h>
#include "component-carrier-mmwave-ue.h"
#include <ns3/lte-ue-component-carrier-manager.h>

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
//class MmWavePhy;
class MmWaveUePhy;
class MmWaveUeMac;
class MmWaveEnbNetDevice;

class MmWaveUeNetDevice : public MmWaveNetDevice
{

public:
  static TypeId GetTypeId (void);

  MmWaveUeNetDevice (void);

  virtual ~MmWaveUeNetDevice (void);

  virtual void DoDispose ();

  uint32_t GetCsgId () const;

  void SetCsgId (uint32_t csgId);

  void UpdateConfig (void);

  virtual bool DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

  Ptr<MmWaveUePhy> GetPhy (uint8_t index) const;

  Ptr<LteUeComponentCarrierManager> GetComponentCarrierManager (void) const;

  uint64_t GetImsi () const;

  uint16_t GetEarfcn () const;

  Ptr<EpcUeNas> GetNas (void) const;

  Ptr<LteUeRrc> GetRrc () const;

  void SetEarfcn (uint16_t earfcn);

  void SetTargetEnb (Ptr<MmWaveEnbNetDevice> enb);

  Ptr<MmWaveEnbNetDevice> GetTargetEnb (void);

  /**
   * \brief Set the number of antenna elements in the first dimension
   * \param antennaNum the number of antenna elements in the first dimension
   */
  void SetAntennaNumDim1 (uint8_t antennaNum);
  /**
   * \brief Set the number of antenna elements in the second dimension.
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
  /**
   * \brief Set the ComponentCarrier Map for the UE
   * \param ccm the map of ComponentCarrierUe
   */
  void SetCcMap (std::map< uint8_t, Ptr<ComponentCarrierMmWaveUe> > ccm);

  /**
   * \brief Get the ComponentCarrier Map for the UE
   * \returns the map of ComponentCarrierUe
   */
  std::map< uint8_t, Ptr<ComponentCarrierMmWaveUe> >  GetCcMap (void);

  /**
   * \brief Get the size of the component carriers map
   * \return the number of cc that we have
   */
  uint32_t GetCcMapSize () const;

protected:
  // inherited from Object
  virtual void DoInitialize (void);

private:
  Ptr<MmWaveEnbNetDevice> m_targetEnb;
  Ptr<LteUeRrc> m_rrc;
  Ptr<EpcUeNas> m_nas;
  uint64_t m_imsi;
  uint16_t m_earfcn;
  uint32_t m_csgId;
  bool m_isConstructed;
  uint8_t m_antennaNumDim1;  //!< The number of antenna elements in the first dimension.
  uint8_t m_antennaNumDim2;  //!< The number of antenna elements in the second dimension.
  std::map < uint8_t, Ptr<ComponentCarrierMmWaveUe> > m_ccMap; ///< component carrier map
  Ptr<LteUeComponentCarrierManager> m_componentCarrierManager; ///< the component carrier manager
};

}
#endif /* SRC_MMWAVE_MODEL_MMWAVE_UE_NET_DEVICE_H_ */
