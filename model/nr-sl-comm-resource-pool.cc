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

#include "nr-sl-comm-resource-pool.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlCommResourcePool");

NrSlCommResourcePool::NrSlCommResourcePool ()
{
  NS_LOG_FUNCTION (this);
}
NrSlCommResourcePool::~NrSlCommResourcePool ()
{
  NS_LOG_FUNCTION (this);
}

void
NrSlCommResourcePool::SetSlPreConfigFreqInfoList (const std::array <LteRrcSap::SlFreqConfigCommonNr, MAX_NUM_OF_FREQ_SL> &slPreconfigFreqInfoList)
{
  NS_LOG_FUNCTION (this);
  m_slPreconfigFreqInfoList = slPreconfigFreqInfoList;
}

void
NrSlCommResourcePool::SetPhysicalSlPoolMap (NrSlCommResourcePool::PhySlPoolMap phySlPoolMap)
{
  NS_LOG_FUNCTION (this);
  m_phySlPoolMap = phySlPoolMap;
}

const std::vector <std::bitset<1>>
NrSlCommResourcePool::GetPhySlPool (uint16_t bwpId, uint16_t poolId)
{
  NS_LOG_FUNCTION (this);
  NrSlCommResourcePool::PhySlPoolMap::iterator itBwp = m_phySlPoolMap.find (bwpId);
  NS_ABORT_MSG_IF (itBwp == m_phySlPoolMap.end (), "Unable to find BWP id " << bwpId);
  std::unordered_map <uint16_t, std::vector <std::bitset<1>>>::iterator itPool = itBwp->second.find (poolId);
  NS_ABORT_MSG_IF (itPool == itBwp->second.end (), "Unable to find pool id " << poolId);
  return itPool->second;
}

const LteRrcSap::SlResourcePoolNr
NrSlCommResourcePool::GetSlResourcePoolNr (uint16_t bwpId, uint16_t poolId)
{
  NS_LOG_FUNCTION (this);
  LteRrcSap::SlFreqConfigCommonNr slfreqConfigCommon = m_slPreconfigFreqInfoList.at (0);
  LteRrcSap::SlBwpConfigCommonNr slBwpConfigCommon = slfreqConfigCommon.slBwpList.at (bwpId);
  std::array <LteRrcSap::SlResourcePoolConfigNr, MAX_NUM_OF_TX_POOL> slTxPoolSelectedNormal = slBwpConfigCommon.slBwpPoolConfigCommonNr.slTxPoolSelectedNormal;
  LteRrcSap::SlResourcePoolNr pool;
  bool found = false;
  for (const auto& it: slTxPoolSelectedNormal)
    {
      if (it.slResourcePoolId.id == poolId)
        {
          found = true;
          pool = it.slResourcePool;
          break;
        }
    }
  NS_ASSERT_MSG (found == true, "unable to find pool id " << poolId);
  return pool;
}

std::unordered_map <uint16_t, std::vector <std::bitset<1>>>::const_iterator
NrSlCommResourcePool::GetIteratorToPhySlPool (uint16_t bwpId, uint16_t poolId)
{
  NS_LOG_FUNCTION (this);
  NrSlCommResourcePool::PhySlPoolMap::iterator itBwp = m_phySlPoolMap.find (bwpId);
  NS_ABORT_MSG_IF (itBwp == m_phySlPoolMap.end (), "Unable to find bandwidth part id " << bwpId);
  std::unordered_map <uint16_t, std::vector <std::bitset<1>>>::const_iterator itPool = itBwp->second.find (poolId);
  NS_ABORT_MSG_IF (itPool == itBwp->second.end (), "Unable to find pool id " << poolId);
  return itPool;
}

std::list <NrSlCommResourcePool::SlotInfo>
NrSlCommResourcePool::GetSlCommOpportunities (uint16_t absIndexCurretSlot, uint16_t bwpId, uint16_t poolId, uint16_t t1)
{
  NS_LOG_FUNCTION (this);
  std::unordered_map <uint16_t, std::vector <std::bitset<1>>>::const_iterator itPhyPool;
  itPhyPool = GetIteratorToPhySlPool (bwpId, poolId);
  LteRrcSap::SlFreqConfigCommonNr slfreqConfigCommon = m_slPreconfigFreqInfoList.at (0);
  LteRrcSap::SlBwpConfigCommonNr slBwpConfigCommon = slfreqConfigCommon.slBwpList.at (bwpId);
  uint16_t totalSlSymbols = LteRrcSap::GetSlLengthSymbolsValue (slBwpConfigCommon.slBwpGeneric.slLengthSymbols);
  uint16_t slSymbolStart =  LteRrcSap::GetSlStartSymbolValue(slBwpConfigCommon.slBwpGeneric.slStartSymbol);

  const LteRrcSap::SlResourcePoolNr pool = GetSlResourcePoolNr (bwpId, poolId);
  uint16_t t2 = LteRrcSap::GetSlSelWindowValue (pool.slUeSelectedConfigRp.slSelectionWindow);
  uint16_t slotIndex = absIndexCurretSlot + t1;

  std::list <NrSlCommResourcePool::SlotInfo> list;
  uint16_t poolIndex = 0;
  for (uint16_t i = slotIndex; i < t2 + slotIndex; ++i)
    {
      if (itPhyPool->second [poolIndex] == 1)
        {
          NrSlCommResourcePool::SlotInfo info;
          //PSCCH
          info.numSlPscchRbs = LteRrcSap::GetSlFResoPscchValue (pool.slPscchConfig.slFreqResourcePscch);
          info.slPscchSymStart = slSymbolStart;
          info.slPscchSymlength = LteRrcSap::GetSlTResoPscchValue (pool.slPscchConfig.slTimeResourcePscch);
          //PSSCH
          info.slSubchannelSize = LteRrcSap::GetSlSubChSizeValue (pool.slSubchannelSize);
          info.slPsschSymStart = info.slPscchSymStart + info.slPscchSymlength;
          info.slPsschSymlength = (totalSlSymbols - info.slPscchSymlength) - 1;
          info.absSlotIndex = i;
          list.emplace_back (info);
        }
      ++poolIndex;
    }

  return list;
}


}


