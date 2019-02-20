/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */



#include "nr-amc.h"
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/math.h>
#include "ns3/enum.h"
#include "nr-error-model.h"
#include "nr-lte-mi-error-model.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrAmc");
NS_OBJECT_ENSURE_REGISTERED (NrAmc);

NrAmc::NrAmc (const Ptr<MmWavePhyMacCommon> & configParams)
  : m_phyMacConfig (configParams)
{
  NS_LOG_INFO ("Initialze AMC module");

  // We already need the attributes
  ObjectBase::ConstructSelf (AttributeConstructionList ());

  ObjectFactory factory;
  factory.SetTypeId (m_errorModelType);
  m_errorModel = DynamicCast<NrErrorModel> (factory.Create ());
  NS_ASSERT (m_errorModel != nullptr);
}

NrAmc::~NrAmc ()
{
}

TypeId
NrAmc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrAmc")
    .SetParent<Object> ()
    .AddConstructor<NrAmc> ()
    .AddAttribute ("Ber",
                   "The requested BER in assigning MCS (default is 0.00005). Only used with Piro model",
                   DoubleValue (0.00005),
                   MakeDoubleAccessor (&NrAmc::m_ber),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AmcModel",
                   "AMC model used to assign CQI",
                   EnumValue (NrAmc::ErrorModel),
                   MakeEnumAccessor (&NrAmc::m_amcModel),
                   MakeEnumChecker (NrAmc::ErrorModel, "ErrorModel",
                                    NrAmc::PiroEW2010, "PiroEW2010"))
    .AddAttribute ("ErrorModelType",
                   "Type of the Error Model to use when AmcModel is set to ErrorModel",
                   TypeIdValue (NrLteMiErrorModel::GetTypeId ()),
                   MakeTypeIdAccessor (&NrAmc::m_errorModelType),
                   MakeTypeIdChecker ())
  ;
  return tid;
}

TypeId
NrAmc::GetInstanceTypeId() const
{
  return NrAmc::GetTypeId ();
}

uint8_t
NrAmc::GetMcsFromCqi (uint8_t cqi) const
{
  NS_LOG_FUNCTION (cqi);
  NS_ASSERT_MSG (cqi >= 0 && cqi <= 15, "CQI must be in [0..15] = " << cqi);

  double spectralEfficiency = m_errorModel->GetSpectralEfficiencyForCqi (cqi);
  uint8_t mcs = 0;

  while ((mcs < m_errorModel->GetMaxMcs ()) && (m_errorModel->GetSpectralEfficiencyForMcs (mcs + 1) <= spectralEfficiency))
    {
      ++mcs;
    }

  NS_LOG_LOGIC ("mcs = " << mcs);

  return mcs;
}

uint32_t
NrAmc::GetPayloadSize (uint8_t mcs, uint32_t nprb) const
{
  return m_errorModel->GetPayloadSize (m_phyMacConfig->GetNumScsPerRb () - m_phyMacConfig->GetNumRefScPerRb (),
                                       mcs, nprb);
}

uint32_t
NrAmc::CalculateTbSize (uint8_t mcs, uint32_t nprb) const
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (mcs) << nprb);

  NS_ASSERT_MSG (mcs <= m_errorModel->GetMaxMcs (), "MCS=" << static_cast<uint32_t> (mcs) <<
                 " while maximum MCS is " << static_cast<uint32_t> (m_errorModel->GetMaxMcs ()));

  uint32_t payloadSize = GetPayloadSize (mcs, nprb);
  uint32_t tbSize = payloadSize;

  uint32_t cbSize = m_errorModel->GetMaxCbSize (payloadSize, mcs); // max size of a code-block (including m_crcLen)

  if (payloadSize >= m_crcLen)
    {
      tbSize = payloadSize - m_crcLen;
    }

  if (tbSize > cbSize)
    {
      double C = ceil (tbSize / cbSize);
      tbSize = payloadSize - static_cast<uint32_t> (C * m_crcLen);   //subtract bits of m_crcLen used in code-blocks.
    }

  NS_LOG_INFO (" mcs:" << mcs <<
               " subcarriers" << m_phyMacConfig->GetNumScsPerRb () * m_phyMacConfig->GetBandwidthInRbs () <<
               " TB size:" << tbSize);

  return tbSize;
}

uint8_t
NrAmc::CreateCqiFeedbackWbTdma (const SpectrumValue& sinr, uint32_t tbSize,
                                    uint8_t &mcs)
{
  NS_LOG_FUNCTION (this);

  // produces a single CQI/MCS value

  //std::vector<int> cqi;
  uint8_t cqi = 0;
  double seAvg = 0;
  double mcsAvg = 0;
  double cqiAvg = 0;

  Values::const_iterator it;
  if (m_amcModel == PiroEW2010)
    {
      //use PiroEW2010 model
      uint32_t rbNum = 0;
      for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
        {
          double sinr_ = (*it);
          if (sinr_ == 0.0)
            {
              //cqi.push_back (-1); // SINR == 0 (linear units) means no signal in this RB
            }
          else
            {
              /*
               * Compute the spectral efficiency from the SINR
               *                                        SINR
               * spectralEfficiency = log2 (1 + -------------------- )
               *                                    -ln(5*BER)/1.5
               * NB: SINR must be expressed in linear units
               */

              double s = log2 ( 1 + ( sinr_ / ( (-std::log (5.0 * m_ber )) / 1.5) ));
              seAvg += s;

              int cqi_ = GetCqiFromSpectralEfficiency (s);
              mcsAvg += GetMcsFromSpectralEfficiency (s);
              cqiAvg += cqi_;
              rbNum++;

              NS_LOG_LOGIC (" PRB =" << sinr.GetSpectrumModel ()->GetNumBands ()
                                     << ", sinr = " << sinr_
                                     << " (=" << 10 * std::log10 (sinr_) << " dB)"
                                     << ", spectral efficiency =" << s
                                     << ", CQI = " << cqi_ << ", BER = " << m_ber);
              //cqi.push_back (cqi_);
            }
        }
      seAvg /= rbNum;
      mcsAvg /= rbNum;
      cqiAvg /= rbNum;
      cqi = ceil (cqiAvg);  //GetCqiFromSpectralEfficiency (seAvg);
      mcs = GetMcsFromSpectralEfficiency (seAvg);   //ceil(mcsAvg);
    }
  else if (m_amcModel == ErrorModel)
    {
      std::vector <int> rbMap;
      int rbId = 0;
      double sinrAvg = 0;
      for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
        {
          if (*it != 0.0)
            {
              rbMap.push_back (rbId);
              sinrAvg += *it;
            }
          rbId += 1;
        }
      sinrAvg /= rbMap.size ();

      mcs = 0;
      Ptr<NrErrorModelOutput> output;
      while (mcs <= m_errorModel->GetMaxMcs ())
        {
          output = m_errorModel->GetTbDecodificationStats (sinr, rbMap, tbSize, mcs,
                                                           NrErrorModel::NrErrorModelHistory ());
          if (output->m_tbler > 0.1)
            {
              break;
            }
          mcs++;
        }

      if (mcs > 0)
        {
          mcs--;
        }

      if ((output->m_tbler > 0.1) && (mcs == 0))
        {
          cqi = 0;
        }
      else if (mcs == m_errorModel->GetMaxMcs ())
        {
          cqi = 15;   // all MCSs can guarantee the 10 % of BER
        }
      else
        {
          double s = m_errorModel->GetSpectralEfficiencyForMcs (mcs);
          cqi = 0;
          while ((cqi < 15) && (m_errorModel->GetSpectralEfficiencyForCqi (cqi + 1) <= s))
            {
              ++cqi;
            }
        }
      NS_LOG_DEBUG (this << "\t MCS " << (uint16_t)mcs << "-> CQI " << cqi);
    }
  return cqi;
}

uint8_t
NrAmc::GetCqiFromSpectralEfficiency (double s)
{
  NS_LOG_FUNCTION (s);
  NS_ASSERT_MSG (s >= 0.0, "negative spectral efficiency = " << s);
  uint8_t cqi = 0;
  while ((cqi < 15) && (m_errorModel->GetSpectralEfficiencyForCqi (cqi + 1) < s))
    {
      ++cqi;
    }
  NS_LOG_LOGIC ("cqi = " << cqi);
  return cqi;
}

uint8_t
NrAmc::GetMcsFromSpectralEfficiency (double s)
{
  NS_LOG_FUNCTION (s);
  NS_ASSERT_MSG (s >= 0.0, "negative spectral efficiency = " << s);
  uint8_t mcs = 0;
  while ((mcs < m_errorModel->GetMaxMcs ()) && (m_errorModel->GetSpectralEfficiencyForMcs (mcs + 1) < s))
    {
      ++mcs;
    }
  NS_LOG_LOGIC ("cqi = " << mcs);
  return mcs;
}

uint32_t
NrAmc::GetMaxMcs() const
{
  NS_LOG_FUNCTION (this);
  return m_errorModel->GetMaxMcs ();
}

} // namespace ns3

