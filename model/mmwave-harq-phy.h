/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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


#ifndef MMWAVE_HARQ_PHY_MODULE_H
#define MMWAVE_HARQ_PHY_MODULE_H

#include <vector>
#include <unordered_map>
#include <ns3/simple-ref-count.h>
#include "nr-error-model.h"

namespace ns3 {

/**
 * \ingroup error-models
 * \brief The MmWaveHarqPhy class implements the HARQ functionalities related to PHY layer
 *(i.e., decodification buffers for incremental redundancy managment)
 *
*/
class MmWaveHarqPhy : public SimpleRefCount<MmWaveHarqPhy>
{
public:
  MmWaveHarqPhy (uint32_t harqNum);
  ~MmWaveHarqPhy ();

  /**
  * \brief Return the info of the HARQ procId in case of retranmissions
  * for DL (asynchronous)
  * \param harqProcId the HARQ proc id
  * \return the vector of the info related to HARQ proc Id
  */
  const NrErrorModel::NrErrorModelHistory & GetHarqProcessInfoDl (uint16_t rnti, uint8_t harqProcId);

  /**
  * \brief Return the info of the HARQ procId in case of retranmissions
  * for UL (asynchronous)
  * \param rnti the RNTI of the transmitter
  * \param harqProcId the HARQ proc id
  * \return the vector of the info related to HARQ proc Id
  */
  const NrErrorModel::NrErrorModelHistory & GetHarqProcessInfoUl (uint16_t rnti, uint8_t harqProcId);

  /**
  * \brief Update the Info associated to the decodification of an HARQ process
  * for DL (asynchronous)
  * \param rnti the RNTI of the transmitter
  * \param harqProcId the HARQ proc id
  * \param output output of the error model
  */
  void UpdateDlHarqProcessStatus (uint16_t rnti, uint8_t harqProcId,
                                  const Ptr<NrErrorModelOutput> &output);

  /**
  * \brief Reset  the info associated to the decodification of an HARQ process
  * for DL (asynchronous)
  * \param id the HARQ proc id
  */
  void ResetDlHarqProcessStatus (uint16_t rnti, uint8_t id);

  /**
  * \brief Update the MI value associated to the decodification of an HARQ process
  * for DL (asynchronous)
  * \param rnti the RNTI of the transmitter
  * \param harqProcId the HARQ proc id
  * \param output output of the error model
  */
  void UpdateUlHarqProcessStatus (uint16_t rnti, uint8_t harqProcId,
                                  const Ptr<NrErrorModelOutput> &output);

  /**
  * \brief Reset  the info associated to the decodification of an HARQ process
  * for DL (asynchronous)
  * \param id the HARQ proc id
  */
  void ResetUlHarqProcessStatus (uint16_t rnti, uint8_t id);

private:
  uint32_t m_harqNum;
  std::unordered_map <uint16_t, std::vector<NrErrorModel::NrErrorModelHistory> > m_dlHistory;
  std::unordered_map <uint16_t, std::vector<NrErrorModel::NrErrorModelHistory> > m_ulHistory;
};


}

#endif /* MMWAVE_HARQ_PHY_MODULE_H */
