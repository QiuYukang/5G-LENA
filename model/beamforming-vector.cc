// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "beamforming-vector.h"

#include "ns3/angles.h"
#include "ns3/enum.h"
#include "ns3/hexagonal-wraparound-model.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(PhasedArrayAngleConvention);

PhasedArrayModel::ComplexVector
CreateQuasiOmniBfv(const Ptr<const UniformPlanarArray>& antenna)
{
    auto antennaRows = antenna->GetNumRows();
    auto antennaColumns = antenna->GetNumColumns();
    auto numElemsPerPort = antenna->GetNumElemsPerPort();

    double power = 1 / sqrt(numElemsPerPort);
    size_t numPolarizations = antenna->IsDualPol() ? 2 : 1;

    PhasedArrayModel::ComplexVector omni(antennaRows * antennaColumns * numPolarizations);
    uint16_t bfIndex = 0;
    for (size_t pol = 0; pol < numPolarizations; pol++)
    {
        for (uint32_t ind = 0; ind < antennaRows; ind++)
        {
            std::complex<double> c = 0.0;
            if (antennaRows % 2 == 0)
            {
                c = exp(std::complex<double>(0, M_PI * ind * ind / antennaRows));
            }
            else
            {
                c = exp(std::complex<double>(0, M_PI * ind * (ind + 1) / antennaRows));
            }
            for (uint32_t ind2 = 0; ind2 < antennaColumns; ind2++)
            {
                std::complex<double> d = 0.0;
                if (antennaColumns % 2 == 0)
                {
                    d = exp(std::complex<double>(0, M_PI * ind2 * ind2 / antennaColumns));
                }
                else
                {
                    d = exp(std::complex<double>(0, M_PI * ind2 * (ind2 + 1) / antennaColumns));
                }
                omni[bfIndex] = (c * d * power);
                bfIndex++;
            }
        }
    }
    return omni;
}

PhasedArrayModel::ComplexVector
CreateDirectionalBfv(const Ptr<const UniformPlanarArray>& antenna, double sector, double elevation)
{
    UintegerValue uintValueNumColumns;
    antenna->GetAttribute("NumColumns", uintValueNumColumns);

    double hAngle_radian =
        M_PI * (sector / static_cast<double>(uintValueNumColumns.Get())) - 0.5 * M_PI;
    double vAngle_radian = elevation * M_PI / 180;
    uint16_t size = antenna->GetNumElems();
    PhasedArrayModel::ComplexVector tempVector(size);
    auto numAnalogBeamElements = (antenna->GetVElemsPerPort() * antenna->GetHElemsPerPort());
    auto power = 1.0 / sqrt(numAnalogBeamElements);
    for (auto ind = 0; ind < size; ind++)
    {
        Vector loc = antenna->GetElementLocation(ind);
        double phase =
            -2 * M_PI *
            (sin(vAngle_radian) * cos(hAngle_radian) * loc.x +
             sin(vAngle_radian) * sin(hAngle_radian) * loc.y + cos(vAngle_radian) * loc.z);
        tempVector[ind] = (exp(std::complex<double>(0, phase)) * power);
    }
    return tempVector;
}

PhasedArrayModel::ComplexVector
CreateDirectionalBfvAz(const Ptr<const UniformPlanarArray>& antenna, double azimuth, double zenith)
{
    UintegerValue uintValueNumColumns;
    antenna->GetAttribute("NumColumns", uintValueNumColumns);

    double hAngle_radian = azimuth * M_PI / 180;
    double vAngle_radian = zenith * M_PI / 180;
    uint16_t size = antenna->GetNumElems();
    double power = 1 / sqrt(size);
    PhasedArrayModel::ComplexVector tempVector(size);
    if (size == 1)
    {
        tempVector[0] = power; // single AE, no BF
    }
    else
    {
        for (auto ind = 0; ind < size; ind++)
        {
            Vector loc = antenna->GetElementLocation(ind);
            double phase =
                -2 * M_PI *
                (sin(vAngle_radian) * cos(hAngle_radian) * loc.x +
                 sin(vAngle_radian) * sin(hAngle_radian) * loc.y + cos(vAngle_radian) * loc.z);
            tempVector[ind] = exp(std::complex<double>(0, phase)) * power;
        }
    }
    return tempVector;
}

PhasedArrayModel::ComplexVector
CreateDirectPathBfv(const Ptr<MobilityModel>& a,
                    const Ptr<MobilityModel>& b,
                    const Ptr<const UniformPlanarArray>& antenna)
{
    // retrieve the position of the two devices
    Vector aPos = a->GetPosition();
    Vector bPos = b->GetPosition();
    auto wraparoundModel = b->GetObject<Node>()->GetObject<HexagonalWraparoundModel>();
    if (wraparoundModel)
    {
        bPos = wraparoundModel->GetVirtualPosition(aPos, bPos);
    }

    // compute the azimuth and the elevation angles
    Angles completeAngle(bPos, aPos);

    double hAngleRadian = completeAngle.GetAzimuth();

    double vAngleRadian = completeAngle.GetInclination(); // the elevation angle

    // retrieve the number of antenna elements
    int totNoArrayElements = antenna->GetNumElems();
    auto numElemsPerPort = antenna->GetNumElemsPerPort();

    // the total power is divided equally among the antenna elements
    double power = 1 / sqrt(numElemsPerPort);

    PhasedArrayModel::ComplexVector antennaWeights(totNoArrayElements);
    // compute the antenna weights
    for (int ind = 0; ind < totNoArrayElements; ind++)
    {
        Vector loc = antenna->GetElementLocation(ind);
        double phase = -2 * M_PI *
                       (sin(vAngleRadian) * cos(hAngleRadian) * loc.x +
                        sin(vAngleRadian) * sin(hAngleRadian) * loc.y + cos(vAngleRadian) * loc.z);
        antennaWeights[ind] = exp(std::complex<double>(0, phase)) * power;
    }

    return antennaWeights;
}

PhasedArrayModel::ComplexVector
CreateKroneckerBfvImpl(const Ptr<const UniformPlanarArray>& antenna,
                       double vPhasePerEl,
                       double hPhasePerEl)
{
    // retrieve the number of antenna elements to create bf vector
    PhasedArrayModel::ComplexVector bfVector(antenna->GetNumElems());
    auto numAnalogBeamElements = antenna->GetVElemsPerPort() * antenna->GetHElemsPerPort();

    // normalize because the total power is divided equally among the analog beams elements
    auto normalizer = 1.0 / sqrt(numAnalogBeamElements);

    auto numCols = antenna->GetNumColumns();
    auto numRows = antenna->GetNumRows();

    // compute the antenna weights (bfvector)
    for (auto elIdx = size_t{0}; elIdx < antenna->GetNumElems(); elIdx++)
    {
        auto colIdx = elIdx % numCols;
        auto rowIdx = elIdx / numCols;
        auto isSkippedCol = (colIdx >= antenna->GetHElemsPerPort());
        auto isSkippedRow = (rowIdx >= antenna->GetVElemsPerPort());
        if (isSkippedCol || isSkippedRow || (elIdx >= numRows * numCols))
        {
            bfVector[elIdx] = 0.0;
            continue;
        }
        auto combPhase = rowIdx * vPhasePerEl + colIdx * hPhasePerEl;
        bfVector[elIdx] = normalizer * std::complex<double>(cos(combPhase), sin(combPhase));
    }
    return bfVector;
}

PhasedArrayModel::ComplexVector
CreateKroneckerBfvThreeGpp(const Ptr<const UniformPlanarArray>& antenna,
                           double zenith,
                           double azimuth)
{
    NS_ASSERT_MSG(0.0 <= zenith && zenith <= 180.0,
                  "3GPP zenith set to " << zenith << " should be in range [0, 180] degrees.");
    NS_ASSERT_MSG(-90.0 <= azimuth && azimuth <= 90.0,
                  "3GPP azimuth set to " << azimuth << " should be in range [-90, 90] degrees.");

    // Compute phases per element assuming a single bidimensional UPA
    double inc = zenith * M_PI / 180.0; // θ (zenith/inclination)
    double az = azimuth * M_PI / 180.0; // φ (azimuth)

    double dV = antenna->GetAntennaVerticalSpacing();   // in λ
    double dH = antenna->GetAntennaHorizontalSpacing(); // in λ

    double vPhasePerEl = -2.0 * M_PI * dV * std::cos(inc);
    double hPhasePerEl = -2.0 * M_PI * dH * std::sin(inc) * std::sin(az);
    return CreateKroneckerBfvImpl(antenna, vPhasePerEl, hPhasePerEl);
}

PhasedArrayModel::ComplexVector
CreateKroneckerBfvUla(const Ptr<const UniformPlanarArray>& antenna,
                      double rowAngle,
                      double colAngle)
{
    NS_ASSERT_MSG(0.0 <= rowAngle && rowAngle <= 180.0,
                  "ULA vertical angle set to " << rowAngle
                                               << " should be in range [0, 180] degrees.");
    NS_ASSERT_MSG(0.0 <= colAngle && colAngle <= 180.0,
                  "ULA horizontal angle set to " << colAngle
                                                 << " should be in range [0, 180] degrees.");

    // Compute phases per element assuming two separate ULA panels, one vertical and the other
    // horizontal
    double dV = antenna->GetAntennaVerticalSpacing();   // in λ
    double dH = antenna->GetAntennaHorizontalSpacing(); // in λ

    auto vPhasePerEl = -2.0 * M_PI * dV * cos(rowAngle * M_PI / 180.0);
    auto hPhasePerEl = -2.0 * M_PI * dH * cos(colAngle * M_PI / 180.0);
    return CreateKroneckerBfvImpl(antenna, vPhasePerEl, hPhasePerEl);
}

PhasedArrayModel::ComplexVector
CreateKroneckerBfv(const Ptr<const UniformPlanarArray>& antenna, double rowAngle, double colAngle)
{
    switch (CreateObject<PhasedArrayAngleConvention>()->GetConvention())
    {
    case PhasedArrayAngleConvention::THREE_GPP:
        return CreateKroneckerBfvThreeGpp(antenna, rowAngle, colAngle);
    case PhasedArrayAngleConvention::ULA_VH:
        return CreateKroneckerBfvUla(antenna, rowAngle, colAngle);
    default:
        NS_ABORT_MSG("Unexpected angle convention");
    }
}

TypeId
PhasedArrayAngleConvention::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::PhasedArrayAngleConvention")
            .SetParent<Object>()
            .SetGroupName("Nr")
            .AddConstructor<PhasedArrayAngleConvention>()
            .AddAttribute(
                "AngleConvention",
                "Angle input convention: 3GPP (zenith/azimuth) or UlaVH (row/col angles).",
                EnumValue(PhasedArrayAngleConvention::ULA_VH),
                MakeEnumAccessor<PhasedArrayAngleConvention::AngleConvention>(
                    &PhasedArrayAngleConvention::m_angleConvention),
                MakeEnumChecker(PhasedArrayAngleConvention::THREE_GPP,
                                "3GPP",
                                PhasedArrayAngleConvention::ULA_VH,
                                "UlaVH"));
    return tid;
}

PhasedArrayAngleConvention::AngleConvention
PhasedArrayAngleConvention::GetConvention() const
{
    return m_angleConvention;
}
} // namespace ns3
