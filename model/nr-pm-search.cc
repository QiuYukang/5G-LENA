// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-pm-search.h"

#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/enum.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPmSearch");
NS_OBJECT_ENSURE_REGISTERED(NrPmSearch);

TypeId
NrPmSearch::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrPmSearch")
            .SetParent<Object>()
            .AddAttribute("RankLimit",
                          "Max MIMO rank is minimum of num UE ports, num gNB ports, and RankLimit",
                          UintegerValue(UINT8_MAX),
                          MakeUintegerAccessor(&NrPmSearch::m_rankLimit),
                          MakeUintegerChecker<uint8_t>(1, UINT8_MAX))
            .AddAttribute("RankTechnique",
                          "Technique used to determine the MIMO Rank",
                          EnumValue(NrPmSearch::RankTechnique::SVD),
                          MakeEnumAccessor<RankTechnique>(&NrPmSearch::m_rankTechnique),
                          MakeEnumChecker(RankTechnique::SVD,
                                          "SVD",
                                          RankTechnique::WaterFilling,
                                          "WaterFilling",
                                          RankTechnique::Sasaoka,
                                          "Sasaoka"))
            .AddAttribute("RankThreshold",
                          "Rank threshold for SVD selection",
                          DoubleValue(std::numeric_limits<double>::epsilon()),
                          MakeDoubleAccessor(&NrPmSearch::m_rankThreshold),
                          MakeDoubleChecker<double>())
            .AddAttribute("SubbandSize",
                          "Size of subband in PRBs for downsampling",
                          UintegerValue(1),
                          MakeUintegerAccessor(&NrPmSearch::m_subbandSize),
                          MakeUintegerChecker<uint8_t>(1, 32))
            .AddAttribute("EnforceSubbandSize",
                          "Enforce 3GPP standardized sub-band size",
                          BooleanValue(true),
                          MakeBooleanAccessor(&NrPmSearch::m_enforceSubbandSize),
                          MakeBooleanChecker())
            .AddAttribute(
                "DownsamplingTechnique",
                "Algorithm used to downsample PRBs into SBs",
                EnumValue(NrPmSearch::DownsamplingTechnique::FirstPRB),
                MakeEnumAccessor<DownsamplingTechnique>(&NrPmSearch::m_downsamplingTechnique),
                MakeEnumChecker(DownsamplingTechnique::FirstPRB,
                                "FirstPRB",
                                DownsamplingTechnique::RandomPRB,
                                "RandomPRB",
                                DownsamplingTechnique::AveragePRB,
                                "AveragePRB"))
            .AddAttribute("SubbandCqiClamping",
                          "Clamp sub-band CQI range to wideband CQI [-1,+2], according to 3GPP "
                          "2-bit information overhead limit",
                          BooleanValue(true),
                          MakeBooleanAccessor(&NrPmSearch::m_subbandCqiClamping),
                          MakeBooleanChecker());
    return tid;
}

NrPmSearch::NrPmSearch()
    : m_downsamplingUniRand(CreateObject<UniformRandomVariable>())
{
}

void
NrPmSearch::SetAmc(Ptr<const NrAmc> amc)
{
    m_amc = amc;
}

void
NrPmSearch::SetGnbParams(bool isDualPol, size_t numHPorts, size_t numVPorts)
{
    m_nGnbPorts = isDualPol ? 2 * numHPorts * numVPorts : numHPorts * numVPorts;
    m_isGnbDualPol = isDualPol;
    m_nGnbHPorts = numHPorts;
    m_nGnbVPorts = numVPorts;
}

void
NrPmSearch::SetUeParams(size_t numTotalPorts)
{
    m_nRxPorts = numTotalPorts;
}

void
NrPmSearch::SetSubbandSize(size_t subbandSize)
{
    m_subbandSize = subbandSize;
}

size_t
NrPmSearch::GetSubbandSize() const
{
    return m_subbandSize;
}

NrIntfNormChanMat
NrPmSearch::SubbandDownsampling(const NrIntfNormChanMat& channelMatrix)
{
    size_t prbs = channelMatrix.GetNumPages();

    // Check if subband size is allowed for bandwidth
    // 3GPP TS 38.214 Table 5.2.1.4-2
    if (m_enforceSubbandSize)
    {
        if (prbs < 24)
        {
            NS_ASSERT_MSG(m_subbandSize == 1,
                          "Bandwidth parts with less than 24 PRBs should have subbands of size 1");
        }
        else if (prbs >= 24 && prbs <= 72)
        {
            NS_ASSERT_MSG(
                m_subbandSize == 4 || m_subbandSize == 8,
                "Bandwidth parts with 24<=x<=72 PRBs should have subbands of size 4 or 8");
        }
        else if (prbs >= 73 && prbs <= 144)
        {
            NS_ASSERT_MSG(
                m_subbandSize == 8 || m_subbandSize == 16,
                "Bandwidth parts with 73<=x<=144 PRBs should have subbands of size 8 or 16");
        }
        else if (prbs >= 145 && prbs <= 275)
        {
            NS_ASSERT_MSG(
                m_subbandSize == 16 || m_subbandSize == 32,
                "Bandwidth parts with 145<=x<=275 PRBs should have subbands of size 16 or 32");
        }
        else
        {
            NS_ASSERT_MSG(m_subbandSize == 32,
                          "Bandwidth parts with >275 PRBs should have subbands of size 32");
        }
    }

    if (m_subbandSize == 1)
    {
        return channelMatrix;
    }

    // Calculate number of subbands
    size_t nSubbands = GetNumSubbands(channelMatrix);

    // Preallocate resulting matrix
    ComplexMatrixArray subbandChannelMatrix(channelMatrix.GetNumRows(),
                                            channelMatrix.GetNumCols(),
                                            nSubbands);

    switch (m_downsamplingTechnique)
    {
    case DownsamplingTechnique::FirstPRB:
        GetSubbandDownsampleFirstPrb(channelMatrix, subbandChannelMatrix);
        break;
    case DownsamplingTechnique::RandomPRB:
        GetSubbandDownsampleRandomPrb(channelMatrix, subbandChannelMatrix);
        break;
    case DownsamplingTechnique::AveragePRB:
        GetSubbandDownsampleAveragePrb(channelMatrix, subbandChannelMatrix);
        break;
    default:
        NS_ABORT_MSG("Unknown downsampling algorithm");
    }
    return subbandChannelMatrix;
}

size_t
NrPmSearch::GetNumSubbands(const NrIntfNormChanMat& chanMat) const
{
    size_t prbs = chanMat.GetNumPages();
    size_t nSubbands = prbs / m_subbandSize;
    int prbsLastSubband = prbs > m_subbandSize ? prbs % m_subbandSize : 0;
    nSubbands += prbsLastSubband > 0;
    return nSubbands;
}

void
NrPmSearch::GetSubbandDownsampleFirstPrb(const NrIntfNormChanMat& chanMat,
                                         ComplexMatrixArray& downsampledChanMat) const
{
    size_t matSize = chanMat.GetNumRows() * chanMat.GetNumCols();
    size_t nSubbands = downsampledChanMat.GetNumPages();
    for (size_t page = 0; page < nSubbands; page++)
    {
        auto sbPage = downsampledChanMat.GetPagePtr(page);
        auto prbPage = chanMat.GetPagePtr(page * m_subbandSize);
        for (size_t i = 0; i < matSize; i++)
        {
            sbPage[i] = prbPage[i];
        }
    }
}

void
NrPmSearch::GetSubbandDownsampleRandomPrb(const NrIntfNormChanMat& chanMat,
                                          ComplexMatrixArray& downsampledChanMat) const
{
    size_t matSize = chanMat.GetNumRows() * chanMat.GetNumCols();
    size_t nSubbands = downsampledChanMat.GetNumPages();
    size_t prbsLastSubband = chanMat.GetNumPages() % m_subbandSize;
    for (size_t page = 0; page < nSubbands; page++)
    {
        auto sbPage = downsampledChanMat.GetPagePtr(page);
        bool lastPageIsSmaller = prbsLastSubband && page == nSubbands - 1;
        auto prbsInSubband = lastPageIsSmaller ? prbsLastSubband : m_subbandSize;
        NS_ASSERT_MSG(prbsInSubband > 0,
                      "prbsInSubband should be either size of sub-band or its modulus");
        auto randomPrb = m_downsamplingUniRand->GetInteger(0, prbsInSubband - 1);
        auto prbPage = chanMat.GetPagePtr(page * m_subbandSize + randomPrb);
        for (size_t i = 0; i < matSize; i++)
        {
            sbPage[i] = prbPage[i];
        }
    }
}

void
NrPmSearch::GetSubbandDownsampleAveragePrb(const NrIntfNormChanMat& chanMat,
                                           ComplexMatrixArray& downsampledChanMat) const
{
    size_t matSize = chanMat.GetNumRows() * chanMat.GetNumCols();
    size_t nSubbands = downsampledChanMat.GetNumPages();
    size_t prbsLastSubband = chanMat.GetNumPages() % m_subbandSize;
    auto bandSize = m_subbandSize;
    for (size_t page = 0; page < nSubbands; page++)
    {
        // Use full subband size until we hit the last subband, which may be shorter
        if (page == (nSubbands - 1) && prbsLastSubband)
        {
            bandSize = prbsLastSubband;
        }
        // Retrieve downsampled subband pointer
        auto sbPage = downsampledChanMat.GetPagePtr(page);
        for (size_t sbPrb = 0; sbPrb < bandSize; sbPrb++)
        {
            // For each original PRB, accumulate the values
            auto prbPage = chanMat.GetPagePtr(page * m_subbandSize + sbPrb);
            for (size_t i = 0; i < matSize; i++)
            {
                sbPage[i] += prbPage[i];
            }
        }
        // Divide the accumulated values per the number of PRBs
        for (size_t i = 0; i < matSize; i++)
        {
            sbPage[i] /= bandSize;
        }
    }
}

NrIntfNormChanMat
NrPmSearch::SubbandUpsampling(const NrIntfNormChanMat& precMat, size_t numPrbs) const
{
    if (m_subbandSize == 1)
    {
        return precMat;
    }
    auto upsampledMatrix = ComplexMatrixArray(precMat.GetNumRows(), precMat.GetNumCols(), numPrbs);

    for (size_t rb = 0; rb < numPrbs; rb++)
    {
        auto rbPage = upsampledMatrix.GetPagePtr(rb);
        auto sbPage = precMat.GetPagePtr(floor(rb / m_subbandSize));

        for (size_t i = 0; i < upsampledMatrix.GetNumRows() * upsampledMatrix.GetNumCols(); i++)
        {
            rbPage[i] = sbPage[i];
        }
    }
    return upsampledMatrix;
}

int64_t
NrPmSearch::AssignStreams(int64_t streamNum)
{
    m_downsamplingUniRand->SetStream(streamNum);
    return 1;
}

uint8_t
NrPmSearch::SelectRank(NrIntfNormChanMat& channelMatrix) const
{
    uint8_t minRank = 1;
    uint8_t maxRank = 0;

    // First determine the maximum supported rank
    switch (m_rankTechnique)
    {
    case RankTechnique::SVD:
        maxRank = channelMatrix.GetEigenWidebandRank(m_rankThreshold);
        break;
    case RankTechnique::WaterFilling:
        maxRank = channelMatrix.GetWaterfillingWidebandRank(*m_ranks.rbegin(), m_rankThreshold);
        break;
    case RankTechnique::Sasaoka:
        maxRank = channelMatrix.GetSasaokaWidebandRank();
        break;
    default:
        NS_ABORT_MSG("Unknown rank technique");
    }

    maxRank = std::max(minRank, maxRank);
    // If our maxRank is not supported, then take the maximum supported
    // todo: find nearest rank, rounding downwards
    if (std::lower_bound(m_ranks.begin(), m_ranks.end(), maxRank) == m_ranks.end())
    {
        maxRank = *m_ranks.rbegin();
    }
    return maxRank;
}

} // namespace ns3
