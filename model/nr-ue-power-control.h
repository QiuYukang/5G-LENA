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
 * \brief This class implements NR Uplink Power Control functionality.
 * Can operate in two different modes: following specification TS 36.213
 * that is used in LTE, LAA, etc; or  following specification TS 38.213
 * for New Radio technology.
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
 * Specification that are used to implement uplink power control feature are
 * the latest available specifications for LTE and NR:
 * 1) ETSI TS 136 213 V14.2.0 (2017-04)
 * 2) ETSI TS 138 213 V15.6.0 (2019-07)
 *
 */

class NrUePhy;

class NrUePowerControl : public LteUePowerControl
{
public:

  /**
   * Power control supports two technical specifications:
   * 1) TS 36.213, for LTE, LAA, etc.
   * 1) TS 38.213, for New Radio (NR)
   *
   */
  enum TechnicalSpec {
     TS_36_213,
     TS_38_213,
  };

  NrUePowerControl ();
  /**
   * \brief Constructor that sets a pointer to its NrUePhy instance owner.
   * This is necessary in order to obtain information such as numerology
   * that is used in calculation of tranmit power.
   */
  NrUePowerControl (const Ptr<NrUePhy>& nrUePhy);

  virtual ~NrUePowerControl ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  // inherited from Object
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

  /*
   * \brief Sets technical specification according to which will
   * be calculated power
   * \param ts technical specification to be used
   */
  void SetTechnicalSpec (NrUePowerControl::TechnicalSpec ts);

  /**
   * \brief Sets KPusch
   * \param kPusch KPUSCH value to be used in PUSCH transmit power
   */
  void SetKPusch (uint16_t kPusch);

  /**
   * \brief Sets KPucch
   * \param kPusch KPUCCH value to be used in PUSCH transmit power
   */
  void SetK0Pucch (uint16_t kPusch);

  /*
   * \brief Sets weather the device for which is configure this
   *  uplink power control algorithm is for device that is
   *  bandwidth reduced low complexity device or coverage enhanced (BL/CE)
   *  device
   * \param blCe an indicator telling whether device is BL/CE or not
   */
  void SetBlCe (bool blCe);

  /**
   * \brief Sets P0 SRS parameter for calculation of SRS power control
   * \param p0srs value to be set
   */
  void SetP0Srs (bool p0srs);

  /*
   * \brief Set PO nominal PUCCH value
   * \param value the value to set
   */
  void SetPoNominalPucch (int16_t value);

  /*
  * \brief Set PO PUCCH value
  * \param value the value to set
  */
  void SetPoUePucch (int16_t value);

  /**
   * \brief Implements calculation of PUSCH
   * power control according to TS 36.213 and TS 38.213.
   * Overloads instead overrides LteUePowerControl function
   * in order to avoid pass by copy of RB vector.
   * \param rbNum number of RBs used for PUSCH
   */
  double GetPuschTxPower (std::size_t rbNum);

  /**
   * \brief Implements calculation of PUCCH
   * power control according to TS 36.213 and TS 38.213.
   * Overloads instead overrides LteUePowerControl function
   * in order to avoid pass by copy of RB vector.
   * \param rbNum number of RBs used for PUCCH
   */
  double GetPucchTxPower (std::size_t rbNum);

  /**
   * \brief Implements calculation of SRS
   * power control according to TS 36.213 and TS 38.213.
   * Overloads instead overrides LteUePowerControl function
   * in order to avoid pass by copy of RB vector
   * \param rbNum number of RBs used for SRS
   */
  double GetSrsTxPower (std::size_t rbNum);

  /**
   * \brief Function that is called by NrUePhy
   * to notify NrUePowerControl algorithm
   * that TPC command was received by gNB
   * \param tpc the TPC command
   */
  virtual void ReportTpcPusch (uint8_t tpc);

  /**
     * \brief Function that is called by NrUePhy
     * to notify NrUePowerControl algorithm
     * that TPC command for PUCCH was received by gNB
     * \param tpc the TPC command
     */
  virtual void ReportTpcPucch (uint8_t tpc);

private:

  /*
    * \brief Implements conversion from TPC
    * command to absolute delta value. Follows both,
    * TS 36.213 and TS 38.213 specification for PUSCH.
    * In 36.213 table is Table 5.1.1.1-2. and
    * in 38.213 the table is Table 7.1.1-1.
    * \param tpc TPC command value from 0 to 3
    */
   int GetAbsoluteDelta (uint8_t tpc) const;

   /*
    * \brief Implements conversion from TPC
    * command to accumulated delta value. Follows both,
    * TS 36.213 and TS 38.213 specification for PUSCH
    * and PUCCH. In 36.213 tables are Table 5.1.1.1-2.
    * and Table 5.1.2.1-1;
    * while in 38.213 tables are:
    * Table 7.1.1-1 and Table 7.2.1-1.
    * \param tpc TPC command value from 0 to 3
    */
   int GetAccumulatedDelta (uint8_t tpc) const;

   /*
    * \brief Calculates fc value for PUSCH power control
    * according to TS 38.213 7.2.1 formulas.
    */
   void UpdateFc ();

   /*
    * \brief Calculate gc value for PUCCH power control
    * according to TS 38.213 7.2.1 formulas.
    */
   void UpdateGc ();

   /**
    * \brief Calculates PUSCH transmit power
    * according TS 38.213 7.1.1 formulas
    */
  double CalculatePuschTxPowerNr ();

   /**
    * \brief Calculates PUCCH transmit power
    * according TS 38.213 7.2.1 formulas
    */
  double CalculatePucchTxPowerNr ();

   /**
    * \brief Calculates SRS transmit power
    */
  double CalculateSrsTxPowerNr ();


  TechnicalSpec m_technicalSpec;          //!< Technical specification to be used for transmit power calculations
  Ptr<NrUePhy> m_nrUePhy;                 //!< NrUePhy instance owner
  std::vector<int16_t> m_PoNominalPucch;  //!< PO nominal PUCCH
  std::vector<int16_t> m_PoUePucch;       //!< PO US PUCCH
  uint16_t m_M_Pucch {0};                 //!< size of RB list
  double m_delta_F_Pucch {0.0};           //!< Delta F_PUCCH to calculate 38.213 7.2.1 formula for PUCCH transmit power
  double m_deltaTF_control {0.0};         //!< PUCCH transmission power adjustment component for UL BWP of carrier of primary cell
  std::vector <uint8_t> m_deltaPucch;     //!< vector that saves TPC command accumulated values for PUCCH transmit power calculation
  double m_gc {0.0};                      //!< Is the current PUCCH power control adjustment state. This variable is used for calculation of PUCCH transmit power.
  double m_hc {0.0};                      //!< Is the current SRS power control adjustment state. This variable is used for calculation of SRS transmit power.
  uint16_t m_k_PUSCH {0};                 //!< One of the principal parameters for the calculation of the PUSCH pc accumulation state m_fc
  uint16_t m_k_PUCCH {0};                 //!< One of the principal parameters for the calculation of the PUCCH pc accumulation state m_gc
  bool m_blCe {false};                    /*!< When set to true means that this power control is applied to bandwidth reduced,
                                          low complexity or coverage enhanced device.By default this attribute is set to false.
                                          Default BL/CE mode is CEModeB.
                                          */
  double m_P_0_SRS{0.0};                  //!< P_0_SRS parameter for calculation of SRS power control

};

}

#endif /* NR_UE_POWER_CONTROL_H */
