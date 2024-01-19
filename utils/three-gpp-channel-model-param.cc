/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering,
// Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
// Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "three-gpp-channel-model-param.h"

#include "ns3/double.h"
#include "ns3/integer.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include "ns3/phased-array-model.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include <ns3/simulator.h>

#include <algorithm>
#include <random>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ThreeGppChannelModelParam");

NS_OBJECT_ENSURE_REGISTERED(ThreeGppChannelModelParam);

ThreeGppChannelModelParam::ThreeGppChannelModelParam()
    : ThreeGppChannelModel()
{
    NS_LOG_FUNCTION(this);
}

ThreeGppChannelModelParam::~ThreeGppChannelModelParam()
{
    NS_LOG_FUNCTION(this);
}

void
ThreeGppChannelModelParam::DoDispose()
{
    ThreeGppChannelModel::DoDispose();
}

TypeId
ThreeGppChannelModelParam::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ThreeGppChannelModelParam")
            .SetGroupName("Spectrum")
            .SetParent<ThreeGppChannelModel>()
            .AddConstructor<ThreeGppChannelModelParam>()
            .AddAttribute("Ro",
                          "Cross polarization correlation parameter.",
                          DoubleValue(0.0),
                          MakeDoubleAccessor(&ThreeGppChannelModelParam::SetRo),
                          MakeDoubleChecker<double>())
            .AddAttribute(
                "ParametrizedCorrelation",
                "Whether the parameter value Ro will be used as the term for the correlation or "
                "the 3gpp term: std::sqrt (1 / k). When true Ro will be used, otherwise, 3gpp "
                "term.",
                BooleanValue(true),
                MakeBooleanAccessor(&ThreeGppChannelModelParam::m_parametrizedCorrelation),
                MakeBooleanChecker())

        ;
    return tid;
}

void
ThreeGppChannelModelParam::SetRo(double ro)
{
    m_Ro = ro;
}

Ptr<MatrixBasedChannelModel::ChannelMatrix>
ThreeGppChannelModelParam::GetNewChannel(Ptr<const ThreeGppChannelParams> channelParams,
                                         Ptr<const ParamsTable> table3gpp,
                                         const Ptr<const MobilityModel> sMob,
                                         const Ptr<const MobilityModel> uMob,
                                         Ptr<const PhasedArrayModel> sAntenna,
                                         Ptr<const PhasedArrayModel> uAntenna) const
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT_MSG(m_frequency > 0.0, "Set the operating frequency first!");

    // create a channel matrix instance
    Ptr<ChannelMatrix> channelMatrix = Create<ChannelMatrix>();
    channelMatrix->m_generatedTime = Simulator::Now();
    channelMatrix->m_nodeIds =
        std::make_pair(sMob->GetObject<Node>()->GetId(), uMob->GetObject<Node>()->GetId());

    // check if channelParams structure is generated in direction s-to-u or u-to-s
    bool isSameDirection = (channelParams->m_nodeIds == channelMatrix->m_nodeIds);

    MatrixBasedChannelModel::Double2DVector rayAodRadian;
    MatrixBasedChannelModel::Double2DVector rayAoaRadian;
    MatrixBasedChannelModel::Double2DVector rayZodRadian;
    MatrixBasedChannelModel::Double2DVector rayZoaRadian;

    // if channel params is generated in the same direction in which we
    // generate the channel matrix, angles and zenith od departure and arrival are ok,
    // just set them to corresponding variable that will be used for the generation
    // of channel matrix, otherwise we need to flip angles and zeniths of departure and arrival
    if (isSameDirection)
    {
        rayAodRadian = channelParams->m_rayAodRadian;
        rayAoaRadian = channelParams->m_rayAoaRadian;
        rayZodRadian = channelParams->m_rayZodRadian;
        rayZoaRadian = channelParams->m_rayZoaRadian;
    }
    else
    {
        rayAodRadian = channelParams->m_rayAoaRadian;
        rayAoaRadian = channelParams->m_rayAodRadian;
        rayZodRadian = channelParams->m_rayZoaRadian;
        rayZoaRadian = channelParams->m_rayZodRadian;
    }

    // Step 11: Generate channel coefficients for each cluster n and each receiver
    //  and transmitter element pair u,s.
    // where n is cluster index, u and s are receive and transmit antenna element.
    size_t uSize = uAntenna->GetNumberOfElements();
    size_t sSize = sAntenna->GetNumberOfElements();

    // NOTE: Since each of the strongest 2 clusters are divided into 3 sub-clusters,
    // the total cluster will generally be numReducedCLuster + 4.
    // However, it might be that m_cluster1st = m_cluster2nd. In this case the
    // total number of clusters will be numReducedCLuster + 2.
    uint16_t numOverallCluster = (channelParams->m_cluster1st != channelParams->m_cluster2nd)
                                     ? channelParams->m_reducedClusterNumber + 4
                                     : channelParams->m_reducedClusterNumber + 2;
    Complex3DVector hUsn(uSize, sSize, numOverallCluster); // channel coefficient hUsn (u, s, n);
    NS_ASSERT(channelParams->m_reducedClusterNumber <= channelParams->m_clusterPhase.size());
    NS_ASSERT(channelParams->m_reducedClusterNumber <= channelParams->m_clusterPower.size());
    NS_ASSERT(channelParams->m_reducedClusterNumber <=
              channelParams->m_crossPolarizationPowerRatios.size());
    NS_ASSERT(channelParams->m_reducedClusterNumber <= rayZoaRadian.size());
    NS_ASSERT(channelParams->m_reducedClusterNumber <= rayZodRadian.size());
    NS_ASSERT(channelParams->m_reducedClusterNumber <= rayAoaRadian.size());
    NS_ASSERT(channelParams->m_reducedClusterNumber <= rayAodRadian.size());
    NS_ASSERT(table3gpp->m_raysPerCluster <= channelParams->m_clusterPhase[0].size());
    NS_ASSERT(table3gpp->m_raysPerCluster <=
              channelParams->m_crossPolarizationPowerRatios[0].size());
    NS_ASSERT(table3gpp->m_raysPerCluster <= rayZoaRadian[0].size());
    NS_ASSERT(table3gpp->m_raysPerCluster <= rayZodRadian[0].size());
    NS_ASSERT(table3gpp->m_raysPerCluster <= rayAoaRadian[0].size());
    NS_ASSERT(table3gpp->m_raysPerCluster <= rayAodRadian[0].size());

    double x = sMob->GetPosition().x - uMob->GetPosition().x;
    double y = sMob->GetPosition().y - uMob->GetPosition().y;
    double distance2D = sqrt(x * x + y * y);
    // NOTE we assume hUT = min (height(a), height(b)) and
    // hBS = max (height (a), height (b))
    double hUt = std::min(sMob->GetPosition().z, uMob->GetPosition().z);
    double hBs = std::max(sMob->GetPosition().z, uMob->GetPosition().z);
    // compute the 3D distance using eq. 7.4-1
    double distance3D = std::sqrt(distance2D * distance2D + (hBs - hUt) * (hBs - hUt));

    Angles sAngle(uMob->GetPosition(), sMob->GetPosition());
    Angles uAngle(sMob->GetPosition(), uMob->GetPosition());

    Complex2DVector raysPreComp(channelParams->m_reducedClusterNumber,
                                table3gpp->m_raysPerCluster); // stores part of the ray expression,
    // cached as independent from the u- and s-indexes
    Double2DVector sinCosA; // cached multiplications of sin and cos of the ZoA and AoA angles
    Double2DVector sinSinA; // cached multiplications of sines of the ZoA and AoA angles
    Double2DVector cosZoA;  // cached cos of the ZoA angle
    Double2DVector sinCosD; // cached multiplications of sin and cos of the ZoD and AoD angles
    Double2DVector sinSinD; // cached multiplications of the cosines of the ZoA and AoA angles
    Double2DVector cosZoD;  // cached cos of the ZoD angle

    // resize to appropriate dimensions
    sinCosA.resize(channelParams->m_reducedClusterNumber);
    sinSinA.resize(channelParams->m_reducedClusterNumber);
    cosZoA.resize(channelParams->m_reducedClusterNumber);
    sinCosD.resize(channelParams->m_reducedClusterNumber);
    sinSinD.resize(channelParams->m_reducedClusterNumber);
    cosZoD.resize(channelParams->m_reducedClusterNumber);
    for (uint8_t nIndex = 0; nIndex < channelParams->m_reducedClusterNumber; nIndex++)
    {
        sinCosA[nIndex].resize(table3gpp->m_raysPerCluster);
        sinSinA[nIndex].resize(table3gpp->m_raysPerCluster);
        cosZoA[nIndex].resize(table3gpp->m_raysPerCluster);
        sinCosD[nIndex].resize(table3gpp->m_raysPerCluster);
        sinSinD[nIndex].resize(table3gpp->m_raysPerCluster);
        cosZoD[nIndex].resize(table3gpp->m_raysPerCluster);
    }
    // pre-compute the terms which are independent from uIndex and sIndex
    for (uint8_t nIndex = 0; nIndex < channelParams->m_reducedClusterNumber; nIndex++)
    {
        for (uint8_t mIndex = 0; mIndex < table3gpp->m_raysPerCluster; mIndex++)
        {
            DoubleVector initialPhase = channelParams->m_clusterPhase[nIndex][mIndex];
            NS_ASSERT(4 <= initialPhase.size());
            double k = channelParams->m_crossPolarizationPowerRatios[nIndex][mIndex];

            // cache the component of the "rays" terms which depend on the random angle of arrivals
            // and departures and initial phases only
            auto [rxFieldPatternPhi, rxFieldPatternTheta] = uAntenna->GetElementFieldPattern(
                Angles(channelParams->m_rayAoaRadian[nIndex][mIndex],
                       channelParams->m_rayZoaRadian[nIndex][mIndex]));
            auto [txFieldPatternPhi, txFieldPatternTheta] = sAntenna->GetElementFieldPattern(
                Angles(channelParams->m_rayAodRadian[nIndex][mIndex],
                       channelParams->m_rayZodRadian[nIndex][mIndex]));

            double Ro = 0;
            if (m_parametrizedCorrelation)
            {
                Ro = m_Ro;
            }
            else
            {
                Ro = std::sqrt(1 / k);
            }

            raysPreComp(nIndex, mIndex) =
                std::complex<double>(cos(initialPhase[0]), sin(initialPhase[0])) *
                    rxFieldPatternTheta * txFieldPatternTheta +
                std::complex<double>(cos(initialPhase[1]), sin(initialPhase[1])) * Ro *
                    rxFieldPatternTheta * txFieldPatternPhi +
                std::complex<double>(cos(initialPhase[2]), sin(initialPhase[2])) * Ro *
                    rxFieldPatternPhi * txFieldPatternTheta +
                std::complex<double>(cos(initialPhase[3]), sin(initialPhase[3])) *
                    rxFieldPatternPhi * txFieldPatternPhi;

            // cache the component of the "rxPhaseDiff" terms which depend on the random angle of
            // arrivals only
            double sinRayZoa = sin(rayZoaRadian[nIndex][mIndex]);
            double sinRayAoa = cos(rayAoaRadian[nIndex][mIndex]);
            double cosRayAoa = cos(rayAoaRadian[nIndex][mIndex]);
            sinCosA[nIndex][mIndex] = sinRayZoa * cosRayAoa;
            sinSinA[nIndex][mIndex] = sinRayZoa * sinRayAoa;
            cosZoA[nIndex][mIndex] = cos(rayZoaRadian[nIndex][mIndex]);

            // cache the component of the "txPhaseDiff" terms which depend on the random angle of
            // departure only
            double sinRayZod = sin(rayZodRadian[nIndex][mIndex]);
            double sinRayAod = cos(rayAodRadian[nIndex][mIndex]);
            double cosRayAod = cos(rayAodRadian[nIndex][mIndex]);
            sinCosD[nIndex][mIndex] = sinRayZod * cosRayAod;
            sinSinD[nIndex][mIndex] = sinRayZod * sinRayAod;
            cosZoD[nIndex][mIndex] = cos(rayZodRadian[nIndex][mIndex]);
        }
    }

    // The following for loops computes the channel coefficients
    // Keeps track of how many sub-clusters have been added up to now
    uint8_t numSubClustersAdded = 0;
    for (uint8_t nIndex = 0; nIndex < channelParams->m_reducedClusterNumber; nIndex++)
    {
        for (size_t uIndex = 0; uIndex < uSize; uIndex++)
        {
            Vector uLoc = uAntenna->GetElementLocation(uIndex);

            for (size_t sIndex = 0; sIndex < sSize; sIndex++)
            {
                Vector sLoc = sAntenna->GetElementLocation(sIndex);
                // Compute the N-2 weakest cluster, assuming 0 slant angle and a
                // polarization slant angle configured in the array (7.5-22)
                if (nIndex != channelParams->m_cluster1st && nIndex != channelParams->m_cluster2nd)
                {
                    std::complex<double> rays(0, 0);
                    for (uint8_t mIndex = 0; mIndex < table3gpp->m_raysPerCluster; mIndex++)
                    {
                        // lambda_0 is accounted in the antenna spacing uLoc and sLoc.
                        double rxPhaseDiff =
                            2 * M_PI *
                            (sinCosA[nIndex][mIndex] * uLoc.x + sinSinA[nIndex][mIndex] * uLoc.y +
                             cosZoA[nIndex][mIndex] * uLoc.z);

                        double txPhaseDiff =
                            2 * M_PI *
                            (sinCosD[nIndex][mIndex] * sLoc.x + sinSinD[nIndex][mIndex] * sLoc.y +
                             cosZoD[nIndex][mIndex] * sLoc.z);

                        // NOTE Doppler is computed in the CalcBeamformingGain function and is
                        // simplified to only account for the center angle of each cluster.
                        rays += raysPreComp(nIndex, mIndex) *
                                std::complex<double>(cos(rxPhaseDiff), sin(rxPhaseDiff)) *
                                std::complex<double>(cos(txPhaseDiff), sin(txPhaseDiff));
                    }
                    rays *=
                        sqrt(channelParams->m_clusterPower[nIndex] / table3gpp->m_raysPerCluster);
                    hUsn(uIndex, sIndex, nIndex) = rays;
                }
                else //(7.5-28)
                {
                    std::complex<double> raysSub1(0, 0);
                    std::complex<double> raysSub2(0, 0);
                    std::complex<double> raysSub3(0, 0);

                    for (uint8_t mIndex = 0; mIndex < table3gpp->m_raysPerCluster; mIndex++)
                    {
                        // ZML:Just remind me that the angle offsets for the 3 subclusters were not
                        // generated correctly.
                        double rxPhaseDiff =
                            2 * M_PI *
                            (sinCosA[nIndex][mIndex] * uLoc.x + sinSinA[nIndex][mIndex] * uLoc.y +
                             cosZoA[nIndex][mIndex] * uLoc.z);

                        double txPhaseDiff =
                            2 * M_PI *
                            (sinCosD[nIndex][mIndex] * sLoc.x + sinSinD[nIndex][mIndex] * sLoc.y +
                             cosZoD[nIndex][mIndex] * sLoc.z);

                        std::complex<double> raySub =
                            raysPreComp(nIndex, mIndex) *
                            std::complex<double>(cos(rxPhaseDiff), sin(rxPhaseDiff)) *
                            std::complex<double>(cos(txPhaseDiff), sin(txPhaseDiff));

                        switch (mIndex)
                        {
                        case 9:
                        case 10:
                        case 11:
                        case 12:
                        case 17:
                        case 18:
                            raysSub2 += raySub;
                            break;
                        case 13:
                        case 14:
                        case 15:
                        case 16:
                            raysSub3 += raySub;
                            break;
                        default: // case 1,2,3,4,5,6,7,8,19,20
                            raysSub1 += raySub;
                            break;
                        }
                    }
                    raysSub1 *=
                        sqrt(channelParams->m_clusterPower[nIndex] / table3gpp->m_raysPerCluster);
                    raysSub2 *=
                        sqrt(channelParams->m_clusterPower[nIndex] / table3gpp->m_raysPerCluster);
                    raysSub3 *=
                        sqrt(channelParams->m_clusterPower[nIndex] / table3gpp->m_raysPerCluster);
                    hUsn(uIndex, sIndex, nIndex) = raysSub1;
                    hUsn(uIndex,
                         sIndex,
                         channelParams->m_reducedClusterNumber + numSubClustersAdded) = raysSub2;
                    hUsn(uIndex,
                         sIndex,
                         channelParams->m_reducedClusterNumber + numSubClustersAdded + 1) =
                        raysSub3;
                }
            }
        }
        if (nIndex == channelParams->m_cluster1st || nIndex == channelParams->m_cluster2nd)
        {
            numSubClustersAdded += 2;
        }
    }

    if (channelParams->m_losCondition == ChannelCondition::LOS) //(7.5-29) && (7.5-30)
    {
        double lambda = 3.0e8 / m_frequency; // the wavelength of the carrier frequency
        std::complex<double> phaseDiffDueToDistance(cos(-2 * M_PI * distance3D / lambda),
                                                    sin(-2 * M_PI * distance3D / lambda));

        const double sinUAngleIncl = sin(uAngle.GetInclination());
        const double cosUAngleIncl = cos(uAngle.GetInclination());
        const double sinUAngleAz = sin(uAngle.GetAzimuth());
        const double cosUAngleAz = cos(uAngle.GetAzimuth());
        const double sinSAngleIncl = sin(sAngle.GetInclination());
        const double cosSAngleIncl = cos(sAngle.GetInclination());
        const double sinSAngleAz = sin(sAngle.GetAzimuth());
        const double cosSAngleAz = cos(sAngle.GetAzimuth());

        for (size_t uIndex = 0; uIndex < uSize; uIndex++)
        {
            Vector uLoc = uAntenna->GetElementLocation(uIndex);
            double rxPhaseDiff = 2 * M_PI *
                                 (sinUAngleIncl * cosUAngleAz * uLoc.x +
                                  sinUAngleIncl * sinUAngleAz * uLoc.y + cosUAngleIncl * uLoc.z);

            for (size_t sIndex = 0; sIndex < sSize; sIndex++)
            {
                Vector sLoc = sAntenna->GetElementLocation(sIndex);
                std::complex<double> ray(0, 0);
                double txPhaseDiff =
                    2 * M_PI *
                    (sinSAngleIncl * cosSAngleAz * sLoc.x + sinSAngleIncl * sinSAngleAz * sLoc.y +
                     cosSAngleIncl * sLoc.z);

                auto [rxFieldPatternPhi, rxFieldPatternTheta] = uAntenna->GetElementFieldPattern(
                    Angles(uAngle.GetAzimuth(), uAngle.GetInclination()));
                auto [txFieldPatternPhi, txFieldPatternTheta] = sAntenna->GetElementFieldPattern(
                    Angles(sAngle.GetAzimuth(), sAngle.GetInclination()));

                ray = (rxFieldPatternTheta * txFieldPatternTheta -
                       rxFieldPatternPhi * txFieldPatternPhi) *
                      phaseDiffDueToDistance *
                      std::complex<double>(cos(rxPhaseDiff), sin(rxPhaseDiff)) *
                      std::complex<double>(cos(txPhaseDiff), sin(txPhaseDiff));

                double kLinear = pow(10, channelParams->m_K_factor / 10.0);
                // the LOS path should be attenuated if blockage is enabled.
                hUsn(uIndex, sIndex, 0) =
                    sqrt(1.0 / (kLinear + 1)) * hUsn(uIndex, sIndex, 0) +
                    sqrt(kLinear / (1 + kLinear)) * ray /
                        pow(10,
                            channelParams->m_attenuation_dB[0] / 10.0); //(7.5-30) for tau = tau1
                for (uint16_t nIndex = 1; nIndex < hUsn.GetNumPages(); nIndex++)
                {
                    hUsn(uIndex, sIndex, nIndex) *=
                        sqrt(1.0 / (kLinear + 1)); //(7.5-30) for tau = tau2...tauN
                }
            }
        }
    }

    NS_LOG_DEBUG("Husn (sAntenna, uAntenna):" << sAntenna->GetId() << ", " << uAntenna->GetId());
    for (uint16_t cIndex = 0; cIndex < hUsn.GetNumPages(); cIndex++)
    {
        for (uint16_t rowIdx = 0; rowIdx < hUsn.GetNumRows(); rowIdx++)
        {
            for (uint16_t colIdx = 0; colIdx < hUsn.GetNumCols(); colIdx++)
            {
                NS_LOG_DEBUG(" " << hUsn(rowIdx, colIdx, cIndex) << ",");
            }
        }
    }

    NS_LOG_INFO("size of coefficient matrix (rows, columns, clusters) = ("
                << hUsn.GetNumRows() << ", " << hUsn.GetNumCols() << ", " << hUsn.GetNumPages()
                << ")");
    channelMatrix->m_channel = hUsn;
    return channelMatrix;
}

} // namespace ns3
