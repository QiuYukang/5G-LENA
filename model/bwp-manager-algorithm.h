/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Natale Patriciello <natale.patriciello@gmail.com>
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
 */
#ifndef BWPMANAGERALGORITHM_H
#define BWPMANAGERALGORITHM_H

#include <ns3/object-base.h>
#include <ns3/eps-bearer.h>

namespace ns3 {

/**
 * \brief Interface for a Bwp selection algorithm based on the bearer
 *
 * \see GetBwpForEpsBearer
 */
class BwpManagerAlgorithm : public ObjectBase
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the object
   */
  static TypeId GetTypeId ();

  BwpManagerAlgorithm () : ObjectBase ()
  {
  }
  virtual ~BwpManagerAlgorithm () override
  {
  }
  virtual TypeId GetInstanceTypeId (void) const override;

  virtual uint8_t GetBwpForEpsBearer (const EpsBearer::Qci &v) const = 0;
};

class BwpManagerAlgorithmStatic : public BwpManagerAlgorithm
{
public:
  /**
   * \brief GetTypeId
   * \return The TypeId of the object
   */
  static TypeId GetTypeId ();

  BwpManagerAlgorithmStatic () : BwpManagerAlgorithm ()
  {
  }
  virtual ~BwpManagerAlgorithmStatic () override
  {
  }

  virtual uint8_t GetBwpForEpsBearer (const EpsBearer::Qci &v) const override;

  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetConvVoiceBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_CONV_VOICE] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetConvVideoBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_CONV_VIDEO] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetGamingBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_GAMING] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNonConvVideoBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_NON_CONV_VIDEO] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcPttBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_MC_PUSH_TO_TALK] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNmcPttBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_NMC_PUSH_TO_TALK] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcVideoBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_MC_VIDEO] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetGbrV2xBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::GBR_V2X] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetImsBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_IMS] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpOpBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_OPERATOR] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoGamingBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_VOICE_VIDEO_GAMING] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpPremiumBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_PREMIUM] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetVideoTcpDefaultBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_VIDEO_TCP_DEFAULT] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcDelaySignalBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_MC_DELAY_SIGNAL] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetMcDataBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_MC_DATA] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetNgbrV2xBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_V2X] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetLowLatEmbbBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::NGBR_LOW_LAT_EMBB] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetDiscreteAutSmallBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::DGBR_DISCRETE_AUT_SMALL] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetDiscreteAutLargeBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::DGBR_DISCRETE_AUT_LARGE] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetItsBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::DGBR_ITS] = bwpIndex;
  }
  /**
   * \brief Set BWP index of the QCI in the function name
   * \param bwpIndex Bwp Index to be assigned to the selected QCI
   */
  void SetElectricityBwp (uint8_t bwpIndex)
  {
    m_qciToBwpMap[EpsBearer::DGBR_ELECTRICITY] = bwpIndex;
  }

private:
  /**
   * \brief Map between QCI and BWP
   */
  std::unordered_map <uint8_t, uint8_t> m_qciToBwpMap;
};

} // namespace ns3
#endif // BWPMANAGERALGORITHM_H
