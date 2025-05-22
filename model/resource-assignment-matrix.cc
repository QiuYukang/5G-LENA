// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "resource-assignment-matrix.h"

#include "../utils/termcolor.hpp"

#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3
{
static constexpr ResourceAssignmentMatrix::Rnti EMPTY_RESOURCE_RNTI =
    std::numeric_limits<ResourceAssignmentMatrix::Rnti>::max();

static constexpr ResourceAssignmentMatrix::Rnti CTRL_RESOURCE_RNTI = EMPTY_RESOURCE_RNTI - 1;
// todo: plot resource grid like in https://www.sharetechnote.com/html/5G/5G_ResourceGrid.html

NS_LOG_COMPONENT_DEFINE("ResourceAssignmentMatrix");

ResourceAssignmentMatrix::ResourceAssignmentMatrix(std::vector<bool> notchingMask,
                                                   uint8_t numSymbols)
{
    m_notchingMask = notchingMask;
    m_symbolResources.resize(numSymbols);
    m_beamIdAssigned.resize(numSymbols, false);
    for (auto& symbolResource : m_symbolResources)
    {
        symbolResource.rbgs =
            std::vector<ResourceMatrixEntry>(notchingMask.size(),
                                             {EMPTY_RESOURCE_RNTI, ResourceType::EMPTY});
    }
}

void
ResourceAssignmentMatrix::AssignBeamIdToSymbols(BeamId beamId,
                                                uint8_t startingSymbol,
                                                uint8_t numSymbols)
{
    NS_LOG_FUNCTION(this);
    for (auto symbol = startingSymbol; symbol < startingSymbol + numSymbols; symbol++)
    {
        NS_ASSERT_MSG(
            m_symbolResources.size() > symbol,
            "Mismatch between the symbol allocated and size of resource assignment matrix");
        NS_ASSERT_MSG(
            !m_beamIdAssigned.at(symbol) ||
                (m_beamIdAssigned.at(symbol) && (m_symbolResources.at(symbol).beamId == beamId)),
            "Assigning more than a beam per symbol (" << m_symbolResources.at(symbol).beamId
                                                      << ") vs (" << beamId << ")");
        m_symbolResources.at(symbol).beamId = beamId;
        m_beamIdAssigned.at(symbol) = true;
    }
}

void
ResourceAssignmentMatrix::AssignTdmaChannelDuringSymbolToUe(ResourceType type,
                                                            Rnti rnti,
                                                            uint8_t startingSymbol,
                                                            uint8_t numSymbols)
{
    NS_LOG_FUNCTION(this);
    for (auto symbol = startingSymbol; symbol < startingSymbol + numSymbols; symbol++)
    {
        NS_ASSERT_MSG(
            m_symbolResources.size() > symbol,
            "Mismatch between the symbol allocated and size of resource assignment matrix");
        NS_ASSERT_MSG(!m_notchingMask.empty(), "Notching mask was not properly configured");

        auto& symbolResources = m_symbolResources.at(symbol);
        for (std::size_t rbg = 0; rbg < m_notchingMask.size(); rbg++)
        {
            // Notching mask 111000111 prevents allocation on unset bits
            if (m_notchingMask.at(rbg))
            {
                NS_ASSERT_MSG(symbolResources.rbgs.at(rbg).allocatedUe == EMPTY_RESOURCE_RNTI,
                              "Allocating the same RBG for two different UEs during symbol "
                                  << (uint16_t)symbol);
                symbolResources.rbgs.at(rbg).allocatedUe = rnti;
                symbolResources.rbgs.at(rbg).allocatedResourceType = type;
                m_ueNumberOfResources[rnti]++;
            }
        }
    }
}

void
ResourceAssignmentMatrix::AssignOfdmaRbgDuringSymbolToUe(ResourceType type,
                                                         Rnti rnti,
                                                         uint16_t rbg,
                                                         uint8_t startingSymbol,
                                                         uint8_t numSymbols)
{
    NS_LOG_FUNCTION(this);
    for (auto symbol = startingSymbol; symbol < startingSymbol + numSymbols; symbol++)
    {
        NS_ASSERT_MSG(
            m_symbolResources.size() > symbol,
            "Mismatch between the symbol allocated and size of resource assignment matrix");
        NS_ASSERT_MSG(!m_notchingMask.empty(), "Notching mask was not properly configured");
        NS_ASSERT_MSG(m_notchingMask.size() > rbg, "RBG is bigger than notching mask");
        if (type == ResourceType::DL_DATA || type == ResourceType::UL_DATA ||
            type == ResourceType::SRS || type == ResourceType::MSG3 || type == ResourceType::HARQ)
        {
            // Assert should not apply to PCCCH, PDCCH nor PUCCH
            NS_ASSERT_MSG(m_notchingMask.at(rbg),
                          "Trying to assign a notched RBG"); // Notching mask 111000111 only allows
                                                             // allocation on set bits
        }

        auto& symbolResources = m_symbolResources.at(symbol);

        NS_ASSERT_MSG(symbolResources.rbgs.at(rbg).allocatedUe == EMPTY_RESOURCE_RNTI,
                      "Allocating the same resource for two different UEs");
        symbolResources.rbgs.at(rbg).allocatedUe = rnti;
        symbolResources.rbgs.at(rbg).allocatedResourceType = type;
        m_ueNumberOfResources[rnti]++;
    }
}

void
ResourceAssignmentMatrix::AssignTdmaChannelDuringSymbolToCtrl(ResourceType type,
                                                              uint8_t startingSymbol,
                                                              uint8_t numSymbols)
{
    AssignTdmaChannelDuringSymbolToUe(type, CTRL_RESOURCE_RNTI, startingSymbol, numSymbols);
}

void
ResourceAssignmentMatrix::AssignOfdmaRbgDuringSymbolToCtrl(ResourceType type,
                                                           Rbg rbg,
                                                           uint8_t startingSymbol,
                                                           uint8_t numSymbols)
{
    AssignOfdmaRbgDuringSymbolToUe(type, CTRL_RESOURCE_RNTI, rbg, startingSymbol, numSymbols);
}

std::size_t
ResourceAssignmentMatrix::GetAssignedResourcesTotal()
{
    std::size_t assignedResources = 0;
    for (auto& symbolResource : m_symbolResources)
    {
        for (std::size_t rbg = 0; rbg < m_notchingMask.size(); rbg++)
        {
            // We discount notched resources
            if (!m_notchingMask.at(rbg))
            {
                assignedResources++;
                continue;
            }
            // And assigned resources
            if (symbolResource.rbgs.at(rbg).allocatedUe != EMPTY_RESOURCE_RNTI)
            {
                assignedResources++;
            }
        }
    }
    return assignedResources;
}

std::size_t
ResourceAssignmentMatrix::GetFreeResourcesTotal()
{
    std::size_t freeResources = 0;
    for (auto& symbolResource : m_symbolResources)
    {
        for (std::size_t rbg = 0; rbg < m_notchingMask.size(); rbg++)
        {
            // We skip notched resources
            if (!m_notchingMask.at(rbg))
            {
                continue;
            }
            // And assigned resources
            if (symbolResource.rbgs.at(rbg).allocatedUe == EMPTY_RESOURCE_RNTI)
            {
                freeResources++;
            }
        }
    }
    return freeResources;
}

std::vector<ResourceAssignmentMatrix::AssignedResourceElement>
ResourceAssignmentMatrix::GetAssignedResourcesToUe(Rnti rnti)
{
    std::vector<ResourceAssignmentMatrix::AssignedResourceElement> ueResources;
    uint8_t symbol = 0;
    for (auto& symbolResource : m_symbolResources)
    {
        for (std::size_t rbg = 0; rbg < m_notchingMask.size(); rbg++)
        {
            if (symbolResource.rbgs.at(rbg).allocatedUe == rnti)
            {
                ueResources.push_back({symbolResource.beamId, static_cast<uint16_t>(rbg), symbol});
            }
        }
        symbol++;
    }
    return ueResources;
}

std::size_t
ResourceAssignmentMatrix::GetNumAssignedResourcesToUe(ResourceAssignmentMatrix::Rnti rnti)
{
    return m_ueNumberOfResources[rnti];
}

void
ResourceAssignmentMatrix::PlotResourceMatrix()
{
    using namespace termcolor;

    // Map RNTIs to equally spaced colors
    std::unordered_map<uint16_t, uint8_t> rntiToColor;

    // Empty spaces are always white
    rntiToColor[EMPTY_RESOURCE_RNTI] = 255;
    rntiToColor[CTRL_RESOURCE_RNTI] = 0;

    // Plot grid
    std::cout << "Symbols \\ RBG" << std::endl;
    std::unordered_map<ResourceType, uint32_t> numRes;

    for (auto& symbolResource : m_symbolResources)
    {
        for (std::size_t rbg = 0; rbg < m_notchingMask.size(); rbg++)
        {
            auto rnti = symbolResource.rbgs.at(rbg).allocatedUe;
            auto type = symbolResource.rbgs.at(rbg).allocatedResourceType;
            switch (rnti)
            {
            case EMPTY_RESOURCE_RNTI:
                std::cout << on_white << " * " << reset;
                break;
            case CTRL_RESOURCE_RNTI:
                std::cout << on_red << " * " << reset;
                break;
            default:
                numRes[type]++;
                switch (type)
                {
                case ResourceType::EMPTY:
                    std::cout << on_white << " * " << reset;
                    break;
                case ResourceType::DL_DCI:
                    std::cout << on_green << " * " << reset;
                    break;
                case ResourceType::UL_DCI:
                    std::cout << on_cyan << " * " << reset;
                    break;
                case ResourceType::DL_DATA:
                    std::cout << on_bright_green << " * " << reset;
                    break;
                case ResourceType::UL_DATA:
                    std::cout << on_bright_cyan << " * " << reset;
                    break;
                case ResourceType::HARQ:
                    std::cout << on_magenta << " * " << reset;
                    break;
                case ResourceType::SRS:
                    std::cout << on_bright_magenta << " * " << reset;
                    break;
                case ResourceType::MSG3:
                    std::cout << on_color<200> << " * " << reset;
                    break;
                case ResourceType::CSI_RS:
                    std::cout << on_blue << " * " << reset;
                    break;
                case ResourceType::PDSCH_DMRS:
                    std::cout << on_red << " * " << reset;
                    break;
                case ResourceType::PUSCH_DMRS:
                    std::cout << on_bright_blue << " * " << reset;
                    break;
                case ResourceType::PBSCH_DMRS:
                    std::cout << on_bright_red << " * " << reset;
                    break;
                case ResourceType::PTRS:
                    std::cout << on_yellow << " * " << reset;
                    break;
                case ResourceType::TRS:
                    std::cout << on_bright_yellow << " * " << reset;
                    break;
                default:
                    std::cout << on_grey << " * " << reset;
                }
            }
        }
        std::cout << std::endl;
    }
    // Plot stats
    auto freeResources = GetFreeResourcesTotal();
    auto usedResources = GetAssignedResourcesTotal();
    auto totalResources = freeResources + usedResources;
    auto controlResources = GetAssignedResourcesToUe(CTRL_RESOURCE_RNTI).size();
    using RT = ResourceType;

    // clang-format off
    std::cout << "\nUE resources: " << totalResources - freeResources - controlResources
              << "\n├ " << on_green << " * " << reset << " DL_DCI: " << numRes[RT::DL_DCI]
              << "\n├ " << on_cyan << " * " << reset << " UL_DCI: " << numRes[RT::UL_DCI]
              << "\n├ " << on_bright_green << " * " << reset << " DL_DATA: " << numRes[RT::DL_DATA]
              << "\n├ " << on_bright_cyan << " * " << reset << " UL_DATA: " << numRes[RT::UL_DATA]
              << "\n├ " << on_magenta << " * " << reset << " HARQ: " << numRes[RT::HARQ]
              << "\n├ " << on_bright_magenta << " * " << reset << " SRS: " << numRes[RT::SRS]
              << "\n├ " << on_color<200> << " * " << reset << " MSG3: " << numRes[RT::MSG3]
              << "\n├ " << on_blue << " * " << reset << " CSI_RS: " << numRes[RT::CSI_RS]
              << "\n├ " << on_red << " * " << reset << " PDSCH_DMRS: " << numRes[RT::PDSCH_DMRS]
              << "\n├ " << on_bright_blue << " * " << reset << " PUSCH_DMRS: " << numRes[RT::PUSCH_DMRS]
              << "\n├ " << on_bright_red << " * " << reset << " PBSCH_DMRS: " << numRes[RT::PBSCH_DMRS]
              << "\n├ " << on_yellow << " * " << reset << " PTRS: " << numRes[RT::PTRS]
              << "\n└ " << on_bright_yellow << " * " << reset << " TRS: " << numRes[RT::TRS]
              << "\n"   << on_red << " * " << reset << " Control resources: " << controlResources
              << "\n"   << on_white << " * " << reset << " Empty resources: " << freeResources
              << "\n   Total resources: " << totalResources
              << "\n" << std::endl;
    // clang-format on
}

void
ResourceAssignmentMatrix::CheckResourceMatrixFromVarTtiAllocInfo(
    const std::deque<VarTtiAllocInfo>& allocInfo,
    const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& ueMap,
    const std::vector<bool>& notchingMask,
    uint8_t numSym,
    bool plot)
{
    ResourceAssignmentMatrix resourceMatrix(notchingMask, numSym);
    for (auto& alloc : allocInfo)
    {
        auto ueMapEntry = ueMap.find(alloc.m_dci->m_rnti);
        auto beamId = OMNI_BEAM_ID;
        if (ueMapEntry != ueMap.end())
        {
            beamId = alloc.m_isOmni ? beamId : ueMapEntry->second->m_beamId;
        }
        resourceMatrix.AssignBeamIdToSymbols(beamId,
                                             alloc.m_dci->m_symStart,
                                             alloc.m_dci->m_numSym);

        ResourceAssignmentMatrix::ResourceType type;
        if (alloc.m_dci->m_type == ns3::DciInfoElementTdma::CTRL)
        {
            if (alloc.m_dci->m_format == DciInfoElementTdma::DL)
            {
                type = ResourceAssignmentMatrix::ResourceType::DL_DCI;
            }
            else
            {
                type = ResourceAssignmentMatrix::ResourceType::UL_DCI;
            }
        }
        else
        {
            if (alloc.m_dci->m_format == DciInfoElementTdma::DL)
            {
                type = ResourceAssignmentMatrix::ResourceType::DL_DATA;
            }
            else
            {
                type = ResourceAssignmentMatrix::ResourceType::UL_DATA;
            }
            if (alloc.m_dci->m_rv > 0)
            {
                type = ResourceAssignmentMatrix::ResourceType::HARQ;
            }
        }
        if (alloc.m_dci->m_type == DciInfoElementTdma::SRS)
        {
            type = ResourceAssignmentMatrix::ResourceType::SRS;
        }
        if (alloc.m_dci->m_type == DciInfoElementTdma::MSG3)
        {
            type = ResourceAssignmentMatrix::ResourceType::MSG3;
        }
        for (size_t rbg = 0; rbg < alloc.m_dci->m_rbgBitmask.size(); rbg++)
        {
            if (alloc.m_dci->m_rbgBitmask.at(rbg))
            {
                resourceMatrix.AssignOfdmaRbgDuringSymbolToUe(type,
                                                              alloc.m_dci->m_rnti,
                                                              rbg,
                                                              alloc.m_dci->m_symStart,
                                                              alloc.m_dci->m_numSym);
            }
        }
    }
    if (plot)
    {
        resourceMatrix.PlotResourceMatrix();
    }
}

} // namespace ns3
