/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-sl-phy-mac-common.h"

namespace ns3
{

bool
NrSlSlotAlloc::operator<(const NrSlSlotAlloc& rhs) const
{
    return (sfn < rhs.sfn);
}

bool
NrSlVarTtiAllocInfo::operator<(const NrSlVarTtiAllocInfo& rhs) const
{
    return (symStart < rhs.symStart);
}

} // namespace ns3
