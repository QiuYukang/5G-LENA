/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef BWPMANAGERUE_H
#define BWPMANAGERUE_H

#include <ns3/simple-ue-component-carrier-manager.h>

namespace ns3 {

class BwpManagerAlgorithm;

/**
 * \ingroup bwp
 * \brief The BwpManagerUe class
 */
class BwpManagerUe : public SimpleUeComponentCarrierManager
{
public:
  static TypeId GetTypeId ();

  BwpManagerUe ();
  virtual ~BwpManagerUe () override;
protected:
  virtual void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params) override;
  virtual std::vector<LteUeCcmRrcSapProvider::LcsConfig> DoAddLc (uint8_t lcId,  LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu) override;
  virtual LteMacSapUser* DoConfigureSignalBearer (uint8_t lcId,  LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu) override;

private:
  BwpManagerAlgorithm *m_algorithm;
  std::unordered_map<uint8_t, EpsBearer::Qci> m_lcToBearerMap; //!< Map from LCID to bearer ID
};

} // namespace ns3
#endif // BWPMANAGERUE_H
