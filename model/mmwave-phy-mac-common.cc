/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
*                           Sourjya Dutta <sdutta@nyu.edu>
*                           Russell Ford <russell.ford@nyu.edu>
*                          Menglei Zhang <menglei@nyu.edu>
*                          Biljana Bojovic <biljana.bojovic@cttc.es>
*/


#include "mmwave-phy-mac-common.h"
#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/string.h>
#include <ns3/attribute-accessor-helper.h>
#include <algorithm>
#include "mmwave-mac-scheduler-tdma-rr.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWavePhyMacCommon");

NS_OBJECT_ENSURE_REGISTERED (MmWavePhyMacCommon);

TypeId
MmWavePhyMacCommon::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWavePhyMacCommon")
    .SetParent<Object> ()
    .AddConstructor<MmWavePhyMacCommon> ()
    .AddAttribute ("CtrlSymbols",
                   "Number of OFDM symbols for DL control per subframe",
                   UintegerValue (1),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_ctrlSymbols),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NumReferenceSymbols",
                   "Number of reference symbols per slot",
                   UintegerValue (6),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_numRefSymbols),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CenterFreq",
                   "The center frequency in Hz",
                   DoubleValue (28e9),
                   MakeDoubleAccessor (&MmWavePhyMacCommon::m_centerFrequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Bandwidth",
                   "The system bandwidth in Hz",
                   DoubleValue (400e6),
                   MakeDoubleAccessor (&MmWavePhyMacCommon::SetBandwidth,
                                       &MmWavePhyMacCommon::GetBandwidth),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("UlSchedDelay",
                   "Number of TTIs between UL scheduling decision and subframe to which it applies",
                   UintegerValue (2),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_ulSchedDelay),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NumRbPerRbg",
                   "Number of resource blocks per resource block group",
                   UintegerValue (1),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_numRbPerRbg),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Numerology",
                   "The 3gpp numerology to be used",
                   UintegerValue (4),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::SetNumerology,
                                         &MmWavePhyMacCommon::GetNumerology),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("NumHarqProcess",
                   "Number of concurrent stop-and-wait Hybrid ARQ processes per user",
                   UintegerValue (20),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_numHarqProcess),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("SymbolsPerSlot",
                   "Number of symbols in one slot, including 2 of control",
                   UintegerValue (14),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_symbolsPerSlot),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("HarqDlTimeout",
                   "Harq dl timeout",
                   UintegerValue (20),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_harqTimeout),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("TbDecodeLatency",
                   "TB decode latency",
                   UintegerValue (100.0),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_tbDecodeLatencyUs),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("L1L2CtrlLatency",
                   "L1L2 CTRL decode latency in slot",
                   UintegerValue (2),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_l1L2CtrlLatency),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("L1L2DataLatency",
                   "L1L2 Data decode latency in slot",
                   UintegerValue (2),
                   MakeUintegerAccessor (&MmWavePhyMacCommon::m_l1L2CtrlLatency),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MacSchedulerType",
                   "The type of scheduler to be used for the MAC. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::MmWaveMacScheduler.",
                   TypeIdValue (MmWaveMacSchedulerTdmaRR::GetTypeId ()),
                   MakeTypeIdAccessor (&MmWavePhyMacCommon::m_macSchedType),
                   MakeTypeIdChecker ())
    .AddAttribute ("ComponentCarrierId",
                   "Component carrier ID",
                    UintegerValue (0),
                    MakeUintegerAccessor (&MmWavePhyMacCommon::m_componentCarrierId),
                    MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

MmWavePhyMacCommon::MmWavePhyMacCommon ()
  : m_symbolPeriod (0.00000416),
  m_symbolsPerSlot (14),
  m_slotPeriod (0.0001),
  m_ctrlSymbols (1),
  m_dlCtrlSymbols (1),
  m_ulCtrlSymbols (1),
  m_fixedTtisPerSlot (8),
  m_subframesPerFrame (10),
  m_numRefSymbols (6),
  m_numRbPerRbg (1),
  m_numerology (4),
  m_subcarrierSpacing (14e6),
  m_rbNum (72),
  m_numRefScPerRb (3),
  m_numSubCarriersPerRb (12),
  m_numHarqProcess (20),
  m_harqTimeout (20),
  m_centerFrequency (28e9),
  m_bandwidth (400e6),
  m_bandwidthConfigured (false),
  m_l1L2CtrlLatency (2),
  m_l1L2DataLatency (2),
  m_ulSchedDelay (1),
  m_tbDecodeLatencyUs (100.0),
  m_maxTbSizeBytes (0x7FFF),
  m_componentCarrierId (0)
{
  NS_LOG_INFO ("MmWavePhyMacCommon constructor");
}


void
MmWavePhyMacCommon::DoInitialize (void)
{
  NS_LOG_INFO ("Initialized MmWavePhyMacCommon");
}

void
MmWavePhyMacCommon::DoDispose (void)
{
}

MmWavePhyMacCommon::~MmWavePhyMacCommon (void)
{
}

Time MmWavePhyMacCommon::GetSymbolPeriod(void) const
{
  return m_symbolPeriod;
}

uint32_t
MmWavePhyMacCommon::GetCtrlSymbols (void) const
{
  return m_ctrlSymbols;
}

uint8_t
MmWavePhyMacCommon::GetDlCtrlSymbols (void) const
{
  return m_dlCtrlSymbols;
}

uint8_t MmWavePhyMacCommon::GetUlCtrlSymbols (void) const
{
  return m_ulCtrlSymbols;
}

uint8_t
MmWavePhyMacCommon::GetSymbolsPerSlot (void) const
{
  return m_symbolsPerSlot;
}

Time
MmWavePhyMacCommon::GetSlotPeriod (void) const
{
  return m_slotPeriod;
}

uint32_t
MmWavePhyMacCommon::GetVarTtisPerSlot (void) const
{
  return m_fixedTtisPerSlot;
}

uint32_t
MmWavePhyMacCommon::GetSubframesPerFrame (void) const
{
  return m_subframesPerFrame;
}

uint32_t
MmWavePhyMacCommon::GetSlotsPerSubframe (void) const
{
  return m_slotsPerSubframe;
}

uint32_t
MmWavePhyMacCommon::GetNumReferenceSymbols (void)
{
  return m_numRefSymbols;
}

uint8_t
MmWavePhyMacCommon::GetUlSchedDelay (void) const
{
  return m_ulSchedDelay;
}

uint32_t
MmWavePhyMacCommon::GetNumScsPerRb (void) const
{
  return m_numSubCarriersPerRb;
}

double
MmWavePhyMacCommon::GetSubcarrierSpacing (void) const
{
  return m_subcarrierSpacing;
}

uint32_t
MmWavePhyMacCommon::GetNumRefScPerRb (void) const
{
  return m_numRefScPerRb;
}

// for TDMA, number of reference subcarriers across entire bandwidth (default to 1/4th of SCs)
uint32_t
MmWavePhyMacCommon::GetNumRefScPerSym (void) const
{
  return m_numSubCarriersPerRb * m_rbNum  / 4;
}

uint32_t
MmWavePhyMacCommon::GetNumRbPerRbg (void) const
{
  return m_numRbPerRbg;
}

uint32_t
MmWavePhyMacCommon::GetNumerology (void) const
{
  return m_numerology;
}

double
MmWavePhyMacCommon::GetBandwidth (void) const
{
  return (GetSubcarrierSpacing () * GetNumScsPerRb () * m_rbNum);
}

uint32_t
MmWavePhyMacCommon::GetBandwidthInRbg () const
{
  return m_rbNum / m_numRbPerRbg;
}

/*
 * brief: bandwidth in number of RBs
 */
uint32_t
MmWavePhyMacCommon::GetBandwidthInRbs () const
{
  return m_rbNum;
}


double
MmWavePhyMacCommon::GetCenterFrequency (void) const
{
  return m_centerFrequency;
}

uint16_t
MmWavePhyMacCommon::GetL1L2CtrlLatency (void) const
{
  return m_l1L2CtrlLatency;
}

uint32_t
MmWavePhyMacCommon::GetL1L2DataLatency (void) const
{
  return m_l1L2DataLatency;
}

uint32_t
MmWavePhyMacCommon::GetNumHarqProcess (void) const
{
  return m_numHarqProcess;
}

uint8_t
MmWavePhyMacCommon::GetHarqTimeout (void) const
{
  return m_harqTimeout;
}

uint32_t
MmWavePhyMacCommon::GetTbDecodeLatency (void) const
{
  return m_tbDecodeLatencyUs;
}

uint32_t
MmWavePhyMacCommon::GetMaxTbSize (void) const
{
  return m_maxTbSizeBytes;
}

void
MmWavePhyMacCommon::SetSymbolPeriod (double prdSym)
{
  m_symbolPeriod = Seconds (prdSym);
}

void
MmWavePhyMacCommon::SetSymbolsPerSlot (uint8_t numSym)
{
  m_symbolsPerSlot = numSym;
}

void
MmWavePhyMacCommon::SetSlotPeriod (double period)
{
  m_slotPeriod = Seconds (period);
}

void
MmWavePhyMacCommon::SetCtrlSymbols (uint32_t ctrlSymbols)
{
  m_ctrlSymbols = ctrlSymbols;
}

void
MmWavePhyMacCommon::SetDlCtrlSymbols (uint8_t ctrlSymbols)
{
  m_dlCtrlSymbols = ctrlSymbols;
}

void
MmWavePhyMacCommon::SetUlCtrlSymbols (uint8_t ctrlSymbols)
{
  m_ulCtrlSymbols = ctrlSymbols;
}

void
MmWavePhyMacCommon::SetVarTtiPerSlot (uint32_t numVarTti)
{
  m_fixedTtisPerSlot = numVarTti;
}

void
MmWavePhyMacCommon::SetSubframePerFrame (uint32_t numSf)
{
  m_subframesPerFrame = numSf;
}

void
MmWavePhyMacCommon::SetNumReferenceSymbols (uint32_t refSym)
{
  m_numRefSymbols = refSym;
}

void
MmWavePhyMacCommon::SetUlSchedDelay (uint32_t tti)
{
  m_ulSchedDelay = tti;
}

void
MmWavePhyMacCommon::SetNumScsPrRb (uint32_t numScs)
{
  m_numSubCarriersPerRb = numScs;
}

void
MmWavePhyMacCommon::SetNumRefScPerRb (uint32_t numRefSc)
{
  m_numRefScPerRb = numRefSc;
}

void
MmWavePhyMacCommon::SetRbNum (uint32_t numRB)
{
  m_rbNum = numRB;
}

/*
 * brief
 * rbgSize size of RBG in number of resource blocks
 */
void
MmWavePhyMacCommon::SetNumRbPerRbg (uint32_t rbgSize)
{
  m_numRbPerRbg = rbgSize;
}

void
MmWavePhyMacCommon::SetNumerology (uint32_t numerology)
{
  NS_ASSERT_MSG ( (0 <= numerology) && (numerology <= 5), "Numerology not defined.");

  m_numerology = numerology;
  m_slotsPerSubframe  = std::pow (2, numerology);
  m_slotPeriod = Seconds (0.001 / m_slotsPerSubframe);
  m_symbolPeriod = (m_slotPeriod / m_symbolsPerSlot);
  m_numSubCarriersPerRb = 12;
  m_subcarrierSpacing = 15 * std::pow (2, numerology) * 1000;

  NS_ASSERT_MSG (m_bandwidthConfigured, "Bandwidth not configured, bandwidth has to be configured in order to configure properly the numerology");

  m_rbNum = m_bandwidth / (m_subcarrierSpacing * m_numSubCarriersPerRb);

  NS_LOG_INFO (" Numerology configured:" << m_numerology <<
               " slots per subframe: " << m_slotsPerSubframe <<
               " slot period:" << m_slotPeriod <<
               " symbol period:" << m_symbolPeriod <<
               " subcarrier spacing: " << m_subcarrierSpacing <<
               " number of RBs: " << m_rbNum );
}

/*
 * brief Set bandwidth value in Hz
 * param bandwidth the bandwidth value in Hz
 */
void
MmWavePhyMacCommon::SetBandwidth (double bandwidth)
{
  m_bandwidth = bandwidth;
  m_bandwidthConfigured = true;
}

void
MmWavePhyMacCommon::SetCentreFrequency (double fc)
{
  m_centerFrequency = fc;
}

void
MmWavePhyMacCommon::SetL1L2CtrlLatency (uint32_t delaySfs)
{
  m_l1L2CtrlLatency = delaySfs;
}

void
MmWavePhyMacCommon::SetL1L2DataLatency (uint32_t delayVarTtis)
{
  m_l1L2DataLatency = delayVarTtis;
}

void
MmWavePhyMacCommon::SetNumHarqProcess (uint32_t numProcess)
{
  m_numHarqProcess = numProcess;
}

void
MmWavePhyMacCommon::SetHarqDlTimeout (uint8_t harqDlTimeout)
{
  m_harqTimeout = harqDlTimeout;
}

void
MmWavePhyMacCommon::SetTbDecodeLatency (uint32_t us)
{
  m_tbDecodeLatencyUs = us;
}

void
MmWavePhyMacCommon::SetMaxTbSize (uint32_t bytes)
{
  m_maxTbSizeBytes = bytes;
}

void
MmWavePhyMacCommon::SetCcId (uint8_t ccId)
{
  m_componentCarrierId = ccId;
}

uint8_t
MmWavePhyMacCommon::GetCcId (void)
{
  return m_componentCarrierId;
}

void
SlotAllocInfo::Merge (const SlotAllocInfo &other)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (other.m_sfnSf == m_sfnSf);

  m_numSymAlloc += other.m_numSymAlloc;

  for (auto extAlloc : other.m_varTtiAllocInfo)
    {
      m_varTtiAllocInfo.push_front (extAlloc);
    }

  // Sort over the symStart of the DCI (VarTtiAllocInfo::operator <)
  std::sort (m_varTtiAllocInfo.begin (), m_varTtiAllocInfo.end ());
}

std::ostream & operator<< (std::ostream & os, VarTtiAllocInfo::TddMode const & item)
{
  if (item == VarTtiAllocInfo::DL)
    {
      os << "DL";
    }
  else if (item == VarTtiAllocInfo::UL)
    {
      os << "UL";
    }
  else
    {
      os << "NA";
    }
  return os;
}

std::ostream &operator<< (std::ostream &os, const DlHarqInfo &item)
{
  if (item.IsReceivedOk ())
    {
      os << "ACK feedback ";
    }
  else
    {
      os << "NACK feedback ";
    }
  os << "for ProcessID: " << static_cast<uint32_t> (item.m_harqProcessId) << " of UE "
     << static_cast<uint32_t> (item.m_rnti) << " Num Retx: " << static_cast<uint32_t> (item.m_numRetx);
  return os;
}

std::ostream &operator<< (std::ostream &os, const UlHarqInfo &item)
{
  if (item.IsReceivedOk ())
    {
      os << "ACK feedback ";
    }
  else
    {
      os << "NACK feedback ";
    }
  os << "for ProcessID: " << static_cast<uint32_t> (item.m_harqProcessId) << " of UE "
     << static_cast<uint32_t> (item.m_rnti) << " Num Retx: " << static_cast<uint32_t> (item.m_numRetx);

  return os;
}

std::ostream & operator<< (std::ostream & os, SfnSf const & item)
{
  os << "FrameNum: " << static_cast<uint32_t> (item.m_frameNum) <<
    " SubFrameNum: " << static_cast<uint32_t> (item.m_subframeNum) <<
    " SlotNum: " << static_cast<uint32_t> (item.m_slotNum) <<
    " VarTtiNum: " << static_cast<uint32_t> (item.m_varTtiNum);
  return os;
}

}
