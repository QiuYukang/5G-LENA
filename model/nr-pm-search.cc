/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-pm-search.h"

#include <numeric>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPmSearch");
NS_OBJECT_ENSURE_REGISTERED(NrPmSearch);

TypeId
NrPmSearch::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrPmSearch")
            .SetParent<Object>()
            .AddAttribute("RankLimit",
                          "Max MIMO rank is minimum of num UE ports, num gNB ports, and RankLimit",
                          UintegerValue(UINT8_MAX),
                          MakeUintegerAccessor(&NrPmSearch::m_rankLimit),
                          MakeUintegerChecker<uint8_t>());
    return tid;
}

void
NrPmSearch::SetAmc(Ptr<const NrAmc> amc)
{
    m_amc = amc;
}

void
NrPmSearch::SetGnbParams(bool isDualPol, size_t numHPorts, size_t numVPorts)
{
    m_nGnbPorts = isDualPol ? 2 * numHPorts * numVPorts : numHPorts * numVPorts;
    m_isGnbDualPol = isDualPol;
    m_nGnbHPorts = numHPorts;
    m_nGnbVPorts = numVPorts;
}

void
NrPmSearch::SetUeParams(size_t numTotalPorts)
{
    m_nRxPorts = numTotalPorts;
}

void
NrPmSearch::SetSubbandSize(size_t subbandSize)
{
    m_subbandSize = subbandSize;
}

size_t
NrPmSearch::GetSubbandSize() const
{
    return m_subbandSize;
}

} // namespace ns3
