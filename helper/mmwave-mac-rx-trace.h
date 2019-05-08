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
*
*/

#ifndef SRC_MMWAVE_HELPER_MMWAVE_MAC_RX_TRACE_H_
#define SRC_MMWAVE_HELPER_MMWAVE_MAC_RX_TRACE_H_

#include <ns3/object.h>
#include <ns3/spectrum-value.h>
#include <ns3/mmwave-phy-mac-common.h>
#include <ns3/mmwave-control-messages.h>
#include <ns3/mmwave-enb-mac.h>
#include <iostream>

namespace ns3 {

class MmwaveMacRxTrace : public Object
{
public:
  MmwaveMacRxTrace ();
  virtual ~MmwaveMacRxTrace ();
  static TypeId GetTypeId (void);

  /**
   *  Trace sink for Enb Mac Received Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] pointer to msg to get the msg type
   */
  static void RxedEnbMacCtrlMsgsCallback (Ptr<MmwaveMacRxTrace> macStats, std::string path, SfnSf sfn,
                                          uint16_t rnti, Ptr<MmWaveControlMessage> msg);

  /**
   *  Trace sink for Enb Mac Transmitted Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] pointer to msg to get the msg type
   */
  static void TxedEnbMacCtrlMsgsCallback (Ptr<MmwaveMacRxTrace> macStats, std::string path, SfnSf sfn,
                                          uint16_t rnti, Ptr<MmWaveControlMessage> msg);

  /**
   *  Trace sink for Ue Mac Received Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] pointer to msg to get the msg type
   */
  static void RxedUeMacCtrlMsgsCallback (Ptr<MmwaveMacRxTrace> macStats, std::string path, SfnSf sfn,
                                         uint16_t rnti, Ptr<MmWaveControlMessage> msg);

  /**
   *  Trace sink for Ue Mac Transmitted Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] rnti
   * \param [in] pointer to msg to get the msg type
   */
  static void TxedUeMacCtrlMsgsCallback (Ptr<MmwaveMacRxTrace> macStats, std::string path, SfnSf sfn,
                                         uint16_t rnti, Ptr<MmWaveControlMessage> msg);

private:

  static std::ofstream m_rxedEnbMacCtrlMsgsFile;
  static std::string m_rxedEnbMacCtrlMsgsFileName;
  static std::ofstream m_txedEnbMacCtrlMsgsFile;
  static std::string m_txedEnbMacCtrlMsgsFileName;

  static std::ofstream m_rxedUeMacCtrlMsgsFile;
  static std::string m_rxedUeMacCtrlMsgsFileName;
  static std::ofstream m_txedUeMacCtrlMsgsFile;
  static std::string m_txedUeMacCtrlMsgsFileName;
};

} /* namespace ns3 */

#endif /* SRC_MMWAVE_HELPER_MMWAVE_MAC_RX_TRACE_H_ */
