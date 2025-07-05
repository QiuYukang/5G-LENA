// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "nr-mac-scheduler-ofdma.h"

#include "ns3/random-variable-stream.h"

namespace ns3
{

/**
 * @class NrMacSchedulerOfdmaRandom
 * @brief Implements a random OFDMA MAC scheduler for NR
 * (New Radio).
 *
 * This class extends NrMacSchedulerOfdma and provides a random scheduling mechanism for user
 * equipment (UE). The scheduler allocates resources to UEs in a random manner. The available
 * RBGs are divided among UEs in a random manner to ensure that all UEs get assigned,
 * with no clear preference to a particular UE.
 * The generated interference is random in the power/time/frequency/spatial
 * domains because of the random selection of UEs.
 *
 * Key functionalities include:
 * - Overrides SortUeVector function from NrMacSchedulerOfdma to allow random DL and UL Ofdma
 * scheduling.
 */
class NrMacSchedulerOfdmaRandom : public NrMacSchedulerOfdma
{
  public:
    /**
     * @brief GetTypeId
     * @return The TypeId of the class
     */
    static TypeId GetTypeId();

    /**
     * @brief NrMacSchedulerOfdmaRandom constructor
     */
    NrMacSchedulerOfdmaRandom();

    /**
     * @brief ~NrMacSchedulerOfdmaRandom deconstructor
     */
    ~NrMacSchedulerOfdmaRandom() override
    {
    }

  protected:
    // RR is a simple scheduler: it doesn't do anything in the next
    // inherited calls.
    std::shared_ptr<NrMacSchedulerUeInfo> CreateUeRepresentation(
        const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) const override;

    std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq& lhs,
                       const NrMacSchedulerNs3::UePtrAndBufferReq& rhs)>
    GetUeCompareDlFn() const override
    {
        return nullptr;
    }

    std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq& lhs,
                       const NrMacSchedulerNs3::UePtrAndBufferReq& rhs)>
    GetUeCompareUlFn() const override
    {
        return nullptr;
    }

    void AssignedDlResources(const UePtrAndBufferReq& ue,
                             const FTResources& assigned,
                             const FTResources& totAssigned) const override;

    void AssignedUlResources(const UePtrAndBufferReq& ue,
                             const FTResources& assigned,
                             const FTResources& totAssigned) const override;

    void NotAssignedDlResources(const UePtrAndBufferReq& ue,
                                const FTResources& notAssigned,
                                const FTResources& totalAssigned) const override
    {
    }

    void NotAssignedUlResources(const UePtrAndBufferReq& ue,
                                const FTResources& notAssigned,
                                const FTResources& totalAssigned) const override
    {
    }

    void BeforeDlSched(const UePtrAndBufferReq& ue,
                       const FTResources& assignableInIteration) const override
    {
    }

    void BeforeUlSched(const UePtrAndBufferReq& ue,
                       const FTResources& assignableInIteration) const override
    {
    }

    void SortUeVector(std::vector<UePtrAndBufferReq>* ueVector,
                      [[maybe_unused]] const GetCompareUeFn& GetCompareFn) const override;

    int64_t AssignStreams(int64_t stream) override;

  private:
    //!< uniform random variable used to shuffle vector of users for scheduling
    Ptr<UniformRandomVariable> m_uniformRvShuffle;
};

} // namespace ns3
