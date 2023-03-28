/*
 * Copyright (c) 2022 CTTC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Biljana Bojovic <bbojovic@cttc.es>
 */

#ifndef TRAFFIC_GENERATOR_NGMN_FTP_MULTI
#define TRAFFIC_GENERATOR_NGMN_FTP_MULTI

#include "traffic-generator.h"

#include <ns3/random-variable-stream.h>

namespace ns3
{

class Address;
class Socket;

/**
 * File transfer application that allow sending multiple files in a row
 * where each file is of a variable file size with a variable reading time.
 * Current implementation follows the FTP model explained in the Annex A of
 * White Paper by the NGMN Alliance.
 *
 * An FTP session is a sequence of file transfers separated by reading times.
 * The two main FTP session parameters are:
 *      - The size S of a file to be transferred
 *      - The reading time D, i.e. the time interval between end of download
 *      of previous file and the user request for the next file
 *
 * The file size follows Truncated Lognormal Distribution, while the
 * reading time follows Exponential Distribution.
 */

class TrafficGeneratorNgmnFtpTestCase;

class TrafficGeneratorNgmnFtpMulti : public TrafficGenerator
{
    friend TrafficGeneratorNgmnFtpTestCase;

  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /*
     * \brief Constructor
     */
    TrafficGeneratorNgmnFtpMulti();
    /*
     * \brief Destructor
     */
    ~TrafficGeneratorNgmnFtpMulti() override;
    /**
     * \brief Sets the packet size
     */
    void SetPacketSize(uint32_t packetSize);

  protected:
    // inherited
    void DoDispose() override;
    // inherited
    void DoInitialize() override;

  private:
    // inherited from Application base class.
    // Called at time specified by Start by the DoInitialize method
    void StartApplication() override;
    /**
     * \brief Generates reading time using exponential distribution
     *
     */
    void PacketBurstSent() override;
    /**
     * \brief Generate the next file size to transfer
     */
    void GenerateNextPacketBurstSize() override;
    /**
     * \brief Get next reading time
     * \return the next reading time
     */
    Time GetNextReadingTime();
    /**
     * \brief Get the amount of data to transfer
     * \return the amount of data to transfer
     */
    uint32_t GetNextPacketSize() const override;

    uint32_t m_maxFileSize;                       //!< Max file size in number of bytes
    Ptr<ExponentialRandomVariable> m_readingTime; //!< Exponential random variable for reading time
    Ptr<LogNormalRandomVariable> m_fileSize; //!< Lognormal random variable for file size generation
    double m_readingTimeMean{0.0};           //!< The mean reading time in seconds
    double m_fileSizeMu{0.0}; //!< Mu parameter of lognormal distribution for file size generation
    double m_fileSizeSigma{
        0.0}; //!< Sigma parameter of lognormal distribution for file size generation
    uint32_t m_packetSize{0}; //!< Size of data to send each time
};

} // namespace ns3

#endif /* TRAFFIC_GENERATOR_FTP_MULTI */
