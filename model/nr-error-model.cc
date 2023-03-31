/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-error-model.h"

#include <ns3/log.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrErrorModel");
NS_OBJECT_ENSURE_REGISTERED(NrErrorModel);

NrErrorModel::NrErrorModel()
    : Object()
{
    NS_LOG_FUNCTION(this);
}

NrErrorModel::~NrErrorModel()
{
}

TypeId
NrErrorModel::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrErrorModel").SetParent<Object>();
    return tid;
}

TypeId
NrErrorModel::GetInstanceTypeId() const
{
    return NrErrorModel::GetTypeId();
}

} // namespace ns3
