// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mimo-matrices.h"

#include "ns3/matrix-array.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#include <Eigen/Dense>
#include <Eigen/SVD>
#pragma GCC diagnostic pop

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
