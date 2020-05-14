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

#include "nr-spectrum-value-helper.h"
#include <map>
#include <cmath>
#include <ns3/log.h>
#include <ns3/fatal-error.h>
#include <ns3/string.h>
#include <ns3/abort.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSpectrumValueHelper");


struct NrSpectrumModelId
{
  /**
   * Constructor
   *
   * \param f center frequency
   * \param b bandwidth in RBs
   * \param s subcarrierSpacing
   */
  NrSpectrumModelId (double f, uint16_t b, double s);
  double frequency; ///<
  uint16_t bandwidth; ///< bandwidth
  double subcarrierSpacing;
};

NrSpectrumModelId::NrSpectrumModelId (double f, uint16_t b, double s)
  : frequency (f),
  bandwidth (b),
  subcarrierSpacing (s)
{
}

/**
 * \brief Operator < so that it can be the key in a g_nrSpectrumModelMap
 * \param a lhs
 * \param b rhs
 * \returns true if earfcn less than or if earfcn equal and bandwidth less than or if earfcn and bandwidth equal and sucarrier spacing less than
 */
bool
operator < (const NrSpectrumModelId& a, const NrSpectrumModelId& b)
{
  return ( (a.frequency < b.frequency) ||
           ( (a.frequency == b.frequency) && (a.bandwidth < b.bandwidth) ) ||
           ( (a.frequency == b.frequency) && (a.bandwidth == b.bandwidth ) && (a.subcarrierSpacing < b.subcarrierSpacing))
         );
}

static std::map<NrSpectrumModelId, Ptr<SpectrumModel> > g_nrSpectrumModelMap; ///< nr spectrum model map


Ptr<const SpectrumModel>
NrSpectrumValueHelper::GetSpectrumModel (double bandwidth, double centerFrequency, uint8_t numerology)
{
  uint32_t scSpacing = 15000 * static_cast<uint32_t> (std::pow (2, numerology));
  uint32_t numRbs = static_cast<uint32_t> (bandwidth / (scSpacing * SUBCARRIERS_PER_RB));

  NS_ABORT_MSG_IF (numRbs == 0, "Total bandwidth is less than the RB width. Total bandwidth should be increased.");

  return GetSpectrumModel (numRbs, centerFrequency, scSpacing);
}

Ptr<const SpectrumModel>
NrSpectrumValueHelper::GetSpectrumModel (uint32_t numRbs, double centerFrequency, double subcarrierSpacing)
{
  NS_LOG_FUNCTION (centerFrequency << numRbs << subcarrierSpacing);

  NrSpectrumModelId modelId = NrSpectrumModelId (centerFrequency, numRbs, subcarrierSpacing);

  if (g_nrSpectrumModelMap.find (modelId) != g_nrSpectrumModelMap.end ())
    {
      return g_nrSpectrumModelMap.find (modelId)->second;
    }

  NS_ASSERT_MSG (centerFrequency != 0, "The carrier frequency cannot be set to 0");
  double f = centerFrequency - (numRbs * subcarrierSpacing * SUBCARRIERS_PER_RB / 2.0);
  Bands rbs; // A vector representing all resource blocks
  for (uint32_t numrb = 0; numrb < numRbs; ++numrb)
    {
      BandInfo rb;
      rb.fl = f;
      f += subcarrierSpacing * SUBCARRIERS_PER_RB / 2;
      rb.fc = f;
      f += subcarrierSpacing * SUBCARRIERS_PER_RB / 2;
      rb.fh = f;
      rbs.push_back (rb);
    }

  Ptr<SpectrumModel> model = Create<SpectrumModel> (rbs);
  // save this model to the map of spectrum models
  g_nrSpectrumModelMap.insert (std::pair<NrSpectrumModelId, Ptr<SpectrumModel> > (modelId, model));

  return model;
}

Ptr<SpectrumValue>
NrSpectrumValueHelper::CreateTxPowerSpectralDensity (double powerTx, const std::vector <int>& activeRbs, const Ptr<const SpectrumModel>& spectrumModel)
{
  NS_LOG_FUNCTION (powerTx << activeRbs << spectrumModel);
  Ptr<SpectrumValue> txPsd = Create <SpectrumValue> (spectrumModel);
  double powerTxW = std::pow (10., (powerTx - 30) / 10);
  double txPowerDensity = 0;
  double subbandWidth = (spectrumModel->Begin()->fh - spectrumModel->Begin()->fl);
  NS_ABORT_MSG_IF(subbandWidth < 180000, "Erroneous spectrum model. RB width should be equal or greater than 180KHz");
  txPowerDensity = powerTxW / (subbandWidth * spectrumModel->GetNumBands());
  for (std::vector <int>::const_iterator it = activeRbs.begin (); it != activeRbs.end (); it++)
    {
      int rbId = (*it);
      (*txPsd)[rbId] = txPowerDensity;
    }
  NS_LOG_LOGIC (*txPsd);
  return txPsd;
}

Ptr<const SpectrumValue>
NrSpectrumValueHelper::CreateTxPowerSpectralDensity (double powerTx, const Ptr<const SpectrumModel>& txSm)
{
  std::vector<int> activeRbs;
  for (size_t rbId = 0; rbId < txSm->GetNumBands(); rbId++)
    {
      activeRbs.push_back(rbId);
    }
  return CreateTxPowerSpectralDensity (powerTx, activeRbs, txSm);
}


Ptr<SpectrumValue>
NrSpectrumValueHelper::CreateNoisePowerSpectralDensity (double noiseFigureDb, const Ptr<const SpectrumModel>& spectrumModel)
{
  NS_LOG_FUNCTION (noiseFigureDb << spectrumModel);
  const double kT_dBm_Hz = -174.0;  // dBm/Hz
  double kT_W_Hz = std::pow (10.0, (kT_dBm_Hz - 30) / 10.0);
  double noiseFigureLinear = std::pow (10.0, noiseFigureDb / 10.0);
  double noisePowerSpectralDensity =  kT_W_Hz * noiseFigureLinear;

  Ptr<SpectrumValue> noisePsd = Create <SpectrumValue> (spectrumModel);
  (*noisePsd) = noisePowerSpectralDensity;
  return noisePsd;
}

uint64_t
NrSpectrumValueHelper::GetEffectiveBandwidth (double bandwidth, uint8_t numerology)
{
  NS_LOG_FUNCTION (bandwidth << numerology);
  uint32_t scSpacing = 15000 * static_cast<uint32_t> (std::pow (2, numerology));
  uint32_t numRbs = static_cast<uint32_t> (bandwidth / (scSpacing * SUBCARRIERS_PER_RB));
  NS_LOG_DEBUG ("Total bandwidth: "<<bandwidth<<" effective bandwidth:"<<numRbs * (scSpacing * SUBCARRIERS_PER_RB));
  return numRbs * (scSpacing * SUBCARRIERS_PER_RB);
}

} // namespace ns3
