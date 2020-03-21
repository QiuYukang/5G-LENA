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

  virtual uint32_t GetSymbolsPerSlot () const override;

  virtual Time GetSlotPeriod () const override;

  virtual uint32_t GetRbNum () const override;

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

uint32_t
MmWaveMemberPhySapProvider::GetSymbolsPerSlot() const
{
  return m_phy->GetSymbolsPerSlot ();
}

Time
MmWaveMemberPhySapProvider::GetSlotPeriod() const
{
  return m_phy->GetSlotPeriod ();
}

uint32_t
MmWaveMemberPhySapProvider::GetRbNum () const
{
  return m_phy->GetRbNum ();
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
MmWavePhy::SetNumerology (uint16_t numerology)
{
  NS_LOG_FUNCTION (this);

  m_numerology = numerology;
  m_slotsPerSubframe  = static_cast<uint16_t> (std::pow (2, numerology));
  m_slotPeriod = Seconds (0.001 / m_slotsPerSubframe);
  m_subcarrierSpacing = 15 * static_cast<uint32_t> (std::pow (2, numerology)) * 1000;
  m_symbolPeriod = (m_slotPeriod / m_symbolsPerSlot);

  UpdateRbNum ();

  NS_LOG_INFO (" Numerology configured:" << m_numerology <<
               " slots per subframe: " << m_slotsPerSubframe <<
               " slot period:" << m_slotPeriod <<
               " symbol period:" << m_symbolPeriod <<
               " subcarrier spacing: " << m_subcarrierSpacing <<
               " number of RBs: " << m_rbNum );
}

uint16_t
MmWavePhy::GetNumerology() const
{
  return m_numerology;
}

void
MmWavePhy::SetSymbolsPerSlot (uint16_t symbolsPerSlot)
{
  NS_LOG_FUNCTION (this);
  m_symbolsPerSlot = symbolsPerSlot;
  m_symbolPeriod = (m_slotPeriod / m_symbolsPerSlot);
}

uint32_t
MmWavePhy::GetSymbolsPerSlot () const
{
  return m_symbolsPerSlot;
}

Time
MmWavePhy::GetSlotPeriod () const
{
  NS_ABORT_IF (m_slotPeriod.IsNegative ());
  return m_slotPeriod;
}

uint32_t
MmWavePhy::GetNumScsPerRb ()
{
  return 12;
}

void
MmWavePhy::DoSetCellId (uint16_t cellId)
{
  NS_LOG_FUNCTION (this);
  m_cellId = cellId;
}

void
MmWavePhy::SendRachPreamble (uint32_t PreambleId, uint32_t Rnti)
{
  NS_LOG_FUNCTION (this);
  // This function is called by the SAP, SO it has to stay at the L1L2CtrlDelay rule
  m_raPreambleId = PreambleId;
  Ptr<MmWaveRachPreambleMessage> msg = Create<MmWaveRachPreambleMessage> ();
  msg->SetSourceBwp (GetBwpId ());
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
      NS_ASSERT (tag.GetSfn ().GetNumerology () == GetNumerology());
      uint64_t key = tag.GetSfn ().GetEncodingWithSymStart (tag.GetSymStart ());
      auto it = m_packetBurstMap.find (key);

      if (it == m_packetBurstMap.end ())
        {
          it = m_packetBurstMap.insert (std::make_pair (key, CreateObject<PacketBurst> ())).first;
        }
      it->second->AddPacket (p);
      NS_LOG_INFO ("Adding a packet for the Packet Burst of " << tag.GetSfn () <<
                   " at sym " << +tag.GetSymStart () << std::endl);
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
MmWavePhy::GetPacketBurst (SfnSf sfn, uint8_t sym)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (sfn.GetNumerology () == GetNumerology());
  Ptr<PacketBurst> pburst;
  auto it = m_packetBurstMap.find (sfn.GetEncodingWithSymStart (sym));

  if (it == m_packetBurstMap.end ())
    {
      // Changed to a fatal error; if it happens, the MAC has scheduled something
      // that we, in reality, don't have. It is a symptom that something is
      // going not so well...
      NS_FATAL_ERROR ("Packet burst not found for " << sfn << " at sym " << +sym);
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

  return MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity  (m_txPower, rbIndexVector, sm, GetChannelBandwidth ());
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

bool
MmWavePhy::HasDlSlot () const
{
  for (const auto & v : m_tddPattern)
    {
      if (v == LteNrTddSlotType::F || v == LteNrTddSlotType::DL)
        {
          return true;
        }
    }
  return false;
}

bool MmWavePhy::HasUlSlot () const
{
  for (const auto & v : m_tddPattern)
    {
      if (v == LteNrTddSlotType::F || v == LteNrTddSlotType::UL)
        {
          return true;
        }
    }
  return false;
}

void
MmWavePhy::UpdateRbNum ()
{
  NS_LOG_FUNCTION (this);

  m_rbNum = static_cast<uint32_t> (GetChannelBandwidth () / (m_subcarrierSpacing * GetNumScsPerRb ()));

  NS_ASSERT (m_rbNum > 0);

  NS_LOG_INFO ("Updated RbNum to " << m_rbNum);

  if (m_spectrumPhy)
    {
      // Update the noisePowerSpectralDensity, as it depends on m_rbNum
      m_spectrumPhy->SetNoisePowerSpectralDensity (GetNoisePowerSpectralDensity());
      NS_LOG_INFO ("Noise Power Spectral Density updated");
    }
  else
    {
      NS_LOG_INFO ("Noise Power Spectral Density NOT updated");
    }
}

bool
MmWavePhy::IsTdd (const std::vector<LteNrTddSlotType> &pattern)
{
  bool anUl = false;
  bool aDl = false;

  for (const auto & v : pattern)
    {
      // An F slot: we are TDD
      if (v == LteNrTddSlotType::F)
        {
          return true;
        }

      if (v == LteNrTddSlotType::UL)
        {
          anUl = true;
        }
      else if (v == LteNrTddSlotType::DL)
        {
          aDl = true;
        }
    }

  return ! (anUl ^ aDl);
}

void
MmWavePhy::InitializeMessageList ()
{
  NS_LOG_FUNCTION (this);
  m_controlMessageQueue.clear ();

  for (unsigned i = 0; i <= GetL1L2CtrlLatency (); i++)
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

void
MmWavePhy::InstallSpectrumPhy (const Ptr<MmWaveSpectrumPhy> &spectrumPhy)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_spectrumPhy != nullptr);
  m_spectrumPhy = spectrumPhy;
  m_spectrumPhy->SetNoisePowerSpectralDensity (GetNoisePowerSpectralDensity());
}

void MmWavePhy::SetBwpId (uint16_t bwpId)
{
  m_bwpId = bwpId;
}

uint16_t
MmWavePhy::GetBwpId () const
{
  return m_bwpId;
}

uint16_t
MmWavePhy::GetCellId () const
{
  return m_cellId;
}

uint32_t
MmWavePhy::GetL1L2CtrlLatency() const
{
  return 2;
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
              Ptr<PacketBurst> pburst = GetPacketBurst (slotSfn, alloc.m_dci->m_symStart);
              if (pburst && pburst->GetNPackets() > 0)
                {
                  newBursts.insert (std::make_pair (currentSfn.GetEncodingWithSymStart (alloc.m_dci->m_symStart), pburst));
                  sfnMap.insert (std::make_pair (currentSfn.GetEncodingWithSymStart (alloc.m_dci->m_symStart),
                                                 it->m_sfnSf.GetEncodingWithSymStart (alloc.m_dci->m_symStart)));
                }
              else
                {
                  NS_LOG_INFO ("No packet burst found for " << slotSfn);
                }
            }
        }

      NS_LOG_INFO ("Set slot allocation for " << it->m_sfnSf << " to " << currentSfn);
      it->m_sfnSf = currentSfn;
      currentSfn.Add (1);
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
  NS_ASSERT (retVal.GetNumerology () == GetNumerology ());
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
  m_slotAllocInfo.erase (m_slotAllocInfo.begin ());
  return ret;
}


SlotAllocInfo
MmWavePhy::RetrieveSlotAllocInfo (const SfnSf &sfnsf)
{
  NS_LOG_FUNCTION (" slot " << sfnsf);
  NS_ASSERT (sfnsf.GetNumerology () == GetNumerology ());

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
  NS_ASSERT (sfnsf.GetNumerology () == GetNumerology ());
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
  NS_ABORT_MSG_IF (m_subcarrierSpacing < 0.0, "Set a valid numerology");

  return MmWaveSpectrumValueHelper::GetSpectrumModel (GetRbNum (),
                                                      GetCentralFrequency (),
                                                      GetNumScsPerRb (),
                                                      m_subcarrierSpacing);
}

Time
MmWavePhy::GetSymbolPeriod () const
{
  NS_LOG_FUNCTION (this);
  return m_symbolPeriod;
}

uint32_t
MmWavePhy::GetRbNum () const
{
  NS_LOG_FUNCTION (this);
  return m_rbNum;
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
