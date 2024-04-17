/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_SL_UE_MAC_HARQ_H
#define NR_SL_UE_MAC_HARQ_H

#include "nr-phy-mac-common.h"

#include <ns3/object.h>
#include <ns3/traced-callback.h>

#include <deque>
#include <map>
#include <optional>
#include <unordered_set>

namespace ns3
{

class PacketBurst;
class Packet;

/**
 * \ingroup MAC
 * \brief NR Sidelink MAC HARQ entity
 *
 * This is HARQ entity to NR SL MAC PDUs under retransmission. The
 * total number of HARQ/Sidelink processes can be configured
 * only once by calling \ref InitHarqBuffer (uint8_t, uint8_t), which is
 * the responsibility of a UE MAC.
 */
class NrSlUeMacHarq : public Object
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId(void);

    /**
     * \brief NrSlUeMacHarq constructor
     */
    NrSlUeMacHarq();

    /**
     * \brief NrSlUeMacHarq destructor
     */
    virtual ~NrSlUeMacHarq();

    /**
     * \brief Add destination to this HARQ entity
     *
     * This method is responsible to initialize NR SL HARQ process id buffer
     * (see NrSlUeMacHarq#m_nrSlHarqIdBuffer) and NR SL HARQ packet buffer
     * (see NrSlUeMacHarq#m_nrSlHarqPktBuffer). The size of these buffers
     * will be equivalent to the maximum number of sidelink processes passed
     * through this method.
     *
     * \param maxSlProcessesMultiplePdu The maximum number of sidelink
     *        processes for multiple PDU grants for this HARQ entity.
     * \param maxSlProcesses The maximum number of sidelink processes for
     *        this HARQ entity.
     */
    void InitHarqBuffer(uint8_t maxSlProcessesMultiplePdu, uint8_t maxSlProcesses);

    /**
     * \brief Assign NR Sidelink HARQ process id to a destination
     *
     * Allocate and assign a HARQ process id to a destination
     *
     * The timeout value is to protect against the process ID becoming blocked
     * on a HARQ-protected transmission that is never acknowledged.
     *
     * \param dstL2Id The destination Layer 2 id
     * \param multiplePdu Whether the process corresponds to a multiple PDU grant
     * \param timeout The timeout value
     * \return The NR Sidelink HARQ id
     */
    std::optional<uint8_t> AllocateNrSlHarqProcessId(uint32_t dstL2Id,
                                                     bool multiplePdu,
                                                     Time timeout);

    /**
     * \brief Deallocate a previously allocated HARQ process ID
     *
     * If the HARQ ID is no longer allocated (e.g., due to a previous
     * timeout or HARQ acknowledgment), then this method does nothing.
     *
     * \param harqId The HARQ process ID to deallocate
     */
    void DeallocateNrSlHarqProcessId(uint8_t harqId);

    /**
     * Stop and restart the timer protecting the deallocation of the
     * HARQ process ID.  If the HARQ ID is not allocated, this method
     * will return false.
     * \param harqId the HARQ process ID
     * \param timeout The new expiration time (relative to now)
     * \return true if the timer was renewed
     */
    bool RenewProcessIdTimer(uint8_t harqId, Time timeout);

    /**
     * \brief Get the number of available HARQ process ids
     * \return The number of available HARQ process ids
     */
    uint32_t GetNumAvailableHarqIds() const;

    /**
     * \brief Is the given HARQ id available
     * \param harqId The HARQ process id
     * \return returns true if the HARQ id is available; otherwise false
     */
    bool IsHarqIdAvailable(uint8_t harqId) const;

    /**
     * \brief Add the packet to the Sidelink process buffer, which is identified
     *        using destination L2 id, LC id, and the HARQ id.
     * \param dstL2Id The destination Layer 2 id
     * \param lcId The logical channel id
     * \param harqId The HARQ id
     * \param pkt Packet
     */
    void AddPacket(uint32_t dstL2Id, uint8_t lcId, uint8_t harqId, Ptr<Packet> pkt);

    /**
     * \brief Get the packet burst from the Sidelink process buffer, which is
     *        identified using destination L2 id and the HARQ id.
     *
     * This method may return nullptr if no matching PacketBurst is found
     *
     * \param dstL2Id The destination Layer 2 id
     * \param harqId The HARQ id
     * \return The packet burst (if found) or a nullptr if not found
     */
    Ptr<PacketBurst> GetPacketBurst(uint32_t dstL2Id, uint8_t harqId) const;

    /**
     * \brief Receive NR Sidelink Harq feedback
     * \param harqInfo Sidelink HARQ info structure
     */
    void RecvNrSlHarqFeedback(SlHarqInfo harqInfo);

    /**
     * Flush the HARQ buffer associated with the HARQ process id.
     *
     * \param harqId HARQ process ID
     */
    void FlushNrSlHarqBuffer(uint8_t harqId);

    /**
     * TracedCallback signature for received HARQ feedback
     * \param [in] slHarqInfo received SlHarqInfo parameter
     */
    typedef void (*RxHarqFeedbackTracedCallback)(const SlHarqInfo& slHarqInfo);

    /**
     * TracedCallback signature for HARQ process allocate
     * \param [in] harqId HARQ process ID
     * \param [in] dstL2Id Destination L2 ID
     * \param [in] multiplePdu whether the process is for a multiple PDU grant
     * \param [in] timeout Timeout
     * \param [in] available Number of remaining available HARQ process IDs
     */
    typedef void (*AllocateTracedCallback)(uint8_t harqId,
                                           uint32_t dstL2Id,
                                           bool multiplePdu,
                                           Time timeout,
                                           std::size_t available);

    /**
     * TracedCallback signature for HARQ process deallocate
     * \param [in] harqId HARQ process ID
     * \param [in] available Number of remaining available HARQ process IDs
     */
    typedef void (*DeallocateTracedCallback)(uint8_t harqId, std::size_t available);

    /**
     * TracedCallback signature for request for packet burst (retransmission)
     * \param [in] dstL2Id Destination L2 ID
     * \param [in] harqId HARQ process ID
     */
    typedef void (*PacketBurstTracedCallback)(uint32_t dstL2Id, uint8_t harqId);

    /**
     * TracedCallback signature for HARQ timer expiry
     * \param [in] harqId HARQ process ID
     */
    typedef void (*TimeoutTracedCallback)(uint8_t harqId);

  protected:
    /**
     * \brief DoDispose method inherited from Object
     */
    void virtual DoDispose() override;

  private:
    /**
     * \brief struct to store the NR SL HARQ information
     */
    struct NrSlProcessInfo
    {
        Ptr<PacketBurst> pktBurst; //!< TB under HARQ
        // maintain list of LCs contained in this TB
        // used to signal HARQ failure to RLC handlers
        std::unordered_set<uint8_t> lcidList;                   //!< LC id container
        uint32_t dstL2Id{std::numeric_limits<uint32_t>::max()}; //!< Destination L2 id
        bool multiplePdu{false}; //!< Whether this process is for a multiple PDU grant
        EventId timer;           //!< Timer to expire process ID if not successfully ACKed
        bool allocated{false};   //!< Whether this process is allocated
    };

    /**
     * Timer handler to prevent HARQ process from being bound to a transport
     * block that is never acknowledged.
     * \param harqId the HARQ process ID
     */
    void HarqProcessTimerExpiry(uint8_t harqId);

    /**
     * Re-initialize the packet buffer data structure.
     * \param harqId the HARQ process ID to reset
     */
    void ResetPacketBuffer(uint8_t harqId);

    uint8_t m_maxSlProcessesMultiplePdu{0}; //!< Maximum no. of SL processes for multiple PDU grants
    uint8_t m_maxSlProcesses{0};            //!< Maximum no. of SL processes
    std::vector<NrSlProcessInfo> m_nrSlHarqPktBuffer; //!< NR SL HARQ packet buffer
    uint8_t m_numProcessesMultiplePdu{
        0}; //!< Number of SL processes allocated for multiple PDU grants
    std::deque<uint8_t> m_nrSlHarqIdBuffer; //!< A container to store available HARQ/SL process ids

    TracedCallback<const SlHarqInfo&> m_rxHarqFeedback; //!< Trace of SlHarqInfo
    TracedCallback<uint8_t, uint32_t, bool, Time, std::size_t>
        m_allocateTrace;                                    //!< Trace HARQ ID allocation
    TracedCallback<uint8_t, std::size_t> m_deallocateTrace; //! Trace HARQ ID deallocation
    TracedCallback<uint32_t, uint8_t> m_packetBurstTrace;   //! Trace PB requests
    TracedCallback<uint8_t> m_timeoutTrace;                 //! Trace HARQ timer expiry
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_HARQ_H */
