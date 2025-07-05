// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mac-scheduler-ofdma-random.h"

#include "ns3/log.h"
#include "ns3/shuffle.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("NrMacSchedulerOfdmaRandom");
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerOfdmaRandom);

TypeId
NrMacSchedulerOfdmaRandom::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacSchedulerOfdmaRandom")
                            .SetParent<NrMacSchedulerOfdma>()
                            .AddConstructor<NrMacSchedulerOfdmaRandom>();
    return tid;
}

NrMacSchedulerOfdmaRandom::NrMacSchedulerOfdmaRandom()
    : NrMacSchedulerOfdma()
{
    NS_LOG_FUNCTION(this);
    m_uniformRvShuffle = CreateObject<UniformRandomVariable>();
}

void
NrMacSchedulerOfdmaRandom::AssignedDlResources(
    const UePtrAndBufferReq& ue,
    [[maybe_unused]] const FTResources& assigned,
    [[maybe_unused]] const FTResources& totAssigned) const
{
    NS_LOG_FUNCTION(this);
    GetFirst GetUe;
    GetUe(ue)->UpdateDlMetric();
}

void
NrMacSchedulerOfdmaRandom::AssignedUlResources(
    const UePtrAndBufferReq& ue,
    [[maybe_unused]] const FTResources& assigned,
    [[maybe_unused]] const FTResources& totAssigned) const
{
    NS_LOG_FUNCTION(this);
    GetFirst GetUe;
    GetUe(ue)->UpdateUlMetric();
}

std::shared_ptr<NrMacSchedulerUeInfo>
NrMacSchedulerOfdmaRandom::CreateUeRepresentation(
    const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) const
{
    NS_LOG_FUNCTION(this);
    return std::make_shared<NrMacSchedulerUeInfo>(
        params.m_rnti,
        params.m_beamId,
        std::bind(&NrMacSchedulerOfdmaRandom::GetNumRbPerRbg, this));
}

void
NrMacSchedulerOfdmaRandom::SortUeVector(std::vector<UePtrAndBufferReq>* ueVector,
                                        const GetCompareUeFn& GetCompareFn) const
{
    NS_LOG_FUNCTION(this);
    if (ueVector == nullptr)
    {
        return;
    }

    Shuffle(ueVector->begin(), ueVector->end(), m_uniformRvShuffle);
}

int64_t
NrMacSchedulerOfdmaRandom::AssignStreams(int64_t stream)
{
    NS_LOG_FUNCTION(this << stream);
    NrMacSchedulerNs3::AssignStreams(stream);
    m_uniformRvShuffle->SetStream(stream);
    return 1;
}

} // namespace ns3
