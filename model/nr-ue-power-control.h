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


#ifndef NR_UE_POWER_CONTROL_H
#define NR_UE_POWER_CONTROL_H

#include <ns3/ptr.h>
#include <ns3/traced-callback.h>
#include <ns3/object.h>
#include <vector>
#include <ns3/lte-ue-power-control.h>


namespace ns3 {

/**
 * \brief This class realizes NR Uplink Power Control functionality.
 *
 * NrUePowerControl entity is responsible for calculating total
 * power that will be used to transmit PUSCH, PUCCH and SRS.
 *
 * NrUePowerControl extends and redefines some of the functionalities
 * defined in LteUePowerControl, to make it more flexible and
 * compatible with NR standard (e.g. support different numerologies).
 * It also extends LteUePowerControls to support power control for PUCCH.
 *
 * NrUePowerControl computes the TX power based on pre-configured
 * parameters and current measurements, such as path loss.
 * NrUePhy should pass to NrUePowerControl RSRP, while
 * referenceSignalPower is configurable by attribute system.
 * NrUePowerControl uses latter values to calculate path loss.
 * When closed loop power control is being used NrUePhy should also
 * pass TPC values to NrUePowerControl.
 *
 */

class NrUePhy;

class NrUePowerControl : public LteUePowerControl
{
public:

  NrUePowerControl ();
  virtual ~NrUePowerControl ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  // inherited from Object
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

  /**
   * \brief Sets a pointer to its NrUePhy instance owner.
   * This is necessary in order to obtain information such as numerology
   * that is used in calculation of tranmit power.
   */
  void Install (const Ptr<NrUePhy>& nrUePhy);

  /// Calculate PUSCH transmit power function
  virtual void CalculatePuschTxPower () override;
  /// Calculate PUCCH transmit power function
  virtual void CalculatePucchTxPower () override;
  /// Calculate SRS transmit power function
  virtual void CalculateSrsTxPower () override;

private:

  Ptr<NrUePhy> m_nrUePhy; //!< NrUePhy instance owner

};

}

#endif /* NR_UE_POWER_CONTROL_H */
