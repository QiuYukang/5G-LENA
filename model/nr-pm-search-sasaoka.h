// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_PM_SEARCH_SASAOKA_H
#define NR_PM_SEARCH_SASAOKA_H

#include "nr-pm-search-full.h"

namespace ns3
{

/// @brief An implementation of NrPmSearch that uses exhaustive search for 3GPP Type-I codebooks.
/// This class differs from NrPmSearchFull in terms of search space, by using a technique
/// proposed in "PMI/RI Selection Based on Channel Capacity Increment Ratio" by
/// Naoto Sasaoka, Takumi Sasaki and Yoshio Itoh.
/// It determines the optimal wideband rank via a rank estimation based
/// on the increment of channel capacity for each additional rank.
/// It then executes an exhaustive search to find the I1 and I2 combination that produces
/// the highest mutual information.

class NrPmSearchSasaoka : public NrPmSearchFull
{
  public:
    /// @brief Get TypeId
    /// @return the TypeId
    static TypeId GetTypeId();

    /// @brief Constructor for NrPmSearchSasaoka
    NrPmSearchSasaoka();

    /**
     * @brief Create CQI feedback with optimal rank, optimal PMI, and corresponding CQI values.
     * Optimal rank is considered as the rank that maximizes the achievable TB size when using the
     * optimal PMI. The optimal WB/SB PMI values are updated based on pmiUpdate. If there is no
     * update to the PMI values, the previously found PMI values are used.
     * @param rxSignalRb the receive signal parameters (channel and interference matrices)
     * @param pmiUpdate struct that defines if WB/SB PMIs need to be updated
     * @return the CQI feedback message that contains the optimum CQI, RI, PMI, and full precoding
     * matrix (dimensions: nGnbPorts * rank * nRbs)
     */
    PmCqiInfo CreateCqiFeedbackMimo(const NrMimoSignal& rxSignalRb, PmiUpdate pmiUpdate) override;

    /// @brief Find the optimal subband precoding matrix for the given wideband precoding.
    /// @param sbNormChanMat the interference-normed channel matrix per subband
    /// @param i1 the index of the wideband precoding matrix W1
    /// @param rank the rank (number of MIMO layers)
    /// @return a struct containing wideband and subband PMIs, and full precoding matrix.
    Ptr<NrPmSearchFull::PrecMatParams> FindOptSubbandPrecoding(
        const NrIntfNormChanMat& sbNormChanMat,
        size_t i1,
        uint8_t rank) const override;

  protected:
    /**
     * @brief Create CQI feedback with optimal rank, optimal PMI, and corresponding CQI values.
     * Optimal rank is considered as the rank that maximizes the achievable TB size when using the
     * optimal PMI. The optimal WB/SB PMI values are updated based on pmiUpdate. If there is no
     * update to the PMI values, the previously found PMI values are used.
     * @param rxSignalRb the receive signal parameters (channel and interference matrices)
     * @param pmiUpdate struct that defines if WB/SB PMIs need to be updated
     * @return the CQI feedback message that contains the optimum CQI, RI, PMI, and full precoding
     * matrix (dimensions: nGnbPorts * rank * nRbs)
     */
    size_t GetWidebandI1(Ptr<const NrCbTypeOne> cb, const ComplexMatrixArray& Havg) const;

  private:
    uint8_t m_periodMaxRank;
};

} // namespace ns3

#endif // NR_PM_SEARCH_SASAOKA_H
