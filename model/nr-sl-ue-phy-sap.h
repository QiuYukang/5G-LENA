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

#ifndef NR_SL_UE_PHY_SAP_H
#define NR_SL_UE_PHY_SAP_H

#include <stdint.h>
#include <ns3/ptr.h>
#include <ns3/nstime.h>


namespace ns3 {

/**
 * \ingroup nr
 *
 * Service Access Point (SAP) offered by the UE PHY to the UE MAC
 * for NR Sidelink
 *
 * This is the PHY SAP Provider, i.e., the part of the SAP that contains
 * the UE PHY methods called by the UE MAC
 */
class NrSlUePhySapProvider
{
public:
  /**
   * \brief Destructor
   */
  virtual ~NrSlUePhySapProvider ();

  //Sidelink Communication
  /**
   * \brief Ask the PHY the bandwidth in RBs
   *
   * \return the bandwidth in RBs
   */
  virtual uint32_t GetBwInRbs () const = 0;
  /**
   * \brief Get the slot period
   * \return the slot period (depend on the numerology)
   */
  virtual Time GetSlotPeriod () const = 0;
  /**
   * set the current sidelink transmit pool
   * \param pool The transmission pool
   */
 // virtual void SetSlCommTxPool (Ptr<SidelinkTxCommResourcePool> pool) = 0;

};


/**
 * \ingroup nr
 *
 * Service Access Point (SAP) offered by the UE MAC to the UE PHY
 * for NR Sidelink
 *
 * This is the CPHY SAP User, i.e., the part of the SAP that contains the UE
 * MAC methods called by the UE PHY
*/
class NrSlUePhySapUser
{
public:
  /**
   * \brief Destructor
   */
  virtual ~NrSlUePhySapUser ();

  /**
   * \brief Gets the active Sidelink pool id used for transmission and reception
   *
   * \return The active TX pool id
   */
  virtual uint8_t GetSlActiveTxPoolId () = 0;

};


/**
 * \ingroup nr
 *
 * Template for the implementation of the NrSlUePhySapProvider as a member
 * of an owner class of type C to which all methods are forwarded.
 *
 * Usually, methods are forwarded to UE PHY class, which are called by UE MAC
 * to perform NR Sidelink.
 *
 */
template <class C>
class MemberNrSlUePhySapProvider : public NrSlUePhySapProvider
{
public:
  /**
   * \brief Constructor
   *
   * \param owner The owner class
   */
  MemberNrSlUePhySapProvider (C* owner);

  virtual uint32_t GetBwInRbs () const;
  virtual Time GetSlotPeriod () const;

  // methods inherited from NrSlUePhySapProvider go here
  //NR Sidelink communication

private:
  MemberNrSlUePhySapProvider ();
  C* m_owner; ///< the owner class
};

template <class C>
MemberNrSlUePhySapProvider<C>::MemberNrSlUePhySapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
uint32_t
MemberNrSlUePhySapProvider<C>::GetBwInRbs () const
{
  return m_owner->DoGetBwInRbs ();
}

template <class C>
Time
MemberNrSlUePhySapProvider<C>::GetSlotPeriod () const
{
  return m_owner->DoGetSlotPeriod ();
}

//Sidelink communication
/*
template <class C>
void
MemberNrSlUePhySapProvider<C>::SetSlCommTxPool (Ptr<SidelinkTxCommResourcePool> pool)
{
  m_owner->DoSetSlCommTxPool (pool);
}
*/



/**
 * \ingroup nr
 *
 * Template for the implementation of the NrSlUePhySapUser as a member
 * of an owner class of type C to which all methods are forwarded.
 *
 * Usually, methods are forwarded to UE MAC class, which are called by UE PHY
 * to perform NR Sidelink.
 *
 */
template <class C>
class MemberNrSlUePhySapUser : public NrSlUePhySapUser
{
public:
  /**
   * \brief Constructor
   *
   * \param owner The owner class
   */
  MemberNrSlUePhySapUser (C* owner);

  // methods inherited from NrSlUePhySapUser go here
  virtual uint8_t GetSlActiveTxPoolId ();

private:
  C* m_owner; ///< the owner class
};

template <class C>
MemberNrSlUePhySapUser<C>::MemberNrSlUePhySapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
uint8_t
MemberNrSlUePhySapUser<C>::GetSlActiveTxPoolId ()
{
  return m_owner->DoGetSlActiveTxPoolId ();
}




} // namespace ns3


#endif // NR_SL_UE_PHY_SAP_H
