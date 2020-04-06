/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#ifndef NR_EESM_CC_H
#define NR_EESM_CC_H

#include "nr-eesm-error-model.h"

namespace ns3 {

/**
 * \ingroup error-models
 * @brief The NrEesmCc class
 */
class NrEesmCc : public NrEesmErrorModel
{
public:
  /**
   * \brief Get the type id of the object
   * \return the type id of the object
   */
  static TypeId GetTypeId (void);
  NrEesmCc();
  virtual ~NrEesmCc () override;

protected:
  double ComputeSINR (const SpectrumValue& sinr, const std::vector<int>& map, uint8_t mcs,
                      uint32_t sizeBit, const NrErrorModel::NrErrorModelHistory &sinrHistory) const override;
  double GetMcsEq (uint16_t mcsTx) const override;
};

} // namespace ns3

#endif // NR_EESM_CC_H
