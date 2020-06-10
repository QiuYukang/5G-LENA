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
#include "ns3/nr-spectrum-value-helper.h"
#include "ns3/beamforming-vector.h"
#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-ue-net-device.h"
#include <ns3/nr-spectrum-phy.h>
#include <ns3/spectrum-converter.h>
#include <ns3/buildings-module.h>
#include <ns3/mobility-building-info.h>
#include <ns3/beam-manager.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRadioEnvironmentMapHelper");

NS_OBJECT_ENSURE_REGISTERED (NrRadioEnvironmentMapHelper);

NrRadioEnvironmentMapHelper::NrRadioEnvironmentMapHelper ()
{
  NS_LOG_FUNCTION (this);
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
                      .AddAttribute ("SimTag",
                                     "simulation tag that will be concatenated to output file names"
                                     "in order to distinguish them, for example: NR_REM-${SimTag}.out. "
                                     "nr-ues-${SimTag}.txt, nr-gnbs-${SimTag}.txt, nr-buildings-${SimTag}.txt.",
                                      StringValue (""),
                                      MakeStringAccessor (&NrRadioEnvironmentMapHelper::SetSimTag),
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
                      .AddAttribute ("InstallationDelay",
                                     "How many time it is needed in the simulation to configure phy parameters at UE, "
                                     "depends on RRC message timing.",
                                     TimeValue (MilliSeconds (100)),
                                     MakeTimeAccessor (&NrRadioEnvironmentMapHelper::SetInstallationDelay),
                                     MakeTimeChecker())
    ;
  return tid;
}

void
NrRadioEnvironmentMapHelper::SetRemMode (enum RemMode remMode)
{
  m_remMode = remMode;
}

void
NrRadioEnvironmentMapHelper::SetSimTag (std::string simTag)
{
  m_simTag = simTag;
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

void
NrRadioEnvironmentMapHelper::SetInstallationDelay (Time installationDelay)
{
  m_installationDelay = installationDelay;
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
  NS_LOG_FUNCTION (this);
    // RTD and RRD devices should have the same spectrum model to perform calculation,
    // if some of the RTD devices is of the different model then its transmission will have to
    // converted into spectrum model of this device
    m_rrd.spectrumModel = ueDevice->GetObject<NrUeNetDevice> ()->GetPhy (bwpId)->GetSpectrumModel ();

    m_rrd.mob->SetPosition (ueDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ());


    std::ostringstream oss;
    oss << "nr-ues" << m_simTag.c_str() <<".txt";
    PrintGnuplottableUeListToFile (oss.str());

    Ptr<MobilityBuildingInfo> buildingInfo = CreateObject<MobilityBuildingInfo> ();
    m_rrd.mob->AggregateObject (buildingInfo);

    //Get Ue Phy
    Ptr<NrUeNetDevice> nrUeNetDev = ueDevice->GetObject<NrUeNetDevice> ();
    Ptr<const NrUePhy> rrdPhy = nrUeNetDev->GetPhy (bwpId);
    NS_ASSERT_MSG (rrdPhy, "rrdPhy is null");

    m_rrd.antenna = Copy (rrdPhy->GetAntennaArray ());

    m_noisePsd = NrSpectrumValueHelper::CreateNoisePowerSpectralDensity (rrdPhy->GetNoiseFigure (), m_rrd.spectrumModel);

    ConfigurePropagationModelsFactories (rrdPhy); // we can call only once configuration of prop.models

}

void NrRadioEnvironmentMapHelper::ConfigureRtdList (const NetDeviceContainer& rtdDevs, const Ptr<NetDevice>& rrdDev, uint8_t bwpId)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator netDevIt = rtdDevs.Begin ();
      netDevIt != rtdDevs.End ();
      ++netDevIt)
    {
      Ptr<NrGnbNetDevice> nrNetDev = (*netDevIt)->GetObject<NrGnbNetDevice> ();
      Ptr<const NrGnbPhy> rtdPhy = nrNetDev->GetPhy (bwpId);
      NS_ASSERT_MSG (rtdPhy, "rtdPhy is null");

      if (rtdPhy->GetSpectrumModel () != m_rrd.spectrumModel)
        {
          if (rtdPhy->GetSpectrumModel ()->IsOrthogonal (*m_rrd.spectrumModel))
            {
              NS_LOG_WARN ("RTD device is configured to operate on a spectrum "
                           "that is orthogonal to the one of RRD device. Hence, "
                           "that RTD device will not be considered in the "
                           "calculation of this REM map.");
              continue;
            }
          else
            {
              NS_LOG_WARN ("RTD device with different spectrum model, this may slow "
                           "down significantly the REM map creation. Consider setting "
                           "the same frequency, bandwidth, and numerology to all "
                           "devices which are used for REM map creation.");
            }
        };

      RemDevice rtd;
      //Configure spectrum model which will be needed to create tx PSD
      rtd.spectrumModel = rtdPhy->GetSpectrumModel ();
      rtd.mob->SetPosition ((*netDevIt)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ());
      Ptr<MobilityBuildingInfo> buildingInfo = CreateObject<MobilityBuildingInfo> ();
      rtd.mob->AggregateObject (buildingInfo);
      rtd.antenna = Copy (rtdPhy->GetAntennaArray ());
      // since we have delayed install maybe beamforming vector has changed in the RTD device
      // we want to save beamforming vector toward UE in the case that we will
      // need it for BEAM_SHAPE calculation
      rtd.antenna->SetBeamformingVector ((rtdPhy->GetBeamManager()->GetBeamformingVector(rrdDev)));
      //Configure power
      rtd.txPower = rtdPhy->GetTxPower ();

      NS_LOG_INFO ("RTD spectrum model: " << rtd.spectrumModel->GetUid () <<
                   ", RTD number of bands: " << rtd.spectrumModel->GetNumBands () <<
                   ", create new RTD element... " <<
                   ", rtdPhy->GetCentralFrequency () " << rtdPhy->GetCentralFrequency () / 1e6 << " MHz " <<
                   ", BW: " << rtdPhy->GetChannelBandwidth () / 1e6 << " MHz " <<
                   ", num: "<< rtdPhy->GetNumerology ());

      m_remDev.push_back (rtd);
    }
  NS_ASSERT_MSG(m_remDev.size(),"No RTD devices configured. Check if the RTD "
                                "devices are on the operating on the same "
                                "spectrum as RRD device.");
}

void
NrRadioEnvironmentMapHelper::ConfigurePropagationModelsFactories (const Ptr<const NrPhy>& rtdPhy)
{
  NS_LOG_FUNCTION (this);
  Ptr<const NrSpectrumPhy> txSpectrumPhy = rtdPhy->GetSpectrumPhy ();
  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel ();
  m_propagationLossModel = DynamicCast<ThreeGppPropagationLossModel> (txSpectrumChannel->GetPropagationLossModel ());

  m_spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (txSpectrumChannel->GetSpectrumPropagationLossModel ());
  m_channelConditionModel = m_propagationLossModel->GetChannelConditionModel() ;

  /***** configure pathloss model factory *****/
  ConfigureObjectFactory (m_propagationLossModelFactory, m_propagationLossModel);
  /***** configure spectrum model factory *****/
  ConfigureObjectFactory (m_spectrumLossModelFactory, m_spectrumLossModel);
  /***** configure channel condition model factory *****/
  ConfigureObjectFactory (m_channelConditionModelFactory, m_channelConditionModel);
}

void
NrRadioEnvironmentMapHelper::ConfigureObjectFactory (ObjectFactory& objectFactory, const Ptr<Object>& object)
{
  NS_LOG_FUNCTION (this);

  TypeId tid = object->GetInstanceTypeId ();
  objectFactory.SetTypeId (object->GetInstanceTypeId ());

  NS_LOG_DEBUG ("Configure object factory for:" << tid.GetName ());

  bool hasParent = false;
  do
    {
      for (size_t i = 0; i < tid.GetAttributeN (); i++)
        {
          ns3::TypeId::AttributeInformation attributeInfo = tid.GetAttribute (i);

          if (attributeInfo.checker->GetValueTypeName () == "ns3::PointerValue")
            {
              if (attributeInfo.name == "ChannelConditionModel")
                {
                  NS_LOG_INFO ("Skipping to copy ChannelConditionModel."
                               "According to REM design it should be created "
                               "as a new object (not copied).");
                }
              else
                {
                  NS_LOG_WARN ("This factory has a PointerValue attribute that "
                               "is not compatible with this REM helper version.");
                }

              continue;
            }

          // create initial attribute value to store attribute
          Ptr<AttributeValue> attributeValue = attributeInfo.checker->Create () ;
          // get the attribute value from the object
          object->GetAttribute(attributeInfo.name, *attributeValue);
          //skip pointer value attributes and warn if there is some new unexpceted attribute
          objectFactory.Set (attributeInfo.name, *attributeValue);
          //log current attribute name and value
          NS_LOG_DEBUG ("Copy attribute: " << attributeInfo.name << " value:" <<
                        attributeValue->SerializeToString (attributeInfo.checker));
        }

      if (tid.HasParent ())
        {
          tid = tid.GetParent ();
          hasParent = true;
        }
      else
        {
          hasParent = false;
        }
    }
  while (hasParent);
}

void
NrRadioEnvironmentMapHelper::CreateRem (NetDeviceContainer &gnbNetDev,
                                        Ptr<NetDevice> &ueDevice, uint8_t bwpId)
{
  NS_LOG_FUNCTION (this);

  Simulator::Schedule (m_installationDelay, &NrRadioEnvironmentMapHelper::DelayedInstall, this, gnbNetDev, ueDevice, bwpId );
}

void
NrRadioEnvironmentMapHelper::DelayedInstall (NetDeviceContainer &gnbNetDev,
                                         Ptr<NetDevice> &ueDevice, uint8_t bwpId)
{
  NS_LOG_FUNCTION (this);

  ConfigureRrd (ueDevice, bwpId);
  ConfigureRtdList (gnbNetDev, ueDevice, bwpId);
  CreateListOfRemPoints ();
  if (m_remMode == COVERAGE_AREA)
    {
      CalcCoverageAreaRemMap();
    }
  else if (m_remMode == BEAM_SHAPE)
    {
      CalcBeamShapeRemMap();
    }
  else
    {
      NS_FATAL_ERROR ("Unknown REM mode");
    }
  PrintRemToFile ();

  std::ostringstream ossGnbs;
  ossGnbs << "nr-gnbs" << m_simTag.c_str() <<".txt";
  PrintGnuplottableGnbListToFile (ossGnbs.str());
  std::ostringstream ossBuildings;
  ossBuildings << "nr-buildings" << m_simTag.c_str() <<".txt";
  PrintGnuplottableBuildingListToFile (ossBuildings.str());
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
          //In case a REM Point is in the same position as a rtd, ignore this point
          bool isPositionRtd = false;
          for (std::list<RemDevice>::iterator itRtd = m_remDev.begin (); itRtd != m_remDev.end (); ++itRtd)
          {
            if (itRtd->mob->GetPosition () == Vector (x, y, m_z))
            {
              isPositionRtd = true;
            }
          }

          if (!isPositionRtd)
          {
            RemPoint remPoint;

            remPoint.pos.x = x;
            remPoint.pos.y = y;
            remPoint.pos.z = m_z;

            m_rem.push_back (remPoint);
          }
        }
    }
}

void
NrRadioEnvironmentMapHelper::ConfigureQuasiOmniBfv (RemDevice& device)
{
  NS_LOG_FUNCTION (this);
  // configure beam on rrd antenna to be quasi-omni
  UintegerValue numRows, numColumns;
  device.antenna->GetAttribute ("NumRows", numRows);
  device.antenna->GetAttribute ("NumColumns", numColumns);
  // configure RRD antenna to have quasi omni beamforming vector
  device.antenna->SetBeamformingVector (CreateQuasiOmniBfv (numRows.Get (), numColumns.Get ()));
}

void
NrRadioEnvironmentMapHelper::ConfigureDirectPathBfv (RemDevice& device, const RemDevice& otherDevice, const Ptr<const ThreeGppAntennaArrayModel>& antenna)
{
  NS_LOG_FUNCTION (this);
  device.antenna->SetBeamformingVector (CreateDirectPathBfv (device.mob, otherDevice.mob, antenna));
}

Ptr<SpectrumValue>
NrRadioEnvironmentMapHelper::CalcRxPsdValue (RemDevice& itRtd)
{
  PropagationModels tempPropModels = CreateTemporalPropagationModels ();

  // initialize the devices in the ThreeGppSpectrumPropagationLossModel
  tempPropModels.remSpectrumLossModelCopy->AddDevice (itRtd.dev, itRtd.antenna);
  tempPropModels.remSpectrumLossModelCopy->AddDevice (m_rrd.dev, m_rrd.antenna);
  Ptr<const SpectrumValue> txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity (itRtd.txPower, itRtd.spectrumModel);

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

Ptr<SpectrumValue>
NrRadioEnvironmentMapHelper::GetMaxValue(const std::list <Ptr<SpectrumValue>>& values)
{
  //TODO add this abort, if necessary add include for abort.h
  NS_ABORT_MSG_IF (values.size () == 0, "Must provide a list of values.");

  Ptr<SpectrumValue> maxValue = *(values.begin ());

  for (const auto &value: values)
    {
      if (Sum (*(value)) > Sum (*(maxValue)))
        {
          *maxValue = *value;
        }
    }
  return maxValue;
}

double NrRadioEnvironmentMapHelper::CalculateMaxSnr (const std::list <Ptr<SpectrumValue>>& receivedPowerList)
{
  Ptr<SpectrumValue> maxSnr = GetMaxValue (receivedPowerList);
  SpectrumValue snr = (*maxSnr) / (*m_noisePsd);
  return 10 * log10 (Sum (snr) / snr.GetSpectrumModel ()->GetNumBands ());
}

double NrRadioEnvironmentMapHelper::CalculateSnr (const Ptr<SpectrumValue>& usefulSignal)
{
   SpectrumValue snr = (*usefulSignal) / (*m_noisePsd);
   return 10 * log10 (Sum (snr) / snr.GetSpectrumModel ()->GetNumBands ());
}

double
NrRadioEnvironmentMapHelper::CalculateSinr (const Ptr<SpectrumValue>& usefulSignal,
                                            const std::list <Ptr<SpectrumValue>>& interferenceSignals)
{
  Ptr<SpectrumValue> interferencePsd = nullptr;

  if (interferenceSignals.size () == 0)
    {
      return CalculateSnr (usefulSignal);
    }
  else
    {
      interferencePsd = Create<SpectrumValue> (m_rrd.spectrumModel);
    }

  // sum all interfering signals
  for (auto rxInterfPower: interferenceSignals)
    {
      *interferencePsd += (*rxInterfPower);
    }
  // calculate sinr

  SpectrumValue sinr = (*usefulSignal) / (*interferencePsd + *m_noisePsd) ;

  // calculate average sinr over RBs, convert it from linear to dB units, and return it
  return 10 * log10 (Sum (sinr) / sinr.GetSpectrumModel ()->GetNumBands ()) ;
}

double
NrRadioEnvironmentMapHelper::CalculateMaxSinr (const std::list <Ptr<SpectrumValue>>& receivedPowerList)
{
  // we calculate sinr considering for each RTD as if it would be TX device, and the rest of RTDs interferers
  std::list <double> sinrList;

  for (std::list <Ptr<SpectrumValue>>::const_iterator it = receivedPowerList.begin ();
        it!=receivedPowerList.end (); it++)
    {
      //all signals - rxPower = interference
      std::list <Ptr<SpectrumValue>> interferenceSignals;
      std::list <Ptr<SpectrumValue>>::const_iterator tempit = it;

      if (it!=receivedPowerList.begin ())
        {
          interferenceSignals.insert (interferenceSignals.begin (), receivedPowerList.begin (), it);
        }

      interferenceSignals.insert (interferenceSignals.end (), ++tempit, receivedPowerList.end ());
      NS_ASSERT(interferenceSignals.size () == receivedPowerList.size ()-1);
      sinrList.push_back (CalculateSinr (*it, interferenceSignals));
    }
  return GetMaxValue (sinrList);
}

void
NrRadioEnvironmentMapHelper::CalcBeamShapeRemMap ()
{
  NS_LOG_FUNCTION (this);
  //Save REM creation start time
  auto remStartTime = std::chrono::system_clock::now ();
  uint16_t calcRxPsdCounter = 0;

  for (std::list<RemPoint>::iterator itRemPoint = m_rem.begin ();
      itRemPoint != m_rem.end ();
      ++itRemPoint)
    {
      //perform calculation m_numOfIterationsToAverage times and get the average value
      double sumSnr = 0.0, sumSinr = 0.0;
      m_rrd.mob->SetPosition (itRemPoint->pos);

      Ptr <MobilityBuildingInfo> buildingInfo = m_rrd.mob->GetObject <MobilityBuildingInfo> ();
      buildingInfo->MakeConsistent (m_rrd.mob);
      NS_ASSERT_MSG (buildingInfo, "buildingInfo is null");

      for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
        {
          std::list <Ptr<SpectrumValue>> receivedPowerList;// RTD node id, rxPsd of the singal coming from that node

          for (std::list<RemDevice>::iterator itRtd = m_remDev.begin (); itRtd != m_remDev.end (); ++itRtd)
            {
              calcRxPsdCounter++;
               // calculate received power from the current RTD device
              receivedPowerList.push_back (CalcRxPsdValue (*itRtd));

              NS_LOG_UNCOND ("Done:" <<
                             (double)calcRxPsdCounter/(m_rem.size()*m_numOfIterationsToAverage*m_remDev.size ()) * 100 <<
                             " %."); // how many times will be called CalcRxPsdValues

            } //end for std::list<RemDev>::iterator  (RTDs)

          sumSnr += CalculateMaxSnr (receivedPowerList);
          sumSinr += CalculateMaxSinr (receivedPowerList);

          receivedPowerList.clear ();
        }//end for m_numOfIterationsToAverage  (Average)

      itRemPoint->avgSnrDb = sumSnr / static_cast <double> (m_numOfIterationsToAverage);
      itRemPoint->avgSinrDb = sumSinr / static_cast <double> (m_numOfIterationsToAverage);

      NS_LOG_INFO ("Avg snr value saved:"<<itRemPoint->avgSnrDb);
      NS_LOG_INFO ("Avg sinr value saved:"<<itRemPoint->avgSinrDb);
    } //end for std::list<RemPoint>::iterator  (RemPoints)

  auto remEndTime = std::chrono::system_clock::now ();
  std::chrono::duration<double> remElapsedSeconds = remEndTime - remStartTime;
  NS_LOG_UNCOND ("REM map created. Total time needed to create the REM map:" <<
                 remElapsedSeconds.count () / 60 << " minutes.");
}

double
NrRadioEnvironmentMapHelper::GetMaxValue (const std::list<double>& listOfValues) const
{
  NS_ABORT_MSG_IF (listOfValues.size () == 0, "GetMaxValue should not be called "
                                              "with an empty list.");

  double maxValue = *(listOfValues.begin ());
  //start from second element, the first is already taken into account
  for(auto it = ++listOfValues.begin (); it != listOfValues.end (); ++it)
    {
      if (*it > maxValue)
        {
          maxValue = *it;
        }
    }
  return maxValue;
}

void
NrRadioEnvironmentMapHelper::CalcCoverageAreaRemMap ()
{
  NS_LOG_FUNCTION (this);
  //Save REM creation start time
  auto remStartTime = std::chrono::system_clock::now ();
  uint16_t calcRxPsdCounter = 0;

  for (std::list<RemPoint>::iterator itRemPoint = m_rem.begin ();
      itRemPoint != m_rem.end ();
      ++itRemPoint)
    {
      //perform calculation m_numOfIterationsToAverage times and get the average value
      double sumSnr = 0.0, sumSinr = 0.0;
      m_rrd.mob->SetPosition (itRemPoint->pos);

      // all RTDs should point toward that RemPoint with DirectPah beam, this is definition of worst-case scenario
     for(std::list<RemDevice>::iterator itRtd = m_remDev.begin ();
         itRtd != m_remDev.end ();
                ++itRtd)
        {
          ConfigureDirectPathBfv (*itRtd, m_rrd, itRtd->antenna);
        }

      for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
        {
          std::list<double> sinrsPerBeam; // vector in which we will save sinr per each RRD beam
          std::list<double> snrsPerBeam; // vector in which we will save snr per each RRD beam

          // For each beam configuration at RemPoint/RRD we should calculate SINR, there are as many beam configurations at RemPoint as many RTDs
          for (std::list<RemDevice>::iterator itRtdBeam = m_remDev.begin (); itRtdBeam != m_remDev.end (); ++itRtdBeam)
            {
              //configure RRD beam toward RTD
              ConfigureDirectPathBfv (m_rrd, *itRtdBeam, m_rrd.antenna);

              std::list<Ptr<SpectrumValue>> interferenceSignalsRxPsds;
              Ptr<SpectrumValue> usefulSignalRxPsd;

              // For this configuration of beam at RRD, we need to calculate RX PSD,
              // and in order to be able to calculate SINR for that beam,
              // we need to calculate received PSD for each RTD using this beam at RRD
              for(std::list<RemDevice>::iterator itRtdCalc = m_remDev.begin (); itRtdCalc != m_remDev.end (); ++itRtdCalc)
                {
                  // increase counter de calcRXPsd calls
                  calcRxPsdCounter++;
                  // calculate received power from the current RTD device
                  Ptr<SpectrumValue> receivedPower = CalcRxPsdValue (*itRtdCalc);

                  // is this received power useful signal (from RTD for which I configured my beam) or is interference signal

                  if (itRtdBeam->dev->GetNode ()->GetId () == itRtdCalc->dev->GetNode ()->GetId ())
                    {
                      if (usefulSignalRxPsd != nullptr)
                        {
                          NS_FATAL_ERROR ("Already assigned usefulSignal!");
                        }
                      usefulSignalRxPsd = receivedPower;
                    }
                  else
                    {
                      interferenceSignalsRxPsds.push_back (receivedPower);  //interference
                    }
                }

              sinrsPerBeam.push_back (CalculateSinr (usefulSignalRxPsd, interferenceSignalsRxPsds));
              snrsPerBeam.push_back (CalculateSnr (usefulSignalRxPsd));

              NS_LOG_UNCOND ("Done:" <<
                             (double)calcRxPsdCounter/(m_rem.size()*m_numOfIterationsToAverage*m_remDev.size ()*m_remDev.size()) * 100 <<
                             " %."); // how many times will be called CalcRxPsdValues

            } //end for std::list<RemDev>::iterator  (RTDs)

          sumSnr += GetMaxValue (snrsPerBeam);
          sumSinr += GetMaxValue (sinrsPerBeam);

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
  NS_LOG_FUNCTION (this);

  PropagationModels propModels;
  //create rem copy of channel condition
  Ptr<ChannelConditionModel> condModelCopy = m_channelConditionModelFactory.Create<ChannelConditionModel> ();

  //create rem copy of propagation model
  propModels.remPropagationLossModelCopy = m_propagationLossModelFactory.Create <ThreeGppPropagationLossModel> ();
  propModels.remPropagationLossModelCopy->SetChannelConditionModel (condModelCopy);

  //create rem copy of spectrum loss model
  propModels.remSpectrumLossModelCopy = m_spectrumLossModelFactory.Create <ThreeGppSpectrumPropagationLossModel> ();
  propModels.remSpectrumLossModelCopy->SetChannelModelAttribute ("ChannelConditionModel", PointerValue (condModelCopy));

  // copy the rest of channel model attributes
  CopyThreeGppChannelModelAttributeValues (propModels.remSpectrumLossModelCopy);

  return propModels;
}


void
NrRadioEnvironmentMapHelper::CopyThreeGppChannelModelAttributeValues (Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel)
{
  NS_LOG_FUNCTION (this);

  for (size_t i = 0; i < ThreeGppChannelModel::GetTypeId ().GetAttributeN (); i++)
    {
      ns3::TypeId::AttributeInformation attributeInfo = ThreeGppChannelModel::GetTypeId ().GetAttribute (i);

      if (attributeInfo.checker->GetValueTypeName () == "ns3::PointerValue")
        {
          if (attributeInfo.name == "ChannelConditionModel")
            {
              NS_LOG_INFO ("Skipping to copy ChannelConditionModel. According to "
                           "REM design it should be created as a new object (not copied).");
            }
          else
            {
              NS_LOG_WARN ("ThreeGppChannelModel has a new pointer attribute that "
                           "is not compatible with this REM helper version.");
            }

          continue;
        }

      Ptr<AttributeValue> attributeValue = attributeInfo.checker->Create ();
      m_spectrumLossModel->GetChannelModelAttribute (attributeInfo.name, *attributeValue);
      spectrumLossModel->SetChannelModelAttribute (attributeInfo.name, *attributeValue);

      NS_LOG_DEBUG ("Copy attribute: " << attributeInfo.name << " value:" <<
                    attributeValue->SerializeToString (attributeInfo.checker));
    }
}

void
NrRadioEnvironmentMapHelper::PrintGnuplottableGnbListToFile (std::string filename)
{
  NS_LOG_FUNCTION (this);
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
                    " left font \"Helvetica,4\" textcolor rgb \"white\" front  point pt 2 ps 1 lc rgb \"white\" offset 0,0" <<
                    std::endl;
    }

  gnbOutFile.close ();
}

void
NrRadioEnvironmentMapHelper::PrintGnuplottableUeListToFile (std::string filename)
{
  NS_LOG_FUNCTION (this);
  std::ofstream ueOutFile;
  ueOutFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!ueOutFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  Vector pos = m_rrd.node->GetObject<MobilityModel> ()->GetPosition ();

  ueOutFile << "set label \"" << m_rrd.dev->GetNode ()->GetId () <<
               "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,4\" textcolor rgb \"grey\" front point pt 1 ps 1 lc rgb \"grey\" offset 0,0" <<
               std::endl;

  ueOutFile.close ();
}

void
NrRadioEnvironmentMapHelper::PrintGnuplottableBuildingListToFile (std::string filename)
{
  NS_LOG_FUNCTION (this);
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  uint32_t index = 0;

  for (BuildingList::Iterator it = BuildingList::Begin (); it != BuildingList::End (); ++it)
    {
      ++index;
      Box box = (*it)->GetBoundaries ();
      outFile << "set object " << index << " rect from " <<
                 box.xMin  << "," << box.yMin << " to " <<
                 box.xMax  << "," << box.yMax <<
                 " front fs empty " << " border 3 " <<
                 std::endl;
    }

  outFile.close ();
}

void
NrRadioEnvironmentMapHelper::PrintRemToFile ()
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;
  oss << "NR_REM" << m_simTag.c_str() <<".out";

  std::ofstream outFile;
  std::string outputFile = oss.str ();
  outFile.open (outputFile.c_str ());

  if (!outFile.is_open ())
      {
        NS_FATAL_ERROR ("Can't open file " << (outputFile));
        return;
      }

  for (std::list<RemPoint>::iterator it = m_rem.begin ();
       it != m_rem.end ();
       ++it)
    {
      outFile << it->pos.x << "\t"
                << it->pos.y << "\t"
                << it->pos.z << "\t"
                << it->avgSnrDb << "\t"
                << it->avgSinrDb << "\t"
                << std::endl;
    }

  outFile.close();

  if (!m_simTag.empty())
    {
      CreateCustomGnuplotFile ();
    }

  Finalize ();
}

void
NrRadioEnvironmentMapHelper::CreateCustomGnuplotFile ()
{
  NS_LOG_FUNCTION (this);
  std::ostringstream oss;
  oss << "plot_rem" << m_simTag.c_str() <<".gnuplot";
  std::string filename = oss.str ();

  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
    if (!outFile.is_open ())
      {
        NS_LOG_ERROR ("Can't open file " << filename);
        return;
      }

  outFile<<"set xlabel \"x-coordinate (m)\""<<std::endl;
  outFile<<"set ylabel \"y-coordinate (m)\""<<std::endl;
  outFile<<"set cblabel \"SNR (dB)\""<<std::endl;
  outFile<<"unset key"<<std::endl;
  outFile<<"set term postscript eps color"<<std::endl;
  outFile<<"set output \"rem-snr"<<m_simTag<<".eps\""<<std::endl;
  outFile<<"set size ratio -1"<<std::endl;
  outFile<<"set cbrange [-5:30]"<<std::endl;
  outFile<<"set xrange ["<<m_xMin<<":"<<m_xMax<<"]"<<std::endl;
  outFile<<"set yrange ["<<m_yMin<<":"<<m_yMax<<"]"<<std::endl;
  outFile<<"plot \"NR_REM"<<m_simTag<<".out\" using ($1):($2):($4) with image"<<std::endl;

  outFile<<"set xlabel \"x-coordinate (m)\""<<std::endl;
  outFile<<"set ylabel \"y-coordinate (m)\""<<std::endl;
  outFile<<"set cblabel \"SINR (dB)\""<<std::endl;
  outFile<<"unset key"<<std::endl;
  outFile<<"set term postscript eps color"<<std::endl;
  outFile<<"set output \"rem-sinr"<<m_simTag<<".eps\""<<std::endl;
  outFile<<"set size ratio -1"<<std::endl;
  outFile<<"set cbrange [-5:30]"<<std::endl;
  outFile<<"set xrange ["<<m_xMin<<":"<<m_xMax<<"]"<<std::endl;
  outFile<<"set yrange ["<<m_yMin<<":"<<m_yMax<<"]"<<std::endl;
  outFile<<"plot \"NR_REM"<<m_simTag<<".out\" using ($1):($2):($5) with image"<<std::endl;

  outFile.close ();
}


void
NrRadioEnvironmentMapHelper::Finalize ()
{
  NS_LOG_FUNCTION (this);
  // TODO test if we can call this  
  //system("gnuplot -p nr-ues.txt nr-gnbs.txt buildings.txt plot_rem.gnuplot");
  // TODO if yes, then we can add also convert command
  Simulator::Stop ();
}


} // namespace ns3
