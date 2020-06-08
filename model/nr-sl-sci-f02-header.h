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

#ifndef NR_SL_SCI_F02_HEADER_H
#define NR_SL_SCI_F02_HEADER_H

#include "ns3/header.h"
#include <iostream>

namespace ns3 {

/**
 * \ingroup nr
 * \brief The packet header for the NR Sidelink Control Information (SCI)
 * format 02 (TS 38.212 Sec 8.3 Rel 16). The following fields must be set
 * before adding this header to a packet.
 *
 * - m_harqId [5 bits]
 * - m_ndi [1 bit]
 * - m_rv [2 bits]
 * - m_srcId [8 bits]
 * - m_dstId [16 bits]
 *
 * Following are the fields which are not mandatory to be set since, the
 * code is not yet ready to use them.
 *
 * - m_csiReq [1 bit]
 * - m_zoneId [12 bits]
 * - m_CommRange [4 bits]
 *
 * The total size of this header is 8 bytes, including the above three
 * non mandatory fields plus 15 bits of zero padding.
 *
 * The use of this header is only possible if:
 * - All the mandatory fields are set using their respective setter methods.
 *   Otherwise, serialization will hit an assert.
 */
class NrSlSciF02Header : public Header
{
public:
  /**
   * \brief Constructor
   *
   * Creates an SCI header
   */
  NrSlSciF02Header ();
  ~NrSlSciF02Header ();

  /**
   * \brief Set the HARQ process id field
   *
   * \param harqId The HARQ process id
   */
  void SetHarqId (uint8_t harqId);
  /**
   * \brief Set the new data indicator field
   *
   * \param ndi The new data indicator
   */
  void SetNdi (uint8_t ndi);
  /**
   * \brief Set the redundancy version
   *
   * \param rv The redundancy version
   */
  void SetRv (uint8_t rv);
  /**
   * \brief Set the layer 2 source id
   *
   * \param srcId The layer 2 source id
   */
  void SetSrcId (uint32_t srcId);
  /**
   * \brief Set the layer 2 destination id
   *
   * \param dstId The layer 2 destination id
   */
  void SetDstId (uint32_t dstId);
  /**
   * \brief Set the Channel State Information request flag
   *
   * \param csiReq The channel state information request flag
   */
  void SetCsiReq (uint8_t csiReq);
  /**
   * \brief Set zone id
   *
   * \param zoneId The zone id
   */
  void SetZoneId (uint16_t zoneId);
  /**
   * \brief Set the communication range requirement
   *
   * Indicates the communication range requirement, see <b>sl-TransRange</b>
   * IE in TS 38.331
   *
   * \param commRange The communication range requirement
   */
  void SetCommRange (uint8_t commRange);

  /**
   * \brief Get the HARQ process id
   *
   * \return The HARQ process id
   */
  uint8_t GetHarqId () const;
  /**
   * \brief Get the new data indicator field value
   *
   * \return The new data indicator field value
   */
  uint8_t GetNdi () const;
  /**
   * \brief Get the Redundancy version
   *
   * \return The Redundancy version
   */
  uint8_t GetRv () const;
  /**
   * \brief Get the source layer 2 id
   *
   * \return The source layer 2 id
   */
  uint8_t GetSrcId () const;
  /**
   * \brief Get the destination layer 2 id
   *
   * \return The destination layer 2 id
   */
  uint16_t GetDstId () const;
  /**
   * \brief Get the Channel State Information request flag
   *
   * \return The channel state information request flag
   */
  uint8_t GetCsiReq () const;
  /**
   * \brief Get zone id
   *
   * \return The zone id
   */
  uint16_t GetZoneId () const;
  /**
   * \brief Get the communication range requirement
   *
   * Indicates the communication range requirement, see <b>sl-TransRange</b>
   * IE in TS 38.331
   *
   * \return The communication range requirement
   */
  uint8_t GetCommRange () const;

  /**
   * \brief Ensure that mandatory fields are configured
   *
   * All the mandatory fields are initialized by default with
   * an invalid value. Therefore, if a mandatory field value is
   * different than this invalid value, we consider it set.
   *
   * \return True if all the mandatory fields are set, false otherwise
   */
  bool EnsureMandConfig () const;

  /**
   * \brief Equality operator
   *
   * \param [in] b the NrSlSciF02Header to compare
   * \returns \c true if \pname{b} is equal
   */
  bool operator == (const NrSlSciF02Header &b) const;



  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  //Mandatory SCI format 02 fields
  uint8_t m_harqId {std::numeric_limits <uint8_t>::max ()}; //!< The HARQ process id
  uint8_t m_ndi {std::numeric_limits <uint8_t>::max ()}; //!< The new data indicator
  uint8_t m_rv {std::numeric_limits <uint8_t>::max ()}; //!< The redundancy version
  uint32_t m_srcId {std::numeric_limits <uint32_t>::max ()}; //!< The source layer 2 id
  uint32_t m_dstId {std::numeric_limits <uint32_t>::max ()}; //!< The destination layer 2 id
  //SCI fields end

  //fields which are not used yet, therefore, it is not mandatory to set them
  uint8_t m_csiReq {0}; //!< The channel state information request flag
  uint16_t m_zoneId {0}; //!< The zone id
  uint8_t m_commRange {0}; //!< The communication range requirement
};


} // namespace ns3

#endif // NR_SL_SCI_F02_HEADER_H
