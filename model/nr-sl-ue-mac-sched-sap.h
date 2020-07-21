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
*/
#ifndef NR_SL_UE_MAC_SCHED_SAP_PROVIDER_H
#define NR_SL_UE_MAC_SCHED_SAP_PROVIDER_H

#include "sfnsf.h"

#include <iostream>
#include <list>
#include <vector>
#include <memory>


namespace ns3 {

/**
 * \ingroup scheduler
 *
 * \brief The SAP interface between NR UE MAC and NR SL UE scheduler
 */
class NrSlUeMacSchedSapProvider
{
public:
  /**
   * ~NrSlUeMacSchedSapProvider
   */
  virtual ~NrSlUeMacSchedSapProvider () = default;

  /**
   * \brief NR Sidelink slot info.
   */
  struct NrSlSlotInfo
  {
    /**
     * \brief NrSlSlotInfo constructor
     *
     * \param numSlPscchRbs Indicates the number of PRBs for PSCCH in a resource pool where it is not greater than the number PRBs of the subchannel.
     * \param slPscchSymStart Indicates the starting symbol used for sidelink PSCCH in a slot
     * \param slPscchSymlength Indicates the total number of symbols available for sidelink PSCCH
     * \param slPsschSymStart Indicates the starting symbol used for sidelink PSSCH in a slot
     * \param slPsschSymlength Indicates the total number of symbols available for sidelink PSSCH
     * \param slSubchannelSize Indicates the subchannel size in number of RBs
     * \param slMaxNumPerReserve Indicates the maximum number of reserved PSCCH/PSSCH resources that can be indicated by an SCI.
     * \param sfn The SfnSf
     */
    NrSlSlotInfo (uint16_t numSlPscchRbs, uint16_t slPscchSymStart, uint16_t slPscchSymLength,
                  uint16_t slPsschSymStart, uint16_t slPsschSymLength, uint16_t slSubchannelSize,
                  uint16_t slMaxNumPerReserve, SfnSf sfn)
    {
      this->numSlPscchRbs = numSlPscchRbs;
      this->slPscchSymStart = slPscchSymStart;
      this->slPscchSymLength = slPscchSymLength;
      this->slPsschSymStart = slPsschSymStart;
      this->slPsschSymLength = slPsschSymLength;
      this->slSubchannelSize = slSubchannelSize;
      this->slMaxNumPerReserve = slMaxNumPerReserve;
      this->sfn = sfn;
    }
    //PSCCH
    uint16_t numSlPscchRbs {0}; //!< Indicates the number of PRBs for PSCCH in a resource pool where it is not greater than the number PRBs of the subchannel.
    uint16_t slPscchSymStart {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the starting symbol used for sidelink PSCCH in a slot
    uint16_t slPscchSymLength {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of symbols available for sidelink PSCCH
    //PSSCH
    uint16_t slPsschSymStart {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the starting symbol used for sidelink PSSCH in a slot
    uint16_t slPsschSymLength {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of symbols available for sidelink PSSCH
    //subchannel size in RBs
    uint16_t slSubchannelSize {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the subchannel size in number of RBs
    uint16_t slMaxNumPerReserve {std::numeric_limits <uint16_t>::max ()}; //!< The maximum number of reserved PSCCH/PSSCH resources that can be indicated by an SCI.
    SfnSf sfn {}; //!< The SfnSf
  };

  /**
   * Parameters for NrSlMacSapProvider::ReportNrSlBufferStatus
   */
  struct SchedUeNrSlReportBufferStatusParams
  {
    /**
     * \brief NrSlReportBufferStatusParameters constructor
     *
     * \param rnti RNTI
     * \param lcid Logical Channel ID
     * \param txQueueSize The current size of the RLC transmission queue
     * \param txQueueHolDelay The Head Of Line delay of the transmission queue
     * \param retxQueueSize The current size of the RLC retransmission queue
     * \param retxQueueHolDelay The Head Of Line delay of the retransmission queue
     * \param srcL2Id Sidelink source L2 ID (24 bits)
     * \param destL2Id Sidelink destination L2 ID (24 bits)
     */
    SchedUeNrSlReportBufferStatusParams (uint16_t rnti, uint8_t lcId,
                                         uint32_t txQueueSize, uint16_t txQueueHolDelay,
                                         uint32_t retxQueueSize, uint16_t retxQueueHolDelay,
                                         uint16_t statusPduSize, uint32_t srcL2Id,  uint32_t dstL2Id)
    {
      this->rnti = rnti;
      this->lcid = lcId;
      this->txQueueSize = txQueueSize;
      this->txQueueHolDelay = txQueueHolDelay;
      this->retxQueueSize = retxQueueSize;
      this->retxQueueHolDelay = retxQueueHolDelay;
      this->statusPduSize = statusPduSize;
      this->srcL2Id = srcL2Id;
      this->dstL2Id = dstL2Id;
    }

    uint16_t rnti {std::numeric_limits <uint16_t>::max ()};  //!< the C-RNTI identifying the UE
    uint8_t lcid {std::numeric_limits <uint8_t>::max ()};  //!< the logical channel id corresponding to the sending RLC instance
    uint32_t txQueueSize {0};  //!< the current size of the RLC transmission queue
    uint16_t txQueueHolDelay {std::numeric_limits <uint16_t>::max ()};  //!< the Head Of Line delay of the transmission queue
    uint32_t retxQueueSize {0};  //!< the current size of the RLC retransmission queue in bytes
    uint16_t retxQueueHolDelay {std::numeric_limits <uint16_t>::max ()};  //!< the Head Of Line delay of the retransmission queue
    uint16_t statusPduSize {0};  //!< the current size of the pending STATUS RLC  PDU message in bytes
    uint32_t srcL2Id {0};  //!< Source L2 ID (24 bits)
    uint32_t dstL2Id {0};  //!< Destination L2 ID (24 bits)
  };

  /**
   * \brief Send NR Sidelink RLC buffer status report from UE MAC to the UE scheduler
   *
   * \param params NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams
   */
  virtual void SchedUeNrSlRlcBufferReq (const struct SchedUeNrSlReportBufferStatusParams& params) = 0;
  /**
   * \brief Send NR Sidleink trigger request from UE MAC to the UE scheduler
   *
   * \param dstL2Id The destination layer 2 id
   * \param params NrSlUeMacSchedSapProvider::NrSlSlotInfo
   */
  virtual void SchedUeNrSlTriggerReq (uint32_t dstL2Id, const std::list <NrSlSlotInfo>& params) = 0;
};

/**
 * \ingroup scheduler
 *
 * \brief The Interface between NR SL UE Scheduler and NR UE MAC
 */
class NrSlUeMacSchedSapUser
{
public:
  /**
   * \brief ~NrSlUeMacSchedSapUser
   */
  virtual ~NrSlUeMacSchedSapUser () = default;

  struct NrSlSlotAlloc
  {
    SfnSf sfn {}; //!< The SfnSf
    uint32_t dstL2Id {std::numeric_limits <uint32_t>::max ()}; //!< The destination Layer 2 Id

    uint8_t ndi {std::numeric_limits <uint8_t>::max ()}; //!< The flag to indicate the new data allocation
    uint8_t lcId {std::numeric_limits <uint8_t>::max ()}; //!< The Logical channel id
    uint8_t priority {std::numeric_limits <uint8_t>::max ()}; //!< The LC priority
    uint32_t tbSize {std::numeric_limits <uint32_t>::max ()}; //!< The transport block size

    uint16_t mcs {std::numeric_limits <uint16_t>::max ()}; //!< The MCS
    uint16_t indexSubchannelStart {std::numeric_limits <uint16_t>::max ()}; //!< Index of the first subchannel allocated
    uint16_t subchannelLength {std::numeric_limits <uint16_t>::max ()}; //!< Total number of subchannel allocated
    uint16_t indexSymStart {std::numeric_limits <uint16_t>::max ()}; //!< Index of the first symbol allocated
    uint16_t SymLength {std::numeric_limits <uint16_t>::max ()}; //!< Total number of symbol allocated
    uint16_t maxNumPerReserve {std::numeric_limits <uint16_t>::max ()}; //!< The maximum number of reserved PSCCH/PSSCH resources that can be indicated by an SCI.
    uint8_t  gapReTx1 {std::numeric_limits <uint8_t>::max ()}; //!< The gap between a transmission and its first retransmission in slots
    uint8_t  gapReTx2 {std::numeric_limits <uint8_t>::max ()}; //!< The gap between a transmission and its first retransmission in slots
  };

  /**
   * \brief Less than operator overloaded for NrSlSlotAlloc
   *
   * \param l first NrSlSlotAlloc
   * \param r second NrSlSlotAlloc
   * \returns true if first NrSlSlotAlloc SfnSf parameter values are less than the second NrSlSlotAlloc SfnSf parameters"
   */
  friend bool operator < (const NrSlSlotAlloc &l, const NrSlSlotAlloc &r)
  {
    return l.sfn < r.sfn;
  }

  /**
   * \brief Send the NR Sidelink allocation from the UE scheduler to UE MAC
   *
   * \param params NrSlUeMacSchedSapUser::NrSlSlotAlloc
   */
  virtual void SchedUeNrSlConfigInd (const NrSlUeMacSchedSapUser::NrSlSlotAlloc& params) = 0;

  /**
   * \brief Method to get total number of sub-channels.
   *
   * \return the total number of sub-channels.
   */
  virtual uint8_t GetTotalSubCh () const = 0;

};

std::ostream & operator<< (std::ostream & os, NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams const & p);


} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHED_SAP_PROVIDER_H */
