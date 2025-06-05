// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "wraparound-three-gpp-spectrum-propagation-loss-model.h"

#include "ns3/constant-position-mobility-model.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/double.h"
#include "ns3/hexagonal-wraparound-model.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/spectrum-signal-parameters.h"
#include "ns3/string.h"
#include "ns3/three-gpp-propagation-loss-model.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("WraparoundThreeGppSpectrumPropagationLossModel");
NS_OBJECT_ENSURE_REGISTERED(WraparoundThreeGppSpectrumPropagationLossModel);

Ptr<MobilityModel>
GetWraparoundMobilityModel(Ptr<const MobilityModel> b, Ptr<const MobilityModel> a)
{
    Ptr<MobilityModel> c;
    if (ConstantPositionMobilityModel::GetTypeId() == a->GetInstanceTypeId())
    {
        c = Copy(StaticCast<const ConstantPositionMobilityModel>(a));
    }
    else if (ConstantVelocityMobilityModel::GetTypeId() == a->GetInstanceTypeId())
    {
        c = Copy(StaticCast<const ConstantVelocityMobilityModel>(a));
    }
    else
    {
        NS_ABORT_MSG("Unsupported mobility model");
    }
    // Set NodeId for models that retrieve that later
    c->UnidirectionalAggregateObject(a->GetObject<Node>());

    auto wraparound = a->GetObject<HexagonalWraparoundModel>();
    if (wraparound)
    {
        auto aPos = a->GetPosition();
        auto bPos = b->GetPosition();
        c->SetPosition(wraparound->GetRelativeVirtualPosition(bPos, aPos));
    }
    return c;
}

WraparoundThreeGppSpectrumPropagationLossModel::WraparoundThreeGppSpectrumPropagationLossModel()
{
    NS_LOG_FUNCTION(this);
    ThreeGppPropagationLossModel::InstallDoCalcRxPowerPrologueFunction(GetWraparoundMobilityModel);
}

WraparoundThreeGppSpectrumPropagationLossModel::~WraparoundThreeGppSpectrumPropagationLossModel()
{
    NS_LOG_FUNCTION(this);
}

TypeId
WraparoundThreeGppSpectrumPropagationLossModel::GetTypeId()
{
    static TypeId tid = TypeId("ns3::WraparoundThreeGppSpectrumPropagationLossModel")
                            .SetParent<DistanceBasedThreeGppSpectrumPropagationLossModel>()
                            .SetGroupName("Spectrum")
                            .AddConstructor<WraparoundThreeGppSpectrumPropagationLossModel>();
    return tid;
}

Ptr<SpectrumSignalParameters>
WraparoundThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity(
    Ptr<const SpectrumSignalParameters> params,
    Ptr<const MobilityModel> a,
    Ptr<const MobilityModel> b,
    Ptr<const PhasedArrayModel> aPhasedArrayModel,
    Ptr<const PhasedArrayModel> bPhasedArrayModel) const
{
    NS_LOG_FUNCTION(this);

    return DistanceBasedThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity(
        params,
        GetWraparoundMobilityModel(b, a),
        b,
        aPhasedArrayModel,
        bPhasedArrayModel);
}

} // namespace ns3
