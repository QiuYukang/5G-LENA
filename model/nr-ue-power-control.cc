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
#include <ns3/uinteger.h>
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
    .AddAttribute ("TSpec",
                   "Technical specification TS 36.213 or TS 38.213,"
                   "By default is set TS to 36.213. To configure TS 36.213 "
                   "set the value TS36.213, while for TS 38.213 should be "
                   "configured TS28.213.",
                   EnumValue (NrUePowerControl::TS_36_213),
                   MakeEnumAccessor (&NrUePowerControl::SetTechnicalSpec),
                   MakeEnumChecker (NrUePowerControl::TS_36_213, "TS36.213",
                                    NrUePowerControl::TS_38_213, "TS38.213"))
    .AddAttribute ("KPusch",
                   "K_PUSCH parameter needed for PUSCH accumulation state "
                   "calculation. "
                   "This value must be carefully configured according to "
                   "TS 36.213 or TS 38.213 and taking into account the type "
                   " of simulation scenario. E.g. TDD, FDD, frame structure "
                   "type, etc. For, LTE FDD or FDD-TDD and frame structure "
                   "type 1, KPusch is 4.",
                   UintegerValue (4),
                   MakeUintegerAccessor(&NrUePowerControl::SetKPusch),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("K0Pucch",
                   "K0_PUCCH parameter needed for PUCCH accumulation state "
                   "calculation. Should be configured according TS 36.213 or "
                   "TS 38.213 specification depending on TSpec attribute "
                   "setting. According to TS 38.213 for FDD or FDD-TDD and "
                   "primary cell frame structure type 1, M is equal to 1 "
                   "and K0PUCCH is 4",
                   UintegerValue (4),
                   MakeUintegerAccessor (&NrUePowerControl::SetK0Pucch),
                   MakeUintegerChecker<uint16_t> ())
     .AddAttribute ("BL_CE",
                    "When set to true means that this power control is applied to "
                    "bandwidth reduced, low complexity or coverage enhanced (BL/CE) device."
                    "By default this attribute is set to false. Default BL_CE "
                    "mode is CEModeB.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&NrUePowerControl::SetBlCe),
                    MakeBooleanChecker ());
  return tid;
}

void
NrUePowerControl::SetKPusch (uint16_t kPusch)
{
  m_k_PUSCH = kPusch;
}

void
NrUePowerControl::SetK0Pucch (uint16_t kPucch)
{
  m_k_PUSCH = kPucch;
}

void
NrUePowerControl::SetTechnicalSpec (NrUePowerControl::TechnicalSpec ts)
{
  m_technicalSpec = ts;
}

void
NrUePowerControl::SetBlCe (bool blCe)
{
  m_blCe = blCe;
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

void
NrUePowerControl::ReportTpcPusch (uint8_t tpc)
{
  NS_LOG_FUNCTION (this);

  // if closed loop is not enabled return from this function
  if (!m_closedLoop)
     {
       m_fc = 0;
       m_hc = 0;
       return;
     }

  if (m_accumulationEnabled)
    {
      m_deltaPusch.push_back (GetAccumulatedDelta(tpc));
      NS_LOG_INFO ("Reported TPC: " << (int)tpc << " delta accumulated: " << GetAccumulatedDelta (tpc) << " Fc: " << m_fc);
    }
  else
    {
      m_deltaPusch.push_back (GetAbsoluteDelta(tpc));
      NS_LOG_INFO ("Reported TPC: " << (int)tpc << " delta absolute: " << GetAbsoluteDelta (tpc) << " Fc: " << m_fc);
    }

  /**
   * If m_technicalSpec == TS_38_213 we should only save the
   * deltas, and once that transmission occasion appears then
   * apply the formula that will calculate the new value for
   * m_fc, m_hc and m_gc and reset the stored values, because
   * they are not needed to be saved any more.
   *
   * It technical specification == TS_36_213 we can update
   * immediately between it does not depend on previous
   * occasion and neither on the latest PUSCH time.
   */

  if (m_technicalSpec == TS_36_213)
    {
      // PUSCH power control accumulation or absolute value configuration
      if (m_accumulationEnabled)
        {
          if (m_deltaPusch.size () == m_k_PUSCH) // the feedback/report that should be used is from (i-m_k_PUSCH) report
            {
              if ((m_curPuschTxPower <= m_Pcmin && m_deltaPusch.at (0) < 0)
                  || (m_curPuschTxPower >= m_Pcmax && m_deltaPusch.at (0) > 0))
                {
                  //TPC commands for serving cell shall not be accumulated
                  m_deltaPusch.erase (m_deltaPusch.begin ());
                }
              else
                {
                  m_fc = m_fc + m_deltaPusch.at (0);
                  m_deltaPusch.erase (m_deltaPusch.begin ());
                }
            }
          else
            {
              m_fc = 0;
            }
        }
      else
        { // m_deltaPusch contains absolute values, assign an absolute value
          m_fc = m_deltaPusch.at (0);
          m_deltaPusch.erase (m_deltaPusch.begin ());
        }
    }
  else if (m_technicalSpec == TS_38_213)
    {
      // don't allow infinite accumulation of TPC command if they are maybe not used
      // the maximum number of command that will be saved is 100
      if (m_deltaPusch.size() == 100)
        {
          m_deltaPusch.erase (m_deltaPusch.begin ());
        }
      // update of m_fc and m_hc happens in a separated function, UpdateFc
    }
  else
    {
      NS_FATAL_ERROR ("Unknown technical specification.");
    }
}

int
NrUePowerControl::GetAbsoluteDelta (uint8_t tpc)
{
  int deltaAbsolute = 0;

  switch (tpc)
  {
    case 0:
      deltaAbsolute = -4;
      break;
    case 1:
      deltaAbsolute = -1;
      break;
    case 2:
      deltaAbsolute = 1;
      break;
    case 3:
      deltaAbsolute = 4;
      break;
    default:
      NS_FATAL_ERROR ("Unexpected TPC value");
  }
  return deltaAbsolute;
}

int
NrUePowerControl::GetAccumulatedDelta (uint8_t tpc)
{
  int deltaAccumulated = 0;

  switch (tpc)
       {
       case 0:
         deltaAccumulated = -1;
         break;
       case 1:
         deltaAccumulated = 0;
         break;
       case 2:
         deltaAccumulated = 1;
         break;
       case 3:
         deltaAccumulated = 3;
         break;
       default:
         NS_FATAL_ERROR ("Unexpected TPC value");
       }
  return deltaAccumulated;
}

void
NrUePowerControl::ReportTpcPucch (uint8_t tpc)
{
  NS_LOG_FUNCTION (this);

  // if closed loop is not enabled return from this function
  if (!m_closedLoop)
     {
       m_gc = 0;
       return;
     }

  /**
   * According to 36.213 and 38.213 there is only
   * accumulated mode for PUCCH.
   */
  m_deltaPucch.push_back (GetAccumulatedDelta (tpc));

  /**
   * If m_technicalSpec == TS_38_213 we should only save the
   * deltas, and once that transmission occasion appears then
   * apply the formula that will calculate the new value for
   * m_gc and reset the stored values, because
   * they are not needed to be saved any more.
   *
   * It technical specification == TS_36_213 we can update
   * immediately because it does not depend on previous
   * occasion and neither on the latest PUSCH time.
   */

  if (m_technicalSpec == TS_36_213)
    {
      // PUCCH power control accumulation update
      if (m_deltaPucch.size () == m_k_PUCCH) // the feedback/report that should be used is from (i-m_k_PUSCH) report
        {
          if ((m_curPucchTxPower <= m_Pcmin && m_deltaPucch.at (0) < 0) || (m_curPucchTxPower >= m_Pcmax && m_deltaPucch.at (0) > 0))
             {
               //TPC commands for should not be accumulated because the maximum or minimum is reached
               m_deltaPucch.erase (m_deltaPucch.begin ());
              }
           else
             {
                m_gc = m_gc + m_deltaPucch.at (0); // gc(i) = gc (i-1) + delta (i- KPUCCH) for TDD and FDD-TDD TS 36.213
                m_deltaPucch.erase (m_deltaPucch.begin ());
              }
         }
    }
  else if (m_technicalSpec == TS_38_213)
    {
      // don't allow infinite accumulation of TPC command if they are maybe not used
      // the maximum number of command that will be saved is 100
      if (m_deltaPucch.size() == 100)
        {
          m_deltaPucch.erase (m_deltaPucch.begin ());
        }
      // update of m_gc happens in a separated function UpdateGc
    }
  else
    {
      NS_FATAL_ERROR ("Unknown technical specification.");
    }
}

void
NrUePowerControl::UpdateFc ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (m_technicalSpec != TS_38_213, "This function is currently being used only for TS 38.213. ");

  // PUSCH power control accumulation or absolute value configuration
  if (m_accumulationEnabled)
    {
      for (const auto& i: m_deltaPusch)
        {
          m_fc +=i; // fc already hold value for fc(i-i0) occasion
        }

      m_deltaPusch.clear(); // we have used these values, no need to save them any more
    }
  else
    {
      if (m_deltaPusch.size ()>0)
        {
          m_fc = m_deltaPusch.back();
          m_deltaPusch.pop_back(); // use the last received absolute TPC command ( 7.1.1 UE behaviour)
          m_deltaPusch.clear ();
        }
    }
}

void
NrUePowerControl::UpdateGc ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (m_technicalSpec != TS_38_213, "This function is currently being used only for TS 38.213. ");

  // PUSCH power control accumulation or absolute value configuration
  for (const auto& i: m_deltaPucch)
    {
      m_gc +=i; // gc already hold value for fc(i-i0) occasion
    }

  m_deltaPucch.clear(); // we have used these values, no need to save them any more
}

//TS 38.213 Table 7.1.1-1 and Table 7.2.1-1,  Mapping of TPC Command Field in DCI to accumulated and absolute value

//Implements from from ts_138213 7.1.1
void
NrUePowerControl::CalculatePuschTxPower ()
{
  NS_LOG_FUNCTION (this);

  // if BL/CE device
  if (m_blCe)
    {
      m_curPuschTxPower = m_Pcmax;
      return;
    }
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

  /**
   * Depends on the previous occasion timing and on the number
   * of symbols since the last PDCCH, hence it should be updated
   * at the transmission occasion time
   */
  if (m_technicalSpec == TS_38_213)
    {
      UpdateFc();
    }

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

  // if BL/CE device
  if (m_blCe)
    {
      m_curPucchTxPower = m_Pcmax;
      return;
    }

  int32_t j = 1;
  int32_t PoPucch = m_PoNominalPucch[j] + m_PoUePucch[j];
  // update RSRP value for pathloss calculation
  SetRsrp (m_nrUePhy->GetRsrp());

  NS_LOG_INFO ("RBs: " << m_M_Pucch << " m_PoPucch: " << PoPucch
                       << " Alpha: " << m_alpha[j] << " PathLoss: " << m_pathLoss
                       << " deltaTF: " << m_deltaTF << " gc: " << m_gc<<" numerology:"<<m_nrUePhy->GetNumerology());

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
   *  m_gc is equal to 0 if If PO_PUCCH value is provided by higher layers. Currently is
   *  calculated in the same way as m_fc for PUSCH
   */

  /**
   * Depends on the previous occasion timing and on the number
   * of symbols since the last PDCCH, hence it should be updated
   * at the transmission occasion time.
   */
  if (m_technicalSpec == TS_38_213)
    {
      UpdateGc();
    }

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

  NS_LOG_INFO ("RBs: " << m_srsBandwidth << " m_PoPusch: " << PoPusch
                      << " Alpha: " << m_alpha[j] << " PathLoss: " << m_pathLoss
                      << " deltaTF: " << m_deltaTF << " fc: " << m_fc);


  double pSrsOffsetValue = -10.5 + m_PsrsOffset * 1.5;

  /*
   * According to TS 36.213, 5.1.3.1, alpha can be the same alpha as for PUSCH,
   * P0_SRS can be used P0_PUSCH, and m_hc (accumulation state) is equal to m_fc.
   *
   * Also, as per TS 38.213. 7.3.1, the latest m_fc value ( PUSCH power
   * control adjustment state) as described in Subclause 7.1.1, if higher layer parameter
   * srs-PowerControlAdjustmentStates indicates a same power control adjustment state for
   * SRS transmissions and PUSCH transmissions
   */
  m_hc = m_fc;

  m_curSrsTxPower = pSrsOffsetValue + 10 * log10 (m_srsBandwidth) + PoPusch + m_alpha[j] * m_pathLoss + m_hc;

  NS_LOG_INFO ("CalcPower: " << m_curSrsTxPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  m_curSrsTxPower = m_curSrsTxPower > m_Pcmin ? m_curSrsTxPower : m_Pcmin;
  m_curSrsTxPower = m_Pcmax < m_curSrsTxPower ? m_Pcmax : m_curSrsTxPower;
  NS_LOG_INFO ("SrsTxPower: " << m_curSrsTxPower);
}

double
NrUePowerControl::GetPuschTxPower (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);
  m_M_Pusch = rbNum;
  CalculatePuschTxPower ();
  m_reportPuschTxPower (m_cellId, m_rnti, m_curPuschTxPower);
  return m_curPuschTxPower;
}


double
NrUePowerControl::GetPucchTxPower (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);
  m_M_Pucch = rbNum;
  CalculatePucchTxPower ();
  m_reportPucchTxPower (m_cellId, m_rnti, m_curPucchTxPower);
  return m_curPucchTxPower;
}

double
NrUePowerControl::GetSrsTxPower (std::size_t rbNum)
{
  NS_LOG_FUNCTION (this);
  m_srsBandwidth = rbNum;
  CalculateSrsTxPower ();
  m_reportSrsTxPower (m_cellId, m_rnti, m_curSrsTxPower);
  return m_curSrsTxPower;
}

} // namespace ns3
