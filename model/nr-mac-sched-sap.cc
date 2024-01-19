/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mac-sched-sap.h"

namespace ns3
{

std::ostream&
operator<<(std::ostream& os, const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters& p)
{
    os << "RNTI: " << p.m_rnti << " LCId: " << static_cast<uint32_t>(p.m_logicalChannelIdentity)
       << " RLCTxQueueSize: " << p.m_rlcTransmissionQueueSize
       << " B, RLCTXHolDel: " << p.m_rlcTransmissionQueueHolDelay
       << " ms, RLCReTXQueueSize: " << p.m_rlcRetransmissionQueueSize
       << " B, RLCReTXHolDel: " << p.m_rlcRetransmissionHolDelay
       << " ms, RLCStatusPduSize: " << p.m_rlcStatusPduSize << " B.";
    return os;
}

NrMacSchedSapUser::~NrMacSchedSapUser()
{
}

} // namespace ns3
