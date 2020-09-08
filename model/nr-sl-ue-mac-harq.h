/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef NR_SL_UE_MAC_HARQ_H
#define NR_SL_UE_MAC_HARQ_H


#include <ns3/object.h>

#include <map>
#include <unordered_set>

namespace ns3 {

class PacketBurst;
class Packet;

/**
 * \ingroup MAC
 * \brief NR Sidelink MAC HARQ entity
 */
class NrSlUeMacHarq : public Object
{
public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrSlUeMacHarq constructor
   */
  NrSlUeMacHarq ();

  /**
   * \brief NrSlUeMacHarq destructor
   */
  virtual ~NrSlUeMacHarq ();

  /**
   * \brief Add destination to this HARQ entity
   *
   * This method is responsible to add the destination into this
   * NR SL HARQ entity and to initialize the number of Sidelink
   * process equivalent to the parameter <\b>maxSidelinkProcess<\b>.
   * Each Sidelink process is capable to store the packets of a
   * transport block, and the LCIDs to which each packet belongs.
   * \see NrSlProcessesBuffer_t
   *
   * \param dstL2Id The destination Layer 2 id
   * \param maxSidelinkProcess The maximum number of sidelink processes for
   *        this destination.
   */
  void AddDst (uint32_t dstL2Id, uint8_t maxSidelinkProcess);

  /**
   * \brief Assign NR Sidelink HARQ process id to a destination
   *
   * This method is used to assign a HARQ process id to a destination
   * if there is an available Sidelink process. In this implementation,
   * the Sidelink process id is basically an index of <\b>NrSlProcessesBuffer_t<\b>
   * vector, which starts from zero, and ends at <\b>maxSidelinkProcess - 1<\b>.
   * Moreover, HARQ process id is the same as Sidelink process id.
   *
   * \param dstL2Id The destination Layer 2 id
   * \return The NR Sidelink HARQ id
   */
  uint8_t AssignNrSlHarqProcessId (uint32_t dstL2Id);

  /**
   * \brief Add the packet to the Sidelink process buffer, which is identified
   *        using destination L2 id, LC id, and the HARQ id.
   * \param dstL2Id The destination Layer 2 id
   * \param lcId The logical channel id
   * \param harqId The HARQ id
   * \param pkt Packet
   */
  void AddPacket (uint32_t dstL2Id, uint8_t lcId, uint8_t harqId, Ptr<Packet> pkt);

  /**
   * \brief Get the packet burst from the Sidelink process buffer, which is identified
   *        using destination L2 id and the HARQ id.
   * \param dstL2Id The destination Layer 2 id
   * \param harqId The HARQ id
   * \return The packet burst
   */
  Ptr<PacketBurst> GetPacketBurst (uint32_t dstL2Id, uint8_t harqId) const;

  /**
   * \brief Receive NR Sidelink Harq feedback
   * \param dstL2Id Destination Layer 2 id
   * \param harqProcessId The harq process id
   */
  void RecvNrSlHarqFeedback (uint32_t dstL2Id, uint8_t harqProcessId);


protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;

private:
  struct NrSlProcessInfo
  {
    Ptr<PacketBurst> pktBurst;
    // maintain list of LCs contained in this TB
    // used to signal HARQ failure to RLC handlers
    std::unordered_set<uint8_t> lcidList;
    /**
     * \brief Status of the SL process
     */
    enum SlProcessStatus
    {
      IDLE = 0,
      BUSY,
    } slProcessStatus {IDLE};   //!< NR Sidelink process status
  };

  typedef std::vector < NrSlProcessInfo> NrSlProcessesBuffer_t; //!< NR SL HARQ process buffer
  std::map <uint32_t, NrSlProcessesBuffer_t> m_nrSlProcessesPackets; //!< Packet under transmission of the SL process of a destination

};

} // namespace ns3

#endif /* NR_SL_UE_MAC_HARQ_H */
