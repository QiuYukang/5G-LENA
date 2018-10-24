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
 * Based on work done by CTTC/NYU
 */

#include "mmwave-mac-scheduler.h"
#include "mmwave-mac-csched-sap.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveMacScheduler");
NS_OBJECT_ENSURE_REGISTERED (MmWaveMacScheduler);

const uint32_t MmWaveMacScheduler::BufferSizeLevelBsrTable[64] = {
  0, 10, 12, 14, 17, 19, 22, 26, 31, 36, 42, 49, 57, 67, 78, 91,
  107, 125, 146, 171, 200, 234, 274, 321, 376, 440, 515, 603,
  706, 826, 967, 1132, 1326, 1552, 1817, 2127, 2490, 2915, 3413,
  3995, 4677, 5476, 6411, 7505, 8787, 10287, 12043, 14099, 16507,
  19325, 22624, 26487, 31009, 36304, 42502, 49759, 58255,
  68201, 79846, 93749, 109439, 128125, 150000, 150000
};

class MmWaveMacGeneralCschedSapProvider : public MmWaveMacCschedSapProvider
{
public:
  MmWaveMacGeneralCschedSapProvider () = delete;
  MmWaveMacGeneralCschedSapProvider (MmWaveMacScheduler* scheduler)
    : m_scheduler (scheduler)
  {
  }

  ~MmWaveMacGeneralCschedSapProvider () = default;

  // inherited from MmWaveMacCschedSapProvider
  virtual void CschedCellConfigReq (const MmWaveMacCschedSapProvider::CschedCellConfigReqParameters& params)
  {
    m_scheduler->DoCschedCellConfigReq (params);
  }
  virtual void CschedUeConfigReq (const MmWaveMacCschedSapProvider::CschedUeConfigReqParameters& params)
  {
    m_scheduler->DoCschedUeConfigReq (params);
  }
  virtual void CschedLcConfigReq (const MmWaveMacCschedSapProvider::CschedLcConfigReqParameters& params)
  {
    m_scheduler->DoCschedLcConfigReq (params);
  }
  virtual void CschedLcReleaseReq (const MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters& params)
  {
    m_scheduler->DoCschedLcReleaseReq (params);
  }
  virtual void CschedUeReleaseReq (const MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters& params)
  {
    m_scheduler->DoCschedUeReleaseReq (params);
  }

private:
  MmWaveMacScheduler* m_scheduler {nullptr};
};

class MmWaveMacGeneralSchedSapProvider : public MmWaveMacSchedSapProvider
{
public:
  MmWaveMacGeneralSchedSapProvider () = delete;
  MmWaveMacGeneralSchedSapProvider (MmWaveMacScheduler* sched)
    : m_scheduler (sched)
  {
  }

  virtual void SchedDlRlcBufferReq (const MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters& params) override
  {
    m_scheduler->DoSchedDlRlcBufferReq (params);
  }
  virtual void SchedDlTriggerReq (const MmWaveMacSchedSapProvider::SchedDlTriggerReqParameters& params) override
  {
    m_scheduler->DoSchedDlTriggerReq (params);
  }
  virtual void SchedUlTriggerReq (const MmWaveMacSchedSapProvider::SchedUlTriggerReqParameters& params) override
  {
    m_scheduler->DoSchedUlTriggerReq (params);
  }
  virtual void SchedDlCqiInfoReq (const MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters& params) override
  {
    m_scheduler->DoSchedDlCqiInfoReq (params);
  }
  virtual void SchedUlCqiInfoReq (const MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters& params) override
  {
    m_scheduler->DoSchedUlCqiInfoReq (params);
  }
  virtual void SchedUlMacCtrlInfoReq (const MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params) override
  {
    m_scheduler->DoSchedUlMacCtrlInfoReq (params);
  }
  virtual void SchedUlSrInfoReq (const SchedUlSrInfoReqParameters &params) override
  {
    m_scheduler->DoSchedUlSrInfoReq (params);
  }
  virtual void SchedSetMcs (uint32_t mcs) override
  {
    m_scheduler->DoSchedSetMcs (mcs);
  }
private:
  MmWaveMacScheduler* m_scheduler {nullptr};
};

TypeId
MmWaveMacScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveMacScheduler")
    .SetParent<Object> ()
  ;

  return tid;
}

MmWaveMacScheduler::MmWaveMacScheduler ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_macSchedSapProvider = new MmWaveMacGeneralSchedSapProvider (this);
  m_macCschedSapProvider = new MmWaveMacGeneralCschedSapProvider (this);
}

MmWaveMacScheduler::~MmWaveMacScheduler ()
{
  NS_LOG_FUNCTION_NOARGS ();
  delete m_macSchedSapProvider;
  m_macSchedSapProvider = nullptr;

  delete m_macCschedSapProvider;
  m_macCschedSapProvider = nullptr;
}

} // namespace ns3


