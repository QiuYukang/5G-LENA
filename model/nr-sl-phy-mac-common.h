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

#ifndef NR_SL_PHY_MAC_COMMON_H
#define NR_SL_PHY_MAC_COMMON_H

#include <stdint.h>
#include <limits>

namespace ns3 {

/**
 * \brief NrSlInfoListElement_s
 *
 * Named similar to http://www.eurecom.fr/~kaltenbe/fapi-2.0/structDlInfoListElement__s.html
 */
struct NrSlInfoListElement_s
{
  uint32_t srcl2Id {std::numeric_limits <uint32_t>::max ()}; //!< Source layer 2 id
  uint32_t dstL2Id {std::numeric_limits <uint32_t>::max ()}; //!< Destination Layer 2 id
  uint8_t  harqProcessId {std::numeric_limits <uint8_t>::max ()}; //!< HARQ process ID
  /// HARQ status enum
  enum HarqStatus_e
  {
    ACK, NACK, INVALID
  } m_harqStatus {INVALID}; //!< HARQ status
};

/// SlPscchUeMacStatParameters structure
struct SlPscchUeMacStatParameters
{
  int64_t timestamp {std::numeric_limits<int64_t>::max ()}; //!< In millisecond
  uint64_t imsi {std::numeric_limits<uint64_t>::max ()}; //!< The IMSI of the scheduled UE
  uint16_t rnti {std::numeric_limits<uint16_t>::max ()}; //!< The RNTI scheduled
  uint32_t frameNum {std::numeric_limits<uint32_t>::max ()}; //!< The frame number
  uint32_t subframeNum {std::numeric_limits<uint32_t>::max ()}; //!< The subframe number
  uint16_t slotNum {std::numeric_limits<uint16_t>::max ()}; //!< The slot number
  uint8_t priority {std::numeric_limits<uint8_t>::max ()}; //!< The priority of a LC. When multiple LCs are multiplexed it is the priority of the LC with the highest priority.
  uint8_t mcs {std::numeric_limits<uint8_t>::max ()}; //!< The MCS for transport block
  uint16_t tbSize {std::numeric_limits<uint16_t>::max ()}; //!< The PSSCH transport block size in bytes
  uint16_t slResourceReservePeriod {std::numeric_limits<uint16_t>::max ()}; //!< The Resource reservation period in milliseconds
  uint16_t totalSubChannels {std::numeric_limits<uint16_t>::max ()}; //!< The total number of sub-channels given the SL bandwidth
  uint8_t indexSubchannelStart {std::numeric_limits<uint8_t>::max ()}; //!< The index of the starting sub-channel
  uint8_t slMaxNumPerReserve {std::numeric_limits<uint8_t>::max ()}; //!< The maximum number of reserved PSCCH/PSSCH resources that can be indicated by an SCI
  uint8_t gapReTx1 {std::numeric_limits<uint8_t>::max ()}; //!< The gap between a transmission and its first re-transmission in slots
  uint8_t gapReTx2 {std::numeric_limits<uint8_t>::max ()}; //!< The gap between a transmission and its second re-transmission in slots

  /**
   *  TracedCallback signature.
   *
   * \param [in] params Value of the SlPscchUeMacStatParameters
   */
  typedef void (* TracedCallback)(const SlPscchUeMacStatParameters &params);
};


}

#endif /* NR_SL_PHY_MAC_COMMON_H_ */
