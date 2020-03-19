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


#ifndef NR_SL_HELPER_H
#define NR_SL_HELPER_H

#include <ns3/object.h>
#include <ns3/net-device-container.h>
#include <ns3/lte-rrc-sap.h>

namespace ns3 {

class NetDeviceContainer;
class NrSlResourcePoolFactory;
class NrSlPreconfigResourcePoolFactory;


class NrSlHelper : public Object
{

public:
  NrSlHelper (void);
  virtual ~NrSlHelper (void);
/*
  void SetSlPoolFactory (Ptr <NrSlResourcePoolFactory> poolFactory);
  void SetSlPreConfigPoolFactory (Ptr <NrSlPreconfigResourcePoolFactory> preconfigPoolFactory);
*/
  void InstallNrSlPreConfiguration (NetDeviceContainer c, const LteRrcSap::SidelinkPreconfigNr preConfig);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


private:
  Ptr<NrSlResourcePoolFactory> m_slResoPoolFactory {nullptr};
  Ptr<NrSlPreconfigResourcePoolFactory> m_slPreConfigResoPoolFactory {nullptr};
};

}

#endif /* NR_SL_HELPER_H */

