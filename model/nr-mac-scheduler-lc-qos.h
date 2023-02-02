/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_MAC_SCHEDULER_LC_QOS_H
#define NR_MAC_SCHEDULER_LC_QOS_H

#include "nr-mac-scheduler-lc-alg.h"

namespace ns3
{

/**
 *\ingroup scheduler
 *
 * \brief Algorithm for distributing the assigned bytes to the different
 * LCGs/LCs of a UE based on the resource type
 *
 */
class NrMacSchedulerLcQos : public NrMacSchedulerLcAlgorithm
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId ();

    /**
     * \brief Get the type ID of this instance
     * \return the Type ID of this instance
     */
    TypeId GetInstanceTypeId() const override;

    /**
     * \brief NrMacSchedulerLcQos constructor
     */
    NrMacSchedulerLcQos ();
    /**
     * \brief NrMacSchedulerLcQos deconstructor
     */
    ~NrMacSchedulerLcQos () override;


    /**
     * \brief Method to decide how to distribute the assigned bytes to the different LCs
     *        for the DL direction. This algorithm is based on the resource type and the
     *        guaranteed bitrate information of an LC.
     * \param ueLCG LCG of an UE
     * \param tbs TBS to divide between the LCG/LC
     * \return A vector of Assignation
     */
    std::vector<Assignation> AssignBytesToDlLC(const std::unordered_map<uint8_t, LCGPtr>& ueLCG,
                                             uint32_t tbs) const override;

    /**
     * \brief Method to decide how to distribute the assigned bytes to the different LCs
     *        for the UL direction. Due to the scheduler limitation the applied algorithm
     *        distributes bytes in a RR fashion (see NrMacSchedulerLcAlgorithm).
     * \param ueLCG LCG of an UE
     * \param tbs TBS to divide between the LCG/LC
     * \return A vector of Assignation
     */
    std::vector<Assignation> AssignBytesToUlLC(const std::unordered_map<uint8_t, LCGPtr>& ueLCG,
                                             uint32_t tbs) const override;

};
}

#endif /*NR_MAC_SCHEDULER_LC_QOS_H*/
