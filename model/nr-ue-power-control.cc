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

#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/math.h>
#include "nr-ue-power-control.h"
#include "nr-ue-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrUePowerControl");

NS_OBJECT_ENSURE_REGISTERED (NrUePowerControl);

NrUePowerControl::NrUePowerControl ()
{
  NS_LOG_FUNCTION (this);
}

NrUePowerControl::NrUePowerControl (const Ptr<NrUePhy>& nrUePhy)
{
  m_nrUePhy = nrUePhy;
}

NrUePowerControl::~NrUePowerControl ()
{
  NS_LOG_FUNCTION (this);
}

void
NrUePowerControl::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
}

void
NrUePowerControl::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrUePowerControl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUePowerControl")
    .SetParent<LteUePowerControl> ()
    .SetGroupName("NrPhy")
    .AddConstructor<NrUePowerControl> ()
  ;
  return tid;
}

//TS 38.213 Table 7.1.1-1 and Table 7.2.1-1,  Mapping of TPC Command Field in DCI to accumulated and absolute value

void
NrUePowerControl::CalculatePuschTxPower ()
{
  NS_LOG_FUNCTION (this);
  int32_t j = 1;
  int32_t PoPusch = m_PoNominalPusch[j] + m_PoUePusch[j];

  NS_LOG_INFO ("RB: " << m_M_Pusch << " m_PoPusch: " << PoPusch
                      << " Alpha: " << m_alpha[j] << " PathLoss: " << m_pathLoss
                      << " deltaTF: " << m_deltaTF << " fc: " << m_fc);

  if ( m_M_Pusch > 0 )
    {
      m_curPuschTxPower = 10 * log10 (1.0 * m_M_Pusch) + PoPusch + m_alpha[j] * m_pathLoss + m_deltaTF + m_fc;
      m_M_Pusch = 0;
    }
  else
    {
      m_curPuschTxPower = PoPusch + m_alpha[j] * m_pathLoss + m_fc;
    }

  NS_LOG_INFO ("CalcPower: " << m_curPuschTxPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  m_curPuschTxPower = m_curPuschTxPower > m_Pcmin ? m_curPuschTxPower : m_Pcmin;
  m_curPuschTxPower = m_Pcmax < m_curPuschTxPower ? m_Pcmax : m_curPuschTxPower;
  NS_LOG_INFO ("PuschTxPower: " << m_curPuschTxPower);
}

void
NrUePowerControl::CalculatePucchTxPower ()
{
  NS_LOG_FUNCTION (this);
  m_curPucchTxPower = m_curPuschTxPower;
  NS_LOG_INFO ("PucchTxPower: " << m_curPucchTxPower);
}

void
NrUePowerControl::CalculateSrsTxPower ()
{
  NS_LOG_FUNCTION (this);
  int32_t j = 1;
  int32_t PoPusch = m_PoNominalPusch[j] + m_PoUePusch[j];

  NS_LOG_INFO ("RB: " << m_M_Pusch << " m_PoPusch: " << PoPusch
                      << " Alpha: " << m_alpha[j] << " PathLoss: " << m_pathLoss
                      << " deltaTF: " << m_deltaTF << " fc: " << m_fc);


  double pSrsOffsetValue = -10.5 + m_PsrsOffset * 1.5;

  m_curSrsTxPower = pSrsOffsetValue + 10 * log10 (m_srsBandwidth) + PoPusch + m_alpha[j] * m_pathLoss + m_fc;

  NS_LOG_INFO ("CalcPower: " << m_curSrsTxPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  m_curSrsTxPower = m_curSrsTxPower > m_Pcmin ? m_curSrsTxPower : m_Pcmin;
  m_curSrsTxPower = m_Pcmax < m_curSrsTxPower ? m_Pcmax : m_curSrsTxPower;
  NS_LOG_INFO ("SrsTxPower: " << m_curSrsTxPower);
}

} // namespace ns3
