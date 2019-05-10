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

#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/antenna-array-basic-model.h>
#include <ns3/event-id.h>
#include <functional>

#ifndef NR_CH_ACCESS_MANAGER_H_
#define NR_CH_ACCESS_MANAGER_H_

namespace ns3 {

class NrChAccessManager : public Object
{
public:
  /**
   * \brief Get the type ID
   * \return the type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief ChannelAccessManager constructor
   */
  NrChAccessManager ();

  /**
   * \brief ~ChannelAccessManager
   */
  virtual ~NrChAccessManager () override;

  /**
   * \brief Set duration of grant for transmission.
   * \param grantDuration duration of grant
   */
  void SetGrantDuration (Time grantDuration);

  /**
   * \brief Get grant duration time.
   * \returns default grant duration
   */
  Time GetGrantDuration () const;

  void SetEdThreshold (double edThreshold);
  double GetEdThreshold () const;

  /**
   * \brief A function that signal that the channel has been earned
   */
  typedef std::function<void (const Time &time)> AccessGrantedCallback;

  /**
   * @brief RequestAccess
   */
  virtual void RequestAccess () = 0;
  /**
   * @brief SetAccessGrantedCallback
   * @param cb
   */
  virtual void SetAccessGrantedCallback (const AccessGrantedCallback &cb) = 0;

  /**
   * @brief ReleaseGrant
   */
  virtual void ReleaseGrant () = 0;

private:
  double m_edThreshold {0.0};
  Time m_grantDuration;
};

class NrAlwaysOnAccessManager : public NrChAccessManager
{
public:
  /**
   * \brief Get the type ID
   * \return the type id
   */
  static TypeId GetTypeId (void);

  NrAlwaysOnAccessManager ();
  ~NrAlwaysOnAccessManager () override;

  virtual void RequestAccess () override;
  virtual void SetAccessGrantedCallback (const AccessGrantedCallback &cb) override;
  virtual void ReleaseGrant () override;

private:
  std::vector<AccessGrantedCallback> m_accessGrantedCb;
};

}

#endif /* NR_CH_ACCESS_MANAGER_H_ */
