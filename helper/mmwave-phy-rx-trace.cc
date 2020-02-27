/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *                        Sourjya Dutta <sdutta@nyu.edu>
 *                        Russell Ford <russell.ford@nyu.edu>
 *                        Menglei Zhang <menglei@nyu.edu>
 */



#include <ns3/log.h>
#include "mmwave-phy-rx-trace.h"
#include <ns3/simulator.h>
#include <stdio.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWavePhyRxTrace");

NS_OBJECT_ENSURE_REGISTERED (MmWavePhyRxTrace);

std::ofstream MmWavePhyRxTrace::m_rxPacketTraceFile;
std::string MmWavePhyRxTrace::m_rxPacketTraceFilename;

std::ofstream MmWavePhyRxTrace::m_rxedEnbPhyCtrlMsgsFile;
std::string MmWavePhyRxTrace::m_rxedEnbPhyCtrlMsgsFileName;
std::ofstream MmWavePhyRxTrace::m_txedEnbPhyCtrlMsgsFile;
std::string MmWavePhyRxTrace::m_txedEnbPhyCtrlMsgsFileName;

std::ofstream MmWavePhyRxTrace::m_rxedUePhyCtrlMsgsFile;
std::string MmWavePhyRxTrace::m_rxedUePhyCtrlMsgsFileName;
std::ofstream MmWavePhyRxTrace::m_txedUePhyCtrlMsgsFile;
std::string MmWavePhyRxTrace::m_txedUePhyCtrlMsgsFileName;
std::ofstream MmWavePhyRxTrace::m_rxedUePhyDlDciFile;
std::string MmWavePhyRxTrace::m_rxedUePhyDlDciFileName;

MmWavePhyRxTrace::MmWavePhyRxTrace ()
{
}

MmWavePhyRxTrace::~MmWavePhyRxTrace ()
{
  if (m_rxPacketTraceFile.is_open ())
    {
      m_rxPacketTraceFile.close ();
    }

  if (m_rxedEnbPhyCtrlMsgsFile.is_open ())
    {
      m_rxedEnbPhyCtrlMsgsFile.close ();
    }

  if (m_txedEnbPhyCtrlMsgsFile.is_open ())
    {
      m_txedEnbPhyCtrlMsgsFile.close ();
    }

  if (m_rxedUePhyCtrlMsgsFile.is_open ())
    {
      m_rxedUePhyCtrlMsgsFile.close ();
    }

  if (m_txedUePhyCtrlMsgsFile.is_open ())
    {
      m_txedUePhyCtrlMsgsFile.close ();
    }

  if (m_rxedUePhyDlDciFile.is_open ())
    {
      m_rxedUePhyDlDciFile.close ();
    }
}

TypeId
MmWavePhyRxTrace::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWavePhyRxTrace")
    .SetParent<Object> ()
    .AddConstructor<MmWavePhyRxTrace> ()
  ;
  return tid;
}

void
MmWavePhyRxTrace::ReportCurrentCellRsrpSinrCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path,
                                                     uint64_t imsi, SpectrumValue& sinr, SpectrumValue& power)
{
  NS_LOG_INFO ("UE" << imsi << "->Generate RsrpSinrTrace");
  phyStats->ReportInterferenceTrace (imsi, sinr);
  //phyStats->ReportPowerTrace (imsi, power);
}

void
MmWavePhyRxTrace::UlSinrTraceCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path,
                                       uint64_t imsi, SpectrumValue& sinr, SpectrumValue& power)
{
  NS_LOG_INFO ("UE" << imsi << "->Generate UlSinrTrace");
  uint64_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  sprintf (fname, "UE_%llu_UL_SINR_dB.txt", (long long unsigned ) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = sinr.ValuesBegin ();
  while (it != sinr.ValuesEnd ())
    {
      //fprintf(log_file, "%d\t%d\t%f\t \n", tti_count/2, rb_count, 10*log10(*it));
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned )tti_count / 8 + 1, (long long unsigned )tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
  //phyStats->ReportInterferenceTrace (imsi, sinr);
  //phyStats->ReportPowerTrace (imsi, power);
}

void
MmWavePhyRxTrace::RxedEnbPhyCtrlMsgsCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path, SfnSf sfn,
                                              uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  if (!m_rxedEnbPhyCtrlMsgsFile.is_open ())
      {
        m_rxedEnbPhyCtrlMsgsFileName = "RxedEnbPhyCtrlMsgsTrace.txt";
        m_rxedEnbPhyCtrlMsgsFile.open (m_rxedEnbPhyCtrlMsgsFileName.c_str ());
        m_rxedEnbPhyCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "MsgType" << std::endl;

        if (!m_rxedEnbPhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedEnbPhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "ENB PHY Rxed" << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t";


  if (msg->GetMessageType () == MmWaveControlMessage::DL_CQI)
    {
      m_rxedEnbPhyCtrlMsgsFile << "DL_CQI";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::SR)
    {
      m_rxedEnbPhyCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::BSR)
    {
      m_rxedEnbPhyCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::RACH_PREAMBLE)
    {
      m_rxedEnbPhyCtrlMsgsFile << "RACH_PREAMBLE";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::DL_HARQ)
    {
      m_rxedEnbPhyCtrlMsgsFile << "DL_HARQ";
    }
  else
    {
      m_rxedEnbPhyCtrlMsgsFile << "Other";
    }
  m_rxedEnbPhyCtrlMsgsFile << std::endl;
}

void
MmWavePhyRxTrace::TxedEnbPhyCtrlMsgsCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path, SfnSf sfn,
                                              uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  if (!m_txedEnbPhyCtrlMsgsFile.is_open ())
      {
        m_txedEnbPhyCtrlMsgsFileName = "TxedEnbPhyCtrlMsgsTrace.txt";
        m_txedEnbPhyCtrlMsgsFile.open (m_txedEnbPhyCtrlMsgsFileName.c_str ());
        m_txedEnbPhyCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "MsgType" << std::endl;

        if (!m_txedEnbPhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedEnbPhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "ENB PHY Txed" << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t";

  if (msg->GetMessageType () == MmWaveControlMessage::MIB)
    {
      m_txedEnbPhyCtrlMsgsFile << "MIB";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::SIB1)
    {
      m_txedEnbPhyCtrlMsgsFile << "SIB1";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::RAR)
    {
      m_txedEnbPhyCtrlMsgsFile << "RAR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::DCI_TDMA)
    {
      m_txedEnbPhyCtrlMsgsFile << "DCI_TDMA";
    }
  else
    {
      m_txedEnbPhyCtrlMsgsFile << "Other";
    }
  m_txedEnbPhyCtrlMsgsFile << std::endl;
}

void
MmWavePhyRxTrace::RxedUePhyCtrlMsgsCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path, SfnSf sfn,
                                             uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  if (!m_rxedUePhyCtrlMsgsFile.is_open ())
      {
        m_rxedUePhyCtrlMsgsFileName = "RxedUePhyCtrlMsgsTrace.txt";
        m_rxedUePhyCtrlMsgsFile.open (m_rxedUePhyCtrlMsgsFileName.c_str ());
        m_rxedUePhyCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "MsgType" << std::endl;

        if (!m_rxedUePhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "UE  PHY Rxed" << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t";

  if (msg->GetMessageType () == MmWaveControlMessage::DCI_TDMA)
    {
      m_rxedUePhyCtrlMsgsFile << "DCI_TDMA";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::MIB)
    {
      m_rxedUePhyCtrlMsgsFile << "MIB";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::SIB1)
    {
      m_rxedUePhyCtrlMsgsFile << "SIB1";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::RAR)
    {
      m_rxedUePhyCtrlMsgsFile << "RAR";
    }
  else
    {
      m_rxedUePhyCtrlMsgsFile << "Other";
    }
  m_rxedUePhyCtrlMsgsFile << std::endl;
}

void
MmWavePhyRxTrace::TxedUePhyCtrlMsgsCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path, SfnSf sfn,
                                             uint16_t rnti, uint8_t ccId, Ptr<const MmWaveControlMessage> msg)
{
  if (!m_txedUePhyCtrlMsgsFile.is_open ())
      {
        m_txedUePhyCtrlMsgsFileName = "TxedUePhyCtrlMsgsTrace.txt";
        m_txedUePhyCtrlMsgsFile.open (m_txedUePhyCtrlMsgsFileName.c_str ());
        m_txedUePhyCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "MsgType" << std::endl;

        if (!m_txedUePhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedUePhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "UE  PHY Txed" << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t";

  if (msg->GetMessageType () == MmWaveControlMessage::RACH_PREAMBLE)
    {
      m_txedUePhyCtrlMsgsFile << "RACH_PREAMBLE";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::SR)
    {
      m_txedUePhyCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::BSR)
    {
      m_txedUePhyCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::DL_CQI)
    {
      m_txedUePhyCtrlMsgsFile << "DL_CQI";
    }
  else if (msg->GetMessageType () == MmWaveControlMessage::DL_HARQ)
    {
      m_txedUePhyCtrlMsgsFile << "DL_HARQ";
    }
  else
    {
      m_txedUePhyCtrlMsgsFile << "Other";
    }
  m_txedUePhyCtrlMsgsFile << std::endl;
}

void
MmWavePhyRxTrace::RxedUePhyDlDciCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path, SfnSf sfn,
                                             uint16_t rnti, uint8_t ccId, uint8_t harqId, uint32_t k1Delay)
{
  if (!m_rxedUePhyDlDciFile.is_open ())
      {
        m_rxedUePhyDlDciFileName = "RxedUePhyDlDciTrace.txt";
        m_rxedUePhyDlDciFile.open (m_rxedUePhyDlDciFileName.c_str ());
        m_rxedUePhyDlDciFile << "Time" << "\t" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "Harq ID" << "\t" << "K1 Delay" << std::endl;

        if (!m_rxedUePhyDlDciFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyDlDciFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "DL DCI Rxed" << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t"
          << static_cast<uint32_t> (harqId) << "\t" << k1Delay << std::endl;

}

void
MmWavePhyRxTrace::TxedUePhyHarqFeedbackCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path, SfnSf sfn,
                                             uint16_t rnti, uint8_t ccId, uint8_t harqId, uint32_t k1Delay)
{
  if (!m_rxedUePhyDlDciFile.is_open ())
      {
        m_rxedUePhyDlDciFileName = "RxedUePhyDlDciTrace.txt";
        m_rxedUePhyDlDciFile.open (m_rxedUePhyDlDciFileName.c_str ());
        m_rxedUePhyDlDciFile << "Time" << "\t" << "\t" << "Entity"  << "\t" << "\t" << "Frame" << "\t" << "SF"
                                 << "\t" << "Slot" << "\t" << "VarTTI" << "\t" << "RNTI" << "\t" << "ccId"
                                 << "\t" << "Harq ID" << "\t" << "K1 Delay" << std::endl;

        if (!m_rxedUePhyDlDciFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyDlDciFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << "HARQ FD Txed" << "\t" << sfn.m_frameNum
          << "\t" << static_cast<uint32_t> (sfn.m_subframeNum) << "\t" << static_cast<uint32_t> (sfn.m_slotNum)
          << "\t" << static_cast<uint32_t> (sfn.m_varTtiNum) << "\t" << rnti << "\t" << static_cast<uint32_t> (ccId) << "\t"
          << static_cast<uint32_t> (harqId) << "\t" << k1Delay << std::endl;

}

void
MmWavePhyRxTrace::ReportInterferenceTrace (uint64_t imsi, SpectrumValue& sinr)
{
  uint64_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  sprintf (fname, "UE_%llu_SINR_dB.txt", (long long unsigned ) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = sinr.ValuesBegin ();
  while (it != sinr.ValuesEnd ())
    {
      //fprintf(log_file, "%d\t%d\t%f\t \n", tti_count/2, rb_count, 10*log10(*it));
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned) tti_count / 8 + 1, (long long unsigned) tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
}

void
MmWavePhyRxTrace::ReportPowerTrace (uint64_t imsi, SpectrumValue& power)
{

  uint32_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  printf (fname, "UE_%llu_ReceivedPower_dB.txt", (long long unsigned) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = power.ValuesBegin ();
  while (it != power.ValuesEnd ())
    {
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned) tti_count / 8 + 1,(long long unsigned) tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
}

void
MmWavePhyRxTrace::ReportPacketCountUeCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path,
                                               UePhyPacketCountParameter param)
{
  phyStats->ReportPacketCountUe (param);
}
void
MmWavePhyRxTrace::ReportPacketCountEnbCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path,
                                                EnbPhyPacketCountParameter param)
{
  phyStats->ReportPacketCountEnb (param);
}

void
MmWavePhyRxTrace::ReportDownLinkTBSize (Ptr<MmWavePhyRxTrace> phyStats, std::string path,
                                        uint64_t imsi, uint64_t tbSize)
{
  phyStats->ReportDLTbSize (imsi, tbSize);
}



void
MmWavePhyRxTrace::ReportPacketCountUe (UePhyPacketCountParameter param)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"UE_%llu_Packet_Trace.txt", (long long unsigned) param.m_imsi);
  log_file = fopen (fname, "a");
  if (param.m_isTx)
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, param.m_noBytes, 0);
    }
  else
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, 0, param.m_noBytes);
    }

  fflush (log_file);
  fclose (log_file);

}

void
MmWavePhyRxTrace::ReportPacketCountEnb (EnbPhyPacketCountParameter param)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"BS_%llu_Packet_Trace.txt",(long long unsigned) param.m_cellId);
  log_file = fopen (fname, "a");
  if (param.m_isTx)
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, param.m_noBytes, 0);
    }
  else
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, 0, param.m_noBytes);
    }

  fflush (log_file);
  fclose (log_file);
}

void
MmWavePhyRxTrace::ReportDLTbSize (uint64_t imsi, uint64_t tbSize)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"UE_%llu_Tb_Size.txt", (long long unsigned) imsi);
  log_file = fopen (fname, "a");

  fprintf (log_file, "%llu \t %llu\n", (long long unsigned )Now ().GetMicroSeconds (), (long long unsigned )tbSize);
  fprintf (log_file, "%lld \t %llu \n",(long long int) Now ().GetMicroSeconds (), (long long unsigned) tbSize);
  fflush (log_file);
  fclose (log_file);
}

void
MmWavePhyRxTrace::RxPacketTraceUeCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path, RxPacketTraceParams params)
{
  if (!m_rxPacketTraceFile.is_open ())
    {
      m_rxPacketTraceFilename = "RxPacketTrace.txt";
      m_rxPacketTraceFile.open (m_rxPacketTraceFilename.c_str ());
      m_rxPacketTraceFile << "\tframe\tsubF\tslot\t1stSym\tsymbol#\tcellId\trnti\ttbSize\tmcs\trv\tSINR(dB)\tcorrupt\tTBler\tCcId" << std::endl;
      if (!m_rxPacketTraceFile.is_open ())
        {
          NS_FATAL_ERROR ("Could not open tracefile");
        }
    }

  m_rxPacketTraceFile << "DL\t" << params.m_frameNum
                      << "\t" << (unsigned)params.m_subframeNum
                      << "\t" << (unsigned)params.m_slotNum
                      << "\t" << (unsigned)params.m_symStart
                      << "\t" << (unsigned)params.m_numSym
                      << "\t" << params.m_cellId
                      << "\t" << params.m_rnti
                      << "\t" << params.m_tbSize
                      << "\t" << (unsigned)params.m_mcs
                      << "\t" << (unsigned)params.m_rv
                      << "\t" << 10 * log10 (params.m_sinr)
                      << "\t" << params.m_corrupt
                      << "\t" << params.m_tbler
                      << "\t" << (unsigned)params.m_ccId << std::endl;

  if (params.m_corrupt)
    {
      NS_LOG_DEBUG ("DL TB error\t" << params.m_frameNum
                                    << "\t" << (unsigned)params.m_subframeNum
                                    << "\t" << (unsigned)params.m_slotNum
                                    << "\t" << (unsigned)params.m_symStart
                                    << "\t" << (unsigned)params.m_numSym
                                    << "\t" << params.m_rnti
                                    << "\t" << params.m_tbSize <<
                    "\t" << (unsigned)params.m_mcs <<
                    "\t" << (unsigned)params.m_rv <<
                    "\t" << params.m_sinr <<
                    "\t" << params.m_tbler <<
                    "\t" << params.m_corrupt <<
                    "\t" << (unsigned)params.m_ccId);
    }
}
void
MmWavePhyRxTrace::RxPacketTraceEnbCallback (Ptr<MmWavePhyRxTrace> phyStats, std::string path, RxPacketTraceParams params)
{
  if (!m_rxPacketTraceFile.is_open ())
    {
      m_rxPacketTraceFilename = "RxPacketTrace.txt";
      m_rxPacketTraceFile.open (m_rxPacketTraceFilename.c_str ());
      m_rxPacketTraceFile << "\tframe\tsubF\tslot\t1stSym\tsymbol#\tcellId\trnti\ttbSize\tmcs\trv\tSINR(dB)\tcorrupt\tTBler\tCcId" << std::endl;

      if (!m_rxPacketTraceFile.is_open ())
        {
          NS_FATAL_ERROR ("Could not open tracefile");
        }
    }
  m_rxPacketTraceFile << "UL\t" << params.m_frameNum << "\t" << (unsigned)params.m_subframeNum
                      << "\t" << (unsigned)params.m_slotNum
                      << "\t" << (unsigned)params.m_symStart
                      << "\t" << (unsigned)params.m_numSym << "\t" << params.m_cellId
                      << "\t" << params.m_rnti << "\t" << params.m_tbSize << "\t" << (unsigned)params.m_mcs << "\t" << (unsigned)params.m_rv << "\t"
                      << 10 * log10 (params.m_sinr) << "\t\t" << params.m_corrupt << " \t" << params.m_tbler << " \t" << params.m_ccId << std::endl;

  if (params.m_corrupt)
    {
      NS_LOG_DEBUG ("UL TB error\t" << params.m_frameNum << "\t" << (unsigned)params.m_subframeNum
                                    << "\t" << (unsigned)params.m_slotNum
                                    << "\t" << (unsigned)params.m_symStart
                                    << "\t" << (unsigned)params.m_numSym
                                    << "\t" << params.m_rnti << "\t" << params.m_tbSize << "\t" << (unsigned)params.m_mcs << "\t" << (unsigned)params.m_rv << "\t"
                                    << params.m_sinr << "\t" << params.m_tbler << "\t" << params.m_corrupt << "\t" << params.m_sinrMin << " \t" << params.m_ccId);
    }
}

} /* namespace ns3 */
