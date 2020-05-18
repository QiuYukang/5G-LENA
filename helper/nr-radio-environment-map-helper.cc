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
    m_rrd.antenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (1), "NumRows", UintegerValue (1));
    //Configure Antenna
    //rrd.antenna->ChangeToOmniTx ();



}

void NrRadioEnvironmentMapHelper::ConfigureRtdList (NetDeviceContainer enbNetDev)
 {
     for (NetDeviceContainer::Iterator netDevIt = enbNetDev.Begin ();
          netDevIt != enbNetDev.End ();
          ++netDevIt)
     {
        /****************** Create Rem Transmitting Devices *******************/
        RemDevice rtd;

        rtd.node = CreateObject<Node> ();            //Create Node

        Ptr<ListPositionAllocator> rtdPositionAlloc = CreateObject<ListPositionAllocator> ();
        //rtdPositionAlloc->Add (Vector(0, 0, m_z));  //Assign an initial position
        rtdPositionAlloc->Add ((*netDevIt)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ());  //Assign the enbNetDev position

        MobilityHelper rtdMobility;                 //Set Mobility
        rtdMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
        rtdMobility.SetPositionAllocator (rtdPositionAlloc);
        rtdMobility.Install (rtd.node);

        rtd.dev =  CreateObject<SimpleNetDevice> ();  //Create device
        rtd.node->AddDevice(rtd.dev);

        rtd.mob = rtd.node->GetObject<MobilityModel> ();
        //rtd.mob = (*netDevIt)->GetNode ()->GetObject<MobilityModel> ();
        //Set Antenna
        rtd.antenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (4), "NumRows", UintegerValue (4));
        //Configure Antenna
        //rtd.antenna->ChangeToOmniTx ();
        //Configure power
        rtd.txPower = 1;
        //Configure bandwidth
        rtd.bandwidth = 100e6;
        //Configure central frequency
        rtd.frequency = 28e9;
        //Configure numerology
        rtd.numerology = 0;

        m_remDev.push_back (rtd);
     }
}

void
NrRadioEnvironmentMapHelper::ConfigurePropagationModelsFactories (Ptr<ThreeGppPropagationLossModel> propagationLossModel,
                                                                  Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel)
{
  //Get all the necessary pointers (things may be missing at this point)
  //If we pass freq, numm and bw, this pointer might not be necessary,
  //so we could remove it
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
  ConfigureRrd ();

  //Save REM creation start time
  auto remStartTime = std::chrono::system_clock::now();

  for (std::list<RemDevice>::iterator itRtd = m_remDev.begin ();
       itRtd != m_remDev.end ();
       ++itRtd)
   {
      std::cout << "Started to generate REM points for: " <<  itRtd->dev->GetNode () <<
                   "There are in total:" << m_rem.size() << std::endl;
      uint16_t pointsCounter = 0;

      // configure beam on rtd antenna to point toward rrd
      // We configure RRD with some point, we want to see only the beam toward a single point
      m_rrd.mob->SetPosition (Vector (10, 0, 1.5));
      itRtd->antenna->SetBeamformingVector (CreateDirectPathBfv (itRtd->mob, m_rrd.mob, itRtd->antenna));


      for (std::list<RemPoint>::iterator itRemPoint = m_rem.begin ();
           itRemPoint != m_rem.end ();
           ++itRemPoint)
       {
          pointsCounter++;

          auto startPsdTime = std::chrono::system_clock::now();
          std::time_t start_time = std::chrono::system_clock::to_time_t(startPsdTime);
          std::cout<<"\n REM point: "<<pointsCounter<<"/"<<m_rem.size()<<" started at:"<<std::ctime(&start_time)<<std::endl;

          m_rrd.mob->SetPosition (itRemPoint->pos);    //Assign to the rrd mobility all the positions of remPoint

          // configure beam on rtd antenna to point toward rrd
          itRtd->antenna->SetBeamformingVector (CreateDirectPathBfv (itRtd->mob, m_rrd.mob, itRtd->antenna));

          // configure beam on rrd antenna to be quasi-omni
          UintegerValue numRows, numColumns;
          m_rrd.antenna->GetAttribute ("NumRows", numRows);
          m_rrd.antenna->GetAttribute ("NumColumns", numColumns);
          m_rrd.antenna->SetBeamformingVector (CreateQuasiOmniBfv (numRows.Get(), numColumns.Get()));

          //perform calculation m_numOfIterationsToAverage times and get the average value
          double sumRssi = 0, sumSnr = 0;

          for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
            {
              CreateTemporalPropagationModels ();

              NS_ASSERT_MSG (m_remSpectrumLossModelCopy, "m_remSpectrumLossModelCopy is null");
              // initialize the devices in the ThreeGppSpectrumPropagationLossModel
              m_remSpectrumLossModelCopy->AddDevice (itRtd->dev, itRtd->antenna);
              m_remSpectrumLossModelCopy->AddDevice (m_rrd.dev, m_rrd.antenna);

              //Bw, Freq and numerology should be passed from the simulation scenario?
              Ptr<const SpectrumModel> sm1 =  MmWaveSpectrumValueHelper::GetSpectrumModel (itRtd->bandwidth, itRtd->frequency, itRtd->numerology);
              Ptr<const SpectrumValue> txPsd1 = MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity (itRtd->txPower, sm1); //txPower?

              // Copy TX PSD to RX PSD, they are now equal rxPsd == txPsd
              Ptr<SpectrumValue> rxPsd = txPsd1->Copy ();
              double pathLossDb = m_remPropagationLossModelCopy->CalcRxPower (0, itRtd->mob, m_rrd.mob);
              double pathGainLinear = std::pow (10.0, (pathLossDb) / 10.0);

              // Apply now calculated pathloss to rxPsd, now rxPsd < txPsd because we had some losses
              *(rxPsd) *= pathGainLinear;

              // Now we call spectrum model, which in this keys add a beamforming gain
              rxPsd = m_remSpectrumLossModelCopy->DoCalcRxPowerSpectralDensity (rxPsd, itRtd->mob, m_rrd.mob);

              // Now we need to apply noise to obtain SNR
              Ptr<SpectrumValue> noisePsd = MmWaveSpectrumValueHelper::CreateNoisePowerSpectralDensity (5, sm1);
              SpectrumValue snr = (*rxPsd) / (*noisePsd);
              itRemPoint->snrdB = 10 * log10 (Sum (snr) / snr.GetSpectrumModel ()->GetNumBands ());
              sumSnr += itRemPoint->snrdB;

              //NS_LOG_UNCOND ("Average rx power 1: " << 10 * log10 (Sum (*rxPsd1) / rxPsd1->GetSpectrumModel ()->GetNumBands ()) << " dBm");
              itRemPoint->sinrdB = itRemPoint->snrdB; // we save SNR,  until we have some interferers, then we will calculate SINR

              double rbWidth = snr.GetSpectrumModel ()->Begin()->fh - snr.GetSpectrumModel ()->Begin()->fl;
              double rssidBm = 10 * log10 (Sum((*noisePsd + *rxPsd)* rbWidth)*1000);
              itRemPoint->rssidBm = rssidBm;

              sumRssi += itRemPoint->rssidBm;

           }  //end for m_numOfIterationsToAverage  (Average)

           itRemPoint->avRssidBm = sumRssi / static_cast <double> (m_numOfIterationsToAverage);
           itRemPoint->avSnrdB = sumSnr / static_cast <double> (m_numOfIterationsToAverage);

           auto endPsdTime = std::chrono::system_clock::now();
           //std::time_t end_time = std::chrono::system_clock::to_time_t(endPsdTime);
           //std::cout<<"\n PSD end time: "<<std::ctime(&end_time)<<std::endl;
           std::chrono::duration<double> elapsed_seconds = endPsdTime - startPsdTime;
           //std::cout<< "REM point finished. Execution time:"<<elapsed_seconds.count() << " seconds."<<std::endl;
           std::cout<<"\n Done:"<<(double)pointsCounter/m_rem.size()*100<<" %.";
           std::cout<<"\n Estimated time to finish REM:"<<(m_rem.size()-pointsCounter)*elapsed_seconds.count()/60<<" minutes."<<std::endl;

        }  //end for std::list<RemPoint>::iterator  (RemPoints)

      auto remEndTime = std::chrono::system_clock::now();
      std::chrono::duration<double> remElapsedSeconds = remEndTime - remStartTime;
      std::cout<<"\n Total time was needed to create the REM map:"<<remElapsedSeconds.count()/60<<" minutes."<<std::endl;

  }  //end for std::list<RemDev>::iterator  (RTDs)
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
                << it->snrdB <<"\t"
                << it->avSnrdB<<"\t"
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
