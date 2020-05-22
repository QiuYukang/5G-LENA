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
#include <ns3/mmwave-spectrum-phy.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRadioEnvironmentMapHelper");

NS_OBJECT_ENSURE_REGISTERED (NrRadioEnvironmentMapHelper);


NrRadioEnvironmentMapHelper::NrRadioEnvironmentMapHelper ()
{
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
    ;
  return tid;
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

void NrRadioEnvironmentMapHelper::ConfigureRrd ()
{
    m_rrd.node = CreateObject<Node> ();                      //Create Node

    Ptr<ListPositionAllocator> rrdPositionAlloc = CreateObject<ListPositionAllocator> ();
    rrdPositionAlloc->Add (Vector(m_xMin, m_yMin, m_z));  //Assign an initial position

    MobilityHelper rrdMobility;                           //Set Mobility
    rrdMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    rrdMobility.SetPositionAllocator (rrdPositionAlloc);
    rrdMobility.Install (m_rrd.node);

    m_rrd.dev =  CreateObject<SimpleNetDevice> ();          //Create device
    m_rrd.node->AddDevice(m_rrd.dev);

    m_rrd.mob = m_rrd.node->GetObject<MobilityModel> ();
    //Set Antenna
    m_rrd.antenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (1), "NumRows", UintegerValue (1), "IsotropicElements", BooleanValue (false));
    //Configure Antenna

    //TODO Copy antenna configuration from UE device
    //m_rrd.antenna = Copy(uePhy->GetAntennaArray());

    //TODO following parameter should be obtained from the UE device, it is done something simillar in ConfigureRtdList function
    Ptr<const SpectrumModel> sm1 =  MmWaveSpectrumValueHelper::GetSpectrumModel (100e6, 28e9, 4);

    //TODO noiseFigure, in this case set to 5, should be obtained from UE device phy
    m_noisePsd = MmWaveSpectrumValueHelper::CreateNoisePowerSpectralDensity (5, sm1);  //TODO take noise figure from UE or RRD
}

void NrRadioEnvironmentMapHelper::ConfigureRtdList (NetDeviceContainer enbNetDev, uint8_t ccId)
 {
     for (NetDeviceContainer::Iterator netDevIt = enbNetDev.Begin ();
          netDevIt != enbNetDev.End ();
          ++netDevIt)
     {
        /****************** Create Rem Transmitting Devices *******************/
        RemDevice rtd;
        rtd.mob->SetPosition((*netDevIt)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ());  //Assign the enbNetDev position

        Ptr<MmWaveEnbNetDevice> mmwNetDev = (*netDevIt)->GetObject<MmWaveEnbNetDevice> ();
        NS_ASSERT_MSG (mmwNetDev, "mmwNetDev is null");
        Ptr<const MmWaveEnbPhy> rtdPhy = mmwNetDev->GetPhy(ccId);

        //TODO if below "Copy (rtdPhy->GetAntennaArray());" works fine, then we can remove all this part
        /*UintegerValue uintValue;
        rtdPhy->GetAntennaArray()->GetAttribute("NumRows", uintValue);
        uint32_t rtdNumRows = static_cast<uint32_t> (uintValue.Get());
        rtdPhy->GetAntennaArray()->GetAttribute("NumColumns", uintValue);
        uint32_t rtdNumColumns = static_cast<uint32_t> (uintValue.Get());
        BooleanValue boolValue;
        rtdPhy->GetAntennaArray()->GetAttribute("IsotropicElements", boolValue);
        bool iso = static_cast<bool> (boolValue.Get());*/
        //Set Antenna
        //rtd.antenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (rtdNumRows), "NumRows", UintegerValue (rtdNumColumns), "IsotropicElements", BooleanValue (iso));

        // TODO We should check if this really copies the parameters
        // Copy should return a deep copy of a antenna.
        rtd.antenna = Copy (rtdPhy->GetAntennaArray());

        //E.g. in the following way
        UintegerValue uintValue1, uintValue2;
        rtd.antenna->GetAttribute("NumColumns", uintValue1);
        rtdPhy->GetAntennaArray()->GetAttribute ("NumColumns", uintValue2);
        //std::cout<<"\n "<<uintValue.Get() << " =? "<<

        //Configure power
        rtd.txPower = rtdPhy->GetTxPower();
        //Configure spectrum model which will be needed to create tx PSD
        // TODO resolve how to obtain central frequency rtdPhy->GetCentralFrequency()
        //double frequency = rtdPhy->GetCentralFrequency(); // why this function is protected??
        rtd.spectrumModel =  MmWaveSpectrumValueHelper::GetSpectrumModel (static_cast <double>(rtdPhy->GetChannelBandwidth()), 28.0e9 , static_cast <uint8_t> (rtdPhy->GetNumerology()));

        ConfigurePropagationModelsFactories (rtdPhy);

        m_remDev.push_back (rtd);
     }
}

void
NrRadioEnvironmentMapHelper::ConfigurePropagationModelsFactories (Ptr<const MmWaveEnbPhy> rtdPhy)
{
  Ptr<const MmWaveSpectrumPhy> txSpectrumPhy = rtdPhy->GetSpectrumPhy();
  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel();

  m_propagationLossModel = DynamicCast<ThreeGppPropagationLossModel> (txSpectrumChannel->GetPropagationLossModel());
  m_spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (txSpectrumChannel->GetSpectrumPropagationLossModel());

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
NrRadioEnvironmentMapHelper::CreateRem (NetDeviceContainer enbNetDev, uint8_t ccId)
{
  NS_LOG_FUNCTION (this);

  m_outFile.open (m_outputFile.c_str ());
  if (!m_outFile.is_open ())
    {
      NS_FATAL_ERROR ("Can't open file " << (m_outputFile));
      return;
    }

  ConfigureRtdList (enbNetDev, ccId);
  CreateListOfRemPoints ();
  ConfigureRrd ();
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

  //I leave this for the moment to perform tests with various resolutions
  std::cout << "m_xStep: " << m_xStep << " m_yStep: " << m_yStep << std::endl;

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
  device.antenna->SetBeamformingVector (CreateQuasiOmniBfv (numRows.Get(), numColumns.Get()));
}

void
NrRadioEnvironmentMapHelper::ConfigureDirectPathBfv (RemDevice& device, const RemDevice& otherDevice)
{
  device.antenna->SetBeamformingVector (CreateDirectPathBfv (device.mob, otherDevice.mob, m_rrd.antenna));
}


Ptr<SpectrumValue>
NrRadioEnvironmentMapHelper::CalcRxPsdValues (RemPoint& itRemPoint, RemDevice& itRtd)
{
  // TODO once we have configure of RTD and RRM devices working we should remove these following lines that configure antenna beams
  // configure beam on rtd antenna to point toward rrd
  itRtd.antenna->SetBeamformingVector (CreateDirectPathBfv (itRtd.mob, m_rrd.mob, itRtd.antenna));
  ConfigureDirectPathBfv (m_rrd, itRtd);
  itRtd.antenna->SetAttribute("IsotropicElements", BooleanValue(true));
  m_rrd.antenna->SetAttribute("IsotropicElements", BooleanValue(true));

  CreateTemporalPropagationModels ();

  NS_ASSERT_MSG (m_remSpectrumLossModelCopy, "m_remSpectrumLossModelCopy is null");
  // initialize the devices in the ThreeGppSpectrumPropagationLossModel
  m_remSpectrumLossModelCopy->AddDevice (itRtd.dev, itRtd.antenna);
  m_remSpectrumLossModelCopy->AddDevice (m_rrd.dev, m_rrd.antenna);

  Ptr<const SpectrumValue> txPsd1 = MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity (itRtd.txPower, itRtd.spectrumModel); //txPower?

  // Copy TX PSD to RX PSD, they are now equal rxPsd == txPsd
  Ptr<SpectrumValue> rxPsd = txPsd1->Copy ();
  double pathLossDb = m_remPropagationLossModelCopy->CalcRxPower (0, itRtd.mob, m_rrd.mob);
  double pathGainLinear = std::pow (10.0, (pathLossDb) / 10.0);

  // Apply now calculated pathloss to rxPsd, now rxPsd < txPsd because we had some losses
  *(rxPsd) *= pathGainLinear;

  // Now we call spectrum model, which in this keys add a beamforming gain
  rxPsd = m_remSpectrumLossModelCopy->DoCalcRxPowerSpectralDensity (rxPsd, itRtd.mob, m_rrd.mob);

  return rxPsd;
}

double NrRadioEnvironmentMapHelper::CalculateSnr (const std::vector <Ptr<SpectrumValue>>& receivedPowerList)
{
   SpectrumValue maxRxPsd (m_noisePsd->GetSpectrumModel()), allSignals (m_noisePsd->GetSpectrumModel()); // we need spectrum model of the receiver in order to call constructor of SpectrumValue,
                                                                                                         //we can obtain model from m_noisePsd, because  m_noisePsd is/will be initialized with
                                                                                                         //RRD parameters (bandwidth, frequency, numerology) in ConfigureRRD device
   maxRxPsd = 0;
   allSignals = 0;

   for (auto rxPower:receivedPowerList)
     {
       allSignals += (*rxPower);

       if (Sum(*(rxPower)) > Sum(maxRxPsd))
         {
           maxRxPsd = *rxPower;
         }
     }

   NS_ASSERT_MSG (Sum(maxRxPsd)!=0, "Maximum RX PSD should be different from 0.");

   SpectrumValue snr = (maxRxPsd) / (*m_noisePsd);
   return 10 * log10 (Sum (snr) / snr.GetSpectrumModel ()->GetNumBands ());
}

double
NrRadioEnvironmentMapHelper::CalculateMaxSinr (const std::vector <Ptr<SpectrumValue>>& receivedPowerList)
{
  SpectrumValue allSignals (m_noisePsd->GetSpectrumModel()), maxSinr (m_noisePsd->GetSpectrumModel());
  allSignals = 0;
  maxSinr = 0;

  // we sum all signals
  for (auto rxPower:receivedPowerList)
    {
      allSignals += (*rxPower);
    }

  // we calculate sinr considering for each RTD as if it would be TX device, and the rest of RTDs interferers
  for (auto rxPower:receivedPowerList)
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
  auto remStartTime = std::chrono::system_clock::now();
  uint16_t pointsCounter = 0;

  for (std::list<RemPoint>::iterator itRemPoint = m_rem.begin ();
      itRemPoint != m_rem.end ();
         ++itRemPoint)
    {
      //perform calculation m_numOfIterationsToAverage times and get the average value
      double sumSnr = 0.0, sumSinr = 0.0;

      for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
        {
          std::vector <Ptr<SpectrumValue>> receivedPowerList;// RTD node id, rxPsd of the singla coming from that node

          for (auto itRtd:m_remDev)
            {
              pointsCounter++;

              m_rrd.mob->SetPosition (itRemPoint->pos);

              receivedPowerList.push_back (CalcRxPsdValues (*itRemPoint, itRtd));

              NS_LOG_UNCOND ("Done:"<<(double)pointsCounter/m_rem.size()*100<<" %.");
            } //end for std::list<RemDev>::iterator  (RTDs)

          sumSnr += CalculateSnr (receivedPowerList);
          sumSinr += CalculateMaxSinr (receivedPowerList);

          receivedPowerList.clear();
        }//end for m_numOfIterationsToAverage  (Average)


      itRemPoint->avgSnrDb = sumSnr / static_cast <double> (m_numOfIterationsToAverage);
      itRemPoint->avgSinrDb = sumSinr / static_cast <double> (m_numOfIterationsToAverage);

      std::cout<<"\n sumSnr:"<<sumSnr<<std::endl;
      std::cout<<"\n avgSnrDb = "<<itRemPoint->avgSnrDb<<std::endl;

    } //end for std::list<RemPoint>::iterator  (RemPoints)

  auto remEndTime = std::chrono::system_clock::now();
  std::chrono::duration<double> remElapsedSeconds = remEndTime - remStartTime;
  NS_LOG_UNCOND ("REM map created. Total time needed to create the REM map:"<<remElapsedSeconds.count()/60<<" minutes.");
}

void
NrRadioEnvironmentMapHelper::CreateTemporalPropagationModels ()
{
  //create rem copy of channel condition
  m_remCondModelCopy = m_channelConditionModelFactory.Create<ChannelConditionModel> ();
  NS_ASSERT_MSG (m_remCondModelCopy, "m_remCondModelCopy is null");

  //create rem copy of propagation model
  m_remPropagationLossModelCopy = m_propagationLossModelFactory.Create <ThreeGppPropagationLossModel> ();
  NS_ASSERT_MSG (m_remPropagationLossModelCopy, "m_remPropagationLossModelCopy is null");
  m_remPropagationLossModelCopy->SetAttribute ("Frequency", DoubleValue (m_propagationLossModel->GetFrequency ()));
  //TODO: Check how to get if shadowing is true or false
  m_remPropagationLossModelCopy->SetAttribute ("ShadowingEnabled", BooleanValue (false));
  m_remPropagationLossModelCopy->SetChannelConditionModel (m_remCondModelCopy);

  // create rem copy of spectrum model
  m_remSpectrumLossModelCopy = m_spectrumLossModelFactory.Create <ThreeGppSpectrumPropagationLossModel> ();
  NS_ASSERT_MSG (m_remSpectrumLossModelCopy, "m_remSpectrumLossModelCopy is null");
  m_remSpectrumLossModelCopy->SetChannelModelAttribute ("Frequency", DoubleValue (m_propagationLossModel->GetFrequency ()));
  //TODO: Check how to get the scenario
  std::string scenario = "UMa";
  m_remSpectrumLossModelCopy->SetChannelModelAttribute ("Scenario", StringValue (scenario));
  m_remSpectrumLossModelCopy->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (m_remCondModelCopy));
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
                << it->avgSnrDb<<"\t"
                << it->avgSinrDb<<"\t"
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
