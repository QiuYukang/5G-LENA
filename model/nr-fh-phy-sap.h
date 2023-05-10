/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_FH_PHY_SAP_H
#define NR_FH_PHY_SAP_H

#include "nr-gnb-phy.h"

namespace ns3
{

/**
 * \brief Service Access Point (SAP) offered by the FhControl instance
 *        to the gnb PHY instance.
 *
 * This is the *NrFhPhySapProvider*, i.e., the part of the SAP that contains
 * the FhControl methods called by the gnb PHY instance.
 *
 * FH Control ---> PHY
 */

class NrFhPhySapProvider
{
  public:
    virtual ~NrFhPhySapProvider();

    virtual void DoesAllocationFit() = 0;
};

/**
 * \brief Service Access Point (SAP) offered by the gnb PHY instance to
 * the FhControl instance.
 *
 * This is the *NrFhPhySapUser*, i.e., the part of the SAP that contains the
 * gnb PHY methods called by the FhControl instance.
 *
 * PHY --> FH Control
 */

class NrFhPhySapUser
{
  public:
    virtual ~NrFhPhySapUser();
};

/**
 * \brief Template for the implementation of the NrFhPhySapProvider as a member of
 *        an owner class of type C to which all methods are forwarded.
 */
template <class C>
class MemberNrFhPhySapProvider : public NrFhPhySapProvider
{
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    MemberNrFhPhySapProvider(C* owner);

    virtual void DoesAllocationFit();

  private:
    MemberNrFhPhySapProvider();
    C* m_owner; ///< the owner class

}; // end of class MemberNrFhPhySapProvider

template <class C>
MemberNrFhPhySapProvider<C>::MemberNrFhPhySapProvider(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
MemberNrFhPhySapProvider<C>::DoesAllocationFit()
{
    return m_owner->DoGetDoesAllocationFit();
}

/**
 * \brief Template for the implementation of the NrFhPhySapUser as a member of an
 *        owner class of type C to which all methods are forwarded.
 */

template <class C>
class MemberNrFhPhySapUser : public NrFhPhySapUser
{
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    MemberNrFhPhySapUser(C* owner);

  private:
    MemberNrFhPhySapUser();
    C* m_owner; ///< the owner class

}; // end of class NrFhPhySapUser

template <class C>
MemberNrFhPhySapUser<C>::MemberNrFhPhySapUser(C* owner)
    : m_owner(owner)
{
}

} // namespace ns3
#endif // NR_FH_PHY_SAP_H
