// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo  <nbaldo@cttc.es>
//
// Ported from LteEpcTft

#ifndef NR_QOS_RULE_H
#define NR_QOS_RULE_H

#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/simple-ref-count.h"

#include <list>

namespace ns3
{

/**
 * This class implements the model for a 5G NR QoS rule
 * which is the set of all packet filters associated with a
 * data radio bearer, as well as selected QoS parameters
 */
class NrQosRule : public SimpleRefCount<NrQosRule>
{
  public:
    /**
     * creates a QoS rule matching any traffic
     *
     * @return a newly created QoS rule that will match any traffic
     */
    static Ptr<NrQosRule> Default();

    /**
     * Indicates the direction of the traffic that is to be classified.
     */
    enum Direction
    {
        DOWNLINK = 1,
        UPLINK = 2,
        BIDIRECTIONAL = 3
    };

    /**
     * Implement the data structure representing a QoS rule packet filter.
     * This was originally designed for 4G LTE (see 3GPP TS 24.008 version
     * 8.7.0 Release 8, Table 10.5.162/3GPP TS  24.008: Traffic flow template
     * information element) but it should generally align with the 5G NR
     * equivalent (3GPP TS 24.501, Section 9.11.4.13 QoS rules).
     *
     * With respect to the packet filter specification in the above doc,
     * the following features are NOT supported:
     *  - IPSec filtering
     */
    struct PacketFilter
    {
        PacketFilter();

        /**
         *
         * @param d the direction
         * @param ra the remote address
         * @param la the local address
         * @param rp the remote port
         * @param lp the local port
         * @param tos the type of service
         *
         * @return true if the parameters match with the PacketFilter,
         * false otherwise.
         */
        bool Matches(Direction d,
                     Ipv4Address ra,
                     Ipv4Address la,
                     uint16_t rp,
                     uint16_t lp,
                     uint8_t tos);

        /**
         *
         * @param d the direction
         * @param ra the remote address
         * @param la the local address
         * @param rp the remote port
         * @param lp the local port
         * @param tos the type of service
         *
         * @return true if the parameters match with the PacketFilter,
         * false otherwise.
         */
        bool Matches(Direction d,
                     Ipv6Address ra,
                     Ipv6Address la,
                     uint16_t rp,
                     uint16_t lp,
                     uint8_t tos);

        /// Used to specify the precedence for the packet filter among all packet filters in the
        /// QoS rule; higher values will be evaluated last.
        uint8_t precedence;

        /// Whether the filter needs to be applied to uplink / downlink only, or in both cases
        Direction direction;

        Ipv4Address remoteAddress; //!< IPv4 address of the remote host
        Ipv4Mask remoteMask;       //!< IPv4 address mask of the remote host
        Ipv4Address localAddress;  //!< IPv4 address of the UE
        Ipv4Mask localMask;        //!< IPv4 address mask of the UE

        Ipv6Address remoteIpv6Address; //!< IPv6 address of the remote host
        Ipv6Prefix remoteIpv6Prefix;   //!< IPv6 address prefix of the remote host
        Ipv6Address localIpv6Address;  //!< IPv6 address of the UE
        Ipv6Prefix localIpv6Prefix;    //!< IPv6 address prefix of the UE

        uint16_t remotePortStart; //!< start of the port number range of the remote host
        uint16_t remotePortEnd;   //!< end of the port number range of the remote host
        uint16_t localPortStart;  //!< start of the port number range of the UE
        uint16_t localPortEnd;    //!< end of the port number range of the UE

        uint8_t typeOfService;     //!< type of service field
        uint8_t typeOfServiceMask; //!< type of service field mask
    };

    NrQosRule();

    /**
     * add a PacketFilter to the QosRule
     *
     * @param f the PacketFilter to be added
     *
     * @return the id( 0 <= id < 16) of the newly added filter, if the addition was successful. Will
     * fail if you try to add more than 16 filters, due to a legacy constraint from TS 24.008.
     */
    uint8_t Add(PacketFilter f);

    /**
     *
     * @param direction
     * @param remoteAddress
     * @param localAddress
     * @param remotePort
     * @param localPort
     * @param typeOfService
     *
     * @return true if any PacketFilter in the QoS rule matches with the
     * parameters, false otherwise.
     */
    bool Matches(Direction direction,
                 Ipv4Address remoteAddress,
                 Ipv4Address localAddress,
                 uint16_t remotePort,
                 uint16_t localPort,
                 uint8_t typeOfService);

    /**
     *
     * @param direction
     * @param remoteAddress
     * @param localAddress
     * @param remotePort
     * @param localPort
     * @param typeOfService
     *
     * @return true if any PacketFilter in the QoS rule matches with the
     * parameters, false otherwise.
     */
    bool Matches(Direction direction,
                 Ipv6Address remoteAddress,
                 Ipv6Address localAddress,
                 uint16_t remotePort,
                 uint16_t localPort,
                 uint8_t typeOfService);

    /**
     * Get the packet filters
     * @return a container of packet filters
     */
    std::list<PacketFilter> GetPacketFilters() const;

  private:
    std::list<PacketFilter> m_filters; ///< packet filter list
    uint8_t m_numFilters;              ///< number of packet filters applied to this QoS rule
};

std::ostream& operator<<(std::ostream& os, const NrQosRule::Direction& d);

} // namespace ns3

#endif /* NR_QOS_RULE_H */
