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

#ifndef NR_RADIO_ENVIRONMENT_MAP_HELPER_H
#define NR_RADIO_ENVIRONMENT_MAP_HELPER_H

#include <ns3/object.h>
#include <ns3/object-factory.h>
#include "ns3/simple-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/nr-gnb-phy.h"
#include "ns3/nr-ue-phy.h"
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <fstream>

namespace ns3 {

class Node;
class NetDevice;
class SpectrumChannel;
class MobilityModel;
class ChannelConditionModel;
class ThreeGppAntennaArrayModel;

/**
 * \brief Generate a radio environment map
 *
 * The purpose of the radio environment map helper is to generate a
 * map where for each point on the map a rem value is calculated.
 *
 * Two general types of maps can be generated according to whether the
 * BeamShape or CoverageArea is selected. The gNB and Ue antennas have
 * to be configured accordingly depending on the map the user wishes
 * to generate.
 * The rem value corresponds to the calculated SNR/SINR at this point.
 *
 * Let us notice that for the SNR/SINR calculations at each REM Point the
 * channel is re-created to avoid spatial and temporal dependencies among
 * independent REM calculations. Moreover, the calculations are the average of
 * N iterations (specified by the user) in order to consider the randomness of
 * the channel
 */
class NrRadioEnvironmentMapHelper : public Object
{
public:

  enum RemMode {
         BEAM_SHAPE,
         COVERAGE_AREA
  };

  /**
   * \brief NrRadioEnvironmentMapHelper constructor
   */
  NrRadioEnvironmentMapHelper ();

  /**
   * \brief destructor
   */
  virtual ~NrRadioEnvironmentMapHelper ();


  // inherited from Object
  virtual void DoDispose (void);


  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  //TODO
  void SetRemMode (enum RemMode remType);

  /**
   * \brief Sets the min x coordinate of the map
   */
  void SetMinX (double xMin);

  /**
   * \brief Sets the min y coordinate of the map
   */
  void SetMinY (double yMin);

  /**
   * \brief Sets the max x coordinate of the map
   */
  void SetMaxX (double xMax);

  /**
   * \brief Sets the max y coordinate of the map
   */
  void SetMaxY (double yMax);

  /**
   * \brief Sets the resolution (number of points)
   * of the map along the x axis
   */
  void SetResX (uint16_t xRes);

  /**
   * \brief Sets the resolution (number of points)
   * of the map along the y axis
   */
  void SetResY (uint16_t yRes);

  /**
   * \brief Sets the z coordinate of the map
   */
  void SetZ (double z);

  /**
   * \brief Sets the number of iterations to
   * calculated the average of rem value
   */
  void SetNumOfItToAverage (uint16_t numOfIterationsToAverage);

  /*
   * \brief Sets the installation delay
   * \param installationDelay delay for the REM installation
   */
  void SetInstallationDelay (Time installationDelay);

  //TODO
  enum RemMode GetRemMode () const;

  /**
   * \return Gets the value of the min x coordinate of the map
   */
  double GetMinX () const;

  /**
   * \return Gets the value of the min y coordinate of the map
   */
  double GetMinY () const;

  /**
   * \return Gets the value of the max x coordinate of the map
   */
  double GetMaxX () const;

  /**
   * \return Gets the value of the max y coordinate of the map
   */
  double GetMaxY () const;

  /**
   * \return Gets the value of the resolution (number of points)
   * of the map along the x axis
   */
  uint16_t GetResX () const;

  /**
   * \return Gets the value of the resolution (number of points)
   * of the map along the y axis
   */
  uint16_t GetResY () const;

  /**
   * \return Gets the value of the z coordinate of the map
   */
  double GetZ () const;

  /**
   * \brief This function is used for the creation of the REM map. When this
   * function is called from an example, it is responsible for calling all the
   * necessary functions for the configuration of RTDs and RRDs and the
   * calculation of SNR/SINR.
   * \param gnbNetDev gNb devices for which the map will be generated
   * \param ueDevice The Ue device for which the map will be generated
   * \param bwpId The bwpId
   */
  void CreateRem (NetDeviceContainer gnbNetDev, Ptr<NetDevice> &ueDevice, uint8_t bwpId);

  /**
   * \brief This method creates the list of Rem Points (coordinates)
   */
  void CreateListOfRemPoints ();

private:

  /**
   * \brief This struct includes the coordinates of each Rem Point
   * and the SNR/SINR values as resulted for the calculations
   */
  struct RemPoint
  {
    Vector pos {0,0,0};
    double avgSnrDb {0};
    double avgSinrDb {0};
  };

  /**
   * \brief This struct includes the configuration of all the devices of
   * the REM: Rem Transmitting Devices (RTDs) and Rem Receiving Devices (RRDs)
   */
  struct RemDevice
  {
    Ptr<Node> node;
    Ptr<SimpleNetDevice> dev;
    Ptr<MobilityModel> mob;
    Ptr<ThreeGppAntennaArrayModel> antenna;
    double txPower {0};
    double bandwidth {0};
    double frequency {0};
    uint16_t numerology {0};
    Ptr<const SpectrumModel> spectrumModel {};

    RemDevice ()
    {
      node = CreateObject<Node> ();
      dev = CreateObject<SimpleNetDevice> ();
      node->AddDevice (dev);
      MobilityHelper mobility;
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (node);

      mob = node->GetObject<MobilityModel> ();  //TODO BB: I think that we can remove mob attribute from RemDevice structure,
                                                //         because when we need it we can easily
                                                //         obtain it as you did here: rtd.node->GetObject<MobilityModel> ()
    }
  };

  /**
   * \brief This struct includes the pointers that copy the propagation
   * Loss Model and Spectrum Propagation Loss model (form the example used
   * to generate the REM map)
   */
  struct PropagationModels
  {
    Ptr<ThreeGppPropagationLossModel> remPropagationLossModelCopy;
    Ptr<ThreeGppSpectrumPropagationLossModel> remSpectrumLossModelCopy;
  };

  /*
   * \brief This function is used to performed a delayed installation of REM map
   * \param gnbNetDev gNb devices for which the map will be generated
   * \param ueDevice The Ue device for which the map will be generated
   * \param bwpId The bwpId
   */
   void InstallRem (NetDeviceContainer gnbNetDev, Ptr<NetDevice> &ueDevice, uint8_t bwpId);

  /**
   * \brief This function is used for the creation of the REM map. When this
   * function is called from an example, it is responsible for calling all the
   * necessary functions for the configuration of RTDs and RRDs and the
   * calculation of SNR/SINR.
   * \param gnbNetDev gNb devices for which the map will be generated
   * \param ueDevice The Ue device for which the map will be generated
   * \return bwpId The bwpId
   */
  double CalculateSnr (const Ptr<SpectrumValue>& usefulSignal);

  Ptr<SpectrumValue> GetMaxValue(const std::list <Ptr<SpectrumValue>>& values);

  //TODO
  double CalculateMaxSnr (const std::list <Ptr<SpectrumValue>>& receivedPowerList);

  //TODO
  double CalculateMaxSinr (const std::list <Ptr<SpectrumValue>>& receivedPowerList);

  //For the given list of receivedPowerList calculates SINR
  //TODO
  double CalculateSinr (const Ptr<SpectrumValue>& usefulSignal, const std::list <Ptr<SpectrumValue>>& receivedPowerList);

  void CalculateRemValues (RemPoint* remPoint);
  /**
   * This method
   */
  void CalcBeamShapeRemMap ();

  double GetMaxValue (const std::list<double>& listOfValues) const;


  void CalcCoverageAreaRemMap ();

  /**
   * T\brief Configures the REM Receiving Device (RRD)
   */
  void ConfigureRrd (Ptr<NetDevice> &ueDevice, uint8_t bwpId);

  /**
   * \brief Configure REM Transmission Devices (RTDs) List
   */
  void ConfigureRtdList (NetDeviceContainer gnbNetDev, uint8_t bwpId);

  /**
   * \brief Configure propagation loss model factories
   */
  void ConfigurePropagationModelsFactories (Ptr<const NrGnbPhy> rtdPhy);

  //TODO
  void ConfigureObjectFactory (ObjectFactory& objectFactory, Ptr<Object> object);

  /**
   * \brief This method creates the temporal Propagation Models
   */
  PropagationModels CreateTemporalPropagationModels ();

  //TODO
  void CopyThreeGppChannelModelAttributeValues (Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel);

  /**
   * \brief Prints the position of the gNbs.
   */
  void PrintGnuplottableGnbListToFile (std::string filename);

  /**
   * \brief Print the position of the UE.
   */
  void PrintGnuplottableUeListToFile (std::string filename);

  /**
   * \brief Print the position of the Buildings.
   */
  void PrintGnuplottableBuildingListToFile (std::string filename);

  /**
   * \brief this method goes through every Rem Point and prints the
   * calculated SNR/SINR values.
   */
  void PrintRemToFile ();

  /**
   * Called when the map generation procedure has been completed.
   * void Finalize ();
   */
  void Finalize ();

  /**
   * \brief Configures quasi-omni beamforming vector on antenna of the device
   * \param device which antenna array will be configured to quasi-omni beamforming vector
   */
  void ConfigureQuasiOmniBfv (RemDevice& device);

  /**
   * \brief Configures direct-path beamforming vector of "device" toward "otherDevice"
   * \param device whose beamforming vector will be configured
   * \param otherDevice toward this device will be configured the beamforming vector of device
   */
  void ConfigureDirectPathBfv (RemDevice& device, const RemDevice& otherDevice);

  Ptr<SpectrumValue> CalcRxPsdValue (RemPoint& itRemPoint,RemDevice& itRtd);

  std::list<RemDevice> m_remDev; ///< List of REM Transmiting Devices (RTDs).
  std::list<RemPoint> m_rem; ///< List of REM points.

  enum RemMode m_remMode; //

  double m_xMin {0};   ///< The `XMin` attribute.
  double m_xMax {0};   ///< The `XMax` attribute.
  uint16_t m_xRes {0}; ///< The `XRes` attribute.
  double m_xStep {0};  ///< Distance along X axis between adjacent listening points.

  double m_yMin {0};   ///< The `YMin` attribute.
  double m_yMax {0};   ///< The `YMax` attribute.
  uint16_t m_yRes {0}; ///< The `YRes` attribute.
  double m_yStep {0};  ///< Distance along Y axis between adjacent listening points.
  double m_z {0};  ///< The `Z` attribute.

  uint16_t m_numOfIterationsToAverage;
  Time m_installationDelay {Seconds(0)};

  RemDevice m_rrd;

  ObjectFactory m_propagationLossModelFactory;
  ObjectFactory m_spectrumLossModelFactory;
  ObjectFactory m_channelConditionModelFactory;

  Ptr<ThreeGppPropagationLossModel> m_propagationLossModel;
  Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLossModel;
  Ptr<ChannelConditionModel> m_channelConditionModel;
  Ptr<SpectrumValue> m_noisePsd; // noise figure PSD that will be used for calculations

  std::string m_outputFile;   ///< The `OutputFile` attribute.
  std::ofstream m_outFile;  ///< Stream the output to a file.

}; // end of `class NrRadioEnvironmentMapHelper`

} // end of `namespace ns3`


#endif // NR_RADIO_ENVIRONMENT_MAP_HELPER_H
