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



#ifndef NR_MAC_PDU_TAG_H
#define NR_MAC_PDU_TAG_H

#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "nr-phy-mac-common.h"

namespace ns3 {

/**
 * \ingroup ue-mac
 * \ingroup gnb-mac
 *
 * \brief The NrMacPduTag class
 */
class NrMacPduTag : public Tag
{
public:
  /**
   * \brief GetTypeId
   * \return the type id object
   */
  static TypeId  GetTypeId (void);

  /**
   * \brief GetInstanceTypeId
   * \return the instance of the object
   */
  virtual TypeId  GetInstanceTypeId (void) const;

  /**
   * Create an empty MacP PDU tag
   */
  NrMacPduTag () = default;

  /**
   * \brief NrMacPduTag constructor
   * \param sfn the SfnSf
   * \param symStart the sym start
   * \param numSym the number of symbol
   */
  NrMacPduTag (SfnSf sfn, uint8_t symStart, uint8_t numSym);

  virtual void  Serialize (TagBuffer i) const;
  virtual void  Deserialize (TagBuffer i);
  virtual uint32_t  GetSerializedSize () const;
  virtual void Print (std::ostream &os) const;

  /**
   * \brief GetSfn
   * \return the SfnSf installed in this object
   */
  SfnSf  GetSfn () const
  {
    return m_sfnSf;
  }

  /**
   * \brief SetSfn
   * \param sfn the SfnSf to install
   */
  void  SetSfn (SfnSf sfn)
  {
    this->m_sfnSf = sfn;
  }

  /**
   * \brief GetSymStart
   * \return the symStart variable installed in this object
   */
  uint8_t GetSymStart () const
  {
    return m_symStart;
  }

  /**
   * \brief GetNumSym
   * \return the numSym variable installed in this object
   */
  uint8_t GetNumSym () const
  {
    return m_numSym;
  }

  /**
   * \brief SetSymStart
   * \param symStart the symStart value to install
   */
  void SetSymStart (uint8_t symStart)
  {
    m_symStart = symStart;
  }

  /**
   * \brief SetNumSym
   * \param numSym the numSym value to install
   */
  void SetNumSym (uint8_t numSym)
  {
    m_numSym = numSym;
  }

protected:
  SfnSf m_sfnSf;           //!< SfnSf
  uint8_t m_symStart {0};  //!< Symstart
  uint8_t m_numSym {0};    //!< Num sym
};

} //namespace ns3

#endif /* NR_MAC_PDU_TAG_H */
