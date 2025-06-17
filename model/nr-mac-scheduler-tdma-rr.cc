// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mac-scheduler-tdma-rr.h"

#include "nr-mac-scheduler-ue-info-rr.h"

#include "ns3/log.h"

#include <algorithm>
#include <functional>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrMacSchedulerTdmaRR");
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerTdmaRR);

TypeId
NrMacSchedulerTdmaRR::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacSchedulerTdmaRR")
                            .SetParent<NrMacSchedulerTdma>()
                            .AddConstructor<NrMacSchedulerTdmaRR>();
    return tid;
}

NrMacSchedulerTdmaRR::NrMacSchedulerTdmaRR()
    : NrMacSchedulerTdma()
{
    NS_LOG_FUNCTION(this);
}

std::shared_ptr<NrMacSchedulerUeInfo>
NrMacSchedulerTdmaRR::CreateUeRepresentation(
    const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) const
{
    NS_LOG_FUNCTION(this);
    return std::make_shared<NrMacSchedulerUeInfoRR>(
        params.m_rnti,
        params.m_beamId,
        std::bind(&NrMacSchedulerTdmaRR::GetNumRbPerRbg, this));
}

void
NrMacSchedulerTdmaRR::AssignedDlResources(const UePtrAndBufferReq& ue,
                                          [[maybe_unused]] const FTResources& assigned,
                                          [[maybe_unused]] const FTResources& totAssigned) const
{
    NS_LOG_FUNCTION(this);
    GetFirst GetUe;
    auto oldTbSize = GetUe(ue)->m_dlTbSize;
    GetUe(ue)->UpdateDlMetric();
    auto newTbSize = GetUe(ue)->m_dlTbSize;

    if (m_dlRntiSet.find(GetUe(ue)->m_rnti) == m_dlRntiSet.end())
    {
        m_dlRntiSet.emplace(GetUe(ue)->m_rnti);
        m_dlRrRntiDeque.push_front(GetUe(ue)->m_rnti);
    }
    auto it = std::find(m_dlRrRntiDeque.begin(), m_dlRrRntiDeque.end(), GetUe(ue)->m_rnti);

    // If transport block size increased, move to end of list
    if (newTbSize > oldTbSize)
    {
        m_dlRrRntiDeque.erase(it);
        m_dlRrRntiDeque.push_back(GetUe(ue)->m_rnti);
    }
    // If it decreased (resources were reaped), move to beginning of list
    else if (newTbSize < oldTbSize)
    {
        m_dlRrRntiDeque.erase(it);
        m_dlRrRntiDeque.push_front(GetUe(ue)->m_rnti);
    }
}

void
NrMacSchedulerTdmaRR::AssignedUlResources(const UePtrAndBufferReq& ue,
                                          [[maybe_unused]] const FTResources& assigned,
                                          [[maybe_unused]] const FTResources& totAssigned) const
{
    NS_LOG_FUNCTION(this);
    GetFirst GetUe;
    GetUe(ue)->UpdateUlMetric();
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq& lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq& rhs)>
NrMacSchedulerTdmaRR::GetUeCompareDlFn() const
{
    return [this](const NrMacSchedulerNs3::UePtrAndBufferReq& a,
                  const NrMacSchedulerNs3::UePtrAndBufferReq& b) {
        for (const auto& c : {a, b})
        {
            if (m_dlRntiSet.find(c.first->m_rnti) == m_dlRntiSet.end())
            {
                m_dlRntiSet.emplace(c.first->m_rnti);
                m_dlRrRntiDeque.push_front(c.first->m_rnti);
            }
        }
        // Search for either A or B RNTI
        auto it = std::find_if(m_dlRrRntiDeque.begin(),
                               m_dlRrRntiDeque.end(),
                               [aRnti = a.first->m_rnti, bRnti = b.first->m_rnti](auto& cRnti) {
                                   return (cRnti == aRnti) | (cRnti == bRnti);
                               });
        // If first found RNTI is A, then A < B
        return *it == a.first->m_rnti;
    };
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq& lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq& rhs)>
NrMacSchedulerTdmaRR::GetUeCompareUlFn() const
{
    return NrMacSchedulerUeInfoRR::CompareUeWeightsUl;
}

} // namespace ns3
