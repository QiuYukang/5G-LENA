/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
// Copyright (c) 2015 NYU WIRELESS, Tandon School of Engineering, New York University
// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#define NS_LOG_APPEND_CONTEXT                                                                      \
    do                                                                                             \
    {                                                                                              \
        std::clog << " [ CellId " << GetCellId() << ", bwpId " << GetBwpId() << "] ";              \
    } while (false);

#include "nr-ue-phy.h"

#include "beam-manager.h"
#include "nr-ch-access-manager.h"
#include "nr-ue-net-device.h"
#include "nr-ue-power-control.h"

#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/log.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/node.h>
#include <ns3/nr-sl-comm-resource-pool.h>
#include <ns3/object-vector.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>

#include <algorithm>
#include <cfloat>

namespace ns3
{

const Time NR_DEFAULT_PMI_INTERVAL_WB{MilliSeconds(10)}; // Wideband PMI update interval
const Time NR_DEFAULT_PMI_INTERVAL_SB{MilliSeconds(2)};  // Subband PMI update interval

NS_LOG_COMPONENT_DEFINE("NrUePhy");
NS_OBJECT_ENSURE_REGISTERED(NrUePhy);

NrUePhy::NrUePhy()
{
    NS_LOG_FUNCTION(this);
    m_wbCqiLast = Simulator::Now();
    m_ueCphySapProvider = new MemberLteUeCphySapProvider<NrUePhy>(this);
    m_powerControl = CreateObject<NrUePowerControl>(this);
    m_nrSlUeCphySapProvider = new MemberNrSlUeCphySapProvider<NrUePhy>(this);
    DoReset();

    Simulator::Schedule(m_ueMeasurementsFilterPeriod, &NrUePhy::ReportUeMeasurements, this);
}

NrUePhy::~NrUePhy()
{
    NS_LOG_FUNCTION(this);
    m_slHarqFbList.clear();
}

void
NrUePhy::DoDispose()
{
    NS_LOG_FUNCTION(this);
    delete m_ueCphySapProvider;
    if (m_powerControl)
    {
        m_powerControl->Dispose();
        m_powerControl = nullptr;
    }
    if (m_cam)
    {
        m_cam->Dispose();
        m_cam = nullptr;
    }
    delete m_nrSlUeCphySapProvider;
    m_slTxPool = nullptr;
    m_slRxPool = nullptr;
    NrPhy::DoDispose();
}

TypeId
NrUePhy::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrUePhy")
            .SetParent<NrPhy>()
            .AddConstructor<NrUePhy>()
            .AddAttribute("TxPower",
                          "Transmission power in dBm",
                          DoubleValue(2.0),
                          MakeDoubleAccessor(&NrUePhy::m_txPower),
                          MakeDoubleChecker<double>())
            .AddAttribute(
                "NoiseFigure",
                "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                "\"the difference in decibels (dB) between"
                " the noise output of the actual receiver to the noise output of an "
                " ideal receiver with the same overall gain and bandwidth when the receivers "
                " are connected to sources at the standard noise temperature T0.\" "
                "In this model, we consider T0 = 290K.",
                DoubleValue(5.0), // nr code from NYU and UniPd assumed in the code the value of
                                  // 5dB, that is why we configure the default value to that
                MakeDoubleAccessor(&NrPhy::SetNoiseFigure, &NrPhy::GetNoiseFigure),
                MakeDoubleChecker<double>())
            .AddAttribute(
                "PowerAllocationType",
                "Defines the type of the power allocation. Currently are supported "
                "two types: \"UniformPowerAllocBw\", which is a uniform power allocation over all "
                "bandwidth (over all RBs), and \"UniformPowerAllocBw\", which is a uniform "
                "power allocation over used (active) RBs. By default is set a uniform power "
                "allocation over used RBs .",
                EnumValue(NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED),
                MakeEnumAccessor<NrSpectrumValueHelper::PowerAllocationType>(
                    &NrPhy::SetPowerAllocationType,
                    &NrPhy::GetPowerAllocationType),
                MakeEnumChecker(NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW,
                                "UniformPowerAllocBw",
                                NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED,
                                "UniformPowerAllocUsed"))
            .AddAttribute("SpectrumPhy",
                          "The SpectrumPhy associated to this NrPhy",
                          TypeId::ATTR_GET,
                          PointerValue(),
                          MakePointerAccessor(&NrPhy::GetSpectrumPhy),
                          MakePointerChecker<NrSpectrumPhy>())
            .AddAttribute("LBTThresholdForCtrl",
                          "After a DL/UL transmission, if we have less than this value to send the "
                          "UL CTRL, we consider the channel as granted",
                          TimeValue(MicroSeconds(25)),
                          MakeTimeAccessor(&NrUePhy::m_lbtThresholdForCtrl),
                          MakeTimeChecker())
            .AddAttribute("TbDecodeLatency",
                          "Transport block decode latency",
                          TimeValue(MicroSeconds(100)),
                          MakeTimeAccessor(&NrPhy::SetTbDecodeLatency, &NrPhy::GetTbDecodeLatency),
                          MakeTimeChecker())
            .AddAttribute("RsrpFilterPeriod",
                          "L1 Filter period for RSRP",
                          TimeValue(MilliSeconds(200)),
                          MakeTimeAccessor(&NrUePhy::m_rsrpFilterPeriod),
                          MakeTimeChecker())
            .AddAttribute("EnableUplinkPowerControl",
                          "If true, Uplink Power Control will be enabled.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&NrUePhy::SetEnableUplinkPowerControl),
                          MakeBooleanChecker())
            .AddAttribute("WbPmiUpdateInterval",
                          "Wideband PMI update interval",
                          TimeValue(NR_DEFAULT_PMI_INTERVAL_WB),
                          MakeTimeAccessor(&NrUePhy::m_wbPmiUpdateInterval),
                          MakeTimeChecker())
            .AddAttribute("SbPmiUpdateInterval",
                          "Subband PMI update interval",
                          TimeValue(NR_DEFAULT_PMI_INTERVAL_SB),
                          MakeTimeAccessor(&NrUePhy::m_sbPmiUpdateInterval),
                          MakeTimeChecker())
            .AddTraceSource("DlDataSinr",
                            "DL DATA SINR statistics.",
                            MakeTraceSourceAccessor(&NrUePhy::m_dlDataSinrTrace),
                            "ns3::NrUePhy::DlDataSinrTracedCallback")
            .AddTraceSource("DlCtrlSinr",
                            "Report the SINR computed for DL CTRL",
                            MakeTraceSourceAccessor(&NrUePhy::m_dlCtrlSinrTrace),
                            "ns3::NrUePhy::DlCtrlSinrTracedCallback")
            .AddAttribute("UeMeasurementsFilterPeriod",
                          "Time period for reporting UE measurements, i.e., the"
                          "length of layer-1 filtering.",
                          TimeValue(MilliSeconds(200)),
                          MakeTimeAccessor(&NrUePhy::m_ueMeasurementsFilterPeriod),
                          MakeTimeChecker())
            .AddTraceSource("ReportUplinkTbSize",
                            "Report allocated uplink TB size for trace.",
                            MakeTraceSourceAccessor(&NrUePhy::m_reportUlTbSize),
                            "ns3::UlTbSize::TracedCallback")
            .AddTraceSource("ReportDownlinkTbSize",
                            "Report allocated downlink TB size for trace.",
                            MakeTraceSourceAccessor(&NrUePhy::m_reportDlTbSize),
                            "ns3::DlTbSize::TracedCallback")
            .AddTraceSource("ReportRsrp",
                            "RSRP statistics.",
                            MakeTraceSourceAccessor(&NrUePhy::m_reportRsrpTrace),
                            "ns3::CurrentRsrp::TracedCallback")
            .AddTraceSource("UePhyRxedCtrlMsgsTrace",
                            "Ue PHY Control Messages Traces.",
                            MakeTraceSourceAccessor(&NrUePhy::m_phyRxedCtrlMsgsTrace),
                            "ns3::NrPhyRxTrace::RxedUePhyCtrlMsgsTracedCallback")
            .AddTraceSource("UePhyTxedCtrlMsgsTrace",
                            "Ue PHY Control Messages Traces.",
                            MakeTraceSourceAccessor(&NrUePhy::m_phyTxedCtrlMsgsTrace),
                            "ns3::NrPhyRxTrace::TxedUePhyCtrlMsgsTracedCallback")
            .AddTraceSource("UePhyRxedDlDciTrace",
                            "Ue PHY DL DCI Traces.",
                            MakeTraceSourceAccessor(&NrUePhy::m_phyUeRxedDlDciTrace),
                            "ns3::NrPhyRxTrace::RxedUePhyDlDciTracedCallback")
            .AddTraceSource("UePhyTxedHarqFeedbackTrace",
                            "Ue PHY DL HARQ Feedback Traces.",
                            MakeTraceSourceAccessor(&NrUePhy::m_phyUeTxedHarqFeedbackTrace),
                            "ns3::NrPhyRxTrace::TxedUePhyHarqFeedbackTracedCallback")
            .AddTraceSource("ReportPowerSpectralDensity",
                            "Power Spectral Density data.",
                            MakeTraceSourceAccessor(&NrUePhy::m_reportPowerSpectralDensity),
                            "ns3::NrUePhy::PowerSpectralDensityTracedCallback");
    return tid;
}

void
NrUePhy::ChannelAccessGranted([[maybe_unused]] const Time& time)
{
    NS_LOG_FUNCTION(this);
    // That will be granted only till the end of the slot
    m_channelStatus = GRANTED;
}

void
NrUePhy::ChannelAccessDenied()
{
    NS_LOG_FUNCTION(this);
    m_channelStatus = NONE;
}

void
NrUePhy::SetUeCphySapUser(LteUeCphySapUser* s)
{
    NS_LOG_FUNCTION(this);
    m_ueCphySapUser = s;
}

LteUeCphySapProvider*
NrUePhy::GetUeCphySapProvider()
{
    NS_LOG_FUNCTION(this);
    return (m_ueCphySapProvider);
}

void
NrUePhy::SetEnableUplinkPowerControl(bool enable)
{
    m_enableUplinkPowerControl = enable;
}

void
NrUePhy::SetTxPower(double pow)
{
    m_txPower = pow;
    m_powerControl->SetTxPower(pow);
}

double
NrUePhy::GetTxPower() const
{
    return m_txPower;
}

double
NrUePhy::GetRsrp() const
{
    return m_rsrp;
}

Ptr<NrUePowerControl>
NrUePhy::GetUplinkPowerControl() const
{
    NS_LOG_FUNCTION(this);
    return m_powerControl;
}

void
NrUePhy::SetUplinkPowerControl(Ptr<NrUePowerControl> pc)
{
    m_powerControl = pc;
}

void
NrUePhy::SetDlAmc(const Ptr<const NrAmc>& amc)
{
    m_amc = amc;

    if (m_pmSearch)
    {
        m_pmSearch->SetAmc(amc);
    }
}

void
NrUePhy::SetSubChannelsForTransmission(const std::vector<int>& mask, uint32_t numSym)
{
    Ptr<SpectrumValue> txPsd = GetTxPowerSpectralDensity(mask);
    NS_ASSERT(txPsd);

    m_reportPowerSpectralDensity(m_currentSlot,
                                 txPsd,
                                 numSym * GetSymbolPeriod(),
                                 m_rnti,
                                 m_imsi,
                                 GetBwpId(),
                                 GetCellId());
    m_spectrumPhy->SetTxPowerSpectralDensity(txPsd);
}

void
NrUePhy::DoSendControlMessage(Ptr<NrControlMessage> msg)
{
    NS_LOG_FUNCTION(this << msg);
    EnqueueCtrlMessage(msg);
}

void
NrUePhy::DoSendControlMessageNow(Ptr<NrControlMessage> msg)
{
    NS_LOG_FUNCTION(this << msg);
    EnqueueCtrlMsgNow(msg);
}

void
NrUePhy::ProcessDataDci(const SfnSf& ulSfnSf,
                        const std::shared_ptr<DciInfoElementTdma>& dciInfoElem)
{
    NS_LOG_FUNCTION(this);

    NS_LOG_DEBUG("UE" << m_rnti << " UL-DCI received for slot " << ulSfnSf << " symStart "
                      << static_cast<uint32_t>(dciInfoElem->m_symStart) << " numSym "
                      << static_cast<uint32_t>(dciInfoElem->m_numSym) << " tbs "
                      << dciInfoElem->m_tbSize << " harqId "
                      << static_cast<uint32_t>(dciInfoElem->m_harqProcess));

    if (ulSfnSf == m_currentSlot)
    {
        InsertAllocation(dciInfoElem);
    }
    else
    {
        InsertFutureAllocation(ulSfnSf, dciInfoElem);
    }
}

void
NrUePhy::ProcessSrsDci([[maybe_unused]] const SfnSf& ulSfnSf,
                       [[maybe_unused]] const std::shared_ptr<DciInfoElementTdma>& dciInfoElem)
{
    NS_LOG_FUNCTION(this);
    // Instruct PHY for transmitting the SRS
    if (ulSfnSf == m_currentSlot)
    {
        InsertAllocation(dciInfoElem);
    }
    else
    {
        InsertFutureAllocation(ulSfnSf, dciInfoElem);
    }
}

void
NrUePhy::RegisterToEnb(uint16_t bwpId)
{
    NS_LOG_FUNCTION(this);

    InitializeMessageList();
    DoSetCellId(bwpId);
}

void
NrUePhy::SetUlCtrlSyms(uint8_t ulCtrlSyms)
{
    m_ulCtrlSyms = ulCtrlSyms;
}

void
NrUePhy::SetDlCtrlSyms(uint8_t dlCtrlSyms)
{
    m_dlCtrlSyms = dlCtrlSyms;
}

void
NrUePhy::SetNumRbPerRbg(uint32_t numRbPerRbg)
{
    m_numRbPerRbg = numRbPerRbg;
}

void
NrUePhy::SetPattern(const std::string& pattern)
{
    NS_LOG_FUNCTION(this);

    static std::unordered_map<std::string, LteNrTddSlotType> lookupTable = {
        {"DL", LteNrTddSlotType::DL},
        {"UL", LteNrTddSlotType::UL},
        {"S", LteNrTddSlotType::S},
        {"F", LteNrTddSlotType::F},
    };

    std::vector<LteNrTddSlotType> vector;
    std::stringstream ss(pattern);
    std::string token;
    std::vector<std::string> extracted;

    while (std::getline(ss, token, '|'))
    {
        extracted.push_back(token);
    }

    vector.reserve(extracted.size());
    for (const auto& v : extracted)
    {
        vector.push_back(lookupTable[v]);
    }

    m_tddPattern = vector;
}

uint32_t
NrUePhy::GetNumRbPerRbg() const
{
    return m_numRbPerRbg;
}

double
NrUePhy::ComputeAvgSinr(const SpectrumValue& sinr)
{
    // averaged SINR among RBs
    double sum = 0.0;
    uint8_t rbNum = 0;
    Values::const_iterator it;

    for (it = sinr.ConstValuesBegin(); it != sinr.ConstValuesEnd(); it++)
    {
        sum += (*it);
        rbNum++;
    }

    double avrgSinr = (rbNum > 0) ? (sum / rbNum) : DBL_MAX;

    return avrgSinr;
}

void
NrUePhy::InsertAllocation(const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);

    VarTtiAllocInfo varTtiInfo(dci);
    m_currSlotAllocInfo.m_varTtiAllocInfo.push_back(varTtiInfo);
    std::sort(m_currSlotAllocInfo.m_varTtiAllocInfo.begin(),
              m_currSlotAllocInfo.m_varTtiAllocInfo.end());
}

void
NrUePhy::InsertFutureAllocation(const SfnSf& sfnSf, const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);

    VarTtiAllocInfo varTtiInfo(dci);
    if (SlotAllocInfoExists(sfnSf))
    {
        auto& ulSlot = PeekSlotAllocInfo(sfnSf);
        ulSlot.m_varTtiAllocInfo.push_back(varTtiInfo);
        std::sort(ulSlot.m_varTtiAllocInfo.begin(), ulSlot.m_varTtiAllocInfo.end());
    }
    else
    {
        SlotAllocInfo slotAllocInfo = SlotAllocInfo(sfnSf);
        slotAllocInfo.m_varTtiAllocInfo.push_back(varTtiInfo);
        PushBackSlotAllocInfo(slotAllocInfo);
    }
}

void
NrUePhy::PhyCtrlMessagesReceived(const Ptr<NrControlMessage>& msg)
{
    NS_LOG_FUNCTION(this);

    if (msg->GetMessageType() == NrControlMessage::DL_DCI)
    {
        auto dciMsg = DynamicCast<NrDlDciMessage>(msg);
        auto dciInfoElem = dciMsg->GetDciInfoElement();

        m_phyRxedCtrlMsgsTrace(m_currentSlot, GetCellId(), m_rnti, GetBwpId(), msg);

        if (dciInfoElem->m_rnti != 0 && dciInfoElem->m_rnti != m_rnti)
        {
            return; // DCI not for me
        }

        SfnSf dciSfn = m_currentSlot;
        uint32_t k0Delay = dciMsg->GetKDelay();
        dciSfn.Add(k0Delay);

        NS_LOG_DEBUG("UE" << m_rnti << " DL-DCI received for slot " << dciSfn << " symStart "
                          << static_cast<uint32_t>(dciInfoElem->m_symStart) << " numSym "
                          << static_cast<uint32_t>(dciInfoElem->m_numSym) << " tbs "
                          << dciInfoElem->m_tbSize << " harqId "
                          << static_cast<uint32_t>(dciInfoElem->m_harqProcess));

        /* BIG ASSUMPTION: We assume that K0 is always 0 */

        auto it = m_harqIdToK1Map.find(dciInfoElem->m_harqProcess);
        if (it != m_harqIdToK1Map.end())
        {
            m_harqIdToK1Map.erase(m_harqIdToK1Map.find(dciInfoElem->m_harqProcess));
        }

        m_harqIdToK1Map.insert(std::make_pair(dciInfoElem->m_harqProcess, dciMsg->GetK1Delay()));

        m_phyUeRxedDlDciTrace(m_currentSlot,
                              GetCellId(),
                              m_rnti,
                              GetBwpId(),
                              dciInfoElem->m_harqProcess,
                              dciMsg->GetK1Delay());

        InsertAllocation(dciInfoElem);

        m_phySapUser->ReceiveControlMessage(msg);

        if (m_enableUplinkPowerControl)
        {
            m_powerControl->ReportTpcPusch(dciInfoElem->m_tpc);
            m_powerControl->ReportTpcPucch(dciInfoElem->m_tpc);
        }
    }
    else if (msg->GetMessageType() == NrControlMessage::UL_DCI)
    {
        auto dciMsg = DynamicCast<NrUlDciMessage>(msg);
        auto dciInfoElem = dciMsg->GetDciInfoElement();

        m_phyRxedCtrlMsgsTrace(m_currentSlot, GetCellId(), m_rnti, GetBwpId(), msg);

        if (dciInfoElem->m_rnti != 0 && dciInfoElem->m_rnti != m_rnti)
        {
            return; // DCI not for me
        }

        SfnSf ulSfnSf = m_currentSlot;
        uint32_t k2Delay = dciMsg->GetKDelay();
        ulSfnSf.Add(k2Delay);

        if (dciInfoElem->m_type == DciInfoElementTdma::DATA)
        {
            ProcessDataDci(ulSfnSf, dciInfoElem);
            m_phySapUser->ReceiveControlMessage(msg);
        }
        else if (dciInfoElem->m_type == DciInfoElementTdma::SRS)
        {
            ProcessSrsDci(ulSfnSf, dciInfoElem);
            // Do not pass the DCI to MAC
        }
    }
    else if (msg->GetMessageType() == NrControlMessage::MIB)
    {
        NS_LOG_DEBUG("received MIB");
        Ptr<NrMibMessage> msg2 = DynamicCast<NrMibMessage>(msg);
        m_phyRxedCtrlMsgsTrace(m_currentSlot, GetCellId(), m_rnti, GetBwpId(), msg);
        m_ueCphySapUser->RecvMasterInformationBlock(GetCellId(), msg2->GetMib());
    }
    else if (msg->GetMessageType() == NrControlMessage::SIB1)
    {
        Ptr<NrSib1Message> msg2 = DynamicCast<NrSib1Message>(msg);
        m_phyRxedCtrlMsgsTrace(m_currentSlot, GetCellId(), m_rnti, GetBwpId(), msg);
        m_ueCphySapUser->RecvSystemInformationBlockType1(GetCellId(), msg2->GetSib1());
    }
    else if (msg->GetMessageType() == NrControlMessage::RAR)
    {
        Ptr<NrRarMessage> rarMsg = DynamicCast<NrRarMessage>(msg);

        Simulator::Schedule((GetSlotPeriod() * (GetL1L2CtrlLatency() / 2)),
                            &NrUePhy::DoReceiveRar,
                            this,
                            rarMsg);
    }
    else
    {
        NS_LOG_INFO("Message type not recognized " << msg->GetMessageType());
        m_phyRxedCtrlMsgsTrace(m_currentSlot, GetCellId(), m_rnti, GetBwpId(), msg);
        m_phySapUser->ReceiveControlMessage(msg);
    }
}

void
NrUePhy::TryToPerformLbt()
{
    NS_LOG_FUNCTION(this);
    uint8_t ulCtrlSymStart = 0;
    uint8_t ulCtrlNumSym = 0;

    for (const auto& alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
        if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL &&
            alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
            ulCtrlSymStart = alloc.m_dci->m_symStart;
            ulCtrlNumSym = alloc.m_dci->m_numSym;
            break;
        }
    }

    if (ulCtrlNumSym != 0)
    {
        // We have an UL CTRL symbol scheduled and we have to transmit CTRLs..
        // .. so we check that we have at least 25 us between the latest DCI,
        // or we have to schedule an LBT event.

        Time limit = m_lastSlotStart + GetSlotPeriod() -
                     ((GetSymbolsPerSlot() - ulCtrlSymStart) * GetSymbolPeriod()) -
                     m_lbtThresholdForCtrl;

        for (const auto& alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
        {
            int64_t symbolPeriod = GetSymbolPeriod().GetMicroSeconds();
            int64_t dciEndsAt = m_lastSlotStart.GetMicroSeconds() +
                                ((alloc.m_dci->m_numSym + alloc.m_dci->m_symStart) * symbolPeriod);

            if (alloc.m_dci->m_type != DciInfoElementTdma::DATA)
            {
                continue;
            }

            if (limit.GetMicroSeconds() < dciEndsAt)
            {
                NS_LOG_INFO("This data DCI ends at "
                            << MicroSeconds(dciEndsAt)
                            << " which is inside the LBT shared COT (the limit is " << limit
                            << "). No need for LBT");
                m_lbtEvent.Cancel(); // Forget any LBT we previously set, because of the new
                                     // DCI information
                m_channelStatus = GRANTED;
            }
            else
            {
                NS_LOG_INFO("This data DCI starts at "
                            << +alloc.m_dci->m_symStart << " for " << +alloc.m_dci->m_numSym
                            << " ends at " << MicroSeconds(dciEndsAt)
                            << " which is outside the LBT shared COT (the limit is " << limit
                            << ").");
            }
        }
        if (m_channelStatus != GRANTED)
        {
            Time sched = m_lastSlotStart - Simulator::Now() + (GetSymbolPeriod() * ulCtrlSymStart) -
                         MicroSeconds(25);
            NS_LOG_DEBUG("Scheduling an LBT for sending the UL CTRL at "
                         << Simulator::Now() + sched);
            m_lbtEvent.Cancel();
            m_lbtEvent = Simulator::Schedule(sched, &NrUePhy::RequestAccess, this);
        }
        else
        {
            NS_LOG_DEBUG("Not scheduling LBT: the UE has a channel status that is GRANTED");
        }
    }
    else
    {
        NS_LOG_DEBUG("Not scheduling LBT; the UE has no UL CTRL symbols available");
    }
}

void
NrUePhy::RequestAccess()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Request access because we have to transmit UL CTRL");
    m_cam->RequestAccess(); // This will put the m_channelStatus to granted when
                            // the channel will be granted.
}

void
NrUePhy::DoReceiveRar(Ptr<NrRarMessage> rarMsg)
{
    NS_LOG_FUNCTION(this);

    NS_LOG_INFO("Received RAR in slot " << m_currentSlot);
    m_phyRxedCtrlMsgsTrace(m_currentSlot, GetCellId(), m_rnti, GetBwpId(), rarMsg);

    for (auto it = rarMsg->RarListBegin(); it != rarMsg->RarListEnd(); ++it)
    {
        if (it->rapId == m_raPreambleId)
        {
            m_phySapUser->ReceiveControlMessage(rarMsg);
        }
    }
}

void
NrUePhy::PushCtrlAllocations(const SfnSf currentSfnSf)
{
    NS_LOG_FUNCTION(this);

    // The UE does not know anything from the GNB yet, so listen on the default
    // bandwidth.
    std::vector<uint8_t> rbgBitmask(GetRbNum(), 1);

    // The UE still doesn't know the TDD pattern, so just add a DL CTRL
    if (m_tddPattern.empty())
    {
        NS_LOG_INFO("TDD Pattern unknown, insert DL CTRL at the beginning of the slot");
        VarTtiAllocInfo dlCtrlSlot(std::make_shared<DciInfoElementTdma>(0,
                                                                        m_dlCtrlSyms,
                                                                        DciInfoElementTdma::DL,
                                                                        DciInfoElementTdma::CTRL,
                                                                        rbgBitmask));
        m_currSlotAllocInfo.m_varTtiAllocInfo.push_front(dlCtrlSlot);
        return;
    }

    uint64_t currentSlotN = currentSfnSf.Normalize() % m_tddPattern.size();

    if (m_tddPattern[currentSlotN] < LteNrTddSlotType::UL)
    {
        NS_LOG_DEBUG("The current TDD pattern indicates that we are in a "
                     << m_tddPattern[currentSlotN]
                     << " slot, so insert DL CTRL at the beginning of the slot");
        VarTtiAllocInfo dlCtrlSlot(std::make_shared<DciInfoElementTdma>(0,
                                                                        m_dlCtrlSyms,
                                                                        DciInfoElementTdma::DL,
                                                                        DciInfoElementTdma::CTRL,
                                                                        rbgBitmask));
        m_currSlotAllocInfo.m_varTtiAllocInfo.push_front(dlCtrlSlot);
    }
    if (m_tddPattern[currentSlotN] > LteNrTddSlotType::DL)
    {
        NS_LOG_DEBUG("The current TDD pattern indicates that we are in a "
                     << m_tddPattern[currentSlotN]
                     << " slot, so insert UL CTRL at the end of the slot");
        VarTtiAllocInfo ulCtrlSlot(
            std::make_shared<DciInfoElementTdma>(GetSymbolsPerSlot() - m_ulCtrlSyms,
                                                 m_ulCtrlSyms,
                                                 DciInfoElementTdma::UL,
                                                 DciInfoElementTdma::CTRL,
                                                 rbgBitmask));
        m_currSlotAllocInfo.m_varTtiAllocInfo.push_back(ulCtrlSlot);
    }
}

void
NrUePhy::StartSlot(const SfnSf& s)
{
    NS_LOG_FUNCTION(this);
    m_currentSlot = s;
    m_lastSlotStart = Simulator::Now();

    // Call MAC before doing anything in PHY
    m_phySapUser->SlotIndication(m_currentSlot); // trigger mac

    // update the current slot object, and insert DL/UL CTRL allocations depending on the TDD
    // pattern
    bool nrAllocExists = SlotAllocInfoExists(m_currentSlot);
    bool slAllocExists = NrSlSlotAllocInfoExists(m_currentSlot);

    /*
     * Clear SL expected TB not received in previous slot.
     * It may happen that a UE is expecting to receive a TB in a slot, however,
     * in the same slot it decided to transmit. In this case, due to the half-duplex
     * nature of the Sidelink it will not receive that TB. Thus, the information
     * inserted in the m_slTransportBlocks buffer will be out-dated in the next slot,
     * hence, must be removed at the beginning of the next slot. This is also due
     * to the fact that in current implementation we always prioritize transmission
     * over reception without looking at the priority of the two TBs, i.e, the one
     * which needs to be transmitted and the one which need to be received.
     * As per the 3GPP standard, a device might prioritize RX over TX as per
     * the priority or vice versa.
     */
    m_spectrumPhy->ClearExpectedSlTb();

    SendSlExpectedTbInfo(s);

    if (slAllocExists)
    {
        NS_ASSERT_MSG(!nrAllocExists, "Can not start SL slot when there is UL allocation");
        StartNrSlSlot(s);
        return;
    }

    if (nrAllocExists)
    {
        m_currSlotAllocInfo = RetrieveSlotAllocInfo(m_currentSlot);
    }
    else
    {
        m_currSlotAllocInfo = SlotAllocInfo(m_currentSlot);
    }

    PushCtrlAllocations(m_currentSlot);

    NS_ASSERT(m_currSlotAllocInfo.m_sfnSf == m_currentSlot);

    NS_LOG_DEBUG("UE " << m_rnti << " start slot " << m_currSlotAllocInfo.m_sfnSf
                       << " composed by the following allocations, total "
                       << m_currSlotAllocInfo.m_varTtiAllocInfo.size());
    for (const auto& alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
        std::string direction;
        std::string type;

        if (alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
            direction = "UL";
        }
        else
        {
            direction = "DL";
        }

        switch (alloc.m_dci->m_type)
        {
        case DciInfoElementTdma::VarTtiType::SRS:
            type = "SRS";
            NS_LOG_DEBUG("Allocation from sym "
                         << static_cast<uint32_t>(alloc.m_dci->m_symStart) << " to sym "
                         << static_cast<uint32_t>(alloc.m_dci->m_numSym + alloc.m_dci->m_symStart)
                         << " direction " << direction << " type " << type);
            break;
        case DciInfoElementTdma::VarTtiType::DATA:
            type = "DATA";
            NS_LOG_INFO("Allocation from sym "
                        << static_cast<uint32_t>(alloc.m_dci->m_symStart) << " to sym "
                        << static_cast<uint32_t>(alloc.m_dci->m_numSym + alloc.m_dci->m_symStart)
                        << " direction " << direction << " type " << type);
            break;
        case DciInfoElementTdma::VarTtiType::CTRL:
            type = "CTRL";
            NS_LOG_DEBUG("Allocation from sym "
                         << static_cast<uint32_t>(alloc.m_dci->m_symStart) << " to sym "
                         << static_cast<uint32_t>(alloc.m_dci->m_numSym + alloc.m_dci->m_symStart)
                         << " direction " << direction << " type " << type);
            break;
        default:
            NS_LOG_ERROR("Unknown type DciInfoElementTdma::VarTtiType " << alloc.m_dci->m_type);
        }
    }

    TryToPerformLbt();

    VarTtiAllocInfo allocation = m_currSlotAllocInfo.m_varTtiAllocInfo.front();
    m_currSlotAllocInfo.m_varTtiAllocInfo.pop_front();

    auto nextVarTtiStart = GetSymbolPeriod() * allocation.m_dci->m_symStart;

    auto ctrlMsgs = PopCurrentSlotCtrlMsgs();
    if (m_netDevice)
    {
        DynamicCast<NrUeNetDevice>(m_netDevice)->RouteOutgoingCtrlMsgs(ctrlMsgs, GetBwpId());
    }
    else
    {
        // No netDevice (that could happen in tests) so just redirect them to us
        for (const auto& msg : ctrlMsgs)
        {
            EncodeCtrlMsg(msg);
        }
    }

    Simulator::Schedule(nextVarTtiStart, &NrUePhy::StartVarTti, this, allocation.m_dci);
}

Time
NrUePhy::DlCtrl(const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);

    Time varTtiDuration = GetSymbolPeriod() * dci->m_numSym;

    NS_LOG_DEBUG("UE" << m_rnti
                      << " RXing DL CTRL frame for"
                         " symbols "
                      << +dci->m_symStart << "-" << +(dci->m_symStart + dci->m_numSym - 1)
                      << "\t start " << Simulator::Now() << " end "
                      << (Simulator::Now() + varTtiDuration));

    m_tryToPerformLbt = true;

    return varTtiDuration;
}

Time
NrUePhy::UlSrs(const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);

    std::vector<int> channelRbs;
    for (uint32_t i = 0; i < GetRbNum(); i++)
    {
        channelRbs.push_back(static_cast<int>(i));
    }
    SetSubChannelsForTransmission(channelRbs, dci->m_numSym);

    std::list<Ptr<NrControlMessage>> srsMsg;
    Ptr<NrSrsMessage> srs = Create<NrSrsMessage>();
    srs->SetSourceBwp(GetBwpId());
    srsMsg.emplace_back(srs);
    Time varTtiDuration = GetSymbolPeriod() * dci->m_numSym;

    m_phyTxedCtrlMsgsTrace(m_currentSlot, GetCellId(), dci->m_rnti, GetBwpId(), *srsMsg.begin());
    m_spectrumPhy->StartTxUlControlFrames(srsMsg, varTtiDuration - NanoSeconds(1.0));

    NS_LOG_DEBUG("UE" << m_rnti << " TXing UL SRS frame for symbols " << +dci->m_symStart << "-"
                      << +(dci->m_symStart + dci->m_numSym - 1) << "\t start " << Simulator::Now()
                      << " end " << (Simulator::Now() + varTtiDuration - NanoSeconds(1.0)));

    ChannelAccessDenied(); // Reset the channel status
    return varTtiDuration;
}

Time
NrUePhy::UlCtrl(const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);

    Time varTtiDuration = GetSymbolPeriod() * dci->m_numSym;

    if (m_ctrlMsgs.empty())
    {
        NS_LOG_DEBUG("UE" << m_rnti << " reserved space for UL CTRL frame for symbols "
                          << +dci->m_symStart << "-" << +(dci->m_symStart + dci->m_numSym - 1)
                          << "\t start " << Simulator::Now() << " end "
                          << (Simulator::Now() + varTtiDuration - NanoSeconds(1.0))
                          << " but no data to transmit");
        m_cam->Cancel();
        return varTtiDuration;
    }
    else if (m_channelStatus != GRANTED)
    {
        NS_LOG_INFO("UE" << m_rnti << " has to transmit CTRL but channel not granted");
        m_cam->Cancel();
        return varTtiDuration;
    }

    for (const auto& msg : m_ctrlMsgs)
    {
        m_phyTxedCtrlMsgsTrace(m_currentSlot, GetCellId(), dci->m_rnti, GetBwpId(), msg);

        if (msg->GetMessageType() == NrControlMessage::DL_HARQ)
        {
            Ptr<NrDlHarqFeedbackMessage> harqMsg = DynamicCast<NrDlHarqFeedbackMessage>(msg);
            uint8_t harqId = harqMsg->GetDlHarqFeedback().m_harqProcessId;

            auto it = m_harqIdToK1Map.find(harqId);
            if (it != m_harqIdToK1Map.end())
            {
                m_phyUeTxedHarqFeedbackTrace(m_currentSlot,
                                             GetCellId(),
                                             m_rnti,
                                             GetBwpId(),
                                             static_cast<uint32_t>(harqId),
                                             it->second);
            }
        }
    }

    std::vector<int> channelRbs;
    for (uint32_t i = 0; i < GetRbNum(); i++)
    {
        channelRbs.push_back(static_cast<int>(i));
    }

    if (m_enableUplinkPowerControl)
    {
        m_txPower = m_powerControl->GetPucchTxPower(channelRbs.size());
    }
    SetSubChannelsForTransmission(channelRbs, dci->m_numSym);

    NS_LOG_DEBUG("UE" << m_rnti << " TXing UL CTRL frame for symbols " << +dci->m_symStart << "-"
                      << +(dci->m_symStart + dci->m_numSym - 1) << "\t start " << Simulator::Now()
                      << " end " << (Simulator::Now() + varTtiDuration - NanoSeconds(1.0)));

    SendCtrlChannels(varTtiDuration - NanoSeconds(1.0));

    ChannelAccessDenied(); // Reset the channel status
    return varTtiDuration;
}

Time
NrUePhy::DlData(const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);

    m_receptionEnabled = true;
    Time varTtiDuration = GetSymbolPeriod() * dci->m_numSym;
    NS_ASSERT(dci->m_rnti == m_rnti);
    m_spectrumPhy->AddExpectedTb({dci->m_ndi,
                                  dci->m_tbSize,
                                  dci->m_mcs,
                                  dci->m_rank,
                                  dci->m_rnti,
                                  FromRBGBitmaskToRBAssignment(dci->m_rbgBitmask),
                                  dci->m_harqProcess,
                                  dci->m_rv,
                                  true,
                                  dci->m_symStart,
                                  dci->m_numSym,
                                  m_currentSlot});
    m_reportDlTbSize(m_netDevice->GetObject<NrUeNetDevice>()->GetImsi(), dci->m_tbSize);
    NS_LOG_INFO("UE" << m_rnti << " RXing DL DATA frame for symbols " << +dci->m_symStart << "-"
                     << +(dci->m_symStart + dci->m_numSym - 1) << " num of rbg assigned: "
                     << FromRBGBitmaskToRBAssignment(dci->m_rbgBitmask).size()
                     << ". RX will take place for " << varTtiDuration);

    return varTtiDuration;
}

Time
NrUePhy::UlData(const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);
    if (m_enableUplinkPowerControl)
    {
        m_txPower = m_powerControl->GetPuschTxPower(
            (FromRBGBitmaskToRBAssignment(dci->m_rbgBitmask)).size());
    }
    SetSubChannelsForTransmission(FromRBGBitmaskToRBAssignment(dci->m_rbgBitmask), dci->m_numSym);
    Time varTtiDuration = GetSymbolPeriod() * dci->m_numSym;
    std::list<Ptr<NrControlMessage>> ctrlMsg;
    Ptr<PacketBurst> pktBurst = GetPacketBurst(m_currentSlot, dci->m_symStart, dci->m_rnti);
    if (pktBurst && pktBurst->GetNPackets() > 0)
    {
        std::list<Ptr<Packet>> pkts = pktBurst->GetPackets();
        LteRadioBearerTag bearerTag;
        if (!pkts.front()->PeekPacketTag(bearerTag))
        {
            NS_FATAL_ERROR("No radio bearer tag");
        }
    }
    else
    {
        // put an error, as something is wrong. The UE should not be scheduled
        // if there is no data for him...
        NS_FATAL_ERROR("The UE " << dci->m_rnti << " has been scheduled without data");
    }
    m_reportUlTbSize(m_netDevice->GetObject<NrUeNetDevice>()->GetImsi(), dci->m_tbSize);

    NS_LOG_DEBUG("UE" << m_rnti << " TXing UL DATA frame for"
                      << " symbols " << +dci->m_symStart << "-"
                      << +(dci->m_symStart + dci->m_numSym - 1) << "\t start " << Simulator::Now()
                      << " end " << (Simulator::Now() + varTtiDuration));

    Simulator::Schedule(NanoSeconds(1.0),
                        &NrUePhy::SendDataChannels,
                        this,
                        pktBurst,
                        ctrlMsg,
                        dci,
                        varTtiDuration - NanoSeconds(2.0));
    return varTtiDuration;
}

void
NrUePhy::StartVarTti(const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);
    Time varTtiDuration;

    m_currTbs = dci->m_tbSize;
    m_receptionEnabled = false;

    if (dci->m_type == DciInfoElementTdma::CTRL && dci->m_format == DciInfoElementTdma::DL)
    {
        varTtiDuration = DlCtrl(dci);
    }
    else if (dci->m_type == DciInfoElementTdma::CTRL && dci->m_format == DciInfoElementTdma::UL)
    {
        varTtiDuration = UlCtrl(dci);
    }
    else if (dci->m_type == DciInfoElementTdma::SRS && dci->m_format == DciInfoElementTdma::UL)
    {
        varTtiDuration = UlSrs(dci);
    }
    else if (dci->m_type == DciInfoElementTdma::DATA && dci->m_format == DciInfoElementTdma::DL)
    {
        varTtiDuration = DlData(dci);
    }
    else if (dci->m_type == DciInfoElementTdma::DATA && dci->m_format == DciInfoElementTdma::UL)
    {
        varTtiDuration = UlData(dci);
    }

    Simulator::Schedule(varTtiDuration, &NrUePhy::EndVarTti, this, dci);
}

void
NrUePhy::EndVarTti(const std::shared_ptr<DciInfoElementTdma>& dci)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("DCI started at symbol "
                 << static_cast<uint32_t>(dci->m_symStart) << " which lasted for "
                 << static_cast<uint32_t>(dci->m_numSym) << " symbols finished");

    if (m_tryToPerformLbt)
    {
        TryToPerformLbt();
        m_tryToPerformLbt = false;
    }

    if (m_currSlotAllocInfo.m_varTtiAllocInfo.empty())
    {
        // end of slot
        m_currentSlot.Add(1);

        Simulator::Schedule(m_lastSlotStart + GetSlotPeriod() - Simulator::Now(),
                            &NrUePhy::StartSlot,
                            this,
                            m_currentSlot);
    }
    else
    {
        VarTtiAllocInfo allocation = m_currSlotAllocInfo.m_varTtiAllocInfo.front();
        m_currSlotAllocInfo.m_varTtiAllocInfo.pop_front();

        Time nextVarTtiStart = GetSymbolPeriod() * allocation.m_dci->m_symStart;

        Simulator::Schedule(nextVarTtiStart + m_lastSlotStart - Simulator::Now(),
                            &NrUePhy::StartVarTti,
                            this,
                            allocation.m_dci);
    }

    m_receptionEnabled = false;
}

void
NrUePhy::PhyDataPacketReceived(const Ptr<Packet>& p)
{
    Simulator::ScheduleWithContext(m_netDevice->GetNode()->GetId(),
                                   GetTbDecodeLatency(),
                                   &NrUePhySapUser::ReceivePhyPdu,
                                   m_phySapUser,
                                   p);
    // m_phySapUser->ReceivePhyPdu (p);
}

void
NrUePhy::SendDataChannels(const Ptr<PacketBurst>& pb,
                          const std::list<Ptr<NrControlMessage>>& ctrlMsg,
                          const std::shared_ptr<DciInfoElementTdma>& dci,
                          const Time& duration)
{
    if (pb->GetNPackets() > 0)
    {
        LteRadioBearerTag tag;
        if (!pb->GetPackets().front()->PeekPacketTag(tag))
        {
            NS_FATAL_ERROR("No radio bearer tag");
        }
    }

    m_spectrumPhy->StartTxDataFrames(pb, ctrlMsg, dci, duration);
}

void
NrUePhy::SendCtrlChannels(Time duration)
{
    m_spectrumPhy->StartTxUlControlFrames(m_ctrlMsgs, duration);
    m_ctrlMsgs.clear();
}

Ptr<NrDlCqiMessage>
NrUePhy::CreateDlCqiFeedbackMessage(const SpectrumValue& sinr)
{
    NS_LOG_FUNCTION(this);
    // Create DL CQI CTRL message
    Ptr<NrDlCqiMessage> msg = Create<NrDlCqiMessage>();
    msg->SetSourceBwp(GetBwpId());
    DlCqiInfo dlcqi;

    dlcqi.m_rnti = m_rnti;
    dlcqi.m_cqiType = DlCqiInfo::WB;

    std::vector<int> cqi;
    dlcqi.m_wbCqi = ComputeCqi(sinr);
    msg->SetDlCqi(dlcqi);
    return msg;
}

void
NrUePhy::GenerateDlCqiReport(const SpectrumValue& sinr)
{
    NS_LOG_FUNCTION(this);
    // Not totally sure what this is about. We have to check.
    if (m_ulConfigured && (m_rnti > 0) && m_receptionEnabled)
    {
        m_dlDataSinrTrace(GetCellId(), m_rnti, ComputeAvgSinr(sinr), GetBwpId());

        if (Simulator::Now() > m_wbCqiLast)
        {
            Ptr<NrDlCqiMessage> msg = CreateDlCqiFeedbackMessage(sinr);

            if (msg)
            {
                DoSendControlMessage(msg);
            }
        }
    }
}

void
NrUePhy::EnqueueDlHarqFeedback(const DlHarqInfo& m)
{
    NS_LOG_FUNCTION(this);
    // get the feedback from NrSpectrumPhy and send it through ideal PUCCH to gNB
    Ptr<NrDlHarqFeedbackMessage> msg = Create<NrDlHarqFeedbackMessage>();
    msg->SetSourceBwp(GetBwpId());
    msg->SetDlHarqFeedback(m);

    auto k1It = m_harqIdToK1Map.find(m.m_harqProcessId);

    NS_LOG_DEBUG("ReceiveLteDlHarqFeedback"
                 << " Harq Process " << static_cast<uint32_t>(k1It->first)
                 << " K1: " << k1It->second << " Frame " << m_currentSlot);

    Time event = m_lastSlotStart + (GetSlotPeriod() * k1It->second);
    if (event <= Simulator::Now())
    {
        Simulator::ScheduleNow(&NrUePhy::DoSendControlMessageNow, this, msg);
    }
    else
    {
        Simulator::Schedule(event - Simulator::Now(), &NrUePhy::DoSendControlMessageNow, this, msg);
    }
}

void
NrUePhy::EnqueueSlHarqFeedback(const SlHarqInfo& m)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Enqueued SL HARQ " << (m.IsReceivedOk() ? "ACK" : "NACK") << " in slot "
                                     << m_currentSlot.Normalize() << " for process "
                                     << +m.m_harqProcessId);
    Ptr<NrSlHarqFeedbackMessage> msg = Create<NrSlHarqFeedbackMessage>();
    msg->SetSlHarqFeedback(m);
    m_slHarqFbList.emplace_back(m_currentSlot, msg);
}

void
NrUePhy::SetCam(const Ptr<NrChAccessManager>& cam)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(cam != nullptr);
    m_cam = cam;
    m_cam->SetAccessGrantedCallback(
        std::bind(&NrUePhy::ChannelAccessGranted, this, std::placeholders::_1));
    m_cam->SetAccessDeniedCallback(std::bind(&NrUePhy::ChannelAccessDenied, this));
}

const SfnSf&
NrUePhy::GetCurrentSfnSf() const
{
    return m_currentSlot;
}

uint16_t
NrUePhy::GetRnti() const
{
    return m_rnti;
}

void
NrUePhy::DoReset()
{
    NS_LOG_FUNCTION(this);
    // initialize NR SL PSCCH packet queue
    m_nrSlPscchPacketBurstQueue.clear();
    Ptr<PacketBurst> pbPscch = CreateObject<PacketBurst>();
    m_nrSlPscchPacketBurstQueue.push_back(pbPscch);

    // initialize NR SL PSSCH packet queue
    m_nrSlPsschPacketBurstQueue.clear();
    // initialize NR SL PSFCH feedback queue
    m_slHarqFbList.clear();
}

void
NrUePhy::DoStartCellSearch(uint16_t dlEarfcn)
{
    NS_LOG_FUNCTION(this << dlEarfcn);
    DoSetInitialBandwidth();
}

void
NrUePhy::DoSynchronizeWithEnb(uint16_t cellId, uint16_t dlEarfcn)
{
    NS_LOG_FUNCTION(this << cellId << dlEarfcn);
    DoSynchronizeWithEnb(cellId);
}

void
NrUePhy::DoSetPa(double pa)
{
    NS_LOG_FUNCTION(this << pa);
}

void
NrUePhy::DoSetRsrpFilterCoefficient(uint8_t rsrpFilterCoefficient)
{
    NS_LOG_FUNCTION(this << +rsrpFilterCoefficient);
}

void
NrUePhy::DoSynchronizeWithEnb(uint16_t cellId)
{
    NS_LOG_FUNCTION(this << cellId);
    DoSetCellId(cellId);
    DoSetInitialBandwidth();
}

BeamId
NrUePhy::GetBeamId([[maybe_unused]] uint16_t rnti) const
{
    NS_LOG_FUNCTION(this);
    // That's a bad specification: the UE PHY doesn't know anything about its beam id.
    NS_FATAL_ERROR("ERROR");
}

void
NrUePhy::ScheduleStartEventLoop(uint32_t nodeId, uint16_t frame, uint8_t subframe, uint16_t slot)
{
    NS_LOG_FUNCTION(this);
    Simulator::ScheduleWithContext(nodeId,
                                   MilliSeconds(0),
                                   &NrUePhy::StartEventLoop,
                                   this,
                                   frame,
                                   subframe,
                                   slot);
}

void
NrUePhy::ReportRsReceivedPower(const SpectrumValue& rsReceivedPower)
{
    NS_LOG_FUNCTION(this << rsReceivedPower);
    m_rsrp = 10 * log10(Integral(rsReceivedPower)) + 30;
    NS_LOG_DEBUG("RSRP value updated: " << m_rsrp << " dBm");

    if (m_enableUplinkPowerControl)
    {
        m_powerControl->SetLoggingInfo(GetCellId(), m_rnti);
        m_powerControl->SetRsrp(m_rsrp);
    }
}

void
NrUePhy::ReceivePss(uint16_t cellId, const Ptr<SpectrumValue>& p)
{
    NS_LOG_FUNCTION(this);

    double sum = 0.0;
    uint16_t nRB = 0;

    uint32_t subcarrierSpacing;
    subcarrierSpacing = 15000 * static_cast<uint32_t>(std::pow(2, GetNumerology()));

    Values::const_iterator itPi;
    for (itPi = p->ConstValuesBegin(); itPi != p->ConstValuesEnd(); itPi++)
    {
        // convert PSD [W/Hz] to linear power [W] for the single RE
        double powerTxW = (*itPi) * subcarrierSpacing;
        sum += powerTxW;
        nRB++;
    }

    // measure instantaneous RSRP now (in dBm)
    double rsrp = 10 * log10(1000 * (sum / static_cast<double>(nRB)));

    NS_LOG_DEBUG("RSRP value updated: " << rsrp << " dBm"
                                        << " for Cell Id: " << cellId << " RNTI: " << m_rnti);

    // store RSRP measurements
    std::map<uint16_t, UeMeasurementsElement>::iterator itMeasMap =
        m_ueMeasurementsMap.find(cellId);
    if (itMeasMap == m_ueMeasurementsMap.end())
    {
        // insert new entry
        UeMeasurementsElement newEl;
        newEl.rsrpSum = rsrp;
        newEl.rsrpNum = 1;
        newEl.rsrqSum = 0;
        newEl.rsrqNum = 0;

        NS_LOG_DEBUG("New RSRP entry for Cell Id: " << cellId << " RNTI: " << m_rnti
                                                    << " RSRP: " << newEl.rsrpSum << " dBm"
                                                    << " number of entries: " << +newEl.rsrpNum);

        m_ueMeasurementsMap.insert(std::pair<uint16_t, UeMeasurementsElement>(cellId, newEl));
    }
    else
    {
        (*itMeasMap).second.rsrpSum += rsrp;
        (*itMeasMap).second.rsrpNum++;

        NS_LOG_DEBUG("Update RSRP entry for Cell Id: "
                     << cellId << " RNTI: " << m_rnti
                     << " RSRP Sum: " << (*itMeasMap).second.rsrpSum << " dBm"
                     << " number of entries: " << +((*itMeasMap).second.rsrpNum));
    }
}

void
NrUePhy::ReportUeMeasurements()
{
    NS_LOG_FUNCTION(this);

    // LteUeCphySapUser::UeMeasurementsParameters ret;

    std::map<uint16_t, UeMeasurementsElement>::iterator it;
    for (it = m_ueMeasurementsMap.begin(); it != m_ueMeasurementsMap.end(); it++)
    {
        double avg_rsrp;
        // double avg_rsrq = 0;
        if ((*it).second.rsrpNum != 0)
        {
            avg_rsrp = (*it).second.rsrpSum / static_cast<double>((*it).second.rsrpNum);
        }
        else
        {
            NS_LOG_WARN(" RSRP nSamples is zero!");
            avg_rsrp = 0;
        }

        NS_LOG_DEBUG(" Report UE Measurements for CellId "
                     << (*it).first << " Reporting UE " << m_rnti << " Av. RSRP " << avg_rsrp
                     << " (nSamples " << +((*it).second.rsrpNum) << ")"
                     << " BwpID " << GetBwpId());

        m_reportRsrpTrace(GetCellId(), m_imsi, m_rnti, avg_rsrp, GetBwpId());

        /*LteUeCphySapUser::UeMeasurementsElement newEl;
        newEl.m_cellId = (*it).first;
        newEl.m_rsrp = avg_rsrp;
        newEl.m_rsrq = avg_rsrq;  //LEAVE IT 0 FOR THE MOMENT
        ret.m_ueMeasurementsList.push_back (newEl);
        ret.m_componentCarrierId = GetBwpId ();*/
    }

    // report to RRC
    // m_ueCphySapUser->ReportUeMeasurements (ret);

    m_ueMeasurementsMap.clear();
    Simulator::Schedule(m_ueMeasurementsFilterPeriod, &NrUePhy::ReportUeMeasurements, this);
}

void
NrUePhy::ReportDlCtrlSinr(const SpectrumValue& sinr)
{
    NS_LOG_FUNCTION(this);
    uint32_t rbUsed = 0;
    double sinrSum = 0.0;

    for (uint32_t i = 0; i < sinr.GetValuesN(); i++)
    {
        double currentSinr = sinr.ValuesAt(i);
        if (currentSinr != 0)
        {
            rbUsed++;
            sinrSum += currentSinr;
        }
    }

    NS_ASSERT(rbUsed);
    m_dlCtrlSinrTrace(GetCellId(), m_rnti, sinrSum / rbUsed, GetBwpId());
}

uint8_t
NrUePhy::ComputeCqi(const SpectrumValue& sinr)
{
    NS_LOG_FUNCTION(this);
    uint8_t mcs; // it is initialized by AMC in the following call
    uint8_t wbCqi = m_amc->CreateCqiFeedbackWbTdma(sinr, mcs);
    return wbCqi;
}

void
NrUePhy::StartEventLoop(uint16_t frame, uint8_t subframe, uint16_t slot)
{
    NS_LOG_FUNCTION(this);

    if (GetChannelBandwidth() == 0)
    {
        NS_LOG_INFO("Initial bandwidth not set, configuring the default one for Cell ID: "
                    << GetCellId() << ", RNTI: " << GetRnti() << ", BWP ID: " << GetBwpId());
        DoSetInitialBandwidth();
    }

    NS_LOG_INFO("PHY starting. Configuration: "
                << std::endl
                << "\t TxPower: " << m_txPower << " dBm" << std::endl
                << "\t NoiseFigure: " << m_noiseFigure << std::endl
                << "\t TbDecodeLatency: " << GetTbDecodeLatency().GetMicroSeconds() << " us "
                << std::endl
                << "\t Numerology: " << GetNumerology() << std::endl
                << "\t SymbolsPerSlot: " << GetSymbolsPerSlot() << std::endl
                << "\t Pattern: " << NrPhy::GetPattern(m_tddPattern) << std::endl
                << "Attached to physical channel: " << std::endl
                << "\t Channel bandwidth: " << GetChannelBandwidth() << " Hz" << std::endl
                << "\t Channel central freq: " << GetCentralFrequency() << " Hz" << std::endl
                << "\t Num. RB: " << GetRbNum());
    SfnSf startSlot(frame, subframe, slot, GetNumerology());
    StartSlot(startSlot);
}

void
NrUePhy::DoSetInitialBandwidth()
{
    NS_LOG_FUNCTION(this);
    // configure initial bandwidth to 6 RBs
    double initialBandwidthHz =
        6 * GetSubcarrierSpacing() * NrSpectrumValueHelper::SUBCARRIERS_PER_RB;
    // divided by 100*1000 because the parameter should be in 100KHz
    uint16_t initialBandwidthIn100KHz = ceil(initialBandwidthHz / (100 * 1000));
    // account for overhead that will be reduced when determining real BW
    uint16_t initialBandwidthWithOverhead = initialBandwidthIn100KHz / (1 - GetRbOverhead());

    NS_ABORT_MSG_IF(initialBandwidthWithOverhead == 0,
                    " Initial bandwidth could not be set. Parameters provided are: "
                    "\n dlBandwidthInRBNum = "
                        << 6 << "\n m_subcarrierSpacing = " << GetSubcarrierSpacing()
                        << "\n NrSpectrumValueHelper::SUBCARRIERS_PER_RB  = "
                        << (unsigned)NrSpectrumValueHelper::SUBCARRIERS_PER_RB
                        << "\n m_rbOh = " << GetRbOverhead());

    DoSetDlBandwidth(initialBandwidthWithOverhead);
}

uint16_t
NrUePhy::DoGetCellId()
{
    return GetCellId();
}

uint32_t
NrUePhy::DoGetDlEarfcn()
{
    // TBD See how to get rid of this function in future
    // Added for the compatibility with 810 MR to LTE.
    NS_LOG_FUNCTION(this);
    NS_LOG_WARN("DoGetDlEarfcn function is called. This function should be removed in future once "
                "NR has its own RRC.");
    return 0;
}

void
NrUePhy::DoSetDlBandwidth(uint16_t dlBandwidth)
{
    NS_LOG_FUNCTION(this << +dlBandwidth);

    SetChannelBandwidth(dlBandwidth);

    NS_LOG_DEBUG("PHY reconfiguring. Result: "
                 << std::endl
                 << "\t TxPower: " << m_txPower << " dBm" << std::endl
                 << "\t NoiseFigure: " << m_noiseFigure << std::endl
                 << "\t TbDecodeLatency: " << GetTbDecodeLatency().GetMicroSeconds() << " us "
                 << std::endl
                 << "\t Numerology: " << GetNumerology() << std::endl
                 << "\t SymbolsPerSlot: " << GetSymbolsPerSlot() << std::endl
                 << "\t Pattern: " << NrPhy::GetPattern(m_tddPattern) << std::endl
                 << "Attached to physical channel: " << std::endl
                 << "\t Channel bandwidth: " << GetChannelBandwidth() << " Hz" << std::endl
                 << "\t Channel central freq: " << GetCentralFrequency() << " Hz" << std::endl
                 << "\t Num. RB: " << GetRbNum());
}

void
NrUePhy::DoConfigureUplink(uint16_t ulEarfcn, uint8_t ulBandwidth)
{
    NS_LOG_FUNCTION(this << ulEarfcn << +ulBandwidth);
    // Ignore this; should be equal to dlBandwidth
    m_ulConfigured = true;
}

void
NrUePhy::DoConfigureReferenceSignalPower(int8_t referenceSignalPower)
{
    NS_LOG_FUNCTION(this << referenceSignalPower);
    m_powerControl->ConfigureReferenceSignalPower(referenceSignalPower);
}

void
NrUePhy::DoSetRnti(uint16_t rnti)
{
    NS_LOG_FUNCTION(this << rnti);
    GetSpectrumPhy()->SetRnti(rnti);
    m_rnti = rnti;
}

void
NrUePhy::DoSetTransmissionMode(uint8_t txMode)
{
    NS_LOG_FUNCTION(this << +txMode);
}

void
NrUePhy::DoSetSrsConfigurationIndex(uint16_t srcCi)
{
    NS_LOG_FUNCTION(this << srcCi);
}

void
NrUePhy::SetPhySapUser(NrUePhySapUser* ptr)
{
    m_phySapUser = ptr;
}

void
NrUePhy::DoResetPhyAfterRlf()
{
    NS_LOG_FUNCTION(this);
    NS_FATAL_ERROR("NrUePhy does not have RLF functionality yet");
}

void
NrUePhy::DoResetRlfParams()
{
    NS_LOG_FUNCTION(this);
    NS_FATAL_ERROR("NrUePhy does not have RLF functionality yet");
}

void
NrUePhy::DoStartInSyncDetection()
{
    NS_LOG_FUNCTION(this);
    NS_FATAL_ERROR("NrUePhy does not have RLF functionality yet");
}

void
NrUePhy::DoSetImsi(uint64_t imsi)
{
    NS_LOG_FUNCTION(this);
    m_imsi = imsi;
}

void
NrUePhy::GenerateDlCqiReportMimo(const std::vector<MimoSignalChunk>& mimoChunks)
{
    NS_LOG_FUNCTION(this);
    // Adopted from NrUePhy::GenerateDlCqiReport: CQI feedback requires properly configured UE
    if (!m_ulConfigured || (m_rnti == 0))
    {
        return;
    }
    // Adopted from NrUePhy::GenerateDlCqiReport: Do not send feedback if this UE was not
    // receiving downlink data (was not scheduled)
    if (!m_receptionEnabled)
    {
        return;
    }

    // Combine multiple signal chunks into a single channel matrix and interference covariance
    auto rxSignal = NrMimoSignal{mimoChunks};

    // Determine if an update to wideband or subband PMI is needed and possible
    auto pmiUpdateParams = CheckUpdatePmi();

    // Create DL CQI message for CQI, PMI, and RI. PMI values are updated only if specified by
    // pmiUpdateParams, otherwise assume same PMI values as during last CQI feedback
    auto cqi = m_pmSearch->CreateCqiFeedbackMimo(rxSignal, pmiUpdateParams);
    auto dlcqi = DlCqiInfo{
        .m_rnti = m_rnti,
        .m_ri = cqi.m_rank,
        .m_cqiType = cqi.m_cqiType,
        .m_wbCqi = cqi.m_wbCqi,
        .m_wbPmi = cqi.m_wbPmi,
        .m_sbCqis = cqi.m_sbCqis,
        .m_sbPmis = cqi.m_sbPmis,
        .m_mcs = cqi.m_mcs,
        .m_optPrecMat = cqi.m_optPrecMat,
    };

    auto msg = Create<NrDlCqiMessage>();
    msg->SetSourceBwp(GetBwpId());
    msg->SetDlCqi(dlcqi);

    DoSendControlMessage(msg);
}

NrPmSearch::PmiUpdate
NrUePhy::CheckUpdatePmi()
{
    // This implementation only checks if sufficient time has passed since the last update.
    // TODO: Improve following logic that defines when to update wideband and/or
    // subband PMIs for two-stage codebooks. The algorithm must allow managing the computational
    // complexity of PMI updates, and take into account availability of PUCCH/PUSCH resources for
    // sending PMI.
    auto pmiUpdate = NrPmSearch::PmiUpdate{};
    auto now = Simulator::Now();
    if (now > m_wbPmiLastUpdate + m_wbPmiUpdateInterval)
    {
        pmiUpdate.updateWb = true;
        m_wbPmiLastUpdate = now;
    }
    if (now > m_sbPmiLastUpdate + m_sbPmiUpdateInterval)
    {
        pmiUpdate.updateSb = true;
        m_sbPmiLastUpdate = now;
    }
    return pmiUpdate;
}

void
NrUePhy::SetPmSearch(Ptr<NrPmSearch> pmSearch)
{
    m_pmSearch = pmSearch;
    NS_ASSERT(m_amc);
    m_pmSearch->SetAmc(m_amc);
}

Ptr<NrPmSearch>
NrUePhy::GetPmSearch() const
{
    return m_pmSearch;
}

void
NrUePhy::PreConfigSlBandwidth(uint16_t slBandwidth)
{
    NS_LOG_FUNCTION(this << slBandwidth);
    if (GetChannelBandwidth() != slBandwidth)
    {
        SetChannelBandwidth(slBandwidth);
    }
}

void
NrUePhy::RegisterSlBwpId(uint16_t bwpId)
{
    NS_LOG_FUNCTION(this);

    // we initialize queues in DoReset;

    SetBwpId(bwpId);
}

NrSlUeCphySapProvider*
NrUePhy::GetNrSlUeCphySapProvider()
{
    NS_LOG_FUNCTION(this);
    return m_nrSlUeCphySapProvider;
}

void
NrUePhy::SetNrSlUeCphySapUser(NrSlUeCphySapUser* s)
{
    NS_LOG_FUNCTION(this);
    m_nrSlUeCphySapUser = s;
}

void
NrUePhy::SetNrSlUePhySapUser(NrSlUePhySapUser* s)
{
    NS_LOG_FUNCTION(this);
    m_nrSlUePhySapUser = s;
}

void
NrUePhy::DoAddNrSlCommTxPool(Ptr<const NrSlCommResourcePool> txPool)
{
    NS_LOG_FUNCTION(this);
    m_slTxPool = txPool;
}

void
NrUePhy::DoAddNrSlCommRxPool(Ptr<const NrSlCommResourcePool> rxPool)
{
    NS_LOG_FUNCTION(this);
    m_slRxPool = rxPool;
}

void
NrUePhy::StartNrSlSlot(const SfnSf& s)
{
    NS_LOG_FUNCTION(this);
    m_nrSlCurrentAlloc = m_nrSlAllocInfoQueue.front();
    m_nrSlAllocInfoQueue.pop_front();
    NS_ASSERT_MSG(m_nrSlCurrentAlloc.sfn == m_currentSlot, "Unable to find NR SL slot allocation");
    NrSlVarTtiAllocInfo varTtiInfo = *(m_nrSlCurrentAlloc.slvarTtiInfoList.begin());
    // erase the retrieved var TTI info
    m_nrSlCurrentAlloc.slvarTtiInfoList.erase(m_nrSlCurrentAlloc.slvarTtiInfoList.begin());
    auto nextVarTtiStart = GetSymbolPeriod() * varTtiInfo.symStart;
    Simulator::Schedule(nextVarTtiStart, &NrUePhy::StartNrSlVarTti, this, varTtiInfo);
}

void
NrUePhy::StartNrSlVarTti(const NrSlVarTtiAllocInfo& varTtiInfo)
{
    NS_LOG_FUNCTION(this);

    Time varTtiDuration;

    if (varTtiInfo.SlVarTtiType == NrSlVarTtiAllocInfo::CTRL)
    {
        varTtiDuration = SlCtrl(varTtiInfo);
        NS_LOG_DEBUG("CTRL " << varTtiDuration.As(Time::MS));
    }
    else if (varTtiInfo.SlVarTtiType == NrSlVarTtiAllocInfo::DATA)
    {
        varTtiDuration = SlData(varTtiInfo);
        NS_LOG_DEBUG("DATA " << varTtiDuration.As(Time::MS));
    }
    else if (varTtiInfo.SlVarTtiType == NrSlVarTtiAllocInfo::FEEDBACK)
    {
        varTtiDuration = SlFeedback(varTtiInfo);
        NS_LOG_DEBUG("FEEDBACK " << varTtiDuration.As(Time::MS));
    }
    else
    {
        NS_FATAL_ERROR("Invalid or unknown SL VarTti type " << varTtiInfo.SlVarTtiType);
    }

    NS_LOG_DEBUG("Scheduling EndNrSlVarTti at time " << (Now() + varTtiDuration).As(Time::S));
    Simulator::Schedule(varTtiDuration, &NrUePhy::EndNrSlVarTti, this, varTtiInfo);
}

void
NrUePhy::EndNrSlVarTti(const NrSlVarTtiAllocInfo& varTtiInfo)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("NR SL var TTI started at symbol " << varTtiInfo.symStart << " which lasted for "
                                                    << varTtiInfo.symLength << " symbols");

    if (m_nrSlCurrentAlloc.slvarTtiInfoList.empty())
    {
        // end of slot
        m_currentSlot.Add(1);
        // we need trigger the NR Slot start
        Simulator::Schedule(m_lastSlotStart + GetSlotPeriod() - Simulator::Now(),
                            &NrUePhy::StartSlot,
                            this,
                            m_currentSlot);
    }
    else
    {
        NrSlVarTtiAllocInfo nextVarTtiInfo = *(m_nrSlCurrentAlloc.slvarTtiInfoList.begin());
        // erase the retrieved var TTI info
        m_nrSlCurrentAlloc.slvarTtiInfoList.erase(m_nrSlCurrentAlloc.slvarTtiInfoList.begin());
        auto nextVarTtiStart = GetSymbolPeriod() * nextVarTtiInfo.symStart;

        Simulator::Schedule(nextVarTtiStart + m_lastSlotStart - Simulator::Now(),
                            &NrUePhy::StartNrSlVarTti,
                            this,
                            nextVarTtiInfo);
    }
}

Time
NrUePhy::SlCtrl(const NrSlVarTtiAllocInfo& varTtiInfo)
{
    NS_LOG_FUNCTION(this);

    Ptr<PacketBurst> pktBurst = PopPscchPacketBurst();
    if (!pktBurst || pktBurst->GetNPackets() == 0)
    {
        NS_FATAL_ERROR("No NR SL CTRL packet to transmit");
    }
    Time varTtiPeriod = GetSymbolPeriod() * varTtiInfo.symLength;
    // -1 ns ensures control ends before data period
    SendNrSlCtrlChannels(pktBurst, varTtiPeriod - NanoSeconds(1.0), varTtiInfo);

    return varTtiPeriod;
}

void
NrUePhy::SendNrSlCtrlChannels(const Ptr<PacketBurst>& pb,
                              const Time& varTtiDuration,
                              const NrSlVarTtiAllocInfo& varTtiInfo)
{
    NS_LOG_FUNCTION(this);

    std::vector<int> channelRbs;
    uint32_t lastRbInPlusOne = (varTtiInfo.rbStart + varTtiInfo.rbLength);
    for (uint32_t i = varTtiInfo.rbStart; i < lastRbInPlusOne; i++)
    {
        channelRbs.push_back(static_cast<int>(i));
    }

    SetSubChannelsForTransmission(channelRbs, varTtiInfo.symLength);
    NS_LOG_DEBUG("Sending PSCCH on SfnSf " << m_currentSlot);
    m_spectrumPhy->StartTxSlCtrlFrames(pb, varTtiDuration);
}

Time
NrUePhy::SlData(const NrSlVarTtiAllocInfo& varTtiInfo)
{
    NS_LOG_FUNCTION(this);

    Time varTtiDuration = GetSymbolPeriod() * varTtiInfo.symLength;
    Ptr<PacketBurst> pktBurst = PopPsschPacketBurst();
    do
    {
        if (pktBurst && pktBurst->GetNPackets() > 0)
        {
            std::list<Ptr<Packet>> pkts = pktBurst->GetPackets();
            LteRadioBearerTag bearerTag;
            if (!pkts.front()->PeekPacketTag(bearerTag))
            {
                NS_FATAL_ERROR("No radio bearer tag");
            }
        }
        else
        {
            // put an error, as something is wrong. The UE should not be scheduled
            // if there is no data for it...
            NS_FATAL_ERROR("The UE " << m_rnti << " has been scheduled without NR SL data");
        }

        NS_LOG_DEBUG("UE" << m_rnti << " TXing NR SL DATA frame for symbols " << varTtiInfo.symStart
                          << "-" << varTtiInfo.symStart + varTtiInfo.symLength - 1 << "\t start "
                          << Simulator::Now() << " end "
                          << (Simulator::Now() + varTtiDuration).GetSeconds());
        Simulator::Schedule(NanoSeconds(1.0),
                            &NrUePhy::SendNrSlDataChannels,
                            this,
                            pktBurst,
                            varTtiDuration - NanoSeconds(2.0),
                            varTtiInfo);
        pktBurst = PopPsschPacketBurst();
    } while (pktBurst);
    return varTtiDuration;
}

void
NrUePhy::SendNrSlDataChannels(const Ptr<PacketBurst>& pb,
                              const Time& varTtiDuration,
                              const NrSlVarTtiAllocInfo& varTtiInfo)
{
    NS_LOG_FUNCTION(this);

    std::vector<int> channelRbs;
    uint32_t lastRbInPlusOne = (varTtiInfo.rbStart + varTtiInfo.rbLength);
    for (uint32_t i = varTtiInfo.rbStart; i < lastRbInPlusOne; i++)
    {
        channelRbs.push_back(static_cast<int>(i));
    }

    SetSubChannelsForTransmission(channelRbs, varTtiInfo.symLength);
    NS_LOG_DEBUG("Sending PSSCH on SfnSf " << m_currentSlot);
    // Assume Sl Data channel is sent through the first stream
    m_spectrumPhy->StartTxSlDataFrames(pb, varTtiDuration);
}

Time
NrUePhy::SlFeedback(const NrSlVarTtiAllocInfo& varTtiInfo)
{
    NS_LOG_FUNCTION(this);

    Time varTtiDuration = GetSymbolPeriod() * varTtiInfo.symLength;

    // Walk the queue and insert all eligible feedback messages to the pktBurst.
    // A message is eligible if the current slot is MinTimeGapPsfch slots or
    // greater than the slot time associated with the feedback in the queue.

    // Note:  Future revisions of this method will need to further filter
    // feedback messages beyond simply whether MinTimeGapPsfch has been
    // exceeded.  For instance, there may be two messages that require the
    // same PSFCH resource (and the higher priority must be selected), or
    // the UE may be expecting feedback on the PSFCH from a prior transmission
    // at higher priority than the feedback queued for sending (in which
    // case the sending of feedback in this slot should be suppressed).

    std::list<Ptr<NrSlHarqFeedbackMessage>> feedbackList;
    uint8_t gap =
        m_slTxPool->GetMinTimeGapPsfch(GetBwpId(), m_nrSlUePhySapUser->GetSlActiveTxPoolId());
    auto it = m_slHarqFbList.begin();
    while (it != m_slHarqFbList.end())
    {
        if (m_currentSlot.Normalize() >= gap + it->first.Normalize())
        {
            NS_LOG_DEBUG("Inserting HARQ FB to packet burst from slot "
                         << it->first.Normalize() << " for sender RNTI "
                         << it->second->GetSlHarqFeedback().m_txRnti << " dstL2Id "
                         << it->second->GetSlHarqFeedback().m_dstL2Id << " harqProcessId "
                         << +it->second->GetSlHarqFeedback().m_harqProcessId << " bwpIndex "
                         << +it->second->GetSlHarqFeedback().m_bwpIndex << " status "
                         << (it->second->GetSlHarqFeedback().IsReceivedOk() ? "ACK" : "NACK"));
            feedbackList.emplace_front(it->second);
            auto prev = it++;
            m_slHarqFbList.erase(prev);
        }
        else if (m_currentSlot.Normalize() >= it->first.Normalize())
        {
            NS_LOG_DEBUG("At slot "
                         << m_currentSlot.Normalize() << "; suppressing (processing delay " << +gap
                         << " slots from " << it->first.Normalize()
                         << ") the insertion of HARQ FB for sender RNTI "
                         << it->second->GetSlHarqFeedback().m_txRnti << " dstL2Id "
                         << it->second->GetSlHarqFeedback().m_dstL2Id << " harqProcessId "
                         << +it->second->GetSlHarqFeedback().m_harqProcessId << " bwpIndex "
                         << +it->second->GetSlHarqFeedback().m_bwpIndex << " status "
                         << (it->second->GetSlHarqFeedback().IsReceivedOk() ? "ACK" : "NACK"));
            ++it;
        }
        else
        {
            ++it;
        }
    }
    // It could be the case that among the eligible HARQ feedback messages to
    // return, we have an earlier NACK that was later overridden by an ACK
    // (possibly due to a blind retransmission).  Deliver only the latest
    // one by iterating the feedback list and using a std::set to
    // check for duplicates.  Because the previous iteration was in reverse,
    // the unique feedback that we want to return will be the first encountered.
    std::list<Ptr<NrSlHarqFeedbackMessage>> uniqueFeedbackList;
    auto it2 = feedbackList.begin();
    std::set<std::pair<uint16_t, uint8_t>> duplicateCheck;
    while (it2 != feedbackList.end())
    {
        uint16_t rnti = (*it2)->GetSlHarqFeedback().m_txRnti;
        uint8_t harqProcessId = (*it2)->GetSlHarqFeedback().m_harqProcessId;
        // If insert() returns false, the (rnti, harqProcessId) already exists
        if (duplicateCheck.insert(std::make_pair(rnti, harqProcessId)).second)
        {
            NS_LOG_DEBUG("Preparing HARQ feedback for sender RNTI " << rnti << " HARQ PID "
                                                                    << +harqProcessId);
            uniqueFeedbackList.emplace_front(*it2);
        }
        ++it2;
    }
    if (!uniqueFeedbackList.empty())
    {
        NS_LOG_DEBUG("UE" << m_rnti << " TXing NR SL FEEDBACK frame for symbols "
                          << varTtiInfo.symStart << "-"
                          << varTtiInfo.symStart + varTtiInfo.symLength - 1 << "\t start "
                          << Simulator::Now().GetSeconds() << " end "
                          << (Simulator::Now() + varTtiDuration).GetSeconds());

        Simulator::Schedule(NanoSeconds(1.0),
                            &NrUePhy::SendNrSlFbChannels,
                            this,
                            uniqueFeedbackList,
                            varTtiDuration - NanoSeconds(2.0),
                            varTtiInfo);
    }
    return varTtiDuration;
}

void
NrUePhy::SendNrSlFbChannels(const std::list<Ptr<NrSlHarqFeedbackMessage>>& feedbackList,
                            const Time& varTtiDuration,
                            const NrSlVarTtiAllocInfo& varTtiInfo)
{
    NS_LOG_FUNCTION(this << varTtiDuration);

    std::vector<int> channelRbs;
    uint32_t lastRbInPlusOne = (varTtiInfo.rbStart + varTtiInfo.rbLength);
    for (uint32_t i = varTtiInfo.rbStart; i < lastRbInPlusOne; i++)
    {
        channelRbs.push_back(static_cast<int>(i));
    }

    SetSubChannelsForTransmission(channelRbs, varTtiInfo.symLength);
    NS_LOG_DEBUG("Sending PSFCH on SfnSf " << m_currentSlot);
    m_spectrumPhy->StartTxSlFeedback(feedbackList, varTtiDuration);
}

void
NrUePhy::PhyPscchPduReceived(const Ptr<Packet>& p, const SpectrumValue& psd)
{
    NS_LOG_FUNCTION(this);
    NrSlSciF1aHeader sciF1a;
    NrSlMacPduTag tag;

    p->PeekHeader(sciF1a);
    p->PeekPacketTag(tag);

    std::unordered_set<uint32_t> destinations = m_nrSlUePhySapUser->GetSlRxDestinations();

    NS_ASSERT_MSG(m_slRxPool != nullptr, "No receiving pools configured");
    uint16_t sbChSize =
        m_slRxPool->GetNrSlSubChSize(GetBwpId(), m_nrSlUePhySapUser->GetSlActiveTxPoolId());
    uint16_t rbStart = sciF1a.GetIndexStartSubChannel() * sbChSize;
    uint16_t lastRbInPlusOne = (sciF1a.GetLengthSubChannel() * sbChSize) + rbStart;
    std::vector<int> rbBitMap;

    for (uint16_t i = rbStart; i < lastRbInPlusOne; ++i)
    {
        rbBitMap.push_back(i);
    }

    double rsrpDbm = GetSidelinkRsrp(psd).second;

    NS_LOG_DEBUG("Sending sensing data to UE MAC. RSRP "
                 << rsrpDbm << " dBm "
                 << " Frame " << m_currentSlot.GetFrame() << " SubFrame "
                 << +m_currentSlot.GetSubframe() << " Slot " << m_currentSlot.GetSlot());

    SensingData sensingData(m_currentSlot,
                            sciF1a.GetSlResourceReservePeriod(),
                            sciF1a.GetLengthSubChannel(),
                            sciF1a.GetIndexStartSubChannel(),
                            sciF1a.GetPriority(),
                            rsrpDbm,
                            sciF1a.GetGapReTx1(),
                            sciF1a.GetIndexStartSbChReTx1(),
                            sciF1a.GetGapReTx2(),
                            sciF1a.GetIndexStartSbChReTx2());

    m_nrSlUePhySapUser->ReceiveSensingData(sensingData);

    auto it = destinations.find(tag.GetDstL2Id());
    if (it != destinations.end())
    {
        NS_LOG_INFO("Received first stage SCI for destination " << *it << " from RNTI "
                                                                << tag.GetRnti());
        // Assume first stream
        m_spectrumPhy->AddSlExpectedTb({UINT8_MAX,
                                        tag.GetTbSize(),
                                        sciF1a.GetMcs(),
                                        UINT8_MAX,
                                        tag.GetRnti(),
                                        rbBitMap,
                                        UINT8_MAX,
                                        UINT8_MAX,
                                        false,
                                        tag.GetSymStart(),
                                        tag.GetNumSym(),
                                        tag.GetSfn()},
                                       tag.GetDstL2Id());
        SaveFutureSlRxGrants(sciF1a, tag, sbChSize);
    }
    else
    {
        NS_LOG_INFO("Ignoring PSCCH! Destination " << tag.GetDstL2Id()
                                                   << " is not monitored by RNTI " << m_rnti);
    }
}

void
NrUePhy::PhyPsfchReceived(uint32_t sendingNodeId, SlHarqInfo harqInfo)
{
    NS_LOG_FUNCTION(this << sendingNodeId);
    Simulator::ScheduleWithContext(m_netDevice->GetNode()->GetId(),
                                   // Add PSFCH decode latency here
                                   // XXX provisional value of 1 slot
                                   GetSlotPeriod(),
                                   &NrSlUePhySapUser::ReceivePsfch,
                                   m_nrSlUePhySapUser,
                                   sendingNodeId,
                                   harqInfo);
}

void
NrUePhy::SaveFutureSlRxGrants(const NrSlSciF1aHeader& sciF1a,
                              const NrSlMacPduTag& tag,
                              const uint16_t sbChSize)
{
    NS_LOG_FUNCTION(this);

    if (sciF1a.GetGapReTx1() != std::numeric_limits<uint8_t>::max())
    {
        uint16_t rbStart = sciF1a.GetIndexStartSbChReTx1() * sbChSize;
        uint16_t lastRbInPlusOne = (sciF1a.GetLengthSubChannel() * sbChSize) + rbStart;
        std::vector<int> rbBitMap;
        for (uint16_t i = rbStart; i < lastRbInPlusOne; ++i)
        {
            rbBitMap.push_back(i);
        }
        SlRxGrantInfo infoTb(tag.GetRnti(),
                             tag.GetDstL2Id(),
                             tag.GetTbSize(),
                             sciF1a.GetMcs(),
                             rbBitMap,
                             tag.GetSymStart(),
                             tag.GetNumSym(),
                             tag.GetSfn().GetFutureSfnSf(sciF1a.GetGapReTx1()));
        m_slRxGrants.push_back(infoTb);
    }
    if (sciF1a.GetGapReTx2() != std::numeric_limits<uint8_t>::max())
    {
        uint16_t rbStart = sciF1a.GetIndexStartSbChReTx2() * sbChSize;
        uint16_t lastRbInPlusOne = (sciF1a.GetLengthSubChannel() * sbChSize) + rbStart;
        std::vector<int> rbBitMap;
        for (uint16_t i = rbStart; i < lastRbInPlusOne; ++i)
        {
            rbBitMap.push_back(i);
        }
        SlRxGrantInfo infoTb(tag.GetRnti(),
                             tag.GetDstL2Id(),
                             tag.GetTbSize(),
                             sciF1a.GetMcs(),
                             rbBitMap,
                             tag.GetSymStart(),
                             tag.GetNumSym(),
                             tag.GetSfn().GetFutureSfnSf(sciF1a.GetGapReTx2()));
        m_slRxGrants.push_back(infoTb);
    }

    NS_LOG_DEBUG("Expecting " << m_slRxGrants.size() << " future PSSCH slots without SCI 1-A");
    for (const auto& it : m_slRxGrants)
    {
        NS_LOG_DEBUG("Expecting on SfnSf " << it.sfn);
    }
}

void
NrUePhy::SendSlExpectedTbInfo(const SfnSf& s)
{
    NS_LOG_FUNCTION(this);
    if (!m_slRxGrants.empty())
    {
        auto expectedTbInfo = m_slRxGrants.front();
        if (expectedTbInfo.sfn == s)
        {
            m_slRxGrants.pop_front();
            m_spectrumPhy->AddSlExpectedTb({UINT8_MAX,
                                            expectedTbInfo.tbSize,
                                            expectedTbInfo.mcs,
                                            UINT8_MAX,
                                            expectedTbInfo.rnti,
                                            expectedTbInfo.rbBitmap,
                                            UINT8_MAX,
                                            UINT8_MAX,
                                            false,
                                            expectedTbInfo.symStart,
                                            expectedTbInfo.numSym,
                                            expectedTbInfo.sfn},
                                           expectedTbInfo.dstId);
        }
    }
}

void
NrUePhy::PhyPsschPduReceived(const Ptr<PacketBurst>& pb, const SpectrumValue& psd)
{
    NS_LOG_FUNCTION(this);
    LteRadioBearerTag tag;
    NrSlSciF2aHeader sciF2a;
    // Separate SCI stage 2 packet from data packets
    std::list<Ptr<Packet>> dataPkts;
    bool foundSci2 = false;
    Ptr<PacketBurst> pdu = pb;
    for (auto p : pdu->GetPackets())
    {
        LteRadioBearerTag tag;
        if (!p->PeekPacketTag(tag))
        {
            // SCI stage 2 is the only packet in the packet burst, which does
            // not have the tag
            p->PeekHeader(sciF2a);
            foundSci2 = true;
        }
        else
        {
            dataPkts.push_back(p);
        }
    }

    NS_ABORT_MSG_IF(foundSci2 == false, "Did not find SCI stage 2 in PSSCH packet burst");
    NS_ASSERT_MSG(!dataPkts.empty(), "Received PHY PDU with not data packets");

    for (auto& pktIt : dataPkts)
    {
        uint32_t srcL2Id = sciF2a.GetSrcId();
        Ptr<Packet> packet = pktIt->Copy();
        packet->RemovePacketTag(tag);

        double rsrpWatt = GetSidelinkRsrp(psd).first;

        // We only monitor RSRP for relay discovery messages (LCID = 4)
        if ((tag.GetLcid() == 4))
        {
            // Store RSRP for L1 filtering
            std::map<uint32_t, UeSlRsrpMeasurementsElement>::iterator itRsrp =
                m_ueSlRsrpMeasurementsMap.find(srcL2Id);
            if (itRsrp == m_ueSlRsrpMeasurementsMap.end())
            {
                NS_LOG_LOGIC(this << "First RSRP measurement entry");
                UeSlRsrpMeasurementsElement elt;
                elt.rsrpSum = rsrpWatt;
                elt.rsrpNum = 1;
                m_ueSlRsrpMeasurementsMap.insert(
                    std::pair<uint32_t, UeSlRsrpMeasurementsElement>(srcL2Id, elt));
            }
            else
            {
                NS_LOG_LOGIC(this << "RSRP Measurement entry found... Adding values");
                itRsrp->second.rsrpSum += rsrpWatt;
                itRsrp->second.rsrpNum++;
            }
        }
    }

    NS_LOG_INFO("Scheduling ReceivePsschPhyPdu after decode latency of "
                << GetTbDecodeLatency().As(Time::US));
    Simulator::ScheduleWithContext(m_netDevice->GetNode()->GetId(),
                                   GetTbDecodeLatency(),
                                   &NrSlUePhySapUser::ReceivePsschPhyPdu,
                                   m_nrSlUePhySapUser,
                                   pb);
}

std::pair<double, double>
NrUePhy::GetSidelinkRsrp(SpectrumValue psd)
{
    // Measure instantaneous S-RSRP...
    double sum = 0.0;
    uint16_t numRB = 0;

    for (Values::const_iterator itPi = psd.ConstValuesBegin(); itPi != psd.ConstValuesEnd(); itPi++)
    {
        if ((*itPi))
        {
            uint32_t scSpacing = 15000 * static_cast<uint32_t>(std::pow(2, GetNumerology()));
            uint32_t RbWidthInHz =
                static_cast<uint32_t>(scSpacing * NrSpectrumValueHelper::SUBCARRIERS_PER_RB);
            double powerTxWattPerRb =
                ((*itPi) * RbWidthInHz); // convert PSD [W/Hz] to linear power [W]
            double powerTxWattPerRe =
                (powerTxWattPerRb /
                 NrSpectrumValueHelper::SUBCARRIERS_PER_RB); // power of one RE per RB
            double PowerTxWattDmrsPerRb =
                powerTxWattPerRe *
                3.0; // TS 38.211 sec 8.4.1.3, 3 RE per RB carries PSCCH DMRS, i.e. Comb 4
            sum += PowerTxWattDmrsPerRb;
            numRB++;
        }
    }

    double avrgRsrpWatt = (sum / ((double)numRB * 3.0));
    double rsrpDbm = 10 * log10(1000 * (avrgRsrpWatt));

    return std::make_pair(avrgRsrpWatt, rsrpDbm);
}

void
NrUePhy::DoEnableUeSlRsrpMeasurements()
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_MSG_IF(m_rsrpFilterPeriod.IsZero(),
                    "RSRP filter period must be non-zero; otherwise will endlessly loop");
    Simulator::Schedule(m_rsrpFilterPeriod, &NrUePhy::ReportUeSlRsrpMeasurements, this);
    m_ueSlRsrpMeasurementsEnabled = true;
    // Let the RRC know the L1 measurement period
    m_nrSlUeCphySapUser->SetRsrpFilterPeriod(m_rsrpFilterPeriod);
}

void
NrUePhy::DoDisableUeSlRsrpMeasurements()
{
    NS_LOG_FUNCTION(this);
    m_ueSlRsrpMeasurementsEnabled = false;
}

void
NrUePhy::ReportUeSlRsrpMeasurements()
{
    NS_LOG_FUNCTION(this << m_rnti);
    if (m_ueSlRsrpMeasurementsEnabled)
    {
        NrSlUeCphySapUser::RsrpElementsList rsrpList;
        // Perform the L1 filtering
        for (auto it = m_ueSlRsrpMeasurementsMap.begin(); it != m_ueSlRsrpMeasurementsMap.end();
             it++)
        {
            // L1 filtering: linear average
            double avgRsrpW = it->second.rsrpSum / static_cast<double>(it->second.rsrpNum);
            // The stored values are in W, the report to the MAC/RRC should be in dBm
            double avgRsrpDbm = 10 * log10(1000 * (avgRsrpW));

            NS_LOG_INFO(this << " UE L2 Id " << it->first << " averaged RSRP (dBm) " << avgRsrpDbm
                             << " number of measurements " << it->second.rsrpNum);
            NrSlUeCphySapUser::RsrpElement elt;
            elt.l2Id = it->first;
            elt.rsrp = avgRsrpDbm;
            rsrpList.rsrpMeasurementsList.push_back(elt);

            // Save RSRP Measurements
            m_reportUeSlRsrpMeasurements(m_rnti, it->first, avgRsrpDbm);
        }

        // Notify RRC
        m_nrSlUeCphySapUser->ReceiveUeSlRsrpMeasurements(rsrpList);

        // Schedule next L1 filtering
        Simulator::Schedule(m_rsrpFilterPeriod, &NrUePhy::ReportUeSlRsrpMeasurements, this);

        // Clear map after finishing the L1 filtering
        m_ueSlRsrpMeasurementsMap.clear();
    }
}

} // namespace ns3
