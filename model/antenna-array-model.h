/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef ANTENNA_ARRAY_MODEL_H_
#define ANTENNA_ARRAY_MODEL_H_

#include "antenna-array-basic-model.h"
#include <map>

namespace ns3 {

class AntennaArrayModel : public AntennaArrayBasicModel
{
public:

  /**
   * \brief Predefined antenna orientation options
   */
  enum AntennaOrientation{
    X0,//!< X0 Means that antenna's X axis is set to 0, hence the antenna is placed in Z-Y plane
    Z0,//!< Z0 Means that antenna's Z axis is set to 0, hence the antenna is placed in X-Y plane
    Y0 //!< Y0 Means that antenna's Y axis is set to 0, hence the antenna is placed in X-Z plane
  };

  AntennaArrayModel ();

  virtual ~AntennaArrayModel () override;

  virtual void DoInitialize (void) override;

  static TypeId GetTypeId ();

  virtual double GetGainDb (Angles a) override;

  virtual void SetBeamformingVector (complexVector_t antennaWeights, BeamId beamId,
                                     Ptr<NetDevice> device) override;

  virtual void ChangeBeamformingVector (Ptr<NetDevice> device) override;

  virtual void ChangeToOmniTx () override;

  virtual BeamformingVector GetCurrentBeamformingVector () override;

  virtual BeamformingVector GetBeamformingVector (Ptr<NetDevice> device) override;

  virtual bool IsOmniTx () override;

  virtual double GetRadiationPattern (double vangle, double hangle = 0) override;

  virtual Vector GetAntennaLocation (uint32_t index) override;

  virtual void SetSector (uint32_t sector) override;

  virtual void SetSector (uint8_t sector, double elevation = 90, Ptr<NetDevice> netDevice = nullptr) override;

  void SetAntennaOrientation (enum AntennaArrayModel::AntennaOrientation orientation);

  enum AntennaArrayModel::AntennaOrientation GetAntennaOrientation () const;

  /**
   * \brief Returns the number of antenna elements in the first dimension.
   */
  virtual uint8_t GetAntennaNumDim1 () const override;

  /**
   * \brief Returns the number of antenna elements in the second dimension.
   */
  virtual uint8_t GetAntennaNumDim2 () const override;

  /**
   * \brief Set the number of antenna elements in the first dimension
   * \param antennaNum the number of antenna elements in the first dimension
   */
  virtual void SetAntennaNumDim1 (uint8_t antennaNum) override;
   /**
    * \brief Set the number of antenna elements in the second dimension
    * \param antennaNum the number of antenna elements in the second dimension
    */
  virtual void SetAntennaNumDim2 (uint8_t antennaNum) override;

  /**
  * Get SpectrumModel corresponding to this antenna instance
  * @return SpectrumModel
  */
  virtual Ptr<const SpectrumModel> GetSpectrumModel () const override;

  /**
   * Set SpectrumModel that will be used by this antenna instancew
   * @param sm SpectrumModel to be used
   */
  virtual void SetSpectrumModel (Ptr<const SpectrumModel> sm) override;

private:

  typedef std::map<Ptr<NetDevice>, BeamformingVector> BeamformingStorage; /*!< A type represents a map where the key is a pointer
                                                                               to the device and the value is the BeamformingVector element */
  bool m_omniTx;
  double m_minAngle;
  double m_maxAngle;
  BeamformingVector m_currentBeamformingVector;
  BeamformingStorage m_beamformingVectorMap;

protected:
  double m_disV;       //antenna spacing in the vertical direction in terms of wave length.
  double m_disH;       //antenna spacing in the horizontal direction in terms of wave length.
  AntennaOrientation m_orientation; // antenna orientation, for example, when set to "X0" (x=0) it means that the antenna will be in y-z plane
  double m_antennaGain; //antenna gain

  uint8_t m_antennaNumDim1; //!< The number of antenna elements in the first dimension.
  uint8_t m_antennaNumDim2; //!< The number of antenna elements in the first dimension.

  Ptr<const SpectrumModel> m_spectrumModel;

  BeamformingVector m_omniTxRxW; //!< Beamforming vector that emulates omnidirectional transmission and reception
};

std::ostream & operator<< (std::ostream & os, AntennaArrayModel::BeamId const & item);

} /* namespace ns3 */

#endif /* SRC_ANTENNA_MODEL_ANTENNA_ARRAY_MODEL_H_ */
