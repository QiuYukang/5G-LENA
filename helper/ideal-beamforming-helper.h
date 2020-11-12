/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include <ns3/object-factory.h>
#include <ns3/ideal-beamforming-algorithm.h>
#include <ns3/nstime.h>
#include <ns3/vector.h>
#include <ns3/net-device.h>
#include <ns3/net-device-container.h>
#include "ns3/event-id.h"

#ifndef SRC_NR_HELPER_IDEAL_BEAMFORMING_HELPER_H_
#define SRC_NR_HELPER_IDEAL_BEAMFORMING_HELPER_H_

namespace ns3 {

class NrGnbNetDevice;
class NrUeNetDevice;

/**
 * \ingroup helper
 * \brief The IdealBeamformingHelper class
 */
class IdealBeamformingHelper : public Object
{
public:
  /**
   * \brief IdealBeamformingHelper
   */
  IdealBeamformingHelper ();
  /**
   * \brief ~IdealBeamformingHelper
   */
  virtual ~IdealBeamformingHelper ();

  virtual void DoInitialize ();

  /**
   * \brief Get the Type ID
   * \return the TypeId of the instance
   */
  static TypeId GetTypeId (void);

  /**
   * \brief AddBeamformingTask
   * \param gNbDev
   * \param ueDev
   */
  virtual void AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                   const Ptr<NrUeNetDevice>& ueDev);

  /**
   * \brief SetBeamformingMethod
   * \param beamformingMethod
   */
  virtual void SetBeamformingMethod (const TypeId &beamformingMethod);

  /**
   * \brief SetIdealBeamformingPeriodicity
   * \param v
   */
  void SetPeriodicity (const Time &v);
  /**
   * \brief GetIdealBeamformingPeriodicity
   * \return
   */
  Time GetPeriodicity () const;

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetBeamformingAlgorithmAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Run
   */
  virtual void Run () const;

protected:

  virtual void RunTask (const Ptr<NrGnbNetDevice>& gNbDev, const Ptr<NrUeNetDevice>& ueDev, uint8_t ccId) const;

  /**
   * \brief The beamforming timer has expired; at the next slot, perform beamforming.
   *
   */
  virtual void ExpireBeamformingTimer ();

  std::vector<std::pair<Ptr<NrGnbNetDevice>, Ptr<NrUeNetDevice> > > m_beamformingTasks;
  Time m_beamformingPeriodicity;
  Ptr<IdealBeamformingAlgorithm> m_beamformingAlgorithm;
  EventId m_beamformingTimer; //!< Beamforming timer
};

}; //ns3 namespace


#endif /* SRC_NR_HELPER_IDEAL_BEAMFORMING_HELPER_H_ */
