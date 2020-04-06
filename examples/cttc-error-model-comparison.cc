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
 * for all MCSs of the configured error model and MCS Table, assuming numerology 4
 * and 100 MHz of channel bandwidth.
 *
 * This simulation prints the output to the terminal, showing for each MCS: 1)
 * the TBS that fits in 1 OFDM symbol (whole bandwidth) and 2) the TBS that fits
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
  std::string errorModel = "ns3::NrEesmCcT1";

  CommandLine cmd;

  cmd.AddValue("errorModelType",
               "Error model type: ns3::NrEesmCcT1, ns3::NrEesmCcT2, ns3::NrEesmIrT1, ns3::NrEesmIrT2, ns3::NrLteMiErrorModel",
               errorModel);

  cmd.Parse (argc, argv);

  /*
   * TODO: remove all the instances of SetDefault, NrEesmErrorModel, NrAmc
   */
  Config::SetDefault("ns3::NrAmc::ErrorModelType", TypeIdValue (TypeId::LookupByName(errorModel)));
  Config::SetDefault("ns3::NrAmc::AmcModel", EnumValue (NrAmc::ShannonModel));

  // Compute number of RBs that fit in 100 MHz channel bandwidth with numerology 4 (240 kHz SCS)
  const uint8_t numerology = 4;
  const uint32_t bandwidth = 100e6;
  const uint32_t numRbsInBandwidth = bandwidth / (15e3 * std::pow(2,numerology) * 12) ;

  Ptr<NrAmc> amc = CreateObject<NrAmc> ();

  std::string tbs;
  for (uint32_t mcs = 0; mcs <= amc->GetMaxMcs (); ++mcs)
    {
      std::stringstream ss;
      ss << "\nMCS " << mcs <<
            ". TBS in 1 RB: [" << amc->CalculateTbSize(mcs, 1) <<
            "] bytes. TBS in 1 sym: [" << amc->CalculateTbSize(mcs, numRbsInBandwidth) <<
            "] bytes.";
      tbs += ss.str ();
    }

  std::cout << "NUMEROLOGY 4, 100e6 BANDWIDTH, Error Model: ";
  std::cout << errorModel << ", MCS Table (if applies:) " << eesmTable <<
               ". Results: " << std::endl;
  std::cout << tbs << std::endl;

  return 0;
}


