// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_WRAPAROUND_UTILS_H
#define NR_WRAPAROUND_UTILS_H

#include "ns3/mobility-model.h"
#include "ns3/spectrum-channel.h"
#include "ns3/wraparound-model.h"

namespace ns3
{
Ptr<MobilityModel> GetVirtualMobilityModel(Ptr<SpectrumChannel> channel,
                                           Ptr<MobilityModel> tx,
                                           Ptr<MobilityModel> rx);
}
#endif // NR_WRAPAROUND_UTILS_H
