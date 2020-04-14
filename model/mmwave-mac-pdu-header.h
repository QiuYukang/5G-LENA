/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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


#ifndef MMWAVE_MAC_PDU_HEADER_H
#define MMWAVE_MAC_PDU_HEADER_H

#include "ns3/packet.h"
#include "ns3/nstime.h"

namespace ns3 {

class Tag;

/**
 * \ingroup ue-mac
 * \ingroup gnb-mac
 *
 * \brief The MacSubheader struct
 */
struct MacSubheader
{
  /**
   * \brief MacSubheader constructor
   * \param lcid LC ID
   * \param size Size
   */
  MacSubheader (uint8_t lcid, uint32_t size) :
    m_lcid (lcid), m_size (size)
  {
  }

  /**
   * \brief Get the size in some weird way that probably is OK with some
   * nerd guy inside 3GPP
   *
   * \return I don't know, don't ask this poor documentation writer
   */
  uint32_t GetSize ()
  {
    if (m_size > 127)
      {
        return 3;
      }
    else
      {
        return 2;
      }
  }

  uint8_t   m_lcid;    //!< LC ID
  uint32_t  m_size;    //!< A value that is NOT returned by GetSize()
};

/**
 * \ingroup gnb-mac
 * \ingroup ue-mac
 *
 * \brief The MmWaveMacPduHeader class
 */
class MmWaveMacPduHeader : public Header
{
public:
  /**
   * \brief GetTypeId
   * \return the type id of the object
   */
  static TypeId  GetTypeId (void);
  /**
   * \brief GetInstanceTypeId
   * \return the instance type id
   */
  virtual TypeId  GetInstanceTypeId (void) const;

  /**
   * Create an empty Mac header
   */
  MmWaveMacPduHeader ();

  virtual void  Serialize (Buffer::Iterator i) const;
  virtual uint32_t  Deserialize (Buffer::Iterator i);
  virtual uint32_t  GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

  /**
   * \brief Add a sub header
   * \param rlcPduInfo the RLC PDU info to add
   */
  void  AddSubheader (const MacSubheader &rlcPduInfo);

  /**
   * \brief SetSubheaders
   * \param macSubheaderList the list of subheaders to install
   */
  void SetSubheaders (const std::vector<MacSubheader> &macSubheaderList)
  {
    m_subheaderList = macSubheaderList;
  }

  /**
   * \brief GetSubheaders
   * \return the list of subheaders
   */
  std::vector<MacSubheader> GetSubheaders (void)
  {
    return m_subheaderList;
  }

protected:
  std::vector<MacSubheader> m_subheaderList; //!< Subheader list
  uint32_t m_headerSize; //!< Header size
};

} //namespace ns3

#endif /* MMWAVE_MAC_PDU_HEADER_H */
