/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *                Sourjya Dutta <sdutta@nyu.edu>
 *                Russell Ford <russell.ford@nyu.edu>
 *                Menglei Zhang <menglei@nyu.edu>
 *                Biljana Bojovic <bbojovic@cttc.es>
 */

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      if (m_phyMacConfig)                                                \
        {                                                                \
          std::clog << " [ccId "                                         \
                    << static_cast<uint32_t> (m_phyMacConfig->GetCcId ())\
                    << "] ";                                             \
        }                                                                \
    }                                                                    \
  while (false);
#include "mmwave-enb-mac.h"
#include "mmwave-phy-mac-common.h"
#include "mmwave-mac-pdu-header.h"
#include "mmwave-mac-sched-sap.h"
#include "mmwave-mac-scheduler.h"
#include <ns3/lte-mac-sap.h>
#include <ns3/lte-enb-cmac-sap.h>
#include <ns3/log.h>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("MmWaveEnbMac");

NS_OBJECT_ENSURE_REGISTERED (MmWaveEnbMac);



// //////////////////////////////////////
// member SAP forwarders
// //////////////////////////////////////


class MmWaveEnbMacMemberEnbCmacSapProvider : public LteEnbCmacSapProvider
{
public:
  MmWaveEnbMacMemberEnbCmacSapProvider (MmWaveEnbMac* mac);

  // inherited from LteEnbCmacSapProvider
  virtual void ConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth);
  virtual void AddUe (uint16_t rnti);
  virtual void RemoveUe (uint16_t rnti);
  virtual void AddLc (LcInfo lcinfo, LteMacSapUser* msu);
  virtual void ReconfigureLc (LcInfo lcinfo);
  virtual void ReleaseLc (uint16_t rnti, uint8_t lcid);
  virtual void UeUpdateConfigurationReq (UeConfig params);
  virtual RachConfig GetRachConfig ();
  virtual AllocateNcRaPreambleReturnValue AllocateNcRaPreamble (uint16_t rnti);


private:
  MmWaveEnbMac* m_mac;
};


MmWaveEnbMacMemberEnbCmacSapProvider::MmWaveEnbMacMemberEnbCmacSapProvider (MmWaveEnbMac* mac)
  : m_mac (mac)
{
}

void
MmWaveEnbMacMemberEnbCmacSapProvider::ConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  m_mac->DoConfigureMac (ulBandwidth, dlBandwidth);
}

void
MmWaveEnbMacMemberEnbCmacSapProvider::AddUe (uint16_t rnti)
{
  m_mac->DoAddUe (rnti);
}

void
MmWaveEnbMacMemberEnbCmacSapProvider::RemoveUe (uint16_t rnti)
{
  m_mac->DoRemoveUe (rnti);
}

void
MmWaveEnbMacMemberEnbCmacSapProvider::AddLc (LcInfo lcinfo, LteMacSapUser* msu)
{
  m_mac->DoAddLc (lcinfo, msu);
}

void
MmWaveEnbMacMemberEnbCmacSapProvider::ReconfigureLc (LcInfo lcinfo)
{
  m_mac->DoReconfigureLc (lcinfo);
}

void
MmWaveEnbMacMemberEnbCmacSapProvider::ReleaseLc (uint16_t rnti, uint8_t lcid)
{
  m_mac->DoReleaseLc (rnti, lcid);
}

void
MmWaveEnbMacMemberEnbCmacSapProvider::UeUpdateConfigurationReq (UeConfig params)
{
  m_mac->UeUpdateConfigurationReq (params);
}

LteEnbCmacSapProvider::RachConfig
MmWaveEnbMacMemberEnbCmacSapProvider::GetRachConfig ()
{
  return m_mac->DoGetRachConfig ();
}

LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue
MmWaveEnbMacMemberEnbCmacSapProvider::AllocateNcRaPreamble (uint16_t rnti)
{
  return m_mac->DoAllocateNcRaPreamble (rnti);
}



// SAP
// ENB MAC-Phy
class MmWaveMacEnbMemberPhySapUser : public MmWaveEnbPhySapUser
{
public:
  MmWaveMacEnbMemberPhySapUser (MmWaveEnbMac* mac);

  virtual void ReceivePhyPdu (Ptr<Packet> p) override;

  virtual void ReceiveControlMessage (Ptr<MmWaveControlMessage> msg) override;

  virtual void SlotIndication (SfnSf) override;

  virtual void UlCqiReport (MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters cqi) override;

  virtual void ReceiveRachPreamble (uint32_t raId) override;

  virtual void UlHarqFeedback (UlHarqInfo params) override;

  virtual void BeamChangeReport (AntennaArrayModel::BeamId beamId, uint8_t rnti) override;

private:
  MmWaveEnbMac* m_mac;
};

MmWaveMacEnbMemberPhySapUser::MmWaveMacEnbMemberPhySapUser (MmWaveEnbMac* mac)
  : m_mac (mac)
{

}

void
MmWaveMacEnbMemberPhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
MmWaveMacEnbMemberPhySapUser::ReceiveControlMessage (Ptr<MmWaveControlMessage> msg)
{
  m_mac->DoReceiveControlMessage (msg);
}

void
MmWaveMacEnbMemberPhySapUser::SlotIndication (SfnSf sfn)
{
  m_mac->DoSlotIndication (sfn);
}

void
MmWaveMacEnbMemberPhySapUser::UlCqiReport (MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi)
{
  m_mac->DoUlCqiReport (ulcqi);
}

void
MmWaveMacEnbMemberPhySapUser::ReceiveRachPreamble (uint32_t raId)
{
  m_mac->ReceiveRachPreamble (raId);
}

void
MmWaveMacEnbMemberPhySapUser::UlHarqFeedback (UlHarqInfo params)
{
  m_mac->DoUlHarqFeedback (params);
}

void
MmWaveMacEnbMemberPhySapUser::BeamChangeReport (AntennaArrayModel::BeamId beamId, uint8_t rnti)
{
  m_mac->BeamChangeReport (beamId, rnti);
}

// MAC Sched

class MmWaveMacMemberMacSchedSapUser : public MmWaveMacSchedSapUser
{
public:
  MmWaveMacMemberMacSchedSapUser (MmWaveEnbMac* mac);
  virtual void SchedConfigInd (const struct SchedConfigIndParameters& params);
private:
  MmWaveEnbMac* m_mac;
};

MmWaveMacMemberMacSchedSapUser::MmWaveMacMemberMacSchedSapUser (MmWaveEnbMac* mac)
  : m_mac (mac)
{
  //  Some blank spaces
}

void
MmWaveMacMemberMacSchedSapUser::SchedConfigInd (const struct SchedConfigIndParameters& params)
{
  m_mac->DoSchedConfigIndication (params);
}


class MmWaveMacMemberMacCschedSapUser : public MmWaveMacCschedSapUser
{
public:
  MmWaveMacMemberMacCschedSapUser (MmWaveEnbMac* mac);

  virtual void CschedCellConfigCnf (const struct MmWaveMacCschedSapUser::CschedCellConfigCnfParameters& params);
  virtual void CschedUeConfigCnf (const struct MmWaveMacCschedSapUser::CschedUeConfigCnfParameters& params);
  virtual void CschedLcConfigCnf (const struct MmWaveMacCschedSapUser::CschedLcConfigCnfParameters& params);
  virtual void CschedLcReleaseCnf (const struct MmWaveMacCschedSapUser::CschedLcReleaseCnfParameters& params);
  virtual void CschedUeReleaseCnf (const struct MmWaveMacCschedSapUser::CschedUeReleaseCnfParameters& params);
  virtual void CschedUeConfigUpdateInd (const struct MmWaveMacCschedSapUser::CschedUeConfigUpdateIndParameters& params);
  virtual void CschedCellConfigUpdateInd (const struct MmWaveMacCschedSapUser::CschedCellConfigUpdateIndParameters& params);

private:
  MmWaveEnbMac* m_mac;
};


MmWaveMacMemberMacCschedSapUser::MmWaveMacMemberMacCschedSapUser (MmWaveEnbMac* mac)
  : m_mac (mac)
{
}

void
MmWaveMacMemberMacCschedSapUser::CschedCellConfigCnf (const struct CschedCellConfigCnfParameters& params)
{
  m_mac->DoCschedCellConfigCnf (params);
}

void
MmWaveMacMemberMacCschedSapUser::CschedUeConfigCnf (const struct CschedUeConfigCnfParameters& params)
{
  m_mac->DoCschedUeConfigCnf (params);
}

void
MmWaveMacMemberMacCschedSapUser::CschedLcConfigCnf (const struct CschedLcConfigCnfParameters& params)
{
  m_mac->DoCschedLcConfigCnf (params);
}

void
MmWaveMacMemberMacCschedSapUser::CschedLcReleaseCnf (const struct CschedLcReleaseCnfParameters& params)
{
  m_mac->DoCschedLcReleaseCnf (params);
}

void
MmWaveMacMemberMacCschedSapUser::CschedUeReleaseCnf (const struct CschedUeReleaseCnfParameters& params)
{
  m_mac->DoCschedUeReleaseCnf (params);
}

void
MmWaveMacMemberMacCschedSapUser::CschedUeConfigUpdateInd (const struct CschedUeConfigUpdateIndParameters& params)
{
  m_mac->DoCschedUeConfigUpdateInd (params);
}

void
MmWaveMacMemberMacCschedSapUser::CschedCellConfigUpdateInd (const struct CschedCellConfigUpdateIndParameters& params)
{
  m_mac->DoCschedCellConfigUpdateInd (params);
}

TypeId
MmWaveEnbMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveEnbMac")
    .SetParent<MmWaveMac> ()
    .AddConstructor<MmWaveEnbMac> ()
    .AddTraceSource ("DlScheduling",
                     "Information regarding DL scheduling.",
                     MakeTraceSourceAccessor (&MmWaveEnbMac::m_dlScheduling),
                     "ns3::LteEnbMac::DlSchedulingTracedCallback")
    .AddTraceSource ("SrReq",
                     "Information regarding received scheduling request.",
                     MakeTraceSourceAccessor (&MmWaveEnbMac::m_srCallback),
                     "ns3::MmWaveEnbMac::SrTracedCallback")
  ;
  return tid;
}

MmWaveEnbMac::MmWaveEnbMac (void) :
  m_frameNum (0),
  m_subframeNum (0),
  m_slotNum (0),
  m_varTtiNum (0),
  m_tbUid (0)
{
  NS_LOG_FUNCTION (this);
  m_cmacSapProvider = new MmWaveEnbMacMemberEnbCmacSapProvider (this);
  m_macSapProvider = new EnbMacMemberLteMacSapProvider<MmWaveEnbMac> (this);
  m_phySapUser = new MmWaveMacEnbMemberPhySapUser (this);
  m_macSchedSapUser = new MmWaveMacMemberMacSchedSapUser (this);
  m_macCschedSapUser = new MmWaveMacMemberMacCschedSapUser (this);
  m_ccmMacSapProvider = new MemberLteCcmMacSapProvider<MmWaveEnbMac> (this);
}

MmWaveEnbMac::~MmWaveEnbMac (void)
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveEnbMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_dlCqiReceived.clear ();
  m_ulCqiReceived.clear ();
  m_ulCeReceived.clear ();
  //  m_dlHarqInfoListReceived.clear ();
  //  m_ulHarqInfoListReceived.clear ();
  m_miDlHarqProcessesPackets.clear ();
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_macSchedSapUser;
  delete m_macCschedSapUser;
  //  delete m_macCschedSapUser;
  delete m_phySapUser;
}

void
MmWaveEnbMac::SetComponentCarrierId (uint8_t index)
{
  m_componentCarrierId = index;
}

void
MmWaveEnbMac::SetConfigurationParameters (Ptr<MmWavePhyMacCommon> ptrConfig)
{
  m_phyMacConfig = ptrConfig;
}

Ptr<MmWavePhyMacCommon>
MmWaveEnbMac::GetConfigurationParameters (void) const
{
  return m_phyMacConfig;
}

void
MmWaveEnbMac::ReceiveRachPreamble (uint32_t raId)
{
  ++m_receivedRachPreambleCount[raId];
}

LteMacSapProvider*
MmWaveEnbMac::GetMacSapProvider (void)
{
  return m_macSapProvider;
}

LteEnbCmacSapProvider*
MmWaveEnbMac::GetEnbCmacSapProvider (void)
{
  return m_cmacSapProvider;
}
void
MmWaveEnbMac::SetEnbCmacSapUser (LteEnbCmacSapUser* s)
{
  m_cmacSapUser = s;
}

void
MmWaveEnbMac::SetLteCcmMacSapUser (LteCcmMacSapUser* s)
{
  m_ccmMacSapUser = s;
}


LteCcmMacSapProvider*
MmWaveEnbMac::GetLteCcmMacSapProvider ()
{
  return m_ccmMacSapProvider;
}

void
MmWaveEnbMac::DoSlotIndication (SfnSf sfnSf)
{
  m_frameNum = sfnSf.m_frameNum;
  m_subframeNum = sfnSf.m_subframeNum;
  m_slotNum = sfnSf.m_slotNum;
  m_varTtiNum = sfnSf.m_varTtiNum;

  // --- DOWNLINK ---
  // Send Dl-CQI info to the scheduler    if(m_dlCqiReceived.size () > 0)
  {
    MmWaveMacSchedSapProvider::SchedDlCqiInfoReqParameters dlCqiInfoReq;
    dlCqiInfoReq.m_sfnsf = sfnSf;

    dlCqiInfoReq.m_cqiList.insert (dlCqiInfoReq.m_cqiList.begin (), m_dlCqiReceived.begin (), m_dlCqiReceived.end ());
    m_dlCqiReceived.erase (m_dlCqiReceived.begin (), m_dlCqiReceived.end ());

    m_macSchedSapProvider->SchedDlCqiInfoReq (dlCqiInfoReq);
  }

  if (!m_receivedRachPreambleCount.empty ())
    {
      // process received RACH preambles and notify the scheduler
      Ptr<MmWaveRarMessage> rarMsg = Create<MmWaveRarMessage> ();

      for (std::map<uint8_t, uint32_t>::const_iterator it = m_receivedRachPreambleCount.begin ();
           it != m_receivedRachPreambleCount.end ();
           ++it)
        {
          uint16_t rnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
          NS_LOG_INFO (rnti);
          MmWaveRarMessage::Rar rar;
          rar.rapId = (*it).first;
          rar.rarPayload.m_rnti = rnti;
          rarMsg->AddRar (rar);
          //NS_ASSERT_MSG((*it).second ==1, "Two user send the same Rach ID, collision detected");
        }
      m_phySapProvider->SendControlMessage (rarMsg);
      m_receivedRachPreambleCount.clear ();
    }

  // --- UPLINK ---
  // Send UL-CQI info to the scheduler
  for (uint16_t i = 0; i < m_ulCqiReceived.size (); i++)
    {
      //m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & frameNum) << 16) | ((0xFF & subframeNum) << 8) | (0xFF & varTtiNum);
      m_macSchedSapProvider->SchedUlCqiInfoReq (m_ulCqiReceived.at (i));
    }
  m_ulCqiReceived.clear ();

  // Send SR info to the scheduler
  {
    MmWaveMacSchedSapProvider::SchedUlSrInfoReqParameters params;
    params.m_snfSf = SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0);
    params.m_srList.insert (params.m_srList.begin(), m_srRntiList.begin (), m_srRntiList.end ());
    m_srRntiList.clear();

    m_macSchedSapProvider->SchedUlSrInfoReq (params);
  }

  // Send UL BSR reports to the scheduler
  if (m_ulCeReceived.size () > 0)
    {
      MmWaveMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters ulMacReq;
      ulMacReq.m_sfnSf = sfnSf;
      ulMacReq.m_macCeList.insert (ulMacReq.m_macCeList.begin (), m_ulCeReceived.begin (), m_ulCeReceived.end ());
      m_ulCeReceived.erase (m_ulCeReceived.begin (), m_ulCeReceived.end ());
      m_macSchedSapProvider->SchedUlMacCtrlInfoReq (ulMacReq);
    }

  if (m_varTtiNum == 0)
    {
      SfnSf dlSfn = SfnSf (m_frameNum, m_subframeNum, m_slotNum, 0).IncreaseNoOfSlotsWithLatency (
        m_phyMacConfig->GetL1L2CtrlLatency (),
        m_phyMacConfig->GetSlotsPerSubframe (),
        m_phyMacConfig->GetSubframesPerFrame ());
      SfnSf ulSfn = dlSfn.CalculateUplinkSlot (
        m_phyMacConfig->GetUlSchedDelay (),
        m_phyMacConfig->GetSlotsPerSubframe (),
        m_phyMacConfig->GetSubframesPerFrame ());

      MmWaveMacSchedSapProvider::SchedDlTriggerReqParameters dlParams;
      MmWaveMacSchedSapProvider::SchedUlTriggerReqParameters ulParams;

      dlParams.m_snfSf = dlSfn;
      ulParams.m_snfSf = ulSfn;

      // Forward DL HARQ feebacks collected during last subframe TTI
      if (m_dlHarqInfoReceived.size () > 0)
        {
          dlParams.m_dlHarqInfoList = m_dlHarqInfoReceived;
          // empty local buffer
          m_dlHarqInfoReceived.clear ();
        }

      // Forward UL HARQ feebacks collected during last TTI
      if (m_ulHarqInfoReceived.size () > 0)
        {
          ulParams.m_ulHarqInfoList = m_ulHarqInfoReceived;
          // empty local buffer
          m_ulHarqInfoReceived.clear ();
        }

      {
        for (const auto & ue : m_rlcAttached)
          {
            MmWaveMacCschedSapProvider::CschedUeConfigReqParameters params;
            params.m_rnti = ue.first;
            params.m_beamId = m_phySapProvider->GetBeamId (ue.first);
            params.m_transmissionMode = 0;   // set to default value (SISO) for avoiding random initialization (valgrind error)
            m_macCschedSapProvider->CschedUeConfigReq (params);
          }
      }

      m_macSchedSapProvider->SchedUlTriggerReq (ulParams);
      m_macSchedSapProvider->SchedDlTriggerReq (dlParams);
    }
}

void
MmWaveEnbMac::SetMcs (int mcs)
{
  m_macSchedSapProvider->SchedSetMcs (mcs);
}

void
MmWaveEnbMac::AssociateUeMAC (uint64_t imsi)
{
  //NS_LOG_UNCOND (this<<"Associate UE (imsi:"<< imsi<<" ) with enb");

  //m_associatedUe.push_back (imsi);

}

void
MmWaveEnbMac::SetForwardUpCallback (Callback <void, Ptr<Packet> > cb)
{
  m_forwardUpCallback = cb;
}

void
MmWaveEnbMac::ReceiveBsrMessage  (MacCeElement bsr)
{
  NS_LOG_FUNCTION (this);
  // in order to use existing SAP interfaces we need to convert MacCeElement to MacCeListElement_s

  MacCeListElement_s mcle;
  mcle.m_rnti = bsr.m_rnti;
  mcle.m_macCeValue.m_bufferStatus = bsr.m_macCeValue.m_bufferStatus;
  mcle.m_macCeValue.m_crnti = bsr.m_macCeValue.m_crnti;
  mcle.m_macCeValue.m_phr = bsr.m_macCeValue.m_phr;
  mcle.m_macCeValue.m_bufferStatus = bsr.m_macCeValue.m_bufferStatus;

  if (bsr.m_macCeType == MacCeElement::BSR)
    {
      mcle.m_macCeType = MacCeListElement_s::BSR;
    }
  else if (bsr.m_macCeType == MacCeElement::CRNTI)
    {
      mcle.m_macCeType = MacCeListElement_s::CRNTI;
    }
  else if (bsr.m_macCeType == MacCeElement::PHR)
    {
      mcle.m_macCeType = MacCeListElement_s::PHR;
    }

  m_ccmMacSapUser->UlReceiveMacCe (mcle, m_componentCarrierId);
}

void
MmWaveEnbMac::DoReportMacCeToScheduler (MacCeListElement_s bsr)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (this << " bsr Size " << (uint16_t) m_ulCeReceived.size ());
  uint32_t size = 0;

  //send to LteCcmMacSapUser
  //convert MacCeListElement_s to MacCeElement

  MacCeElement mce;
  mce.m_rnti = bsr.m_rnti;
  mce.m_macCeValue.m_bufferStatus = bsr.m_macCeValue.m_bufferStatus;
  mce.m_macCeValue.m_crnti = bsr.m_macCeValue.m_crnti;
  mce.m_macCeValue.m_phr = bsr.m_macCeValue.m_phr;
  mce.m_macCeValue.m_bufferStatus = bsr.m_macCeValue.m_bufferStatus;

  if (bsr.m_macCeType == MacCeListElement_s::BSR)
    {
      mce.m_macCeType = MacCeElement::BSR;
    }
  else if (bsr.m_macCeType == MacCeListElement_s::CRNTI)
    {
      mce.m_macCeType = MacCeElement::CRNTI;
    }
  else if (bsr.m_macCeType == MacCeListElement_s::PHR)
    {
      mce.m_macCeType = MacCeElement::PHR;
    }

  for (const auto & v : bsr.m_macCeValue.m_bufferStatus)
    {
      size += v;
    }

  m_ulCeReceived.push_back (mce);   // this to called when LteUlCcmSapProvider::ReportMacCeToScheduler is called
  NS_LOG_DEBUG (" Reported by UE " << static_cast<uint32_t> (bsr.m_macCeValue.m_crnti) <<
                " size " << size << " bsr vector ize after push_back " <<
                static_cast<uint32_t> (m_ulCeReceived.size ()));
}

void
MmWaveEnbMac::DoReportSrToScheduler(uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  m_srRntiList.push_back (rnti);
  m_srCallback (m_componentCarrierId, rnti);
}

void
MmWaveEnbMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  LteRadioBearerTag tag;
  p->RemovePacketTag (tag);
  uint16_t rnti = tag.GetRnti ();
  MmWaveMacPduHeader macHeader;
  p->RemoveHeader (macHeader);
  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (rnti);
  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << rnti);
  std::vector<MacSubheader> macSubheaders = macHeader.GetSubheaders ();
  uint32_t currPos = 0;
  for (unsigned ipdu = 0; ipdu < macSubheaders.size (); ipdu++)
    {
      if (macSubheaders[ipdu].m_size == 0)
        {
          continue;
        }
      std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (macSubheaders[ipdu].m_lcid);
      NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID" << macSubheaders[ipdu].m_lcid);
      Ptr<Packet> rlcPdu;
      if ((p->GetSize () - currPos) < (uint32_t)macSubheaders[ipdu].m_size)
        {
          NS_LOG_ERROR ("Packet size less than specified in MAC header (actual= " \
                        << p->GetSize () << " header= " << (uint32_t)macSubheaders[ipdu].m_size << ")" );
        }
      else if ((p->GetSize () - currPos) > (uint32_t)macSubheaders[ipdu].m_size)
        {
          NS_LOG_DEBUG ("Fragmenting MAC PDU (packet size greater than specified in MAC header (actual= " \
                        << p->GetSize () << " header= " << (uint32_t)macSubheaders[ipdu].m_size << ")" );
          rlcPdu = p->CreateFragment (currPos, macSubheaders[ipdu].m_size);
          currPos += macSubheaders[ipdu].m_size;
          (*lcidIt).second->ReceivePdu (LteMacSapUser::ReceivePduParameters (rlcPdu, rnti, macSubheaders[ipdu].m_lcid));
        }
      else
        {
          rlcPdu = p->CreateFragment (currPos, p->GetSize () - currPos);
          currPos = p->GetSize ();
          (*lcidIt).second->ReceivePdu (LteMacSapUser::ReceivePduParameters (rlcPdu, rnti, macSubheaders[ipdu].m_lcid));
        }
      NS_LOG_DEBUG ("Enb Mac Rx Packet, Rnti:" << rnti << " lcid:" << macSubheaders[ipdu].m_lcid << " size:" << macSubheaders[ipdu].m_size);
    }
}

MmWaveEnbPhySapUser*
MmWaveEnbMac::GetPhySapUser ()
{
  return m_phySapUser;
}

void
MmWaveEnbMac::SetPhySapProvider (MmWavePhySapProvider* ptr)
{
  m_phySapProvider = ptr;
}

MmWaveMacSchedSapUser*
MmWaveEnbMac::GetMmWaveMacSchedSapUser ()
{
  return m_macSchedSapUser;
}

void
MmWaveEnbMac::SetMmWaveMacSchedSapProvider (MmWaveMacSchedSapProvider* ptr)
{
  m_macSchedSapProvider = ptr;
}

MmWaveMacCschedSapUser*
MmWaveEnbMac::GetMmWaveMacCschedSapUser ()
{
  return m_macCschedSapUser;
}

void
MmWaveEnbMac::SetMmWaveMacCschedSapProvider (MmWaveMacCschedSapProvider* ptr)
{
  m_macCschedSapProvider = ptr;
}

void
MmWaveEnbMac::DoUlCqiReport (MmWaveMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi)
{
  if (ulcqi.m_ulCqi.m_type == UlCqiInfo::PUSCH)
    {
      NS_LOG_DEBUG (this << " eNB rxed an PUSCH UL-CQI");
    }
  else if (ulcqi.m_ulCqi.m_type == UlCqiInfo::SRS)
    {
      NS_LOG_DEBUG (this << " eNB rxed an SRS UL-CQI");
    }
  NS_LOG_INFO ("*** UL CQI report SINR " << LteFfConverter::fpS11dot3toDouble (ulcqi.m_ulCqi.m_sinr[0]) <<
               " frame " << m_frameNum <<
               " subframe " << m_subframeNum <<
               " slot" << m_slotNum <<
               " varTtiNum " << m_varTtiNum );

  NS_ASSERT (ulcqi.m_sfnSf.m_varTtiNum != 0);
  m_ulCqiReceived.push_back (ulcqi);
}


void
MmWaveEnbMac::DoReceiveControlMessage  (Ptr<MmWaveControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  switch (msg->GetMessageType ())
    {
    case (MmWaveControlMessage::SR):
      {
        // Report it to the CCM. Then he will call the right MAC
        Ptr<MmWaveSRMessage> sr = DynamicCast<MmWaveSRMessage> (msg);
        m_ccmMacSapUser->UlReceiveSr (sr->GetRNTI (), m_componentCarrierId);
        break;
      }
    case (MmWaveControlMessage::DL_CQI):
      {
        Ptr<MmWaveDlCqiMessage> cqi = DynamicCast<MmWaveDlCqiMessage> (msg);
        DlCqiInfo cqiElement = cqi->GetDlCqi ();
        NS_ASSERT (cqiElement.m_rnti != 0);
        m_dlCqiReceived.push_back (cqiElement);
        break;
      }
    case (MmWaveControlMessage::BSR):
      {
        Ptr<MmWaveBsrMessage> bsr = DynamicCast<MmWaveBsrMessage> (msg);
        ReceiveBsrMessage (bsr->GetBsr ());
        break;
      }
    case (MmWaveControlMessage::DL_HARQ):
      {
        Ptr<MmWaveDlHarqFeedbackMessage> dlharq = DynamicCast<MmWaveDlHarqFeedbackMessage> (msg);
        DoDlHarqFeedback (dlharq->GetDlHarqFeedback ());
        break;
      }
    default:
      NS_LOG_INFO ("Control message not supported/expected");
    }

}

void
MmWaveEnbMac::DoUlHarqFeedback (UlHarqInfo params)
{
  NS_LOG_FUNCTION (this);
  m_ulHarqInfoReceived.push_back (params);
}

void
MmWaveEnbMac::DoDlHarqFeedback (DlHarqInfo params)
{
  NS_LOG_FUNCTION (this);
  // Update HARQ buffer
  std::map <uint16_t, MmWaveDlHarqProcessesBuffer_t>::iterator it =  m_miDlHarqProcessesPackets.find (params.m_rnti);
  NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());

  if (params.m_harqStatus == DlHarqInfo::ACK)
    {
      // discard buffer
      Ptr<PacketBurst> emptyBuf = CreateObject <PacketBurst> ();
      (*it).second.at (params.m_harqProcessId).m_pktBurst = emptyBuf;
      NS_LOG_DEBUG (this << " HARQ-ACK UE " << params.m_rnti << " harqId " << (uint16_t)params.m_harqProcessId);
    }
  else if (params.m_harqStatus == DlHarqInfo::NACK)
    {
      /*if (params.m_numRetx == 3)
 {
  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (params.m_rnti);
  for (unsigned i = 0; i < (*it).second.at (params.m_harqProcessId).m_lcidList.size (); i++)
  {
  std::map<uint8_t, LteMacSapUser*>::iterator lcidIt =
    rntiIt->second.find ((*it).second.at (params.m_harqProcessId).m_lcidList[i]);
  NS_ASSERT (lcidIt != rntiIt->second.end ());
  lcidIt->second->NotifyDlHarqDeliveryFailure (params.m_harqProcessId);
  }
 }*/
      NS_LOG_DEBUG (this << " HARQ-NACK UE " << params.m_rnti << " harqId " << (uint16_t)params.m_harqProcessId);
    }
  else
    {
      NS_FATAL_ERROR (" HARQ functionality not implemented");
    }

  m_dlHarqInfoReceived.push_back (params);
}

void
MmWaveEnbMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);
  MmWaveMacSchedSapProvider::SchedDlRlcBufferReqParameters schedParams;
  schedParams.m_logicalChannelIdentity = params.lcid;
  schedParams.m_rlcRetransmissionHolDelay = params.retxQueueHolDelay;
  schedParams.m_rlcRetransmissionQueueSize = params.retxQueueSize;
  schedParams.m_rlcStatusPduSize = params.statusPduSize;
  schedParams.m_rlcTransmissionQueueHolDelay = params.txQueueHolDelay;
  schedParams.m_rlcTransmissionQueueSize = params.txQueueSize;
  schedParams.m_rnti = params.rnti;

  m_macSchedSapProvider->SchedDlRlcBufferReq (schedParams);
}

// forwarded from LteMacSapProvider
void
MmWaveEnbMac::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
{
  params.componentCarrierId = m_componentCarrierId;
  // TB UID passed back along with RLC data as HARQ process ID
  uint32_t tbMapKey = ((params.rnti & 0xFFFF) << 8) | (params.harqProcessId & 0xFF);
  std::map<uint32_t, struct MacPduInfo>::iterator it = m_macPduMap.find (tbMapKey);
  if (it == m_macPduMap.end ())
    {
      NS_FATAL_ERROR ("No MAC PDU storage element found for this TB UID/RNTI");
    }
  else
    {
      if (it->second.m_pdu == 0)
        {
          it->second.m_pdu = params.pdu;
        }
      else
        {
          it->second.m_pdu->AddAtEnd (params.pdu);   // append to MAC PDU
        }

      MacSubheader subheader (params.lcid, params.pdu->GetSize ());
      it->second.m_macHeader.AddSubheader (subheader);   // add RLC PDU sub-header into MAC header
      it->second.m_numRlcPdu++;
    }
}

void
MmWaveEnbMac::DoSchedConfigIndication (MmWaveMacSchedSapUser::SchedConfigIndParameters ind)
{
  m_phySapProvider->SetSlotAllocInfo (ind.m_slotAllocInfo);
  //m_phySapProvider->SetUlSlotAllocInfo (ind.m_ulSlotAllocInfo);

  for (unsigned islot = 0; islot < ind.m_slotAllocInfo.m_varTtiAllocInfo.size (); islot++)
    {
      VarTtiAllocInfo &varTtiAllocInfo = ind.m_slotAllocInfo.m_varTtiAllocInfo[islot];
      if (varTtiAllocInfo.m_varTtiType != VarTtiAllocInfo::CTRL && varTtiAllocInfo.m_tddMode == VarTtiAllocInfo::DL)
        {
          uint16_t rnti = varTtiAllocInfo.m_dci->m_rnti;
          std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (rnti);
          if (rntiIt == m_rlcAttached.end ())
            {
              NS_FATAL_ERROR ("Scheduled UE " << rntiIt->first << " not attached");
            }
          else
            {

              // Call RLC entities to generate RLC PDUs
              auto dciElem = varTtiAllocInfo.m_dci;
              uint8_t tbUid = dciElem->m_harqProcess;

              // update Harq Processes
              if (dciElem->m_ndi == 1)
                {
                  NS_ASSERT (dciElem->m_format == DciInfoElementTdma::DL);
                  std::vector<RlcPduInfo> &rlcPduInfo = varTtiAllocInfo.m_rlcPduInfo;
                  NS_ASSERT (rlcPduInfo.size () > 0);
                  SfnSf pduSfn = ind.m_sfnSf;
                  pduSfn.m_varTtiNum = varTtiAllocInfo.m_dci->m_symStart;
                  MacPduInfo macPduInfo (pduSfn, varTtiAllocInfo.m_dci->m_tbSize, rlcPduInfo.size (), *dciElem.get ());
                  // insert into MAC PDU map
                  uint32_t tbMapKey = ((rnti & 0xFFFF) << 8) | (tbUid & 0xFF);
                  std::pair <std::map<uint32_t, struct MacPduInfo>::iterator, bool> mapRet =
                    m_macPduMap.insert (std::pair<uint32_t, struct MacPduInfo> (tbMapKey, macPduInfo));
                  if (!mapRet.second)
                    {
                      NS_FATAL_ERROR ("MAC PDU map element exists");
                    }

                  // new data -> force emptying correspondent harq pkt buffer
                  std::map <uint16_t, MmWaveDlHarqProcessesBuffer_t>::iterator harqIt = m_miDlHarqProcessesPackets.find (rnti);
                  NS_ASSERT (harqIt != m_miDlHarqProcessesPackets.end ());
                  Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
                  harqIt->second.at (tbUid).m_pktBurst = pb;
                  harqIt->second.at (tbUid).m_lcidList.clear ();

                  std::map<uint32_t, struct MacPduInfo>::iterator pduMapIt = mapRet.first;
                  pduMapIt->second.m_numRlcPdu = 0;
                  for (unsigned int ipdu = 0; ipdu < rlcPduInfo.size (); ipdu++)
                    {
                      NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << rnti);
                      std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (rlcPduInfo[ipdu].m_lcid);
                      NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID" << rlcPduInfo[ipdu].m_lcid);
                      NS_LOG_DEBUG ("Notifying RLC of TX opportunity for TB " << (unsigned int)tbUid << " PDU num " << ipdu << " size " << (unsigned int) rlcPduInfo[ipdu].m_size);
                      MacSubheader subheader (rlcPduInfo[ipdu].m_lcid, rlcPduInfo[ipdu].m_size);

                      // The MAC and RLC already consider 2 bytes for the header.
                      // that's a repetition, and prevent transmitting very small
                      // portions.
                      //(*lcidIt).second->NotifyTxOpportunity ((rlcPduInfo[ipdu].m_size)-subheader.GetSize (), 0, tbUid, m_componentCarrierId, rnti, rlcPduInfo[ipdu].m_lcid);

                      (*lcidIt).second->NotifyTxOpportunity (LteMacSapUser::TxOpportunityParameters ((rlcPduInfo[ipdu].m_size), 0, tbUid, m_componentCarrierId, rnti, rlcPduInfo[ipdu].m_lcid));
                      harqIt->second.at (tbUid).m_lcidList.push_back (rlcPduInfo[ipdu].m_lcid);
                    }

                  if (pduMapIt->second.m_numRlcPdu == 0)
                    {
                      MacSubheader subheader (3, 0);    // add subheader for empty packet
                      pduMapIt->second.m_macHeader.AddSubheader (subheader);
                    }
                  pduMapIt->second.m_pdu->AddHeader (pduMapIt->second.m_macHeader);

                  MmWaveMacPduHeader hdrTst;
                  pduMapIt->second.m_pdu->PeekHeader (hdrTst);

                  NS_ASSERT (pduMapIt->second.m_pdu->GetSize () > 0);
                  LteRadioBearerTag bearerTag (rnti, pduMapIt->second.m_size, 0);
                  pduMapIt->second.m_pdu->AddPacketTag (bearerTag);
                  NS_LOG_DEBUG ("eNB sending MAC pdu size " << pduMapIt->second.m_pdu->GetSize ());
                  for (unsigned i = 0; i < pduMapIt->second.m_macHeader.GetSubheaders ().size (); i++)
                    {
                      NS_LOG_DEBUG ("Subheader " << i << " size " << pduMapIt->second.m_macHeader.GetSubheaders ().at (i).m_size);
                    }
                  NS_LOG_DEBUG ("Total MAC PDU size " << pduMapIt->second.m_pdu->GetSize ());
                  harqIt->second.at (tbUid).m_pktBurst->AddPacket (pduMapIt->second.m_pdu);

                  m_phySapProvider->SendMacPdu (pduMapIt->second.m_pdu);
                  m_macPduMap.erase (pduMapIt);    // delete map entry

                  m_dlScheduling (ind.m_sfnSf.m_frameNum, ind.m_sfnSf.m_subframeNum, ind.m_sfnSf.m_slotNum,
                                  dciElem->m_tbSize, dciElem->m_mcs, dciElem->m_rnti, m_componentCarrierId);


                }
              else
                {
                  NS_LOG_INFO ("DL retransmission");
                  if (dciElem->m_tbSize > 0)
                    {

                      //uint16_t rnti,
                      //uint8_t mcs0, uint16_t tbs0Size,
                      //uint8_t mcs1, uint16_t tbs1Size);
                      // HARQ retransmission -> retrieve TB from HARQ buffer

                      std::map <uint16_t, MmWaveDlHarqProcessesBuffer_t>::iterator it = m_miDlHarqProcessesPackets.find (rnti);
                      NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
                      Ptr<PacketBurst> pb = it->second.at (tbUid).m_pktBurst;
                      for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
                        {
                          Ptr<Packet> pkt = (*j)->Copy ();
                          MmWaveMacPduTag tag;         // update PDU tag for retransmission
                          if (!pkt->RemovePacketTag (tag))
                            {
                              NS_FATAL_ERROR ("No MAC PDU tag");
                            }
                          tag.SetSfn (SfnSf (ind.m_sfnSf.m_frameNum, ind.m_sfnSf.m_subframeNum, ind.m_sfnSf.m_slotNum, dciElem->m_symStart));
                          tag.SetSymStart (dciElem->m_symStart);
                          tag.SetNumSym (dciElem->m_numSym);
                          pkt->AddPacketTag (tag);
                          m_phySapProvider->SendMacPdu (pkt);
                        }
                    }
                }
            }
        }
    }
}

uint8_t MmWaveEnbMac::AllocateTbUid (void)
{
  return m_tbUid++;
}

// ////////////////////////////////////////////
// CMAC SAP
// ////////////////////////////////////////////

void
MmWaveEnbMac::DoConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << " ulBandwidth=" << (uint16_t) ulBandwidth << " dlBandwidth=" << (uint16_t) dlBandwidth);
  MmWaveMacCschedSapProvider::CschedCellConfigReqParameters params;
  // Configure the subset of parameters used by FfMacScheduler
  params.m_ulBandwidth = ulBandwidth;
  params.m_dlBandwidth = dlBandwidth;
  //m_macChTtiDelay = m_phySapProvider->GetMacChTtiDelay ();  // Gets set by MmWavePhyMacCommon
  m_macCschedSapProvider->CschedCellConfigReq (params);
}

void
MmWaveEnbMac::BeamChangeReport (AntennaArrayModel::BeamId beamId, uint8_t rnti)
{
  MmWaveMacCschedSapProvider::CschedUeConfigReqParameters params;
  params.m_rnti = rnti;
  params.m_beamId = beamId;
  params.m_transmissionMode = 0;   // set to default value (SISO) for avoiding random initialization (valgrind error)
  m_macCschedSapProvider->CschedUeConfigReq (params);
}

void
MmWaveEnbMac::DoAddUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " rnti=" << rnti);
  std::map<uint8_t, LteMacSapUser*> empty;
  std::pair <std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator, bool>
  ret = m_rlcAttached.insert (std::pair <uint16_t,  std::map<uint8_t, LteMacSapUser*> >
                                (rnti, empty));
  NS_ASSERT_MSG (ret.second, "element already present, RNTI already existed");
  //m_associatedUe.push_back (rnti);

  MmWaveMacCschedSapProvider::CschedUeConfigReqParameters params;
  params.m_rnti = rnti;
  params.m_beamId = m_phySapProvider->GetBeamId (rnti);
  params.m_transmissionMode = 0;   // set to default value (SISO) for avoiding random initialization (valgrind error)
  m_macCschedSapProvider->CschedUeConfigReq (params);

  // Create DL transmission HARQ buffers
  MmWaveDlHarqProcessesBuffer_t buf;
  uint16_t harqNum = m_phyMacConfig->GetNumHarqProcess ();
  buf.resize (harqNum);
  for (uint8_t i = 0; i < harqNum; i++)
    {
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      buf.at (i).m_pktBurst = pb;
    }
  m_miDlHarqProcessesPackets.insert (std::pair <uint16_t, MmWaveDlHarqProcessesBuffer_t> (rnti, buf));

}

void
MmWaveEnbMac::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " rnti=" << rnti);
  MmWaveMacCschedSapProvider::CschedUeReleaseReqParameters params;
  params.m_rnti = rnti;
  m_macCschedSapProvider->CschedUeReleaseReq (params);
  m_miDlHarqProcessesPackets.erase (rnti);
  m_rlcAttached.erase (rnti);
}

void
MmWaveEnbMac::DoAddLc (LteEnbCmacSapProvider::LcInfo lcinfo, LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_FUNCTION (this);

  std::map <LteFlowId_t, LteMacSapUser* >::iterator it;

  LteFlowId_t flow (lcinfo.rnti, lcinfo.lcId);

  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (lcinfo.rnti);
  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "RNTI not found");
  std::map<uint8_t, LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (lcinfo.lcId);
  if (lcidIt == rntiIt->second.end ())
    {
      rntiIt->second.insert (std::pair<uint8_t, LteMacSapUser*> (lcinfo.lcId, msu));
    }
  else
    {
      NS_LOG_ERROR ("LC already exists");
    }

  // CCCH (LCID 0) is pre-configured
  // see FF LTE MAC Scheduler
  // Interface Specification v1.11,
  // 4.3.4 logicalChannelConfigListElement
  if (lcinfo.lcId != 0)
    {
      struct MmWaveMacCschedSapProvider::CschedLcConfigReqParameters params;
      params.m_rnti = lcinfo.rnti;
      params.m_reconfigureFlag = false;

      struct LogicalChannelConfigListElement_s lccle;
      lccle.m_logicalChannelIdentity = lcinfo.lcId;
      lccle.m_logicalChannelGroup = lcinfo.lcGroup;
      lccle.m_direction = LogicalChannelConfigListElement_s::DIR_BOTH;
      lccle.m_qosBearerType = lcinfo.isGbr ? LogicalChannelConfigListElement_s::QBT_GBR : LogicalChannelConfigListElement_s::QBT_NON_GBR;
      lccle.m_qci = lcinfo.qci;
      lccle.m_eRabMaximulBitrateUl = lcinfo.mbrUl;
      lccle.m_eRabMaximulBitrateDl = lcinfo.mbrDl;
      lccle.m_eRabGuaranteedBitrateUl = lcinfo.gbrUl;
      lccle.m_eRabGuaranteedBitrateDl = lcinfo.gbrDl;
      params.m_logicalChannelConfigList.push_back (lccle);

      m_macCschedSapProvider->CschedLcConfigReq (params);
    }
}

void
MmWaveEnbMac::DoReconfigureLc (LteEnbCmacSapProvider::LcInfo lcinfo)
{
  NS_FATAL_ERROR ("not implemented");
}

void
MmWaveEnbMac::DoReleaseLc (uint16_t rnti, uint8_t lcid)
{
  //Find user based on rnti and then erase lcid stored against the same
  std::map <uint16_t, std::map<uint8_t, LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (rnti);
  rntiIt->second.erase (lcid);

  struct MmWaveMacCschedSapProvider::CschedLcReleaseReqParameters params;
  params.m_rnti = rnti;
  params.m_logicalChannelIdentity.push_back (lcid);
  m_macCschedSapProvider->CschedLcReleaseReq (params);
}

void
MmWaveEnbMac::UeUpdateConfigurationReq (LteEnbCmacSapProvider::UeConfig params)
{
  NS_LOG_FUNCTION (this);
  // propagates to scheduler
  MmWaveMacCschedSapProvider::CschedUeConfigReqParameters req;
  req.m_rnti = params.m_rnti;
  req.m_transmissionMode = params.m_transmissionMode;
  req.m_beamId = m_phySapProvider->GetBeamId (params.m_rnti);
  req.m_reconfigureFlag = true;
  m_macCschedSapProvider->CschedUeConfigReq (req);
}

LteEnbCmacSapProvider::RachConfig
MmWaveEnbMac::DoGetRachConfig ()
{
  return LteEnbCmacSapProvider::RachConfig ();
}

LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue
MmWaveEnbMac::DoAllocateNcRaPreamble (uint16_t rnti)
{
  return LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue ();
}

// ////////////////////////////////////////////
// CSCHED SAP
// ////////////////////////////////////////////


void
MmWaveEnbMac::DoCschedCellConfigCnf (MmWaveMacCschedSapUser::CschedCellConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveEnbMac::DoCschedUeConfigCnf (MmWaveMacCschedSapUser::CschedUeConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveEnbMac::DoCschedLcConfigCnf (MmWaveMacCschedSapUser::CschedLcConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
  // Call the CSCHED primitive
  // m_cschedSap->LcConfigCompleted();
}

void
MmWaveEnbMac::DoCschedLcReleaseCnf (MmWaveMacCschedSapUser::CschedLcReleaseCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveEnbMac::DoCschedUeReleaseCnf (MmWaveMacCschedSapUser::CschedUeReleaseCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveEnbMac::DoCschedUeConfigUpdateInd (MmWaveMacCschedSapUser::CschedUeConfigUpdateIndParameters params)
{
  NS_LOG_FUNCTION (this);
  // propagates to RRC
  LteEnbCmacSapUser::UeConfig ueConfigUpdate;
  ueConfigUpdate.m_rnti = params.m_rnti;
  ueConfigUpdate.m_transmissionMode = params.m_transmissionMode;
  m_cmacSapUser->RrcConfigurationUpdateInd (ueConfigUpdate);
}

void
MmWaveEnbMac::DoCschedCellConfigUpdateInd (MmWaveMacCschedSapUser::CschedCellConfigUpdateIndParameters params)
{
  NS_LOG_FUNCTION (this);
}

}
