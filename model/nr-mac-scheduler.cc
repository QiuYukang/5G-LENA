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

#include "nr-mac-scheduler.h"
#include "nr-mac-csched-sap.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacScheduler");
NS_OBJECT_ENSURE_REGISTERED (NrMacScheduler);

const uint32_t NrMacScheduler::BufferSizeLevelBsrTable[64] = {
  0, 10, 12, 14, 17, 19, 22, 26, 31, 36, 42, 49, 57, 67, 78, 91,
  107, 125, 146, 171, 200, 234, 274, 321, 376, 440, 515, 603,
  706, 826, 967, 1132, 1326, 1552, 1817, 2127, 2490, 2915, 3413,
  3995, 4677, 5476, 6411, 7505, 8787, 10287, 12043, 14099, 16507,
  19325, 22624, 26487, 31009, 36304, 42502, 49759, 58255,
  68201, 79846, 93749, 109439, 128125, 150000, 150000
};

class NrMacGeneralCschedSapProvider : public NrMacCschedSapProvider
{
public:
  NrMacGeneralCschedSapProvider () = delete;
  NrMacGeneralCschedSapProvider (NrMacScheduler* scheduler)
    : m_scheduler (scheduler)
  {
  }

  ~NrMacGeneralCschedSapProvider () = default;

  // inherited from NrMacCschedSapProvider
  virtual void CschedCellConfigReq (const NrMacCschedSapProvider::CschedCellConfigReqParameters& params)
  {
    m_scheduler->DoCschedCellConfigReq (params);
  }
  virtual void CschedUeConfigReq (const NrMacCschedSapProvider::CschedUeConfigReqParameters& params)
  {
    m_scheduler->DoCschedUeConfigReq (params);
  }
  virtual void CschedLcConfigReq (const NrMacCschedSapProvider::CschedLcConfigReqParameters& params)
  {
    m_scheduler->DoCschedLcConfigReq (params);
  }
  virtual void CschedLcReleaseReq (const NrMacCschedSapProvider::CschedLcReleaseReqParameters& params)
  {
    m_scheduler->DoCschedLcReleaseReq (params);
  }
  virtual void CschedUeReleaseReq (const NrMacCschedSapProvider::CschedUeReleaseReqParameters& params)
  {
    m_scheduler->DoCschedUeReleaseReq (params);
  }

private:
  NrMacScheduler* m_scheduler {nullptr};
};

class NrMacGeneralSchedSapProvider : public NrMacSchedSapProvider
{
public:
  NrMacGeneralSchedSapProvider () = delete;
  NrMacGeneralSchedSapProvider (NrMacScheduler* sched)
    : m_scheduler (sched)
  {
  }

  virtual void SchedDlRlcBufferReq (const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters& params) override
  {
    m_scheduler->DoSchedDlRlcBufferReq (params);
  }
  virtual void SchedDlTriggerReq (const NrMacSchedSapProvider::SchedDlTriggerReqParameters& params) override
  {
    m_scheduler->DoSchedDlTriggerReq (params);
  }
  virtual void SchedUlTriggerReq (const NrMacSchedSapProvider::SchedUlTriggerReqParameters& params) override
  {
    m_scheduler->DoSchedUlTriggerReq (params);
  }
  virtual void SchedDlCqiInfoReq (const NrMacSchedSapProvider::SchedDlCqiInfoReqParameters& params) override
  {
    m_scheduler->DoSchedDlCqiInfoReq (params);
  }
  virtual void SchedUlCqiInfoReq (const NrMacSchedSapProvider::SchedUlCqiInfoReqParameters& params) override
  {
    m_scheduler->DoSchedUlCqiInfoReq (params);
  }
  virtual void SchedUlMacCtrlInfoReq (const NrMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params) override
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
  virtual void SchedDlRachInfoReq (const SchedDlRachInfoReqParameters& params) override
  {
    m_scheduler->DoSchedDlRachInfoReq (params);
  }
  virtual uint8_t GetDlCtrlSyms () const override
  {
    return m_scheduler->GetDlCtrlSyms ();
  }
  virtual uint8_t GetUlCtrlSyms () const override
  {
    return m_scheduler->GetUlCtrlSyms ();
  };
private:
  NrMacScheduler* m_scheduler {nullptr};
};

TypeId
NrMacScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacScheduler")
    .SetParent<Object> ()
  ;

  return tid;
}

NrMacScheduler::NrMacScheduler ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_macSchedSapProvider = new NrMacGeneralSchedSapProvider (this);
  m_macCschedSapProvider = new NrMacGeneralCschedSapProvider (this);
}

NrMacScheduler::~NrMacScheduler ()
{
  NS_LOG_FUNCTION_NOARGS ();
  delete m_macSchedSapProvider;
  m_macSchedSapProvider = nullptr;

  delete m_macCschedSapProvider;
  m_macCschedSapProvider = nullptr;
}

} // namespace ns3


