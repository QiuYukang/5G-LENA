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
*/

#include "lena-error-model.h"
#include <ns3/log.h>
#include <ns3/lte-amc.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LenaErrorModel");
NS_OBJECT_ENSURE_REGISTERED (LenaErrorModel);

LenaErrorModel::LenaErrorModel () : NrLteMiErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

LenaErrorModel::~LenaErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t
LenaErrorModel::GetPayloadSize (uint32_t usefulSC, uint8_t mcs, uint32_t rbNum, NrErrorModel::Mode mode) const
{
  NS_LOG_FUNCTION (this);

  NS_UNUSED (usefulSC);
  NS_ASSERT (rbNum >= 13);
  static LteAmc lenaAmc;

  NS_LOG_DEBUG ("Asking LENA AMC to retrieve the TBS for MCS " << +mcs << " and RB " << rbNum / 13);

  if (mode == NrErrorModel::DL)
    {
      return (lenaAmc.GetDlTbSizeFromMcs (mcs, rbNum / 13) / 8);
    }
  else
    {
      return (lenaAmc.GetUlTbSizeFromMcs (mcs, rbNum / 13) / 8);
    }
}

TypeId
LenaErrorModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LenaErrorModel")
    .SetParent<NrLteMiErrorModel> ()
    .AddConstructor<LenaErrorModel> ()
  ;
  return tid;
}

TypeId
LenaErrorModel::GetInstanceTypeId() const
{
  return LenaErrorModel::GetTypeId ();
}

}
