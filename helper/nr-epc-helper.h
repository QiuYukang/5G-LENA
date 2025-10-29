// Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Authors:
//   Jaume Nin <jnin@cttc.es>
//   Nicola Baldo <nbaldo@cttc.es>
//   Manuel Requena <manuel.requena@cttc.es>

#ifndef NR_EPC_HELPER_H
#define NR_EPC_HELPER_H

#include "ns3/data-rate.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/node-container.h"
#include "ns3/nr-eps-bearer.h"
#include "ns3/nr-qos-rule.h"
#include "ns3/object.h"

#include <optional>

namespace ns3
{

class Node;
class NetDevice;
class VirtualNetDevice;
class NrEpcX2;

/**
 * @ingroup nr
 *
 * @brief Base helper class to handle the creation of the EPC entities.
 *
 * This class provides the API for the implementation of helpers that
 * allow to create EPC entities and the nodes and interfaces that host
 * and connect them.
 */
class NrEpcHelper : public Object
{
  public:
    /**
     * Constructor
     */
    NrEpcHelper();

    /**
     * Destructor
     */
    ~NrEpcHelper() override;

    // inherited from Object
    /**
     * Register this type.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();
    void DoDispose() override;

    /**
     * Add an gNB to the EPC
     *
     * @param gnbNode the previously created gNB node which is to be
     * added to the EPC
     * @param nrGnbNetDevice the NrGnbNetDevice of the gNB node
     * @param cellIds IDs of cells served by this gNB
     */
    virtual void AddGnb(Ptr<Node> gnbNode, Ptr<NetDevice> nrGnbNetDevice, uint16_t cellId) = 0;

    /**
     * Notify the EPC of the existence of a new UE which might attach at a later time
     *
     * @param ueNrDevice the UE device to be attached
     * @param imsi the unique identifier of the UE
     */
    virtual void AddUe(Ptr<NetDevice> ueNrDevice, uint64_t imsi) = 0;

    /**
     * Add an X2 interface between two gNB
     *
     * @param gnbNode1 one gNB peer of the X2 interface
     * @param gnbNode2 the other gNB peer of the X2 interface
     */
    virtual void AddX2Interface(Ptr<Node> gnbNode1, Ptr<Node> gnbNode2) = 0;

    /**
     * Add an S1 interface between an gNB and a SGW
     *
     * @param gnb gNB peer of the S1 interface
     * @param gnbAddress gNB IPv4 address of the S1 interface
     * @param sgwAddress SGW IPv4 address of the S1 interface
     * @param cellIds cellIds of the gNB
     */
    virtual void AddS1Interface(Ptr<Node> gnb,
                                Ipv4Address gnbAddress,
                                Ipv4Address sgwAddress,
                                uint16_t cellId) = 0;

    /**
     * Activate an EPS bearer, setting up the corresponding S1-U tunnel.
     *
     *
     *
     * @param ueNrDevice the Ipv4-enabled device of the UE, normally
     * connected via the NR radio interface
     * @param imsi the unique identifier of the UE
     * @param rule the QoS rule of the new bearer
     * @param bearer struct describing the characteristics of the EPS bearer to be activated
     * @return bearer ID
     */
    virtual uint8_t ActivateEpsBearer(Ptr<NetDevice> ueNrDevice,
                                      uint64_t imsi,
                                      Ptr<NrQosRule> rule,
                                      NrEpsBearer bearer) = 0;

    /**
     * Get the SGW node
     *
     * @return a pointer to the SGW
     */
    virtual Ptr<Node> GetSgwNode() const = 0;

    /**
     * Get the PGW node
     *
     * @return a pointer to the PGW node
     * The primary use intended for this method is to allow the user to configure the SGi
     * interface of the PGW, i.e., to connect the PGW to the internet.
     */
    virtual Ptr<Node> GetPgwNode() const = 0;

    /**
     * Assign IPv4 addresses to UE devices
     *
     * @param ueDevices the set of UE devices
     *
     * @return the interface container, \see Ipv4AddressHelper::Assign() which has similar semantics
     */
    virtual Ipv4InterfaceContainer AssignUeIpv4Address(NetDeviceContainer ueDevices) = 0;

    /**
     * Assign IPv6 addresses to UE devices
     *
     * @param ueDevices the set of UE devices
     *
     * @return the interface container, \see Ipv6AddressHelper::Assign() which has similar semantics
     */
    virtual Ipv6InterfaceContainer AssignUeIpv6Address(NetDeviceContainer ueDevices) = 0;

    /**
     *
     * @return the IPv4 address of the Default Gateway to be used by UEs to reach the internet
     */
    virtual Ipv4Address GetUeDefaultGatewayAddress() = 0;

    /**
     *
     * @return the IPv6 address of the Default Gateway to be used by UEs to reach the internet
     */
    virtual Ipv6Address GetUeDefaultGatewayAddress6() = 0;

    /**
     * Assign a fixed random variable stream number to the random variables used.
     *
     * @param stream first stream index to use
     * @return the number of stream indices (possibly zero) that have been assigned
     */
    virtual int64_t AssignStreams(int64_t stream) = 0;

    /**
     * Setup a P2P link connecting the PGW to a remote host, and create a route
     * from the remote host to the UEs using IPv4 networks.
     * Addresses are hardcoded for simplicity.
     *
     * RemoteHost --P2P-- PGW ----- SGW -- gNB -- UEs
     * 1.0.0.2            1.0.0.1
     *                    7.0.0.1                 7.0.0.0/8
     *
     * @param dataRateValue Link speed connecting the PGW and the remote host
     * @param mtu MTU of link connecting the PGW and the remote host
     * @param delay One-way propagation delay of link connecting the PGW and the remote host
     * @return a pair containing a node pointer to the remote host and its IPv4 address
     */
    virtual std::pair<Ptr<Node>, Ipv4Address> SetupRemoteHost(std::optional<std::string> dataRate,
                                                              std::optional<uint16_t> mtu,
                                                              std::optional<Time> delay) = 0;

    /**
     * Setup a P2P link connecting the PGW to a remote host, and create a route
     * from the remote host to the UEs using IPv6 networks.
     * Addresses are hardcoded for simplicity.
     *
     * RemoteHost ---------------P2P----------- PGW ---------------------- SGW -- gNB -- UEs
     * 6001:db80:0000:0000:0200:00ff:fe00:0007  6001:db80:0000:0000:0200:00ff:fe00:0006
     *                                          7777:f00d:0000:0000:0000:0000:0000:0001  7777:f00d:
     *
     * @param dataRateValue Link speed connecting the PGW and the remote host
     * @param mtu MTU of link connecting the PGW and the remote host
     * @param delay One-way propagation delay of link connecting the PGW and the remote host
     * @return a pair containing a node pointer to the remote host and its IPv6 address
     */
    virtual std::pair<Ptr<Node>, Ipv6Address> SetupRemoteHost6(std::optional<std::string> dataRate,
                                                               std::optional<uint16_t> mtu,
                                                               std::optional<Time> delay) = 0;
};

} // namespace ns3

#endif // NR_EPC_HELPER_H
