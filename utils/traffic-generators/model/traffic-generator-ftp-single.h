/*
 * Copyright (c) 2010 Georgia Institute of Technology
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
 * Author: George F. Riley <riley@ece.gatech.edu>
 * Edited by: Biljana Bojovic <bbojovic@cttc.es>
 * Based on the file-transfer-application.h/cc whose original author was George F. Riley
 * <riley@ece.gatech.edu> Extended to support different traffic types: Biljana Bojovic
 * <bbojovic@cttc.es>
 */

#ifndef TRAFFIC_GENERATOR_FTP_SINGLE_H
#define TRAFFIC_GENERATOR_FTP_SINGLE_H

#include "traffic-generator.h"

namespace ns3
{

/**
 * File transfer application used to send a single file
 */

class TrafficGeneratorFtpSingle : public TrafficGenerator
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    TrafficGeneratorFtpSingle();

    ~TrafficGeneratorFtpSingle() override;

    /**
     * \brief Set the file size to try to transfer
     *
     * \param fileSize the size of a file to try to transfer
     */
    void SetFileSize(uint32_t fileSize);
    /**
     * \brief Sets the packet size
     */
    void SetPacketSize(uint32_t packetSize);

  protected:
    // overrides
    void DoDispose() override;
    void DoInitialize() override;

  private:
    /**
     * \brief Sets the file size to transfer
     */
    void GenerateNextPacketBurstSize() override;
    /**
     * \brief Get the amount of data to transfer
     * \return the amount of data to transfer
     */
    uint32_t GetNextPacketSize() const override;

    uint32_t m_fileSize{0};   //!< Limit total number of bytes sent
    uint32_t m_packetSize{0}; //!< Size of data to send each time
};

} // namespace ns3

#endif /* TRAFFIC_GENERATOR_FTP_SINGLE_H */
