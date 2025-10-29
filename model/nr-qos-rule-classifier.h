// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo  <nbaldo@cttc.es>

#ifndef NR_QOS_RULE_CLASSIFIER_H
#define NR_QOS_RULE_CLASSIFIER_H

#include "nr-qos-rule.h"

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

#include <map>

namespace ns3
{

class NrQosRule;
class Packet;

/**
 * @brief classifies IP packets according to QoS rules
 *
 * @note this implementation works with IPv4 and IPv6.
 * When there is fragmentation of IP packets, UDP/TCP ports maybe missing.
 *
 * The following actions are performed to use the port info present in the first segment with
 * the next fragments:
 *  - Port info is stored if it is available, i.e. it is the first fragment with UDP/TCP protocol
 *    and there is enough data in the payload of the IP packet for the port numbers.
 *  - Port info is used for the next fragments.
 *  - Port info is deleted, when the last fragment is processed.
 *
 * When we cannot cache the port info, the QoS rule of the default bearer is used. This may happen
 * if there is reordering or losses of IP packets.
 */
class NrQosRuleClassifier : public SimpleRefCount<NrQosRuleClassifier>
{
  public:
    NrQosRuleClassifier();

    /**
     * add a QoS rule to the Classifier
     *
     * @param rule the QoS rule to be added
     * @param id the ID of the bearer which will be classified
     *
     */
    void Add(Ptr<NrQosRule> rule, uint32_t id);

    /**
     * delete an existing QoS rule from the classifier
     *
     * @param id the identifier of the QoS rule to be deleted
     */
    void Delete(uint32_t id);

    /**
     * classify an IP packet
     *
     * @param p the IP packet. The outmost header can only be an IPv4 or an IPv6 header.
     * @param direction the QoS rule direction (can be downlink, uplink or bi-directional)
     * @param protocolNumber the protocol of the packet. Only IPv4 and IPv6 are supported.
     *
     * @return the identifier (>0) of the first rule that matches with the IP packet; 0 if no rule
     * matched.
     */
    uint32_t Classify(Ptr<Packet> p, NrQosRule::Direction direction, uint16_t protocolNumber);

  protected:
    std::map<uint32_t, Ptr<NrQosRule>> m_qosRuleMap; ///< QoS rule map

    std::map<std::tuple<uint32_t, uint32_t, uint8_t, uint16_t>, std::pair<uint32_t, uint32_t>>
        m_classifiedIpv4Fragments; ///< Map with already classified IPv4 Fragments
                                   ///< An entry is added when the port info is available, i.e.
                                   ///<   first fragment, UDP/TCP protocols and enough payload data
                                   ///< An entry is used if port info is not available, i.e.
                                   ///<   not first fragment or not enough payload data for TCP/UDP
                                   ///< An entry is removed when the last fragment is classified
                                   ///<   Note: If last fragment is lost, entry is not removed
};

} // namespace ns3

#endif /* NR_QOS_RULE_CLASSIFIER_H */
