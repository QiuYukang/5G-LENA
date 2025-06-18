// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mimo-matrices.h"

#include "ns3/matrix-array.h"

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <Eigen/Dense>
#include <Eigen/SVD>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace ns3
{

template <class T>
using EigenMatrix = Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>;
template <class T>
using ConstEigenMatrix = Eigen::Map<const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>;

NrIntfNormChanMat
NrCovMat::CalcIntfNormChannelMimo(const ComplexMatrixArray& chanMat) const
{
    auto res = NrIntfNormChanMat{
        ComplexMatrixArray{chanMat.GetNumRows(), chanMat.GetNumCols(), chanMat.GetNumPages()}};
    for (size_t iRb = 0; iRb < chanMat.GetNumPages(); iRb++)
    {
        ConstEigenMatrix<std::complex<double>> covMatEigen(GetPagePtr(iRb),
                                                           GetNumRows(),
                                                           GetNumCols());
        ConstEigenMatrix<std::complex<double>> chanMatEigen(chanMat.GetPagePtr(iRb),
                                                            chanMat.GetNumRows(),
                                                            chanMat.GetNumCols());
        EigenMatrix<std::complex<double>> resEigen(res.GetPagePtr(iRb),
                                                   res.GetNumRows(),
                                                   res.GetNumCols());
        auto covMat = covMatEigen.selfadjointView<Eigen::Upper>();
        resEigen = covMat.llt().matrixL().solve(chanMatEigen);
    }
    return res;
}

ComplexMatrixArray
NrIntfNormChanMat::ComputeMseMimo(const ComplexMatrixArray& precMats) const
{
    auto nDims = precMats.GetNumCols();
    auto identity = Eigen::MatrixXcd::Identity(nDims, nDims);
    auto res = ComplexMatrixArray{nDims, nDims, precMats.GetNumPages()};
    auto chanPrec = (*this) * precMats;
    auto chanCov = chanPrec.HermitianTranspose() * chanPrec;
    for (size_t iRb = 0; iRb < res.GetNumPages(); iRb++)
    {
        ConstEigenMatrix<std::complex<double>> chanCovEigen(chanCov.GetPagePtr(iRb),
                                                            chanCov.GetNumRows(),
                                                            chanCov.GetNumCols());
        EigenMatrix<std::complex<double>> resEigen(res.GetPagePtr(iRb),
                                                   res.GetNumRows(),
                                                   res.GetNumCols());
        Eigen::MatrixXcd temp = chanCovEigen + identity;
        resEigen = temp.selfadjointView<Eigen::Lower>().llt().solve(identity);
    }
    return res;
}

uint8_t
NrIntfNormChanMat::GetSasaokaWidebandRank() const
{
    // Extract eigenvalues for each subband
    std::vector<std::vector<double>> subbandRankEigenval(m_numPages);
    for (std::size_t subband = 0; subband < m_numPages; subband++)
    {
        ConstEigenMatrix<std::complex<double>> HEigen(this->GetPagePtr(subband),
                                                      this->GetNumRows(),
                                                      this->GetNumCols());
        auto eigenvalues = HEigen.eigenvalues();

        // Store them in descending order
        subbandRankEigenval[subband].resize(eigenvalues.size());
        for (int64_t i = 0; i < eigenvalues.size(); i++)
        {
            subbandRankEigenval[subband][i] =
                std::abs(eigenvalues.coeff(eigenvalues.size() - i - 1));
        }
    }
    // Calculate capacity increment for each rank
    int minDim = std::min(m_numCols, m_numRows);
    std::vector<double> rankCapacityIncrease(minDim);
    uint8_t selectedRank = 0;
    // Found via linear regression using scikitlearn by comparing outputs against full search
    std::array<double, 4> coeffs{0., 1.84181129, 0.11705455, 1.39847256};
    for (int rank = 1; rank <= minDim; rank++)
    {
        auto cap = 0.0;
        for (std::size_t subband = 0; subband < m_numPages; subband++)
        {
            cap += log2(1 + subbandRankEigenval[subband][rank - 1] / rank);
        }
        rankCapacityIncrease[rank - 1] = cap / m_numPages;
    }
    // Check if higher than threshold obtained by a calibrated function alpha(rank)
    double rankD = 1.0;
    for (int rank = 1; rank <= minDim; rank++)
    {
        rankD += (rankCapacityIncrease[rank - 1] / rankCapacityIncrease[0]) * coeffs[rank - 1];
    }
    selectedRank = round(rankD);
    return selectedRank;
}

uint8_t
NrIntfNormChanMat::GetWaterfillingWidebandRank(uint8_t maxRank, double thr) const
{
    // Compute the rank via SVD decomposition
    ConstEigenMatrix<std::complex<double>> HEigen(this->GetPagePtr(0),
                                                  this->GetNumRows(),
                                                  this->GetNumCols());
    auto eigenvalues = HEigen.eigenvalues();

    NS_ASSERT_MSG(maxRank >= 1, "maxRank should be equal or greater to 1");
    uint8_t bestRank = 0;
    double bestRankCapacity = 0.0;
    for (uint8_t rankI = 1; rankI <= maxRank; rankI++)
    {
        double capacity = 0.0;
        for (uint8_t stream = 0; stream < rankI; stream++)
        {
            // Eigen values are sorted in increasing order, but we need to compute in inverse order
            capacity +=
                log2(1 + std::abs(eigenvalues(eigenvalues.rows() - stream - 1, 0)) / rankI / thr);
        }
        if (capacity >= bestRankCapacity)
        {
            bestRank = rankI;
            bestRankCapacity = capacity;
        }
    }
    return bestRank;
}

uint8_t
NrIntfNormChanMat::GetEigenWidebandRank(double thr) const
{
    if (thr == 0.0)
    {
        thr = std::numeric_limits<double>::epsilon();
    }

    // Compute the rank via SVD decomposition
    ConstEigenMatrix<std::complex<double>> HEigen(this->GetPagePtr(0),
                                                  this->GetNumRows(),
                                                  this->GetNumCols());

    auto svd = HEigen.jacobiSvd();

    // Set value threshold to limit rank search
    svd.setThreshold(thr);
    return svd.rank();
}

std::vector<uint8_t>
NrIntfNormChanMat::GetEigenSubbandRanks(double thr) const
{
    std::vector<uint8_t> ranks;
    for (size_t iRb = 0; iRb < this->GetNumPages(); iRb++)
    {
        // Compute the rank via SVD decomposition
        ConstEigenMatrix<std::complex<double>> HEigen(this->GetPagePtr(iRb),
                                                      this->GetNumRows(),
                                                      this->GetNumCols());
        auto svd = HEigen.jacobiSvd();

        // Set value threshold to limit rank search
        svd.setThreshold(thr);
        ranks.push_back(svd.rank());
    }
    return ranks;
}

ComplexMatrixArray
NrIntfNormChanMat::ExtractOptimalPrecodingMatrices(uint8_t rank) const
{
    NS_ASSERT_MSG(rank > 0, "Rank should be greater than 0");
    ComplexMatrixArray optPrecoders(this->GetNumRows(), rank, this->GetNumPages());
    for (size_t iRb = 0; iRb < m_numPages; iRb++)
    {
        ConstEigenMatrix<std::complex<double>> H(this->GetPagePtr(iRb),
                                                 this->GetNumRows(),
                                                 this->GetNumCols());

        auto svd = H.jacobiSvd(Eigen::ComputeFullV);
        auto V = svd.matrixV();
        for (size_t i = 0; i < m_numRows; i++)
        {
            for (size_t j = 0; j < rank; j++)
            {
                optPrecoders(i, j, iRb) = V(i, j);
            }
        }
    }
    return optPrecoders;
}

} // namespace ns3
