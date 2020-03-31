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
#ifndef NODE_DISTRIBUTION_SCENARIO_INTERFACE_H
#define NODE_DISTRIBUTION_SCENARIO_INTERFACE_H

#include <ns3/node-container.h>

namespace ns3 {

/**
 * \brief Represents a scenario with base stations and user terminals.
 *
 * Set the relevant settings, and then call CreateScenario. After that call,
 * the node containers can be retrieved through GetBaseStations and GetUserTerminals.
 */
class NodeDistributionScenarioInterface
{
public:
  /**
   * \brief ~NodeDistributionScenarioInterface
   */
  virtual ~NodeDistributionScenarioInterface ();
  /**
   * \brief Get the list of gnb/base station nodes
   * \return A NodeContainer with all the Gnb (or base stations)
   */
  const NodeContainer & GetBaseStations () const;
  /**
   * \brief Get the list of user nodes
   * \return A NodeContainer with all the users
   */
  const NodeContainer & GetUserTerminals () const;

  /**
   * \brief Create the scenario, with the configured parameter.
   *
   * After this call, GetGnbs and GetUes will return the containers
   * with the nodes created and positioned.
   */
  virtual void CreateScenario () = 0;

  /**
   * \brief SetGnbHeight
   * \param h height
   */
  void SetBsHeight (double h);

  /**
   * \brief SetUeHeight
   * \param h heights
   */
  void SetUtHeight (double h);

  /**
   * \brief SetBsNumber
   * \param n the number of bs
   *
   * Will invalidate already existing BS (recreating the container)
   */
  void SetBsNumber (uint32_t n);

  /**
   * \brief SetUtNumber
   * \param n the number of ut
   *
   * Will invalidate already existing UT (recreating the container)
   */
  void SetUtNumber (uint32_t n);

protected:
  NodeContainer m_bs;  //!< Base stations
  NodeContainer m_ut;  //!< User Terminals

  double m_utHeight {-1.0}; //!< Height of UE nodes
  double m_bsHeight {-1.0}; //!< Height of gNB nodes
};

} // namespace ns3

#endif // NODE_DISTRIBUTION_SCENARIO_INTERFACE_H
