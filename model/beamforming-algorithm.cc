/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "beamforming-algorithm.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("BeamformingAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(BeamformingAlgorithm);

BeamformingAlgorithm::BeamformingAlgorithm()
{
}

BeamformingAlgorithm::~BeamformingAlgorithm()
{
}

TypeId
BeamformingAlgorithm::GetTypeId()
{
    static TypeId tid = TypeId("ns3::BeamformingAlgorithm").SetParent<Object>();

    return tid;
}

} // namespace ns3
