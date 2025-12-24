// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mcs-tables.h"

#include "ns3/abort.h"

namespace ns3
{

/**
 * @brief Table of modulation order (Q_m) for MCS Table 1
 *
 * 29 MCS values (0-28) as per TS 38.214 Table 5.1.3.1-1
 */
const std::vector<uint8_t> NrMcsTables::m_mcsMTable1 = {
    // QPSK (Q_m = 2)
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    // 16QAM (Q_m = 4)
    4,
    4,
    4,
    4,
    4,
    4,
    4,
    // 64QAM (Q_m = 6)
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6};

/**
 * @brief Table of Effective Code Rate (ECR) for MCS Table 1
 *
 * Code rates R * 1024 from TS 38.214 Table 5.1.3.1-1.
 */
const std::vector<double> NrMcsTables::m_mcsEcrTable1 = {
    // QPSK (Q_m = 2): MCS 0-9
    120.0 / 1024.0, // MCS 0
    157.0 / 1024.0, // MCS 1
    193.0 / 1024.0, // MCS 2
    251.0 / 1024.0, // MCS 3
    308.0 / 1024.0, // MCS 4
    379.0 / 1024.0, // MCS 5
    449.0 / 1024.0, // MCS 6
    526.0 / 1024.0, // MCS 7
    602.0 / 1024.0, // MCS 8
    679.0 / 1024.0, // MCS 9
    // 16QAM (Q_m = 4): MCS 10-16
    340.0 / 1024.0, // MCS 10
    378.0 / 1024.0, // MCS 11
    434.0 / 1024.0, // MCS 12
    490.0 / 1024.0, // MCS 13
    553.0 / 1024.0, // MCS 14
    616.0 / 1024.0, // MCS 15
    658.0 / 1024.0, // MCS 16
    // 64QAM (Q_m = 6): MCS 17-28
    438.0 / 1024.0, // MCS 17
    466.0 / 1024.0, // MCS 18
    517.0 / 1024.0, // MCS 19
    567.0 / 1024.0, // MCS 20
    616.0 / 1024.0, // MCS 21
    666.0 / 1024.0, // MCS 22
    719.0 / 1024.0, // MCS 23
    772.0 / 1024.0, // MCS 24
    822.0 / 1024.0, // MCS 25
    873.0 / 1024.0, // MCS 26
    910.0 / 1024.0, // MCS 27
    948.0 / 1024.0  // MCS 28
};

/**
 * @brief Table of spectral efficiency for MCS Table 1
 *
 * Spectral efficiency = Q_m * R for each MCS index.
 * 29 MCS values (0-28) as per TS 38.214 Table 5.1.3.1-1
 */
const std::vector<double> NrMcsTables::m_spectralEfficiencyForMcs1 = {
    // QPSK (Q_m = 2): MCS 0-9
    0.23,
    0.31,
    0.38,
    0.49,
    0.60,
    0.74,
    0.88,
    1.03,
    1.18,
    1.33,
    // 16QAM (Q_m = 4): MCS 10-16
    1.33,
    1.48,
    1.70,
    1.91,
    2.16,
    2.41,
    2.57,
    // 64QAM (Q_m = 6): MCS 17-28
    2.57,
    2.73,
    3.03,
    3.32,
    3.61,
    3.90,
    4.21,
    4.52,
    4.82,
    5.12,
    5.33,
    5.55};

/**
 * @brief Table of spectral efficiency for CQI Table 1
 *
 * 16 CQI values (0-15) as per TS 38.214 Table 5.2.2.1-2
 */
const std::vector<double> NrMcsTables::m_spectralEfficiencyForCqi1 = {
    0.0,  // CQI 0: out of range
    0.15, // CQI 1
    0.23, // CQI 2
    0.38, // CQI 3
    0.60, // CQI 4
    0.88, // CQI 5
    1.18, // CQI 6
    1.48, // CQI 7
    1.91, // CQI 8
    2.41, // CQI 9
    2.73, // CQI 10
    3.32, // CQI 11
    3.90, // CQI 12
    4.52, // CQI 13
    5.12, // CQI 14
    5.55  // CQI 15
};

/**
 * @brief Table of modulation order (Q_m) for MCS Table 2
 *
 * 28 MCS values (0-27) as per TS 38.214 Table 5.1.3.1-2
 */
const std::vector<uint8_t> NrMcsTables::m_mcsMTable2 = {
    // QPSK (Q_m = 2): MCS 0-4
    2,
    2,
    2,
    2,
    2,
    // 16QAM (Q_m = 4): MCS 5-10
    4,
    4,
    4,
    4,
    4,
    4,
    // 64QAM (Q_m = 6): MCS 11-19
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    6,
    // 256QAM (Q_m = 8): MCS 20-27
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8};

/**
 * @brief Table of Effective Code Rate (ECR) for MCS Table 2
 *
 * Code rates from TS 38.214 Table 5.1.3.1-2.
 */
const std::vector<double> NrMcsTables::m_mcsEcrTable2 = {
    // QPSK (Q_m = 2): MCS 0-4
    120.0 / 1024.0, // MCS 0
    193.0 / 1024.0, // MCS 1
    308.0 / 1024.0, // MCS 2
    449.0 / 1024.0, // MCS 3
    602.0 / 1024.0, // MCS 4
    // 16QAM (Q_m = 4): MCS 5-10
    378.0 / 1024.0, // MCS 5
    434.0 / 1024.0, // MCS 6
    490.0 / 1024.0, // MCS 7
    553.0 / 1024.0, // MCS 8
    616.0 / 1024.0, // MCS 9
    658.0 / 1024.0, // MCS 10
    // 64QAM (Q_m = 6): MCS 11-19
    466.0 / 1024.0, // MCS 11
    517.0 / 1024.0, // MCS 12
    567.0 / 1024.0, // MCS 13
    616.0 / 1024.0, // MCS 14
    666.0 / 1024.0, // MCS 15
    719.0 / 1024.0, // MCS 16
    772.0 / 1024.0, // MCS 17
    822.0 / 1024.0, // MCS 18
    873.0 / 1024.0, // MCS 19
    // 256QAM (Q_m = 8): MCS 20-27
    682.5 / 1024.0, // MCS 20
    711.0 / 1024.0, // MCS 21
    754.0 / 1024.0, // MCS 22
    797.0 / 1024.0, // MCS 23
    841.0 / 1024.0, // MCS 24
    885.0 / 1024.0, // MCS 25
    916.5 / 1024.0, // MCS 26
    948.0 / 1024.0  // MCS 27
};

/**
 * @brief Table of spectral efficiency for MCS Table 2
 *
 * Spectral efficiency = Q_m * R for each MCS index.
 * 28 MCS values (0-27) as per TS 38.214 Table 5.1.3.1-2
 */
const std::vector<double> NrMcsTables::m_spectralEfficiencyForMcs2 = {
    // QPSK (Q_m = 2): MCS 0-4
    0.23,
    0.38,
    0.60,
    0.88,
    1.18,
    // 16QAM (Q_m = 4): MCS 5-10
    1.48,
    1.70,
    1.91,
    2.16,
    2.41,
    2.57,
    // 64QAM (Q_m = 6): MCS 11-19
    2.73,
    3.03,
    3.32,
    3.61,
    3.90,
    4.21,
    4.52,
    4.82,
    5.12,
    // 256QAM (Q_m = 8): MCS 20-27
    5.33,
    5.55,
    5.89,
    6.23,
    6.57,
    6.91,
    7.16,
    7.41};

/**
 * @brief Table of spectral efficiency for CQI Table 2
 *
 * 16 CQI values (0-15) as per TS 38.214 Table 5.2.2.1-3
 */
const std::vector<double> NrMcsTables::m_spectralEfficiencyForCqi2 = {
    0.0,  // CQI 0: out of range
    0.15, // CQI 1
    0.38, // CQI 2
    0.88, // CQI 3
    1.48, // CQI 4
    1.91, // CQI 5
    2.41, // CQI 6
    2.73, // CQI 7
    3.32, // CQI 8
    3.90, // CQI 9
    4.52, // CQI 10
    5.12, // CQI 11
    5.55, // CQI 12
    6.23, // CQI 13
    6.91, // CQI 14
    7.41  // CQI 15
};

uint8_t
NrMcsTables::GetModulationOrder(uint8_t mcs, uint8_t table)
{
    const auto& mcsMTable = GetMcsMTableRef(table);
    NS_ABORT_MSG_IF(mcs >= mcsMTable.size(),
                    "MCS " << +mcs << " out of range for Table " << +table);
    return mcsMTable[mcs];
}

double
NrMcsTables::GetCodeRate(uint8_t mcs, uint8_t table)
{
    const auto& ecrTable = GetMcsEcrTableRef(table);
    NS_ABORT_MSG_IF(mcs >= ecrTable.size(), "MCS " << +mcs << " out of range for Table " << +table);
    return ecrTable[mcs];
}

double
NrMcsTables::GetSpectralEfficiencyForMcs(uint8_t mcs, uint8_t table)
{
    const auto& seTable = GetSpectralEfficiencyForMcsRef(table);
    NS_ABORT_MSG_IF(mcs >= seTable.size(), "MCS " << +mcs << " out of range for Table " << +table);
    return seTable[mcs];
}

double
NrMcsTables::GetSpectralEfficiencyForCqi(uint8_t cqi, uint8_t table)
{
    const auto& seTable = GetSpectralEfficiencyForCqiRef(table);
    NS_ABORT_MSG_IF(cqi >= seTable.size(), "CQI " << +cqi << " out of range for Table " << +table);
    return seTable[cqi];
}

uint8_t
NrMcsTables::GetMaxMcs(uint8_t table)
{
    const auto& mcsMTable = GetMcsMTableRef(table);
    return static_cast<uint8_t>(mcsMTable.size() - 1);
}

const std::vector<uint8_t>&
NrMcsTables::GetMcsMTableRef(uint8_t table)
{
    NS_ABORT_MSG_IF(table != 1 && table != 2, "Invalid MCS table: " << +table);
    return (table == 1) ? m_mcsMTable1 : m_mcsMTable2;
}

const std::vector<double>&
NrMcsTables::GetMcsEcrTableRef(uint8_t table)
{
    NS_ABORT_MSG_IF(table != 1 && table != 2, "Invalid MCS table: " << +table);
    return (table == 1) ? m_mcsEcrTable1 : m_mcsEcrTable2;
}

const std::vector<double>&
NrMcsTables::GetSpectralEfficiencyForMcsRef(uint8_t table)
{
    NS_ABORT_MSG_IF(table != 1 && table != 2, "Invalid MCS table: " << +table);
    return (table == 1) ? m_spectralEfficiencyForMcs1 : m_spectralEfficiencyForMcs2;
}

const std::vector<double>&
NrMcsTables::GetSpectralEfficiencyForCqiRef(uint8_t table)
{
    NS_ABORT_MSG_IF(table != 1 && table != 2, "Invalid CQI table: " << +table);
    return (table == 1) ? m_spectralEfficiencyForCqi1 : m_spectralEfficiencyForCqi2;
}

} // namespace ns3
