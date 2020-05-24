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

#include "nr-radio-environment-map-helper.h"

#include <ns3/abort.h>
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/uinteger.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include "ns3/pointer.h"
#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/node.h>
#include "ns3/mobility-module.h"
#include <ns3/constant-position-mobility-model.h>
#include <ns3/spectrum-model.h>
#include "ns3/mmwave-spectrum-value-helper.h"
#include "ns3/beamforming-vector.h"
#include "ns3/mmwave-enb-net-device.h"
#include "ns3/mmwave-ue-net-device.h"
#include <ns3/mmwave-spectrum-phy.h>
#include <ns3/spectrum-converter.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRadioEnvironmentMapHelper");

NS_OBJECT_ENSURE_REGISTERED (NrRadioEnvironmentMapHelper);


NrRadioEnvironmentMapHelper::NrRadioEnvironmentMapHelper (double bandwidth, double frequency, uint8_t numerology)
{
  // all devices must have the same spectrum model to perform calculation,
  // if some of the device is of the different then its transmission will have to
  // converted into spectrum model of this device
  m_rrd.spectrumModel = MmWaveSpectrumValueHelper::GetSpectrumModel (bandwidth, frequency, numerology);
}

NrRadioEnvironmentMapHelper::NrRadioEnvironmentMapHelper ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

NrRadioEnvironmentMapHelper::~NrRadioEnvironmentMapHelper ()
{

}

void
NrRadioEnvironmentMapHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrRadioEnvironmentMapHelper::GetTypeId (void)
{
  NS_LOG_FUNCTION ("NrRadioEnvironmentMapHelper::GetTypeId");
  static TypeId tid = TypeId ("ns3::NrRadioEnvironmentMapHelper")
                      .SetParent<Object> ()
                      .SetGroupName("Nr")
                      .AddConstructor<NrRadioEnvironmentMapHelper> ()
                      .AddAttribute ("OutputFile",
                                     "the filename to which the NR Radio"
                                     "Environment Map is saved",
                                     StringValue ("NR_REM.out"),
                                     MakeStringAccessor (&NrRadioEnvironmentMapHelper::m_outputFile),
                                     MakeStringChecker ())
                      .AddAttribute ("XMin",
                                     "The min x coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMinX,
                                                         &NrRadioEnvironmentMapHelper::GetMinX),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("YMin",
                                     "The min y coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMinY,
                                                         &NrRadioEnvironmentMapHelper::GetMinY),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("XMax",
                                     "The max x coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMaxX,
                                                         &NrRadioEnvironmentMapHelper::GetMaxX),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("YMax",
                                     "The max y coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMaxY,
                                                         &NrRadioEnvironmentMapHelper::GetMaxY),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("XRes",
                                     "The resolution (number of points) of the"
                                     "map along the x axis.",
                                     UintegerValue (100),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetResX,
                                                           &NrRadioEnvironmentMapHelper::GetResX),
                                     MakeUintegerChecker<uint32_t> (2,std::numeric_limits<uint16_t>::max ()))
                      .AddAttribute ("YRes",
                                     "The resolution (number of points) of the"
                                     "map along the y axis.",
                                     UintegerValue (100),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetResY,
                                                           &NrRadioEnvironmentMapHelper::GetResY),
                                     MakeUintegerChecker<uint16_t> (2,std::numeric_limits<uint16_t>::max ()))
                      .AddAttribute ("Z",
                                     "The value of the z coordinate for which"
                                     "the map is to be generated.",
                                     DoubleValue (1.5),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetZ,
                                                         &NrRadioEnvironmentMapHelper::GetZ),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("IterForAverage",
                                     "Number of iterations for the calculation"
                                     "of the average rem value.",
                                     UintegerValue (1),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetNumOfItToAverage),
                                                           //&NrRadioEnvironmentMapHelper::GetMaxPointsPerIt),
                                     MakeUintegerChecker<uint16_t> ())
                      .AddAttribute ("RemMode",
                                     "There are two high level modes of Rem generation: "
                                     "a) BEAM_SHAPE in which are represented the beams that are configured "
                                     "in the user's script scenario, considering that the receiver always has quasi-omni, and that all the beams "
                                     "point toward the UE which is passed as UE of interest. The purpose of this map is to illustrate "
                                     "the REM of the scenario that is configured."
                                     "b) COVERAGE_AREA which produces two REM maps: the worst-case SINR and best-SNR for each rem position;"
                                     "Worst case SINR means that all interfering devices use for the transmission the beam toward the rem point;"
                                     "and also for the best-SNR, for each transmitting device and the REM point are used the best directional beam-pair "
                                     "and then is selected the best SNR.",
                                     EnumValue (NrRadioEnvironmentMapHelper::COVERAGE_AREA),
                                     MakeEnumAccessor (&NrRadioEnvironmentMapHelper::SetRemMode,
                                                       &NrRadioEnvironmentMapHelper::GetRemMode),
                                     MakeEnumChecker (NrRadioEnvironmentMapHelper::BEAM_SHAPE, "BeamShape",
                                                      NrRadioEnvironmentMapHelper::COVERAGE_AREA, "CoverageArea"))
    ;
  return tid;
}

void
NrRadioEnvironmentMapHelper::SetRemMode (enum RemMode remMode)
{
  m_remMode = remMode;
}

void
NrRadioEnvironmentMapHelper::SetMinX (double xMin)
{
  m_xMin = xMin;
}

void
NrRadioEnvironmentMapHelper::SetMinY (double yMin)
{
  m_yMin = yMin;
}

void
NrRadioEnvironmentMapHelper::SetMaxX (double xMax)
{
  m_xMax = xMax;
}

void
NrRadioEnvironmentMapHelper::SetMaxY (double yMax)
{
  m_yMax = yMax;
}

void
NrRadioEnvironmentMapHelper::SetResX (uint16_t xRes)
{
  m_xRes = xRes;
}

void
NrRadioEnvironmentMapHelper::SetResY (uint16_t yRes)
{
  m_yRes = yRes;
}

void
NrRadioEnvironmentMapHelper::SetZ (double z)
{
  m_z = z;
}

void
NrRadioEnvironmentMapHelper::SetNumOfItToAverage (uint16_t numOfIterationsToAverage)
{
  m_numOfIterationsToAverage = numOfIterationsToAverage;
}

enum NrRadioEnvironmentMapHelper::RemMode
NrRadioEnvironmentMapHelper::GetRemMode () const
{
  return m_remMode;
}

double
NrRadioEnvironmentMapHelper::GetMinX () const
{
  return m_xMin;
}

double
NrRadioEnvironmentMapHelper::GetMinY () const
{
  return m_yMin;
}

double
NrRadioEnvironmentMapHelper::GetMaxX () const
{
  return m_xMax;
}

double
NrRadioEnvironmentMapHelper::GetMaxY () const
{
  return m_yMax;
}

uint16_t
NrRadioEnvironmentMapHelper::GetResX () const
{
  return m_xRes;
}

uint16_t
NrRadioEnvironmentMapHelper::GetResY () const
{
  return m_yRes;
}

double
NrRadioEnvironmentMapHelper::GetZ () const
{
  return m_z;
}

void NrRadioEnvironmentMapHelper::ConfigureRrd (Ptr<NetDevice> &ueDevice, uint8_t bwpId)
{
    m_rrd.mob->SetPosition (ueDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ());

    //Get Ue Phy
    Ptr<MmWaveUeNetDevice> mmwUeNetDev = ueDevice->GetObject<MmWaveUeNetDevice> ();
    Ptr<const MmWaveUePhy> rrdPhy = mmwUeNetDev->GetPhy (bwpId);
    NS_ASSERT_MSG (rrdPhy, "rrdPhy is null");

    m_rrd.antenna = Copy (rrdPhy->GetAntennaArray ());

    m_noisePsd = MmWaveSpectrumValueHelper::CreateNoisePowerSpectralDensity (rrdPhy->GetNoiseFigure (), m_rrd.spectrumModel);
}

void NrRadioEnvironmentMapHelper::ConfigureRtdList (NetDeviceContainer enbNetDev, uint8_t bwpId)
{
  for (NetDeviceContainer::Iterator netDevIt = enbNetDev.Begin ();
      netDevIt != enbNetDev.End ();
      ++netDevIt)
    {
      RemDevice rtd;

      rtd.mob->SetPosition ((*netDevIt)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ());

      Ptr<MmWaveEnbNetDevice> mmwNetDev = (*netDevIt)->GetObject<MmWaveEnbNetDevice> ();
      Ptr<const MmWaveEnbPhy> rtdPhy = mmwNetDev->GetPhy (bwpId);
      NS_ASSERT_MSG (rtdPhy, "rtdPhy is null");

      rtd.antenna = Copy (rtdPhy->GetAntennaArray ());
      //Configure power
      rtd.txPower = rtdPhy->GetTxPower ();
      //Configure spectrum model which will be needed to create tx PSD
      rtd.spectrumModel = rtdPhy->GetSpectrumModel ();

      if (rtd.spectrumModel != m_rrd.spectrumModel)
        {
          NS_LOG_WARN ("RTD device with different spectrum model, this may slow "
                       "down significantly the REM map creation. Consider setting "
                       "the same frequency, bandwidth, and numerology to all "
                       "devices which are used for REM map creation.");
        };

      NS_LOG_INFO ("\n rtd spectrum model: " << rtd.spectrumModel->GetUid () <<
                   "\n rtd number of bands: " << rtd.spectrumModel->GetNumBands () <<
                   "\n create new RTD element... " <<
                   "\n rtdPhy->GetCentralFrequency () " << rtdPhy->GetCentralFrequency () / 10e6 << " MHz " <<
                   "\n bw: " << rtdPhy->GetChannelBandwidth () / 10e6 << " MHz " <<
                   "\n num: "<< rtdPhy->GetNumerology ());

      if (netDevIt == enbNetDev.Begin())
        {
          ConfigurePropagationModelsFactories (rtdPhy); // we can call only once configuration of prop.models
        }

      m_remDev.push_back (rtd);
    }
}

void
NrRadioEnvironmentMapHelper::ConfigurePropagationModelsFactories (Ptr<const MmWaveEnbPhy> rtdPhy)
{
  Ptr<const MmWaveSpectrumPhy> txSpectrumPhy = rtdPhy->GetSpectrumPhy ();
  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel ();

  m_propagationLossModel = DynamicCast<ThreeGppPropagationLossModel> (txSpectrumChannel->GetPropagationLossModel ());
  Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (txSpectrumChannel->GetSpectrumPropagationLossModel ());

  NS_ASSERT_MSG (m_propagationLossModel, "m_propagationLossModel is null");
  NS_ASSERT_MSG (m_spectrumLossModel, "m_spectrumLossModel is null");

  /***** configure channel condition model factory *****/
  m_channelConditionModelFactory.SetTypeId (m_propagationLossModel->GetChannelConditionModel ()->GetInstanceTypeId ());
  /***** configure pathloss model factory *****/
  m_propagationLossModelFactory.SetTypeId (m_propagationLossModel->GetInstanceTypeId ());
  /***** configure spectrum model factory *****/
  m_spectrumLossModelFactory.SetTypeId (m_spectrumLossModel->GetInstanceTypeId ());
}

void
NrRadioEnvironmentMapHelper::CreateRem (NetDeviceContainer enbNetDev, Ptr<NetDevice> &ueDevice, uint8_t bwpId)
{
  NS_LOG_FUNCTION (this);

  m_outFile.open (m_outputFile.c_str ());
  if (!m_outFile.is_open ())
    {
      NS_FATAL_ERROR ("Can't open file " << (m_outputFile));
      return;
    }

  ConfigureRtdList (enbNetDev, bwpId);
  CreateListOfRemPoints ();
  ConfigureRrd (ueDevice, bwpId);
  CalcCurrentRemMap ();
  PrintRemToFile ();
  PrintGnuplottableUeListToFile ("ues.txt");
  PrintGnuplottableEnbListToFile ("enbs.txt");
}

void
NrRadioEnvironmentMapHelper::CreateListOfRemPoints ()
{
  NS_LOG_FUNCTION (this);

  //Create the list of the REM Points

  m_xStep = (m_xMax - m_xMin)/(m_xRes);
  m_yStep = (m_yMax - m_yMin)/(m_yRes);

  NS_ASSERT_MSG (m_xMax > m_xMin, "xMax must be higher than xMin");
  NS_ASSERT_MSG (m_yMax > m_yMin, "yMax must be higher than yMin");
  NS_ASSERT_MSG (m_xRes != 0 || m_yRes != 0, "Resolution must be higher than 0");

  NS_LOG_INFO ("m_xStep: " << m_xStep << " m_yStep: " << m_yStep);

  for (double x = m_xMin; x < m_xMax + 0.5*m_xStep; x += m_xStep)
    {
      for (double y = m_yMin; y < m_yMax + 0.5*m_yStep ; y += m_yStep)
        {
          RemPoint remPoint;

          remPoint.pos.x = x;
          remPoint.pos.y = y;
          remPoint.pos.z = m_z;

          m_rem.push_back (remPoint);
        }
    }
}

void
NrRadioEnvironmentMapHelper::ConfigureQuasiOmniBfv (RemDevice& device)
{
  // configure beam on rrd antenna to be quasi-omni
  UintegerValue numRows, numColumns;
  device.antenna->GetAttribute ("NumRows", numRows);
  device.antenna->GetAttribute ("NumColumns", numColumns);
  // configure RRD antenna to have quasi omni beamforming vector
  device.antenna->SetBeamformingVector (CreateQuasiOmniBfv (numRows.Get (), numColumns.Get ()));
}

void
NrRadioEnvironmentMapHelper::ConfigureDirectPathBfv (RemDevice& device, const RemDevice& otherDevice)
{
  device.antenna->SetBeamformingVector (CreateDirectPathBfv (device.mob, otherDevice.mob, m_rrd.antenna));
}

Ptr<SpectrumValue>
NrRadioEnvironmentMapHelper::CalcRxPsdValue (RemPoint& itRemPoint, RemDevice& itRtd)
{
  PropagationModels tempPropModels = CreateTemporalPropagationModels ();

  // initialize the devices in the ThreeGppSpectrumPropagationLossModel
  tempPropModels.remSpectrumLossModelCopy->AddDevice (itRtd.dev, itRtd.antenna);
  tempPropModels.remSpectrumLossModelCopy->AddDevice (m_rrd.dev, m_rrd.antenna);
  Ptr<const SpectrumValue> txPsd = MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity (itRtd.txPower, itRtd.spectrumModel);

  // check if RTD has the same spectrum model as RRD
  // if they have do nothing, if they dont, then convert txPsd of RTD device so to be according to spectrum model of RRD

  Ptr <const SpectrumValue> convertedTxPsd;
  if (itRtd.spectrumModel->GetUid () == m_rrd.spectrumModel->GetUid ())
    {
      NS_LOG_LOGIC ("no spectrum conversion needed");
      convertedTxPsd = txPsd;
    }
  else
    {
      NS_LOG_LOGIC ("Converting TXPSD of RTD device " << itRtd.spectrumModel->GetUid ()  << " --> " << m_rrd.spectrumModel->GetUid ());
      SpectrumConverter converter (itRtd.spectrumModel, m_rrd.spectrumModel);
      convertedTxPsd = converter.Convert (txPsd);
    }

  // Copy TX PSD to RX PSD, they are now equal rxPsd == txPsd
  Ptr<SpectrumValue> rxPsd = convertedTxPsd->Copy ();
  double pathLossDb = tempPropModels.remPropagationLossModelCopy->CalcRxPower (0, itRtd.mob, m_rrd.mob);
  double pathGainLinear = std::pow (10.0, (pathLossDb) / 10.0);

  // Apply now calculated pathloss to rxPsd, now rxPsd < txPsd because we had some losses
  *(rxPsd) *= pathGainLinear;

  // Now we call spectrum model, which in this keys add a beamforming gain
  rxPsd = tempPropModels.remSpectrumLossModelCopy->DoCalcRxPowerSpectralDensity (rxPsd, itRtd.mob, m_rrd.mob);

  return rxPsd;
}

double NrRadioEnvironmentMapHelper::CalculateSnr (const std::vector <Ptr<SpectrumValue>>& receivedPowerList)
{
   SpectrumValue maxRxPsd (m_rrd.spectrumModel);
   SpectrumValue allSignals (m_rrd.spectrumModel); // we need spectrum model of the receiver in order to call constructor of SpectrumValue
   allSignals = 0;
   maxRxPsd = 0;

   for (auto rxPower: receivedPowerList)
     {
       allSignals += (*rxPower);
       if (Sum (*(rxPower)) > Sum (maxRxPsd))
         {
           maxRxPsd = *rxPower;
         }
     }

   NS_ASSERT_MSG (Sum (maxRxPsd) != 0, "Maximum RX PSD should be different from 0.");

   SpectrumValue snr = (maxRxPsd) / (*m_noisePsd);
   return 10 * log10 (Sum (snr) / snr.GetSpectrumModel ()->GetNumBands ());
}

double
NrRadioEnvironmentMapHelper::CalculateMaxSinr (const std::vector <Ptr<SpectrumValue>>& receivedPowerList)
{
  SpectrumValue allSignals (m_rrd.spectrumModel), maxSinr (m_rrd.spectrumModel);
  allSignals = 0;
  maxSinr = 0;

  // we sum all signals
  for (auto rxPower: receivedPowerList)
    {
      allSignals += (*rxPower);
    }

  // we calculate sinr considering for each RTD as if it would be TX device, and the rest of RTDs interferers
  for (auto rxPower: receivedPowerList)
    {
      SpectrumValue interf =  (allSignals) - (*rxPower) + (*m_noisePsd);
      SpectrumValue sinr = (*rxPower) / interf;

      if (Sum (sinr) > Sum (maxSinr))
        {
          maxSinr = sinr;
        }
    }
  return 10 * log10 (Sum (maxSinr) / maxSinr.GetSpectrumModel ()->GetNumBands ()) ;
}

void
NrRadioEnvironmentMapHelper::CalcCurrentRemMap ()
{
  //Save REM creation start time
  auto remStartTime = std::chrono::system_clock::now ();
  uint16_t calcRxPsdCounter = 0;

  if (m_remMode == COVERAGE_AREA)
    {
      //configure each RTD beam toward RRD
     for(std::list<RemDevice>::iterator itRtd = m_remDev.begin ();
         itRtd != m_remDev.end ();
                ++itRtd)
        {
         ConfigureDirectPathBfv (*itRtd, m_rrd);
        }
    }
  else if (m_remMode == BEAM_SHAPE)
    {
      ConfigureQuasiOmniBfv (m_rrd);
    }

  for (std::list<RemPoint>::iterator itRemPoint = m_rem.begin ();
      itRemPoint != m_rem.end ();
      ++itRemPoint)
    {
      //perform calculation m_numOfIterationsToAverage times and get the average value
      double sumSnr = 0.0, sumSinr = 0.0;
      m_rrd.mob->SetPosition (itRemPoint->pos);

      for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
        {
          std::vector <Ptr<SpectrumValue>> receivedPowerList;// RTD node id, rxPsd of the singal coming from that node

          for (std::list<RemDevice>::iterator itRtd = m_remDev.begin(); itRtd != m_remDev.end (); ++itRtd)
            {
              calcRxPsdCounter++;

              if (m_remMode == COVERAGE_AREA)
                {
                  //configure RRD beam toward RTD
                  ConfigureDirectPathBfv (m_rrd, *itRtd);
                }
               // calculate received power from the current RTD device
               receivedPowerList.push_back (CalcRxPsdValue (*itRemPoint, *itRtd));

              NS_LOG_UNCOND ("Done:" <<
                             (double)calcRxPsdCounter/(m_rem.size()*m_numOfIterationsToAverage*m_remDev.size ()) * 100 <<
                             " %."); // how many times will be called CalcRxPsdValues

            } //end for std::list<RemDev>::iterator  (RTDs)

          sumSnr += CalculateSnr (receivedPowerList);
          sumSinr += CalculateMaxSinr (receivedPowerList);

          receivedPowerList.clear();
        }//end for m_numOfIterationsToAverage  (Average)

      itRemPoint->avgSnrDb = sumSnr / static_cast <double> (m_numOfIterationsToAverage);
      itRemPoint->avgSinrDb = sumSinr / static_cast <double> (m_numOfIterationsToAverage);

    } //end for std::list<RemPoint>::iterator  (RemPoints)

  auto remEndTime = std::chrono::system_clock::now ();
  std::chrono::duration<double> remElapsedSeconds = remEndTime - remStartTime;
  NS_LOG_UNCOND ("REM map created. Total time needed to create the REM map:" <<
                 remElapsedSeconds.count () / 60 << " minutes.");
}

NrRadioEnvironmentMapHelper::PropagationModels
NrRadioEnvironmentMapHelper::CreateTemporalPropagationModels ()
{
  //create rem copy of channel condition
  Ptr<ChannelConditionModel> m_remCondModelCopy = m_channelConditionModelFactory.Create<ChannelConditionModel> ();
  NS_ASSERT_MSG (m_remCondModelCopy, "m_remCondModelCopy is null");

  PropagationModels propModels;

  //create rem copy of propagation model
  propModels.remPropagationLossModelCopy = m_propagationLossModelFactory.Create <ThreeGppPropagationLossModel> ();
  NS_ASSERT_MSG (propModels.remPropagationLossModelCopy, "propModels.remPropagationLossModelCopy is null");
  propModels.remPropagationLossModelCopy->SetAttribute ("Frequency", DoubleValue (m_propagationLossModel->GetFrequency ()));
  //TODO: Check how to get if shadowing is true or false
  propModels.remPropagationLossModelCopy->SetAttribute ("ShadowingEnabled", BooleanValue (false));
  propModels.remPropagationLossModelCopy->SetChannelConditionModel (m_remCondModelCopy);

  // create rem copy of spectrum model
  propModels.remSpectrumLossModelCopy = m_spectrumLossModelFactory.Create <ThreeGppSpectrumPropagationLossModel> ();
  NS_ASSERT_MSG (propModels.remSpectrumLossModelCopy, "propModels.remSpectrumLossModelCopy is null");
  propModels.remSpectrumLossModelCopy->SetChannelModelAttribute ("Frequency", DoubleValue (m_propagationLossModel->GetFrequency ()));
  //TODO: Check how to get the scenario
  std::string scenario = "UMa";
  propModels.remSpectrumLossModelCopy->SetChannelModelAttribute ("Scenario", StringValue (scenario));
  propModels.remSpectrumLossModelCopy->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (m_remCondModelCopy));

  return propModels;
}

void
NrRadioEnvironmentMapHelper::PrintGnuplottableEnbListToFile (std::string filename)
{
  std::ofstream gnbOutFile;
  gnbOutFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!gnbOutFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  for (std::list<RemDevice>::iterator itRtd = m_remDev.begin ();
       itRtd != m_remDev.end ();
       ++itRtd)
   {
      Vector pos = itRtd->dev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();

      gnbOutFile << "set label \"" << itRtd->dev->GetNode ()->GetId () <<
                    "\" at "<< pos.x << "," << pos.y <<
                    " left font \"Helvetica,4\" textcolor rgb \"white\" front  point pt 2 ps 0.3 lc rgb \"white\" offset 0,0" <<
                    std::endl;
    }
}

void
NrRadioEnvironmentMapHelper::PrintGnuplottableUeListToFile (std::string filename)
{
  std::ofstream ueOutFile;
  ueOutFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!ueOutFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  Vector pos = m_rrd.node->GetObject<MobilityModel> ()->GetPosition ();

  ueOutFile << "set label \"" << m_rrd.dev->GetNode ()->GetId () <<
               "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,4\" textcolor rgb \"grey\" front point pt 1 ps 0.3 lc rgb \"grey\" offset 0,0" <<
               std::endl;
}

void
NrRadioEnvironmentMapHelper::PrintRemToFile ()
{
  NS_LOG_FUNCTION (this);

  for (std::list<RemPoint>::iterator it = m_rem.begin ();
       it != m_rem.end ();
       ++it)
    {
      m_outFile << it->pos.x << "\t"
                << it->pos.y << "\t"
                << it->pos.z << "\t"
                << it->avgSnrDb << "\t"
                << it->avgSinrDb << "\t"
                << std::endl;
    }

  Finalize ();
}

void
NrRadioEnvironmentMapHelper::Finalize ()
{
  NS_LOG_FUNCTION (this);
  m_outFile.close ();

  //Simulator::Stop ();
}


} // namespace ns3
