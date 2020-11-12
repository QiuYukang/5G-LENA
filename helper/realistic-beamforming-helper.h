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
 * \brief The RealisticBeamformingHelper class that helps user create beamforming tasks
 * and configure when these tasks should be executed. This helper also collects SRS measurements
 * for each gNB and UE. This helper class is currently compatible only with the
 * RealisticBeamformingAlgorithm.
 *
 * Similarly to IdealBeamformingHelper, since there is no real beamforming procedure,
 * there must be a class that will be able to emulate the beamforming procedure, and that is,
 * to update the beamforming vectors of both devices, gNB and UE, at the same time.
 *
 * This class allows setting what will be the trigger event to update the beamfomring vectors.
 * E.g., the trigger event can be that it has been received a certain number of SRSs signals
 * from a UE.
 * This helper saves all SRS reports for each gNB and all of its users, and it is saved per
 * component carrier identified by cellId.
 *
 */
class RealisticBeamformingHelper : public IdealBeamformingHelper
{
public:

  struct SrsSinrReport
  {
    Time time {0}; // srs report time
    double srsSinr {0}; // srs SINR in watts
    double counter {0}; // counter of SRS reports between consecutive beamforming updates
    SrsSinrReport (){}
    SrsSinrReport (Time rTime, double sValue, double c):time (rTime), srsSinr (sValue), counter (c) {};
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

  /**
   * \brief Sets the beamforming update trigger event, trigger event type
   * is one for all the nodes
   * \param triggerEvent triggerEvent type
   */
  void SetTriggerEvent (RealisticBeamformingHelper::TriggerEvent triggerEvent);
  /**
   * \return Returns the trigger event type
   */
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
  /**
   * \brief Adds the beamforming task to the list of tasks
   * \gnbDev gNbDev pointer to gNB device
   * \ueDev ueDev pointer to UE device
   */
  virtual void AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                   const Ptr<NrUeNetDevice>& ueDev) override;
  /**
   * \brief Saves SRS sinr report for this RNTI
   * \param srsSinr
   * \param rnti
   */
  void SaveSrsSinrReport (uint16_t cellId, uint16_t rnti, double srsSinr);
  /**
   * \brief When the condition for triggering a beamforming update is fullfilled
   * this function will be triggered
   * \param cellId id that uniquely identifies the gNB phy
   * \param rnti id that uniquely identifies the user of gNb
   * \param srsSinr value of srsSinr to be passed to RealisticBeamformingAlgorithm
   */
  void TriggerBeamformingAlgorithm (uint16_t cellId, uint16_t rnti, double srsSinr);
  // inherited from IdealBamformingHelper
  virtual void Run () const override;
  virtual void ExpireBeamformingTimer () override;

private:

  bool m_periodicityMode {true}; //!< When true beamforming will be triggered by using srsCountPeriodicity attribute, when false delay attribute will be used
  uint16_t m_srsSinrPeriodicity {3}; //!< Periodicity of beamforming update in number of SRS SINR reports
  Time m_srsToBeamformingDelay {MilliSeconds(0)}; //!< How much time to wait after the last SRS to update the beamforming vectors
  typedef std::unordered_map <uint16_t, SrsSinrReport > SrsReports; //!< List of SRS reports by RNTI
  std::unordered_map< uint16_t, SrsReports > m_srsSinrReportsListsPerCellId; //!< SRS reports per cellId
  TriggerEvent m_triggerEvent; //!< Defines what will be the trigger event for the update of the beamforming vectors

};

}; //ns3 namespace


#endif /* SRC_NR_HELPER_REALISTIC_BEAMFORMING_HELPER_H_ */
