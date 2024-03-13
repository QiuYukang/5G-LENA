/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-harq-phy.h"

#include <ns3/assert.h>
#include <ns3/log.h>

NS_LOG_COMPONENT_DEFINE("NrHarqPhy");

namespace ns3
{

NrHarqPhy::~NrHarqPhy()
{
    NS_LOG_FUNCTION(this);
    m_dlHistory.clear();
    m_ulHistory.clear();
}

const NrErrorModel::NrErrorModelHistory&
NrHarqPhy::GetHarqProcessInfoDl(uint16_t rnti, uint8_t harqProcId)
{
    NS_LOG_FUNCTION(this);
    return GetHarqProcessInfo(&m_dlHistory, rnti, harqProcId);
}

const NrErrorModel::NrErrorModelHistory&
NrHarqPhy::GetHarqProcessInfoUl(uint16_t rnti, uint8_t harqProcId)
{
    NS_LOG_FUNCTION(this);
    return GetHarqProcessInfo(&m_ulHistory, rnti, harqProcId);
}

void
NrHarqPhy::UpdateDlHarqProcessStatus(uint16_t rnti,
                                     uint8_t harqProcId,
                                     const Ptr<NrErrorModelOutput>& output)
{
    NS_LOG_FUNCTION(this);
    UpdateHarqProcessStatus(&m_dlHistory, rnti, harqProcId, output);
}

void
NrHarqPhy::ResetDlHarqProcessStatus(uint16_t rnti, uint8_t id)
{
    NS_LOG_FUNCTION(this);
    ResetHarqProcessStatus(&m_dlHistory, rnti, id);
}

void
NrHarqPhy::UpdateUlHarqProcessStatus(uint16_t rnti,
                                     uint8_t harqProcId,
                                     const Ptr<NrErrorModelOutput>& output)
{
    NS_LOG_FUNCTION(this);
    UpdateHarqProcessStatus(&m_ulHistory, rnti, harqProcId, output);
}

void
NrHarqPhy::ResetUlHarqProcessStatus(uint16_t rnti, uint8_t id)
{
    NS_LOG_FUNCTION(this);
    ResetHarqProcessStatus(&m_ulHistory, rnti, id);
}

NrHarqPhy::HistoryMap::iterator
NrHarqPhy::GetHistoryMapOf(NrHarqPhy::HistoryMap* map, uint16_t rnti) const
{
    NrHarqPhy::HistoryMap::iterator it = map->find(rnti);
    if (it == map->end())
    {
        auto ret = map->insert(std::make_pair(rnti, ProcIdHistoryMap()));
        NS_ASSERT(ret.second);

        it = ret.first;
    }

    return it;
}

NrHarqPhy::ProcIdHistoryMap::iterator
NrHarqPhy::GetProcIdHistoryMapOf(NrHarqPhy::ProcIdHistoryMap* map, uint16_t procId) const
{
    NrHarqPhy::ProcIdHistoryMap::iterator it = map->find(procId);
    if (it == map->end())
    {
        auto ret = map->insert(std::make_pair(procId, NrErrorModel::NrErrorModelHistory()));
        NS_ASSERT(ret.second);

        it = ret.first;
    }

    return it;
}

void
NrHarqPhy::ResetHarqProcessStatus(NrHarqPhy::HistoryMap* map,
                                  uint16_t rnti,
                                  uint8_t harqProcId) const
{
    NS_LOG_FUNCTION(this);

    NrHarqPhy::HistoryMap::iterator historyMap = GetHistoryMapOf(map, rnti);

    ProcIdHistoryMap* procIdMap = &(historyMap->second);

    GetProcIdHistoryMapOf(procIdMap, harqProcId)->second.clear();
}

void
NrHarqPhy::UpdateHarqProcessStatus(NrHarqPhy::HistoryMap* map,
                                   uint16_t rnti,
                                   uint8_t harqProcId,
                                   const Ptr<NrErrorModelOutput>& output) const
{
    NS_LOG_FUNCTION(this);

    NrHarqPhy::HistoryMap::iterator historyMap = GetHistoryMapOf(map, rnti);

    ProcIdHistoryMap* procIdMap = &(historyMap->second);

    GetProcIdHistoryMapOf(procIdMap, harqProcId)->second.emplace_back(output);
}

const NrErrorModel::NrErrorModelHistory&
NrHarqPhy::GetHarqProcessInfo(NrHarqPhy::HistoryMap* map, uint16_t rnti, uint8_t harqProcId) const
{
    NrHarqPhy::HistoryMap::iterator historyMap = GetHistoryMapOf(map, rnti);

    ProcIdHistoryMap* procIdMap = &(historyMap->second);

    return GetProcIdHistoryMapOf(procIdMap, harqProcId)->second;
}

// NR SL
const NrErrorModel::NrErrorModelHistory&
NrHarqPhy::GetHarqProcessInfoSl(uint16_t rnti, uint8_t harqProcId)
{
    return GetHarqProcessInfo(&m_slHistory, rnti, harqProcId);
}

void
NrHarqPhy::UpdateSlDataHarqProcessStatus(uint16_t rnti,
                                         uint8_t harqProcId,
                                         const Ptr<NrErrorModelOutput>& output)
{
    NS_LOG_FUNCTION(this);
    UpdateHarqProcessStatus(&m_slHistory, rnti, harqProcId, output);
}

void
NrHarqPhy::ResetSlDataHarqProcessStatus(uint16_t rnti, uint8_t id)
{
    NS_LOG_FUNCTION(this);
    ResetHarqProcessStatus(&m_slHistory, rnti, id);
}

void
NrHarqPhy::IndicatePrevDecoded(uint16_t rnti, uint8_t harqId)
{
    NS_LOG_FUNCTION(this << rnti << static_cast<uint16_t>(harqId));

    uint32_t decodedId = (rnti << 8) + harqId;

    if (m_slDecodedTb.find(decodedId) == m_slDecodedTb.end())
    {
        m_slDecodedTb.insert(decodedId);
    }
}

bool
NrHarqPhy::IsPrevDecoded(uint16_t rnti, uint8_t harqId)
{
    NS_LOG_FUNCTION(this << rnti << static_cast<uint16_t>(harqId));

    uint32_t decodedId = (rnti << 8) + harqId;

    std::unordered_set<uint32_t>::iterator it = m_slDecodedTb.find(decodedId);

    bool prevDecoded = (it != m_slDecodedTb.end());

    return prevDecoded;
}

void
NrHarqPhy::RemovePrevDecoded(uint16_t rnti, uint8_t harqId)
{
    NS_LOG_FUNCTION(this << rnti << static_cast<uint16_t>(harqId));

    uint32_t decodedId = (rnti << 8) + harqId;

    std::unordered_set<uint32_t>::iterator it = m_slDecodedTb.find(decodedId);

    if (it != m_slDecodedTb.end())
    {
        m_slDecodedTb.erase(it);
    }
}

} // namespace ns3
