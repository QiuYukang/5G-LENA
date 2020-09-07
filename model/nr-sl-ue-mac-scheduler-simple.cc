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

#include "nr-sl-ue-mac-scheduler-simple.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacSchedulerSimple");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacSchedulerSimple);

NrSlUeMacSchedulerSimple::NrSlUeMacSchedulerSimple ()
{
}

TypeId
NrSlUeMacSchedulerSimple::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacSchedulerSimple")
    .SetParent<NrSlUeMacSchedulerNs3> ()
    .AddConstructor<NrSlUeMacSchedulerSimple> ()
    .SetGroupName ("nr")
  ;
  return tid;
}

bool
NrSlUeMacSchedulerSimple::DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps,
                                            const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                                            NrSlUeMacSchedSapUser::NrSlSlotAlloc &slotAlloc)
{
  NS_LOG_FUNCTION (this);
  bool allocated = false;
  std::set <uint16_t> randTxOpps = RandomlySelectSlots (txOpps);
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>::const_iterator txOppsIt = txOpps.begin ();
  std::advance (txOppsIt, *(randTxOpps.begin ()));

  const auto & lcgMap = dstInfo->GetNrSlLCG (); //Map of unique_ptr should not copy

  NS_ASSERT_MSG (lcgMap.size () == 1, "NrSlUeMacSchedulerSimple can handle only one LCG");

  std::vector<uint8_t> lcVector = lcgMap.begin ()->second->GetLCId ();
  NS_ASSERT_MSG (lcVector.size () == 1, "NrSlUeMacSchedulerSimple can handle only one LC");

  uint32_t bufferSize = lcgMap.begin ()->second->GetTotalSizeOfLC (lcVector.at (0));

  if (bufferSize == 0)
    {
      return allocated;
    }

  uint32_t tbs = 0;
  uint8_t assignedSbCh = 0;
  uint16_t availableSymbols = txOppsIt->slPsschSymLength;
  NS_LOG_DEBUG ("Total available symbols for PSSCH = " << availableSymbols);
  do
    {
      assignedSbCh++;
      tbs = GetNrSlAmc ()->CalculateTbSize (dstInfo->GetDstMcs (), txOppsIt->slSubchannelSize * assignedSbCh * availableSymbols);
    }
  while (tbs < bufferSize && (GetTotalSubCh () - assignedSbCh) > 0);

  tbs = tbs - 8 /*(8 bytes overhead of SCI stage 2)*/;

  allocated = true;
  NrSlUeMacSchedSapUser::SlRlcPduInfo slRlcPduInfo (lcVector.at (0), tbs);
  slotAlloc.slRlcPduInfo.push_back (slRlcPduInfo);
  slotAlloc.ndi = 1;
  slotAlloc.rv = 0;
  slotAlloc.indexSubchannelStart = 0;
  slotAlloc.subchannelLength = assignedSbCh;
  slotAlloc.indexSymStart = txOppsIt->slPsschSymStart;
  slotAlloc.SymLength = availableSymbols;

  slotAlloc.sfn = txOppsIt->sfn;
  slotAlloc.dstL2Id = dstInfo->GetDstL2Id ();
  slotAlloc.mcs = dstInfo->GetDstMcs ();
  slotAlloc.maxNumPerReserve = txOppsIt->slMaxNumPerReserve;
  uint16_t gapReTx1 = randTxOpps.size () > 1 ? *(std::next (randTxOpps.begin (), 1)) - *randTxOpps.begin () : 0;
  slotAlloc.gapReTx1 = static_cast <uint8_t> (gapReTx1);
  uint16_t gapReTx2 = randTxOpps.size () > 2 ? *(std::next (randTxOpps.begin (), 2)) - *randTxOpps.begin () : 0;
  slotAlloc.gapReTx2 = static_cast <uint8_t> (gapReTx2);
  slotAlloc.priority = lcgMap.begin ()->second->GetLcPriority (lcVector.at (0));

  lcgMap.begin ()->second->AssignedData (lcVector.at (0), tbs);
  return allocated;
}


std::set <uint16_t>
NrSlUeMacSchedulerSimple::RandomlySelectSlots (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps)
{
  NS_LOG_FUNCTION (this);

  std::set <uint16_t> randIndex;

  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>::const_iterator txOppsIt = txOpps.begin ();

  NS_ASSERT_MSG (txOpps.size () >= txOppsIt->slMaxNumPerReserve,
                 "not enough txOpps to perform " << txOppsIt->slMaxNumPerReserve << " transmissions");

  NS_ASSERT_MSG (txOppsIt->slMaxNumPerReserve >= 1 && txOppsIt->slMaxNumPerReserve < 4,
                 "slMaxNumPerReserve should be greater than 1 and less than 4");

  uint16_t totalReTx = txOppsIt->slMaxNumPerReserve - 1;
  uint16_t txOppSize = static_cast <uint16_t> (txOpps.size ());
  uint16_t reTxWindSize = static_cast <uint16_t> (GetNrSlReTxWindow ());

  uint16_t firstTxSlot = m_uniformVariable->GetInteger (1, (txOppSize - totalReTx));

  uint32_t firstTxIndex = firstTxSlot - 1; //adjust since list index start from 0

  randIndex.insert (firstTxIndex);

  if (txOppsIt->slMaxNumPerReserve == 1)
    {
      return randIndex;
    }

  uint16_t remainingTxSlots = txOppSize - firstTxSlot;

  uint16_t finalRetxWind = std::min (reTxWindSize, remainingTxSlots);

  uint16_t lastSlotForRetxOne = (finalRetxWind - totalReTx) + 1 + firstTxSlot;

  uint16_t reTxOneSlot = m_uniformVariable->GetInteger ((firstTxSlot + 1), lastSlotForRetxOne);

  uint16_t reTxOneIndex = reTxOneSlot - 1; //adjust since list index start from 0

  randIndex.insert (reTxOneIndex);

  if (txOppsIt->slMaxNumPerReserve == 2)
    {
      return randIndex;
    }

  uint16_t lastSlotForRetxTwo = firstTxSlot + finalRetxWind;

  uint16_t reTxTwoSlot = m_uniformVariable->GetInteger ((reTxOneSlot + 1), lastSlotForRetxTwo);

  uint16_t reTxTwoIndex = reTxTwoSlot - 1; //adjust since list index start from 0

  randIndex.insert (reTxTwoIndex);
  return randIndex;
}



} //namespace ns3
