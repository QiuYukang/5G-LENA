/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
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
#pragma once

#include "mmwave-phy-mac-common.h"
#include "mmwave-mac-scheduler-ue-info.h"
#include <memory>

namespace ns3 {

class MmWaveAmc;

/**
 * \ingroup mac-schedulers
 * \brief CQI management for schedulers.
 *
 * The scheduler will call either DlWBCQIReported or DlSBCQIReported to calculate
 * a new DL MCS. For UL, only the method UlSBCQIReported is implemented,
 * and it is a bit more complicated. For any detail, check the respective
 * documentation.
 *
 * \see UlSBCQIReported
 * \see DlWBCQIReported
 */
class MmWaveMacSchedulerCQIManagement
{
public:
  /**
   * \brief MmWaveMacSchedulerCQIManagement default constructor
   */
  MmWaveMacSchedulerCQIManagement () = default;

  /**
   * \brief MmWaveMacSchedulerCQIManagement copy constructor (deleted)
   * \param o other instance
   */
  MmWaveMacSchedulerCQIManagement (const MmWaveMacSchedulerCQIManagement &o) = delete;
  /**
    * \brief Deconstructor
    */
  ~MmWaveMacSchedulerCQIManagement () = default;

  /**
   * \brief Set the pointer to the MmWaveAmc model
   * \param config PhyMac config
   * \param amc AMC model to calculate CQI
   * \param startMcsDl Default MCS when a CQI is reset (DL)
   * \param startMcsUl Default MCS when a CQI is reset (UL)
   */
  void
  ConfigureCommonParameters (const Ptr<MmWavePhyMacCommon> &config,
                             const Ptr<MmWaveAmc> &amc,
                             uint8_t startMcsDl, uint8_t startMcsUl)
  {
    m_phyMacConfig = config;
    m_amc = amc;
    m_startMcsDl = startMcsDl;
    m_startMcsUl = startMcsUl;
  }

  /**
   * \brief A wideband CQI has been reported for the specified UE
   * \param info WB CQI
   * \param ueInfo UE
   * \param expirationTime expiration time of the CQI in number of slot
   *
   * Store the CQI information inside the m_dlCqi value of the UE, and then
   * calculate the corresponding MCS through MmWaveAmc. The information is
   * contained in the structure DlCqiInfo, so no need to make calculation
   * here.
   */
  void DlWBCQIReported (const DlCqiInfo &info, const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo,
                        uint32_t expirationTime) const;
  /**
   * \brief SB CQI reported
   * \param info SB CQI
   * \param ueInfo UE
   *
   * NOT IMPLEMENTED
   */
  void DlSBCQIReported (const DlCqiInfo &info, const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo) const;

  /**
   * \brief An UL SB CQI has been reported for the specified UE
   * \param expirationTime expiration time (in slot) of the CQI value
   * \param numSym symbols allocated
   * \param tbs TBS of the allocation
   * \param params parameters of the received CQI
   * \param ueInfo UE info
   *
   * To calculate the UL MCS, is necessary to remember the allocation done to
   * be able to retrieve the number of symbols and the TBS assigned. This is
   * done inside the class MmWaveMacSchedulerNs3, and here we assume correct
   * parameters as input.
   *
   * From a vector of SINR (along the entire band) a SpectrumValue is calculated
   * and then passed as input to MmWaveAmc::CreateCqiFeedbackWbTdma. From this
   * function, we have as a result an updated value of CQI, as well as an updated
   * version of MCS for the UL.
   */
  void UlSBCQIReported (uint32_t expirationTime, uint8_t numSym, uint32_t tbs,
                        const MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params,
                        const std::shared_ptr<MmWaveMacSchedulerUeInfo> &ueInfo) const;

  /**
   * \brief Refresh the DL CQI for all the UE
   *
   * This method should be called every slot.
   * Decrement the validity counter DL CQI, and if a CQI expires, reset its
   * value to the default (MCS 0)
   *
   * \param m_ueMap UE map
   */
  void RefreshDlCqiMaps (const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &m_ueMap) const;

  /**
   * \brief Refresh the UL CQI for all the UE
   *
   * This method should be called every slot.
   * Decrement the validity counter UL CQI, and if a CQI expires, reset its
   * value to the default (MCS 0)
   *
   * \param m_ueMap UE map
   */
  void RefreshUlCqiMaps (const std::unordered_map<uint16_t, std::shared_ptr<MmWaveMacSchedulerUeInfo> > &m_ueMap) const;

private:
  Ptr<MmWavePhyMacCommon> m_phyMacConfig; //!< PhyMac config
  Ptr<MmWaveAmc> m_amc;                   //!< MmWaveAmc model pointer
  uint8_t m_startMcsDl {0};
  uint8_t m_startMcsUl {0};
};

} // namespace ns3
