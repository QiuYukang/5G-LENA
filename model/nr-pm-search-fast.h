// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_PM_SEARCH_FAST_H
#define NR_PM_SEARCH_FAST_H

#include "nr-pm-search-full.h"

namespace ns3
{

/// @brief An implementation of NrPmSearch that uses exhaustive search for 3GPP Type-I codebooks.
/// This class differs from NrPmSearchFull in terms of search space. It determines
/// the optimal wideband rank and i1 index based on the average of the subbands channel matrix.
/// It finally creates a CQI/PMI/RI feedback message by looping over every subband,
/// finding an i2 for each of them that results in the largest achievable TB size.

class NrPmSearchFast : public NrPmSearchFull
{
  public:
    /// @brief Get TypeId
    /// @return the TypeId
    static TypeId GetTypeId();

    /// @brief Constructor for NrPmSearchFast
    NrPmSearchFast();

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

#endif // NR_PM_SEARCH_FAST_H
