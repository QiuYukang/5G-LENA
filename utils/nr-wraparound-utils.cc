// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-wraparound-utils.h"

namespace ns3
{
Ptr<MobilityModel>
GetVirtualMobilityModel(Ptr<SpectrumChannel> channel, Ptr<MobilityModel> tx, Ptr<MobilityModel> rx)
{
    // Ensures channel wraparound is applied to txMobilityModel.
    // This is required, since we do not send SSBs for our RSRP based attachment nor ideal
    // beamforming. SSBs are wrapped automatically by the spectrum channel from ns-3.46 and onwards,
    // if a wraparound mobility model is set.
    auto wraparound = channel->GetObject<WraparoundModel>();
    return wraparound ? wraparound->GetVirtualMobilityModel(tx, rx) : tx;
}
} // namespace ns3
