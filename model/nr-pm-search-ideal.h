// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_PM_SEARCH_IDEAL_H
#define NR_PM_SEARCH_IDEAL_H

#include "nr-pm-search-full.h"

namespace ns3
{

/// @brief An implementation of NrPmSearch that find the ideal precoding matrix.

class NrPmSearchIdeal : public NrPmSearchFull
{
  public:
    /// @brief Get TypeId
    /// @return the TypeId
    static TypeId GetTypeId();

    /// @brief Constructor for NrPmSearchIdeal
    NrPmSearchIdeal()
        : NrPmSearchFull() {};

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

#endif // NR_PM_SEARCH_IDEAL_H
