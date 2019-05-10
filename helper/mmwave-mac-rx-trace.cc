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

#include <ns3/log.h>
#include "mmwave-mac-rx-trace.h"
#include <ns3/simulator.h>
#include <stdio.h>
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmwaveMacRxTrace");

NS_OBJECT_ENSURE_REGISTERED (MmwaveMacRxTrace);

std::ofstream MmwaveMacRxTrace::m_rxedEnbMacCtrlMsgsFile;
std::string MmwaveMacRxTrace::m_rxedEnbMacCtrlMsgsFileName;
std::ofstream MmwaveMacRxTrace::m_txedEnbMacCtrlMsgsFile;
std::string MmwaveMacRxTrace::m_txedEnbMacCtrlMsgsFileName;

std::ofstream MmwaveMacRxTrace::m_rxedUeMacCtrlMsgsFile;
std::string MmwaveMacRxTrace::m_rxedUeMacCtrlMsgsFileName;
std::ofstream MmwaveMacRxTrace::m_txedUeMacCtrlMsgsFile;
std::string MmwaveMacRxTrace::m_txedUeMacCtrlMsgsFileName;

MmwaveMacRxTrace::MmwaveMacRxTrace ()
{
}

MmwaveMacRxTrace::~MmwaveMacRxTrace ()
{
  if (m_rxedEnbMacCtrlMsgsFile.is_open ())
    {
      m_rxedEnbMacCtrlMsgsFile.close ();
    }

  if (m_txedEnbMacCtrlMsgsFile.is_open ())
    {
      m_txedEnbMacCtrlMsgsFile.close ();
    }

  if (m_rxedUeMacCtrlMsgsFile.is_open ())
    {
      m_rxedUeMacCtrlMsgsFile.close ();
    }

  if (m_txedUeMacCtrlMsgsFile.is_open ())
    {
      m_txedUeMacCtrlMsgsFile.close ();
    }
}

TypeId
MmwaveMacRxTrace::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmwaveMacRxTrace")
    .SetParent<Object> ()
    .AddConstructor<MmwaveMacRxTrace> ()
  ;
  return tid;
}

void
MmwaveMacRxTrace::RxedEnbMacCtrlMsgsCallback (Ptr<MmwaveMacRxTrace> macStats, std::string path, SfnSf sfn,
                                              uint16_t rnti, uint8_t ccId, Ptr<MmWaveControlMessage> msg)
{
  if (!m_rxedEnbMacCtrlMsgsFile.is_open ())
      {
        m_rxedEnbMacCtrlMsgsFileName = "RxedEnbMacCtrlMsgsTrace.txt";
        m_rxedEnbMacCtrlMsgsFile.open (m_rxedEnbMacCtrlMsgsFileName.c_str ());
        m_rxedEnbMacCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "MsgType" << std::endl;

        if (!m_rxedEnbMacCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedEnbMacCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "ENB MAC Rxed"  << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t";

  if (msg->GetMessageType () == MmWaveControlMessage::SR)
    {
      m_rxedEnbMacCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::DL_CQI)
    {
      m_rxedEnbMacCtrlMsgsFile << "DL_CQI";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::BSR)
    {
      m_rxedEnbMacCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::DL_HARQ)
    {
      m_rxedEnbMacCtrlMsgsFile << "DL_HARQ";
    }
  else
    {
      m_rxedEnbMacCtrlMsgsFile << "Other";
    }
  m_rxedEnbMacCtrlMsgsFile << std::endl;
}

void
MmwaveMacRxTrace::TxedEnbMacCtrlMsgsCallback (Ptr<MmwaveMacRxTrace> macStats, std::string path, SfnSf sfn,
                                              uint16_t rnti, uint8_t ccId, Ptr<MmWaveControlMessage> msg)
{
  if (!m_txedEnbMacCtrlMsgsFile.is_open ())
      {
        m_txedEnbMacCtrlMsgsFileName = "TxedEnbMacCtrlMsgsTrace.txt";
        m_txedEnbMacCtrlMsgsFile.open (m_txedEnbMacCtrlMsgsFileName.c_str ());
        m_txedEnbMacCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "MsgType" << std::endl;

        if (!m_txedEnbMacCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedEnbMacCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "ENB MAC Txed"  << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t";

  if (msg->GetMessageType () == MmWaveControlMessage::RAR)
    {
      m_txedEnbMacCtrlMsgsFile << "RAR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::DL_CQI)
    {
      m_txedEnbMacCtrlMsgsFile << "DL_CQI";
    }
  else
    {
      m_txedEnbMacCtrlMsgsFile << "Other";
    }

  m_txedEnbMacCtrlMsgsFile << std::endl;
}

void
MmwaveMacRxTrace::RxedUeMacCtrlMsgsCallback (Ptr<MmwaveMacRxTrace> macStats, std::string path, SfnSf sfn,
                                             uint16_t rnti, uint8_t ccId, Ptr<MmWaveControlMessage> msg)
{
  if (!m_rxedUeMacCtrlMsgsFile.is_open ())
      {
        m_rxedUeMacCtrlMsgsFileName = "RxedUeMacCtrlMsgsTrace.txt";
        m_rxedUeMacCtrlMsgsFile.open (m_rxedUeMacCtrlMsgsFileName.c_str ());
        m_rxedUeMacCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "MsgType" << std::endl;

        if (!m_rxedUeMacCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUeMacCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "UE  MAC Rxed" << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t";

  if (msg->GetMessageType () == MmWaveControlMessage::DCI_TDMA)
    {
      m_rxedUeMacCtrlMsgsFile << "DCI_TDMA";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::RAR)
    {
      m_rxedUeMacCtrlMsgsFile << "RAR";
    }
  else
    {
      m_rxedUeMacCtrlMsgsFile << "Other";
    }
  m_rxedUeMacCtrlMsgsFile << std::endl;
}

void
MmwaveMacRxTrace::TxedUeMacCtrlMsgsCallback (Ptr<MmwaveMacRxTrace> macStats, std::string path, SfnSf sfn,
                                             uint16_t rnti, uint8_t ccId, Ptr<MmWaveControlMessage> msg)
{
  if (!m_txedUeMacCtrlMsgsFile.is_open ())
      {
        m_txedUeMacCtrlMsgsFileName = "TxedUeMacCtrlMsgsTrace.txt";
        m_txedUeMacCtrlMsgsFile.open (m_txedUeMacCtrlMsgsFileName.c_str ());
        m_txedUeMacCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "MsgType" << std::endl;

        if (!m_txedUeMacCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedUeMacCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "UE  MAC Txed" << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t";

  if (msg->GetMessageType () == MmWaveControlMessage::BSR)
    {
      m_txedUeMacCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::SR)
    {
      m_txedUeMacCtrlMsgsFile << "SR";
    }
  else
    {
      m_txedUeMacCtrlMsgsFile << "Other";
    }
  m_txedUeMacCtrlMsgsFile << std::endl;
}

} /* namespace ns3 */
