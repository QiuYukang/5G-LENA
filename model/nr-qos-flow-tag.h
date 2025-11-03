// Copyright (c) 2011,2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Marco Miozzo  <marco.miozzo@cttc.es>
//         Nicola Baldo <nbaldo@cttc.es>

#ifndef NR_QOS_FLOW_TAG_H
#define NR_QOS_FLOW_TAG_H

#include "ns3/tag.h"

namespace ns3
{

class Tag;

/**
 * Tag used to define the RNTI and QoS flow ID for packets
 * interchanged between the NrEpcGnbApplication and the NrGnbNetDevice
 */

class NrQosFlowTag : public Tag
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    /**
     * Create an empty NrQosFlowTag
     */
    NrQosFlowTag();

    /**
     * Create a NrQosFlowTag with the given RNTI and flow id
     *
     * @param rnti the value of the RNTI to set
     * @param qfi the value of the flow Id to set
     */
    NrQosFlowTag(uint16_t rnti, uint8_t qfi);

    /**
     * Set the RNTI to the given value.
     *
     * @param rnti the value of the RNTI to set
     */
    void SetRnti(uint16_t rnti);

    /**
     * Set the QoS flow Id to the given value.
     *
     * @param qfi the value of the QoS flow Id to set
     */
    void SetQfi(uint8_t qfi);

    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    uint32_t GetSerializedSize() const override;
    void Print(std::ostream& os) const override;

    /**
     * Get RNTI function
     * @returns the RNTI
     */
    uint16_t GetRnti() const;
    /**
     * Get QoS flow Id function
     * @returns the QoS flow Id
     */
    uint8_t GetQfi() const;

  private:
    uint16_t m_rnti; ///< RNTI value
    uint8_t m_qfi;   ///< QoS Flow Id value
};

} // namespace ns3

#endif /* NR_QOS_FLOW_TAG_H */
