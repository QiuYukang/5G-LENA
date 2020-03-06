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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/mmwave-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nr-module.h"

/**
 * \file cttc-error-model-comparison.cc
 * \ingroup examples
 * \brief Error model example comparison: TBS for all MCSs.
 *
 * This example allows the user to compare the Transport Block Size that is obtained
 * for each MCS index under different error models (NR and LTE) and different MCS Tables.
 * It allows the user to configure the MCS Table and the error model.
 *
 * For example, the MCS table can be toggled by the argument,
 * e.g. "--eesmTable=1" for MCS Table1.
 * The NR error model can be set as "--errorModel=ns3::NrEesmErrorModel",
 * while "--errorModel=ns3::NrLteMiErrorModel" configures the LTE error model.
 *
 * There is no deployment scenario configured, the example directly computes the TBS
 * for all MCSs of the configured error model and MCS Table, assuming numerology 4,
 * 100 MHz of channel bandwidth, and 28 GHz of central channel frequency.
 *
 * This simulation prints the output to the terminal, showing for each MCS
 * the TBS that fits in 1 OFDM symbol (whole bandwidth) and the TBS that fits
 * in 1 OFDM symbol and a single RB.
 *
 * To run the simulation with the default configuration one shall run the
 * following in the command line:
 *
 * ./waf --run cttc-error-model-comparison
 *
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcErrorModelComparisonExample");

int
main (int argc, char *argv[])
{
  std::string errorModel = "ns3::NrEesmErrorModel";
  uint32_t eesmTable = 1;

  CommandLine cmd;

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
  Config::SetDefault("ns3::NrAmc::AmcModel", EnumValue (NrAmc::ShannonModel));


  Ptr<MmWavePhyMacCommon> config = CreateObject<MmWavePhyMacCommon> ();
  config->SetNumerology (4);
  config->SetBandwidth(100e6);

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


