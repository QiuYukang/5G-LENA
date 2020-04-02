/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Biljana Bojovic <bbojovic@cttc.es>
 */


#ifndef COMPONENT_CARRIER_GNB_H
#define COMPONENT_CARRIER_GNB_H

#include <ns3/component-carrier.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include "ns3/mmwave-phy.h"
#include <ns3/mmwave-enb-phy.h>
#include <ns3/pointer.h>


namespace ns3 {

class MmWaveEnbMac;
class MmWaveMacScheduler;

/**
 * \ingroup mmwave
 *
 * Defines a single carrier for gnb.
 *
 */
class BandwidthPartGnb : public ComponentCarrierBaseStation
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  BandwidthPartGnb ();

  virtual ~BandwidthPartGnb (void) override;

  /**
   * \return a pointer to the physical layer.
   */
  Ptr<MmWaveEnbPhy> GetPhy (void);

  /**
   * \return a pointer to the MAC layer.
   */
  Ptr<MmWaveEnbMac> GetMac (void);

  /**
   * \return a pointer to the Mac Scheduler.
   */
  Ptr<MmWaveMacScheduler> GetScheduler ();

  /**
   * Set the LteEnbPhy
   * \param s a pointer to the LteEnbPhy
   */
  void SetPhy (Ptr<MmWaveEnbPhy> s);
  /**
   * Set the LteEnbMac
   * \param s a pointer to the LteEnbMac
   */
  void SetMac (Ptr<MmWaveEnbMac> s);

  /**
   * Set the FfMacScheduler Algorithm
   * \param s a pointer to the FfMacScheduler
   */
  void SetMmWaveMacScheduler (Ptr<MmWaveMacScheduler> s);

  virtual void SetDlBandwidth (uint16_t bw) override { m_dlBandwidth = bw; }
  virtual void SetUlBandwidth (uint16_t bw) override { m_ulBandwidth = bw; }

  /**
   * \brief Set this bandwidth part as primary.
   * \param primaryCarrier true or false.
   *
   * Unfortunately, for the "false" value, the method will do nothing. Every carrier
   * starts as "not primary", so please, if you have to use SetAsPrimary (false)
   * think two times.
   */
  void SetAsPrimary (bool primaryCarrier);

private:
  Ptr<MmWaveEnbPhy> m_phy; ///< the Phy instance of this eNodeB component carrier
  Ptr<MmWaveEnbMac> m_mac; ///< the MAC instance of this eNodeB component carrier
  Ptr<MmWaveMacScheduler> m_scheduler; ///< the scheduler instance of this eNodeB component carrier
};

} // namespace ns3



#endif /* COMPONENT_CARRIER_H */
