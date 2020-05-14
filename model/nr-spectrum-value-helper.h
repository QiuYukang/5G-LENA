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

#ifndef NR_SPECTRUM_VALUE_HELPER_H
#define NR_SPECTRUM_VALUE_HELPER_H

#include <ns3/spectrum-value.h>
#include <vector>

namespace ns3 {


/**
 * \ingroup spectrum
 *
 * \brief This class provides a set of useful functions when working with spectrum model for NR
 */
class NrSpectrumValueHelper
{

public:

  static const uint8_t SUBCARRIERS_PER_RB = 12; //!< subcarriers per resource block

  /**
   * \brief Creates or obtains from a global map a spectrum model for a given bandwidth,
   * center frequency and numerology.
   * \param bandwidth of this band in Hz
   * \param centerFrequency the center frequency of this band
   * \param numerology the NR numerology with which will be compatible this spectrum model
   * \return pointer to a spectrum model with defined characteristics
   */
  static Ptr<const SpectrumModel> GetSpectrumModel (double bandwidth, double centerFrequency, uint8_t numerology);

  /**
   * \brief Creates or obtains from a global map a spectrum model with a given number of RBs,
   * center frequency and subcarrier spacing.
   * \param numRbs bandwidth in number of RBs
   * \param centerFrequency the center frequency of this band
   * \return pointer to a spectrum model with defined characteristics
   */
  static Ptr<const SpectrumModel> GetSpectrumModel (uint32_t numRbs, double centerFrequency, double subcarrierSpacing);

  /**
   * \brief Create SpectrumValue that will represent transmit power spectral density
   * \param powerTx total power in dBm
   * \param activeRbs vector of RBs that are active for this transmission
   * \param spectrumModel spectrumModel to be used to create this SpectrumValue
   */
  static Ptr<SpectrumValue> CreateTxPowerSpectralDensity (double powerTx,
                                                          const std::vector <int>& activeRbs,
                                                          const Ptr<const SpectrumModel>& spectrumModel);

  /**
    * \brief Create SpectrumValue that will represent transmit power spectral density,
    * and assuming that all RBs are active.
    * \param powerTx total power in dBm
    * \param txSm spectrumModel to be used to create this SpectrumValue
    * \return spectrum value representing power spectral density for given parameters
    */
  static Ptr<const SpectrumValue> CreateTxPowerSpectralDensity (double powerTx, const Ptr<const SpectrumModel>& txSm);

  /**
   * \brief Create a SpectrumValue that models the power spectral density of AWGN
   * \param noiseFigure the noise figure in dB  w.r.t. a reference temperature of 290K
   * \param spectrumModel the SpectrumModel instance to be used to create the output spectrum value
   * \return a pointer to a newly allocated SpectrumValue representing the noise Power Spectral Density in W/Hz for each Resource Block
   */
  static Ptr<SpectrumValue> CreateNoisePowerSpectralDensity (double noiseFigure, const Ptr<const SpectrumModel>& spectrumModel);

  /**
   * \brief Returns the effective bandwidth for the total system bandwidth
   * \param bandwidth the total system bandwidth in Hz
   * \param numerology the numerology to be used over the whole bandwidth
   * \return effective bandwidth which is the sum of bandwidths of all sub-bands, in Hz
   */
  static uint64_t GetEffectiveBandwidth (double bandwidth, uint8_t numerology);
};


} // namespace ns3



#endif /*  NR_SPECTRUM_VALUE_HELPER_H */
