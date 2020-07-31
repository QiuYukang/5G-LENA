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

#include "flow-monitor-output-stats.h"
#include "power-output-stats.h"
#include "slot-output-stats.h"
#include "rb-output-stats.h"
namespace ns3 {

void
LenaV2Utils::ReportSinrNr (SinrOutputStats *stats, uint16_t cellId, uint16_t rnti,
                           double power, double avgSinr, uint16_t bwpId)
{
  stats->SaveSinr (cellId, rnti, power, avgSinr, bwpId);
}

void
LenaV2Utils::ReportPowerNr (PowerOutputStats *stats, const SfnSf & sfnSf,
                            Ptr<const SpectrumValue> txPsd, const Time &t, uint16_t rnti, uint64_t imsi,
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
LenaV2Utils::ReportRbStatsNr (RbOutputStats *stats, const SfnSf &sfnSf, uint8_t sym,
                              const std::vector<int> &rbUsed, uint16_t bwpId,
                              uint16_t cellId)
{
  stats->SaveRbStats (sfnSf, sym, rbUsed, bwpId, cellId);
}

void
LenaV2Utils::ReportGnbRxDataNr (PowerOutputStats *gnbRxDataStats, const SfnSf &sfnSf,
                                Ptr<const SpectrumValue> rxPsd, const Time &t, uint16_t bwpId,
                                uint16_t cellId)
{
  gnbRxDataStats->SavePower (sfnSf, rxPsd, t, 0, 0, bwpId, cellId);
}

void
LenaV2Utils::ConfigureBwpTo (BandwidthPartInfoPtr & bwp, double centerFreq, double bwpBw)
{
  bwp->m_centralFrequency = centerFreq;
  bwp->m_higherFrequency = centerFreq + (bwpBw / 2);
  bwp->m_lowerFrequency = centerFreq - (bwpBw / 2);
  bwp->m_channelBandwidth = bwpBw;
}

void
LenaV2Utils::SetLenaV2SimulatorParameters (const HexagonalGridScenarioHelper &gridScenario,
                                           const std::string &scenario,
                                           const std::string &radioNetwork,
                                           std::string errorModel,
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
                                           PowerOutputStats *ueTxPowerStats,
                                           PowerOutputStats *gnbRxPowerStats,
                                           SlotOutputStats *slotStats, RbOutputStats *rbStats,
                                           const std::string &scheduler,
                                           uint32_t bandwidthMHz, uint32_t freqScenario)
{
  /*
   * Create the radio network related parameters
   */
  uint8_t numScPerRb = 1;  //!< The reference signal density is different in LTE and in NR
  double rbOverhead = 0.1;
  uint32_t harqProcesses = 20;
  uint32_t n1Delay = 2;
  uint32_t n2Delay = 2;
  if (radioNetwork == "LTE")
    {
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

  nrHelper = CreateObject<NrHelper> ();

  Ptr<IdealBeamformingHelper> idealBeamformingHelper;

  // in LTE non-calibration we want to use predefined beams that we set directly
  // through beam manager. Hence, we do not need any ideal algorithm.
  // For other cases, we need it (and the beam will be overwritten)
  if (radioNetwork == "NR" || calibration)
    {
      idealBeamformingHelper = CreateObject<IdealBeamformingHelper> ();
      nrHelper->SetIdealBeamformingHelper (idealBeamformingHelper);
    }

  Ptr<NrPointToPointEpcHelper> epcHelper = DynamicCast<NrPointToPointEpcHelper> (baseEpcHelper);
  nrHelper->SetEpcHelper (epcHelper);

  double txPowerBs = 0.0;

  BandwidthPartInfo::Scenario scene;
  if (scenario == "UMi")
    {
      txPowerBs = 30;
      scene =  BandwidthPartInfo::UMi_StreetCanyon_LoS;
    }
  else if (scenario == "UMa")
    {
      txPowerBs = 43;
      scene = BandwidthPartInfo::UMa_LoS;
    }
  else if (scenario == "RMa")
    {
      txPowerBs = 43;
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
   * Create the necessary operation bands.
   *
   * In the 0 frequency scenario, each sector operates, in a separate band,
   * while for scenario 1 all the sectors are in the same band. Please note that
   * a single BWP in FDD is half the size of the corresponding TDD BWP, and the
   * parameter bandwidthMHz refers to the size of the FDD BWP.
   *
   * TDD scenario 0:
   *
   * |----------------Band1--------------|
   * |----CC1----|----CC2----|----CC3----|   // Sector i will go in BWPi
   * |----BWP1---|----BWP2---|----BWP3---|
   *
   * FDD scenario 0:
   *
   * And the configured spectrum division for FDD operation is:
   * |---------Band1---------|---------Band2---------|---------Band3---------|
   * |----------CC1----------|----------CC1----------|----------CC1----------| // Sector i will go in Bandi
   * |----BWP1---|----BWP2---|----BWP1---|----BWP2---|----BWP1---|----BWP2---| // DL in the first, UL in the second
   *
   *
   * TDD scenario 1:
   *
   * |----Band1----|
   * |-----CC1-----|
   * |-----BWP1----|
   *
   * FDD scenario 1:
   *
   * And the configured spectrum division for FDD operation is:
   * |---------Band1---------|
   * |----------CC1----------|
   * |----BWP1---|----BWP2---|
   *
   * This is tightly coupled with what happens in lena-v1-utils.cc
   *
   */
  OperationBandInfo band1, band2, band3;
  band1.m_bandId = 0;
  band2.m_bandId = 1;
  band3.m_bandId = 2;

  if (freqScenario == 0) // NON_OVERLAPPING
    {
      CcBwpCreator ccBwpCreator;

      double bandwidthBand = operationMode == "FDD" ? bandwidthMHz * 1e6 : bandwidthMHz * 1e6 * 2;
      uint8_t numCcPerBand = 1; // one for each sector

      CcBwpCreator::SimpleOperationBandConf bandConf1 (2125e6, bandwidthBand, numCcPerBand, scene);
      bandConf1.m_numBwp = operationMode == "FDD" ? 2 : 1; // FDD will have 2 BWPs per CC

      CcBwpCreator::SimpleOperationBandConf bandConf2 (2145e6, bandwidthBand, numCcPerBand, scene);
      bandConf2.m_numBwp = operationMode == "FDD" ? 2 : 1; // FDD will have 2 BWPs per CC

      CcBwpCreator::SimpleOperationBandConf bandConf3 (2165e6, bandwidthBand, numCcPerBand, scene);
      bandConf3.m_numBwp = operationMode == "FDD" ? 2 : 1; // FDD will have 2 BWPs per CC

      band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1); // Create, then configure
      band2 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf2); // Create, then configure
      band3 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf3); // Create, then configure

      if (operationMode == "FDD")
        {
          ConfigureBwpTo (band1.m_cc[0]->m_bwp[0], 2120e6, bandwidthBand);
          ConfigureBwpTo (band1.m_cc[0]->m_bwp[1], 2130e6, bandwidthBand);

          ConfigureBwpTo (band2.m_cc[0]->m_bwp[0], 2140e6, bandwidthBand);
          ConfigureBwpTo (band2.m_cc[0]->m_bwp[1], 2150e6, bandwidthBand);

          ConfigureBwpTo (band3.m_cc[0]->m_bwp[0], 2160e6, bandwidthBand);
          ConfigureBwpTo (band3.m_cc[0]->m_bwp[1], 1850e6, bandwidthBand);
        }
      else
        {
          ConfigureBwpTo (band1.m_cc[0]->m_bwp[0], 2120e6, bandwidthBand);
          ConfigureBwpTo (band2.m_cc[0]->m_bwp[0], 2140e6, bandwidthBand);
          ConfigureBwpTo (band3.m_cc[0]->m_bwp[0], 2160e6, bandwidthBand);
        }

      std::cout << "BWP Configuration for NON_OVERLAPPING case, mode " << operationMode
                << std::endl << band1 << std::endl << band2 << std::endl << band3;
    }
  else // OVERLAPPING
    {
      CcBwpCreator ccBwpCreator;

      double bandwidthBand = operationMode == "FDD" ? bandwidthMHz * 1e6 : bandwidthMHz * 1e6 * 2;
      uint8_t numCcPerBand = 1;

      CcBwpCreator::SimpleOperationBandConf bandConf1 (2120e6, bandwidthBand, numCcPerBand, scene);
      bandConf1.m_numBwp = operationMode == "FDD" ? 2 : 1; // FDD will have 2 BWPs per CC

      // We use the helper function to create the band, and manually we go
      // to change what is wrong
      band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);

      if (operationMode == "FDD")
        {
          ConfigureBwpTo (band1.m_cc[0]->m_bwp[0], 2120e6, bandwidthBand);
          ConfigureBwpTo (band1.m_cc[0]->m_bwp[1], 1930e6, bandwidthBand);
        }
      else
        {
          // TDD here, so use the double of the passed parameter (that is for FDD)
          // You can see this in the definition of bandwidthBand
          ConfigureBwpTo (band1.m_cc[0]->m_bwp[0], 2120e6, bandwidthBand);
        }

      std::cout << "BWP Configuration for OVERLAPPING case, mode " << operationMode
                << std::endl << band1 << std::endl;
    }

  if (calibration)
    {
      // LENA-compatibility-bug: Put all the sectors and stuff at the same central frequency
      // in case of non-overlapping mode and FDD
      if (operationMode == "FDD" && freqScenario == 0)
        {
          band1.m_cc[0]->m_bwp[0]->m_centralFrequency = 2.16e+09;
          band1.m_cc[0]->m_bwp[1]->m_centralFrequency = 1.93e+09;
          band2.m_cc[0]->m_bwp[0]->m_centralFrequency = 2.16e+09;
          band2.m_cc[0]->m_bwp[1]->m_centralFrequency = 1.93e+09;
          band3.m_cc[0]->m_bwp[0]->m_centralFrequency = 2.16e+09;
          band3.m_cc[0]->m_bwp[1]->m_centralFrequency = 1.93e+09;
        }

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

  BandwidthPartInfoPtrVector sector1Bwps, sector2Bwps, sector3Bwps;
  if (freqScenario == 0) // NON_OVERLAPPING
    {
      sector1Bwps = CcBwpCreator::GetAllBwps ({band1});
      sector2Bwps = CcBwpCreator::GetAllBwps ({band2});
      sector3Bwps = CcBwpCreator::GetAllBwps ({band3});
    }
  else // OVERLAPPING
    {
      sector1Bwps = CcBwpCreator::GetAllBwps ({band1});
      sector2Bwps = CcBwpCreator::GetAllBwps ({band1});
      sector3Bwps = CcBwpCreator::GetAllBwps ({band1});
    }

  /*
   * Start to account for the bandwidth used by the example, as well as
   * the total power that has to be divided among the BWPs. Since we are TDD
   * or FDD with 2 BWP only, there is no need to divide anything.
   */
  double totalTxPower = txPowerBs; //Convert to mW
  double x = pow (10, totalTxPower / 10);

  /*
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

  if (radioNetwork == "LTE" && calibration == true)
    {
      idealBeamformingHelper->SetAttribute ("IdealBeamformingMethod", TypeIdValue (QuasiOmniDirectPathBeamforming::GetTypeId ()));
    }
  else if (radioNetwork == "NR")
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
  if (calibration)
    {
      nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (1));
      nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (1));
    }
  else
    {
      nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (5));
      nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (2));
    }

  nrHelper->SetGnbAntennaAttribute ("IsotropicElements", BooleanValue (false));
  nrHelper->SetGnbAntennaAttribute ("ElementGain", DoubleValue (0));

  // UE transmit power
  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (23.0));

  // Set LTE RBG size
  // TODO: What these values would be in TDD? bandwidthMhz refers to FDD.
  // for example, for TDD, if we have bandwidthMhz to 20, we will have a 40 MHz
  // BWP.
  if (radioNetwork == "LTE")
    {
      if (bandwidthMHz == 20)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
        }
      else if (bandwidthMHz == 15)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
        }
      else if (bandwidthMHz == 10)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (3));
        }
      else if (bandwidthMHz == 5)
        {
          nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (2));
        }
      else
        {
          NS_ABORT_MSG ("Currently, only supported bandwidths are 5, 10, 15, and 20MHz, you chose " << bandwidthMHz);
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
  gnbSector1NetDev = nrHelper->InstallGnbDevice (gnbSector1Container, sector1Bwps);
  gnbSector2NetDev = nrHelper->InstallGnbDevice (gnbSector2Container, sector2Bwps);
  gnbSector3NetDev = nrHelper->InstallGnbDevice (gnbSector3Container, sector3Bwps);
  ueSector1NetDev = nrHelper->InstallUeDevice (ueSector1Container, sector1Bwps);
  ueSector2NetDev = nrHelper->InstallUeDevice (ueSector2Container, sector2Bwps);
  ueSector3NetDev = nrHelper->InstallUeDevice (ueSector3Container, sector3Bwps);

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

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy->GetBeamManager ()->SetPredefinedBeam (3, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));

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

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy0->GetBeamManager ()->SetPredefinedBeam (3, 30);

          antenna0->SetAttribute ("BearingAngle", DoubleValue (orientationRads));
          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy1->GetBeamManager ()->SetPredefinedBeam (3, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (numerology));

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

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy->GetBeamManager ()->SetPredefinedBeam (2, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));

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

          // configure the beam that points toward the center of hexagonal.
          // In case of beamforming, it will be overwritten.
          phy0->GetBeamManager ()->SetPredefinedBeam (2, 30);

          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy1->GetBeamManager ()->SetPredefinedBeam (2, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (numerology));

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

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy->GetBeamManager ()->SetPredefinedBeam (0, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));

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

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy0->GetBeamManager ()->SetPredefinedBeam (0, 30);

          Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy (gnb, 1);
          Ptr<ThreeGppAntennaArrayModel> antenna1 =
            ConstCast<ThreeGppAntennaArrayModel> (phy1->GetSpectrumPhy ()->GetAntennaArray ());
          antenna1->SetAttribute ("BearingAngle", DoubleValue (orientationRads));

          // configure the beam that points toward the center of hexagonal
          // In case of beamforming, it will be overwritten.
          phy1->GetBeamManager ()->SetPredefinedBeam (0, 30);

          // Set numerology
          nrHelper->GetGnbPhy (gnb, 0)->SetAttribute ("Numerology", UintegerValue (numerology));
          nrHelper->GetGnbPhy (gnb, 1)->SetAttribute ("Numerology", UintegerValue (numerology));

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
                                                   MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
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
                                                   MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
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
                                                   MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
        }
      else
        {
          uePhyFirst->TraceConnectWithoutContext ("ReportPowerSpectralDensity",
                                                  MakeBoundCallback (&ReportPowerNr, ueTxPowerStats));
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
      gnbPhy->TraceConnectWithoutContext ("RBDataStats",
                                          MakeBoundCallback (&ReportRbStatsNr, rbStats));
      gnbPhy->GetSpectrumPhy()->TraceConnectWithoutContext ("RxDataTrace",
                                                            MakeBoundCallback (&ReportGnbRxDataNr, gnbRxPowerStats));

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
      gnbPhy->TraceConnectWithoutContext ("RBDataStats",
                                          MakeBoundCallback (&ReportRbStatsNr, rbStats));
      gnbPhy->GetSpectrumPhy()->TraceConnectWithoutContext ("RxDataTrace",
                                                            MakeBoundCallback (&ReportGnbRxDataNr, gnbRxPowerStats));

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
      gnbPhy->TraceConnectWithoutContext ("RBDataStats",
                                          MakeBoundCallback (&ReportRbStatsNr, rbStats));
      gnbPhy->GetSpectrumPhy()->TraceConnectWithoutContext ("RxDataTrace",
                                                            MakeBoundCallback (&ReportGnbRxDataNr, gnbRxPowerStats));

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
