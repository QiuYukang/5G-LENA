// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#ifndef NR_MAC_SCHEDULER_OFDMA_SYMBOL_PER_BEAM_H
#define NR_MAC_SCHEDULER_OFDMA_SYMBOL_PER_BEAM_H

#include "nr-mac-scheduler-ns3.h"
#include "nr-mac-scheduler-ofdma.h"

#include "ns3/log.h"
#include "ns3/type-id.h"

#include <unordered_set>

namespace ns3
{

/**
 * @ingroup scheduler
 * @brief The base for all the OFDMA symbols-per-beam schedulers
 *
 * The OFDMA scheduler first schedule symbols for the active beams,
 * before starting scheduling RBGs of each beam to the UEs in that beam.
 *
 * GetSymPerBeam() receives the number of symbols available to be distributed
 * per beam, and the list of active beams and its UEs is part of the activeDl map.
 *
 * There are multiple implementations of symbols-per-beam allocators,
 * implementing different policies.
 *
 * @see NrMacSchedulerOfdmaSymbolPerBeamLB
 * @see NrMacSchedulerOfdmaSymbolPerBeamRR
 * @see NrMacSchedulerOfdmaSymbolPerBeamPF
 */
class NrMacSchedulerOfdmaSymbolPerBeam : public Object
{
  public:
    NrMacSchedulerOfdmaSymbolPerBeam() = default;
    static TypeId GetTypeId();
    virtual NrMacSchedulerNs3::BeamSymbolMap GetSymPerBeam(
        uint32_t symAvail,
        const NrMacSchedulerNs3::ActiveUeMap& activeDl) const = 0;
};

class NrMacSchedulerOfdmaSymbolPerBeamLB : public NrMacSchedulerOfdmaSymbolPerBeam
{
  public:
    NrMacSchedulerOfdmaSymbolPerBeamLB() = default;
    static TypeId GetTypeId();
    /**
     * @brief Calculate the number of symbols to assign to each beam based on beam's UEs buffer load
     * @param symAvail Number of available symbols
     * @param activeDl Map of active DL UE and their beam
     * @return symbols per beam allocation map
     * Each beam has a different requirement in terms of byte that should be
     * transmitted with that beam. That requirement depends on the number of UE
     * that are inside such beam, and how many bytes they have to transmit.
     *
     * For the beam \f$ b \f$, the number of assigned symbols is the following:
     *
     * \f$ sym_{b} = BufSize(b) * \frac{symAvail}{BufSizeTotal} \f$
     */
    NrMacSchedulerNs3::BeamSymbolMap GetSymPerBeam(
        uint32_t symAvail,
        const NrMacSchedulerNs3::ActiveUeMap& activeDl) const override;
};

class NrMacSchedulerOfdmaSymbolPerBeamRR : public NrMacSchedulerOfdmaSymbolPerBeam
{
  public:
    NrMacSchedulerOfdmaSymbolPerBeamRR() = default;
    static TypeId GetTypeId();

    /**
     * @brief Allocate all symbols to the first active beam in the round-robin queue
     * @param symAvail Number of available symbols
     * @param activeDl Map of active DL UE and their beam
     * @return symbols per beam allocation map
     */
    NrMacSchedulerNs3::BeamSymbolMap GetSymPerBeam(
        uint32_t symAvail,
        const NrMacSchedulerNs3::ActiveUeMap& activeDl) const override;

  private:
    mutable std::deque<BeamId> m_rrBeams; //!< Queue of order of beams to transmit
    mutable std::unordered_set<BeamId, BeamIdHash> m_rrBeamsSet; //!< Set of known beams
};

class NrMacSchedulerOfdmaSymbolPerBeamPF : public NrMacSchedulerOfdmaSymbolPerBeam
{
  public:
    using GetBwInRbgFromSchedFunc = std::function<uint16_t(void)>;
    using GetAmcFromSchedFunc = std::function<Ptr<NrAmc>()>;
    NrMacSchedulerOfdmaSymbolPerBeamPF(GetAmcFromSchedFunc amc = nullptr,
                                       GetBwInRbgFromSchedFunc bandwidthInRbgFunc = nullptr);
    static TypeId GetTypeId();

    /**
     * @brief Calculate the number of symbols to assign to each beam based on a PF approximation
     * @param symAvail Number of available symbols
     * @param activeDl Map of active DL UE and their beam
     * @return symbols per beam allocation map
     */
    NrMacSchedulerNs3::BeamSymbolMap GetSymPerBeam(
        uint32_t symAvail,
        const NrMacSchedulerNs3::ActiveUeMap& activeDl) const override;

  private:
    mutable std::unordered_map<BeamId, std::tuple<uint64_t, uint64_t, uint64_t>, BeamIdHash>
        activeBeamsPrevAndCurrTBS; //!< Set of known beams, their previous TBS and current TBS
    /// Function that retrieves error model associated with scheduler
    GetAmcFromSchedFunc m_getAmcFunc;
    /// Function that retrieves bandwidth in RBG from the scheduler
    GetBwInRbgFromSchedFunc m_getBwFunc;
};
} // namespace ns3

#endif // NR_MAC_SCHEDULER_OFDMA_SYMBOL_PER_BEAM_H
