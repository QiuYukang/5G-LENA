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

#include <cstdint>
#include <map>
#include <optional>

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
     * @param qfi the QoS Flow ID (QFI) of the bearer which will be classified
     *
     * QFI must take a unique value between 0 and 63
     *
     */
    void Add(Ptr<NrQosRule> rule, uint8_t qfi);

    /**
     * delete an existing QoS rule from the classifier
     *
     * @param qfi the QoS Flow ID (QFI) QoS rule to be deleted
     * @return true if an entry for the QFI was found and deleted
     */
    bool Delete(uint8_t qfi);

    /**
     * clear all QoS rule from the classifier
     */
    void Clear();

    /**
     * classify an IP packet
     *
     * The packet is classified by iterating the QoS rules in increasing order of precedence
     * value until a match is found.
     *
     * @param p the IP packet. The outermost header can only be an IPv4 or an IPv6 header.
     * @param direction the QoS rule direction (can be downlink, uplink or bi-directional)
     * @param protocolNumber the protocol of the packet. Only IPv4 and IPv6 are supported.
     *
     * @return the QoS flow identifier (0-63) if a rule matches; std::nullopt if no rule matched.
     *         QFI=0 is reserved for the default bearer.
     */
    std::optional<uint8_t> Classify(Ptr<Packet> p,
                                    NrQosRule::Direction direction,
                                    uint16_t protocolNumber);

  protected:
    /**
     * QoS rules stored in a multimap keyed by rule precedence.
     *
     * Key: Rule precedence (0-255). Rules with lower precedence values are
     * evaluated first during packet classification, per 3GPP TS 24.501.
     *
     * Value: Ptr<NrQosRule> containing the rule and its associated metadata
     * (precedence, QFI, packet filters).
     *
     * Using multimap allows multiple rules to have the same precedence value.
     * During classification, rules are iterated in precedence order (ascending)
     * until a matching rule is found. The QFI is obtained from the matched rule.
     *
     * For rules with identical precedence values, iteration order is determined by
     * insertion order (the order Add() was called). This provides deterministic
     * behavior and allows implicit control of evaluation order via insertion sequence
     * when precedence values are identical.
     *
     * When deleting a rule by QFI, the map is iterated to find the entry whose
     * rule->GetQfi() matches the requested QFI.
     */
    std::multimap<uint8_t, Ptr<NrQosRule>> m_qosRuleMap;

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
