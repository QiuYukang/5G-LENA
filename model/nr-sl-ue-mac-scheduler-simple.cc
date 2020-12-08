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
                                            std::set<NrSlSlotAlloc> &slotAllocList)
{
  NS_LOG_FUNCTION (this);
  bool allocated = false;
  NS_ASSERT_MSG (txOpps.size () > 0, "Scheduler received an empty txOpps list from UE MAC");
  const auto & lcgMap = dstInfo->GetNrSlLCG (); //Map of unique_ptr should not copy

  NS_ASSERT_MSG (lcgMap.size () == 1, "NrSlUeMacSchedulerSimple can handle only one LCG");

  std::vector<uint8_t> lcVector = lcgMap.begin ()->second->GetLCId ();
  NS_ASSERT_MSG (lcVector.size () == 1, "NrSlUeMacSchedulerSimple can handle only one LC");

  uint32_t bufferSize = lcgMap.begin ()->second->GetTotalSizeOfLC (lcVector.at (0));

  if (bufferSize == 0)
    {
      return allocated;
    }


  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> selectedTxOpps;
  selectedTxOpps = RandomlySelectSlots (txOpps);
  NS_ASSERT_MSG (selectedTxOpps.size () > 0, "Scheduler should select at least 1 slot from txOpps");
  uint32_t tbs = 0;
  uint8_t assignedSbCh = 0;
  uint16_t availableSymbols = selectedTxOpps.begin ()->slPsschSymLength;
  uint16_t sbChSize = selectedTxOpps.begin ()->slSubchannelSize;
  NS_LOG_DEBUG ("Total available symbols for PSSCH = " << availableSymbols);
  do
    {
      assignedSbCh++;
      tbs = GetNrSlAmc ()->CalculateTbSize (dstInfo->GetDstMcs (), sbChSize * assignedSbCh * availableSymbols);
    }
  while (tbs < bufferSize + 5 /*(5 bytes overhead of SCI format 2A)*/ && (GetTotalSubCh () - assignedSbCh) > 0);

  //Now, before allocating bytes to LCs we subtract 5 bytes for SCI format 2A
  //since we already took it into account while computing the TB size.
  tbs = tbs - 5 /*(5 bytes overhead of SCI stage 2)*/;

  allocated = true;

  for (const auto &itTxOpps:selectedTxOpps)
    {
      NrSlSlotAlloc slotAlloc;
      slotAlloc.sfn = itTxOpps.sfn;
      slotAlloc.dstL2Id = dstInfo->GetDstL2Id ();
      slotAlloc.priority = lcgMap.begin ()->second->GetLcPriority (lcVector.at (0));
      SlRlcPduInfo slRlcPduInfo (lcVector.at (0), tbs);
      slotAlloc.slRlcPduInfo.push_back (slRlcPduInfo);
      slotAlloc.mcs = dstInfo->GetDstMcs ();
      //PSCCH
      slotAlloc.numSlPscchRbs = itTxOpps.numSlPscchRbs;
      slotAlloc.slPscchSymStart = itTxOpps.slPscchSymStart;
      slotAlloc.slPscchSymLength = itTxOpps.slPscchSymLength;
      //PSSCH
      slotAlloc.slPsschSymStart = itTxOpps.slPsschSymStart;
      slotAlloc.slPsschSymLength = availableSymbols;
      slotAlloc.slPsschSubChStart = 0;
      slotAlloc.slPsschSubChLength = assignedSbCh;
      slotAlloc.maxNumPerReserve = itTxOpps.slMaxNumPerReserve;
      slotAlloc.ndi = slotAllocList.empty () == true ? 1 : 0;
      slotAlloc.rv = GetRv (static_cast<uint8_t>(slotAllocList.size ()));
      if (static_cast<uint16_t>(slotAllocList.size ()) % itTxOpps.slMaxNumPerReserve == 0)
        {
          slotAlloc.txSci1A = true;
          if (slotAllocList.size () + itTxOpps.slMaxNumPerReserve <= selectedTxOpps.size ())
            {
              slotAlloc.slotNumInd = itTxOpps.slMaxNumPerReserve;
            }
          else
            {
              slotAlloc.slotNumInd = selectedTxOpps.size () - slotAllocList.size ();
            }
        }
      else
        {
          slotAlloc.txSci1A = false;
          //Slot, which does not carry SCI 1-A can not indicate future TXs
          slotAlloc.slotNumInd = 0;
        }

      slotAllocList.emplace (slotAlloc);
    }

  lcgMap.begin ()->second->AssignedData (lcVector.at (0), tbs);
  return allocated;
}


std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrSlUeMacSchedulerSimple::RandomlySelectSlots (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps)
{
  NS_LOG_FUNCTION (this);

  uint8_t totalTx = GetSlMaxTxTransNumPssch ();
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> newTxOpps;

  if (txOpps.size () > totalTx)
    {
      while (newTxOpps.size () != totalTx)
        {
          auto txOppsIt = txOpps.begin ();
          // Walk through list until the random element is reached
          std::advance (txOppsIt, m_uniformVariable->GetInteger (0, txOpps.size () - 1));
          //copy the randomly selected slot info into the new list
          newTxOpps.emplace_back (*txOppsIt);
          //erase the selected one from the list
          txOppsIt = txOpps.erase (txOppsIt);
        }
    }
  else
    {
      newTxOpps = txOpps;
    }
  //sort the list by SfnSf before returning
  newTxOpps.sort ();
  NS_ASSERT_MSG (newTxOpps.size () <= totalTx, "Number of randomly selected slots exceeded total number of TX");
  return newTxOpps;
}



} //namespace ns3
