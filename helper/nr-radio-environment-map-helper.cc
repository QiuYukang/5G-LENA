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
#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/node.h>
#include "ns3/mobility-module.h"
#include <ns3/constant-position-mobility-model.h>
#include <ns3/spectrum-model.h>
#include "ns3/mmwave-spectrum-value-helper.h"



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
                      /*.AddAttribute ("MaxPointsPerIteration",
                                     "Maximum number of REM points to be "
                                     "calculated per iteration.",
                                     UintegerValue (20000),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetMaxPointsPerIt,
                                                           &NrRadioEnvironmentMapHelper::GetMaxPointsPerIt),
                                     MakeUintegerChecker<uint32_t> (1,std::numeric_limits<uint32_t>::max ()))*/
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

/*void
NrRadioEnvironmentMapHelper::SetMaxPointsPerIt (uint32_t maxPointsPerIt)
{
  m_maxPointsPerIteration = maxPointsPerIt;
}*/

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
NrRadioEnvironmentMapHelper::CreateRem (Ptr<ThreeGppPropagationLossModel> propagationLossModel,
                                            Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel)
{
  NS_LOG_FUNCTION (this);

  m_outFile.open (m_outputFile.c_str ());
  if (!m_outFile.is_open ())
    {
      NS_FATAL_ERROR ("Can't open file " << (m_outputFile));
      return;
    }

  //Get all the necessaty pointers (things may be missing at this point)
  m_propagationLossModel = propagationLossModel;
  m_spectrumLossModel = spectrumLossModel;
  m_condModel = m_propagationLossModel->GetChannelConditionModel ();

  CreateListOfRemPoints ();
  CalcRemValue ();
  PrintRemToFile ();

}

void
NrRadioEnvironmentMapHelper::CreateListOfRemPoints ()
{
  NS_LOG_FUNCTION (this);

  //Create the list of the REM Points

  for (double x = m_xMin; x < m_xMax + 0.5*m_xRes; x += m_xRes)
    {
      for (double y = m_yMin; y < m_yMax + 0.5*m_yRes ; y += m_yRes)
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
          //doCalcRxPsd


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
  rrd.antenna = CreateObjectWithAttributes<ThreeGppAntennaArrayModel> ("NumColumns", UintegerValue (2), "NumRows", UintegerValue (2));
  //Configure Antenna
  rrd.antenna->ChangeToOmniTx ();


  // initialize the devices in the ThreeGppSpectrumPropagationLossModel
  m_spectrumLossModel->AddDevice (rtd.dev, rtd.antenna);
  m_spectrumLossModel->AddDevice (rrd.dev, rrd.antenna);

  //Bw, Freq and numerology should be passed for the simulation scenario?
  Ptr<const SpectrumModel> sm1 =  MmWaveSpectrumValueHelper::GetSpectrumModel (100e6, 28.0e9, 0);
  Ptr<const SpectrumValue> txPsd1 = MmWaveSpectrumValueHelper::CreateTxPowerSpectralDensity (40, sm1); //txPower?


  for (std::list<RemPoint>::iterator it = m_rem.begin ();
       it != m_rem.end ();
       ++it)
    {
      rrd.mob->SetPosition (it->pos);    //Assign to the rrd mobility all the positions of remPoint

      //Ptr<SpectrumValue> rxPsd1 = m_spectrumLossModel->DoCalcRxPowerSpectralDensity (txPsd1, rtd.mob, rrd.mob);
      //NS_LOG_UNCOND ("Average rx power 1: " << 10 * log10 (Sum (*rxPsd1) / rxPsd1->GetSpectrumModel ()->GetNumBands ()) << " dBm");
      //it->sinr = 10 * log10 (Sum (*rxPsd1) / rxPsd1->GetSpectrumModel ()->GetNumBands ());

      it->rssi = m_propagationLossModel->CalcRxPower(5, rtd.mob, rrd.mob);

      //It calculates (and doesn't crash, but I get always the same value :( )
    }

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
                << it->sinr << "\t"
                << it->rssi << "\t"
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
