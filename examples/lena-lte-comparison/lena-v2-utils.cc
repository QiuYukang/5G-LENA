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
#include "lena-v2-utils.h"

#include "radio-network-parameters-helper.h"
#include "flow-monitor-output-stats.h"
#include "power-output-stats.h"
#include "slot-output-stats.h"

namespace ns3 {

void
LenaV2Utils::ReportSinrNr (SinrOutputStats *stats, uint16_t cellId, uint16_t rnti,
                           double power, double avgSinr, uint16_t bwpId)
{
  stats->SaveSinr (cellId, rnti, power, avgSinr, bwpId);
}

void
LenaV2Utils::ReportPowerNr (PowerOutputStats *stats, const SfnSf & sfnSf,
                            Ptr<SpectrumValue> txPsd, Time t, uint16_t rnti, uint64_t imsi,
                            uint16_t bwpId, uint16_t cellId)
{
  stats->SavePower (sfnSf, txPsd, t, rnti, imsi, bwpId, cellId);
}

void
LenaV2Utils::ReportSlotStatsNr (SlotOutputStats *stats, const SfnSf &sfnSf, uint32_t scheduledUe,
                                uint32_t usedReg, uint32_t usedSym,
                                uint32_t availableRb, uint32_t availableSym, uint16_t bwpId,
                                uint16_t cellId)
{
  stats->SaveSlotStats (sfnSf, scheduledUe, usedReg, usedSym,
                        availableRb, availableSym, bwpId, cellId);
}

void
LenaV2Utils::SetLenaV2SimulatorParameters (const HexagonalGridScenarioHelper &gridScenario,
                                           const std::string &scenario,
                                           const std::string &radioNetwork,
                                           std::string &errorModel,
                                           const std::string &operationMode,
                                           const std::string &direction,
                                           uint16_t numerology,
                                           const std::string &pattern,
                                           const NodeContainer &gnbSector1Container,
                                           const NodeContainer &gnbSector2Container,
                                           const NodeContainer &gnbSector3Container,
                                           const NodeContainer &ueSector1Container,
                                           const NodeContainer &ueSector2Container,
                                           const NodeContainer &ueSector3Container,
                                           const Ptr<PointToPointEpcHelper> &baseEpcHelper,
                                           Ptr<NrHelper> &nrHelper,
                                           NetDeviceContainer &gnbSector1NetDev,
                                           NetDeviceContainer &gnbSector2NetDev,
                                           NetDeviceContainer &gnbSector3NetDev,
                                           NetDeviceContainer &ueSector1NetDev,
                                           NetDeviceContainer &ueSector2NetDev,
                                           NetDeviceContainer &ueSector3NetDev,
                                           bool calibration,
                                           SinrOutputStats *sinrStats,
                                           PowerOutputStats *powerStats,
                                           SlotOutputStats *slotStats,
                                           const std::string &scheduler,
                                           uint32_t bandwidthMHz)
{
  /*
   * Create the radio network related parameters
   */
  RadioNetworkParametersHelper ranHelper;
  uint8_t numScPerRb = 1;  //!< The reference signal density is different in LTE and in NR
  double rbOverhead = 0.1;
  uint32_t harqProcesses = 20;
  uint32_t n1Delay = 2;
  uint32_t n2Delay = 2;
  ranHelper.SetScenario (scenario);
  if (radioNetwork == "LTE")
    {
      ranHelper.SetNetworkToLte (operationMode, 1, bandwidthMHz);
      rbOverhead = 0.1;
      harqProcesses = 8;
      n1Delay = 4;
      n2Delay = 4;
      if (errorModel == "")
        {
          errorModel = "ns3::LenaErrorModel";
        }
      else if (errorModel != "ns3::NrLteMiErrorModel" && errorModel != "ns3::LenaErrorModel")
        {
          NS_ABORT_MSG ("The selected error model is not recommended for LTE");
        }
    }
  else if (radioNetwork == "NR")
    {
      ranHelper.SetNetworkToNr (operationMode, numerology, 1, bandwidthMHz);
      rbOverhead = 0.04;
      harqProcesses = 20;
      if (errorModel == "")
        {
          errorModel = "ns3::NrEesmCcT2";
        }
      else if (errorModel == "ns3::NrLteMiErrorModel")
        {
          NS_ABORT_MSG ("The selected error model is not recommended for NR");
        }
    }
  else
    {
      NS_ABORT_MSG ("Unrecognized radio network technology");
    }

  /*
   * Setup the NR module. We create the various helpers needed for the
   * NR simulation:
   * - IdealBeamformingHelper, which takes care of the beamforming part
   * - NrHelper, which takes care of creating and connecting the various
   * part of the NR stack
   */

  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper> ();
  nrHelper = CreateObject<NrHelper> ();

  // Put the pointers inside nrHelper
  nrHelper->SetIdealBeamformingHelper (idealBeamformingHelper);

  Ptr<NrPointToPointEpcHelper> epcHelper = DynamicCast<NrPointToPointEpcHelper> (baseEpcHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum division. We create one operational band containing three
   * component carriers, and each CC containing a single bandwidth part
   * centered at the frequency specified by the input parameters.
   * Each spectrum part length is, as well, specified by the input parameters.
   * The operational band will use StreetCanyon channel or UrbanMacro modeling.
   */
  BandwidthPartInfoPtrVector allBwps, bwps1, bwps2, bwps3;
  CcBwpCreator ccBwpCreator;
  // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
  // a single BWP per CC. Get the spectrum values from the RadioNetworkParametersHelper
  double centralFrequencyBand = ranHelper.GetCentralFrequency ();
  double bandwidthBand = ranHelper.GetBandwidth ();
  const uint8_t numCcPerBand = 1; // In this example, each cell will have one CC with one BWP
  BandwidthPartInfo::Scenario scene;
  if (scenario == "UMi")
    {
      scene =  BandwidthPartInfo::UMi_StreetCanyon_LoS;
    }
  else if (scenario == "UMa")
    {
      scene = BandwidthPartInfo::UMa_LoS;
    }
  else if (scenario == "RMa")
    {
      scene = BandwidthPartInfo::RMa_LoS;
    }
  else
    {
      NS_ABORT_MSG ("Unsupported scenario " << scenario << ". Supported values: UMi, UMa, RMa");
    }

  /*
   * Attributes of ThreeGppChannelModel still cannot be set in our way.
   * TODO: Coordinate with Tommaso
   */
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (100)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));

  // Disable shadowing in calibration, and enable it in non-calibration mode
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (!calibration));

  // Noise figure for the UE
  nrHelper->SetUePhyAttribute ("NoiseFigure", DoubleValue (9.0));

  // Error Model: UE and GNB with same spectrum error model.
  nrHelper->SetUlErrorModel (errorModel);
  nrHelper->SetDlErrorModel (errorModel);

  // Both DL and UL AMC will have the same model behind.
  nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ShannonModel));
  nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ShannonModel));

  /*
   * Adjust the average number of Reference symbols per RB only for LTE case,
   * which is larger than in NR. We assume a value of 4 (could be 3 too).
   */
  nrHelper->SetGnbDlAmcAttribute ("NumRefScPerRb", UintegerValue (numScPerRb));
  nrHelper->SetGnbUlAmcAttribute ("NumRefScPerRb", UintegerValue (1));  //FIXME: Might change in LTE

  nrHelper->SetGnbPhyAttribute ("RbOverhead", DoubleValue (rbOverhead));
  nrHelper->SetGnbPhyAttribute ("N2Delay", UintegerValue (n2Delay));
  nrHelper->SetGnbPhyAttribute ("N1Delay", UintegerValue (n1Delay));

  nrHelper->SetUeMacAttribute ("NumHarqProcess", UintegerValue (harqProcesses));
  nrHelper->SetGnbMacAttribute ("NumHarqProcess", UintegerValue (harqProcesses));

  /*
   * Create the necessary operation bands. In this example, each sector operates
   * in a separate band. Each band contains a single component carrier (CC),
   * which is made of one BWP in TDD operation mode or two BWPs in FDD mode.
   * Note that BWPs have the same bandwidth. Therefore, CCs and bands in FDD are
   * twice larger than in TDD.
   *
   * The configured spectrum division for TDD operation is:
   * |---Band1---|---Band2---|---Band3---|
   * |----CC1----|----CC2----|----CC3----|
   * |----BWP1---|----BWP2---|----BWP3---|
   *
   * And the configured spectrum division for FDD operation is:
   * |---------Band1---------|---------Band2---------|---------Band3---------|
   * |----------CC1----------|----------CC2----------|----------CC3----------|
   * |----BWP1---|----BWP2---|----BWP3---|----BWP4---|----BWP5---|----BWP6---|
   */
  double centralFrequencyBand1 = centralFrequencyBand - bandwidthBand;
  double centralFrequencyBand2 = centralFrequencyBand;
  double centralFrequencyBand3 = centralFrequencyBand + bandwidthBand;
  double bandwidthBand1 = bandwidthBand;
  double bandwidthBand2 = bandwidthBand;
  double bandwidthBand3 = bandwidthBand;

  uint8_t numBwpPerCc = 1;
  if (operationMode == "FDD")
    {
      numBwpPerCc = 2; // FDD will have 2 BWPs per CC
    }

  CcBwpCreator::SimpleOperationBandConf bandConf1 (centralFrequencyBand1, bandwidthBand1, numCcPerBand, scene);
  bandConf1.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC
  CcBwpCreator::SimpleOperationBandConf bandConf2 (centralFrequencyBand2, bandwidthBand2, numCcPerBand, scene);
  bandConf2.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC
  CcBwpCreator::SimpleOperationBandConf bandConf3 (centralFrequencyBand3, bandwidthBand3, numCcPerBand, scene);
  bandConf3.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC

  // By using the configuration created, it is time to make the operation bands
  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);
  OperationBandInfo band2 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf2);
  OperationBandInfo band3 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf3);

  if (calibration)
    {
      band1.m_cc[0]->m_bwp[0]->m_centralFrequency = 2.16e+09;
      band1.m_cc[0]->m_bwp[1]->m_centralFrequency = 1.93e+09;
      band2.m_cc[0]->m_bwp[0]->m_centralFrequency = 2.16e+09;
      band2.m_cc[0]->m_bwp[1]->m_centralFrequency = 1.93e+09;
      band3.m_cc[0]->m_bwp[0]->m_centralFrequency = 2.16e+09;
      band3.m_cc[0]->m_bwp[1]->m_centralFrequency = 1.93e+09;

      // Do not initialize fading (beamforming gain)
      nrHelper->InitializeOperationBand (&band1, NrHelper::INIT_PROPAGATION | NrHelper::INIT_CHANNEL);
      nrHelper->InitializeOperationBand (&band2, NrHelper::INIT_PROPAGATION | NrHelper::INIT_CHANNEL);
      nrHelper->InitializeOperationBand (&band3, NrHelper::INIT_PROPAGATION | NrHelper::INIT_CHANNEL);
    }
  else
    {
      // Init everything
      nrHelper->InitializeOperationBand (&band1);
      nrHelper->InitializeOperationBand (&band2);
      nrHelper->InitializeOperationBand (&band3);
    }


  allBwps = CcBwpCreator::GetAllBwps ({band1,band2,band3});
  bwps1 = CcBwpCreator::GetAllBwps ({band1});
  bwps2 = CcBwpCreator::GetAllBwps ({band2});
  bwps3 = CcBwpCreator::GetAllBwps ({band3});

  /*
   * Start to account for the bandwidth used by the example, as well as
   * the total power that has to be divided among the BWPs. Since there is only
   * one band and one BWP occupying the entire band, there is no need to divide
   * power among BWPs.
   */
  double totalTxPower = ranHelper.GetTxPower (); //Convert to mW
  double x = pow (10, totalTxPower / 10);

  /*
   * allBwps contains all the spectrum configuration needed for the nrHelper.
   *
   * Now, we can setup the attributes. We can have three kind of attributes:
   * (i) parameters that are valid for all the bandwidth parts and applies to
   * all nodes, (ii) parameters that are valid for all the bandwidth parts
   * and applies to some node only, and (iii) parameters that are different for
   * every bandwidth parts. The approach is:
   *
   * - for (i): Configure the attribute through the helper, and then install;
   * - for (ii): Configure the attribute through the helper, and then install
   * for the first set of nodes. Then, change the attribute through the helper,
   * and install again;
   * - for (iii): Install, and then configure the attributes by retrieving
   * the pointer needed, and calling "SetAttribute" on top of such pointer.
   *
   */

  /*
   *  Case (i): Attributes valid for all the nodes
   */
  // Beamforming method
  if (radioNetwork == "LTE")
    {
      idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (QuasiOmniDirectPathBeamforming::GetTypeId ()));
    }
  else
    {
      idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));
    }

  // Scheduler type

  if (scheduler == "PF")
    {
      nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaPF"));
    }
  else if (scheduler == "RR")
    {
      nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaRR"));
    }

  nrHelper->SetSchedulerAttribute ("DlCtrlSymbols", UintegerValue (1));

  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("IsotropicElements", BooleanValue (true));
  nrHelper->SetUeAntennaAttribute ("ElementGain", DoubleValue (0));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (false));
  nrHelper->SetGnbAntennaAttribute ("ElementGain", DoubleValue (0));

  // UE transmit power
  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (23.0));

  // Set LTE RBG size
  if (radioNetwork == "LTE")
    {
      double singleCcBw = numBwpPerCc == 2 ? bandwidthBand / 2 : bandwidthBand;
      if (singleCcBw == 20e6)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
        }
      else if (singleCcBw == 15e6)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
        }
      else if (singleCcBw == 10e6)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (3));
        }
      else if (singleCcBw == 5e6)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (2));
        }
      else
        {
          NS_ABORT_MSG ("Currently, only supported bandwidths are 5, 10, 15, and 20MHz, you chose " << singleCcBw);
        }
    }

  // We assume a common traffic pattern for all UEs
  uint32_t bwpIdForLowLat = 0;
  if (operationMode == "FDD" && direction == "UL")
    {
      bwpIdForLowLat = 1;
    }

  // gNb routing between Bearer and bandwidth part
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_DEFAULT", UintegerValue (bwpIdForLowLat));

  // Ue routing between Bearer and bandwidth part
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_DEFAULT", UintegerValue (bwpIdForLowLat));

  /*
   * We miss many other parameters. By default, not configuring them is equivalent
   * to use the default values. Please, have a look at the documentation to see
   * what are the default values for all the attributes you are not seeing here.
   */

  /*
   * Case (ii): Attributes valid for a subset of the nodes
   */

  // NOT PRESENT IN THIS SIMPLE EXAMPLE

  /*
   * We have configured the attributes we needed. Now, install and get the pointers
   * to the NetDevices, which contains all the NR stack:
   */

  //  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gridScenario.GetBaseStations (), allBwps);
  gnbSector1NetDev = nrHelper->InstallGnbDevice (gnbSector1Container, bwps1);
  gnbSector2NetDev = nrHelper->InstallGnbDevice (gnbSector2Container, bwps2);
  gnbSector3NetDev = nrHelper->InstallGnbDevice (gnbSector3Container, bwps3);
  ueSector1NetDev = nrHelper->InstallUeDevice (ueSector1Container, bwps1);
  ueSector2NetDev = nrHelper->InstallUeDevice (ueSector2Container, bwps2);
  ueSector3NetDev = nrHelper->InstallUeDevice (ueSector3Container, bwps3);

  /*
   * Case (iii): Go node for node and change the attributes we have to setup
   * per-node.
   */

  // Sectors (cells) of a site are pointing at different directions
  double orientationRads = gridScenario.GetAntennaOrientationRadians (0, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbSector1NetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbSector1NetDev.Get (numCell);
      uint32_t numBwps = nrHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
            ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray ());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
            ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray ());
          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));
          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));
          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          nrHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG ("Incorrect number of BWPs per CC");
        }
    }

  orientationRads = gridScenario.GetAntennaOrientationRadians (1, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbSector2NetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbSector2NetDev.Get (numCell);
      uint32_t numBwps = nrHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
            ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray ());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
            ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray ());
          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));
          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          nrHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG ("Incorrect number of BWPs per CC");
        }
    }

  orientationRads = gridScenario.GetAntennaOrientationRadians (2, gridScenario.GetNumSectorsPerSite ());
  for (uint32_t numCell = 0; numCell < gnbSector3NetDev.GetN (); ++numCell)
    {
      Ptr<NetDevice> gnb = gnbSector3NetDev.Get (numCell);
      uint32_t numBwps = nrHelper->GetNumberBwp (gnb);
      if (numBwps == 1)  // TDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna =
            ConstCast<ThreeGppAntennaArrayModel> (phy->GetSpectrumPhy ()->GetAntennaArray ());
          antenna->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue (pattern));
        }

      else if (numBwps == 2)  //FDD
        {
          // Change the antenna orientation
          Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy (gnb, 0);
          Ptr<ThreeGppAntennaArrayModel> antenna0 =
            ConstCast<ThreeGppAntennaArrayModel> (phy0->GetSpectrumPhy ()->GetAntennaArray ());
          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));
          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (ranHelper.GetNumerology ()));

          // Set TX power
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("TxPower", DoubleValue (-30.0));

          // Set TDD pattern
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          // Link the two FDD BWP
          nrHelper->GetBwpManagerGnb (gnb)->SetOutputLink (1, 0);
        }

      else
        {
          NS_ABORT_MSG ("Incorrect number of BWPs per CC");
        }
    }


  // Set the UE routing:

  if (operationMode == "FDD")
    {
      for (uint32_t i = 0; i < ueSector1NetDev.GetN (); i++)
        {
          nrHelper->GetBwpManagerUe (ueSector1NetDev.Get (i))->SetOutputLink (0, 1);
        }

      for (uint32_t i = 0; i < ueSector2NetDev.GetN (); i++)
        {
          nrHelper->GetBwpManagerUe (ueSector2NetDev.Get (i))->SetOutputLink (0, 1);
        }

      for (uint32_t i = 0; i < ueSector3NetDev.GetN (); i++)
        {
          nrHelper->GetBwpManagerUe (ueSector3NetDev.Get (i))->SetOutputLink (0, 1);
        }
    }

  for (uint32_t i = 0; i < ueSector1NetDev.GetN (); i++)
    {
      auto uePhyFirst = nrHelper->GetUePhy (ueSector1NetDev.Get (i), 0);
      uePhyFirst->TraceConnectWithoutContext ("ReportCurrentCellRsrpSinr",
                                              MakeBoundCallback (&ReportSinrNr, sinrStats));

      if (operationMode == "FDD")
        {
          auto uePhySecond = nrHelper->GetUePhy (ueSector1NetDev.Get (i), 1);
          uePhySecond->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                   MakeBoundCallback (&ReportPowerNr, powerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, powerStats));
        }
    }

  for (uint32_t i = 0; i < ueSector2NetDev.GetN (); i++)
    {
      auto uePhyFirst = nrHelper->GetUePhy (ueSector2NetDev.Get (i), 0);
      uePhyFirst->TraceConnectWithoutContext ("ReportCurrentCellRsrpSinr",
                                              MakeBoundCallback (&ReportSinrNr, sinrStats));

      if (operationMode == "FDD")
        {
          auto uePhySecond = nrHelper->GetUePhy (ueSector2NetDev.Get (i), 1);
          uePhySecond->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                   MakeBoundCallback (&ReportPowerNr, powerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, powerStats));
        }
    }

  for (uint32_t i = 0; i < ueSector3NetDev.GetN (); i++)
    {
      auto uePhyFirst = nrHelper->GetUePhy (ueSector3NetDev.Get (i), 0);
      uePhyFirst->TraceConnectWithoutContext ("ReportCurrentCellRsrpSinr",
                                              MakeBoundCallback (&ReportSinrNr, sinrStats));

      if (operationMode == "FDD")
        {
          auto uePhySecond = nrHelper->GetUePhy (ueSector3NetDev.Get (i), 1);
          uePhySecond->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                   MakeBoundCallback (&ReportPowerNr, powerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, powerStats));
        }
    }

  // When all the configuration is done, explicitly call UpdateConfig ()

  for (auto it = gnbSector1NetDev.Begin (); it != gnbSector1NetDev.End (); ++it)
    {
      uint32_t bwpId = 0;
      if (operationMode == "FDD" && direction == "UL")
        {
          bwpId = 1;
        }
      auto gnbPhy = nrHelper->GetGnbPhy (*it, bwpId);
      gnbPhy->TraceConnectWithoutContext ("SlotDataStats",
                                          MakeBoundCallback (&ReportSlotStatsNr, slotStats));

      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = gnbSector2NetDev.Begin (); it != gnbSector2NetDev.End (); ++it)
    {
      uint32_t bwpId = 0;
      if (operationMode == "FDD" && direction == "UL")
        {
          bwpId = 1;
        }
      auto gnbPhy = nrHelper->GetGnbPhy (*it, bwpId);
      gnbPhy->TraceConnectWithoutContext ("SlotDataStats",
                                          MakeBoundCallback (&ReportSlotStatsNr, slotStats));

      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = gnbSector3NetDev.Begin (); it != gnbSector3NetDev.End (); ++it)
    {
      uint32_t bwpId = 0;
      if (operationMode == "FDD" && direction == "UL")
        {
          bwpId = 1;
        }
      auto gnbPhy = nrHelper->GetGnbPhy (*it, bwpId);
      gnbPhy->TraceConnectWithoutContext ("SlotDataStats",
                                          MakeBoundCallback (&ReportSlotStatsNr, slotStats));

      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueSector1NetDev.Begin (); it != ueSector1NetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueSector2NetDev.Begin (); it != ueSector2NetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueSector3NetDev.Begin (); it != ueSector3NetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

}

} // namespace ns3
