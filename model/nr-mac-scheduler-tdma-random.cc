// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mac-scheduler-tdma-random.h"

#include "ns3/log.h"

#include <random>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrMacSchedulerTdmaRandom");
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerTdmaRandom);

TypeId
NrMacSchedulerTdmaRandom::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacSchedulerTdmaRandom")
                            .SetParent<NrMacSchedulerTdma>()
                            .AddConstructor<NrMacSchedulerTdmaRandom>();
    return tid;
}

NrMacSchedulerTdmaRandom::NrMacSchedulerTdmaRandom()
    : NrMacSchedulerTdma()
{
    NS_LOG_FUNCTION(this);
}

std::shared_ptr<NrMacSchedulerUeInfo>
NrMacSchedulerTdmaRandom::CreateUeRepresentation(
    const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) const
{
    NS_LOG_FUNCTION(this);
    return std::make_shared<NrMacSchedulerUeInfo>(
        params.m_rnti,
        params.m_beamId,
        std::bind(&NrMacSchedulerTdmaRandom::GetNumRbPerRbg, this));
}

void
NrMacSchedulerTdmaRandom::AssignedDlResources(const UePtrAndBufferReq& ue,
                                              [[maybe_unused]] const FTResources& assigned,
                                              [[maybe_unused]] const FTResources& totAssigned) const
{
    NS_LOG_FUNCTION(this);
    GetFirst GetUe;
    GetUe(ue)->UpdateDlMetric();
}

void
NrMacSchedulerTdmaRandom::AssignedUlResources(const UePtrAndBufferReq& ue,
                                              [[maybe_unused]] const FTResources& assigned,
                                              [[maybe_unused]] const FTResources& totAssigned) const
{
    NS_LOG_FUNCTION(this);
    GetFirst GetUe;
    GetUe(ue)->UpdateUlMetric();
}

void
NrMacSchedulerTdmaRandom::SortUeVector(std::vector<UePtrAndBufferReq>* ueVector,
                                       const GetCompareUeFn& GetCompareFn) const
{

}

} // namespace ns3
