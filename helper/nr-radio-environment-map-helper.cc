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

/*uint32_t
NrRadioEnvironmentMapHelper::GetMaxPointsPerIt () const
{
  return m_maxPointsPerIteration;
}*/

void
NrRadioEnvironmentMapHelper::ConfigurePropagationModelsFactories (Ptr<ThreeGppPropagationLossModel> propagationLossModel,
                                                                  Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel)
{
  //Get all the necessary pointers (things may be missing at this point)
  m_propagationLossModel = propagationLossModel;
  NS_ASSERT_MSG (m_propagationLossModel = propagationLossModel, "m_propagationLossModel is null");

  /***** configure channel condition model factory *****/
  m_channelConditionModelFactory.SetTypeId (m_propagationLossModel->GetChannelConditionModel ()->GetInstanceTypeId ());
  /***** configure pathloss model factory *****/
  m_propagationLossModelFactory.SetTypeId (propagationLossModel->GetInstanceTypeId ());
  /***** configure spectrum model factory *****/
  m_spectrumLossModelFactory.SetTypeId (spectrumLossModel->GetInstanceTypeId ());
}

void
NrRadioEnvironmentMapHelper::CreateRem ()
{
  NS_LOG_FUNCTION (this);

  m_outFile.open (m_outputFile.c_str ());
  if (!m_outFile.is_open ())
    {
      NS_FATAL_ERROR ("Can't open file " << (m_outputFile));
      return;
    }

  CreateListOfRemPoints ();
  CalcRemValue ();
  PrintRemToFile ();
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
NrRadioEnvironmentMapHelper::CalcRemValue ()
{
  //Create objects of rtd and rrm
          //configure nodes, devices, antennas etc
          //Call changToOmniTx
  //Iterate through the list of RemPoints
          //Set position of rtd
          //Set position of rrd
          //for loop m_numOfIterationsToAverage times
                //doCalcRxPsd
                //get average


  /****************** Create Rem Transmitting Device ***********************/
  RemDevice rtd;

  rtd.node = CreateObject<Node>();            //Create Node

  Ptr<ListPositionAllocator> rtdPositionAlloc = CreateObject<ListPositionAllocator> ();
  rtdPositionAlloc->Add (Vector(0, 0, m_z));  //Assign an initial position

  MobilityHelper rtdMobility;                 //Set Mobility
  rtdMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  rtdMobility.SetPositionAllocator (rtdPositionAlloc);
  rtdMobility.Install (rtd.node);

  rtd.dev =  CreateObject<SimpleNetDevice> ();  //Create device
  rtd.node->AddDevice(rtd.dev);

  rtd.mob = rtd.node->GetObject<MobilityModel> ();
  //Set Antenna
  rtd.antenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));
  //Configure Antenna
  rtd.antenna->ChangeToOmniTx ();
  //Configure power
  rtd.txPower = 5;
  //Configure bandwidth
  rtd.bandwidth = 100e6;
  //Configure central frequency
  rtd.frequency = 28e9;
  //Configure numerology
  rtd.numerology = 0;

  /******************* Create Rem Receiving Device ************************/
  RemDevice rrd;

  rrd.node = CreateObject<Node>();                      //Create Node

  Ptr<ListPositionAllocator> rrdPositionAlloc = CreateObject<ListPositionAllocator> ();
  rrdPositionAlloc->Add (Vector(m_xMin, m_yMin, m_z));  //Assign an initial position ----HERE I THINK I DO STH WRONG

  //In general I think I am messing up with the mobility model

  //Also we have to exclude the position of the rtd form the one
  //of rrd otherwise it gives error. This is why I have set the
  //initial position of rtd to -10, 0

  MobilityHelper rrdMobility;                           //Set Mobility
  rrdMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  rrdMobility.SetPositionAllocator (rrdPositionAlloc);
  rrdMobility.Install (rrd.node);

  rrd.dev =  CreateObject<SimpleNetDevice> ();          //Create device
  rrd.node->AddDevice(rrd.dev);

  rrd.mob = rrd.node->GetObject<MobilityModel> ();  //this is where I think I am messing up
  //Set Antenna
  rrd.antenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (1), "NumRows", UintegerValue (1));
  //Configure Antenna
  rrd.antenna->ChangeToOmniTx ();


  std::cout << "Started to generate REM points. There are in total:"<<m_rem.size()<<std::endl;
  uint16_t pointsCounter = 0;


  for (std::list<RemPoint>::iterator it = m_rem.begin ();
       it != m_rem.end ();
       ++it)
    {
      pointsCounter++;

      auto startPsdTime = std::chrono::system_clock::now();
      std::time_t start_time = std::chrono::system_clock::to_time_t(startPsdTime);
      std::cout<<"\n REM point: "<<pointsCounter<<"started at:"<<std::ctime(&start_time)<<std::endl;


      rrd.mob->SetPosition (it->pos);    //Assign to the rrd mobility all the positions of remPoint

      // configure beam on rtd antenna to point toward rrd
      rtd.antenna->SetBeamformingVector (CreateDirectPathBfv (rtd.mob, rrd.mob, rtd.antenna));
      // configure beam on rrd antenna to be quasi-omn

      UintegerValue numRows, numColumns;
      rtd.antenna->GetAttribute ("NumRows", numRows);
      rtd.antenna->GetAttribute ("NumColumns", numColumns);
      rrd.antenna->SetBeamformingVector (CreateQuasiOmniBfv (numRows.Get(), numColumns.Get()));

      //perform calculation m_numOfIterationsToAverage times and get the average value
      double sumRssi = 0;

      for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
        {
          CreateTemporalPropagationModels ();

          NS_ASSERT_MSG (m_remSpectrumLossModelCopy, "m_remSpectrumLossModelCopy is null");
          // initialize the devices in the ThreeGppSpectrumPropagationLossModel
          m_remSpectrumLossModelCopy->AddDevice (rtd.dev, rtd.antenna);
          m_remSpectrumLossModelCopy->AddDevice (rrd.dev, rrd.antenna);

          //Bw, Freq and numerology should be passed from the simulation scenario?
          Ptr<const SpectrumModel> sm1 =  MmWaveSpectrumValueHelper::GetSpectrumModel (rtd.bandwidth, rtd.frequency, rtd.numerology);
          Ptr<const SpectrumValue> txPsd1 = MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity (rtd.txPower, sm1); //txPower?

          // Copy TX PSD to RX PSD, they are now equal rxPsd == txPsd
          Ptr<SpectrumValue> rxPsd = txPsd1->Copy ();
          double pathLossDb = m_remPropagationLossModelCopy->CalcRxPower (0, rtd.mob, rrd.mob);
          double pathGainLinear = std::pow (10.0, (-pathLossDb) / 10.0);

          // Apply now calculated pathloss to rxPsd, now rxPsd < txPsd because we had some losses
          *(rxPsd) *= pathGainLinear;

          // Now we call spectrum model, which in this keys add a beamforming gain
          //rxPsd = m_remSpectrumLossModelCopy->DoCalcRxPowerSpectralDensity (rxPsd, rtd.mob, rrd.mob);

          // Now we need to apply noise to obtain SNR
          Ptr<SpectrumValue> noisePsd = MmWaveSpectrumValueHelper::CreateNoisePowerSpectralDensity (5, sm1);
          SpectrumValue snr = (*rxPsd) / (*noisePsd);
          it->snrdB = 10 * log10 (Sum (snr) / snr.GetSpectrumModel ()->GetNumBands ());
          //NS_LOG_UNCOND ("Average rx power 1: " << 10 * log10 (Sum (*rxPsd1) / rxPsd1->GetSpectrumModel ()->GetNumBands ()) << " dBm");

          double rbWidth = snr.GetSpectrumModel ()->Begin()->fh - snr.GetSpectrumModel ()->Begin()->fl;
          double rssidBm = 10 * log10 (Sum((*noisePsd + *rxPsd)* rbWidth)*1000);
          it->rssidBm = rssidBm;

          sumRssi += it->rssidBm;


      }
      it->avRssidBm = sumRssi / static_cast <double> (m_numOfIterationsToAverage);

      auto endPsdTime = std::chrono::system_clock::now();
      //std::time_t end_time = std::chrono::system_clock::to_time_t(endPsdTime);
      //std::cout<<"\n PSD end time: "<<std::ctime(&end_time)<<std::endl;
      std::chrono::duration<double> elapsed_seconds = endPsdTime - startPsdTime;
      std::cout<< "REM point finished. Execution time:"<<elapsed_seconds.count() << " seconds"<<std::endl;
      std::cout<< "\n Done:"<<(double)pointsCounter/m_rem.size()*100<<" %.";
      std::cout<<"\n Estimated time to finish:"<<(m_rem.size()-pointsCounter)*elapsed_seconds.count()/60<< "minutes"<<std::endl;

    }
}

void
NrRadioEnvironmentMapHelper::CreateTemporalPropagationModels ()
{
  //create rem copy of channel condition
  m_remCondModelCopy = m_channelConditionModelFactory.Create<ThreeGppChannelConditionModel> ();
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
                << it->sinrdB << "\t"
                << it->rssidBm << "\t"
                << it->avRssidBm << "\t"
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
