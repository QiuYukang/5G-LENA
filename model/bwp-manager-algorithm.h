// Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef BWP_MANAGER_ALGORITHM_H
#define BWP_MANAGER_ALGORITHM_H

#include "nr-qos-flow.h"

#include "ns3/object.h"

namespace ns3
{

/**
 * @ingroup bwp
 * @brief Interface for a Bwp selection algorithm based on the flow
 *
 *
 * At the moment, we provide only a static algorithm that has to be configured
 * before the simulation starts (BwpManagerAlgorithmStatic).
 *
 *
 * @section bwp_manager_conf Configuration
 *
 * The algorithm can be set, before the scenario creation, through the
 * helper method NrHelper::SetGnbBwpManagerAlgorithmTypeId(). It is
 * also possible to set attributes, through NrHelper::SetGnbBwpManagerAlgorithmAttribute().
 *
 * For the UE, the methods to use are, respectively, NrHelper::SetUeBwpManagerAlgorithmTypeId()
 * and NrHelper::SetUeBwpManagerAlgorithmAttribute().
 *
 *
 * @see GetBwpForQosFlow
 * @see BwpManagerAlgorithmStatic
 */
class BwpManagerAlgorithm : public Object
{
  public:
    /**
     * @brief GetTypeId
     * @return The TypeId of the object
     */
    static TypeId GetTypeId();

    /**
     * @brief constructor
     */
    BwpManagerAlgorithm() = default;
    /**
     * ~BwpManagerAlgorithm
     */
    ~BwpManagerAlgorithm() override = default;
    /**
     * @brief Get the bandwidth part id for the 5QI specified
     * @param v the 5QI
     * @return the bwp id that the algorithm selects for the 5QI specified
     */
    virtual uint8_t GetBwpForQosFlow(const NrQosFlow::FiveQi& v) const = 0;
};

/**
 * @ingroup bwp
 * @brief The BwpManagerAlgorithmStatic class
 *
 * A static manager: it gets the association through a series of Attributes.
 */
class BwpManagerAlgorithmStatic : public BwpManagerAlgorithm
{
  public:
    /**
     * @brief GetTypeId
     * @return The TypeId of the object
     */
    static TypeId GetTypeId();

    /**
     * @brief constructor
     */
    BwpManagerAlgorithmStatic() = default;
    /**
     * @brief deconstructor
     */
    ~BwpManagerAlgorithmStatic() override = default;

    // inherited
    uint8_t GetBwpForQosFlow(const NrQosFlow::FiveQi& v) const override;

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetConvVoiceBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_CONV_VOICE] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetConvVoiceBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_CONV_VOICE);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetConvVideoBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_CONV_VIDEO] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetConvVideoBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_CONV_VIDEO);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetGamingBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_GAMING] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetGamingBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_GAMING);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetNonConvVideoBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_NON_CONV_VIDEO] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetNonConvVideoBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_NON_CONV_VIDEO);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetMcPttBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_MC_PUSH_TO_TALK] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetMcPttBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_MC_PUSH_TO_TALK);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetNmcPttBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_NMC_PUSH_TO_TALK] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetNmcPttBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_NMC_PUSH_TO_TALK);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetMcVideoBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_MC_VIDEO] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetMcVideoBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_MC_VIDEO);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetGbrV2xBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_V2X] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetGbrV2xBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_V2X);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetImsBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_IMS] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetImsBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_IMS);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetVideoTcpOpBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_VIDEO_TCP_OPERATOR] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetVideoTcpOpBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_VIDEO_TCP_OPERATOR);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetVideoGamingBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_VOICE_VIDEO_GAMING] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetVideoGamingBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_VOICE_VIDEO_GAMING);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetVideoTcpPremiumBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_VIDEO_TCP_PREMIUM] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetVideoTcpPremiumBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_VIDEO_TCP_PREMIUM);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetVideoTcpDefaultBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_VIDEO_TCP_DEFAULT] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetVideoTcpDefaultBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_VIDEO_TCP_DEFAULT);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetMcDelaySignalBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_MC_DELAY_SIGNAL] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetMcDelaySignalBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_MC_DELAY_SIGNAL);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetMcDataBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_MC_DATA] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetMcDataBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_MC_DATA);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetNgbrV2xBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_V2X] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetNgbrV2xBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_V2X);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetLowLatEmbbBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::NGBR_LOW_LAT_EMBB] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetLowLatEmbbBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::NGBR_LOW_LAT_EMBB);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetDiscreteAutSmallBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::DGBR_DISCRETE_AUT_SMALL] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetDiscreteAutSmallBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::DGBR_DISCRETE_AUT_SMALL);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetDiscreteAutLargeBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::DGBR_DISCRETE_AUT_LARGE] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetDiscreteAutLargeBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::DGBR_DISCRETE_AUT_LARGE);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetItsBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::DGBR_ITS] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetItsBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::DGBR_ITS);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetElectricityBwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::DGBR_ELECTRICITY] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetElectricityBwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::DGBR_ELECTRICITY);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetLiveUlStream71Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_LIVE_UL_71] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetLiveUlStream71Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_LIVE_UL_71);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetLiveUlStream72Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_LIVE_UL_72] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetLiveUlStream72Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_LIVE_UL_72);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetLiveUlStream73Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_LIVE_UL_73] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetLiveUlStream73Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_LIVE_UL_73);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetLiveUlStream74Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_LIVE_UL_74] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetLiveUlStream74Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_LIVE_UL_74);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetLiveUlStream76Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::GBR_LIVE_UL_76] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetLiveUlStream76Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::GBR_LIVE_UL_76);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetInterService87Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::DGBR_INTER_SERV_87] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetInterService87Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::DGBR_INTER_SERV_87);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetInterService88Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::DGBR_INTER_SERV_88] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetInterService88Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::DGBR_INTER_SERV_88);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetVisualContent89Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::DGBR_VISUAL_CONTENT_89] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetVisualContent89Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::DGBR_VISUAL_CONTENT_89);
    }

    /**
     * @brief Set BWP index of the 5QI in the function name
     * @param bwpIndex Bwp Index to be assigned to the selected 5QI
     */
    void SetVisualContent90Bwp(uint8_t bwpIndex)
    {
        m_fiveQiToBwpMap[NrQosFlow::DGBR_VISUAL_CONTENT_90] = bwpIndex;
    }

    /**
     * @brief Get the BWP index of the 5QI in the function name
     * @return the BWP index of the selected 5QI
     */
    uint8_t GetVisualContent90Bwp() const
    {
        return m_fiveQiToBwpMap.at(NrQosFlow::DGBR_VISUAL_CONTENT_90);
    }

  private:
    /**
     * @brief Map between 5QI and BWP
     */
    std::unordered_map<uint8_t, uint8_t> m_fiveQiToBwpMap;
};

} // namespace ns3
#endif // BWP_MANAGER_ALGORITHM_H
