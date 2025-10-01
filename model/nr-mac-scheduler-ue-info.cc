// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mac-scheduler-ue-info.h"

#include "ns3/log.h"

#include <numeric>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrMacSchedulerUeInfo");

NrMacSchedulerUeInfo::NrMacSchedulerUeInfo(uint16_t rnti, BeamId beamId, const GetRbPerRbgFn& fn)
    : m_rnti(rnti),
      m_beamId(beamId),
      m_getNumRbPerRbg(fn)
{
}

NrMacSchedulerUeInfo::~NrMacSchedulerUeInfo()
{
}

std::vector<uint16_t>&
NrMacSchedulerUeInfo::GetDlRBG(const UePtr& ue)
{
    return ue->m_dlRBG;
}

std::vector<uint16_t>&
NrMacSchedulerUeInfo::GetUlRBG(const UePtr& ue)
{
    return ue->m_ulRBG;
}

std::vector<uint8_t>&
NrMacSchedulerUeInfo::GetDlSym(const UePtr& ue)
{
    return ue->m_dlSym;
}

std::vector<uint8_t>&
NrMacSchedulerUeInfo::GetUlSym(const UePtr& ue)
{
    return ue->m_ulSym;
}

uint8_t&
NrMacSchedulerUeInfo::GetDlMcs(const UePtr& ue)
{
    return ue->m_dlMcs;
}

template <typename T>
uint8_t
ComputeMcs(const NrMacSchedulerUeInfo* ueInfo,
           T NrMacSchedulerUeInfo::SbMcsInfo::* field,
           std::function<uint8_t(double)> postProcessing)
{
    // Compute average field of allocated RBGs
    const auto sum = std::transform_reduce(
        ueInfo->m_dlRBG.begin(),
        ueInfo->m_dlRBG.end(),
        0.0,
        [](auto a, auto b) { return a + b; },
        [ueInfo, field](auto a) {
            return ueInfo->m_dlSbMcsInfo.at(ueInfo->m_rbgToSb.at(a)).*field;
        });
    const auto avg = sum / ueInfo->m_dlRBG.size();
    return postProcessing(avg);
}

uint8_t
NrMacSchedulerUeInfo::GetDlMcs() const
{
    // Return maximum allowed MCS according to Fronthaul control
    if (m_fhMaxMcsAssignable.has_value())
    {
        return m_fhMaxMcsAssignable.value();
    }

    // In case there is no sub-band info or no RBG has been allocated, return the wideband MCS
    if (m_dlSbMcsInfo.empty() || m_dlRBG.empty() || (m_mcsCsiSource == McsCsiSource::WIDEBAND_MCS))
    {
        return m_dlMcs;
    }

    // Otherwise, compute the SINR of allocated RBGs
    switch (m_mcsCsiSource)
    {
    // Estimate MCS based on the average MCS of allocated RBGs
    case McsCsiSource::AVG_MCS: {
        return ComputeMcs(this, &SbMcsInfo::mcs, [](double avg) { return (uint8_t)floor(avg); });
    }
    // Estimate MCS based on the average spectral efficiency of allocated RBGs
    case McsCsiSource::AVG_SPEC_EFF: {
        return ComputeMcs(this,
                          &SbMcsInfo::specEff,
                          std::bind_front(&NrAmc::GetMcsFromSpectralEfficiency, m_dlAmc));
    }
    // Estimate MCS based on the average SINR of allocated RBGs
    case McsCsiSource::AVG_SINR: {
        return ComputeMcs(this, &SbMcsInfo::sinr, [amc = m_dlAmc](double avgSinr) {
            return amc->GetMcsFromSpectralEfficiency(amc->GetSpectralEfficiencyForSinr(avgSinr));
        });
    }
    default:
        NS_ABORT_MSG("Invalid csi source for MCS computation");
    }
}

uint8_t&
NrMacSchedulerUeInfo::GetUlMcs(const UePtr& ue)
{
    return ue->m_ulMcs;
}

uint32_t&
NrMacSchedulerUeInfo::GetDlTBS(const UePtr& ue)
{
    return ue->m_dlTbSize;
}

std::unordered_map<uint8_t, LCGPtr>&
NrMacSchedulerUeInfo::GetDlLCG(const UePtr& ue)
{
    return ue->m_dlLCG;
}

std::unordered_map<uint8_t, LCGPtr>&
NrMacSchedulerUeInfo::GetUlLCG(const UePtr& ue)
{
    return ue->m_ulLCG;
}

NrMacHarqVector&
NrMacSchedulerUeInfo::GetDlHarqVector(const UePtr& ue)
{
    return ue->m_dlHarq;
}

NrMacHarqVector&
NrMacSchedulerUeInfo::GetUlHarqVector(const UePtr& ue)
{
    return ue->m_ulHarq;
}

void
NrMacSchedulerUeInfo::PrintLcInfo(uint16_t ue,
                                  uint8_t lcgId,
                                  uint8_t lcId,
                                  uint8_t qci,
                                  uint8_t P,
                                  uint8_t minP)
{
    NS_LOG_DEBUG("UE " << ue << " LCG ID: " << static_cast<uint32_t>(lcgId) << " LC ID "
                       << static_cast<uint32_t>(lcId) << " QCI: " << static_cast<uint32_t>(qci)
                       << " P: " << static_cast<uint32_t>(P) << " minP: " << +minP);
}

void
NrMacSchedulerUeInfo::ResetDlSchedInfo()
{
    m_dlMRBRetx = 0;
    m_dlRBG.clear();
    m_dlSym.clear();
    m_dlTbSize = 0;
}

void
NrMacSchedulerUeInfo::ResetUlSchedInfo()
{
    m_ulMRBRetx = 0;
    m_ulRBG.clear();
    m_ulSym.clear();
    m_ulTbSize = 0;
}

void
NrMacSchedulerUeInfo::UpdateDlMetric()
{
    if (m_dlRBG.empty())
    {
        m_dlTbSize = 0;
    }
    else
    {
        m_dlTbSize =
            m_dlAmc->CalculateTbSize(GetDlMcs(), m_dlRank, m_dlRBG.size() * GetNumRbPerRbg());
    }
}

void
NrMacSchedulerUeInfo::ResetDlMetric()
{
    m_dlTbSize = 0;
}

void
NrMacSchedulerUeInfo::UpdateUlMetric()
{
    if (m_ulRBG.empty())
    {
        m_ulTbSize = 0;
    }
    else
    {
        m_ulTbSize = m_ulAmc->CalculateTbSize(m_ulMcs, m_ulRank, m_ulRBG.size() * GetNumRbPerRbg());
    }
}

void
NrMacSchedulerUeInfo::ResetUlMetric()
{
    m_ulTbSize = 0;
}

uint32_t
NrMacSchedulerUeInfo::GetTotalDlBuffer() const
{
    uint32_t totBuffer = 0;
    for (const auto& lcgInfo : m_dlLCG)
    {
        const auto& lcg = lcgInfo.second;
        totBuffer += lcg->GetTotalSize();
    }
    return totBuffer;
}

uint32_t
NrMacSchedulerUeInfo::GetNumRbPerRbg() const
{
    return m_getNumRbPerRbg();
}

void
NrMacSchedulerUeInfo::ReleaseLC(uint8_t lcid)
{
    for (auto lcgIt = m_dlLCG.begin(); lcgIt != m_dlLCG.end(); lcgIt++)
    {
        lcgIt->second->ReleaseLC(lcid);
    }
    for (auto lcgIt = m_ulLCG.begin(); lcgIt != m_ulLCG.end(); lcgIt++)
    {
        lcgIt->second->ReleaseLC(lcid);
    }
    auto it = m_dlLCG.begin();
    while (it != m_dlLCG.end())
    {
        if (it->second->GetLCId().empty())
        {
            m_dlLCG.erase(it);
            it = m_dlLCG.begin();
        }
        else
        {
            it++;
        }
    }
    it = m_ulLCG.begin();
    while (it != m_ulLCG.end())
    {
        if (it->second->GetLCId().empty())
        {
            m_ulLCG.erase(it);
            it = m_ulLCG.begin();
        }
        else
        {
            it++;
        }
    }
}

} // namespace ns3
