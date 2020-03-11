/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015 NYU WIRELESS, Tandon School of Engineering, New York University
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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);

#include "mmwave-phy.h"
#include "mmwave-spectrum-value-helper.h"
#include "mmwave-spectrum-phy.h"
#include "mmwave-mac-pdu-tag.h"
#include "mmwave-net-device.h"
#include "mmwave-ue-net-device.h"
#include "mmwave-enb-net-device.h"
#include "beam-manager.h"
#include <ns3/boolean.h>

#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWavePhy");

NS_OBJECT_ENSURE_REGISTERED ( MmWavePhy);

/*   SAP   */
class MmWaveMemberPhySapProvider : public MmWavePhySapProvider
{
public:
  MmWaveMemberPhySapProvider (MmWavePhy* phy);

  virtual void SendMacPdu (Ptr<Packet> p ) override;

  virtual void SendControlMessage (Ptr<MmWaveControlMessage> msg) override;

  virtual void SendRachPreamble (uint8_t PreambleId, uint8_t Rnti) override;

  virtual void SetSlotAllocInfo (SlotAllocInfo slotAllocInfo) override;

  virtual BeamId GetBeamId (uint8_t rnti) const override;

  virtual Ptr<const SpectrumModel> GetSpectrumModel () const override;

  virtual void NotifyConnectionSuccessful () override;

  virtual uint16_t GetBwpId () const override;

  virtual uint16_t GetCellId () const override;

private:
  MmWavePhy* m_phy;
};

MmWaveMemberPhySapProvider::MmWaveMemberPhySapProvider (MmWavePhy* phy)
  : m_phy (phy)
{
  //  Nothing more to do
}

void
MmWaveMemberPhySapProvider::SendMacPdu (Ptr<Packet> p)
{
  m_phy->SetMacPdu (p);
}

void
MmWaveMemberPhySapProvider::SendControlMessage (Ptr<MmWaveControlMessage> msg)
{
  m_phy->EnqueueCtrlMessage (msg);  //May need to change
}

void
MmWaveMemberPhySapProvider::SendRachPreamble (uint8_t PreambleId, uint8_t Rnti)
{
  m_phy->SendRachPreamble (PreambleId, Rnti);
}

void
MmWaveMemberPhySapProvider::SetSlotAllocInfo (SlotAllocInfo slotAllocInfo)
{
  m_phy->PushBackSlotAllocInfo (slotAllocInfo);
}

BeamId
MmWaveMemberPhySapProvider::GetBeamId (uint8_t rnti) const
{
  return m_phy->GetBeamId (rnti);
}

Ptr<const SpectrumModel>
MmWaveMemberPhySapProvider::GetSpectrumModel() const
{
  return m_phy->GetSpectrumModel ();
}

void
MmWaveMemberPhySapProvider::NotifyConnectionSuccessful ()
{
  m_phy->NotifyConnectionSuccessful ();
}

uint16_t
MmWaveMemberPhySapProvider::GetBwpId() const
{
  return m_phy->GetBwpId ();
}

uint16_t
MmWaveMemberPhySapProvider::GetCellId() const
{
  return m_phy->GetCellId ();
}

/* ======= */

TypeId
MmWavePhy::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::MmWavePhy")
    .SetParent<Object> ()
  ;

  return tid;
}

std::vector<int>
MmWavePhy::FromRBGBitmaskToRBAssignment (const std::vector<uint8_t> rbgBitmask) const
{
  NS_ASSERT (rbgBitmask.size () == m_phyMacConfig->GetBandwidthInRbg ());
  std::vector<int> ret;

  for (uint32_t i = 0; i < rbgBitmask.size (); ++i)
    {
      if (rbgBitmask.at (i) == 1)
        {
          for (uint32_t k = 0; k < GetNumRbPerRbg (); ++k)
            {
              ret.push_back ((i * GetNumRbPerRbg ()) + k);
            }
        }
    }

  NS_ASSERT (static_cast<uint32_t> (std::count (rbgBitmask.begin (), rbgBitmask.end (), 1) * GetNumRbPerRbg ())
             == ret.size ());
  return ret;
}

MmWavePhy::MmWavePhy ()
  : m_currSlotAllocInfo (SfnSf (0,0,0,0)),
    m_tbDecodeLatencyUs (100.0)
{
  NS_LOG_FUNCTION (this);
  m_phySapProvider = new MmWaveMemberPhySapProvider (this);
}

MmWavePhy::~MmWavePhy ()
{
  NS_LOG_FUNCTION (this);
  m_slotAllocInfo.clear ();
  m_controlMessageQueue.clear ();
  delete m_phySapProvider;
}

void
MmWavePhy::InstallAntenna (const Ptr<ThreeGppAntennaArrayModel> &antenna)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_spectrumPhy != nullptr);

  m_beamManager = CreateObject<BeamManager>();
  m_beamManager->Configure(antenna);
}

void
MmWavePhy::SetDevice (Ptr<MmWaveNetDevice> d)
{
  NS_LOG_FUNCTION (this);
  m_netDevice = d;
}

void
MmWavePhy::InstallCentralFrequency (double f)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_centralFrequency >= 0.0);
  m_centralFrequency = f;
}

void
MmWavePhy::DoSetCellId (uint16_t bwpId)
{
  NS_LOG_FUNCTION (this);
  m_bwpId = bwpId;
}

void
MmWavePhy::SendRachPreamble (uint32_t PreambleId, uint32_t Rnti)
{
  NS_LOG_FUNCTION (this);
  // This function is called by the SAP, SO it has to stay at the L1L2CtrlDelay rule
  m_raPreambleId = PreambleId;
  Ptr<MmWaveRachPreambleMessage> msg = Create<MmWaveRachPreambleMessage> ();
  msg->SetRapId (PreambleId);
  EnqueueCtrlMessage (msg); // Enqueue at the end
}

void
MmWavePhy::SetMacPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  MmWaveMacPduTag tag;
  if (p->PeekPacketTag (tag))
    {
      NS_ASSERT ((tag.GetSfn ().m_slotNum >= 0) && (tag.GetSfn ().m_varTtiNum < m_phyMacConfig->GetSymbolsPerSlot ()));

      auto it = m_packetBurstMap.find (tag.GetSfn ().Encode ());

      if (it == m_packetBurstMap.end ())
        {
          it = m_packetBurstMap.insert (std::make_pair (tag.GetSfn ().Encode (), CreateObject<PacketBurst> ())).first;
        }
      it->second->AddPacket (p);
      NS_LOG_INFO ("Adding a packet for the Packet Burst of " << tag.GetSfn ());
    }
  else
    {
      NS_FATAL_ERROR ("No MAC packet PDU header available");
    }
}

void
MmWavePhy::NotifyConnectionSuccessful ()
{
  NS_LOG_FUNCTION (this);
  m_isConnected = true;
}

Ptr<PacketBurst>
MmWavePhy::GetPacketBurst (SfnSf sfn)
{
  NS_LOG_FUNCTION (this);
  Ptr<PacketBurst> pburst;
  auto it = m_packetBurstMap.find (sfn.Encode ());

  if (it == m_packetBurstMap.end ())
    {
      NS_LOG_ERROR ("Packet burst not found for " << sfn);
      return pburst;
    }
  else
    {
      pburst = it->second;
      m_packetBurstMap.erase (it);
    }
  return pburst;
}

Ptr<SpectrumValue>
MmWavePhy::GetNoisePowerSpectralDensity () const
{
  return MmWaveSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_noiseFigure, GetSpectrumModel ());
}

Ptr<SpectrumValue>
MmWavePhy::GetTxPowerSpectralDensity (const std::vector<int> &rbIndexVector) const
{
  Ptr<const SpectrumModel> sm = GetSpectrumModel ();

  return MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity  (m_txPower, rbIndexVector, sm, m_phyMacConfig->GetBandwidth());
}

double
MmWavePhy::GetCentralFrequency() const
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_centralFrequency < 0.0);
  return m_centralFrequency;
}

void
MmWavePhy::EnqueueCtrlMessage (const Ptr<MmWaveControlMessage> &m)
{
  NS_LOG_FUNCTION (this);

  m_controlMessageQueue.at (m_controlMessageQueue.size () - 1).push_back (m);
}

void
MmWavePhy::EnqueueCtrlMsgNow (const Ptr<MmWaveControlMessage> &msg)
{
  NS_LOG_FUNCTION (this);

  m_controlMessageQueue.at (0).push_back (msg);
}

void
MmWavePhy::EnqueueCtrlMsgNow (const std::list<Ptr<MmWaveControlMessage> > &listOfMsgs)
{
  for (const auto & msg : listOfMsgs)
    {
      m_controlMessageQueue.at (0).push_back (msg);
    }
}

void
MmWavePhy::EncodeCtrlMsg (const Ptr<MmWaveControlMessage> &msg)
{
  NS_LOG_FUNCTION (this);

  m_ctrlMsgs.push_back (msg);
}

void
MmWavePhy::InitializeMessageList ()
{
  NS_LOG_FUNCTION (this);
  m_controlMessageQueue.clear ();

  for (unsigned i = 0; i <= m_phyMacConfig->GetL1L2CtrlLatency (); i++)
    {
      m_controlMessageQueue.push_back (std::list<Ptr<MmWaveControlMessage> > ());
    }
}


std::list<Ptr<MmWaveControlMessage> >
MmWavePhy::PopCurrentSlotCtrlMsgs (void)
{
  NS_LOG_FUNCTION (this);
  if (m_controlMessageQueue.empty ())
    {
      std::list<Ptr<MmWaveControlMessage> > emptylist;
      return (emptylist);
    }

  if (m_controlMessageQueue.at (0).size () > 0)
    {
      std::list<Ptr<MmWaveControlMessage> > ret = m_controlMessageQueue.front ();
      m_controlMessageQueue.erase (m_controlMessageQueue.begin ());
      std::list<Ptr<MmWaveControlMessage> > newlist;
      m_controlMessageQueue.push_back (newlist);
      return (ret);
    }
  else
    {
      m_controlMessageQueue.erase (m_controlMessageQueue.begin ());
      std::list<Ptr<MmWaveControlMessage> > newlist;
      m_controlMessageQueue.push_back (newlist);
      std::list<Ptr<MmWaveControlMessage> > emptylist;
      return (emptylist);
    }
}

Ptr<MmWavePhyMacCommon>
MmWavePhy::GetConfigurationParameters (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyMacConfig;
}

void
MmWavePhy::InstallSpectrumPhy (const Ptr<MmWaveSpectrumPhy> &spectrumPhy)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_spectrumPhy != nullptr);
  m_spectrumPhy = spectrumPhy;
  if (m_phyMacConfig != nullptr)
    {
      m_spectrumPhy->SetNoisePowerSpectralDensity (GetNoisePowerSpectralDensity());
    }
}

uint16_t
MmWavePhy::GetBwpId () const
{
  return m_bwpId;
}

uint16_t
MmWavePhy::GetCellId () const
{
  if (!m_netDevice)
    {
      return UINT16_MAX;
    }
  auto enbNetDevice = DynamicCast<MmWaveEnbNetDevice> (m_netDevice);
  auto ueNetDevice = DynamicCast<MmWaveUeNetDevice> (m_netDevice);
  if (enbNetDevice)
    {
      return enbNetDevice->GetCellId ();
    }
  else
    {
      return ueNetDevice->GetCellId ();
    }
}

Ptr<MmWaveSpectrumPhy>
MmWavePhy::GetSpectrumPhy () const
{
  return m_spectrumPhy;
}


MmWavePhySapProvider*
MmWavePhy::GetPhySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_phySapProvider;
}

void
MmWavePhy::PushBackSlotAllocInfo (const SlotAllocInfo &slotAllocInfo)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("setting info for slot " << slotAllocInfo.m_sfnSf);

  // That's not so complex, as the list would typically be of 2 or 3 elements.
  bool updated = false;
  for (auto & alloc : m_slotAllocInfo)
    {
      if (alloc.m_sfnSf == slotAllocInfo.m_sfnSf)
        {
          NS_LOG_INFO ("Merging inside existing allocation");
          alloc.Merge (slotAllocInfo);
          updated = true;
          break;
        }
    }
  if (! updated)
    {
      m_slotAllocInfo.push_back (slotAllocInfo);
      m_slotAllocInfo.sort ();
      NS_LOG_INFO ("Pushing allocation at the end of the list");
    }

  std::stringstream output;

  for (const auto & alloc : m_slotAllocInfo)
    {
      output << alloc;
    }
  NS_LOG_INFO (output.str ());
}

void
MmWavePhy::PushFrontSlotAllocInfo (const SfnSf &newSfnSf,
                                   const SlotAllocInfo &slotAllocInfo)
{
  NS_LOG_FUNCTION (this);

  m_slotAllocInfo.push_front (slotAllocInfo);
  SfnSf currentSfn = newSfnSf;
  std::unordered_map<uint64_t, Ptr<PacketBurst>> newBursts; // map between new sfn and the packet burst
  std::unordered_map<uint64_t, uint64_t> sfnMap; // map between new and old sfn, for debugging

  // all the slot allocations  (and their packet burst) have to be "adjusted":
  // directly modify the sfn for the allocation, and temporarly store the
  // burst (along with the new sfn) into newBursts.
  for (auto it = m_slotAllocInfo.begin (); it != m_slotAllocInfo.end (); ++it)
    {
      auto slotSfn = it->m_sfnSf;
      for (const auto &alloc : it->m_varTtiAllocInfo)
        {
          if (alloc.m_dci->m_type == DciInfoElementTdma::DATA)
            {
              slotSfn.m_varTtiNum = alloc.m_dci->m_symStart;
              Ptr<PacketBurst> pburst = GetPacketBurst (slotSfn);
              if (pburst && pburst->GetNPackets() > 0)
                {
                  currentSfn.m_varTtiNum = alloc.m_dci->m_symStart;
                  newBursts.insert (std::make_pair (currentSfn.Encode(), pburst));
                  sfnMap.insert (std::make_pair (currentSfn.Encode(), it->m_sfnSf.Encode()));
                }
              else
                {
                  NS_LOG_INFO ("No packet burst found for " << slotSfn);
                }
            }
        }

      currentSfn.m_varTtiNum = 0;
      NS_LOG_INFO ("Set slot allocation for " << it->m_sfnSf << " to " << currentSfn);
      it->m_sfnSf = currentSfn;
      currentSfn = currentSfn.IncreaseNoOfSlots (m_phyMacConfig->GetSlotsPerSubframe(), m_phyMacConfig->GetSubframesPerFrame());
    }

  for (const auto & burstPair : newBursts)
    {
      SfnSf old, latest;
      old.Decode (sfnMap.at (burstPair.first));
      latest.Decode (burstPair.first);

      for (auto & p : burstPair.second->GetPackets())
        {
          MmWaveMacPduTag tag;
          bool ret = p->RemovePacketTag (tag);
          NS_ASSERT (ret);

          tag.SetSfn (latest);
          p->AddPacketTag (tag);
        }


      m_packetBurstMap.insert (std::make_pair (burstPair.first, burstPair.second));
      NS_LOG_INFO ("PacketBurst with " << burstPair.second->GetNPackets() <<
                   "packets for SFN " << old << " now moved to SFN " << latest);
    }
}


bool
MmWavePhy::SlotAllocInfoExists (const SfnSf &retVal) const
{
  NS_LOG_FUNCTION (this);
  for (const auto & alloc : m_slotAllocInfo)
    {
      if (alloc.m_sfnSf == retVal)
        {
          return true;
        }
    }
  return false;
}

SlotAllocInfo
MmWavePhy::RetrieveSlotAllocInfo ()
{
  NS_LOG_FUNCTION (this);
  SlotAllocInfo ret = *m_slotAllocInfo.begin ();
  m_slotAllocInfo.erase(m_slotAllocInfo.begin ());
  return ret;
}


SlotAllocInfo
MmWavePhy::RetrieveSlotAllocInfo (const SfnSf &sfnsf)
{
  NS_LOG_FUNCTION (" slot " << sfnsf);

  for (auto allocIt = m_slotAllocInfo.begin(); allocIt != m_slotAllocInfo.end (); ++allocIt)
    {
      if (allocIt->m_sfnSf == sfnsf)
        {
          SlotAllocInfo ret = *allocIt;
          m_slotAllocInfo.erase (allocIt);
          return ret;
        }
    }

  NS_FATAL_ERROR("Didn't found the slot");
  return SlotAllocInfo (sfnsf);
}

SlotAllocInfo &
MmWavePhy::PeekSlotAllocInfo (const SfnSf &sfnsf)
{
  NS_LOG_FUNCTION (this);
  for (auto & alloc : m_slotAllocInfo)
    {
      if (alloc.m_sfnSf == sfnsf)
        {
          return alloc;
        }
    }

  NS_FATAL_ERROR ("Didn't found the slot");
}

size_t
MmWavePhy::SlotAllocInfoSize() const
{
  NS_LOG_FUNCTION (this);
  return m_slotAllocInfo.size ();
}

bool
MmWavePhy::IsCtrlMsgListEmpty() const
{
  NS_LOG_FUNCTION (this);
  return m_controlMessageQueue.empty () || m_controlMessageQueue.at (0).empty();
}

Ptr<BeamManager> MmWavePhy::GetBeamManager() const
{
  return m_beamManager;
}

Ptr<const SpectrumModel>
MmWavePhy::GetSpectrumModel () const
{
  NS_LOG_FUNCTION (this);

  return MmWaveSpectrumValueHelper::GetSpectrumModel (m_phyMacConfig->GetBandwidthInRbs(),
                                                      GetCentralFrequency (),
                                                      m_phyMacConfig->GetNumScsPerRb(),
                                                      m_phyMacConfig->GetSubcarrierSpacing());
}

Ptr<const ThreeGppAntennaArrayModel>
MmWavePhy::GetAntennaArray() const
{
  return m_beamManager->GetAntennaArray ();
}

void
MmWavePhy::SetNoiseFigure (double d)
{
  m_noiseFigure = d;

  if (m_spectrumPhy)
    {
      m_spectrumPhy->SetNoisePowerSpectralDensity (GetNoisePowerSpectralDensity());
    }
}

double
MmWavePhy::GetNoiseFigure () const
{
  return m_noiseFigure;
}


void
MmWavePhy::SetTbDecodeLatency (Time us)
{
  m_tbDecodeLatencyUs = us;
}

Time
MmWavePhy::GetTbDecodeLatency (void) const
{
  return m_tbDecodeLatencyUs;
}


}
