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

#include "sfnsf.h"
#include "nr-phy-mac-common.h"

#include <stdint.h>
#include <limits>
#include <vector>
#include <set>

namespace ns3 {

/**
 * \ingroup utils
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

/**
 * \ingroup utils
 * \brief SlPscchUeMacStatParameters structure
 */
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
   * \ingroup utils
   *  TracedCallback signature.
   *
   * \param [in] params Value of the SlPscchUeMacStatParameters
   */
  typedef void (* TracedCallback)(const SlPscchUeMacStatParameters &params);
};

/**
 * \ingroup utils
 * \brief The SlRlcPduInfo struct
 *
 * \see NrSlSlotAlloc
 */
struct SlRlcPduInfo
{
  SlRlcPduInfo (uint8_t lcid, uint32_t size) :
    lcid (lcid), size (size)
  {}
  uint8_t lcid  {0}; //!< The Logical channel id
  uint32_t size {0}; //!< The transport block size
};

/**
 * \ingroup utils
 * \brief A struct used by the NR SL UE MAC scheduler to communicate slot
 *        allocation to UE MAC.
 */
struct NrSlSlotAlloc
{
  SfnSf sfn {}; //!< The SfnSf
  uint32_t dstL2Id {std::numeric_limits <uint32_t>::max ()}; //!< The destination Layer 2 Id

  uint8_t ndi {std::numeric_limits <uint8_t>::max ()}; //!< The flag to indicate the new data allocation
  uint8_t rv {std::numeric_limits <uint8_t>::max ()}; //!< The redundancy version
  uint8_t priority {std::numeric_limits <uint8_t>::max ()}; //!< The LC priority
  std::vector <SlRlcPduInfo> slRlcPduInfo; //!< The vector containing the transport block size per LC id
  uint16_t mcs {std::numeric_limits <uint16_t>::max ()}; //!< The MCS
  //PSCCH
  uint16_t numSlPscchRbs {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the number of PRBs for PSCCH in a resource pool where it is not greater than the number PRBs of the subchannel.
  uint16_t slPscchSymStart {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the starting symbol used for sidelink PSCCH in a slot
  uint16_t slPscchSymLength {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of symbols available for sidelink PSCCH
  //PSSCH
  uint16_t slPsschSymStart {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the starting symbol used for sidelink PSSCH in a slot
  uint16_t slPsschSymLength {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of symbols allocated for sidelink PSSCH
  uint16_t slPsschSubChStart {std::numeric_limits <uint16_t>::max ()}; //!< Index of the first subchannel allocated for data
  uint16_t slPsschSubChLength {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of subchannel allocated for data

  uint16_t maxNumPerReserve {std::numeric_limits <uint16_t>::max ()}; //!< The maximum number of reserved PSCCH/PSSCH resources that can be indicated by an SCI.
  uint8_t  gapReTx1 {std::numeric_limits <uint8_t>::max ()}; //!< The gap between a transmission and its first retransmission in slots
  uint8_t  gapReTx2 {std::numeric_limits <uint8_t>::max ()}; //!< The gap between a transmission and its first retransmission in slots

  /**
   * \ingroup utils
   * \brief Less than operator overloaded for NrSlSlotAlloc
   * \param rhs other NrSlSlotAlloc to compare
   * \return true if this NrSlSlotAlloc SfnSf parameter values are less than the rhs NrSlSlotAlloc SfnSf parameters
   *
   * The comparison is done on sfnSf
   */
  bool operator < (const NrSlSlotAlloc& rhs) const;
};

/**
 * \ingroup utils
 * \brief A struct used by UE MAC to communicate the time and frequency
 *        allocation information of a Var TTI to UE PHY.
 *
 * The UE MAC will pass this struct along with the SFnSF of the slot to
 * communicate a Var TTI allocation information to UE PHY.
 */
struct NrSlVarTtiAllocInfo
{
  uint16_t symStart {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the starting symbol
  uint16_t symLength {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of contiguous symbols
  uint16_t rbStart {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the starting resource block
  uint16_t rbLength {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of contiguous resource block
  /**
   * \ingroup utils
   * \brief The slVarTtiType enum
   */
  enum
  {
    CTRL,      //!< Used for SL CTRL
    DATA,      //!< Used for SL DATA
    INVALID    //!< Default value. Used to initialize
  } SlVarTtiType {INVALID};

  /**
   * \ingroup utils
   * \brief Less than operator overloaded for NrSlVarTtiAllocInfo
   * \param rhs other NrSlVarTtiAllocInfo to compare
   * \return true if this NrSlVarTtiAllocInfo symbol start value is
   *         less than the rhs NrSlVarTtiAllocInfo symbol start
   *
   * The comparison is done on the symbol start
   */
  bool operator < (const NrSlVarTtiAllocInfo& o) const;
};

/**
 * \ingroup utils
 * \brief A struct used by UE PHY to store the time and frequency
 *        allocation information of each Var TTI communicated by UE MAC.
 *
 * All the Var TTI allocations stored by this struct should belong to
 * the same slot, and each Var TTI in slvarTtiInfoList list must differ
 * in their symbol start.
 */
struct NrSlPhySlotAlloc
{
  SfnSf sfn {}; //!< The SfnSf
  std::set<NrSlVarTtiAllocInfo> slvarTtiInfoList; //!< NR Sidelink variable TTI info list
};

/**
 * \ingroup utils
 * \brief The SlDataRxPacketTraceParams struct
 */
struct SlRxDataPacketTraceParams:public RxPacketTraceParams
{
  uint16_t m_txRnti {std::numeric_limits <uint16_t>::max ()}; //!< The TX UE RNTI
  uint8_t m_ndi {std::numeric_limits <uint8_t>::max ()}; //!< The flag to indicate the new data allocation
  double m_tblerSci2; //!< TBLER of SCI stage 2
  bool m_sci2Corrupted {false}; //!< The flag to indicate the success/failure of SCI stage 2 decodification
  int m_rbStart; //!< The start RB index
  int m_rbEnd; //!< The end RB index
  uint32_t m_dstL2Id {0}; //!< The destination layer 2 id
  uint16_t m_srcL2Id {0}; //!< The source layer 2 id
};

/**
 * \ingroup utils
 * \brief The SlCtrlRxPacketTraceParams struct
 */
struct SlRxCtrlPacketTraceParams:public RxPacketTraceParams
{
  uint16_t m_txRnti {std::numeric_limits <uint16_t>::max ()}; //!< The TX UE RNTI
  double m_tblerSci1; //!< TBLER of SCI stage 1
  bool m_sci1Corrupted {false}; //!< The flag to indicate the new data allocation
  int m_rbStart; //!< The start RB index
  int m_rbEnd; //!< The end RB index
  uint8_t m_priority {std::numeric_limits <uint8_t>::max ()}; //!< The priority
  uint16_t m_slResourceReservePeriod {std::numeric_limits <uint16_t>::max ()}; //!< The resource reservation period
  uint16_t m_totalSubChannels {0}; //!< The total number of subchannel
  uint8_t m_indexStartSubChannel {std::numeric_limits <uint8_t>::max ()}; //!< The starting index of the subchannel
  uint8_t m_lengthSubChannel {std::numeric_limits <uint8_t>::max ()}; //!< The total number of subchannel assigned
  uint8_t m_maxNumPerReserve {std::numeric_limits <uint8_t>::max ()}; //!< The MaxNumPerReserved
  uint32_t m_dstL2Id {0}; //!< The destination layer 2 id
};

}

#endif /* NR_SL_PHY_MAC_COMMON_H_ */
