/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nr-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcErrorModelComparisonExample");

int
main (int argc, char *argv[])
{
  uint32_t mcs = 13;

  std::string errorModel = "ns3::NrEesmErrorModel";
  uint32_t eesmTable = 1;

  CommandLine cmd;

  cmd.AddValue ("mcs",
                "The MCS that will be used in this example",
                mcs);
  cmd.AddValue("errorModelType",
               "Error model type: ns3::NrEesmErrorModel , ns3::NrLteErrorModel",
               errorModel);
  cmd.AddValue("eesmTable",
               "Table to use when error model is Eesm (1 for McsTable1 or 2 for McsTable2)",
               eesmTable);

  cmd.Parse (argc, argv);

  if (eesmTable == 1)
    {
      Config::SetDefault("ns3::NrEesmErrorModel::McsTable", EnumValue (NrEesmErrorModel::McsTable1));
    }
  else if (eesmTable == 2)
    {
      Config::SetDefault("ns3::NrEesmErrorModel::McsTable", EnumValue (NrEesmErrorModel::McsTable2));
    }
  else
    {
      NS_FATAL_ERROR ("Valid tables are 1 or 2, you set " << eesmTable);
    }

  Config::SetDefault("ns3::NrAmc::ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));
  Config::SetDefault("ns3::NrAmc::AmcModel", EnumValue (NrAmc::PiroEW2010));


  Ptr<MmWavePhyMacCommon> config = CreateObject<MmWavePhyMacCommon> ();
  config->SetNumerology (4);
  config->SetBandwidth(100e6);
  config->SetCentreFrequency(28e9);

  Ptr<NrAmc> amc = CreateObject<NrAmc> (config);

  std::string tbs;

  for (uint32_t mcs = 0; mcs <= amc->GetMaxMcs (); ++mcs)
    {
      std::stringstream ss;
      ss << "\nMCS " << mcs <<
            " TBS in 1 RBG: [" << amc->CalculateTbSize(mcs, config->GetNumRbPerRbg ()) <<
            "] TBS in 1 sym: [" << amc->CalculateTbSize(mcs, config->GetNumRbPerRbg() * config->GetBandwidthInRbg ()) <<
            "]";
      tbs += ss.str ();
    }

  std::cout << "NUMEROLOGY 4, 100e6 BANDWIDTH, 28e9 CENTRE FREQ, Error Model: ";
  std::cout << errorModel << "Table (if apply:) " << eesmTable << ". Results: " << std::endl;
  std::cout << tbs << std::endl;

  return 0;
}


