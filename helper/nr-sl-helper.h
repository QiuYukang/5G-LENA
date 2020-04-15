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
class NrSlCommResourcePoolFactory;
class NrSlCommPreconfigResourcePoolFactory;
class NrUeNetDevice;
class NrAmc;


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

  void SetUeSlAmcAttribute (const std::string &n, const AttributeValue &v);

  /**
   * \brief Set the ErrorModel for SL AMC and UE spectrum at the same time
   * \param errorModelTypeId The TypeId of the error model
   *
   * Equivalent to the calls to
   *
   * * SetUeSlAmcAttribute ("ErrorModelType", ....
   * * SetUeSpectrumAttribute ("SlErrorModelType", ...
   *
   */
  void SetSlErrorModel(const std::string &errorModelTypeId);


  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


private:

  /**
   * \brief Configure the UE parameters
   *
   * This method is used to configure the UE parameters,
   * which can not be set via RRC.
   *
   * \param dev The NrUeNetDevice
   * \param freqCommon The <tt> struct SlFreqConfigCommonNr </tt> to retrieve
   *        SL BWP related configuration
   * \paran general The <tt> struct SlPreconfigGeneralNr </tt> to retrieve
   *        general parameters for a BWP, e.g., TDD pattern
   */
  bool ConfigUeParams (const Ptr<NrUeNetDevice> &dev,
                       const LteRrcSap::SlFreqConfigCommonNr &freqCommon,
                       const LteRrcSap::SlPreconfigGeneralNr &general);
  /*
   * brief Create UE SL AMC object from UE SL AMC factory
   *
   * \returns Ptr of type NrAmc
   */
   Ptr<NrAmc> CreateUeSlAmc () const;
  //Ptr<NrSlCommResourcePoolFactory> m_slResoPoolFactory {nullptr};
  //Ptr<NrSlCommPreconfigResourcePoolFactory> m_slPreConfigResoPoolFactory {nullptr};
  ObjectFactory m_ueSlAmcFactory;        //!< UE SL AMC Object factory

};

}

#endif /* NR_SL_HELPER_H */

