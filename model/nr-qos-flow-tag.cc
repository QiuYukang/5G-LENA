// Copyright (c) 2011,2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Marco Miozzo <marco.miozzo@cttc.es>
//         Nicola Baldo <nbaldo@cttc.es>

#include "nr-qos-flow-tag.h"

#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(NrQosFlowTag);

TypeId
NrQosFlowTag::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrQosFlowTag")
            .SetParent<Tag>()
            .SetGroupName("Nr")
            .AddConstructor<NrQosFlowTag>()
            .AddAttribute("Rnti",
                          "The RNTI that indicates the UE to which the packet belongs",
                          UintegerValue(0),
                          MakeUintegerAccessor(&NrQosFlowTag::GetRnti),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("Qfi",
                          "The QoS Flow ID within the UE to which the packet belongs",
                          UintegerValue(0),
                          MakeUintegerAccessor(&NrQosFlowTag::GetQfi),
                          MakeUintegerChecker<uint8_t>());
    return tid;
}

TypeId
NrQosFlowTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

NrQosFlowTag::NrQosFlowTag()
    : m_rnti(0),
      m_qfi(0)
{
}

NrQosFlowTag::NrQosFlowTag(uint16_t rnti, uint8_t qfi)
    : m_rnti(rnti),
      m_qfi(qfi)
{
}

void
NrQosFlowTag::SetRnti(uint16_t rnti)
{
    m_rnti = rnti;
}

void
NrQosFlowTag::SetQfi(uint8_t qfi)
{
    m_qfi = qfi;
}

uint32_t
NrQosFlowTag::GetSerializedSize() const
{
    return 3;
}

void
NrQosFlowTag::Serialize(TagBuffer i) const
{
    i.WriteU16(m_rnti);
    i.WriteU8(m_qfi);
}

void
NrQosFlowTag::Deserialize(TagBuffer i)
{
    m_rnti = (uint16_t)i.ReadU16();
    m_qfi = (uint8_t)i.ReadU8();
}

uint16_t
NrQosFlowTag::GetRnti() const
{
    return m_rnti;
}

uint8_t
NrQosFlowTag::GetQfi() const
{
    return m_qfi;
}

void
NrQosFlowTag::Print(std::ostream& os) const
{
    os << "rnti=" << m_rnti << ", qfi=" << (uint16_t)m_qfi;
}

} // namespace ns3
