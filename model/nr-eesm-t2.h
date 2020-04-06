/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#ifndef NR_EESM_T2_H
#define NR_EESM_T2_H

#include <vector>
#include "nr-eesm-error-model.h"

namespace ns3 {

/**
 * \ingroup error-models
 * \brief The NrEesmT2 struct
 *
 * NR MCS/CQI Table2 (refers to tables 5.1.3.1-2 and 5.2.2.1-3 in TS38.214)
 *
 * Values used inside NrEesmIrT2 and NrEesmCcT2 classes
 *
 * \see NrEesmIrT2
 * \see NrEesmCcT2
 */
struct NrEesmT2
{
  /**
   * \brief NrEesmT2 constructor. Initialize the pointers
   */
  NrEesmT2 ();

  const std::vector<double> *m_betaTable {nullptr};  //!< Beta table
  const std::vector<double> *m_mcsEcrTable {nullptr}; //!< MCS-ECR table
  const NrEesmErrorModel::SimulatedBlerFromSINR *m_simulatedBlerFromSINR {nullptr}; //!< BLER from SINR table
  const std::vector<uint8_t> *m_mcsMTable {nullptr}; //!< MCS-M table
  const std::vector<double> *m_spectralEfficiencyForMcs {nullptr}; //!< Spectral-efficiency for MCS
  const std::vector<double> *m_spectralEfficiencyForCqi {nullptr}; //!< Spectral-efficiency for CQI
};

} // namespace ns3

#endif // NR_EESM_T2_H
