/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/command-line.h>
#include <ns3/show-progress.h>

#include "cttc-nr-3gpp-calibration.h"

using namespace ns3;

/**
 * \ingroup examples
 * \file cttc-nr-3gpp-calibration-user.cc
 * \brief A multi-cell network deployment with site sectorization
 *
 * This example describes how to setup a simulation using the 3GPP channel model
 * from TR 38.900. This example consists of an hexagonal grid deployment
 * consisting on a central site and a number of outer rings of sites around this
 * central site. Each site is sectorized, meaning that a number of three antenna
 * arrays or panels are deployed per gNB. These three antennas are pointing to
 * 30º, 150º and 270º w.r.t. the horizontal axis. We allocate a band to each
 * sector of a site, and the bands are contiguous in frequency.
 *
 * We provide a number of simulation parameters that can be configured in the
 * command line, such as the number of UEs per cell or the number of outer rings.
 * Please have a look at the possible parameters to know what you can configure
 * through the command line.
 *
 * With the default configuration, the example will create one DL flow per UE.
 * The example will print on-screen the end-to-end result of each flow,
 * as well as writing them on a file.
 *
 * \code{.unparsed}
$ ./waf --run "cttc-nr-3gpp-calibration-user --Help"
    \endcode
 *
 */
int
main (int argc, char *argv[])
{
  Parameters params;
  /*
   * From here, we instruct the ns3::CommandLine class of all the input parameters
   * that we may accept as input, as well as their description, and the storage
   * variable.
   */
  CommandLine cmd;

  cmd.AddValue ("configurationType",
                "Choose among a) customConf and b) calibrationConf."
                "a) allows custom configuration through the command line,"
                "while b) allows user to select one of the predefined"
                "calibration scenarios. Please notice that if b) is selected"
                "custom parameters should not be set through the command line",
                params.confType);
  cmd.AddValue ("configurationScenario",
                "The calibration scenario string (DenseA, DenseB, RuralA, RuralB)."
                "This variable must be set when calibrationConf is choosen",
                params.configurationScenario);
  cmd.AddValue ("scenario",
                "The urban scenario string (UMa,UMi,RMa)",
                params.scenario);
  cmd.AddValue ("numRings",
                "The number of rings around the central site",
                params.numOuterRings);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per cell or gNB in multiple-ue topology",
                params.ueNumPergNb);
  cmd.AddValue ("siteFile",
                "Path to file of tower coordinates (instead of hexagonal grid)",
                params.baseStationFile);
  cmd.AddValue ("useSiteFile",
                "If true, it will be used site file, otherwise it will be used "
                "numRings parameter to create scenario.",
                params.useSiteFile);
  cmd.AddValue ("appGenerationTime",
                "Duration applications will generate traffic.",
                params.appGenerationTime);
  cmd.AddValue ("numerologyBwp",
                "The numerology to be used (NR only)",
                params.numerologyBwp);
  cmd.AddValue ("pattern",
                "The TDD pattern to use",
                params.pattern);
  cmd.AddValue ("direction",
                "The flow direction (DL or UL)",
                params.direction);
  cmd.AddValue ("simulator",
                "The cellular network simulator to use: LENA or 5GLENA",
                params.simulator);
  cmd.AddValue ("technology",
                "The radio access network technology (LTE or NR)",
                params.radioNetwork);
  cmd.AddValue ("operationMode",
                "The network operation mode can be TDD or FDD",
                params.operationMode);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                params.simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                params.outputDir);
  cmd.AddValue ("errorModelType",
               "Error model type: ns3::NrEesmCcT1, ns3::NrEesmCcT2, "
               "ns3::NrEesmIrT1, ns3::NrEesmIrT2, ns3::NrLteMiErrorModel",
               params.errorModel);
  cmd.AddValue ("calibration",
                "disable a bunch of things to make LENA and NR_LTE comparable",
                params.calibration);
  cmd.AddValue ("trafficScenario",
                "0: saturation (80 Mbps/20 MHz), 1: latency (1 pkt of 12 bytes), "
                "2: low-load (1 Mbps), 3: medium-load (20Mbps)",
                params.trafficScenario);
  cmd.AddValue ("scheduler",
                "PF: Proportional Fair, RR: Round-Robin",
                params.scheduler);
  cmd.AddValue ("bandwidth",
                "BW in MHz for each BWP (integer value): valid values are 20, 10, 5",
                params.bandwidthMHz);
  cmd.AddValue ("startingFreq",
                "Frequency for the first band. Rest of the bands will be configured"
                "accordingly based on the configured BW",
                params.startingFreq);
  cmd.AddValue ("freqScenario",
                "0: NON_OVERLAPPING (each sector in different freq), 1: OVERLAPPING (same freq for all sectors)",
                params.freqScenario);
  cmd.AddValue ("downtiltAngle",
                "Base station antenna down tilt angle (deg)",
                params.downtiltAngle);
  cmd.AddValue ("bearingAngle",
                "Base station antenna bearing angle (rad)",
                params.bearingAngle);
  cmd.AddValue ("enableUlPc",
                "Whether to enable or disable UL power control",
                params.enableUlPc);
  cmd.AddValue ("powerAllocation",
                "Power allocation can be a)UniformPowerAllocBw or b)UniformPowerAllocUsed.",
                params.powerAllocation);
  cmd.AddValue ("xMin",
                "The min x coordinate of the rem map",
                params.xMinRem);
  cmd.AddValue ("xMax",
                "The max x coordinate of the rem map",
                params.xMaxRem);
  cmd.AddValue ("xRes",
                "The resolution on the x axis of the rem map",
                params.xResRem);
  cmd.AddValue ("yMin",
                "The min y coordinate of the rem map",
                params.yMinRem);
  cmd.AddValue ("yMax",
                "The max y coordinate of the rem map",
                params.yMaxRem);
  cmd.AddValue ("yRes",
                "The resolution on the y axis of the rem map",
                params.yResRem);
  cmd.AddValue ("z",
                "The z coordinate of the rem map",
                params.zRem);
  cmd.AddValue ("dlRem",
                "Generates DL REM without executing simulation",
                params.dlRem);
  cmd.AddValue ("ulRem",
                "Generates UL REM without executing simulation",
                params.ulRem);
  cmd.AddValue ("remSector",
                "For which sector to generate the rem",
                 params.remSector);
  cmd.AddValue ("progressInterval",
                "Progress reporting interval",
                params.progressInterval);
  //Newly added parameters
  cmd.AddValue ("gnbTxPower",
                "The transmit power of the gNB",
                params.gnbTxPower);
  cmd.AddValue ("ueTxPower",
                "The transmit power of the UE",
                params.ueTxPower);
  cmd.AddValue ("gnbNumRows",
                "The number of rows of the phased array of the gNB",
                params.gnbNumRows);
  cmd.AddValue ("gnbNumColumns",
                "The number of columns of the phased array of the gNB",
                params.gnbNumColumns);
  cmd.AddValue ("ueNumRows",
                "The number of rows of the phased array of the UE",
                params.ueNumRows);
  cmd.AddValue ("ueNumColumns",
                "The number of columns of the phased array of the UE",
                params.ueNumColumns);
  cmd.AddValue ("gnbHSpacing",
                "Horizontal spacing between antenna elements, "
                "in multiples of wave length, for the gNB",
                params.gnbHSpacing);
  cmd.AddValue ("gnbVSpacing",
                "Vertical spacing between antenna elements, "
                "in multiples of wave length for the gNB",
                params.gnbVSpacing);
  cmd.AddValue ("ueHSpacing",
                "Horizontal spacing between antenna elements, "
                "in multiples of wave length, for the UE",
                params.ueHSpacing);
  cmd.AddValue ("ueVSpacing",
                "Vertical spacing between antenna elements, "
                "in multiples of wave length, for the UE",
                params.ueVSpacing);
  cmd.AddValue ("gnbNoiseFigure",
                "gNB Noise Figure",
                params.gnbNoiseFigure);
  cmd.AddValue ("ueNoiseFigure",
                "UE Noise Figure",
                params.ueNoiseFigure);
  cmd.AddValue ("enableRealBF",
                "If true, Real BeamForming method is configured (must be disabled "
                "for calibration)",
                params.enableRealBF);
  cmd.AddValue ("gnbEnable3gppElement",
                "If true, it enables 3GPP Antenna element configuration in the gNB",
                params.gnbEnable3gppElement);
  cmd.AddValue ("ueEnable3gppElement",
                "If true, it enables 3GPP Antenna element configuration in the UE",
                params.ueEnable3gppElement);
  cmd.AddValue ("enableShadowing",
                "If true, it enables shadowing (must be disabled for calibration)",
                params.enableShadowing);


  // Parse the command line
  cmd.Parse (argc, argv);
  params.Validate ();

  //in case calibrationConf is choosen, it sets the parameters of one
  //of the four pre-defined scenarios (DenseA, DenseB, RuralA, RuralB)
  ChooseCalibrationScenario (params);
  
  std::cout << params;

  ShowProgress spinner (params.progressInterval);
  
  Nr3gppCalibration (params);

  return 0;
}
