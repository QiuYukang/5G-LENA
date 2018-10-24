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



#include "mmwave-amc.h"
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/math.h>
#include "ns3/enum.h"
#include "mmwave-mi-error-model.h"

NS_LOG_COMPONENT_DEFINE ("MmWaveAmc");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MmWaveAmc);

static const double SpectralEfficiencyForCqi[16] = {
  0.0,     // out of range
  0.15, 0.23, 0.38, 0.6, 0.88, 1.18,
  1.48, 1.91, 2.41,
  2.73, 3.32, 3.9, 4.52, 5.12, 5.55
};

static const double SpectralEfficiencyForMcs[32] = {
  0.15, 0.19, 0.23, 0.31, 0.38, 0.49, 0.6, 0.74, 0.88, 1.03, 1.18,
  1.33, 1.48, 1.7, 1.91, 2.16, 2.41, 2.57,
  2.73, 3.03, 3.32, 3.61, 3.9, 4.21, 4.52, 4.82, 5.12, 5.33, 5.55,
  0, 0, 0
};

static const uint8_t ModulationSchemeForMcs[32] = {
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  4, 4, 4, 4, 4, 4, 4,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
  2,      // reserved
  4,      // reserved
  6,      // reserved
};

MmWaveAmc::MmWaveAmc ()
{
  NS_LOG_ERROR ("This construcor should not be invoked");
}

MmWaveAmc::MmWaveAmc (Ptr<MmWavePhyMacCommon> ConfigParams)
  : m_phyMacConfig (ConfigParams)
{
  NS_LOG_INFO ("Initialze AMC module");
}

MmWaveAmc::~MmWaveAmc ()
{

}

TypeId
MmWaveAmc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveAmc")
    .SetParent<Object> ()
    .AddConstructor<MmWaveAmc> ()
    .AddAttribute ("Ber",
                   "The requested BER in assigning MCS (default is 0.00005).",
                   DoubleValue (0.00005),
                   MakeDoubleAccessor (&MmWaveAmc::m_ber),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("AmcModel",
                   "AMC model used to assign CQI",
                   EnumValue (MmWaveAmc::MiErrorModel),
                   MakeEnumAccessor (&MmWaveAmc::m_amcModel),
                   MakeEnumChecker (MmWaveAmc::MiErrorModel, "Vienna",
                                    MmWaveAmc::PiroEW2010, "PiroEW2010"));
  return tid;
}

int
MmWaveAmc::GetMcsFromCqi (int cqi)
{
  NS_LOG_FUNCTION (cqi);
  NS_ASSERT_MSG (cqi >= 0 && cqi <= 15, "CQI must be in [0..15] = " << cqi);
  double spectralEfficiency = SpectralEfficiencyForCqi[cqi];
  int mcs = 0;
  while ((mcs < 28) && (SpectralEfficiencyForMcs[mcs + 1] <= spectralEfficiency))
    {
      ++mcs;
    }
  NS_LOG_LOGIC ("mcs = " << mcs);
  return mcs;
}

// NPRB in all symbols
double
MmWaveAmc::GetSpectralEfficiency (unsigned mcs, unsigned nprb) const
{
  uint32_t rscElement = (m_phyMacConfig->GetNumScsPerRb () - m_phyMacConfig->GetNumRefScPerRb ()) * nprb;
  double Rcode = McsEcrTable[mcs];
  uint8_t Qm = ModulationSchemeForMcs[mcs];

  double spectralEfficiency = rscElement * Qm * Rcode;

  NS_LOG_INFO (" mcs:" << mcs <<
               " subcarriers" << m_phyMacConfig->GetNumScsPerRb () * m_phyMacConfig->GetBandwidthInRbs () <<
               " rsc element:" << rscElement);

  return spectralEfficiency;
}

// NPRB in 1 symbol
double
MmWaveAmc::GetSpectralEfficiency (unsigned mcs, unsigned nsymb, unsigned nprb) const
{
  return GetSpectralEfficiency (mcs, nsymb * nprb);
}

uint32_t
MmWaveAmc::GetTbSizeFromMcsSymbols (unsigned mcs, unsigned nprb) const
{
  NS_LOG_FUNCTION (mcs);
  NS_ASSERT_MSG (mcs < 29, "MCS=" << mcs);
  NS_ABORT_IF (m_phyMacConfig->GetNumScsPerRb () < m_phyMacConfig->GetNumRefScPerRb ());

  uint32_t tbSize = 0;
  double cbSize = 6144;    //max size of a code-block (including m_crcLen)
  double spectralEfficiency = GetSpectralEfficiency (mcs, nprb);

  if (spectralEfficiency >= m_crcLen)
    {
      tbSize = static_cast<uint32_t> (spectralEfficiency - m_crcLen);
    }

  if (tbSize > cbSize)
    {
      double C = ceil (tbSize / cbSize);
      tbSize = static_cast<uint32_t> (spectralEfficiency - static_cast<uint32_t> (C * m_crcLen));   //subtract bits of m_crcLen used in code-blocks.
    }

  NS_LOG_INFO ("Time:" << Simulator::Now () << " mcs:" << mcs <<
               " subcarriers" << m_phyMacConfig->GetNumScsPerRb () * m_phyMacConfig->GetBandwidthInRbs () <<
               " TB size:" << tbSize);

  return tbSize;
}

uint32_t
MmWaveAmc::GetTbSizeFromMcsSymbols (unsigned mcs, unsigned nsymb, unsigned nprb) const
{
  return GetTbSizeFromMcsSymbols (mcs, nsymb * nprb);
}

int
MmWaveAmc::GetNumSymbolsFromTbsMcs (unsigned tbSize, unsigned mcs)
{
  NS_LOG_FUNCTION (mcs);
  NS_ASSERT_MSG (mcs < 29, "MCS=" << mcs);
  //unsigned itb = McsToItbs[mcs];
  int rscElementPerSym = (m_phyMacConfig->GetNumScsPerRb () * m_phyMacConfig->GetBandwidthInRbs ()
                          - m_phyMacConfig->GetNumRefScPerSym ());
  double Rcode = McsEcrTable[mcs];
  double Qm = ModulationSchemeForMcs[mcs];
  uint16_t cbSize = 6144;    //max size of a code-block (including m_crcLen)
  if (tbSize > cbSize)
    {
      int C = ceil ((double)tbSize / ((double)(6144)));
      tbSize += C * m_crcLen; //subtract bits of m_crcLen used in code-blocks.
    }
  int reqRscElement = (tbSize + m_crcLen) / (Qm * Rcode);

  return ceil ((double)reqRscElement / (double)rscElementPerSym);
}

std::vector<int>
MmWaveAmc::CreateCqiFeedbacksTdma (const SpectrumValue& sinr, uint8_t numSym, unsigned nprb)
{
  NS_LOG_FUNCTION (this);

  std::vector<int> cqi;
  Values::const_iterator it;
  if (m_amcModel == PiroEW2010)
    {
      //use PiroEW2010 model
      for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
        {
          double sinr_ = (*it);
          if (sinr_ == 0.0)
            {
              cqi.push_back (-1);   // SINR == 0 (linear units) means no signal in this RB
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

              int cqi_ = GetCqiFromSpectralEfficiency (s);

              NS_LOG_LOGIC (" PRB =" << cqi.size ()
                                     << ", sinr = " << sinr_
                                     << " (=" << 10 * std::log10 (sinr_) << " dB)"
                                     << ", spectral efficiency =" << s
                                     << ", CQI = " << cqi_ << ", BER = " << m_ber);

              cqi.push_back (cqi_);
            }
        }
    }
  else if (m_amcModel == MiErrorModel)
    {
      int rbId = 0;
      for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
        {
          uint8_t mcs = 0;
          TbStats_t tbStats;
          std::vector <int> rbMap;
          rbMap.push_back (rbId++);
          while (mcs <= 28)
            {
              MmWaveHarqProcessInfoList_t harqInfoList;
              tbStats = MmWaveMiErrorModel::GetTbDecodificationStats (sinr, rbMap,
                                                                      GetTbSizeFromMcsSymbols (mcs, numSym, nprb) / 8,
                                                                      mcs, harqInfoList);
              if (tbStats.tbler > 0.1)
                {
                  break;
                }
              mcs++;

            }
          if (mcs > 0)
            {
              mcs--;
            }
          NS_LOG_DEBUG (this << "\t MCS " << (uint16_t)mcs << " TBLER " << tbStats.tbler);
          int rbCqi = 0;
          if ((tbStats.tbler > 0.1)&&(mcs == 0))
            {
              rbCqi = 0;
            }
          else if (mcs == 28)
            {
              rbCqi = 15;   // all MCSs can guarantee the 10 % of BER
            }
          else
            {
              double s = SpectralEfficiencyForMcs[mcs];
              rbCqi = 0;
              while ((rbCqi < 15) && (SpectralEfficiencyForCqi[rbCqi + 1] <= s))
                {
                  ++rbCqi;
                }
            }
          NS_LOG_DEBUG (this << "\t MCS " << (uint16_t)mcs << "-> CQI " << rbCqi);
          cqi.push_back (rbCqi);
        }
    }
  return cqi;
}

uint8_t
MmWaveAmc::CreateCqiFeedbackWbTdma (const SpectrumValue& sinr, uint8_t numSym, uint32_t tbSize, uint8_t &mcs)
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
  else if (m_amcModel == MiErrorModel)
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
      sinrAvg /= rbId;

      mcs = 0;
      TbStats_t tbStats;
      while (mcs <= 28)
        {
          MmWaveHarqProcessInfoList_t harqInfoList;
          tbStats = MmWaveMiErrorModel::GetTbDecodificationStats (sinr, rbMap, tbSize, mcs, harqInfoList);
          if (tbStats.tbler > 0.1)
            {
              break;
            }
          mcs++;
        }
      if (mcs > 0)
        {
          mcs--;
        }
      //  MmWaveHarqProcessInfoList_t harqInfoList;
      //  TbStats_t tbStatsFinal = MmWaveMiErrorModel::GetTbDecodificationStats (sinr, rbMap, tbSize, mcs, harqInfoList);
      //  NS_LOG_UNCOND ("TBLER " << tbStatsFinal.tbler << " for chunks " << rbMap.size () << " numSym "
      //                 << (unsigned)numSym << " tbSize " << tbSize << " mcs " << (unsigned)mcs << " sinr " << sinrAvg);
      //  NS_LOG_UNCOND (sinr);
      if ((tbStats.tbler > 0.1)&&(mcs == 0))
        {
          cqi = 0;
        }
      else if (mcs == 28)
        {
          cqi = 15;   // all MCSs can guarantee the 10 % of BER
        }
      else
        {
          double s = SpectralEfficiencyForMcs[mcs];
          cqi = 0;
          while ((cqi < 15) && (SpectralEfficiencyForCqi[cqi + 1] <= s))
            {
              ++cqi;
            }
        }
      NS_LOG_DEBUG (this << "\t MCS " << (uint16_t)mcs << "-> CQI " << cqi);
    }
  return cqi;
}

int
MmWaveAmc::GetCqiFromSpectralEfficiency (double s)
{
  NS_LOG_FUNCTION (s);
  NS_ASSERT_MSG (s >= 0.0, "negative spectral efficiency = " << s);
  int cqi = 0;
  while ((cqi < 15) && (SpectralEfficiencyForCqi[cqi + 1] < s))
    {
      ++cqi;
    }
  NS_LOG_LOGIC ("cqi = " << cqi);
  return cqi;
}

int
MmWaveAmc::GetMcsFromSpectralEfficiency (double s)
{
  NS_LOG_FUNCTION (s);
  NS_ASSERT_MSG (s >= 0.0, "negative spectral efficiency = " << s);
  int mcs = 0;
  while ((mcs < 28) && (SpectralEfficiencyForMcs[mcs + 1] < s))
    {
      ++mcs;
    }
  NS_LOG_LOGIC ("cqi = " << mcs);
  return mcs;
}

}

