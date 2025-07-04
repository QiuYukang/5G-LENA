// Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "parse-string-to-vector.h"

#include <regex>

namespace ns3
{

std::vector<double>
ParseVBarSeparatedValuesStringToVector(std::string verticalBarString)
{
    static const std::regex delim{R"(\|)"};
    std::sregex_token_iterator first{verticalBarString.begin(), verticalBarString.end(), delim, -1};
    std::sregex_token_iterator last;

    std::vector<double> out;
    out.reserve(std::distance(first, last));

    for (auto it = first; it != last; ++it)
    {
        out.push_back(std::stod(it->str()));
    }
    return out;
}

} // namespace ns3
