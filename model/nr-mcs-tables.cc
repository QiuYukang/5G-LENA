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
 * Code rates R * 1024 from TS 38.214 Table 5.1.3.1-1, stored as double values
 * to two decimal places.
 * For example, MCS 0 has R * 1024 = 120, so ECR = 120/1024 is rounded to 0.12
 */
const std::vector<double> NrMcsTables::m_mcsEcrTable1 = {
    // QPSK (Q_m = 2): MCS 0-9
    0.12, // MCS 0: 120/1024
    0.15, // MCS 1: 157/1024
    0.19, // MCS 2: 193/1024
    0.25, // MCS 3: 251/1024
    0.30, // MCS 4: 308/1024
    0.37, // MCS 5: 379/1024
    0.44, // MCS 6: 449/1024
    0.51, // MCS 7: 526/1024
    0.59, // MCS 8: 602/1024
    0.66, // MCS 9: 679/1024
    // 16QAM (Q_m = 4): MCS 10-16
    0.33, // MCS 10: 340/1024
    0.37, // MCS 11: 378/1024
    0.42, // MCS 12: 434/1024
    0.48, // MCS 13: 490/1024
    0.54, // MCS 14: 553/1024
    0.60, // MCS 15: 616/1024
    0.64, // MCS 16: 658/1024
    // 64QAM (Q_m = 6): MCS 17-28
    0.43, // MCS 17: 438/1024
    0.46, // MCS 18: 466/1024
    0.50, // MCS 19: 517/1024
    0.55, // MCS 20: 567/1024
    0.60, // MCS 21: 616/1024
    0.65, // MCS 22: 666/1024
    0.70, // MCS 23: 719/1024
    0.75, // MCS 24: 772/1024
    0.80, // MCS 25: 822/1024
    0.85, // MCS 26: 873/1024
    0.89, // MCS 27: 910/1024
    0.93  // MCS 28: 948/1024
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
 * Code rates from TS 38.214 Table 5.1.3.1-2, stored as double values
 * to two decimal places.
 */
const std::vector<double> NrMcsTables::m_mcsEcrTable2 = {
    // QPSK (Q_m = 2): MCS 0-4
    0.12, // MCS 0
    0.19, // MCS 1
    0.30, // MCS 2
    0.44, // MCS 3
    0.59, // MCS 4
    // 16QAM (Q_m = 4): MCS 5-10
    0.37, // MCS 5
    0.42, // MCS 6
    0.48, // MCS 7
    0.54, // MCS 8
    0.60, // MCS 9
    0.64, // MCS 10
    // 64QAM (Q_m = 6): MCS 11-19
    0.46, // MCS 11
    0.50, // MCS 12
    0.55, // MCS 13
    0.60, // MCS 14
    0.65, // MCS 15
    0.70, // MCS 16
    0.75, // MCS 17
    0.80, // MCS 18
    0.85, // MCS 19
    // 256QAM (Q_m = 8): MCS 20-27
    0.67, // MCS 20
    0.69, // MCS 21
    0.74, // MCS 22
    0.77, // MCS 23
    0.82, // MCS 24
    0.86, // MCS 25
    0.90, // MCS 26
    0.93  // MCS 27
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
