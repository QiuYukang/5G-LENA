// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#define NS_LOG_APPEND_CONTEXT                                                                      \
    do                                                                                             \
    {                                                                                              \
        std::clog << " [ CellId " << GetCellId() << ", bwpId " << GetBwpId() << "] ";              \
    } while (false);
#include "nr-mac-scheduler-harq-rr.h"

#include "nr-fh-control.h"

#include "ns3/boolean.h"
#include "ns3/log.h"

#include <algorithm>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrMacSchedulerHarqRr");
NS_OBJECT_ENSURE_REGISTERED(NrMacSchedulerHarqRr);

TypeId
NrMacSchedulerHarqRr::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrMacSchedulerHarqRr")
            .SetParent<Object>()
            .AddAttribute("ConsolidateHarqRetx",
                          "Consolidate HARQ DCI through reshaping to improve resource utilization",
                          BooleanValue(false),
                          MakeBooleanAccessor(&NrMacSchedulerHarqRr::m_consolidateHarqRetx),
                          MakeBooleanChecker());
    return tid;
}

NrMacSchedulerHarqRr::NrMacSchedulerHarqRr()
{
}

void
NrMacSchedulerHarqRr::InstallGetBwpIdFn(const std::function<uint16_t()>& fn)
{
    m_getBwpId = fn;
}

void
NrMacSchedulerHarqRr::InstallGetCellIdFn(const std::function<uint16_t()>& fn)
{
    m_getCellId = fn;
}

void
NrMacSchedulerHarqRr::InstallGetBwInRBG(const std::function<uint16_t()>& fn)
{
    m_getBwInRbg = fn;
}

void
NrMacSchedulerHarqRr::InstallGetFhControlMethodFn(const std::function<uint8_t()>& fn)
{
    m_getFhControlMethod = fn;
}

void
NrMacSchedulerHarqRr::InstallDoesFhAllocationFitFn(
    const std::function<bool(uint16_t bwpId, uint32_t mcs, uint32_t nRegs, uint8_t dlRank)>& fn)
{
    m_getDoesAllocationFit = fn;
}

void
NrMacSchedulerHarqRr::InstallReshapeAllocation(
    const std::function<
        const std::vector<DciInfoElementTdma>(const std::vector<DciInfoElementTdma>& dcis,
                                              uint8_t& startingSymbol,
                                              uint8_t& numSymbols,
                                              std::vector<bool>& bitmask,
                                              const bool isDl)>& fn)
{
    m_getReshapeAllocation = fn;
}

std::vector<BeamId>
NrMacSchedulerHarqRr::GetBeamOrderRR(NrMacSchedulerNs3::ActiveHarqMap activeHarqMap) const
{
    std::vector<BeamId> ret(activeHarqMap.size());

    for (const auto& el : activeHarqMap)
    {
        // Add new beams to the round-robin queue
        if (m_rrBeamsSet.find(el.first) == m_rrBeamsSet.end())
        {
            m_rrBeams.push_back(el.first);
            m_rrBeamsSet.insert(el.first);
        }
    }

    // Find first active beam in the round-robin queue
    for (size_t i = 0; i < m_rrBeams.size(); ++i)
    {
        // If front beam in round-robin queue is active,
        // put it at the beginning of the order
        if (activeHarqMap.find(m_rrBeams.front()) != activeHarqMap.end())
        {
            ret[i] = m_rrBeams.front();
        }
        // Move round-robin front queue item to the end
        m_rrBeams.push_back(m_rrBeams.front());
        m_rrBeams.pop_front();
    }
    return ret;
}

std::ostream&
operator<<(std::ostream& os, const std::vector<bool>& h)
{
    for (auto hb : h)
    {
        os << (int)hb;
    }
    return os;
}

/**
 * @brief Schedule DL HARQ in RR fashion
 * @param startingPoint starting point of the first retransmission.
 * @param symAvail Available symbols
 * @param activeDlHarq Map of the active HARQ processes
 * @param ueMap Map of the UEs
 * @param dlHarqToRetransmit HARQ feedbacks that could not be transmitted (to fill)
 * @param dlHarqFeedback all the HARQ feedbacks
 * @param slotAlloc Slot allocation info
 * @return the VarTtiSlotAlloc ID to use next
 *
 * The algorithm is a bit complex, but nothing special. The HARQ should be
 * placed in 2D space as they were before. Probably there is an error in the algorithm.
 */
uint8_t
NrMacSchedulerHarqRr::ScheduleDlHarq(
    NrMacSchedulerNs3::PointInFTPlane* startingPoint,
    uint8_t symAvail,
    const Ns3Sched::ActiveHarqMap& activeDlHarq,
    const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& ueMap,
    std::vector<DlHarqInfo>* dlHarqToRetransmit,
    const std::vector<DlHarqInfo>& dlHarqFeedback,
    SlotAllocInfo* slotAlloc) const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(startingPoint->m_rbg == 0);
    const auto preexistingDciNum = slotAlloc->m_varTtiAllocInfo.size();
    auto currStartingSymbol = startingPoint->m_sym;

    const bool isDl = true;
    auto dlBitmask = m_getDlBitmask();
    NS_LOG_INFO("We have " << activeDlHarq.size() << " beams with data to RETX");
    for (const auto beamId : GetBeamOrderRR(activeDlHarq))
    {
        const auto& beam = *activeDlHarq.find(beamId);
        const auto preexistingDciNumToBeam = slotAlloc->m_varTtiAllocInfo.size();
        auto beamStartingSymbol = currStartingSymbol;
        std::vector<uint16_t> allocatedUe;
        NS_LOG_INFO(" Try to assign HARQ resource for Beam sector: "
                    << static_cast<uint32_t>(beam.first.GetSector())
                    << " Beam theta:  " << static_cast<uint32_t>(beam.first.GetElevation())
                    << " # HARQ to Retx=" << beam.second.size());

        for (auto it : beam.second)
        {
            HarqProcess& harqProcess = it->second;
            NS_ASSERT_MSG(harqProcess.m_status == HarqProcess::RECEIVED_FEEDBACK,
                          "Process " << static_cast<uint32_t>(it->first)
                                     << " is not in RECEIVED_FEEDBACK status");

            harqProcess.m_status = HarqProcess::WAITING_FEEDBACK;
            harqProcess.m_timer = 0;

            auto& dciInfoReTx = harqProcess.m_dciElement;

            uint32_t rbgAssigned =
                std::count(dciInfoReTx->m_rbgBitmask.begin(), dciInfoReTx->m_rbgBitmask.end(), 1) *
                dciInfoReTx->m_numSym;
            uint32_t rbgAvail = (GetBandwidthInRbg() - startingPoint->m_rbg) * symAvail;

            NS_LOG_INFO("Evaluating space to retransmit HARQ PID="
                        << static_cast<uint32_t>(dciInfoReTx->m_harqProcess) << " for UE="
                        << static_cast<uint32_t>(dciInfoReTx->m_rnti) << " SYM assigned previously="
                        << static_cast<uint32_t>(dciInfoReTx->m_numSym)
                        << " RBG assigned previously=" << static_cast<uint32_t>(rbgAssigned)
                        << " SYM avail=" << static_cast<uint32_t>(symAvail)
                        << " RBG avail for this beam=" << rbgAvail);

            if (std::find(allocatedUe.begin(), allocatedUe.end(), dciInfoReTx->m_rnti) !=
                allocatedUe.end())
            {
                NS_LOG_INFO("UE " << dciInfoReTx->m_rnti
                                  << " already has an HARQ allocated, buffer this HARQ process"
                                  << static_cast<uint32_t>(dciInfoReTx->m_harqProcess));
                BufferHARQFeedback(dlHarqFeedback,
                                   dlHarqToRetransmit,
                                   dciInfoReTx->m_rnti,
                                   dciInfoReTx->m_harqProcess);
                continue;
            }
            else if (rbgAvail < rbgAssigned)
            {
                NS_LOG_INFO("No resource for this retx, we have to buffer it");
                BufferHARQFeedback(dlHarqFeedback,
                                   dlHarqToRetransmit,
                                   dciInfoReTx->m_rnti,
                                   dciInfoReTx->m_harqProcess);
                continue;
            }
            if (GetFromSchedFhControlMethod() == NrFhControl::FhControlMethod::Postponing ||
                GetFromSchedFhControlMethod() == NrFhControl::FhControlMethod::OptimizeMcs ||
                GetFromSchedFhControlMethod() == NrFhControl::FhControlMethod::OptimizeRBs)
            {
                if (GetDoesFhAllocationFit(GetBwpId(),
                                           dciInfoReTx->m_mcs,
                                           rbgAssigned,
                                           dciInfoReTx->m_rank) == 0)
                {
                    NS_LOG_INFO("No FH resources for this retx, we have to buffer it");
                    BufferHARQFeedback(dlHarqFeedback,
                                       dlHarqToRetransmit,
                                       dciInfoReTx->m_rnti,
                                       dciInfoReTx->m_harqProcess);
                    continue;
                }
            }

            // Pass copies, not to have to commit to any changes
            uint8_t symAvailBackup = symAvail;
            auto dlBitmaskBackup = dlBitmask;
            auto currStartingSymbolBackup = currStartingSymbol;
            std::vector<DciInfoElementTdma> reshapedDcis;
            if (m_consolidateHarqRetx)
            {
                reshapedDcis = m_getReshapeAllocation({*harqProcess.m_dciElement},
                                                      currStartingSymbolBackup,
                                                      symAvailBackup,
                                                      dlBitmaskBackup,
                                                      isDl);
            }
            else
            {
                // If not reshaping, we just change at most the starting symbol.
                // But first we check if there are collisions.
                symAvailBackup -= harqProcess.m_dciElement->m_numSym;
                bool collision = false;
                for (std::size_t i = 0; i < dlBitmaskBackup.size(); i++)
                {
                    if (harqProcess.m_dciElement->m_rbgBitmask.at(i))
                    {
                        if (!dlBitmaskBackup.at(i))
                        {
                            collision = true;
                            break;
                        }
                        dlBitmaskBackup.at(i) = false;
                    }
                }
                if (!collision)
                {
                    reshapedDcis.emplace_back(currStartingSymbolBackup,
                                              harqProcess.m_dciElement->m_numSym,
                                              harqProcess.m_dciElement->m_rbgBitmask,
                                              *harqProcess.m_dciElement);
                }
            }

            // If allocation reshaping did not work, buffer DCI
            if (reshapedDcis.empty())
            {
                NS_LOG_INFO("This HARQ allocation collides with a previously allocated HARQ, "
                            "we have to buffer it");
                BufferHARQFeedback(dlHarqFeedback,
                                   dlHarqToRetransmit,
                                   dciInfoReTx->m_rnti,
                                   dciInfoReTx->m_harqProcess);
                continue;
            }

            // This code currently only passes one DCI at a time for reshaping, while the
            // function actually supports multiple. So here we only consume the first element.
            harqProcess.m_dciElement = std::make_shared<DciInfoElementTdma>(reshapedDcis.front());
            dciInfoReTx = harqProcess.m_dciElement;
            auto numSymbols = dciInfoReTx->m_numSym;
            if (symAvail < numSymbols)
            {
                NS_LOG_INFO("No symbols available for this HARQ allocation, we have to buffer it");
                BufferHARQFeedback(dlHarqFeedback,
                                   dlHarqToRetransmit,
                                   dciInfoReTx->m_rnti,
                                   dciInfoReTx->m_harqProcess);
                continue;
            }

            // Commit changes made to number of symbols, RBG bitmask and starting symbol
            // during reshaping
            symAvail = symAvailBackup;
            dlBitmask = dlBitmaskBackup;
            currStartingSymbol = currStartingSymbolBackup;
            allocatedUe.push_back(dciInfoReTx->m_rnti);

            NS_ASSERT(dciInfoReTx->m_format == DciInfoElementTdma::DL);
            auto dci = std::make_shared<DciInfoElementTdma>(dciInfoReTx->m_rnti,
                                                            dciInfoReTx->m_format,
                                                            dciInfoReTx->m_symStart,
                                                            dciInfoReTx->m_numSym,
                                                            dciInfoReTx->m_mcs,
                                                            dciInfoReTx->m_rank,
                                                            dciInfoReTx->m_precMats,
                                                            dciInfoReTx->m_tbSize,
                                                            0,
                                                            dciInfoReTx->m_rv + 1,
                                                            DciInfoElementTdma::DATA,
                                                            dciInfoReTx->m_bwpIndex,
                                                            dciInfoReTx->m_tpc);

            dci->m_rbgBitmask = harqProcess.m_dciElement->m_rbgBitmask;
            dci->m_harqProcess = dciInfoReTx->m_harqProcess;

            harqProcess.m_dciElement = dci;
            dciInfoReTx = harqProcess.m_dciElement;

            VarTtiAllocInfo slotInfo(dciInfoReTx);
            NS_LOG_DEBUG(
                "UE" << dciInfoReTx->m_rnti << " gets DL symbols "
                     << static_cast<uint32_t>(dciInfoReTx->m_symStart) << "-"
                     << static_cast<uint32_t>(dciInfoReTx->m_symStart + dciInfoReTx->m_numSym - 1)
                     << " tbs " << dciInfoReTx->m_tbSize << " harqId "
                     << static_cast<uint32_t>(dciInfoReTx->m_harqProcess) << " rv "
                     << static_cast<uint32_t>(dciInfoReTx->m_rv) << " RETX on RBGs"
                     << dciInfoReTx->m_rbgBitmask);
            for (const auto& rlcPdu : harqProcess.m_rlcPduInfo)
            {
                slotInfo.m_rlcPduInfo.push_back(rlcPdu);
            }
            slotAlloc->m_varTtiAllocInfo.push_back(slotInfo);
            ueMap.find(dciInfoReTx->m_rnti)->second->m_dlMRBRetx =
                dciInfoReTx->m_numSym * rbgAssigned;
        }
        // If there are still symbols left for the next beam, reset RBG mask
        if (symAvail > 0)
        {
            dlBitmask = m_getDlBitmask();
            // Advance symbol for OFDMA to prevent overlapping allocations with different beams
            if (beamStartingSymbol == currStartingSymbol)
            {
                auto symbolsUsedForBeam = nr::CountUsedSymbolsFromVarAllocTtiRange(
                    startingPoint->m_sym,
                    slotAlloc->m_varTtiAllocInfo.begin() + preexistingDciNumToBeam,
                    slotAlloc->m_varTtiAllocInfo.end());
                currStartingSymbol += symbolsUsedForBeam;
                symAvail -= symbolsUsedForBeam;
            }
        }
    }
    NS_ASSERT(startingPoint->m_rbg == 0);

    auto usedSymbols = nr::CountUsedSymbolsFromVarAllocTtiRange(
        startingPoint->m_sym,
        slotAlloc->m_varTtiAllocInfo.begin() + preexistingDciNum,
        slotAlloc->m_varTtiAllocInfo.end());
    return usedSymbols;
}

/**
 * @brief Schedule the UL HARQ
 * @param startingPoint starting point of the first retransmission.
 * It should be set to the next available starting point
 * @param symAvail Available symbols
 * @param ueMap Map of the UEs
 * @param ulHarqToRetransmit HARQ feedbacks that could not be transmitted (to fill)
 * @param ulHarqFeedback all the HARQ feedbacks
 * @param slotAlloc Slot allocation info
 * @return the VarTtiSlotAlloc ID to use next
 *
 * The algorithm for scheduling the UL HARQ is straightforward. Since the UL
 * transmission are all TDMA, for each NACKed process a DCI is built, with
 * the exact same specification as the first transmission. If there aren't
 * available symbols to retransmit the data, the feedback is buffered for
 * the next slot.
 */
uint8_t
NrMacSchedulerHarqRr::ScheduleUlHarq(
    NrMacSchedulerNs3::PointInFTPlane* startingPoint,
    uint8_t symAvail,
    const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& ueMap,
    std::vector<UlHarqInfo>* ulHarqToRetransmit,
    const std::vector<UlHarqInfo>& ulHarqFeedback,
    SlotAllocInfo* slotAlloc) const
{
    NS_LOG_FUNCTION(this);
    uint8_t symUsed = 0;
    NS_ASSERT(startingPoint->m_rbg == 0);

    NS_LOG_INFO("Scheduling UL HARQ starting from sym "
                << +startingPoint->m_sym << " and RBG " << startingPoint->m_rbg
                << ". Available symbols: " << symAvail
                << " number of feedback: " << ulHarqFeedback.size());

    for (uint16_t i = 0; i < ulHarqFeedback.size() && symAvail > 0; i++)
    {
        const UlHarqInfo& harqInfo = ulHarqFeedback.at(i);
        uint8_t harqId = harqInfo.m_harqProcessId;
        uint16_t rnti = harqInfo.m_rnti;

        NS_ABORT_IF(harqInfo.IsReceivedOk());

        // retx correspondent block: retrieve the UL-DCI
        HarqProcess& harqProcess = ueMap.find(rnti)->second->m_ulHarq.Find(harqId)->second;
        NS_ASSERT(harqProcess.m_status == HarqProcess::RECEIVED_FEEDBACK);

        harqProcess.m_status = HarqProcess::WAITING_FEEDBACK;
        harqProcess.m_timer = 0;
        auto& dciInfoReTx = harqProcess.m_dciElement;

        NS_LOG_INFO("Feedback is for UE " << rnti << " process " << +harqId
                                          << " sym: " << +dciInfoReTx->m_numSym);

        if (symAvail >= dciInfoReTx->m_numSym)
        {
            symAvail -= dciInfoReTx->m_numSym;
            symUsed += dciInfoReTx->m_numSym;

            NS_ASSERT(dciInfoReTx->m_format == DciInfoElementTdma::UL);

            auto dci =
                std::make_shared<DciInfoElementTdma>(dciInfoReTx->m_rnti,
                                                     dciInfoReTx->m_format,
                                                     startingPoint->m_sym - dciInfoReTx->m_numSym,
                                                     dciInfoReTx->m_numSym,
                                                     dciInfoReTx->m_mcs,
                                                     dciInfoReTx->m_rank,
                                                     dciInfoReTx->m_precMats,
                                                     dciInfoReTx->m_tbSize,
                                                     0,
                                                     dciInfoReTx->m_rv + 1,
                                                     DciInfoElementTdma::DATA,
                                                     dciInfoReTx->m_bwpIndex,
                                                     dciInfoReTx->m_tpc);
            dci->m_rbgBitmask = harqProcess.m_dciElement->m_rbgBitmask;
            dci->m_harqProcess = harqId;
            harqProcess.m_dciElement = dci;
            dciInfoReTx = harqProcess.m_dciElement;

            startingPoint->m_sym -= dciInfoReTx->m_numSym;

            VarTtiAllocInfo slotInfo(dciInfoReTx);
            NS_LOG_DEBUG(
                "UE" << dciInfoReTx->m_rnti << " gets UL symbols "
                     << static_cast<uint32_t>(dciInfoReTx->m_symStart) << "-"
                     << static_cast<uint32_t>(dciInfoReTx->m_symStart + dciInfoReTx->m_numSym - 1)
                     << " tbs " << dciInfoReTx->m_tbSize << " harqId "
                     << static_cast<uint32_t>(dciInfoReTx->m_harqProcess) << " rv "
                     << static_cast<uint32_t>(dciInfoReTx->m_rv) << " RETX");
            slotAlloc->m_varTtiAllocInfo.push_front(slotInfo);
            slotAlloc->m_numSymAlloc += dciInfoReTx->m_numSym;

            ueMap.find(rnti)->second->m_ulMRBRetx = dciInfoReTx->m_numSym * GetBandwidthInRbg();
        }
        else
        {
            ulHarqToRetransmit->push_back(ulHarqFeedback.at(i));
        }
    }

    NS_ASSERT(startingPoint->m_rbg == 0);

    return symUsed;
}

/**
 * @brief Sort Dl Harq retx based on their symbol requirement
 * @param activeDlHarq map of the active retx
 */
void
NrMacSchedulerHarqRr::SortDlHarq(NrMacSchedulerNs3::ActiveHarqMap* activeDlHarq) const
{
    NS_LOG_FUNCTION(this);

    // Order based on required sym
    static struct
    {
        bool operator()(const NrMacSchedulerNs3::HarqVectorIterator& a,
                        const NrMacSchedulerNs3::HarqVectorIterator& b) const
        {
            return a->second.m_dciElement->m_numSym > b->second.m_dciElement->m_numSym;
        }
    } CompareNumSym;

    for (auto& it : *activeDlHarq)
    {
        std::stable_sort(it.second.begin(), it.second.end(), CompareNumSym);
    }
}

/**
 * @brief (In theory) sort UL HARQ retx
 * @param activeUlHarq map of the active retx
 *
 * Since in the uplink we are still TDMA, there is no need of sorting
 * the HARQ. The HARQ will be picked one by one until there are no
 * available symbol to transmit, and what is not transmitted will be queued
 * for the next slot.
 */
void
NrMacSchedulerHarqRr::SortUlHarq(
    [[maybe_unused]] NrMacSchedulerNs3::ActiveHarqMap* activeUlHarq) const
{
    NS_LOG_FUNCTION(this);
}

/**
 * @brief Find the specified HARQ process and buffer it into a vector
 * @param dlHarqFeedback HARQ not retransmitted list
 * @param dlHarqToRetransmit HARQ buffer list (to retransmit)
 * @param rnti RNTI to find
 * @param harqProcess process ID to find
 */
void
NrMacSchedulerHarqRr::BufferHARQFeedback(const std::vector<DlHarqInfo>& dlHarqFeedback,
                                         std::vector<DlHarqInfo>* dlHarqToRetransmit,
                                         uint16_t rnti,
                                         uint8_t harqProcess) const
{
    NS_LOG_FUNCTION(this);

    bool found = false;
    for (const auto& feedback : dlHarqFeedback)
    {
        if (feedback.m_rnti == rnti && feedback.m_harqProcessId == harqProcess)
        {
            dlHarqToRetransmit->push_back(feedback);
            found = true;
            break;
        }
    }
    NS_ASSERT(found);
}

uint16_t
NrMacSchedulerHarqRr::GetBwpId() const
{
    return m_getBwpId();
}

uint16_t
NrMacSchedulerHarqRr::GetCellId() const
{
    return m_getCellId();
}

uint16_t
NrMacSchedulerHarqRr::GetBandwidthInRbg() const
{
    return m_getBwInRbg();
}

uint8_t
NrMacSchedulerHarqRr::GetFromSchedFhControlMethod() const
{
    return m_getFhControlMethod();
}

bool
NrMacSchedulerHarqRr::GetDoesFhAllocationFit(uint16_t bwpId,
                                             uint32_t mcs,
                                             uint32_t nRegs,
                                             uint8_t dlRank) const
{
    return m_getDoesAllocationFit(bwpId, mcs, nRegs, dlRank);
}

void
NrMacSchedulerHarqRr::InstallGetDlBitmask(const std::function<std::vector<bool>()>& fn)
{
    m_getDlBitmask = fn;
}

void
NrMacSchedulerHarqRr::InstallGetUlBitmask(const std::function<std::vector<bool>()>& fn)
{
    m_getUlBitmask = fn;
}

} // namespace ns3
