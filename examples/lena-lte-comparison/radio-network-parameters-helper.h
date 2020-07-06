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
#ifndef RADIO_NETWORK_PARAMETERS_HELPER_H
#define RADIO_NETWORK_PARAMETERS_HELPER_H

#include <string>
namespace ns3 {
class RadioNetworkParametersHelper
{
public:

  void SetScenario (const std::string & scenario);

  /**
   * \brief Set the radio network parameters to LTE.
   * \param operation mode that can be FDD or TDD in order to determine bandwidth
   * \param numCcs number of component carriers
   * \param bandwidthHz bandwidth of a single carrier in MHz
   */
  void SetNetworkToLte (const std::string operationMode,
                        uint16_t numCcs, uint32_t bandwidthMHz);

  /**
   * \brief Set the radio network parameters to NR.
   * \param operation mode that can be FDD or TDD in order to determine bandwidth
   * \param numerology Numerology to use.
   * \param numCcs number of component carriers
   * \param bandwidthMHz bandwidth in MHz
   */
  void SetNetworkToNr (const std::string operationMode,
                       uint16_t numerology,
                       uint16_t numCcs,
                       uint32_t bandwidthMHz);

  /**
   * \brief Gets the BS transmit power
   * \return Transmit power in dBW
   */
  double GetTxPower ();

  /**
   * \brief Gets the operation bandwidth
   * \return Bandwidth in Hz
   */
  double GetBandwidth ();

  /**
   * \brief Gets the central frequency
   * \return Central frequency in Hz
   */
  double GetCentralFrequency ();

  /**
   * \brief Gets the band numerology
   * \return Numerology
   */
  uint16_t GetNumerology ();

private:
  double m_txPower {-1.0};            //!< Transmit power in dBm
  double m_bandwidth {0.0};           //!< System bandwidth in Hz
  double m_centralFrequency {-1.0};   //!< Band central frequency in Hz
  uint16_t m_numerology {0};          //!< Operation band numerology
};

} // namespace ns3

#endif // RADIO_NETWORK_PARAMETERS_HELPER_H
