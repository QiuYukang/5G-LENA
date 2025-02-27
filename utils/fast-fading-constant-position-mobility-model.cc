// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "fast-fading-constant-position-mobility-model.h"

using namespace ns3;
NS_OBJECT_ENSURE_REGISTERED(FastFadingConstantPositionMobilityModel);

TypeId
FastFadingConstantPositionMobilityModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::FastFadingConstantPositionMobilityModel")
            .SetParent<ConstantPositionMobilityModel>()
            .SetGroupName("Mobility")
            .AddConstructor<FastFadingConstantPositionMobilityModel>()
            .AddAttribute(
                "FakeVelocity",
                "The current velocity of the mobility model.",
                VectorValue(Vector(0.0, 0.0, 0.0)), // ignored initial value.
                MakeVectorAccessor(&FastFadingConstantPositionMobilityModel::m_fakeVelocity),
                MakeVectorChecker());
    return tid;
}

Vector
FastFadingConstantPositionMobilityModel::DoGetVelocity() const
{
    return m_fakeVelocity;
}
