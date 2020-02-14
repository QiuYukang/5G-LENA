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
 * This class provides the BLER estimation based on EESM metrics, assuming LDPC
 * coding with block segmentation as per TS 38.212 Sect. 5.2.2, and modulation
 * and coding of MCS Table1/Table2 in TS 38.214 including up to 256-QAM. The MCS
 * and CQI Tables of NR (Table1/Table2) are specified in McsTable. The BLER-SINR
 * curves are obtained from a link level simulator (from ID) that uses LDPC
 * coding and said MCSs. In case of HARQ, the model currently follows HARQ with
 * Chase Combining, so that the SINReff is updated, but not the ECR, as per
 * IEEE 802.16m-08/004r2.
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
   * \brief NR Table to be used for MCSs and CQIs in TS38.214.
   */
  enum McsTable
  {
    McsTable1 = 0,  //!< NR MCS/CQI Table1 (corresponds to tables 5.1.3.1-1 and 5.2.2.1-2 in TS38.214)
    McsTable2 = 1   //!< NR MCS/CQI Table2 (refers to tables 5.1.3.1-2 and 5.2.2.1-3 in TS38.214)
  };
  /**
   * \brief HARQ method used for PHY abstraction retransmissions combining
   */
  enum HarqMethod
  {
    HarqCc = 0,  //!< HARC Chase Combining
    HarqIr = 1   //!< HARQ Incremental Redundancy
  };

  /**
   * \brief Get an output for the decodification error probability of a given
   * transport block, assuming the EESM method, NR LDPC coding and block
   * segmentation, MCSs Table1/Table2 in NR, and HARQ based on CC.
   *
   * \param sinr SINR vector
   * \param map RB map
   * \param size Transport block size in Bytes
   * \param mcs MCS
   * \param history History of the retransmission
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

  /**
   * \brief Set the NR Tables to be used for MCSs and CQIs in TS38.214, based on
   * m_mcsTable parameter. It icludes configuraiton of the beta tables
   * ECR tables, BLER-SINR tables, M tables, SE for MCS tables, SE for CQI tables
   */
  void SetMcsTable (McsTable input);
  /**
   * \brief Get the NR Table being used for MCSs and CQIs in TS38.214
   */
  McsTable GetMcsTable () const;

  /**
   * \brief Set the HARQ method, based on m_harqMethod parameter.
   */
  void SetHarqMethod (HarqMethod input);
  /**
   * \brief Get the HARQ method
   */
  HarqMethod GetHarqMethod () const;

  typedef std::vector<double> DoubleVector;
  typedef std::tuple<DoubleVector, DoubleVector> DoubleTuple;
  typedef std::vector<std::vector<std::map<uint32_t, DoubleTuple> > > SimulatedBlerFromSINR;


private:
  static std::vector<std::string> m_bgTypeName;

  McsTable m_mcsTable {McsTable1};
  HarqMethod m_harqMethod {HarqCc};
  const std::vector<double> *m_betaTable {nullptr};
  const std::vector<double> *m_mcsEcrTable {nullptr};
  const SimulatedBlerFromSINR *m_simulatedBlerFromSINR {nullptr};
  const std::vector<uint8_t> *m_mcsMTable {nullptr};
  const std::vector<double> *m_spectralEfficiencyForMcs {nullptr};
  const std::vector<double> *m_spectralEfficiencyForCqi {nullptr};

  /**
   * \brief compute the effective SINR for the specified MCS and SINR, according
   * to the EESM method
   *
   * \param sinr the perceived sinrs in the whole bandwidth (vector, per RB)
   * \param map the actives RBs for the TB
   * \param mcs the MCS of the TB
   * \return the effective SINR
   */
  double SinrEff (const SpectrumValue& sinr, const std::vector<int>& map, uint8_t mcs);

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
   * \param history History of the retransmission
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
