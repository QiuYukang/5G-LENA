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

}

#endif /* NR_SL_PHY_MAC_COMMON_H_ */
