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
#include "radio-network-parameters-helper.h"
#include <ns3/abort.h>

namespace ns3 {

void
RadioNetworkParametersHelper::SetScenario (const std::string &scenario)
{
  NS_ABORT_MSG_IF (scenario != "UMa" && scenario != "UMi",
                   "Unsupported scenario");

  if (scenario == "UMa")
    {
      m_txPower = 49;
    }
  else
    {
      m_txPower = 44;
    }
}

  void
RadioNetworkParametersHelper::SetNetworkToLte (const std::string operationMode,
                                               uint16_t numCcs)
{
  m_numerology = 0;
  m_centralFrequency = 2e9;
  m_bandwidth = 20e6 * numCcs;  // 100 RBs per CC (freqReuse)
  if (operationMode == "FDD")
    {
      m_bandwidth += m_bandwidth;
    }

}

void
RadioNetworkParametersHelper::SetNetworkToNr (const std::string operationMode,
                                              uint16_t numerology,
                                              uint16_t numCcs)
{
  m_numerology = numerology;
  m_centralFrequency = 2e9;
  m_bandwidth = 20e6 * numCcs;  // 100 RBs per CC (freqReuse)
  if (operationMode == "FDD")
    {
      m_bandwidth += m_bandwidth;
    }
}

double
RadioNetworkParametersHelper::GetTxPower ()
{
  return m_txPower;
}

double
RadioNetworkParametersHelper::GetBandwidth ()
{
  return m_bandwidth;
}

double
RadioNetworkParametersHelper::GetCentralFrequency ()
{
  return m_centralFrequency;
}

uint16_t
RadioNetworkParametersHelper::GetNumerology ()
{
  return m_numerology;
}

} // namespace ns3
