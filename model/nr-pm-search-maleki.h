// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_PM_SEARCH_MALEKI_H
#define NR_PM_SEARCH_MALEKI_H

#include "nr-pm-search-full.h"

namespace ns3
{

/// @brief An implementation of NrPmSearch that uses a search-free technique for 3GPP Type-I
/// codebooks.
/// This class differs from NrPmSearchFull by avoiding expensive searches, using the technique
/// proposed in "Low Complexity PMI Selection for BICM-MIMO Rate Maximization
/// in 5G New Radio Systems" by Marjan Maleki, Juening Jin, and Martin Haardt.
/// Inspired by "A Search-free Algorithm for Precoder Selection in FD-MIMO Systems
/// with DFT-based Codebooks" by Federico Penna, Hongbing Cheng, and Jungwon Lee.

class NrPmSearchMaleki : public NrPmSearchFull
{
  public:
    /// @brief Get TypeId
    /// @return the TypeId
    static TypeId GetTypeId();

    /// @brief Constructor for NrPmSearchMaleki
    NrPmSearchMaleki();

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

  private:
    uint8_t m_periodMaxRank;
};

} // namespace ns3

#endif // NR_PM_SEARCH_MALEKI_H
