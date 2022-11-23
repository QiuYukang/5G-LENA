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

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrMacScheduler");
NS_OBJECT_ENSURE_REGISTERED(NrMacScheduler);

class NrMacGeneralCschedSapProvider : public NrMacCschedSapProvider
{
  public:
    NrMacGeneralCschedSapProvider() = delete;

    NrMacGeneralCschedSapProvider(NrMacScheduler* scheduler)
        : m_scheduler(scheduler)
    {
    }

    ~NrMacGeneralCschedSapProvider() override = default;

    // inherited from NrMacCschedSapProvider
    void CschedCellConfigReq(
        const NrMacCschedSapProvider::CschedCellConfigReqParameters& params) override
    {
        m_scheduler->DoCschedCellConfigReq(params);
    }

    void CschedUeConfigReq(
        const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) override
    {
        m_scheduler->DoCschedUeConfigReq(params);
    }

    void CschedLcConfigReq(
        const NrMacCschedSapProvider::CschedLcConfigReqParameters& params) override
    {
        m_scheduler->DoCschedLcConfigReq(params);
    }

    void CschedLcReleaseReq(
        const NrMacCschedSapProvider::CschedLcReleaseReqParameters& params) override
    {
        m_scheduler->DoCschedLcReleaseReq(params);
    }

    void CschedUeReleaseReq(
        const NrMacCschedSapProvider::CschedUeReleaseReqParameters& params) override
    {
        m_scheduler->DoCschedUeReleaseReq(params);
    }

  private:
    NrMacScheduler* m_scheduler{nullptr};
};

class NrMacGeneralSchedSapProvider : public NrMacSchedSapProvider
{
  public:
    NrMacGeneralSchedSapProvider() = delete;

    NrMacGeneralSchedSapProvider(NrMacScheduler* sched)
        : m_scheduler(sched)
    {
    }

    void SchedDlRlcBufferReq(
        const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters& params) override
    {
        m_scheduler->DoSchedDlRlcBufferReq(params);
    }

    void SchedDlTriggerReq(
        const NrMacSchedSapProvider::SchedDlTriggerReqParameters& params) override
    {
        m_scheduler->DoSchedDlTriggerReq(params);
    }

    void SchedUlTriggerReq(
        const NrMacSchedSapProvider::SchedUlTriggerReqParameters& params) override
    {
        m_scheduler->DoSchedUlTriggerReq(params);
    }

    void SchedDlCqiInfoReq(
        const NrMacSchedSapProvider::SchedDlCqiInfoReqParameters& params) override
    {
        m_scheduler->DoSchedDlCqiInfoReq(params);
    }

    void SchedUlCqiInfoReq(
        const NrMacSchedSapProvider::SchedUlCqiInfoReqParameters& params) override
    {
        m_scheduler->DoSchedUlCqiInfoReq(params);
    }

    void SchedUlMacCtrlInfoReq(
        const NrMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params) override
    {
        m_scheduler->DoSchedUlMacCtrlInfoReq(params);
    }

    void SchedUlSrInfoReq(const SchedUlSrInfoReqParameters& params) override
    {
        m_scheduler->DoSchedUlSrInfoReq(params);
    }

    void SchedSetMcs(uint32_t mcs) override
    {
        m_scheduler->DoSchedSetMcs(mcs);
    }

    void SchedDlRachInfoReq(const SchedDlRachInfoReqParameters& params) override
    {
        m_scheduler->DoSchedDlRachInfoReq(params);
    }

    uint8_t GetDlCtrlSyms() const override
    {
        return m_scheduler->GetDlCtrlSyms();
    }

    uint8_t GetUlCtrlSyms() const override
    {
        return m_scheduler->GetUlCtrlSyms();
    };

  private:
    NrMacScheduler* m_scheduler{nullptr};
};

TypeId
NrMacScheduler::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrMacScheduler").SetParent<Object>();

    return tid;
}

NrMacScheduler::NrMacScheduler()
{
    NS_LOG_FUNCTION_NOARGS();
    m_macSchedSapProvider = new NrMacGeneralSchedSapProvider(this);
    m_macCschedSapProvider = new NrMacGeneralCschedSapProvider(this);
}

NrMacScheduler::~NrMacScheduler()
{
    NS_LOG_FUNCTION_NOARGS();
    delete m_macSchedSapProvider;
    m_macSchedSapProvider = nullptr;

    delete m_macCschedSapProvider;
    m_macCschedSapProvider = nullptr;
}

} // namespace ns3
