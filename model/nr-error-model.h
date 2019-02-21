/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
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
 */
#ifndef NRERRORMODEL_H
#define NRERRORMODEL_H

#include <ns3/object.h>
#include <vector>
#include <ns3/spectrum-value.h>

namespace ns3 {

/**
 * \defgroup error-models
 *
 * \brief Error models for the NR module
 *
 * The error models are used for calculating the error probability after a packet
 * is received (in the class MmWaveSpectrumPhy) but also can be used for calculating the
 * best MCS to use before a transmission (in the class NrAmc). Please
 * take a look to the documentation of these classes if you wish to get more
 * information about their operations.
 *
 * The error model interface is defined in the class NrErrorModel. Each model
 * should implement the pure virtual functions defined there, to be ready
 * to work with the spectrum and the amc.
 *
 * The main output of an error model is a subclass of NrErrorModelOutput.
 * The spectrum will take care of creating a vector of all instances returned
 * by the error model for the same transmission (e.g., after a retransmission
 * of the original transmission), in order to create an "history" of the outputs
 * returned by the model.
 *
 * \see NrErrorModel
 * \see NrErrorModelOutput
 */

/**
 * \ingroup error-models
 * \brief Store the output of an NRErrorModel
 *
 * Each error model
 */
struct NrErrorModelOutput : public SimpleRefCount<NrErrorModelOutput>
{
  /**
   * \brief NrErrorModelOutput default constructor (deleted)
   */
  NrErrorModelOutput () = delete;
  /**
   * \brief Official NrErrorModelOutput constructor
   * \param tbler transport block error rate to store
   */
  NrErrorModelOutput (double tbler) :
    m_tbler (tbler)
  {
  }
  /**
   * \brief ~NrErrorModelOutput
   */
  virtual ~NrErrorModelOutput ()
  {
  }

  double m_tbler     {0.0}; //!< Transport Block Error Rate
};

/**
 * \ingroup error-models
 * \brief Interface for calculating the error probability for a transport block
 *
 * Any error model that wishes to work in Spectrum or in AMC should use
 * this class as a base class. Please implement The GetInstanceTypeId method
 * in your subclasses.
 *
 * \see GetTbDecodificationStats
 */
class NrErrorModel : public Object
{
public:
  /**
   * \brief GetTypeId
   * \return the TypeId of the class
   */
  static TypeId GetTypeId ();

  /**
   * \brief Get the type ID of this instance
   * \return the Type ID of this instance
   */
  TypeId GetInstanceTypeId (void) const override;

  /**
   * \brief NrErrorModel default constructor
   */
  NrErrorModel ();

  /**
   * \brief deconstructor
   */
  virtual ~NrErrorModel ();

  /**
   * \brief Vector of previous output
   *
   *
   * Used in case of HARQ: any result will ve stored in this vector and used
   * to decode next retransmissions.
   */
  typedef std::vector<Ptr<NrErrorModelOutput> > NrErrorModelHistory;

  /**
   * \brief Get an output for the decodification error probability of a given
   * transport block.
   *
   * The subclasses can store more information by subclassing the NrErrorModelOutput
   * class, and returning a casted instance. The error model should take into
   * consideration the history, even if some time (e.g., when called by the AMC
   * or when called the first time by the spectrum model) the history will be
   * empty.
   *
   * This method should not return a nullptr, ever.
   *
   * \param sinr SINR vector
   * \param map RB map
   * \param size Transport block size
   * \param mcs MCS
   * \param history History of the retransmission
   * \return A pointer to an output, with the tbler and other customized values
   */
  virtual Ptr<NrErrorModelOutput> GetTbDecodificationStats (const SpectrumValue& sinr,
                                                            const std::vector<int>& map,
                                                            uint32_t size, uint8_t mcs,
                                                            const NrErrorModelHistory &history) = 0;

  /**
   * \brief Get the SpectralEfficiency for a given CQI
   * \param cqi CQI to take into consideration
   * \return the spectral efficiency
   */
  virtual double GetSpectralEfficiencyForCqi (uint8_t cqi) = 0;

  /**
   * \brief Get the SpectralEfficiency for a given MCS
   * \param mcs MCS to take into consideration
   * \return the spectral efficiency
   */
  virtual double GetSpectralEfficiencyForMcs (uint8_t mcs) = 0;

  /**
   * \brief Get the payload size (in bytes) for a given mcs and resource block number
   *
   * \param usefulSc Useful subcarriers
   * \param mcs MCS
   * \param rbNum Number of resource blocks (even in more than 1 symbol)
   * \return The payload size of the resource blocks, in bytes
   */
  virtual uint32_t GetPayloadSize (uint32_t usefulSc, uint8_t mcs, uint32_t rbNum) const = 0;

  /**
   * \brief Get the maximum codeblock size
   *
   * \param tbSize Transport block size for which calculate the CB size
   * \param mcs MCS of the transmission
   * \return the codeblock size
   */
  virtual uint32_t GetMaxCbSize (uint32_t tbSize, uint8_t mcs) const = 0;

  /**
   * \brief Get the maximum MCS
   *
   * \return the maximum MCS that is permitted with the error model
   */
  virtual uint8_t GetMaxMcs () = 0;
};

} // namespace ns3
#endif // NRERRORMODEL_H