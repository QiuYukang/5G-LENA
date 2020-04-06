/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef NR_EESM_ERROR_MODEL_H
#define NR_EESM_ERROR_MODEL_H

#include "nr-error-model.h"
#include <map>

namespace ns3 {

class NrL2smEesmTestCase;

/**
 * \ingroup error-models
 * \brief The NrEesmErrorModelOutput struct
 *
 * Error model output returned by the class NrEesmErrorModel.
 * \see NrEesmErrorModel
 */
struct NrEesmErrorModelOutput : public NrErrorModelOutput
{
  NrEesmErrorModelOutput () = delete;
  NrEesmErrorModelOutput (double tbler) : NrErrorModelOutput (tbler)
  {
  }

  double m_sinrEff {0.0};   //!< Effective SINR
  SpectrumValue m_sinr;     //!< perceived SINRs in the whole bandwidth
  std::vector<int> m_map;   //!< map of the active RBs
  uint32_t m_infoBits {0};  //!< number of info bits
  uint32_t m_codeBits {0};  //!< number of code bits
};

/**
 * \ingroup error-models
 *
 * \brief Eesm error model
 *
 * This class provides the BLER estimation based on EESM metrics, assuming LDPC
 * coding with block segmentation as per TS 38.212 Sect. 5.2.2, and modulation
 * and coding of MCS Table1/Table2 in TS 38.214 including up to 256-QAM. The MCS
 * and CQI Tables of NR (Table1/Table2) are specified in McsTable. The BLER-SINR
 * curves are obtained from a link level simulator (from ID) that uses LDPC
 * coding and said MCSs. In case of HARQ, the model currently follows HARQ with
 * Chase Combining, so that the SINReff is updated, but not the ECR, as per
 * IEEE 802.16m-08/004r2.
 *
 * The classes to use in your simulations are:
 *
 * * NrEesmIrT1, for IR - Table 1
 * * NrEesmIrT2, for IR - Table 2
 * * NrEesmCcT1, for CC - Table 1
 * * NrEesmCcT2, for CC - Table 2
 *
 * We provide the implementation of the Chase Combining-HARQ and the IR-HARQ
 * in NrEesmCc and NrEesmIr, respectively.
 *
 * \see NrEesmIrT1
 * \see NrEesmIrT2
 * \see NrEesmCcT1
 * \see NrEesmCcT2
 */
class NrEesmErrorModel : public NrErrorModel
{
public:
  friend NrL2smEesmTestCase;
  static TypeId GetTypeId ();

  /**
   * \brief Get the type ID of this instance
   * \return the Type ID of this instance
   */
  TypeId GetInstanceTypeId (void) const override;

  NrEesmErrorModel ();
  virtual ~NrEesmErrorModel () override;

  /**
   * \brief Get an output for the decodification error probability of a given
   * transport block, assuming the EESM method, NR LDPC coding and block
   * segmentation, MCSs Table1/Table2 in NR, and HARQ based on CC.
   *
   * \param sinr SINR vector
   * \param map RB map
   * \param size Transport block size in Bytes
   * \param mcs MCS
   * \param sinrHistory History of the retransmission
   * \return A pointer to an output, with the tbler and SINR vector, effective
   * SINR, RB map, code bits, and info bits.
   */
  virtual Ptr<NrErrorModelOutput> GetTbDecodificationStats (const SpectrumValue& sinr,
                                                            const std::vector<int>& map,
                                                            uint32_t size, uint8_t mcs,
                                                            const NrErrorModelHistory &sinrHistory) override;

  /**
   * \brief Get the SE for a given CQI, following the CQIs in NR Table1/Table2
   * in TS38.214
   */
  virtual double GetSpectralEfficiencyForCqi (uint8_t cqi) override;
  /**
   * \brief Get the SE for a given MCS, following the MCSs in NR Table1/Table2
   * in TS38.214
   */
  virtual double GetSpectralEfficiencyForMcs (uint8_t mcs) override;
  /**
   * \brief Get the payload size in Bytes, following the MCSs in NR
   */
  virtual uint32_t GetPayloadSize (uint32_t usefulSc, uint8_t mcs, uint32_t rbNum) const override;
  /**
   * \brief Get the maximum code block size in Bytes, as per NR. It depends on the LDPC
   * base graph type
   * \param tbSize Transport block size in Bytes
   * \param mcs MCS
   */
  virtual uint32_t GetMaxCbSize (uint32_t tbSize, uint8_t mcs) const override;
  /**
  * \brief Get the maximum MCS. It depends on NR tables being used
  */
  virtual uint8_t GetMaxMcs () override;

  typedef std::vector<double> DoubleVector;
  typedef std::tuple<DoubleVector, DoubleVector> DoubleTuple;
  typedef std::vector<std::vector<std::map<uint32_t, DoubleTuple> > > SimulatedBlerFromSINR;

protected:
  /**
   * \brief function to print the RB map
   * \param map the RB map
   * \return a string that contains the RB map in a readable way
   */
  std::string PrintMap (const std::vector<int> &map) const;

  /**
   * \brief compute the effective SINR for the specified MCS and SINR, according
   * to the EESM method
   *
   * \param sinr the perceived sinrs in the whole bandwidth (vector, per RB)
   * \param map the actives RBs for the TB
   * \param mcs the MCS of the TB
   * \return the effective SINR
   */
  double SinrEff (const SpectrumValue& sinr, const std::vector<int>& map, uint8_t mcs) const;

  /**
   * \brief Compute the SINR
   * \param sinr SINR of the new transmission
   * \param map RB map
   * \param mcs MCS of the transmission
   * \param sizeBit size (in bit) of the transmission
   * \param sinrHistory history of the SINR of the previous transmission
   * \return the single SINR value
   *
   * Called in GetTbBitDecodificationStats(). Please implement this function
   * in a way that calculating the SINR of the new transmission
   * takes in consideration the sinr history.
   *
   * \see NrEesmIr
   * \see NrEesmCc
   */
  virtual double ComputeSINR (const SpectrumValue& sinr, const std::vector<int>& map, uint8_t mcs,
                              uint32_t sizeBit, const NrErrorModel::NrErrorModelHistory &sinrHistory) const = 0;

  /**
   * \brief Get the "Equivalent MCS"
   * \param mcsTx MCS of the transmission
   * \return the equivalent MCS
   *
   * Called in GetTbDecodificationStats()
   * \see NrEesmIr
   * \see NrEesmCc
   */
  virtual double GetMcsEq (uint16_t mcsTx) const = 0;

  /**
   * \return pointer to a static vector that represents the beta table
   */
  virtual const std::vector<double> * GetBetaTable () const = 0;
  /**
   * \return pointer to a static vector that represents the MCS-ECR table
   */
  virtual const std::vector<double> * GetMcsEcrTable () const = 0;
  /**
   * \return pointer to a table of BLER vs SINR
   */
  virtual const SimulatedBlerFromSINR * GetSimulatedBlerFromSINR () const = 0;
  /**
   * \return pointer to a static vector that represents the MCS-M table
   */
  virtual const std::vector<uint8_t> * GetMcsMTable () const = 0;
  /**
   * \return pointer to a static vector that represents the spectral efficiency for MCS
   */
  virtual const std::vector<double> * GetSpectralEfficiencyForMcs () const = 0;
  /**
   * \return pointer to a static vector that represents the spectral efficiency for CQI
   */
  virtual const std::vector<double> * GetSpectralEfficiencyForCqi () const = 0;

private:
  static std::vector<std::string> m_bgTypeName;

  /**
   * \brief map the effective SINR into CBLER for the specified MCS and CB size,
   * according to the EESM method
   *
   * \param sinrEff effective SINR per bit of a code-block
   * \param mcs the MCS of the TB
   * \param cbSize the size of the CB in BITS
   * \return the code block error rate
   */
  double MappingSinrBler (double sinrEff, uint8_t mcs, uint32_t cbSize);

  /**
   * \brief Get an output for the decodification error probability of a given
   * transport block, assuming the EESM method, NR LDPC coding and block
   * segmentation, MCSs Table1/Table2 in NR, and HARQ based on CC.
   *
   * \param sinr SINR vector
   * \param map RB map
   * \param size Transport block size in BITS
   * \param mcs MCS
   * \param sinrHistory History of the retransmission
   * \return A pointer to an output, with the tbler and SINR vector, effective
   * SINR, RB map, code bits, and info bits.
   */
  Ptr<NrErrorModelOutput> GetTbBitDecodificationStats (const SpectrumValue& sinr,
                                                       const std::vector<int>& map,
                                                       uint32_t size, uint8_t mcs,
                                                       const NrErrorModelHistory &sinrHistory);

  /**
   * \brief Type of base graph for LDPC coding
   */
  enum GraphType
  {
    FIRST = 0,  //!< LDPC base graph 1
    SECOND = 1  //!< LDPC base graph 2
  };
  /**
   * \brief Get Base Graph type of LDPC coding (1 or 2) for the given TBS and MCS
   * of a specific NR table
   *
   * \param tbSize the size of the TB (in bits)
   * \param mcs the MCS of the TB
   * \return the GraphType used for the TB
   */
  GraphType GetBaseGraphType (uint32_t tbSize, uint8_t mcs) const;

  const std::vector<double> & GetSinrDbVectorFromSimulatedValues (GraphType graphType, uint8_t mcs, uint32_t cbSizeIndex) const;
  const std::vector<double> & GetBLERVectorFromSimulatedValues (GraphType graphType, uint8_t mcs, uint32_t cbSizeIndex) const;
};


} // namespace ns3

#endif /* NR_EESM_ERROR_MODEL_H */
