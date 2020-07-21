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
    .AddAttribute ("PoNominalPucch",
                   "P_O_NOMINAL_PUCCH   INT (-126 ... 24), Default value -80",
                   IntegerValue (-80),
                   MakeIntegerAccessor (&NrUePowerControl::SetPoNominalPucch),
                   MakeIntegerChecker<int16_t> ())
    .AddAttribute ("PoUePucch",
                   "P_O_UE_PUCCH   INT(-8...7), Default value 0",
                   IntegerValue (0),
                   MakeIntegerAccessor (&NrUePowerControl::SetPoUePucch),
                   MakeIntegerChecker<int16_t> ())
  ;
  return tid;
}

void
NrUePowerControl::SetPoNominalPucch (int16_t value)
{
  NS_LOG_FUNCTION (this);

  if (m_PoNominalPucch.empty ())
    {
      m_PoNominalPucch.push_back (value);
      m_PoNominalPucch.push_back (value);
      m_PoNominalPucch.push_back (value);
    }
  else
    {
      m_PoNominalPucch [0] = value;
      m_PoNominalPucch [1] = value;
      m_PoNominalPucch [2] = value;
    }
}

void
NrUePowerControl::SetPoUePucch (int16_t value)
{
  NS_LOG_FUNCTION (this);
  if (m_PoUePucch.empty ())
    {
      m_PoUePucch.push_back (value);
      m_PoUePucch.push_back (value);
      m_PoUePucch.push_back (0);
    }
  else
    {
      m_PoUePucch [0] = value;
      m_PoUePucch [1] = value;
      m_PoUePucch [2] = 0;
    }
}

//TS 38.213 Table 7.1.1-1 and Table 7.2.1-1,  Mapping of TPC Command Field in DCI to accumulated and absolute value

//Implements from from ts_138213 7.1.1
void
NrUePowerControl::CalculatePuschTxPower ()
{
  NS_LOG_FUNCTION (this);
  int32_t j = 1;
  int32_t PoPusch = m_PoNominalPusch[j] + m_PoUePusch[j];

  // update RSRP value for pathloss calculation
  SetRsrp (m_nrUePhy->GetRsrp());

  NS_LOG_INFO ("RBs: " << m_M_Pusch << " m_PoPusch: " << PoPusch
                      << " Alpha: " << m_alpha[j] << " PathLoss: " << m_pathLoss
                      << " deltaTF: " << m_deltaTF << " fc: " << m_fc<<" numerology:"<<m_nrUePhy->GetNumerology());

  double puschComponent = 0;

  if (m_M_Pusch > 0)
    {
      puschComponent = 10 * log10 ( std::pow (2, m_nrUePhy->GetNumerology()) * m_M_Pusch);
      m_M_Pusch = 0;
    }

  /**
   *  - m_pathloss is a downlink path-loss estimate in dB calculated by the UE using
   *  reference signal (RS) index for a DL BWP that is linked with UL BWP b of carrier
   *  f of serving cell c
   *  m_pathloss = referenceSignalPower – higher layer filtered RSRP, where referenceSignalPower is
   *  provided by higher layers and RSRP is defined in [7, TS 38.215] for the reference serving cell and the higher
   *  layer filter configuration is defined in [12, TS 38.331] for the reference serving cell.
   *
   *  m_deltaTF currently in the code is always 0. By spec. is deltaTF is 0 when Ks is 0, and
   *  Ks is provided by higher layer parameter deltaMCS provided for each UL BWP b of each carrier f and
   *  serving cell c. According to 38.213 2.1.1. If the PUSCH transmission is over more than one layer [6, TS 38.214],
   *  then deltaTF is 0.
   *
   *  fc is accumulation or current absolute (calculation by using correction values received in TPC commands)
   */

  m_curPuschTxPower = PoPusch + puschComponent + m_alpha[j] * m_pathLoss + m_deltaTF + m_fc;

  NS_LOG_INFO ("Calculated PUSCH power:" << m_curPuschTxPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  m_curPuschTxPower = std::min (std::max(m_Pcmin, m_curPuschTxPower), m_Pcmax);

  NS_LOG_INFO ("PUSCH TxPower after min/max contstraints: " << m_curPuschTxPower);
}

//Implements from from ts_138213 7.2.1
void
NrUePowerControl::CalculatePucchTxPower ()
{
  NS_LOG_FUNCTION (this);
  int32_t j = 1;
  int32_t PoPucch = m_PoNominalPucch[j] + m_PoUePucch[j];
  // update RSRP value for pathloss calculation
  SetRsrp (m_nrUePhy->GetRsrp());

  NS_LOG_INFO ("RBs: " << m_M_Pucch << " m_PoPucch: " << PoPucch
                       << " Alpha: " << m_alpha[j] << " PathLoss: " << m_pathLoss
                       << " deltaTF: " << m_deltaTF << " fc: " << m_fc<<" numerology:"<<m_nrUePhy->GetNumerology());

  double pucchComponent = 0;

  if (m_M_Pucch > 0)
    {
      pucchComponent = 10 * log10 ( std::pow (2, m_nrUePhy->GetNumerology()) * m_M_Pucch);
      m_M_Pucch = 0;
    }

  /**
   *  - m_pathloss is a downlink path-loss estimate in dB calculated by the UE using
   *  reference signal (RS) index for a DL BWP that is linked with UL BWP b of carrier
   *  f of serving cell c
   *  m_pathloss = referenceSignalPower – higher layer filtered RSRP, where referenceSignalPower is
   *  provided by higher layers and RSRP is defined in [7, TS 38.215] for the reference serving cell and the higher
   *  layer filter configuration is defined in [12, TS 38.331] for the reference serving cell.
   *
   *  m_delta_F_Pucch is a PUCCH transmission power adjustment component for UL BWP b of
   *  carrier f of primary cell c .
   *
   *  m_deltaTF_control currently in the code is always 0. is a PUCCH transmission power adjustment
   *  component for UL BWP b of carrier f of primary cell c .
   *
   *  m_fc is equal to 0 if If PO_PUCCH value is provided by higher layers. Currently is
   *  calculated in the same way as m_fc for PUSCH
   */

  // use the latest m_fc value, since in our model there is currently no difference between them
  m_gc = m_fc;

  m_curPucchTxPower = PoPucch + pucchComponent + m_alpha[j] * m_pathLoss + m_delta_F_Pucch + m_deltaTF_control +  m_gc;
  NS_LOG_INFO ("Calculated PUCCH power: " << m_curPucchTxPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  m_curPucchTxPower = std::min (std::max (m_Pcmin, m_curPucchTxPower), m_Pcmax);
  NS_LOG_INFO ("PUCCH TxPower after min/max contstraints: " << m_curPucchTxPower);
}

void
NrUePowerControl::CalculateSrsTxPower ()
{
  NS_LOG_FUNCTION (this);
  int32_t j = 1;
  int32_t PoPusch = m_PoNominalPusch[j] + m_PoUePusch[j];

  // update RSRP value for pathloss calculation
  SetRsrp (m_nrUePhy->GetRsrp());

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
