// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NS3_PARSE_STRING_TO_VECTOR_H
#define NS3_PARSE_STRING_TO_VECTOR_H

#include <string>
#include <vector>

namespace ns3
{
/***
 * @brief Parse list of values separated by | and transform into double vector
 * @param verticalBarString String with values separated by vertical bar e.g. 0|10|20
 * @return vector with values casted to double
 */
std::vector<double> ParseVBarSeparatedValuesStringToVector(std::string verticalBarString);

} // namespace ns3
#endif // NS3_PARSE_STRING_TO_VECTOR_H
