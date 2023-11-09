#include "nr-mimo-matrices.h"

#include <ns3/fatal-error.h>

namespace ns3
{

NrIntfNormChanMat
NrCovMat::CalcIntfNormChannelMimo([[maybe_unused]] const ComplexMatrixArray& chanMat) const
{
    NS_FATAL_ERROR("MIMO channel normalization requires Eigen matrix library.");
}

ComplexMatrixArray
NrIntfNormChanMat::ComputeMseMimo([[maybe_unused]] const ComplexMatrixArray& precMats) const
{
    NS_FATAL_ERROR("MIMO MSE computation requires Eigen matrix library.");
}

} // namespace ns3
