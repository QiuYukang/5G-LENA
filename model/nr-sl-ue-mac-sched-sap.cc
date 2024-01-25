/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-sl-ue-mac-sched-sap.h"

namespace ns3
{

std::ostream&
operator<<(std::ostream& os,
           const NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& p)
{
    os << "RNTI: " << p.rnti << " LCId: " << static_cast<uint32_t>(p.lcid)
       << " RLCTxQueueSize: " << p.txQueueSize << " B, RLCTXHolDel: " << p.txQueueHolDelay
       << " ms, RLCReTXQueueSize: " << p.retxQueueSize
       << " B, RLCReTXHolDel: " << p.retxQueueHolDelay
       << " ms, RLCStatusPduSize: " << p.statusPduSize << " B, source layer 2 id: " << p.srcL2Id
       << ", destination layer 2 id " << p.dstL2Id;
    return os;
}

bool
NrSlUeMacSchedSapProvider::NrSlSlotInfo::operator<(const NrSlSlotInfo& rhs) const
{
    return sfn < rhs.sfn;
}

} // namespace ns3
