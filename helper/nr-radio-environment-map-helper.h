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
#include "ns3/mmwave-enb-phy.h"
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
 * The rem value corresponds to the calculated SINR at this point.
 */
class NrRadioEnvironmentMapHelper : public Object
{
public:
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
     * This method
     */
    void CreateRem (NetDeviceContainer enbNetDev, uint8_t ccId);

    /**
     * This method creates the list of Rem Points
     */
    void CreateListOfRemPoints ();

private:
    /**
     * This method
     */
    void CalcRemValue ();

    /**
     * This method configures the REM Receiving Device
     */
    void ConfigureRrd ();

    /**
     * \brief Configure REM Transmission Devices List
     */
    void ConfigureRtdList (NetDeviceContainer enbNetDev, uint8_t ccId);

    /**
     * \brief Configure propagation loss models
     */
    void ConfigurePropagationModelsFactories (Ptr<const MmWaveEnbPhy> rtdPhy);

    /**
     * This method creates the temporal Propagation Models
     */
    void CreateTemporalPropagationModels ();

    /**
     * Print the position of the gNb.
     */
    void PrintGnuplottableEnbListToFile (std::string filename);

    /**
     * Print the position of the UE.
     */
    void PrintGnuplottableUeListToFile (std::string filename);

    /**
     * Go through every listener, write the computed SINR, and then reset it.
     */
    void PrintRemToFile ();

     /**
      * Called when the map generation procedure has been completed.
      * void Finalize ();
      */
    void Finalize ();

    struct RemDevice
    {
      Ptr<Node> node;
      Ptr<SimpleNetDevice> dev;
      Ptr<MobilityModel> mob;
      Ptr<ThreeGppAntennaArrayModel> antenna;
      double txPower {0}; // TODO just check if this is the good place, attribute is per RTD device
      double bandwidth {0}; // TODO Check if these three b,f,n maybe should be the parameters/attributes of RemHelper, because 1 REM map makes sense fore 1 configuration of channel,
      double frequency {0}; // TODO -||-   or maybe we don't need these three as parameter, maybe we can just save pointer to SpectrumModel so we can create txPsd when we need to do so.
      uint16_t numerology {0}; // TODO -||-
    };

    std::list<RemDevice> m_remDev;

    struct RemPoint
    {
      Vector pos {0,0,0};
      double sinrdB {0};
      double rssidBm {0};
      double avRssidBm {0};
      double snrdB {0};
      double avSnrdB {0};
    };

    /// List of listeners in the environment.
    std::list<RemPoint> m_rem;

    double m_xMin;   ///< The `XMin` attribute.
    double m_xMax;   ///< The `XMax` attribute.
    uint16_t m_xRes; ///< The `XRes` attribute.
    double m_xStep;  ///< Distance along X axis between adjacent listening points.

    double m_yMin;   ///< The `YMin` attribute.
    double m_yMax;   ///< The `YMax` attribute.
    uint16_t m_yRes; ///< The `YRes` attribute.
    double m_yStep;  ///< Distance along Y axis between adjacent listening points.

    //uint32_t m_maxPointsPerIteration;  ///< The `MaxPointsPerIteration` attribute.

    double m_z;  ///< The `Z` attribute.

    uint16_t m_numOfIterationsToAverage;

    RemDevice m_rrd;

    ObjectFactory m_propagationLossModelFactory;
    ObjectFactory m_spectrumLossModelFactory;
    ObjectFactory m_channelConditionModelFactory;

    Ptr<ThreeGppPropagationLossModel> m_remPropagationLossModelCopy;
    Ptr<ThreeGppSpectrumPropagationLossModel> m_remSpectrumLossModelCopy;
    Ptr<ChannelConditionModel> m_remCondModelCopy;

    Ptr<ThreeGppPropagationLossModel> m_propagationLossModel;
    Ptr<ThreeGppSpectrumPropagationLossModel> m_spectrumLossModel;
    //Ptr<ChannelConditionModel> m_condModel;

    std::string m_outputFile;   ///< The `OutputFile` attribute.

    /// The channel object taken from the `ChannelPath` attribute.

    std::ofstream m_outFile;  ///< Stream the output to a file.

}; // end of `class NrRadioEnvironmentMapHelper`

} // end of `namespace ns3`


#endif // NR_RADIO_ENVIRONMENT_MAP_HELPER_H
