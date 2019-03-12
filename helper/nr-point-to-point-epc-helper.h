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
#ifndef NR_POINT_TO_POINT_EPC_HELPER_H
#define NR_POINT_TO_POINT_EPC_HELPER_H

#include <ns3/point-to-point-epc-helper.h>

namespace ns3 {

/**
 * \ingroup nr
 * \brief Create an EPC network with PointToPoint links,
 * based on LTE's PointToPointEpcHelper
 *
 * \see PointToPointEpcHelper
 */
class NrPointToPointEpcHelper : public PointToPointEpcHelper
{
public:
  /**
   * Constructor
   */
  NrPointToPointEpcHelper ();

  /**
   * Destructor
   */
  virtual ~NrPointToPointEpcHelper () override;

  // inherited from Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

protected:
  virtual void DoAddX2Interface (const Ptr<EpcX2> &enb1X2, const Ptr<NetDevice> &enb1LteDev,
                                 const Ipv4Address &enb1X2Address,
                                 const Ptr<EpcX2> &enb2X2, const Ptr<NetDevice> &enb2LteDev,
                                 const Ipv4Address &enb2X2Address) const override;
  virtual void DoActivateEpsBearerForUe (const Ptr<NetDevice> &ueDevice,
                                         const Ptr<EpcTft> &tft,
                                         const EpsBearer &bearer) const override;
};

} // namespace ns3

#endif // NR_POINT_TO_POINT_EPC_HELPER_H
