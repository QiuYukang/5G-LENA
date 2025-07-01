// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#define NS_LOG_APPEND_CONTEXT                                                                      \
    do                                                                                             \
    {                                                                                              \
        std::clog << " [ CellId " << GetCellId() << ", bwpId " << GetBwpId() << "] ";              \
    } while (false);

#include "nr-mac-scheduler-ofdma.h"

#include "nr-fh-control.h"

#include "ns3/log.h"

#include <algorithm>
#include <random>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("NrMacSchedulerOfdma");
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerOfdma);

TypeId
NrMacSchedulerOfdma::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrMacSchedulerOfdma")
            .SetParent<NrMacSchedulerTdma>()
            .AddAttribute("SymPerBeamType",
                          "Type of symbol allocation per beam",
                          EnumValue(SymPerBeamType::LOAD_BASED),
                          MakeEnumAccessor<SymPerBeamType>(&NrMacSchedulerOfdma::SetSymPerBeamType),
                          MakeEnumChecker<SymPerBeamType>(SymPerBeamType::LOAD_BASED,
                                                          "LOAD_BASED",
                                                          SymPerBeamType::ROUND_ROBIN,
                                                          "ROUND_ROBIN",
                                                          SymPerBeamType::PROPORTIONAL_FAIR,
                                                          "PROPORTIONAL_FAIR"))
            .AddTraceSource(
                "SymPerBeam",
                "Number of assigned symbol per beam. Gets called every time an assignment is made",
                MakeTraceSourceAccessor(&NrMacSchedulerOfdma::m_tracedValueSymPerBeam),
                "ns3::TracedValueCallback::Uint32");
    return tid;
}

NrMacSchedulerOfdma::NrMacSchedulerOfdma()
    : NrMacSchedulerTdma()
{
}

void
NrMacSchedulerOfdma::SetSymPerBeamType(SymPerBeamType type)
{
    m_symPerBeamType = type;
    switch (m_symPerBeamType)
    {
    case SymPerBeamType::PROPORTIONAL_FAIR:
        m_symPerBeam = CreateObject<NrMacSchedulerOfdmaSymbolPerBeamPF>(
            [this]() { return m_dlAmc; },
            std::bind_front(&NrMacSchedulerOfdma::GetBandwidthInRbg, this));
        break;
    case SymPerBeamType::ROUND_ROBIN:
        m_symPerBeam = CreateObject<NrMacSchedulerOfdmaSymbolPerBeamRR>();
        break;
    case SymPerBeamType::LOAD_BASED:
        m_symPerBeam = CreateObject<NrMacSchedulerOfdmaSymbolPerBeamLB>();
        break;
    default:
        NS_ABORT_MSG("Invalid NrMacSchedulerOfdma::m_symPerBeamType");
    }
}

NrMacSchedulerOfdma::BeamSymbolMap
NrMacSchedulerOfdma::GetSymPerBeam(uint32_t symAvail,
                                   const NrMacSchedulerNs3::ActiveUeMap& activeDl) const
{
    BeamSymbolMap ret = m_symPerBeam->GetSymPerBeam(symAvail, activeDl);

    // Ensure we have one entry per beam
    for (const auto& [beam, ueVector] : activeDl)
    {
        if (ret.find(beam) == ret.end())
        {
            ret[beam] = 0;
        }
    }

    // Trigger the trace source firing, using const_cast as we don't change
    // the internal state of the class
    for (const auto& v : ret)
    {
        const_cast<NrMacSchedulerOfdma*>(this)->m_tracedValueSymPerBeam = v.second;
    }
    return ret;
}

bool
NrMacSchedulerOfdma::AdvanceToNextUeToSchedule(
    std::vector<UePtrAndBufferReq>::iterator& schedInfoIt,
    const std::vector<UePtrAndBufferReq>::iterator end,
    uint32_t resourcesAssignable) const
{
    // Skip UEs which already have enough resources to transmit
    while (schedInfoIt != end)
    {
        const uint32_t bufQueueSize = schedInfoIt->second;
        if (schedInfoIt->first->m_dlTbSize >= std::max(bufQueueSize, 10U))
        {
            std::advance(schedInfoIt, 1);
        }
        else
        {
            if (m_nrFhSchedSapProvider &&
                m_nrFhSchedSapProvider->GetFhControlMethod() ==
                    NrFhControl::FhControlMethod::OptimizeRBs &&
                !ShouldScheduleUeBasedOnFronthaul(schedInfoIt, resourcesAssignable))
            {
                std::advance(schedInfoIt, 1);
            }
            else
            {
                return true; // UE left to schedule
            }
        }
    }
    return false; // No UE left to schedule
}

bool
NrMacSchedulerOfdma::ShouldScheduleUeBasedOnFronthaul(
    const std::vector<UePtrAndBufferReq>::iterator& schedInfoIt,
    uint32_t resourcesAssignable) const
{
    GetFirst GetUe;
    uint32_t quantizationStep = resourcesAssignable;
    uint32_t maxAssignable = m_nrFhSchedSapProvider->GetMaxRegAssignable(
        GetBwpId(),
        GetUe(*schedInfoIt)->m_dlMcs,
        GetUe(*schedInfoIt)->m_rnti,
        GetUe(*schedInfoIt)->m_dlRank); // maxAssignable is in REGs
    // set a minimum of the maxAssignable equal to 5 RBGs
    maxAssignable = std::max(maxAssignable, 5 * resourcesAssignable);

    // the minimum allocation is one resource in freq, containing rbgAssignable
    // in time (REGs)
    return GetUe(*schedInfoIt)->m_dlRBG.size() + quantizationStep <= maxAssignable;
}

void
NrMacSchedulerOfdma::AllocateCurrentResourceToUe(std::shared_ptr<NrMacSchedulerUeInfo> currentUe,
                                                 const uint32_t& currentRbg,
                                                 const uint32_t beamSym,
                                                 FTResources& assignedResources,
                                                 std::vector<bool>& availableRbgs)
{
    // Assign 1 RBG for each available symbols for the beam,
    // and then update the count of available resources
    auto& assignedRbgs = currentUe->m_dlRBG;
    auto existingRbgs = assignedRbgs.size();
    assignedRbgs.resize(assignedRbgs.size() + beamSym);
    std::fill(assignedRbgs.begin() + existingRbgs, assignedRbgs.end(), currentRbg);
    assignedResources.m_rbg++; // We increment one RBG

    auto& assignedSymbols = currentUe->m_dlSym;
    auto existingSymbols = assignedSymbols.size();
    assignedSymbols.resize(assignedSymbols.size() + beamSym);
    std::iota(assignedSymbols.begin() + existingSymbols, assignedSymbols.end(), 0);
    assignedResources.m_sym = beamSym; // We keep beams per symbol fixed, since it depends on beam

    availableRbgs.at(currentRbg) = false; // Mark RBG as occupied
}

void
NrMacSchedulerOfdma::DeallocateCurrentResourceFromUe(
    std::shared_ptr<NrMacSchedulerUeInfo> currentUe,
    const uint32_t& currentRbg,
    const uint32_t beamSym,
    FTResources& assignedResources,
    std::vector<bool>& availableRbgs)
{
    auto& assignedRbgs = currentUe->m_dlRBG;
    auto& assignedSymbols = currentUe->m_dlSym;

    assignedRbgs.resize(assignedRbgs.size() - beamSym);
    assignedSymbols.resize(assignedSymbols.size() - beamSym);

    NS_ASSERT_MSG(assignedResources.m_rbg > 0,
                  "Should have more than 0 resources allocated before deallocating");
    assignedResources.m_rbg--; // We decrement the allocated RBGs
    // We zero symbols allocated in case number of RBGs reaches 0
    assignedResources.m_sym = assignedResources.m_rbg == 0 ? 0 : assignedResources.m_sym;
    availableRbgs.at(currentRbg) = true;
}

bool
NrMacSchedulerOfdma::AttemptAllocationOfCurrentResourceToUe(
    std::vector<UePtrAndBufferReq>::iterator schedInfoIt,
    std::set<uint32_t>& remainingRbgSet,
    const uint32_t beamSym,
    FTResources& assignedResources,
    std::vector<bool>& availableRbgs) const
{
    auto currentUe = schedInfoIt->first;

    uint32_t currentRbgPos = std::numeric_limits<uint32_t>::max();

    // Use wideband information in case there is no sub-band feedback yet
    if (currentUe->m_dlSbMcsInfo.empty() ||
        m_mcsCsiSource == NrMacSchedulerUeInfo::McsCsiSource::WIDEBAND_MCS)
    {
        currentRbgPos = *remainingRbgSet.begin();
    }
    else
    {
        // Find the best resource for UE among the available ones
        int maxCqi = 0;
        for (auto resourcePos : remainingRbgSet)
        {
            const auto resourceSb = currentUe->m_rbgToSb.at(resourcePos);
            if (currentUe->m_dlSbMcsInfo.at(resourceSb).cqi > maxCqi)
            {
                currentRbgPos = resourcePos;
                maxCqi = currentUe->m_dlSbMcsInfo.at(resourceSb).cqi;
            }
        }

        // Do not schedule RBGs that are lower than 4 CQI than maximum
        if (!currentUe->m_dlRBG.empty())
        {
            const auto bestCqi =
                currentUe->m_dlSbMcsInfo.at(currentUe->m_rbgToSb.at(currentUe->m_dlRBG.at(0))).cqi;
            if (maxCqi < bestCqi - 4)
            {
                return false;
            }
        }

        // Do not schedule RBGs with sub-band CQI equals to zero
        if (currentRbgPos == std::numeric_limits<uint32_t>::max())
        {
            return false;
        }
    }

    AllocateCurrentResourceToUe(currentUe,
                                currentRbgPos,
                                beamSym,
                                assignedResources,
                                availableRbgs);
    // Save previous tbSize to check if we need to undo this allocation because of a bad
    // MCS
    const auto previousTbSize = currentUe->m_dlTbSize;

    AssignedDlResources(*schedInfoIt, FTResources(beamSym, beamSym), assignedResources);

    // Check if the allocated RBG had a bad MCS and lowered our overall tbsize
    const auto currentTbSize = currentUe->m_dlTbSize;
    if (currentTbSize < previousTbSize * 0.99 && currentUe->GetDlMcs() > 0)
    {
        // Undo allocation
        DeallocateCurrentResourceFromUe(currentUe,
                                        currentRbgPos,
                                        beamSym,
                                        assignedResources,
                                        availableRbgs);

        // Update UE stats to go back to previous state
        AssignedDlResources(*schedInfoIt, FTResources(beamSym, beamSym), assignedResources);
        return false; // Unsuccessful allocation
    }
    remainingRbgSet.erase(currentRbgPos);
    return true; // Successful allocation
}

void
NrMacSchedulerOfdma::DeallocateResourcesDueToFronthaulConstraint(
    const std::vector<UePtrAndBufferReq>& ueVector,
    const uint32_t& beamSym,
    FTResources& assignedResources,
    std::vector<bool>& availableRbgs) const
{
    GetFirst GetUe;
    std::vector<UePtrAndBufferReq> fhUeVector = ueVector;
    auto rng = std::default_random_engine{};
    std::shuffle(std::begin(fhUeVector), std::end(fhUeVector), rng);
    for (auto schedInfoIt : fhUeVector)
    {
        const auto numAssignedResourcesToUe = GetUe(schedInfoIt)->m_dlRBG.size();
        if (numAssignedResourcesToUe > 0) // UEs with an actual allocation
        {
            if (DoesFhAllocationFit(GetBwpId(),
                                    GetUe(schedInfoIt)->GetDlMcs(),
                                    numAssignedResourcesToUe,
                                    GetUe(schedInfoIt)->m_dlRank) == 0)
            {
                // remove allocation if the UE does not fit in the available FH
                // capacity
                while (!schedInfoIt.first->m_dlRBG.empty())
                {
                    uint32_t resourceAssigned = schedInfoIt.first->m_dlRBG.back();
                    DeallocateCurrentResourceFromUe(schedInfoIt.first,
                                                    resourceAssigned,
                                                    beamSym,
                                                    assignedResources,
                                                    availableRbgs);
                }
            }
        }
    }
}

/**
 * @brief Assign the available DL RBG to the UEs
 * @param symAvail Available symbols
 * @param activeDl Map of active UE and their beams
 * @return a map between beams and the symbol they need
 *
 * The algorithm redistributes the frequencies to all the UEs inside a beam.
 * The pre-requisite is to calculate the symbols for each beam, done with
 * the function GetSymPerBeam().
 * The pseudocode is the following (please note that sym_of_beam is a value
 * returned by the GetSymPerBeam() function):
 * <pre>
 * while frequencies > 0:
 *    sort (ueVector);
 *    ueVector.first().m_dlRBG += 1 * sym_of_beam;
 *    frequencies--;
 *    UpdateUeDlMetric (ueVector.first());
 * </pre>
 *
 * To sort the UEs, the method uses the function returned by GetUeCompareDlFn().
 * Two fairness helper are hard-coded in the method: the first one is avoid
 * to assign resources to UEs that already have their buffer requirement covered,
 * and the other one is avoid to assign symbols when all the UEs have their
 * requirements covered.
 */
NrMacSchedulerNs3::BeamSymbolMap
NrMacSchedulerOfdma::AssignDLRBG(uint32_t symAvail, const ActiveUeMap& activeDl) const
{
    NS_LOG_FUNCTION(this);

    NS_LOG_DEBUG("# beams active flows: " << activeDl.size() << ", # sym: " << symAvail);

    GetFirst GetBeamId;
    GetSecond GetUeVector;
    BeamSymbolMap symPerBeam = GetSymPerBeam(symAvail, activeDl);

    // Iterate through the different beams
    for (const auto& el : activeDl)
    {
        // Distribute the RBG evenly among UEs of the same beam
        uint32_t beamSym = symPerBeam.at(GetBeamId(el));
        std::vector<UePtrAndBufferReq> ueVector;
        FTResources assignedResources(0, 0);
        std::vector<bool> availableRbgs = GetDlBitmask();
        std::set<uint32_t> remainingRbgSet;
        for (size_t i = 0; i < availableRbgs.size(); i++)
        {
            if (availableRbgs.at(i))
            {
                remainingRbgSet.emplace(i);
            }
        }

        NS_ASSERT(!remainingRbgSet.empty());

        for (const auto& ue : GetUeVector(el))
        {
            ueVector.emplace_back(ue);
            BeforeDlSched(ueVector.back(), FTResources(beamSym, beamSym));
        }
        bool reapingResources = true;
        while (reapingResources)
        {
            // While there are resources to schedule
            while (!remainingRbgSet.empty())
            {
                // Keep track if resources are being allocated. If not, then stop.
                const auto prevRemaining = remainingRbgSet.size();

                if (m_activeDlAi)
                {
                    CallNotifyDlFn(ueVector);
                }
                // Sort UEs based on the selected scheduler policy (PF, RR, QoS, AI)
                SortUeVector(&ueVector, std::bind(&NrMacSchedulerOfdma::GetUeCompareDlFn, this));

                // Select the first UE
                auto schedInfoIt = ueVector.begin();

                // Advance schedInfoIt iterator to the next UE to schedule
                while (AdvanceToNextUeToSchedule(schedInfoIt, ueVector.end(), beamSym))
                {
                    // Try to allocate the resource to the current UE
                    // If it fails, try again for the next UE
                    if (!AttemptAllocationOfCurrentResourceToUe(schedInfoIt,
                                                                remainingRbgSet,
                                                                beamSym,
                                                                assignedResources,
                                                                availableRbgs))
                    {
                        std::advance(schedInfoIt, 1); // Get the next UE
                        continue;
                    }
                    // Update metrics
                    GetFirst GetUe;
                    NS_LOG_DEBUG("assignedResources "
                                 << GetUe(*schedInfoIt)->m_dlRBG.back() << " DL RBG, spanned over "
                                 << beamSym << " SYM, to UE " << GetUe(*schedInfoIt)->m_rnti);

                    // Update metrics for the unsuccessful UEs (who did not get any resource in this
                    // iteration)
                    for (auto& ue : ueVector)
                    {
                        if (GetUe(ue)->m_rnti != GetUe(*schedInfoIt)->m_rnti)
                        {
                            NotAssignedDlResources(ue,
                                                   FTResources(beamSym, beamSym),
                                                   assignedResources);
                        }
                    }
                    break; // Successful allocation
                }
                // No more UEs to allocate in the current beam
                if (prevRemaining == remainingRbgSet.size())
                {
                    break;
                }
            }

            // If we got here, we either allocated all resources (remainingRbgSet.empty()),
            // or the remaining RBGs do not improve TBS of UEs (prevRemaining ==
            // remainingRbgSet.size()).

            // Now we need to check if there is a UE with less than the minimal TBS.
            std::sort(ueVector.begin(), ueVector.end(), [](auto a, auto b) {
                GetFirst GetUe;
                return GetUe(a)->m_dlTbSize > GetUe(b)->m_dlTbSize;
            });

            // In case there is, reap its resources and redistribute to other UEs at same beam.
            if (!ueVector.empty() && ueVector.back().first->m_dlTbSize < 10)
            {
                auto& ue = ueVector.back();
                while (!ue.first->m_dlRBG.empty())
                {
                    auto reapedRbg = ue.first->m_dlRBG.back();
                    DeallocateCurrentResourceFromUe(ue.first,
                                                    reapedRbg,
                                                    beamSym,
                                                    assignedResources,
                                                    availableRbgs);
                    remainingRbgSet.emplace(reapedRbg);
                }
                // Update DL metrics
                AssignedDlResources(ue, FTResources(beamSym, beamSym), assignedResources);

                // After all resources were reaped, update statistics
                for (auto& uev : ueVector)
                {
                    NotAssignedDlResources(uev, FTResources(beamSym, beamSym), assignedResources);
                }

                // Remove UE from allocation vector (it won't receive more resources in this round)
                ueVector.pop_back();
                continue;
            }
            reapingResources = false;
        }
        if (m_nrFhSchedSapProvider)
        {
            if (m_nrFhSchedSapProvider->GetFhControlMethod() ==
                NrFhControl::FhControlMethod::OptimizeMcs)
            {
                GetFirst GetUe;
                for (auto& schedInfoIt : GetUeVector(el)) // over all UEs with data
                {
                    if (!GetUe(schedInfoIt)->m_dlRBG.empty()) // UEs with an actual allocation
                    {
                        uint8_t maxMcsAssignable = m_nrFhSchedSapProvider->GetMaxMcsAssignable(
                            GetBwpId(),
                            GetUe(schedInfoIt)->m_dlRBG.size(),
                            GetUe(schedInfoIt)->m_rnti,
                            GetUe(schedInfoIt)->m_dlRank); // max MCS index assignable

                        NS_LOG_DEBUG("UE " << GetUe(schedInfoIt)->m_rnti
                                           << " MCS from sched: " << GetUe(schedInfoIt)->GetDlMcs()
                                           << " FH max MCS: " << maxMcsAssignable);

                        GetUe(schedInfoIt)->m_fhMaxMcsAssignable =
                            std::min(GetUe(schedInfoIt)->GetDlMcs(), maxMcsAssignable);
                    }
                }
            }

            if (GetFhControlMethod() == NrFhControl::FhControlMethod::Postponing ||
                GetFhControlMethod() == NrFhControl::FhControlMethod::OptimizeMcs ||
                GetFhControlMethod() == NrFhControl::FhControlMethod::OptimizeRBs)
            {
                DeallocateResourcesDueToFronthaulConstraint(ueVector,
                                                            beamSym,
                                                            assignedResources,
                                                            availableRbgs);
            }
        }
    }

    return symPerBeam;
}

NrMacSchedulerNs3::BeamSymbolMap
NrMacSchedulerOfdma::AssignULRBG(uint32_t symAvail, const ActiveUeMap& activeUl) const
{
    NS_LOG_FUNCTION(this);

    NS_LOG_DEBUG("# beams active flows: " << activeUl.size() << ", # sym: " << symAvail);

    GetFirst GetBeamId;
    GetSecond GetUeVector;
    BeamSymbolMap symPerBeam = GetSymPerBeam(symAvail, activeUl);

    // Iterate through the different beams
    for (const auto& el : activeUl)
    {
        // Distribute the RBG evenly among UEs of the same beam
        uint32_t beamSym = symPerBeam.at(GetBeamId(el));
        std::vector<UePtrAndBufferReq> ueVector;
        FTResources assigned(0, 0);

        const std::vector<bool> availableRbgs = GetUlBitmask();
        std::set<uint32_t> remainingRbgSet;
        for (size_t i = 0; i < availableRbgs.size(); i++)
        {
            if (availableRbgs.at(i))
            {
                remainingRbgSet.emplace(i);
            }
        }

        NS_ASSERT(!remainingRbgSet.empty());

        for (const auto& ue : GetUeVector(el))
        {
            ueVector.emplace_back(ue);
        }

        for (auto& ue : ueVector)
        {
            BeforeUlSched(ue, FTResources(beamSym * beamSym, beamSym));
        }

        while (!remainingRbgSet.empty())
        {
            if (m_activeUlAi)
            {
                CallNotifyUlFn(ueVector);
            }
            GetFirst GetUe;
            SortUeVector(&ueVector, std::bind(&NrMacSchedulerOfdma::GetUeCompareUlFn, this));
            auto schedInfoIt = ueVector.begin();

            // Ensure fairness: pass over UEs which already has enough resources to transmit
            while (schedInfoIt != ueVector.end())
            {
                uint32_t bufQueueSize = schedInfoIt->second;
                if (GetUe(*schedInfoIt)->m_ulTbSize >= std::max(bufQueueSize, 12U))
                {
                    std::advance(schedInfoIt, 1);
                }
                else
                {
                    break;
                }
            }

            // In the case that all the UE already have their requirements fulfilled,
            // then stop the beam processing and pass to the next
            if (schedInfoIt == ueVector.end())
            {
                break;
            }

            auto assignedRbg = remainingRbgSet.begin();
            // Assign 1 RBG for each available symbols for the beam,
            // and then update the count of available resources
            auto& assignedRbgs = GetUe(*schedInfoIt)->m_ulRBG;
            auto existingRbgs = assignedRbgs.size();
            assignedRbgs.resize(assignedRbgs.size() + beamSym);
            std::fill(assignedRbgs.begin() + existingRbgs, assignedRbgs.end(), *assignedRbg);
            assigned.m_rbg++;

            auto& assignedSymbols = GetUe(*schedInfoIt)->m_ulSym;
            auto existingSymbols = assignedSymbols.size();
            assignedSymbols.resize(assignedSymbols.size() + beamSym);
            std::iota(assignedSymbols.begin() + existingSymbols, assignedSymbols.end(), 0);
            assigned.m_sym = beamSym;

            remainingRbgSet.erase(
                assignedRbg); // Resources are RBG, so they do not consider the beamSym

            // Update metrics
            NS_LOG_DEBUG("Assigned " << assigned.m_rbg << " UL RBG, spanned over " << beamSym
                                     << " SYM, to UE " << GetUe(*schedInfoIt)->m_rnti);
            AssignedUlResources(*schedInfoIt, FTResources(beamSym, beamSym), assigned);

            // Update metrics for the unsuccessful UEs (who did not get any resource in this
            // iteration)
            for (auto& ue : ueVector)
            {
                if (GetUe(ue)->m_rnti != GetUe(*schedInfoIt)->m_rnti)
                {
                    NotAssignedUlResources(ue, FTResources(beamSym, beamSym), assigned);
                }
            }
        }
    }

    return symPerBeam;
}

/**
 * @brief Create the DL DCI in OFDMA mode
 * @param spoint Starting point
 * @param ueInfo UE representation
 * @param maxSym Maximum symbols to use
 * @return a pointer to the newly created instance
 *
 * The function calculates the TBS and then call CreateDci().
 */
std::shared_ptr<DciInfoElementTdma>
NrMacSchedulerOfdma::CreateDlDci(NrMacSchedulerNs3::PointInFTPlane* spoint,
                                 const std::shared_ptr<NrMacSchedulerUeInfo>& ueInfo,
                                 uint32_t maxSym) const
{
    NS_LOG_FUNCTION(this);

    auto dlMcs = ueInfo->GetDlMcs();
    ueInfo->m_fhMaxMcsAssignable.reset(); // Erase value assigned when fronthaul control is enabled
    uint32_t tbs = m_dlAmc->CalculateTbSize(dlMcs,
                                            ueInfo->m_dlRank,
                                            ueInfo->m_dlRBG.size() * GetNumRbPerRbg());
    // NS_ASSERT_MSG(ueInfo->m_dlRBG.size() % maxSym == 0,
    //               " MaxSym " << maxSym << " RBG: " << ueInfo->m_dlRBG.size());
    NS_ASSERT(ueInfo->m_dlRBG.size() <= maxSym * GetBandwidthInRbg());
    NS_ASSERT(spoint->m_rbg < GetBandwidthInRbg());
    NS_ASSERT(maxSym <= UINT8_MAX);

    // 5 bytes for headers (3 mac header, 2 rlc header)
    if (tbs < 10)
    {
        NS_LOG_DEBUG("While creating DCI for UE "
                     << ueInfo->m_rnti << " assigned "
                     << std::set<uint32_t>(ueInfo->m_dlRBG.begin(), ueInfo->m_dlRBG.end()).size()
                     << " DL RBG, but TBS < 10");
        ueInfo->m_dlTbSize = 0;
        return nullptr;
    }

    const auto rbgBitmask = CreateRbgBitmaskFromAllocatedRbgs(ueInfo->m_dlRBG);
    std::ostringstream oss;
    for (const auto& x : rbgBitmask)
    {
        oss << std::to_string(x) << " ";
    }

    NS_LOG_INFO("UE " << ueInfo->m_rnti << " assigned RBG from " << spoint->m_rbg << " with mask "
                      << oss.str() << " for " << static_cast<uint32_t>(maxSym) << " SYM.");

    std::shared_ptr<DciInfoElementTdma> dci =
        std::make_shared<DciInfoElementTdma>(ueInfo->m_rnti,
                                             DciInfoElementTdma::DL,
                                             spoint->m_sym,
                                             maxSym,
                                             dlMcs,
                                             ueInfo->m_dlRank,
                                             ueInfo->m_dlPrecMats,
                                             tbs,
                                             1,
                                             0,
                                             DciInfoElementTdma::DATA,
                                             GetBwpId(),
                                             GetTpc());

    dci->m_rbgBitmask = std::move(rbgBitmask);

    NS_ASSERT(std::count(dci->m_rbgBitmask.begin(), dci->m_rbgBitmask.end(), 0) !=
              GetBandwidthInRbg());

    return dci;
}

std::shared_ptr<DciInfoElementTdma>
NrMacSchedulerOfdma::CreateUlDci(PointInFTPlane* spoint,
                                 const std::shared_ptr<NrMacSchedulerUeInfo>& ueInfo,
                                 uint32_t maxSym) const
{
    NS_LOG_FUNCTION(this);

    uint32_t tbs = m_ulAmc->CalculateTbSize(ueInfo->m_ulMcs,
                                            ueInfo->m_ulRank,
                                            ueInfo->m_ulRBG.size() * GetNumRbPerRbg());

    // If is less than 12, i.e., 7 (3 mac header, 2 rlc header, 2 data) + 5 bytes for
    // the SHORT_BSR. then we can't transmit any new data, so don't create dci.
    if (tbs < 12)
    {
        NS_LOG_DEBUG("While creating UL DCI for UE "
                     << ueInfo->m_rnti << " assigned "
                     << std::set<uint32_t>(ueInfo->m_ulRBG.begin(), ueInfo->m_ulRBG.end()).size()
                     << " UL RBG, but TBS < 12");
        return nullptr;
    }

    uint32_t RBGNum = ueInfo->m_ulRBG.size() / maxSym;
    const auto rbgBitmask = CreateRbgBitmaskFromAllocatedRbgs(ueInfo->m_ulRBG);

    NS_LOG_INFO("UE " << ueInfo->m_rnti << " assigned RBG from " << spoint->m_rbg << " to "
                      << spoint->m_rbg + RBGNum << " for " << static_cast<uint32_t>(maxSym)
                      << " SYM.");

    NS_ASSERT(spoint->m_sym >= maxSym);
    std::shared_ptr<DciInfoElementTdma> dci =
        std::make_shared<DciInfoElementTdma>(ueInfo->m_rnti,
                                             DciInfoElementTdma::UL,
                                             spoint->m_sym - maxSym,
                                             maxSym,
                                             ueInfo->m_ulMcs,
                                             ueInfo->m_ulRank,
                                             ueInfo->m_ulPrecMats,
                                             tbs,
                                             1,
                                             0,
                                             DciInfoElementTdma::DATA,
                                             GetBwpId(),
                                             GetTpc());

    dci->m_rbgBitmask = std::move(rbgBitmask);

    std::ostringstream oss;
    for (auto x : dci->m_rbgBitmask)
    {
        oss << std::to_string(x) << " ";
    }
    NS_LOG_INFO("UE " << ueInfo->m_rnti << " DCI RBG mask: " << oss.str());

    NS_ASSERT(std::count(dci->m_rbgBitmask.begin(), dci->m_rbgBitmask.end(), 0) !=
              GetBandwidthInRbg());

    return dci;
}

void
NrMacSchedulerOfdma::ChangeDlBeam(PointInFTPlane* spoint, uint32_t symOfBeam) const
{
    spoint->m_rbg = 0;
    spoint->m_sym += symOfBeam;
}

void
NrMacSchedulerOfdma::ChangeUlBeam(PointInFTPlane* spoint, uint32_t symOfBeam) const
{
    spoint->m_rbg = 0;
    spoint->m_sym -= symOfBeam;
}

uint8_t
NrMacSchedulerOfdma::GetTpc() const
{
    NS_LOG_FUNCTION(this);
    return 1; // 1 is mapped to 0 for Accumulated mode, and to -1 in Absolute mode TS38.213
              // Table 7.1.1-1
}

std::vector<bool>
NrMacSchedulerOfdma::CreateRbgBitmaskFromAllocatedRbgs(
    const std::vector<uint16_t>& allocatedRbgs) const
{
    std::vector<bool> rbgNotchedBitmask = GetDlNotchedRbgMask();
    if (rbgNotchedBitmask.empty())
    {
        rbgNotchedBitmask = std::vector<bool>(GetBandwidthInRbg(), true);
    }
    std::vector<bool> rbgBitmask = std::vector<bool>(GetBandwidthInRbg(), false);

    NS_ASSERT(rbgNotchedBitmask.size() == rbgBitmask.size());

    // rbgBitmask is all 1s or have 1s in the place we are allowed to transmit.

    for (auto rbg : allocatedRbgs)
    {
        NS_ASSERT_MSG(rbgNotchedBitmask.at(rbg), "Scheduled notched resource");
        rbgBitmask.at(rbg) = true;
    }
    return rbgBitmask;
}

} // namespace ns3
