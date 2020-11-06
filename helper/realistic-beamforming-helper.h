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

#include <ns3/object-factory.h>
#include "ideal-beamforming-helper.h"
#include <ns3/realistic-beamforming-algorithm.h>

#ifndef SRC_NR_HELPER_REALISTIC_BEAMFORMING_HELPER_H_
#define SRC_NR_HELPER_REALISTIC_BEAMFORMING_HELPER_H_

namespace ns3 {

class NrGnbNetDevice;
class NrUeNetDevice;
class NrGnbPhy;
class NrUePhy;

/**
 * \ingroup helper
 * \brief The RealisticBeamformingHelper class
 */
class RealisticBeamformingHelper : public IdealBeamformingHelper
{
public:

  struct SrsSinrReport
  {
    Time reportTime {0};
    double srsSinrValue {0};
    double counter {0};

    SrsSinrReport (){}
    SrsSinrReport (Time rTime, double sValue, double c):reportTime (rTime), srsSinrValue(sValue), counter(c) {};

  };

  enum TriggerEvent
  {
    SRS_COUNT,
    DELAYED_UPDATE
  };

  /**
   * \brief IdealBeamformingHelper
   */
  RealisticBeamformingHelper ();
  /**
   * \brief ~IdealBeamformingHelper
   */
  virtual ~RealisticBeamformingHelper ();

  /**
   * \brief Get the Type ID
   * \return the TypeId of the instance
   */
  static TypeId GetTypeId (void);

  void DoInitialize ();

  void SetTriggerEvent (RealisticBeamformingHelper::TriggerEvent triggerEvent);

  RealisticBeamformingHelper::TriggerEvent GetTriggerEvent () const;

  /**
   * \brief Sets the periodicity of the beamforming update in the number of the
   * SRS SINR reports
   */
  void SetSrsCountPeriodicity (uint16_t periodicity);

  /**
   * \returns Gets the periodicity in the number of SRS SINR reports
   */
  uint16_t GetSrsCountPeriodicity () const;
  /**
   * \brief Sets the delay after the SRS SINR report reception and triggering of the
   * beamforming update
   * \param delay the delay after reception of SRS SINR
   */
  void SetSrsToBeamformingDelay (Time delay);

  /**
   * \return returns the delay after sSRS SINR report and beamforming
   */
  Time GetSrsToBeamformingDelay () const;

  virtual void AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                   const Ptr<NrUeNetDevice>& ueDev) override;

  /**
   * \brief Saves SRS sinr report for this RNTI
   * \param srsSinr
   * \param rnti
   */
  void SaveSrsSinrReport (uint16_t cellId, uint16_t rnti, double srsSinr);

  void TriggerBeamformingAlgorithm (uint16_t cellId, uint16_t rnti, double srsSinr);

  virtual void Run () const override;
  virtual void ExpireBeamformingTimer () override;

private:

  virtual void RunTask (const Ptr<NrGnbNetDevice>& gNbDev, const Ptr<NrUeNetDevice>& ueDev,
                        const Ptr<NrGnbPhy>& gNbPhy, const Ptr<NrUePhy>& uePhy, uint8_t ccId) const ;

  bool m_periodicityMode {true};                  //!< When true beamforming will be triggered by using srsCountPeriodicity attribute, when false delay attribute will be used
  uint16_t m_srsSinrPeriodicity {3};             //!< Periodicity of beamforming update in number of SRS SINR reports
  Time m_srsToBeamformingDelay {MilliSeconds(0)}; //!< How much time to wait after the last SRS to update the beamforming vectors
  std::unordered_map<uint16_t, SrsSinrReport > m_srsSinrReportsPerUe;
  TriggerEvent m_triggerEvent; //!< Defines what will be the trigger event for the update of the beamforming vectors

};

}; //ns3 namespace


#endif /* SRC_NR_HELPER_REALISTIC_BEAMFORMING_HELPER_H_ */
