// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef WRAPAROUND_THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H
#define WRAPAROUND_THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H

#include "distance-based-three-gpp-spectrum-propagation-loss-model.h"

namespace ns3
{

/**
 * @ingroup nr-utils
 * @brief Wraparound 3GPP Spectrum Propagation Loss Model
 *
 * This class inherits DistanceBasedThreeGppSpectrumPropagationLossModel
 * and calculates the fading and beamforming only for the signals
 * being transmitted among nodes whose distance is lower than the
 * max allowed distance that can be configured
 * through the attribute of this class. Before computing that
 * with its parent classes, it also applies the wraparound model in nodes.
 *
 * NOTE: This model is a temporary solution meant EXCLUSIVELY for **nr-4.1**
 * due to calibration work, and EXCLUSIVELY compatible with **ns-3.45**.
 * The hexagonal wraparound model will be upstreamed in ns-3.46.
 *
 * @see DistanceBasedThreeGppSpectrumPropagationLossModel
 */
class WraparoundThreeGppSpectrumPropagationLossModel
    : public DistanceBasedThreeGppSpectrumPropagationLossModel
{
  public:
    /**
     * Constructor
     */
    WraparoundThreeGppSpectrumPropagationLossModel();

    /**
     * Destructor
     */
    ~WraparoundThreeGppSpectrumPropagationLossModel() override;

    /**
     * Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * @brief Computes the received PSD.
     *
     * This function computes the received PSD by applying the 3GPP fast fading
     * model and the beamforming gain. However, if the distance between a and b
     * is higher than allowed this class will return 0 PSD. Before computing that
     * with its parent classes, it also applies the wraparound model in nodes.
     *
     * @param params tx parameters
     * @param a first node mobility model
     * @param b second node mobility model
     * @param aPhasedArrayModel the antenna array of the first node
     * @param bPhasedArrayModel the antenna array of the second node
     * @return the received PSD
     */
    Ptr<SpectrumSignalParameters> DoCalcRxPowerSpectralDensity(
        Ptr<const SpectrumSignalParameters> params,
        Ptr<const MobilityModel> a,
        Ptr<const MobilityModel> b,
        Ptr<const PhasedArrayModel> aPhasedArrayModel,
        Ptr<const PhasedArrayModel> bPhasedArrayModel) const override;
};
} // namespace ns3

#endif /* WRAPAROUND_THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H */
