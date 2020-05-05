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
#include <fstream>

namespace ns3 {

class Node;
class NetDevice;
class SpectrumChannel;
class MobilityModel;

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
     * \brief Sets max number of REM points to be calculated per iteration
     */
    void SetMaxPointsPerIt (uint32_t maxPointsPerIt);


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
     * \return Gets the value of the max number of REM points to be
     * calculated per iteration
     */
    uint32_t GetMaxPointsPerIt () const;

    /**
     * The method will divide the whole map into parts (each contains at most a
     * certain number of listening points), and then call RunOneIteration()
     * on each part, one by one.
     */
    void DelayedInstall ();

    /**
     * Mobilize all the listeners to a specified area. Afterwards, schedule a
     * call to PrintAndReset() in 0.5 milliseconds.
     *
     * \param xMin X coordinate of the first SINR listening point to deploy.
     * \param xMax X coordinate of the last SINR listening point to deploy.
     * \param yMin Y coordinate of the first SINR listening point to deploy.
     * \param yMax Y coordinate of the last SINR listening point to deploy.
     */
    void RunOneIteration (double xMin, double xMax, double yMin, double yMax);

    /// Go through every listener, write the computed SINR, and then reset it.
    void PrintAndReset ();

    /// Called when the map generation procedure has been completed.
    void Finalize ();


private:

    /// A complete Radio Environment Map is composed of many of this structure.
    struct RemPoint
    {
      /// Simplified listener which compute SINR over the DL channel.
      //Ptr<RemSpectrumPhy> phy;
      /// Position of the listener in the environment.
      Ptr<MobilityModel> bmm;
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

    uint32_t m_maxPointsPerIteration;  ///< The `MaxPointsPerIteration` attribute.

    //uint16_t m_earfcn;     ///< The `Earfcn` attribute.
    //uint16_t m_bandwidth;  ///< The `Bandwidth` attribute.

    double m_z;  ///< The `Z` attribute.

    std::string m_channelPath;  ///< The `ChannelPath` attribute.
    std::string m_outputFile;   ///< The `OutputFile` attribute.

    //bool m_stopWhenDone;   ///< The `StopWhenDone` attribute.

    /// The channel object taken from the `ChannelPath` attribute.
    //Ptr<SpectrumChannel> m_channel;

    //double m_noisePower;  ///< The `NoisePower` attribute.

    std::ofstream m_outFile;  ///< Stream the output to a file.

    //bool m_useDataChannel;  ///< The `UseDataChannel` attribute.
    //int32_t m_rbId;         ///< The `RbId` attribute.

}; // end of `class NrRadioEnvironmentMapHelper`

} // end of `namespace ns3`


#endif // NR_RADIO_ENVIRONMENT_MAP_HELPER_H
