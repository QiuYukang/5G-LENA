/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2019 CTTC Sandra Lagen  <sandra.lagen@cttc.es>
*   Copyright (c) 2019 Interdigital <kevin.wanuga@interdigital.com> tables
*   BlerForSinr1 and BlerForSinr2
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

#include "nr-eesm-error-model.h"
#include "ns3/log.h"
#include <cmath>
#include <algorithm>
#include "ns3/enum.h"
#include "mmwave-phy-mac-common.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrEesmErrorModel");
NS_OBJECT_ENSURE_REGISTERED (NrEesmErrorModel);

/**
 * \brief Table of beta values for each standard MCS in Table1 in TS38.214
 */
static const std::vector<double> BetaTable1 = {
  1.1544, 1.1813, 1.2075, 1.2498, 1.2913, 1.3430, 1.3939, 1.45, 1.5053, 1.5614,
  2.9764, 3.2740, 3.7125, 4.1509, 4.6442, 5.1375, 5.4664, 7.9177, 9.0798,
  10.9915, 12.7727, 14.5723, 16.5644, 18.9099, 21.5072, 24.1479, 26.9422,
  28.9536, 30.9325
};

/**
 * \brief Table of beta values for each standard MCS in Table2 in TS38.214
 */
static const std::vector<double> BetaTable2 = {
  1.1544, 1.2075, 1.2963, 1.3939, 1.5053, 3.2740, 3.7125, 4.1509, 4.6442, 5.1375,
  5.4664, 9.0798, 10.9915, 12.7727, 14.5723, 16.5644, 18.9099, 21.5072, 24.1479,
  26.9422, 52.9467, 58.9117, 68.5736, 78.9416, 90.1368, 101.7340, 110.1554,
  118.5677
};

/**
 * \brief Table of ECR of the standard MCSs: 29 MCSs as per Table1 in TS38.214
 */
static const std::vector<double> McsEcrTable1 = {
  // QPSK (M=2)
  0.12, 0.15, 0.19, 0.25, 0.30, 0.37, 0.44, 0.51, 0.59, 0.66, // ECRs of MCSs
  // 16QAM (M=4)
  0.33, 0.37, 0.42, 0.48, 0.54, 0.60, 0.64, // ECRs of MCSs
  // 64QAM (M=6)
  0.43, 0.46, 0.50, 0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.89, 0.93 // ECRs of MCSs
};

/**
 * \brief Table of ECR of the standard MCSs: 28 MCSs as per Table2 in TS38.214
 */
static const std::vector<double> McsEcrTable2 = {
  // QPSK (M=2)
  0.12, 0.19, 0.30, 0.44, 0.59, // ECRs of MCSs
  // 16QAM (M=4)
  0.37, 0.42, 0.48, 0.54, 0.60, 0.64, // ECRs of MCSs
  // 64QAM (M=6)
  0.46, 0.50, 0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, // ECRs of MCSs
  // 256QAM (M=8)
  0.67, 0.69, 0.74, 0.77, 0.82, 0.86, 0.90, 0.93 // ECRs of MCSs
};

/**
 * \brief Table of modulation order of the standard MCSs: 29 MCSs as per Table1
 * in TS38.214
 */
static const std::vector<uint8_t> McsMTable1 = {
  // QPSK (M=2)
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  // 16QAM (M=4)
  4, 4, 4, 4, 4, 4, 4,
  // 64QAM (M=6)
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
};


/**
 * \brief Table of modulation order of the standard MCSs: 28 MCSs as per Table2
 * in TS38.214
 */
static const std::vector<uint8_t> McsMTable2 = {
  // QPSK (M=2)
  2, 2, 2, 2, 2,
  // 16QAM (M=4)
  4, 4, 4, 4, 4, 4,
  // 64QAM (M=6)
  6, 6, 6, 6, 6, 6, 6, 6, 6,
  // 256QAM (M=8)
  8, 8, 8, 8, 8, 8, 8, 8
};

/**
 * \brief Table of Lifting Sizes for LDPC
 */
static const std::vector<uint16_t> LiftingSizeTableBG = {
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24, 26, 28, 30,
  32, 36, 40, 44, 48, 52, 56, 60, 64, 72, 80, 88, 96, 104, 112, 120, 128, 144,
  160, 176, 192, 208, 224, 240, 256, 288, 320, 352, 384
};

/**
 * \brief Table of SE of the standard MCSs: 29 (0 to 28) MCSs as per Table1 in TS38.214
 */
static const std::vector<double> SpectralEfficiencyForMcs1 = {
  // QPSK (M=2)
  0.23, 0.31, 0.38, 0.49, 0.6, 0.74, 0.88, 1.03, 1.18, 1.33, // SEs of MCSs
  // 16QAM (M=4)
  1.33, 1.48, 1.70, 1.91, 2.16, 2.41, 2.57, // SEs of MCSs
  // 64QAM (M=6)
  2.57, 2.73, 3.03, 3.32, 3.61, 3.90, 4.21, 4.52, 4.82, 5.12, 5.33, 5.55  // SEs of MCSs
};

/**
 * \brief Table of SE of the standard MCSs: 28 (0 to 27) MCSs as per Table2 in TS38.214
 */
static const std::vector<double> SpectralEfficiencyForMcs2 = {
  // QPSK (M=2)
  0.23, 0.38, 0.60, 0.88, 1.18, // SEs of MCSs
  // 16QAM (M=4)
  1.48, 1.70, 1.91, 2.16, 2.41, 2.57, // SEs of MCSs
  // 64QAM (M=6)
  2.73, 3.03, 3.32, 3.61, 3.90, 4.21, 4.52, 4.82, 5.12, // SEs of MCSs
  // 256QAM (M=8)
  5.33, 5.55, 5.89, 6.23, 6.57, 6.91, 7.16, 7.41 // SEs of MCSs
};

/**
 * \brief Table of SE of the standard CQIs: 16 CQIs as per Table1 in TS38.214
 */
static const std::vector<double> SpectralEfficiencyForCqi1 = {
  0.0,     // out of range
  0.15, 0.23, 0.38, 0.6, 0.88, 1.18,
  1.48, 1.91, 2.41,
  2.73, 3.32, 3.9, 4.52, 5.12, 5.55
};

/**
 * \brief Table of SE of the standard CQIs: 16 CQIs as per Table2 in TS38.214
 */
static const std::vector<double> SpectralEfficiencyForCqi2 = {
  0.0,     // out of range
  0.15, 0.38, 0.88,
  1.48, 1.91, 2.41,
  2.73, 3.32, 3.90, 4.52, 5.12,
  5.55, 6.23, 6.91, 7.41
};


/**
 * \brief SINR to BLER mapping for MCSs in Table1
 */
static const NrEesmErrorModel::SimulatedBlerFromSINR BlerForSinr1 = {
{ // BG TYPE 1
  { // MCS 0
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 1
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 2
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 3
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 4
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 5.300000e-01, 1.100000e+00, 1.680000e+00, 2.250000e+00, 2.830000e+00, 3.400000e+00 }, // SINR
          { 9.817308e-01, 8.835616e-01, 5.481602e-01, 1.601911e-01, 1.900000e-02, 1.000000e-03 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 5.300000e-01, 1.100000e+00, 1.680000e+00, 2.250000e+00, 2.830000e+00, 3.400000e+00 }, // SINR
          { 9.855769e-01, 8.470395e-01, 4.655331e-01, 1.021483e-01, 7.300000e-03, 5.000000e-04 } // BLER
        }
      },
      { 4224U, // SINR and BLER for CBS 4224
        NrEesmErrorModel::DoubleTuple{
          { 8.300000e-01, 1.200000e+00, 1.580000e+00, 1.950000e+00, 2.330000e+00, 2.700000e+00, 3.080000e+00, 3.450000e+00, 3.830000e+00 }, // SINR
          { 9.393116e-01, 8.392857e-01, 5.337553e-01, 2.614108e-01, 7.540000e-02, 1.220000e-02, 2.200000e-03, 6.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 8.300000e-01, 1.200000e+00, 1.580000e+00, 1.950000e+00, 2.330000e+00, 2.700000e+00, 3.080000e+00, 3.450000e+00 }, // SINR
          { 9.661654e-01, 8.924825e-01, 6.649485e-01, 3.439208e-01, 1.132671e-01, 2.410000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.150000e+00, 1.400000e+00, 1.650000e+00, 1.900000e+00, 2.200000e+00, 2.400000e+00, 2.700000e+00, 2.900000e+00 }, // SINR
          { 9.163732e-01, 8.486842e-01, 6.861702e-01, 1.869420e-01, 3.660000e-02, 9.000000e-03, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 9.100000e-01, 1.240000e+00, 1.570000e+00, 1.900000e+00, 2.200000e+00, 2.600000e+00, 2.900000e+00 }, // SINR
          { 9.807692e-01, 8.715986e-01, 6.649485e-01, 1.657359e-01, 2.900000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 6144U, // SINR and BLER for CBS 6144
        NrEesmErrorModel::DoubleTuple{
          { 1.100000e+00, 1.400000e+00, 1.700000e+00, 2, 2.300000e+00, 2.600000e+00, 2.900000e+00, 3.200000e+00 }, // SINR
          { 9.216549e-01, 7.067308e-01, 4.635036e-01, 2.830717e-01, 5.900000e-02, 4.600000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.400000e+00, 1.600000e+00, 1.900000e+00, 2.200000e+00, 2.500000e+00, 2.780000e+00, 3.050000e+00 }, // SINR
          { 9.498175e-01, 8.081761e-01, 3.753698e-01, 8.440000e-02, 7.700000e-03, 1.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 7168U, // SINR and BLER for CBS 7168
        NrEesmErrorModel::DoubleTuple{
          { 1.200000e+00, 1.400000e+00, 1.600000e+00, 1.700000e+00, 1.900000e+00, 2.070000e+00, 2.250000e+00, 2.430000e+00, 2.600000e+00, 2.780000e+00, 2.950000e+00, 3.130000e+00, 3.300000e+00 }, // SINR
          { 9.799618e-01, 9.084507e-01, 7.183989e-01, 6.020047e-01, 3.095966e-01, 1.206731e-01, 5.460000e-02, 2.400000e-02, 7.700000e-03, 3.000000e-03, 5.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 8064U, // SINR and BLER for CBS 8064
        NrEesmErrorModel::DoubleTuple{
          { 8.000000e-01, 1.500000e+00, 2.300000e+00, 3.100000e+00 }, // SINR
          { 1, 8.741497e-01, 2.540000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 5
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.660000e+00, 1.930000e+00, 2.200000e+00, 2.500000e+00, 2.700000e+00, 3, 3.300000e+00 }, // SINR
          { 9.799618e-01, 8.975694e-01, 6.042654e-01, 2.064860e-01, 6.650000e-02, 5.600000e-03, 2.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.600000e+00, 2.080000e+00, 2.550000e+00, 3.030000e+00, 3.500000e+00, 3.980000e+00, 4.450000e+00, 4.930000e+00, 5.400000e+00, 5.880000e+00 }, // SINR
          { 9.671053e-01, 8.657718e-01, 6.662304e-01, 3.688227e-01, 1.478873e-01, 3.440000e-02, 5.400000e-03, 7.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.600000e+00, 2.080000e+00, 2.550000e+00, 3.030000e+00, 3.500000e+00, 3.980000e+00, 4.450000e+00, 4.930000e+00 }, // SINR
          { 9.540441e-01, 8.233871e-01, 5.328125e-01, 2.191304e-01, 5.580000e-02, 7.200000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 2.070000e+00, 2.300000e+00, 2.500000e+00, 2.700000e+00, 3, 3.200000e+00, 3.430000e+00, 3.650000e+00 }, // SINR
          { 9.166667e-01, 8.117188e-01, 5.700673e-01, 2.939977e-01, 5.820000e-02, 1.080000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.500000e+00, 2.250000e+00, 3, 3.750000e+00, 4.500000e+00, 5.250000e+00 }, // SINR
          { 9.734848e-01, 8.081761e-01, 3.883792e-01, 7.230000e-02, 3.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.600000e+00, 2, 2.400000e+00, 2.800000e+00, 3.200000e+00, 3.600000e+00, 4, 4.400000e+00 }, // SINR
          { 9.356884e-01, 8.081761e-01, 5.220287e-01, 2.137990e-01, 4.980000e-02, 6.500000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.400000e+00, 2.200000e+00, 3, 3.800000e+00, 4.600000e+00 }, // SINR
          { 9.790076e-01, 7.565789e-01, 2.305403e-01, 1.740000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6400U, // SINR and BLER for CBS 6400
        NrEesmErrorModel::DoubleTuple{
          { 1.900000e+00, 2.470000e+00, 2.700000e+00, 3.030000e+00, 3.600000e+00 }, // SINR
          { 9.865385e-01, 6.078199e-01, 2.061475e-01, 4.820000e-02, 0 } // BLER
        }
      },
      { 7552U, // SINR and BLER for CBS 7552
        NrEesmErrorModel::DoubleTuple{
          { 1.700000e+00, 2.100000e+00, 2.600000e+00, 3, 3.400000e+00, 3.820000e+00, 4.250000e+00, 4.680000e+00, 5.100000e+00, 5.530000e+00 }, // SINR
          { 9.564815e-01, 8.616667e-01, 6.342822e-01, 3.883792e-01, 1.863872e-01, 5.780000e-02, 1.180000e-02, 1.800000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.800000e+00, 2.300000e+00, 2.700000e+00, 3.200000e+00, 3.600000e+00, 4.050000e+00, 4.500000e+00, 4.950000e+00, 5.400000e+00, 5.850000e+00 }, // SINR
          { 9.365942e-01, 7.914110e-01, 5.627753e-01, 2.795254e-01, 1.118889e-01, 2.880000e-02, 3.900000e-03, 7.000000e-04, 1.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 6
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 2.750000e+00, 3, 3.300000e+00, 3.500000e+00, 3.800000e+00, 4, 4.250000e+00 }, // SINR
          { 9.680451e-01, 8.205128e-01, 5.226337e-01, 2.182642e-01, 7.820000e-02, 1.060000e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.750000e+00, 3, 3.300000e+00, 3.500000e+00, 3.800000e+00, 4, 4.250000e+00, 4.500000e+00 }, // SINR
          { 9.080357e-01, 7.559172e-01, 4.187294e-01, 2.097245e-01, 4.690000e-02, 1.190000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 2.680000e+00, 2.900000e+00, 3.100000e+00, 3.300000e+00, 3.600000e+00, 3.800000e+00, 4.020000e+00, 4.250000e+00 }, // SINR
          { 9.452555e-01, 6.746032e-01, 4.022082e-01, 1.738227e-01, 3.040000e-02, 5.600000e-03, 1.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 2.730000e+00, 3, 3.300000e+00, 3.500000e+00, 3.800000e+00, 4.100000e+00 }, // SINR
          { 9.258929e-01, 6.553030e-01, 2.448930e-01, 7.830000e-02, 6.400000e-03, 3.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 2.900000e+00, 3.300000e+00, 3.600000e+00, 4, 4.380000e+00 }, // SINR
          { 9.855769e-01, 8.179688e-01, 3.088235e-01, 6.250000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.100000e+00, 2.700000e+00, 3.300000e+00, 3.900000e+00, 4.500000e+00, 5.100000e+00, 5.700000e+00 }, // SINR
          { 9.761450e-01, 8.741611e-01, 6.035714e-01, 2.560729e-01, 4.680000e-02, 3.600000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 2.830000e+00, 3.100000e+00, 3.400000e+00, 3.600000e+00, 3.900000e+00, 4.200000e+00 }, // SINR
          { 9.583333e-01, 6.088517e-01, 2.043619e-01, 5.810000e-02, 5.200000e-03, 2.000000e-04 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.880000e+00, 3.100000e+00, 3.300000e+00, 3.500000e+00, 3.800000e+00, 4, 4.230000e+00 }, // SINR
          { 9.289568e-01, 8.217742e-01, 5.224490e-01, 2.432171e-01, 2.660000e-02, 3.500000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.800000e+00, 3.100000e+00, 3.400000e+00, 3.700000e+00, 4, 4.300000e+00 }, // SINR
          { 9.708647e-01, 7.233146e-01, 2.968023e-01, 3.880000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.800000e+00, 3, 3.200000e+00, 3.400000e+00, 3.600000e+00, 3.800000e+00, 4, 4.200000e+00 }, // SINR
          { 9.826923e-01, 8.319805e-01, 5.309917e-01, 2.024960e-01, 4.530000e-02, 6.100000e-03, 2.000000e-04, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 7
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 3.210000e+00, 3.710000e+00, 3.905000e+00, 4.100000e+00, 4.295000e+00, 4.490000e+00, 4.990000e+00, 5.490000e+00, 5.990000e+00 }, // SINR
          { 9.718045e-01, 8.085443e-01, 6.851852e-01, 5.395299e-01, 3.876923e-01, 2.555668e-01, 5.470000e-02, 5.300000e-03, 3.000000e-04 } // BLER
        }
      },
      { 4032U, // SINR and BLER for CBS 4032
        NrEesmErrorModel::DoubleTuple{
          { 3.505800e+00, 3.756400e+00, 4.007000e+00, 4.257700e+00, 4.508300e+00, 5.008300e+00, 5.508300e+00, 6.008300e+00 }, // SINR
          { 9.122340e-01, 7.932099e-01, 6.117788e-01, 3.974057e-01, 2.105042e-01, 2.970000e-02, 1.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 3, 3.530000e+00, 3.800000e+00, 4.070000e+00, 4.600000e+00 }, // SINR
          { 1, 9.178571e-01, 7.789634e-01, 5.105422e-01, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 3.350000e+00, 3.600000e+00, 3.900000e+00, 4.100000e+00, 4.400000e+00, 4.600000e+00, 4.850000e+00, 5.100000e+00, 5.350000e+00, 5.600000e+00 }, // SINR
          { 9.447464e-01, 8.835616e-01, 6.701571e-01, 4.829545e-01, 2.252232e-01, 1.106794e-01, 1.010000e-02, 1.700000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 3.560800e+00, 3.763000e+00, 3.965300e+00, 4.167500e+00, 4.369800e+00, 4.869800e+00, 5.369800e+00, 5.869800e+00 }, // SINR
          { 9.095745e-01, 7.820122e-01, 6.013033e-01, 4.062500e-01, 2.314560e-01, 2.550000e-02, 1.500000e-03, 4.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 3.200000e+00, 3.700000e+00, 4.300000e+00, 4.900000e+00, 5.500000e+00 }, // SINR
          { 9.942308e-01, 8.733108e-01, 3.209288e-01, 1.570000e-02, 1.000000e-04 } // BLER
        }
      },
      { 5632U, // SINR and BLER for CBS 5632
        NrEesmErrorModel::DoubleTuple{
          { 3.217600e+00, 3.717600e+00, 4.047200e+00, 4.376900e+00, 4.706500e+00, 5.036100e+00, 5.536100e+00 }, // SINR
          { 9.817308e-01, 8.591667e-01, 5.791855e-01, 2.283698e-01, 5.250000e-02, 6.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 3.208800e+00, 3.708800e+00, 4.208800e+00, 4.617200e+00, 5.025700e+00 }, // SINR
          { 9.472222e-01, 7.595588e-01, 1.250000e-01, 9.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 7296U, // SINR and BLER for CBS 7296
        NrEesmErrorModel::DoubleTuple{
          { 3.200000e+00, 3.700000e+00, 4.100000e+00, 4.600000e+00, 5, 5.450000e+00 }, // SINR
          { 9.826923e-01, 8.274194e-01, 4.800380e-01, 8.797417e-02, 8.500000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 3.304500e+00, 3.728600e+00, 4.152600e+00, 4.576600e+00, 5.000700e+00 }, // SINR
          { 9.875000e-01, 7.944785e-01, 2.613402e-01, 1.970000e-02, 3.000000e-04 } // BLER
        }
      }
  },
  { // MCS 8
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.460000e+00, 4.500000e+00, 4.600000e+00, 4.700000e+00, 4.720000e+00, 4.800000e+00, 4.900000e+00, 4.980000e+00, 5, 5.240000e+00, 5.500000e+00 }, // SINR
          { 9.546296e-01, 9.196429e-01, 8.665541e-01, 7.060811e-01, 4.852099e-01, 4.332192e-01, 2.706103e-01, 1.241362e-01, 5.300000e-02, 4.200000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 4.258500e+00, 4.509200e+00, 4.760000e+00, 5.010800e+00, 5.261500e+00 }, // SINR
          { 9.903846e-01, 8.172468e-01, 2.485149e-01, 1.330000e-02, 2.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 4.100000e+00, 4.700000e+00, 5.300000e+00, 5.900000e+00 }, // SINR
          { 9.178571e-01, 2.702991e-01, 4.700000e-03, 0 } // BLER
        }
      },
      { 4608U, // SINR and BLER for CBS 4608
        NrEesmErrorModel::DoubleTuple{
          { 3.800000e+00, 4.400000e+00, 4.700000e+00, 5, 5.600000e+00 }, // SINR
          { 1, 6.477273e-01, 3.983386e-01, 5.580000e-02, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 4.300000e+00, 4.600000e+00, 4.800000e+00, 5.100000e+00, 5.300000e+00 }, // SINR
          { 9.990385e-01, 7.965839e-01, 2.694149e-01, 4.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 3.850000e+00, 4.200000e+00, 4.550000e+00, 4.900000e+00, 5.200000e+00, 5.600000e+00 }, // SINR
          { 1, 8.809122e-01, 4.163934e-01, 1.534714e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 4, 4.500000e+00, 5, 5.500000e+00 }, // SINR
          { 9.884615e-01, 6.898396e-01, 2.110000e-02, 0 } // BLER
        }
      },
      { 6528U, // SINR and BLER for CBS 6528
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.680000e+00, 4.950000e+00, 5.230000e+00, 5.500000e+00 }, // SINR
          { 9.411232e-01, 6.417910e-01, 2.137712e-01, 2.810000e-02, 8.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 3.800000e+00, 4.200000e+00, 4.600000e+00, 5, 5.400000e+00 }, // SINR
          { 1, 9.865385e-01, 6.458333e-01, 7.860000e-02, 4.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 4.005500e+00, 4.540600e+00, 4.808200e+00, 5.610900e+00 }, // SINR
          { 9.734848e-01, 7.611607e-01, 3.080900e-01, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 9
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 4.950000e+00, 5.210000e+00, 5.470000e+00, 5.730000e+00, 5.990000e+00, 6.250000e+00 }, // SINR
          { 9.990385e-01, 9.334532e-01, 5.429025e-01, 8.951744e-02, 3.000000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 5.011200e+00, 5.262100e+00, 5.512900e+00, 5.763800e+00, 6.014700e+00 }, // SINR
          { 9.865385e-01, 7.544379e-01, 2.242451e-01, 1.320000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 5.054100e+00, 5.295900e+00, 5.537700e+00, 5.779500e+00, 6.021300e+00 }, // SINR
          { 9.807692e-01, 7.745536e-01, 2.652311e-01, 1.810000e-02, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 5.078800e+00, 5.311500e+00, 5.544200e+00, 5.776900e+00, 6.009500e+00 }, // SINR
          { 9.846154e-01, 7.612275e-01, 2.121849e-01, 2.460000e-02, 4.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 4.500000e+00, 5.030000e+00, 5.300000e+00, 5.570000e+00, 6.100000e+00 }, // SINR
          { 1, 9.668561e-01, 6.765873e-01, 1.439437e-01, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 4.800000e+00, 5.300000e+00, 5.900000e+00, 6.500000e+00 }, // SINR
          { 1, 6.660156e-01, 2.500000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 4.603200e+00, 5.286400e+00, 5.628000e+00, 5.969600e+00, 6.652800e+00 }, // SINR
          { 1, 9.370504e-01, 3.438347e-01, 1.900000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 4.800000e+00, 5.100000e+00, 5.400000e+00, 5.700000e+00, 6, 6.300000e+00, 6.600000e+00, 6.900000e+00, 7.200000e+00, 7.500000e+00, 7.800000e+00 }, // SINR
          { 9.534672e-01, 8.609272e-01, 7.120166e-01, 5.030000e-01, 2.825112e-01, 1.243824e-01, 4.160000e-02, 1.130000e-02, 2.200000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 7296U, // SINR and BLER for CBS 7296
        NrEesmErrorModel::DoubleTuple{
          { 4.953500e+00, 5.383900e+00, 5.599100e+00, 5.921900e+00, 6.244700e+00 }, // SINR
          { 1, 8.724832e-01, 2.850679e-01, 3.200000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 5.003900e+00, 5.607900e+00, 6.211900e+00, 6.815900e+00 }, // SINR
          { 9.461679e-01, 2.864465e-01, 1.900000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 10
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 5.700000e+00, 5.960000e+00, 6.220000e+00, 6.480000e+00, 6.740000e+00, 7 }, // SINR
          { 9.990385e-01, 9.280576e-01, 4.133987e-01, 3.080000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 5.763800e+00, 6.014900e+00, 6.265900e+00, 6.516900e+00, 6.767900e+00 }, // SINR
          { 9.846154e-01, 7.113260e-01, 1.300466e-01, 2.500000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 5.809700e+00, 6.051800e+00, 6.293800e+00, 6.535900e+00, 6.777900e+00 }, // SINR
          { 9.903846e-01, 7.140884e-01, 1.392423e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 5.837700e+00, 6.070800e+00, 6.303800e+00, 6.536900e+00, 6.769900e+00, 7.003000e+00 }, // SINR
          { 9.866412e-01, 6.690789e-01, 1.150321e-01, 6.000000e-02, 1.570000e-02, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 5.347700e+00, 5.847700e+00, 6.183800e+00, 6.520000e+00, 6.856200e+00, 7.192300e+00 }, // SINR
          { 9.751908e-01, 6.736842e-01, 2.515000e-01, 3.170000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 5.883800e+00, 5.983800e+00, 6.083800e+00, 6.183800e+00, 6.283800e+00, 6.383800e+00, 6.483800e+00, 6.583800e+00 }, // SINR
          { 1, 8.415033e-01, 6.634115e-01, 3.810423e-01, 2.409524e-01, 2.074424e-01, 1.474209e-01, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 5.300000e+00, 5.800000e+00, 6.300000e+00, 6.800000e+00, 7.300000e+00 }, // SINR
          { 9.923077e-01, 8.156646e-01, 3.491736e-01, 4.280000e-02, 1.000000e-03 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 4.911500e+00, 5.368650e+00, 5.825800e+00, 6.282900e+00, 6.740000e+00, 7.197200e+00, 7.654300e+00 }, // SINR
          { 1, 9.000000e-01, 7.099448e-01, 1.685074e-01, 3.240000e-02, 1.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 7296U, // SINR and BLER for CBS 7296
        NrEesmErrorModel::DoubleTuple{
          { 4.447400e+00, 5.754200e+00, 7.061000e+00, 8.367800e+00 }, // SINR
          { 1, 8.575000e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 5.500000e+00, 6.511100e+00, 7.016700e+00, 7.522300e+00 }, // SINR
          { 9.798077e-01, 5.300000e-02, 4.700000e-03, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 11
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 6.033800e+00, 6.332800e+00, 6.631900e+00, 6.930900e+00, 7.230000e+00, 7.730000e+00 }, // SINR
          { 9.932692e-01, 9.151786e-01, 6.105769e-01, 1.890106e-01, 2.160000e-02, 4.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 6.016500e+00, 6.516500e+00, 6.704900e+00, 6.893300e+00, 7.081600e+00, 7.270000e+00, 7.770000e+00 }, // SINR
          { 9.685115e-01, 6.123188e-01, 3.143657e-01, 1.031513e-01, 2.270000e-02, 3.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 5.823100e+00, 6.323100e+00, 6.565400e+00, 6.807700e+00, 7.050000e+00, 7.292300e+00, 7.792300e+00 }, // SINR
          { 9.980769e-01, 8.368506e-01, 5.064741e-01, 1.574248e-01, 2.500000e-02, 2.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 6.363100e+00, 6.655000e+00, 6.946800e+00, 7.238600e+00, 7.530500e+00 }, // SINR
          { 9.837786e-01, 7.942547e-01, 2.870475e-01, 2.550000e-02, 7.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 5.450000e+00, 6.100000e+00, 6.800000e+00, 7.400000e+00, 8.100000e+00, 8.700000e+00 }, // SINR
          { 9.894231e-01, 8.916084e-01, 5.235656e-01, 1.541054e-01, 1.080000e-02, 4.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 6.390000e+00, 6.605800e+00, 6.821600e+00, 7.037400e+00, 7.253200e+00, 7.753200e+00 }, // SINR
          { 9.208633e-01, 6.129227e-01, 2.087500e-01, 2.340000e-02, 1.100000e-03, 2.000000e-04 } // BLER
        }
      },
      { 5760U, // SINR and BLER for CBS 5760
        NrEesmErrorModel::DoubleTuple{
          { 6.300000e+00, 6.800000e+00, 7.300000e+00, 7.800000e+00 }, // SINR
          { 9.923077e-01, 4.146242e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 6.134900e+00, 6.634900e+00, 6.784900e+00, 6.934900e+00, 7.084900e+00, 7.234900e+00, 7.734900e+00 }, // SINR
          { 9.592593e-01, 6.854839e-01, 3.533520e-01, 1.082677e-01, 2.040000e-02, 1.900000e-03, 3.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 6.139500e+00, 6.593000e+00, 7.046400e+00, 7.499800e+00 }, // SINR
          { 9.990385e-01, 7.514620e-01, 2.190000e-02, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 6.163800e+00, 6.511000e+00, 6.858200e+00, 7.205300e+00, 7.552500e+00 }, // SINR
          { 9.990385e-01, 9.402174e-01, 3.798799e-01, 7.300000e-03, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 12
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 6.200000e+00, 7.200000e+00, 8.200000e+00, 9.200000e+00, 1.020000e+01 }, // SINR
          { 9.980769e-01, 8.750000e-01, 1.701624e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 4032U, // SINR and BLER for CBS 4032
        NrEesmErrorModel::DoubleTuple{
          { 6.769200e+00, 7.269200e+00, 7.520500e+00, 7.771800e+00, 8.023100e+00, 8.274400e+00 }, // SINR
          { 9.884615e-01, 6.449005e-01, 2.555894e-01, 3.990000e-02, 2.700000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 6.821000e+00, 7.321000e+00, 7.563500e+00, 7.806100e+00, 8.048700e+00, 8.291200e+00 }, // SINR
          { 9.420956e-01, 5.694444e-01, 1.807554e-01, 2.580000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 6.855400e+00, 7.355400e+00, 7.530800e+00, 7.706200e+00, 7.881600e+00, 8.057000e+00 }, // SINR
          { 9.531250e-01, 5.294421e-01, 2.390684e-01, 7.280000e-02, 1.110000e-02, 9.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 6.872300e+00, 7.372300e+00, 7.653700e+00, 7.935100e+00, 8.216400e+00, 8.497800e+00 }, // SINR
          { 9.476103e-01, 8.404605e-01, 4.063505e-01, 7.710000e-02, 4.700000e-03, 2.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 6.871800e+00, 7.371800e+00, 7.588200e+00, 7.804600e+00, 8.021000e+00, 8.237400e+00, 8.737400e+00 }, // SINR
          { 9.420290e-01, 8.801020e-01, 5.578603e-01, 2.018000e-01, 3.090000e-02, 2.500000e-03, 3.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 6.800000e+00, 7.300000e+00, 7.800000e+00, 8.300000e+00, 8.800000e+00 }, // SINR
          { 1, 9.425182e-01, 2.038835e-01, 1.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 6656U, // SINR and BLER for CBS 6656
        NrEesmErrorModel::DoubleTuple{
          { 7.004100e+00, 7.271600e+00, 7.539000e+00, 7.806400e+00, 8.073900e+00, 8.573900e+00 }, // SINR
          { 9.903846e-01, 8.758503e-01, 4.907946e-01, 8.670000e-02, 3.900000e-03, 8.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 7.036100e+00, 7.472400e+00, 7.908800e+00, 8.345100e+00 }, // SINR
          { 9.807692e-01, 4.678309e-01, 1.050000e-02, 1.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 7.156200e+00, 7.509400e+00, 7.686100e+00, 7.862700e+00, 8.215900e+00 }, // SINR
          { 9.542910e-01, 3.047445e-01, 1.328125e-01, 4.780000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 13
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 7.702700e+00, 8.202700e+00, 8.450300e+00, 8.697800e+00, 8.945400e+00 }, // SINR
          { 1, 8.583333e-01, 2.439202e-01, 8.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 7.725800e+00, 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 1, 9.049296e-01, 2.573980e-01, 4.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 7.725800e+00, 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 1, 9.330357e-01, 2.788462e-01, 4.400000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.601852e-01, 3.724112e-01, 9.500000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.884615e-01, 5.009766e-01, 1.830000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.675573e-01, 4.080645e-01, 6.800000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.855769e-01, 4.761236e-01, 1.010000e-02, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.932692e-01, 4.932692e-01, 8.000000e-03, 0 } // BLER
        }
      },
      { 7552U, // SINR and BLER for CBS 7552
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.299867e+00, 8.363700e+00, 8.576300e+00, 8.788900e+00 }, // SINR
          { 9.951923e-01, 5.369198e-01, 1.240109e-01, 8.300000e-03, 9.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.314176e+00, 8.363700e+00, 8.576300e+00, 8.788900e+00 }, // SINR
          { 9.932692e-01, 4.129902e-01, 1.234006e-01, 5.000000e-03, 7.000000e-04 } // BLER
        }
      }
  },
  { // MCS 14
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.649621e-01, 5.136089e-01, 3.570000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.611742e-01, 4.551971e-01, 2.220000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.740385e-01, 4.454225e-01, 1.860000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.790076e-01, 4.283898e-01, 1.420000e-02, 1.000000e-04 } // BLER
        }
      },
      { 5120U, // SINR and BLER for CBS 5120
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.894231e-01, 6.838235e-01, 4.660000e-02, 2.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.855769e-01, 5.720721e-01, 3.410000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.923077e-01, 5.814220e-01, 2.400000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.923077e-01, 5.432692e-01, 1.610000e-02, 1.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.971154e-01, 7.067039e-01, 3.580000e-02, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.932692e-01, 5.726351e-01, 1.340000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 15
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.951923e-01, 8.792808e-01, 3.351064e-01, 2.140000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.923077e-01, 7.150838e-01, 1.250000e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.951923e-01, 7.412281e-01, 1.118772e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.971154e-01, 7.333815e-01, 1.081878e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.942308e-01, 7.641369e-01, 9.903133e-02, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.980769e-01, 7.097222e-01, 7.160000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 5632U, // SINR and BLER for CBS 5632
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01 }, // SINR
          { 9.980769e-01, 6.536990e-01, 4.360000e-02, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 1, 9.357143e-01, 3.420516e-01, 7.800000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 1, 9.397482e-01, 2.602459e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01 }, // SINR
          { 1, 8.325321e-01, 9.162572e-02, 0 } // BLER
        }
      }
  },
  { // MCS 16
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.509259e-01, 5.225410e-01, 5.610000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01 }, // SINR
          { 9.148936e-01, 3.670520e-01, 2.060000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.555556e-01, 4.508929e-01, 3.290000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.028680e+01, 1.052850e+01, 1.077030e+01, 1.101200e+01, 1.151200e+01 }, // SINR
          { 9.809160e-01, 8.344156e-01, 3.935759e-01, 5.610000e-02, 8.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.638889e-01, 4.513393e-01, 2.200000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01 }, // SINR
          { 9.780534e-01, 4.980315e-01, 2.850000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6144U, // SINR and BLER for CBS 6144
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.990385e-01, 7.993827e-01, 1.207892e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.913462e-01, 6.949728e-01, 7.500000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.961538e-01, 7.271429e-01, 6.450000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.033890e+01, 1.056330e+01, 1.078760e+01, 1.101200e+01, 1.151200e+01 }, // SINR
          { 9.608209e-01, 6.614583e-01, 1.791311e-01, 1.450000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 17
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.099440e+01, 1.130050e+01, 1.160660e+01, 1.191270e+01, 1.221880e+01 }, // SINR
          { 1, 8.818493e-01, 1.824422e-01, 2.600000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.111850e+01, 1.143430e+01, 1.175000e+01, 1.206580e+01 }, // SINR
          { 9.661654e-01, 3.908669e-01, 8.500000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.116970e+01, 1.146840e+01, 1.176710e+01, 1.206580e+01 }, // SINR
          { 9.961538e-01, 7.217514e-01, 7.030000e-02, 4.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.088000e+01, 1.120000e+01, 1.150000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.283000e+01 }, // SINR
          { 1, 7.514620e-01, 6.009390e-01, 3.656069e-01, 1.948758e-01, 8.411120e-02, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.099440e+01, 1.137700e+01, 1.175970e+01, 1.214230e+01 }, // SINR
          { 1, 8.333333e-01, 3.770000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.095000e+01, 1.130000e+01, 1.170000e+01, 12, 1.240000e+01, 1.270000e+01, 1.305000e+01 }, // SINR
          { 9.961538e-01, 7.398256e-01, 4.801136e-01, 2.611515e-01, 8.490000e-02, 2.780000e-02, 0 } // BLER
        }
      },
      { 6016U, // SINR and BLER for CBS 6016
        NrEesmErrorModel::DoubleTuple{
          { 1.090000e+01, 1.140000e+01, 1.162500e+01, 1.185000e+01, 1.207500e+01, 1.230000e+01 }, // SINR
          { 1, 6.585052e-01, 1.927803e-01, 1.940000e-02, 8.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.120000e+01, 1.150000e+01, 1.180000e+01, 1.210000e+01, 1.240000e+01, 1.270000e+01, 13 }, // SINR
          { 9.866412e-01, 7.307143e-01, 5.029880e-01, 2.775938e-01, 1.145546e-01, 3.390000e-02, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.130050e+01, 1.153010e+01, 1.175970e+01, 1.198920e+01, 1.221880e+01 }, // SINR
          { 9.818702e-01, 7.041667e-01, 1.377747e-01, 4.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.130050e+01, 1.160660e+01, 1.191270e+01, 1.221880e+01 }, // SINR
          { 9.753788e-01, 3.709677e-01, 6.900000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 18
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.196360e+01, 1.219070e+01, 1.241790e+01, 1.264510e+01 }, // SINR
          { 9.828244e-01, 5.774887e-01, 6.630000e-02, 7.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.155000e+01, 1.180000e+01, 1.210000e+01, 1.230000e+01, 1.260000e+01, 1.280000e+01, 1.305000e+01 }, // SINR
          { 1, 6.401515e-01, 2.695815e-01, 1.036765e-01, 1.240000e-02, 2.100000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.196360e+01, 1.219070e+01, 1.241790e+01, 1.264510e+01, 1.314510e+01 }, // SINR
          { 9.865385e-01, 6.708115e-01, 9.662327e-02, 2.400000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.196360e+01, 1.219070e+01, 1.241790e+01, 1.264510e+01, 1.314510e+01 }, // SINR
          { 9.932692e-01, 6.606218e-01, 9.226430e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.173000e+01, 12, 1.230000e+01, 1.260000e+01, 1.280000e+01, 1.310000e+01 }, // SINR
          { 1, 3.327836e-01, 6.850000e-02, 7.800000e-03, 9.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.196360e+01, 1.219070e+01, 1.241790e+01, 1.264510e+01 }, // SINR
          { 9.763258e-01, 5.786199e-01, 4.330000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.180220e+01, 1.215040e+01, 1.249860e+01, 1.284690e+01 }, // SINR
          { 1, 5.193878e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 6400U, // SINR and BLER for CBS 6400
        NrEesmErrorModel::DoubleTuple{
          { 1.181210e+01, 1.219080e+01, 1.256940e+01, 1.294800e+01 }, // SINR
          { 1, 6.562500e-01, 4.900000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.163000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.280000e+01, 13 }, // SINR
          { 1, 6.497462e-01, 1.832117e-01, 1.580000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.188790e+01, 1.211510e+01, 1.234220e+01, 1.256940e+01, 1.279660e+01 }, // SINR
          { 9.990385e-01, 8.550000e-01, 1.733815e-01, 2.300000e-03, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 19
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.923077e-01, 8.320064e-01, 3.105037e-01, 2.410000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.865385e-01, 7.962963e-01, 2.366822e-01, 1.170000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.951923e-01, 8.261218e-01, 2.475394e-01, 1.000000e-02, 3.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.942308e-01, 8.409091e-01, 2.485207e-01, 9.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.990385e-01, 8.649329e-01, 2.715517e-01, 1.000000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.990385e-01, 8.128981e-01, 1.842009e-01, 3.800000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 1, 8.707770e-01, 1.973787e-01, 3.600000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 1, 8.279221e-01, 1.474736e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 7168U, // SINR and BLER for CBS 7168
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.990385e-01, 8.750000e-01, 2.142249e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.990385e-01, 8.977273e-01, 2.119932e-01, 2.500000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 20
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.340000e+01, 1.350000e+01, 1.360000e+01, 1.370000e+01, 1.380000e+01, 1.383010e+01, 1.390000e+01, 14, 1.410000e+01, 1.412130e+01, 1.420000e+01, 1.430000e+01, 1.440000e+01, 1.441250e+01, 1.450000e+01, 1.470370e+01 }, // SINR
          { 9.809160e-01, 9.479927e-01, 8.783784e-01, 7.603550e-01, 5.992991e-01, 5.452128e-01, 4.172131e-01, 2.573980e-01, 1.256269e-01, 1.055867e-01, 5.050000e-02, 1.780000e-02, 4.400000e-03, 4.400000e-03, 1.200000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 9.884615e-01, 6.268473e-01, 1.351351e-01, 3.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01, 1.499490e+01 }, // SINR
          { 9.198944e-01, 4.584838e-01, 5.700000e-02, 1.200000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.278649e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 9.000000e-01, 6.213415e-01, 9.919488e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 9.598881e-01, 5.080645e-01, 4.590000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.324770e+01, 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 1, 8.870690e-01, 5.372340e-01, 5.750000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 9.715909e-01, 5.280083e-01, 4.100000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.270000e+01, 1.310000e+01, 1.350000e+01, 14, 1.440000e+01, 1.483000e+01 }, // SINR
          { 1, 9.971154e-01, 9.361314e-01, 6.772487e-01, 3.478709e-01, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.261860e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01 }, // SINR
          { 9.000000e-01, 6.795213e-01, 5.610000e-02, 0 } // BLER
        }
      },
      { 7808U, // SINR and BLER for CBS 7808
        NrEesmErrorModel::DoubleTuple{
          { 1.270000e+01, 1.310000e+01, 1.360000e+01, 14, 1.440000e+01, 1.483000e+01 }, // SINR
          { 9.942308e-01, 9.642857e-01, 7.529240e-01, 3.960938e-01, 1.133590e-01, 0 } // BLER
        }
      }
  },
  { // MCS 21
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.480000e+01, 1.530000e+01, 1.570000e+01, 1.620000e+01 }, // SINR
          { 9.447464e-01, 5.788288e-01, 8.994762e-02, 3.400000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.450000e+01, 1.490000e+01, 1.520000e+01, 1.560000e+01, 1.590000e+01 }, // SINR
          { 9.343525e-01, 5.669643e-01, 2.064860e-01, 1.400000e-02, 8.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.415000e+01, 1.450000e+01, 1.490000e+01, 1.520000e+01, 1.560000e+01, 1.590000e+01 }, // SINR
          { 1, 8.775510e-01, 4.340986e-01, 1.132777e-01, 3.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.410000e+01, 1.460000e+01, 15, 1.550000e+01, 1.590000e+01 }, // SINR
          { 1, 9.038462e-01, 4.228188e-01, 1.840000e-02, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.480000e+01, 1.530000e+01, 1.590000e+01, 1.640000e+01 }, // SINR
          { 1, 8.567881e-01, 1.193994e-01, 1.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.460000e+01, 1.490000e+01, 1.520000e+01, 1.550000e+01, 1.580000e+01, 1.610000e+01, 1.640000e+01, 1.670000e+01, 17, 1.730000e+01 }, // SINR
          { 9.262590e-01, 7.960123e-01, 5.221193e-01, 2.329020e-01, 7.060000e-02, 2.250000e-02, 4.700000e-03, 8.000000e-04, 4.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 14, 1.450000e+01, 15, 1.540000e+01, 1.590000e+01 }, // SINR
          { 9.980769e-01, 8.121069e-01, 8.309944e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.470000e+01, 1.510000e+01, 1.550000e+01, 1.590000e+01 }, // SINR
          { 9.730769e-01, 4.922481e-01, 2.390000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 14, 1.460000e+01, 1.520000e+01, 1.580000e+01 }, // SINR
          { 1, 8.948276e-01, 6.370000e-02, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.420000e+01, 1.460000e+01, 1.510000e+01, 1.550000e+01, 1.590000e+01 }, // SINR
          { 9.980769e-01, 8.836207e-01, 9.797297e-02, 6.000000e-04, 2.000000e-04 } // BLER
        }
      }
  },
  { // MCS 22
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.560000e+01, 1.583000e+01, 1.590000e+01, 1.607000e+01, 1.630000e+01, 1.660000e+01 }, // SINR
          { 9.531250e-01, 7.747006e-01, 2.305759e-01, 5.920000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.460000e+01, 1.527000e+01, 1.560000e+01, 1.593000e+01, 1.660000e+01 }, // SINR
          { 1, 9.836538e-01, 8.819444e-01, 1.929724e-01, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.510000e+01, 1.545000e+01, 1.580000e+01, 1.615000e+01, 1.650000e+01, 1.685000e+01, 1.720000e+01, 1.755000e+01, 1.790000e+01, 1.825000e+01 }, // SINR
          { 9.894231e-01, 9.415468e-01, 8.144904e-01, 5.516304e-01, 2.818792e-01, 1.168785e-01, 3.300000e-02, 5.600000e-03, 9.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.563000e+01, 1.580000e+01, 1.597000e+01, 1.630000e+01, 1.680000e+01 }, // SINR
          { 1, 6.557692e-01, 5.404412e-01, 3.468407e-01, 1.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.530000e+01, 1.563000e+01, 1.597000e+01, 1.630000e+01 }, // SINR
          { 1, 9.961538e-01, 5.145582e-01, 5.850000e-02, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.563000e+01, 1.580000e+01, 1.597000e+01, 1.630000e+01, 1.680000e+01 }, // SINR
          { 1, 8.200637e-01, 7.406069e-01, 4.917954e-01, 8.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.563000e+01, 1.597000e+01, 1.630000e+01 }, // SINR
          { 1, 3.974763e-01, 2.870000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.540000e+01, 1.573000e+01, 1.590000e+01, 1.607000e+01, 1.640000e+01, 17 }, // SINR
          { 1, 7.973602e-01, 6.838235e-01, 4.563849e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.540000e+01, 1.570000e+01, 1.580000e+01, 16, 1.630000e+01, 1.670000e+01 }, // SINR
          { 1, 8.558333e-01, 6.668814e-01, 5.600437e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 1.540000e+01, 1.573000e+01, 1.607000e+01, 1.640000e+01 }, // SINR
          { 1, 9.990385e-01, 5.910138e-01, 6.320000e-02, 5.000000e-04 } // BLER
        }
      }
  },
  { // MCS 23
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.580000e+01, 1.710000e+01, 1.840000e+01, 1.970000e+01 }, // SINR
          { 9.687500e-01, 5.009843e-01, 2.280000e-02, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.590000e+01, 1.690000e+01, 1.790000e+01, 1.890000e+01, 1.990000e+01 }, // SINR
          { 9.780534e-01, 6.064593e-01, 9.153226e-02, 2.000000e-03, 0 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 1.560000e+01, 1.690000e+01, 1.820000e+01, 1.950000e+01 }, // SINR
          { 9.961538e-01, 4.575812e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 1.520000e+01, 1.630000e+01, 1.740000e+01, 1.860000e+01, 1.970000e+01 }, // SINR
          { 9.980769e-01, 8.750000e-01, 1.662269e-01, 8.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.540000e+01, 1.650000e+01, 1.750000e+01, 1.850000e+01, 1.960000e+01 }, // SINR
          { 9.980769e-01, 6.801862e-01, 5.830000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 1.640000e+01, 1.740000e+01, 1.840000e+01, 1.940000e+01 }, // SINR
          { 9.614662e-01, 3.350000e-01, 4.000000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.620000e+01, 17, 1.780000e+01, 1.860000e+01, 1.940000e+01 }, // SINR
          { 9.303571e-01, 3.057927e-01, 9.900000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.620000e+01, 17, 1.780000e+01, 1.850000e+01, 1.930000e+01 }, // SINR
          { 9.799618e-01, 5.362395e-01, 3.650000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.545590e+01, 1.635660e+01, 1.725730e+01, 1.815800e+01, 1.905870e+01 }, // SINR
          { 9.961538e-01, 8.843537e-01, 3.052536e-01, 1.920000e-02, 1.000000e-04 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.567690e+01, 1.632850e+01, 1.698010e+01, 1.763170e+01, 1.828330e+01, 1.893490e+01 }, // SINR
          { 1, 7.962963e-01, 3.098894e-01, 3.380000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.570000e+01, 1.650000e+01, 1.720000e+01, 1.790000e+01, 1.870000e+01 }, // SINR
          { 9.932692e-01, 6.978022e-01, 1.054721e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.602390e+01, 1.652390e+01, 1.700290e+01, 1.748180e+01, 1.796070e+01, 1.843970e+01 }, // SINR
          { 9.895038e-01, 6.779101e-01, 2.800773e-01, 5.080000e-02, 3.900000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.575000e+01, 1.635400e+01, 1.695800e+01, 1.756200e+01, 1.816600e+01 }, // SINR
          { 9.942308e-01, 8.566667e-01, 3.352273e-01, 3.810000e-02, 7.000000e-04 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 16, 1.650000e+01, 1.691650e+01, 1.733300e+01, 1.774950e+01, 1.816600e+01 }, // SINR
          { 9.798077e-01, 6.634115e-01, 2.692308e-01, 5.640000e-02, 5.500000e-03, 2.000000e-04 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.606700e+01, 1.656700e+01, 1.696680e+01, 1.736650e+01, 1.776630e+01, 1.816600e+01 }, // SINR
          { 9.846154e-01, 8.066038e-01, 4.368557e-01, 1.158649e-01, 1.390000e-02, 9.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 1.609220e+01, 1.659220e+01, 1.698570e+01, 1.737910e+01, 1.777260e+01, 1.816600e+01 }, // SINR
          { 9.990385e-01, 6.864973e-01, 2.663848e-01, 4.940000e-02, 2.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 1.544400e+01, 1.641220e+01, 1.738040e+01, 1.834850e+01 }, // SINR
          { 1, 8.518212e-01, 3.810000e-02, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.619300e+01, 1.685070e+01, 1.750830e+01, 1.816600e+01 }, // SINR
          { 9.817308e-01, 5.306017e-01, 3.050000e-02, 2.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.604200e+01, 1.657300e+01, 1.710400e+01, 1.763500e+01, 1.816600e+01 }, // SINR
          { 9.971154e-01, 8.750000e-01, 3.176952e-01, 1.930000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.604200e+01, 1.657300e+01, 1.710400e+01, 1.763500e+01, 1.816600e+01 }, // SINR
          { 9.961538e-01, 7.865854e-01, 1.971831e-01, 6.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.598620e+01, 1.648620e+01, 1.690610e+01, 1.732610e+01, 1.774600e+01 }, // SINR
          { 9.826923e-01, 8.221154e-01, 2.907044e-01, 2.120000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.589240e+01, 1.639240e+01, 1.668800e+01, 1.698360e+01, 1.727920e+01, 1.757480e+01 }, // SINR
          { 9.913462e-01, 7.937117e-01, 3.416442e-01, 5.610000e-02, 3.500000e-03, 3.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.605760e+01, 1.655760e+01, 1.695970e+01, 1.736180e+01, 1.776390e+01 }, // SINR
          { 9.503676e-01, 7.847222e-01, 1.744467e-01, 5.500000e-03, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.633920e+01, 1.685690e+01, 1.737460e+01, 1.789240e+01 }, // SINR
          { 9.617537e-01, 3.051205e-01, 3.400000e-03, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.589240e+01, 1.639240e+01, 1.654020e+01, 1.668800e+01, 1.683580e+01, 1.698360e+01, 1.748360e+01 }, // SINR
          { 9.932692e-01, 8.640940e-01, 6.599741e-01, 4.076613e-01, 1.746528e-01, 5.800000e-02, 6.000000e-04 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 1.635500e+01, 1.641280e+01, 1.647060e+01, 1.652840e+01, 1.658610e+01, 1.708610e+01, 1.758610e+01 }, // SINR
          { 9.583333e-01, 9.303571e-01, 8.932292e-01, 8.406863e-01, 7.700893e-01, 5.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.626710e+01, 1.676710e+01, 1.726710e+01, 1.776710e+01 }, // SINR
          { 9.971154e-01, 6.336634e-01, 3.680000e-02, 3.000000e-04 } // BLER
        }
      },
      { 3368U, // SINR and BLER for CBS 3368
        NrEesmErrorModel::DoubleTuple{
          { 1.580000e+01, 1.630000e+01, 1.680000e+01, 1.730000e+01 }, // SINR
          { 9.990385e-01, 8.292484e-01, 8.062901e-02, 1.000000e-04 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.580000e+01, 1.655000e+01, 1.730000e+01, 1.805000e+01 }, // SINR
          { 1, 8.241935e-01, 7.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.610000e+01, 1.660000e+01, 1.680000e+01, 17, 1.720000e+01 }, // SINR
          { 9.951923e-01, 3.374335e-01, 4.700000e-02, 3.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.631920e+01, 1.637660e+01, 1.643390e+01, 1.649130e+01, 1.654860e+01, 1.704860e+01, 1.754860e+01, 1.804860e+01 }, // SINR
          { 9.420290e-01, 9.107143e-01, 8.558333e-01, 7.797256e-01, 6.871622e-01, 1.893769e-01, 2.100000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.581200e+01, 1.631200e+01, 1.633990e+01, 1.636780e+01, 1.639570e+01, 1.642360e+01, 1.692360e+01, 1.742360e+01, 1.792360e+01 }, // SINR
          { 1, 8.975694e-01, 8.733108e-01, 8.269231e-01, 7.854938e-01, 7.341954e-01, 2.042683e-01, 1.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.598620e+01, 1.648620e+01, 1.683770e+01, 1.718930e+01, 1.754080e+01 }, // SINR
          { 1, 8.200637e-01, 1.357759e-01, 1.800000e-03, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.643010e+01, 1.648610e+01, 1.698610e+01, 1.748610e+01, 1.798610e+01 }, // SINR
          { 9.574074e-01, 9.119718e-01, 2.058824e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.660000e+01, 1.680000e+01, 17, 1.750000e+01 }, // SINR
          { 9.798077e-01, 7.699704e-01, 2.934149e-01, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.615110e+01, 1.653810e+01, 1.673150e+01, 1.692500e+01, 1.731200e+01 }, // SINR
          { 1, 7.078729e-01, 4.035714e-01, 6.860000e-02, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.627270e+01, 1.677270e+01, 1.727270e+01, 1.777270e+01 }, // SINR
          { 9.923077e-01, 3.414634e-01, 2.700000e-03, 2.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.654860e+01, 1.688190e+01, 1.704860e+01, 1.721530e+01, 1.754860e+01 }, // SINR
          { 9.626866e-01, 2.439555e-01, 7.100000e-03, 3.100000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 24
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.670000e+01, 1.760000e+01, 1.840000e+01, 1.930000e+01 }, // SINR
          { 1, 8.913793e-01, 9.958678e-02, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.673620e+01, 1.763190e+01, 1.852760e+01, 1.942330e+01, 2.031900e+01 }, // SINR
          { 9.990385e-01, 9.280576e-01, 3.384718e-01, 1.230000e-02, 1.000000e-04 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 1.670000e+01, 1.760000e+01, 1.850000e+01, 1.940000e+01 }, // SINR
          { 1, 6.726190e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 1.670000e+01, 1.740000e+01, 18, 1.870000e+01 }, // SINR
          { 1, 8.716216e-01, 1.100220e-01, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.685000e+01, 1.750000e+01, 1.810000e+01, 1.880000e+01 }, // SINR
          { 1, 7.822086e-01, 5.110000e-02, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 1.690000e+01, 1.750000e+01, 18, 1.860000e+01 }, // SINR
          { 9.980769e-01, 7.076389e-01, 5.560000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.660000e+01, 1.750000e+01, 1.830000e+01, 1.920000e+01 }, // SINR
          { 1, 7.406609e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.690000e+01, 1.750000e+01, 1.810000e+01, 1.860000e+01, 1.920000e+01 }, // SINR
          { 1, 7.955247e-01, 2.810000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.710000e+01, 1.757000e+01, 1.780000e+01, 1.803000e+01, 1.850000e+01 }, // SINR
          { 9.826923e-01, 6.933060e-01, 2.182642e-01, 5.450000e-02, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.750550e+01, 1.773090e+01, 1.795630e+01, 1.818170e+01, 1.840710e+01, 1.890710e+01 }, // SINR
          { 9.352518e-01, 7.947531e-01, 5.508658e-01, 2.831461e-01, 9.708437e-02, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.648610e+01, 1.728960e+01, 1.782520e+01, 1.809300e+01, 1.836090e+01, 1.889650e+01 }, // SINR
          { 1, 9.388489e-01, 8.423203e-01, 6.330000e-02, 1.600000e-02, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.743360e+01, 1.793360e+01, 1.834420e+01, 1.875490e+01 }, // SINR
          { 9.990385e-01, 1.744105e-01, 4.300000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.689060e+01, 1.739060e+01, 1.787010e+01, 1.834960e+01, 1.882910e+01 }, // SINR
          { 1, 8.932292e-01, 2.425193e-01, 3.500000e-03, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 1.700860e+01, 1.750860e+01, 1.780860e+01, 1.810860e+01, 1.840860e+01, 1.870860e+01 }, // SINR
          { 1, 6.945946e-01, 2.255386e-01, 2.000000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.730860e+01, 1.764190e+01, 1.797520e+01, 1.830860e+01, 1.864190e+01 }, // SINR
          { 9.485294e-01, 5.120482e-01, 7.230000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 1.726950e+01, 1.776950e+01, 1.796190e+01, 1.815430e+01, 1.834670e+01, 1.853910e+01 }, // SINR
          { 1, 3.458904e-01, 1.003772e-01, 1.600000e-02, 1.400000e-03, 4.000000e-04 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 1.705240e+01, 1.755240e+01, 1.793800e+01, 1.832350e+01, 1.870910e+01 }, // SINR
          { 1, 6.274272e-01, 8.910000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.705860e+01, 1.755860e+01, 1.785030e+01, 1.814200e+01, 1.843360e+01, 1.872530e+01 }, // SINR
          { 1, 8.181090e-01, 3.716814e-01, 4.730000e-02, 1.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.683230e+01, 1.745140e+01, 1.807050e+01, 1.868950e+01 }, // SINR
          { 1, 7.950311e-01, 1.770000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.738740e+01, 1.770760e+01, 1.802780e+01, 1.834800e+01, 1.866820e+01 }, // SINR
          { 9.951923e-01, 7.772727e-01, 2.142857e-01, 7.500000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.708860e+01, 1.758860e+01, 1.787530e+01, 1.816200e+01, 1.844860e+01 }, // SINR
          { 1, 6.686198e-01, 1.526428e-01, 5.400000e-03, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.701850e+01, 1.751850e+01, 1.781690e+01, 1.811520e+01, 1.841360e+01 }, // SINR
          { 1, 7.827744e-01, 2.028491e-01, 5.800000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.742940e+01, 1.774260e+01, 1.805580e+01, 1.836900e+01, 1.868220e+01 }, // SINR
          { 9.942308e-01, 8.128931e-01, 2.034790e-01, 6.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.743360e+01, 1.774610e+01, 1.805860e+01, 1.837110e+01, 1.868360e+01 }, // SINR
          { 9.884615e-01, 6.604381e-01, 9.706478e-02, 2.600000e-03, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.750390e+01, 1.780470e+01, 1.810550e+01, 1.840620e+01, 1.870700e+01 }, // SINR
          { 9.942308e-01, 7.573529e-01, 1.203394e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 1.755860e+01, 1.785030e+01, 1.814200e+01, 1.843360e+01, 1.872530e+01 }, // SINR
          { 9.846154e-01, 6.207729e-01, 8.050000e-02, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.755860e+01, 1.785030e+01, 1.799610e+01, 1.814190e+01, 1.843360e+01, 1.887110e+01 }, // SINR
          { 9.732824e-01, 2.976471e-01, 2.061582e-01, 4.400000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.740490e+01, 1.776680e+01, 1.794780e+01, 1.812870e+01, 1.849060e+01, 1.903340e+01 }, // SINR
          { 9.990385e-01, 9.443431e-01, 4.637681e-01, 1.083479e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 3624U, // SINR and BLER for CBS 3624
        NrEesmErrorModel::DoubleTuple{
          { 1.773890e+01, 1.805280e+01, 1.836680e+01, 1.868070e+01 }, // SINR
          { 9.540441e-01, 3.118812e-01, 7.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.770250e+01, 1.801480e+01, 1.832710e+01, 1.863940e+01 }, // SINR
          { 9.942308e-01, 2.750545e-01, 1.200000e-02, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.755860e+01, 1.789490e+01, 1.806300e+01, 1.823110e+01, 1.856740e+01, 1.907180e+01 }, // SINR
          { 9.788462e-01, 5.459402e-01, 1.202107e-01, 8.200000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.730000e+01, 1.765000e+01, 1.782500e+01, 18, 1.835000e+01, 1.887500e+01 }, // SINR
          { 1, 8.716216e-01, 8.400974e-01, 1.070043e-01, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.772340e+01, 1.799990e+01, 1.813810e+01, 1.827640e+01, 1.855290e+01, 1.979720e+01 }, // SINR
          { 9.836538e-01, 2.823661e-01, 1.745480e-01, 1.520000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.754000e+01, 1.772000e+01, 1.790000e+01, 1.810000e+01, 1.830000e+01, 1.840000e+01, 1.860000e+01 }, // SINR
          { 9.903846e-01, 3.322454e-01, 2.868151e-01, 6.980000e-02, 7.200000e-03, 1.900000e-03, 2.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.711330e+01, 1.761330e+01, 1.810400e+01, 1.859470e+01 }, // SINR
          { 1, 8.897569e-01, 6.900000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.772790e+01, 1.803590e+01, 1.834400e+01, 1.865200e+01 }, // SINR
          { 9.913462e-01, 8.870000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.750490e+01, 1.780550e+01, 1.810610e+01, 1.840670e+01 }, // SINR
          { 1, 5.268595e-01, 2.380000e-02, 1.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.747950e+01, 1.778430e+01, 1.808920e+01, 1.839400e+01 }, // SINR
          { 1, 6.868351e-01, 5.590000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 25
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.854550e+01, 1.895640e+01, 1.936730e+01, 2.018900e+01 }, // SINR
          { 9.855769e-01, 5.770000e-02, 5.400000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.844550e+01, 1.875970e+01, 1.907390e+01, 1.970240e+01 }, // SINR
          { 9.687500e-01, 8.253205e-01, 2.190000e-02, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 1.858990e+01, 1.888350e+01, 1.917710e+01, 1.976430e+01 }, // SINR
          { 9.847328e-01, 2.036062e-01, 7.450000e-02, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.842400e+01, 1.891760e+01, 1.916430e+01, 1.941110e+01, 1.990470e+01 }, // SINR
          { 9.503676e-01, 2.896689e-01, 1.100000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 1.858980e+01, 1.890230e+01, 1.921470e+01, 1.952720e+01 }, // SINR
          { 9.837786e-01, 4.702602e-01, 2.040000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.829110e+01, 1.859600e+01, 1.890100e+01, 1.951080e+01 }, // SINR
          { 9.951923e-01, 8.565436e-01, 6.050000e-02, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.862910e+01, 1.892350e+01, 1.921780e+01, 1.951220e+01 }, // SINR
          { 9.570896e-01, 8.766779e-01, 2.024478e-01, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.847080e+01, 1.895400e+01, 1.919560e+01, 1.943710e+01, 1.992030e+01 }, // SINR
          { 1, 9.456699e-02, 2.700000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.798000e+01, 1.855000e+01, 1.911950e+01, 1.969160e+01 }, // SINR
          { 1, 7.993827e-01, 1.478873e-01, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.795000e+01, 1.861000e+01, 1.927040e+01, 1.993050e+01, 2.059060e+01 }, // SINR
          { 9.903846e-01, 4.302721e-01, 5.030000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+01, 1.870000e+01, 1.890000e+01, 1.910000e+01, 1.930000e+01, 1.950000e+01, 1.970000e+01, 1.990000e+01, 2.010000e+01, 2.030000e+01, 2.050000e+01 }, // SINR
          { 9.626866e-01, 8.673986e-01, 6.690415e-01, 4.030854e-01, 1.842972e-01, 4.290000e-02, 1.120000e-02, 2.100000e-03, 2.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+01, 19, 1.925000e+01, 1.950000e+01, 1.975000e+01 }, // SINR
          { 9.837786e-01, 2.121212e-01, 1.550000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 1.770000e+01, 1.830000e+01, 1.860000e+01, 1.890000e+01, 1.950000e+01 }, // SINR
          { 1, 9.761450e-01, 3.676901e-01, 1.245040e-01, 0 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.867000e+01, 19, 1.923000e+01, 1.980000e+01 }, // SINR
          { 9.980769e-01, 4.555160e-01, 2.640000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 18, 1.847000e+01, 1.870000e+01, 1.893000e+01, 1.940000e+01, 2.010000e+01 }, // SINR
          { 1, 8.402318e-01, 1.138332e-01, 6.220000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.860000e+01, 19, 1.950000e+01, 20 }, // SINR
          { 1, 8.566667e-01, 1.442750e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.830000e+01, 1.870000e+01, 1.910000e+01, 1.950000e+01, 20 }, // SINR
          { 1, 9.734848e-01, 3.106527e-01, 6.400000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.860000e+01, 19, 1.940000e+01, 1.990000e+01 }, // SINR
          { 1, 9.951923e-01, 4.879808e-01, 1.350000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.848000e+01, 1.885000e+01, 1.923000e+01, 1.960000e+01, 1.998000e+01, 2.035000e+01, 2.073000e+01 }, // SINR
          { 1, 9.744318e-01, 7.958075e-01, 3.614672e-01, 7.280000e-02, 8.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.853330e+01, 1.875000e+01, 1.896670e+01, 1.940000e+01 }, // SINR
          { 9.516423e-01, 3.731563e-01, 1.089939e-01, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.820000e+01, 1.861670e+01, 1.882500e+01, 1.903330e+01, 1.945000e+01 }, // SINR
          { 1, 9.375000e-01, 5.879630e-01, 8.497191e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+01, 1.891670e+01, 1.912500e+01, 1.933330e+01, 1.975000e+01 }, // SINR
          { 9.865385e-01, 1.447520e-01, 6.300000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.790000e+01, 1.840000e+01, 1.870000e+01, 19, 1.930000e+01, 1.960000e+01, 2.010000e+01, 2.060000e+01 }, // SINR
          { 9.598881e-01, 7.732036e-01, 5.132653e-01, 2.773179e-01, 1.117475e-01, 3.280000e-02, 2.500000e-03, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.746570e+01, 1.810160e+01, 1.873750e+01, 1.937340e+01, 2.000920e+01, 2.064510e+01 }, // SINR
          { 9.894231e-01, 8.818027e-01, 3.852896e-01, 3.920000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.853000e+01, 1.870000e+01, 1.897000e+01, 1.940000e+01, 20 }, // SINR
          { 1, 8.280255e-01, 7.736280e-01, 4.730000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+01, 19, 1.916670e+01, 1.933330e+01, 1.950000e+01 }, // SINR
          { 9.564815e-01, 1.888138e-01, 1.850000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.800210e+01, 1.853070e+01, 1.905930e+01, 1.958790e+01, 2.011650e+01, 2.064510e+01 }, // SINR
          { 9.828244e-01, 7.896341e-01, 2.386364e-01, 1.920000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 3824U, // SINR and BLER for CBS 3824
        NrEesmErrorModel::DoubleTuple{
          { 1.871670e+01, 1.890000e+01, 1.908330e+01, 1.945000e+01, 20 }, // SINR
          { 9.706439e-01, 4.412021e-01, 2.200612e-01, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 18, 1.850000e+01, 1.870000e+01, 1.890000e+01, 1.910000e+01, 1.930000e+01 }, // SINR
          { 9.847328e-01, 4.152961e-01, 1.229207e-01, 1.880000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.733010e+01, 1.834060e+01, 1.935110e+01, 2.036150e+01 }, // SINR
          { 1, 8.879310e-01, 3.740000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.797450e+01, 1.850860e+01, 1.904270e+01, 1.957690e+01, 2.011100e+01 }, // SINR
          { 9.753788e-01, 5.450644e-01, 6.020000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.860000e+01, 1.890000e+01, 1.910000e+01, 1.920000e+01, 1.950000e+01 }, // SINR
          { 1, 9.280576e-01, 3.064320e-01, 4.140000e-02, 2.340000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.804100e+01, 1.856180e+01, 1.908260e+01, 1.960340e+01, 2.012430e+01 }, // SINR
          { 9.555556e-01, 7.001366e-01, 7.580000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.767630e+01, 1.817630e+01, 1.858780e+01, 1.899930e+01, 1.941070e+01, 1.982220e+01 }, // SINR
          { 1, 8.666107e-01, 2.732181e-01, 1.260000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.780140e+01, 1.830140e+01, 1.859430e+01, 1.888730e+01, 1.918030e+01, 1.947320e+01 }, // SINR
          { 9.780534e-01, 8.880208e-01, 4.655331e-01, 8.660000e-02, 4.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.777010e+01, 1.827010e+01, 1.856700e+01, 1.886390e+01, 1.916080e+01, 1.945770e+01 }, // SINR
          { 9.932692e-01, 7.470930e-01, 2.480354e-01, 2.340000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.795960e+01, 1.845960e+01, 1.882390e+01, 1.918810e+01, 1.955230e+01 }, // SINR
          { 9.923077e-01, 7.875767e-01, 2.086794e-01, 7.400000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 26
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.890000e+01, 1.970000e+01, 1.997000e+01, 2.023000e+01, 2.050000e+01, 2.120000e+01 }, // SINR
          { 1, 9.534672e-01, 2.886156e-01, 2.450000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.950000e+01, 1.975000e+01, 20, 2.025000e+01, 2.050000e+01 }, // SINR
          { 9.980769e-01, 7.245763e-01, 7.290000e-02, 1.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.970000e+01, 20, 2.050000e+01, 21 }, // SINR
          { 9.661654e-01, 1.967085e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 1.852410e+01, 1.939640e+01, 1.983250e+01, 2.026870e+01, 2.114100e+01 }, // SINR
          { 1, 6.929348e-01, 5.870000e-02, 1.160000e-02, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.920720e+01, 1.960730e+01, 2.000740e+01, 2.040750e+01, 2.080760e+01 }, // SINR
          { 9.990385e-01, 8.409091e-01, 1.661162e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 1.920000e+01, 1.945000e+01, 1.970000e+01, 1.995000e+01, 2.020000e+01, 2.070000e+01, 2.120000e+01 }, // SINR
          { 9.580224e-01, 9.196429e-01, 8.366013e-01, 7.362717e-01, 5.930233e-01, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.902030e+01, 1.961790e+01, 2.021540e+01, 2.081290e+01 }, // SINR
          { 1, 5.524017e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.934380e+01, 1.972440e+01, 2.010500e+01, 2.048560e+01 }, // SINR
          { 9.568015e-01, 3.020335e-01, 3.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.927090e+01, 1.966190e+01, 2.005300e+01, 2.044400e+01, 2.083500e+01 }, // SINR
          { 9.837786e-01, 5.537281e-01, 3.640000e-02, 8.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.929520e+01, 1.968270e+01, 2.007030e+01, 2.045780e+01, 2.084530e+01 }, // SINR
          { 9.671053e-01, 4.155738e-01, 1.810000e-02, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.920000e+01, 1.947500e+01, 1.975000e+01, 2.002500e+01, 2.030000e+01, 2.080000e+01 }, // SINR
          { 9.740385e-01, 9.276786e-01, 8.368506e-01, 7.131944e-01, 5.556769e-01, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.926610e+01, 1.976610e+01, 1.989070e+01, 2.001520e+01, 2.013970e+01, 2.026430e+01, 2.076430e+01 }, // SINR
          { 9.586397e-01, 6.489899e-01, 3.567416e-01, 1.421291e-01, 4.360000e-02, 1.080000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.918230e+01, 1.958600e+01, 1.998970e+01, 2.039330e+01, 2.079700e+01 }, // SINR
          { 1, 9.411232e-01, 1.927321e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 1.932370e+01, 1.965920e+01, 1.999470e+01, 2.033030e+01 }, // SINR
          { 9.903846e-01, 5.658186e-01, 3.670000e-02, 2.000000e-04 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.960000e+01, 1.977500e+01, 1.995000e+01, 2.012500e+01, 2.030000e+01 }, // SINR
          { 9.923077e-01, 8.176101e-01, 3.056901e-01, 3.430000e-02, 7.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 1.941220e+01, 1.972390e+01, 2.003550e+01, 2.034720e+01, 2.128220e+01 }, // SINR
          { 9.951923e-01, 2.357210e-01, 2.050000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 1.926810e+01, 1.969610e+01, 2.012410e+01, 2.055200e+01 }, // SINR
          { 1, 8.741497e-01, 5.230000e-02, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.890000e+01, 1.940000e+01, 1.960000e+01, 1.980000e+01, 20, 2.020000e+01, 2.070000e+01 }, // SINR
          { 1, 8.368506e-01, 7.147790e-01, 5.641593e-01, 4.206811e-01, 2.726782e-01, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.900250e+01, 1.965470e+01, 2.008960e+01, 2.030700e+01, 2.052440e+01, 2.095930e+01 }, // SINR
          { 1, 9.241071e-01, 6.800000e-02, 3.200000e-03, 7.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.910000e+01, 1.960000e+01, 1.985000e+01, 2.010000e+01, 2.035000e+01, 2.060000e+01 }, // SINR
          { 9.075704e-01, 2.875854e-01, 7.080000e-02, 8.900000e-03, 6.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.945590e+01, 1.992390e+01, 2.015790e+01, 2.039190e+01, 2.085990e+01 }, // SINR
          { 1, 1.836957e-01, 7.140000e-02, 1.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.938170e+01, 1.996530e+01, 2.025710e+01, 2.054900e+01 }, // SINR
          { 1, 2.640000e-02, 2.080000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.958250e+01, 1.980600e+01, 2.002940e+01, 2.025290e+01, 2.092320e+01 }, // SINR
          { 9.217626e-01, 3.828593e-01, 7.580000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.903300e+01, 1.953300e+01, 2.003300e+01, 2.052670e+01 }, // SINR
          { 1, 5.548246e-01, 5.990000e-02, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.979500e+01, 2.011120e+01, 2.042730e+01, 2.074340e+01, 2.105960e+01 }, // SINR
          { 9.828244e-01, 3.945925e-01, 5.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 1.888000e+01, 1.940000e+01, 1.992000e+01, 2.044550e+01, 2.096630e+01, 2.148710e+01 }, // SINR
          { 1, 5.559211e-01, 2.883295e-01, 4.160000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 19, 1.938000e+01, 1.975000e+01, 2.013000e+01, 2.050000e+01 }, // SINR
          { 1, 9.704198e-01, 3.549860e-01, 6.400000e-03, 2.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.981330e+01, 2.005720e+01, 2.022390e+01, 2.039050e+01, 2.055720e+01 }, // SINR
          { 9.533582e-01, 9.157801e-01, 7.250000e-02, 2.910000e-02, 0 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.888300e+01, 1.938300e+01, 1.967470e+01, 1.996630e+01, 2.025800e+01, 2.054970e+01, 2.104970e+01 }, // SINR
          { 1, 8.468543e-01, 7.558824e-01, 6.705729e-01, 5.538043e-01, 4.158416e-01, 0 } // BLER
        }
      },
      { 4032U, // SINR and BLER for CBS 4032
        NrEesmErrorModel::DoubleTuple{
          { 1.935240e+01, 1.969000e+01, 1.986380e+01, 2.003000e+01, 2.037510e+01, 2.088650e+01 }, // SINR
          { 1, 9.855769e-01, 5.680804e-01, 5.289256e-01, 1.000000e-04, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.860000e+01, 1.950000e+01, 20, 2.040000e+01, 2.130000e+01 }, // SINR
          { 1, 8.724832e-01, 1.062554e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.911490e+01, 1.928980e+01, 1.946470e+01, 1.963960e+01, 1.981450e+01, 2.031450e+01, 2.081450e+01 }, // SINR
          { 9.425182e-01, 9.033688e-01, 8.665541e-01, 8.128931e-01, 7.412281e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.910000e+01, 1.967000e+01, 1.990000e+01, 2.023000e+01, 2.080000e+01 }, // SINR
          { 1, 9.699248e-01, 9.521405e-02, 6.220000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.920000e+01, 1.940000e+01, 1.960000e+01, 1.980000e+01, 20, 2.020000e+01 }, // SINR
          { 9.865385e-01, 5.559211e-01, 2.875854e-01, 5.060000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.940000e+01, 1.977000e+01, 1.990000e+01, 2.013000e+01, 2.050000e+01, 2.110000e+01 }, // SINR
          { 1, 6.044601e-01, 3.385695e-01, 2.280000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.885360e+01, 1.935360e+01, 1.957480e+01, 1.979600e+01, 2.001720e+01, 2.023840e+01, 2.073840e+01, 2.123840e+01 }, // SINR
          { 1, 8.437500e-01, 7.558824e-01, 6.432161e-01, 4.898649e-01, 3.620690e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.885000e+01, 1.952000e+01, 1.985000e+01, 2.018000e+01, 2.085000e+01 }, // SINR
          { 1, 4.586331e-01, 3.173804e-01, 6.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.960000e+01, 2.013000e+01, 2.040000e+01, 2.067000e+01, 2.120000e+01 }, // SINR
          { 1, 3.597734e-01, 1.170000e-02, 1.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 27
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 2.046550e+01, 2.084970e+01, 2.123400e+01, 2.161820e+01 }, // SINR
          { 9.971154e-01, 4.342784e-01, 3.000000e-03, 0 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 2.050000e+01, 2.075000e+01, 21, 2.125000e+01, 2.150000e+01 }, // SINR
          { 9.311594e-01, 4.080645e-01, 3.350000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 2.060250e+01, 2.082020e+01, 2.103780e+01, 2.125550e+01, 2.190850e+01 }, // SINR
          { 9.137324e-01, 8.852740e-01, 4.218750e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 2.057080e+01, 2.090780e+01, 2.124470e+01, 2.158170e+01 }, // SINR
          { 1, 5.186492e-01, 2.862812e-01, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 2.040000e+01, 2.097000e+01, 2.130000e+01, 2.153000e+01 }, // SINR
          { 1, 5.160000e-02, 2.990000e-02, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 2.030000e+01, 2.060000e+01, 2.090000e+01, 2.120000e+01, 2.150000e+01, 2.180000e+01, 2.210000e+01, 2.240000e+01, 2.270000e+01, 23 }, // SINR
          { 9.942308e-01, 9.461679e-01, 8.009259e-01, 5.358650e-01, 2.477811e-01, 8.400000e-03, 1.500000e-03, 8.000000e-04, 2.000000e-04, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.975230e+01, 2.025230e+01, 2.075230e+01, 2.093940e+01, 2.112640e+01, 2.131340e+01, 2.150050e+01, 2.200050e+01, 2.250050e+01 }, // SINR
          { 1, 7.544118e-01, 4.932171e-01, 3.825075e-01, 2.745633e-01, 1.812950e-01, 1.138026e-01, 2.600000e-03, 4.000000e-04 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 2.030000e+01, 2.090000e+01, 2.120000e+01, 2.150000e+01, 2.210000e+01 }, // SINR
          { 1, 8.737961e-02, 5.400000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.947670e+01, 2.035680e+01, 2.123680e+01, 2.211690e+01, 2.299690e+01 }, // SINR
          { 9.961538e-01, 8.406863e-01, 2.925408e-01, 2.160000e-02, 1.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.931640e+01, 2.023650e+01, 2.115660e+01, 2.207680e+01, 2.299690e+01 }, // SINR
          { 9.971154e-01, 8.766892e-01, 2.901376e-01, 1.160000e-02, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 2.030000e+01, 2.090000e+01, 2.120000e+01, 2.150000e+01, 2.210000e+01 }, // SINR
          { 1, 7.827744e-01, 5.300000e-03, 1.100000e-03, 0 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.950500e+01, 2.037800e+01, 2.125090e+01, 2.212390e+01, 2.299690e+01 }, // SINR
          { 9.932692e-01, 7.888199e-01, 1.698718e-01, 3.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 1.932990e+01, 2.024670e+01, 2.116340e+01, 2.208010e+01, 2.299690e+01 }, // SINR
          { 1, 9.223214e-01, 3.497928e-01, 1.210000e-02, 3.000000e-04 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 2.012930e+01, 2.062930e+01, 2.092520e+01, 2.122120e+01, 2.151720e+01, 2.181310e+01, 2.231310e+01 }, // SINR
          { 9.425182e-01, 6.911765e-01, 4.655331e-01, 2.009569e-01, 6.600000e-02, 1.530000e-02, 3.000000e-04 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.936540e+01, 2.027330e+01, 2.118120e+01, 2.208900e+01, 2.299690e+01 }, // SINR
          { 1, 9.024823e-01, 1.900376e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.933930e+01, 2.025370e+01, 2.116810e+01, 2.208250e+01, 2.299690e+01 }, // SINR
          { 1, 8.958333e-01, 1.456636e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.034000e+01, 2.067000e+01, 2.101000e+01, 2.134000e+01, 2.234000e+01 }, // SINR
          { 1, 2.107023e-01, 1.170000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.047110e+01, 2.131310e+01, 2.215500e+01, 2.299690e+01 }, // SINR
          { 9.971154e-01, 5.221193e-01, 6.800000e-03, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.070000e+01, 2.085000e+01, 21, 2.115000e+01, 2.130000e+01, 2.180000e+01 }, // SINR
          { 9.865385e-01, 8.750000e-01, 5.141129e-01, 1.669430e-01, 2.990000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.037500e+01, 2.065000e+01, 2.092500e+01, 2.120000e+01 }, // SINR
          { 9.913462e-01, 6.146341e-01, 5.340000e-02, 4.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 2.030000e+01, 2.075000e+01, 2.120000e+01, 2.165000e+01 }, // SINR
          { 1, 7.241379e-01, 5.300000e-03, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.950800e+01, 2.008950e+01, 2.038020e+01, 2.067100e+01, 2.125250e+01 }, // SINR
          { 1, 7.406069e-01, 3.537011e-01, 6.590000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 2.021260e+01, 2.071260e+01, 2.090290e+01, 2.109330e+01, 2.128370e+01, 2.147400e+01, 2.197400e+01 }, // SINR
          { 9.961538e-01, 5.099602e-01, 2.348696e-01, 7.320000e-02, 1.260000e-02, 1.600000e-03, 6.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.966560e+01, 2.016560e+01, 2.066560e+01, 2.095700e+01, 2.124840e+01 }, // SINR
          { 1, 6.732804e-01, 5.330000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.982110e+01, 2.032110e+01, 2.065560e+01, 2.099010e+01, 2.132450e+01, 2.165900e+01 }, // SINR
          { 9.961538e-01, 7.870370e-01, 2.270270e-01, 1.460000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.029560e+01, 2.059580e+01, 2.089590e+01, 2.119600e+01, 2.149620e+01, 2.199620e+01 }, // SINR
          { 9.866412e-01, 8.591667e-01, 4.342784e-01, 7.660000e-02, 3.600000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.980830e+01, 2.044600e+01, 2.108380e+01, 2.172150e+01 }, // SINR
          { 9.990385e-01, 8.311688e-01, 3.740000e-02, 0 } // BLER
        }
      },
      { 4224U, // SINR and BLER for CBS 4224
        NrEesmErrorModel::DoubleTuple{
          { 1.981820e+01, 2.046820e+01, 2.111820e+01, 2.176820e+01 }, // SINR
          { 1, 5.613938e-01, 2.000000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.999690e+01, 2.049690e+01, 2.099690e+01, 2.149690e+01, 2.199690e+01 }, // SINR
          { 9.990385e-01, 2.361891e-01, 8.050000e-02, 1.900000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.010000e+01, 2.060000e+01, 2.080000e+01, 21, 2.120000e+01, 2.140000e+01 }, // SINR
          { 1, 8.389423e-01, 2.781457e-01, 1.940000e-02, 6.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.012850e+01, 2.062850e+01, 2.084680e+01, 2.106520e+01, 2.128350e+01, 2.150190e+01 }, // SINR
          { 9.527778e-01, 5.760135e-01, 1.798214e-01, 2.790000e-02, 2.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 2.010000e+01, 2.060000e+01, 2.080000e+01, 21, 2.120000e+01, 2.140000e+01, 2.190000e+01 }, // SINR
          { 9.438406e-01, 4.166667e-01, 1.827062e-01, 5.320000e-02, 1.130000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.051490e+01, 2.076570e+01, 2.101640e+01, 2.126720e+01, 2.176720e+01 }, // SINR
          { 9.725379e-01, 7.062500e-01, 2.174569e-01, 2.320000e-02, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.024950e+01, 2.074940e+01, 2.099930e+01, 2.124920e+01, 2.174910e+01 }, // SINR
          { 9.932692e-01, 3.532913e-01, 3.820000e-02, 3.200000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.033380e+01, 2.084590e+01, 2.110200e+01, 2.135810e+01 }, // SINR
          { 9.298561e-01, 1.150000e-02, 2.500000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 28
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 2.150000e+01, 2.180000e+01, 2.210000e+01, 2.240000e+01, 2.280000e+01 }, // SINR
          { 1, 9.668561e-01, 3.776119e-01, 1.160000e-02, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 21, 2.120000e+01, 2.140000e+01, 2.160000e+01, 2.180000e+01, 22, 2.220000e+01, 2.240000e+01, 2.260000e+01, 2.280000e+01, 23, 2.320000e+01, 2.340000e+01 }, // SINR
          { 9.846154e-01, 8.932292e-01, 7.695783e-01, 6.350000e-01, 4.708333e-01, 3.123457e-01, 1.852811e-01, 9.914512e-02, 8.400000e-03, 2.400000e-03, 7.000000e-04, 2.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 2.080000e+01, 2.150000e+01, 2.220000e+01, 2.280000e+01, 2.350000e+01, 2.418000e+01 }, // SINR
          { 9.942308e-01, 9.522059e-01, 7.626488e-01, 5.080000e-01, 2.317185e-01, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 2.095000e+01, 2.120000e+01, 2.150000e+01, 2.170000e+01, 22, 2.220000e+01, 2.245000e+01, 2.270000e+01, 2.295000e+01, 2.320000e+01 }, // SINR
          { 1, 8.932292e-01, 7.903963e-01, 6.945946e-01, 5.572687e-01, 4.575812e-01, 7.500000e-03, 5.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 528U, // SINR and BLER for CBS 528
        NrEesmErrorModel::DoubleTuple{
          { 2.130000e+01, 2.160000e+01, 22, 2.230000e+01, 2.270000e+01, 2.305000e+01 }, // SINR
          { 9.913462e-01, 9.583333e-01, 8.640940e-01, 7.205056e-01, 4.975490e-01, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 2.160000e+01, 2.180000e+01, 22, 2.220000e+01, 2.240000e+01, 2.260000e+01, 2.280000e+01 }, // SINR
          { 9.855769e-01, 8.443548e-01, 7.573964e-01, 6.549745e-01, 5.429025e-01, 4.311224e-01, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 2.160000e+01, 2.220000e+01, 2.270000e+01, 2.320000e+01, 2.370000e+01, 2.422000e+01 }, // SINR
          { 9.113475e-01, 6.747382e-01, 3.698680e-01, 1.386439e-01, 2.950000e-02, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 2.060000e+01, 2.127000e+01, 2.160000e+01, 2.193000e+01, 2.260000e+01 }, // SINR
          { 1, 9.875000e-01, 7.656250e-01, 5.506466e-01, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.060000e+01, 2.160000e+01, 2.260000e+01, 2.360000e+01, 2.460000e+01, 2.560000e+01 }, // SINR
          { 1, 6.350000e-01, 4.810000e-02, 4.300000e-03, 1.300000e-03, 6.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.110000e+01, 2.160000e+01, 2.210000e+01, 2.260000e+01, 2.310000e+01, 2.360000e+01, 2.410000e+01, 2.460000e+01, 2.510000e+01, 2.560000e+01, 2.610000e+01 }, // SINR
          { 9.790076e-01, 8.818966e-01, 6.432161e-01, 3.662536e-01, 1.317035e-01, 3.650000e-02, 9.800000e-03, 2.500000e-03, 1.200000e-03, 6.000000e-04, 2.000000e-04 } // BLER
        }
      }
  }
},
{ // BG TYPE 2
  { // MCS 0
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { -2.498600e+00, -2.130300e+00, -1.762000e+00, -1.393700e+00, -1.025400e+00, -6.570900e-01, -2.887500e-01, 7.958400e-02, 4.479200e-01, 8.162500e-01, 1.184600e+00, 1.552900e+00, 1.921300e+00 }, // SINR
          { 9.466912e-01, 8.896552e-01, 7.975460e-01, 6.634115e-01, 4.941860e-01, 3.452869e-01, 1.930941e-01, 8.440000e-02, 3.000000e-02, 7.600000e-03, 1.600000e-03, 4.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { -2.500000e+00, -1.600000e+00, -7.000000e-01, 3.000000e-01, 1.200000e+00, 2.130000e+00 }, // SINR
          { 9.485294e-01, 7.349138e-01, 3.572946e-01, 4.670000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { -2.700000e+00, -2, -1.300000e+00, -6.000000e-01, 1.000000e-01, 8.000000e-01, 1.500000e+00, 2.200000e+00 }, // SINR
          { 9.659091e-01, 8.664966e-01, 6.237805e-01, 3.227564e-01, 7.840000e-02, 8.800000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { -2.570000e+00, -1.900000e+00, -1.300000e+00, -6.000000e-01, 1.000000e-01, 8.000000e-01, 1.480000e+00 }, // SINR
          { 9.531250e-01, 8.274194e-01, 6.237805e-01, 3.227564e-01, 7.840000e-02, 8.800000e-03, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { -2.430000e+00, -1.800000e+00, -1.100000e+00, -5.000000e-01, 1.000000e-01, 7.000000e-01, 1.330000e+00 }, // SINR
          { 9.498175e-01, 7.970679e-01, 4.761236e-01, 1.862056e-01, 3.810000e-02, 2.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { -2.550000e+00, -1.900000e+00, -1.300000e+00, -6.000000e-01, 1.000000e-01, 7.000000e-01 }, // SINR
          { 9.696970e-01, 8.113057e-01, 5.252058e-01, 1.555143e-01, 1.670000e-02, 9.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { -2.550000e+00, -1.900000e+00, -1.200000e+00, -6.000000e-01, 1.000000e-01, 7.000000e-01 }, // SINR
          { 9.678030e-01, 8.370968e-01, 4.715485e-01, 1.459790e-01, 1.060000e-02, 4.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { -3.300000e+00, -2.300000e+00, -1.300000e+00, -3.000000e-01, 7.000000e-01 }, // SINR
          { 9.942308e-01, 9.217626e-01, 5.252058e-01, 7.000000e-02, 9.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { -2.680000e+00, -2.179200e+00, -2.110900e+00, -2.042500e+00, -1.974200e+00, -1.905900e+00, -1.410000e+00, -9.100000e-01, -4.100000e-01, 9.000000e-02, 5.900000e-01, 1.090000e+00, 1.590000e+00 }, // SINR
          { 9.645522e-01, 8.986014e-01, 8.886986e-01, 8.666667e-01, 8.504902e-01, 8.352273e-01, 6.625648e-01, 4.541815e-01, 2.359551e-01, 8.190000e-02, 1.890000e-02, 2.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { -2.200000e+00, -1.950000e+00, -1.700000e+00, -1.450000e+00, -1.200000e+00, -9.500000e-01, -7.000000e-01, -4.500000e-01, -2.000000e-01, 5.000000e-02, 3.000000e-01, 5.500000e-01, 8.000000e-01, 1.050000e+00, 1.300000e+00, 1.550000e+00, 1.800000e+00 }, // SINR
          { 9.058099e-01, 8.470395e-01, 7.765152e-01, 6.779101e-01, 5.852273e-01, 4.630474e-01, 3.572946e-01, 2.519960e-01, 1.567955e-01, 8.975904e-02, 4.670000e-02, 2.090000e-02, 8.800000e-03, 3.200000e-03, 7.000000e-04, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { -3.442700e+00, -2.028200e+00, -1.597100e+00, 2.484000e-01, 6.071200e-01 }, // SINR
          { 9.990385e-01, 8.958333e-01, 6.750000e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { -3.600000e+00, -2.200000e+00, -1.300000e+00, -9.000000e-01, -4.000000e-01, 5.000000e-01, 1.900000e+00 }, // SINR
          { 1, 9.190647e-01, 4.176325e-01, 2.453973e-01, 2.760000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 152U, // SINR and BLER for CBS 152
        NrEesmErrorModel::DoubleTuple{
          { -3.074700e+00, -1.993600e+00, -1.858000e+00, 1.685900e-01 }, // SINR
          { 9.865385e-01, 8.558333e-01, 7.589286e-01, 0 } // BLER
        }
      },
      { 160U, // SINR and BLER for CBS 160
        NrEesmErrorModel::DoubleTuple{
          { -2.153500e+00, -1.884600e+00, -1.388300e+00, -6.231700e-01, 1.419900e-01 }, // SINR
          { 9.258929e-01, 7.842988e-01, 4.824144e-01, 7.210000e-02, 0 } // BLER
        }
      },
      { 176U, // SINR and BLER for CBS 176
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -1.700000e+00, -1.100000e+00, -5.000000e-01, 1.000000e-01, 7.000000e-01 }, // SINR
          { 9.577068e-01, 7.122905e-01, 2.754881e-01, 3.360000e-02, 1.500000e-03, 4.000000e-04 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { -2.800000e+00, -2.100000e+00, -1.300000e+00, -5.000000e-01, 2.000000e-01 }, // SINR
          { 9.932692e-01, 9.122340e-01, 4.170792e-01, 4.260000e-02, 5.000000e-04 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { -2.179100e+00, -2.010800e+00, -1.842400e+00, -1.674100e+00, -1.505800e+00, -1.010000e+00, -5.100000e-01, -1.000000e-02, 4.900000e-01, 9.900000e-01, 1.490000e+00 }, // SINR
          { 9.064685e-01, 8.495066e-01, 8.089623e-01, 7.514535e-01, 6.885027e-01, 4.296075e-01, 1.949612e-01, 5.600000e-02, 9.500000e-03, 1.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { -2.260000e+00, -1.880000e+00, -1.500000e+00, -1.200000e+00, -8.000000e-01, -4.000000e-01, 0, 3.800000e-01 }, // SINR
          { 9.334532e-01, 7.865854e-01, 5.650000e-01, 3.597301e-01, 1.285971e-01, 2.700000e-02, 3.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { -3.107300e+00, -2.086300e+00, -1.573300e+00, -1.065300e+00, -4.423100e-02 }, // SINR
          { 9.971154e-01, 8.801724e-01, 4.889423e-01, 1.981132e-01, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { -2.200000e+00, -1.850000e+00, -1.500000e+00, -1.200000e+00, -8.000000e-01, -5.000000e-01, -1.000000e-01, 2.500000e-01 }, // SINR
          { 9.066901e-01, 7.820122e-01, 5.233740e-01, 2.635983e-01, 6.370000e-02, 1.500000e-02, 1.400000e-03, 6.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { -2.160000e+00, -1.830000e+00, -1.500000e+00, -1.200000e+00, -8.000000e-01, -5.000000e-01, -2.000000e-01, 1.300000e-01, 4.500000e-01, 7.800000e-01 }, // SINR
          { 9.276786e-01, 7.376453e-01, 5.753425e-01, 3.527159e-01, 1.175987e-01, 3.360000e-02, 8.600000e-03, 1.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -2.025000e+00, -1.750000e+00, -1.475000e+00, -1.200000e+00, -9.300000e-01, -6.500000e-01, -3.800000e-01, -1.000000e-01, 1.700000e-01, 4.500000e-01, 7.200000e-01, 1 }, // SINR
          { 9.267857e-01, 8.508333e-01, 7.837423e-01, 6.738281e-01, 5.264523e-01, 3.989028e-01, 2.495059e-01, 1.527947e-01, 7.180000e-02, 3.020000e-02, 1.150000e-02, 3.800000e-03, 1.000000e-03 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { -2.236300e+00, -2.064800e+00, -1.893300e+00, -1.721800e+00, -1.550400e+00, -1.050000e+00, -5.500000e-01, -5.000000e-02, 4.500000e-01, 9.500000e-01 }, // SINR
          { 9.131206e-01, 8.681973e-01, 8.181090e-01, 7.392241e-01, 6.498724e-01, 3.779674e-01, 1.398045e-01, 2.850000e-02, 3.200000e-03, 6.000000e-04 } // BLER
        }
      },
      { 528U, // SINR and BLER for CBS 528
        NrEesmErrorModel::DoubleTuple{
          { -2.700000e+00, -1.900000e+00, -1, -1.000000e-01, 7.000000e-01 }, // SINR
          { 9.932692e-01, 7.984375e-01, 1.040609e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { -2.600000e+00, -2.100000e+00, -1.500000e+00, -9.000000e-01, -3.000000e-01, 2.800000e-01, 8.500000e-01, 1.430000e+00 }, // SINR
          { 9.846154e-01, 9.064685e-01, 5.753425e-01, 1.582598e-01, 1.410000e-02, 7.600000e-03, 6.000000e-04, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -2, -1.700000e+00, -1.400000e+00, -1.100000e+00, -8.000000e-01, -5.000000e-01, -2.000000e-01, 1.000000e-01 }, // SINR
          { 9.289568e-01, 8.437500e-01, 6.844920e-01, 4.520609e-01, 2.224912e-01, 7.910000e-02, 1.950000e-02, 2.900000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { -2.210000e+00, -1.940000e+00, -1.670000e+00, -1.400000e+00, -1.100000e+00, -9.000000e-01, -6.000000e-01, -3.000000e-01, -3.000000e-02 }, // SINR
          { 9.250000e-01, 8.327922e-01, 6.015258e-01, 4.446864e-01, 2.063525e-01, 9.769167e-02, 2.460000e-02, 4.400000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { -2.400000e+00, -2.150000e+00, -1.900000e+00, -1.650000e+00, -1.400000e+00, -1.100000e+00, -9.000000e-01, -7.000000e-01, -4.000000e-01 }, // SINR
          { 9.320652e-01, 8.951049e-01, 8.031250e-01, 6.077381e-01, 3.092752e-01, 9.204728e-02, 3.030000e-02, 7.200000e-03, 5.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -2, -1.700000e+00, -1.400000e+00, -1.100000e+00, -8.000000e-01, -5.000000e-01, -2.000000e-01 }, // SINR
          { 9.580292e-01, 8.657095e-01, 6.324627e-01, 3.876534e-01, 1.702236e-01, 4.500000e-02, 6.300000e-03, 1.000000e-03 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -2.050000e+00, -1.800000e+00, -1.550000e+00, -1.300000e+00, -1, -8.000000e-01, -6.000000e-01, -3.000000e-01, -5.000000e-02 }, // SINR
          { 9.267857e-01, 8.665541e-01, 7.801205e-01, 5.302083e-01, 4.586331e-01, 1.794872e-01, 6.900000e-02, 2.130000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { -2.200000e+00, -1.800000e+00, -1.400000e+00, -9.000000e-01, -5.000000e-01, -7.000000e-02, 3.500000e-01 }, // SINR
          { 9.500000e-01, 7.315341e-01, 3.709440e-01, 6.570000e-02, 6.000000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { -1.980000e+00, -1.700000e+00, -1.500000e+00, -1.200000e+00, -9.000000e-01, -6.000000e-01, -3.200000e-01, -4.000000e-02 }, // SINR
          { 9.798077e-01, 6.844920e-01, 4.735130e-01, 1.855670e-01, 3.930000e-02, 3.100000e-03, 1.200000e-03, 0 } // BLER
        }
      },
      { 1320U, // SINR and BLER for CBS 1320
        NrEesmErrorModel::DoubleTuple{
          { -2.250000e+00, -1.900000e+00, -1.600000e+00, -1.200000e+00, -9.000000e-01, -5.000000e-01, -1.500000e-01 }, // SINR
          { 9.990385e-01, 8.035714e-01, 5.374473e-01, 1.789773e-01, 4.640000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { -1.880000e+00, -1.600000e+00, -1.300000e+00, -1, -8.000000e-01, -5.000000e-01, -2.200000e-01 }, // SINR
          { 9.366071e-01, 4.884615e-01, 2.185864e-01, 5.430000e-02, 1.380000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 1672U, // SINR and BLER for CBS 1672
        NrEesmErrorModel::DoubleTuple{
          { -1.750000e+00, -1.500000e+00, -1.300000e+00, -1, -8.000000e-01, -5.000000e-01, -2.500000e-01 }, // SINR
          { 9.095745e-01, 4.183168e-01, 2.258497e-01, 5.300000e-02, 1.490000e-02, 1.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { -2.191200e+00, -2.012800e+00, -1.763100e+00, -1.513300e+00, -1.381000e+00, -1.263600e+00, -1.013800e+00, -8.140000e-01, -7.641000e-01, -5.857100e-01, -4.073200e-01, -2.289300e-01, -5.054700e-02, 1.278400e-01, 3.062300e-01, 4.846200e-01, 6.630100e-01, 8.414000e-01, 1.019800e+00, 1.198200e+00, 1.376600e+00 }, // SINR
          { 9.078014e-01, 8.608333e-01, 7.924383e-01, 7.026099e-01, 6.556122e-01, 6.083333e-01, 4.956055e-01, 4.087621e-01, 3.887615e-01, 3.103554e-01, 2.305759e-01, 1.700405e-01, 1.173923e-01, 7.480000e-02, 4.670000e-02, 2.670000e-02, 1.520000e-02, 7.700000e-03, 3.400000e-03, 1.800000e-03, 9.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { -2.406200e+00, -1.906200e+00, -1.406200e+00, -9.062000e-01, -7.062000e-01, -5.062000e-01, -3.062000e-01, -2.036000e-01, -1.010000e-01, 1.600000e-03, 1.600000e-03, 2.324500e-01, 2.324500e-01, 4.633000e-01, 4.633000e-01, 6.941500e-01, 6.941500e-01, 9.250000e-01, 9.250000e-01, 1.027600e+00, 1.130200e+00, 1.232800e+00, 1.432800e+00 }, // SINR
          { 9.356884e-01, 8.314516e-01, 6.589744e-01, 4.616426e-01, 3.646132e-01, 2.684989e-01, 1.982283e-01, 1.578947e-01, 1.321278e-01, 1.012251e-01, 1.012251e-01, 5.480000e-02, 5.480000e-02, 2.930000e-02, 2.930000e-02, 1.350000e-02, 1.350000e-02, 5.000000e-03, 5.000000e-03, 3.600000e-03, 2.100000e-03, 1.300000e-03, 7.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { -1.780000e+00, -1.500000e+00, -1.200000e+00, -9.000000e-01, -7.000000e-01, -4.000000e-01, -1.300000e-01 }, // SINR
          { 9.122340e-01, 6.864865e-01, 3.586648e-01, 1.129613e-01, 3.960000e-02, 4.100000e-03, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { -2.149300e+00, -1.994400e+00, -1.775400e+00, -1.556400e+00, -1.315400e+00, -1.271500e+00, -1.090800e+00, -1.074500e+00, -9.100500e-01, -7.551400e-01, -6.002300e-01, -4.453300e-01, -2.904200e-01, -1.355200e-01, 1.938600e-02, 1.742900e-01, 3.292000e-01, 4.841100e-01, 6.390200e-01, 7.939300e-01 }, // SINR
          { 9.040493e-01, 8.534768e-01, 7.559524e-01, 6.568878e-01, 5.306017e-01, 5.059761e-01, 4.024682e-01, 3.852273e-01, 2.991706e-01, 2.178325e-01, 1.603954e-01, 1.085613e-01, 6.830000e-02, 4.070000e-02, 2.520000e-02, 1.360000e-02, 7.400000e-03, 3.300000e-03, 1.300000e-03, 8.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { -1.730000e+00, -1.300000e+00, -8.000000e-01, -4.000000e-01, 0 }, // SINR
          { 9.148936e-01, 4.642210e-01, 5.830000e-02, 2.000000e-03, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { -1.830000e+00, -1.300000e+00, -8.000000e-01, -2.000000e-01, 3.000000e-01 }, // SINR
          { 9.093310e-01, 4.325601e-01, 4.100000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { -1.680000e+00, -1.300000e+00, -9.000000e-01, -5.000000e-01, -2.000000e-01, 2.000000e-01 }, // SINR
          { 9.303571e-01, 4.399123e-01, 7.100000e-02, 3.800000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 3240U, // SINR and BLER for CBS 3240
        NrEesmErrorModel::DoubleTuple{
          { -3.405100e+00, -1.965600e+00, -1.245800e+00, -5.260300e-01, 9.135000e-01 }, // SINR
          { 1, 8.285256e-01, 4.143443e-01, 2.100000e-03, 0 } // BLER
        }
      },
      { 3624U, // SINR and BLER for CBS 3624
        NrEesmErrorModel::DoubleTuple{
          { -2.144400e+00, -1.250000e+00, -3.555700e-01, 1.433300e+00 }, // SINR
          { 9.788462e-01, 4.418403e-01, 2.900000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 1
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { -1.400000e+00, -3.000000e-01, 8.000000e-01, 1.900000e+00, 3 }, // SINR
          { 9.289568e-01, 4.027778e-01, 1.930000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { -1.400000e+00, -5.000000e-01, 4.000000e-01, 1.300000e+00 }, // SINR
          { 9.469424e-01, 3.622159e-01, 9.000000e-03, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { -1.424200e+00, -7.853000e-01, -1.464000e-01, 4.925000e-01, 1.131400e+00, 1.770300e+00 }, // SINR
          { 9.397810e-01, 6.994536e-01, 3.004751e-01, 6.340000e-02, 4.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { -1.611900e+00, -7.471300e-01, 1.176500e-01, 9.824300e-01, 1.847200e+00 }, // SINR
          { 9.583333e-01, 6.250000e-01, 1.202061e-01, 3.300000e-03, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { -1.627200e+00, -1.008000e+00, -3.887600e-01, 2.304600e-01, 8.496800e-01, 1.468900e+00 }, // SINR
          { 9.601852e-01, 8.624161e-01, 4.866412e-01, 1.058024e-01, 6.500000e-03, 3.000000e-04 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { -1.700000e+00, -9.000000e-01, -1.000000e-01, 7.000000e-01, 1.400000e+00 }, // SINR
          { 9.942308e-01, 7.529762e-01, 1.008735e-01, 8.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { -1.662500e+00, -1.116900e+00, -5.712900e-01, -2.562500e-02, 5.200400e-01, 1.065700e+00 }, // SINR
          { 9.699248e-01, 8.136943e-01, 4.396701e-01, 9.883005e-02, 6.400000e-03, 2.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { -1.473700e+00, -9.737000e-01, -6.359800e-01, -2.982700e-01, 3.945200e-02, 3.771700e-01, 8.771700e-01 }, // SINR
          { 9.397482e-01, 8.172468e-01, 5.910138e-01, 3.058575e-01, 1.045875e-01, 2.320000e-02, 7.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { -2.250000e+00, -1.300000e+00, -4.000000e-01, 6.000000e-01, 1.500000e+00, 2.500000e+00 }, // SINR
          { 9.951923e-01, 8.946918e-01, 3.944099e-01, 2.130000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { -1.526300e+00, -1.026300e+00, -6.885800e-01, -3.508600e-01, -1.314800e-02, 3.245700e-01, 8.245700e-01 }, // SINR
          { 9.692308e-01, 8.390523e-01, 5.666667e-01, 2.555556e-01, 6.890000e-02, 1.070000e-02, 4.000000e-04 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { -1.302600e+00, -9.232100e-01, -5.438300e-01, -1.644400e-01, 2.149500e-01, 7.149500e-01 }, // SINR
          { 9.397810e-01, 7.026099e-01, 3.580028e-01, 9.367397e-02, 1.120000e-02, 5.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { -1.909600e+00, -9.946400e-01, -7.965500e-02, 8.353300e-01 }, // SINR
          { 9.980769e-01, 7.942547e-01, 5.650000e-02, 1.000000e-04 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { -1.315100e+00, -8.150900e-01, -3.810300e-01, 5.302900e-02, 4.870900e-01, 9.211500e-01 }, // SINR
          { 9.472222e-01, 6.895161e-01, 2.414611e-01, 2.820000e-02, 2.400000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { -1.432400e+00, -9.323900e-01, -6.278500e-01, -3.233000e-01, -1.876200e-02, 2.857800e-01, 7.857800e-01 }, // SINR
          { 9.790076e-01, 8.349359e-01, 5.976744e-01, 2.851914e-01, 8.573718e-02, 1.630000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { -1.214100e+00, -5.198600e-01, 1.743700e-01, 8.685900e-01 }, // SINR
          { 9.687500e-01, 5.439619e-01, 5.030000e-02, 1.000000e-04 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { -1.447100e+00, -9.190000e-01, -3.908800e-01, 1.372500e-01, 6.653700e-01, 1.193500e+00 }, // SINR
          { 9.884615e-01, 8.132911e-01, 2.936921e-01, 2.710000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { -1.053100e+00, -3.226000e-01, 4.079000e-01, 1.138400e+00 }, // SINR
          { 9.592593e-01, 3.572946e-01, 5.900000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { -1.500000e+00, -8.000000e-01, -2.000000e-01, 4.000000e-01, 1.100000e+00 }, // SINR
          { 9.763258e-01, 5.966435e-01, 1.077209e-01, 3.000000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { -1.400000e+00, -8.000000e-01, -2.000000e-01, 4.000000e-01, 1 }, // SINR
          { 9.645522e-01, 6.237864e-01, 1.014913e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { -1.200000e+00, -8.000000e-01, -3.000000e-01, 1.000000e-01, 6.000000e-01 }, // SINR
          { 9.311594e-01, 6.712963e-01, 1.776874e-01, 2.290000e-02, 7.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { -1.500000e+00, -7.500000e-01, 0, 7.500000e-01, 1.500000e+00, 2.250000e+00, 3 }, // SINR
          { 9.725379e-01, 8.495066e-01, 5.383403e-01, 1.807747e-01, 2.450000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { -1.200000e+00, -8.000000e-01, -3.000000e-01, 1.000000e-01, 5.000000e-01, 9.200000e-01, 1.350000e+00 }, // SINR
          { 9.680451e-01, 7.850610e-01, 3.186090e-01, 6.770000e-02, 5.400000e-03, 1.100000e-03, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { -1.322500e+00, -7.899300e-01, -2.573600e-01, 2.752200e-01 }, // SINR
          { 9.990385e-01, 7.463873e-01, 6.920000e-02, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { -1.350000e+00, -1, -7.000000e-01, -3.000000e-01, 1.000000e-01, 4.000000e-01, 7.500000e-01, 1.100000e+00 }, // SINR
          { 9.012238e-01, 8.625000e-01, 6.250000e-01, 2.150171e-01, 3.280000e-02, 5.300000e-03, 1.700000e-03, 2.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { -1.300000e+00, -9.000000e-01, -5.000000e-01, -1.000000e-01, 3.000000e-01, 7.000000e-01, 1.100000e+00, 1.500000e+00 }, // SINR
          { 9.951923e-01, 9.110915e-01, 5.816210e-01, 1.510856e-01, 1.540000e-02, 2.900000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 736U, // SINR and BLER for CBS 736
        NrEesmErrorModel::DoubleTuple{
          { -1.300000e+00, -1, -6.000000e-01, -2.000000e-01, 2.000000e-01, 5.800000e-01, 9.500000e-01, 1.330000e+00 }, // SINR
          { 9.961538e-01, 9.476103e-01, 6.363065e-01, 1.652961e-01, 1.350000e-02, 2.300000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { -1.309400e+00, -9.554500e-01, -6.014900e-01, -2.475400e-01, 1.064100e-01 }, // SINR
          { 9.990385e-01, 9.084507e-01, 3.420516e-01, 1.420000e-02, 3.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { -1.354500e+00, -9.379000e-01, -5.212900e-01, -1.046900e-01, 3.119200e-01 }, // SINR
          { 9.990385e-01, 8.784247e-01, 1.598726e-01, 8.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { -1.299000e+00, -7.990300e-01, -5.488400e-01, -2.986500e-01, -4.846000e-02, 2.017300e-01 }, // SINR
          { 9.951923e-01, 6.463568e-01, 1.903409e-01, 1.560000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { -1.299800e+00, -7.998500e-01, -5.770100e-01, -3.541600e-01, -1.313100e-01, 9.153800e-02 }, // SINR
          { 9.951923e-01, 7.342857e-01, 3.232648e-01, 5.680000e-02, 3.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { -1.312500e+00, -8.125000e-01, -6.140400e-01, -4.155800e-01, -2.171200e-01, -1.865400e-02 }, // SINR
          { 9.961538e-01, 7.551170e-01, 3.642550e-01, 8.420000e-02, 7.600000e-03, 2.000000e-04 } // BLER
        }
      },
      { 1224U, // SINR and BLER for CBS 1224
        NrEesmErrorModel::DoubleTuple{
          { -1.290400e+00, -9.274100e-01, -5.644300e-01, -2.014500e-01, 1.615400e-01 }, // SINR
          { 9.990385e-01, 9.113475e-01, 2.497520e-01, 3.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { -1.190000e+00, -9.097400e-01, -6.294900e-01, -3.492300e-01, -6.897400e-02 }, // SINR
          { 9.961538e-01, 9.257246e-01, 4.239130e-01, 3.880000e-02, 8.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { -1.109900e+00, -8.397600e-01, -5.696300e-01, -2.994900e-01, -2.935900e-02 }, // SINR
          { 9.809160e-01, 7.681818e-01, 2.038088e-01, 9.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { -1.050000e+00, -7.900000e-01, -5.300000e-01, -2.700000e-01, -1.000000e-02 }, // SINR
          { 9.798077e-01, 7.485380e-01, 1.853767e-01, 8.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { -1.010400e+00, -7.605200e-01, -5.106500e-01, -2.607700e-01, -1.089700e-02 }, // SINR
          { 9.636194e-01, 6.472081e-01, 1.304631e-01, 4.700000e-03, 4.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { -1.470500e+00, -8.811100e-01, -2.917300e-01, 2.976600e-01 }, // SINR
          { 1, 8.301282e-01, 6.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { -9.919200e-01, -7.852600e-01, -5.786100e-01, -3.719500e-01, -1.652900e-01 }, // SINR
          { 9.312500e-01, 5.763575e-01, 1.539634e-01, 1.210000e-02, 5.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { -1.013100e+00, -7.387200e-01, -4.643400e-01, -1.899600e-01, 8.442300e-02 }, // SINR
          { 9.923077e-01, 7.464080e-01, 1.362649e-01, 2.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { -1.298000e+00, -7.979500e-01, -5.728700e-01, -3.477900e-01, -1.227100e-01, 1.023700e-01 }, // SINR
          { 9.932692e-01, 8.009259e-01, 3.024580e-01, 3.040000e-02, 8.000000e-04, 3.000000e-04 } // BLER
        }
      },
      { 2664U, // SINR and BLER for CBS 2664
        NrEesmErrorModel::DoubleTuple{
          { -1.329200e+00, -8.292000e-01, -4.556500e-01, -8.210000e-02 }, // SINR
          { 1, 6.506410e-01, 3.000000e-02, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { -1.210500e+00, -7.667000e-01, -3.229000e-01, 1.209000e-01 }, // SINR
          { 1, 8.183962e-01, 3.020000e-02, 1.000000e-04 } // BLER
        }
      },
      { 3240U, // SINR and BLER for CBS 3240
        NrEesmErrorModel::DoubleTuple{
          { -9.714000e-01, -6.417000e-01, -3.120000e-01, 1.770000e-02 }, // SINR
          { 9.788462e-01, 4.761236e-01, 2.000000e-02, 3.000000e-04 } // BLER
        }
      },
      { 3624U, // SINR and BLER for CBS 3624
        NrEesmErrorModel::DoubleTuple{
          { -1.126900e+00, -8.750300e-01, -6.231600e-01, -3.713000e-01, -1.194300e-01, 3.805700e-01 }, // SINR
          { 9.990385e-01, 9.298561e-01, 4.748134e-01, 4.960000e-02, 1.800000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 2
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, 0, 7.000000e-01, 1.400000e+00, 2.100000e+00 }, // SINR
          { 9.330357e-01, 4.284512e-01, 2.370000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { -1.200000e+00, -4.000000e-01, 4.000000e-01, 1.200000e+00, 2.100000e+00 }, // SINR
          { 1, 8.758503e-01, 2.364232e-01, 4.400000e-03, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, 2.000000e-01, 1.200000e+00, 2.100000e+00 }, // SINR
          { 9.828244e-01, 4.431818e-01, 2.600000e-03, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, 0, 7.000000e-01, 1.300000e+00, 2 }, // SINR
          { 9.903846e-01, 5.854358e-01, 3.100000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { -9.037900e-01, -2.847700e-01, 3.342500e-01, 9.532700e-01, 1.572300e+00, 2.191300e+00 }, // SINR
          { 1, 7.507310e-01, 3.761161e-01, 9.209992e-02, 6.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { -5.981000e-01, 9.245000e-02, 7.830000e-01, 1.473500e+00, 2.164100e+00 }, // SINR
          { 9.406934e-01, 6.113095e-01, 1.542848e-01, 9.300000e-03, 4.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { -1.047100e+00, -4.076100e-01, 2.319200e-01, 8.714500e-01, 1.511000e+00, 2.150500e+00 }, // SINR
          { 1, 8.600993e-01, 4.368557e-01, 8.436749e-02, 5.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { -8.000000e-01, -1.000000e-01, 6.000000e-01, 1.400000e+00 }, // SINR
          { 9.856870e-01, 5.810502e-01, 3.350000e-02, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { -1.100000e+00, -3.000000e-01, 4.000000e-01, 1.200000e+00, 2 }, // SINR
          { 9.990385e-01, 8.330645e-01, 1.780410e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { -9.559100e-01, -4.559100e-01, -9.242400e-02, 2.710600e-01, 6.345500e-01, 9.980300e-01, 1.498000e+00 }, // SINR
          { 1, 7.804878e-01, 4.837786e-01, 2.024960e-01, 5.210000e-02, 7.100000e-03, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { -6.029000e-01, -2.192500e-01, 1.644000e-01, 5.480500e-01, 9.317000e-01, 1.431700e+00 }, // SINR
          { 9.102113e-01, 6.825397e-01, 3.423025e-01, 9.662829e-02, 1.280000e-02, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { -8.529000e-01, -3.529000e-01, -1.523200e-02, 3.224400e-01, 6.601000e-01, 9.977700e-01, 1.497800e+00 }, // SINR
          { 1, 7.294034e-01, 4.480634e-01, 1.678380e-01, 3.410000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { -8.281500e-01, -1.514900e-01, 5.251800e-01, 1.201800e+00, 1.878500e+00 }, // SINR
          { 9.846154e-01, 6.875000e-01, 9.423708e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { -4.634300e-01, -3.808400e-01, -2.982400e-01, -2.156500e-01, -1.330600e-01, 3.669400e-01, 8.669400e-01 }, // SINR
          { 9.320652e-01, 9.005282e-01, 8.468543e-01, 7.960938e-01, 7.201705e-01, 2.100000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { -6.572400e-01, -1.572400e-01, 2.808700e-01, 7.189800e-01, 1.157100e+00, 1.595200e+00 }, // SINR
          { 1, 6.522843e-01, 1.895708e-01, 1.310000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { -3.709700e-01, 1.864700e-02, 4.082600e-01, 7.978800e-01, 1.187500e+00 }, // SINR
          { 9.110915e-01, 5.569690e-01, 1.546818e-01, 1.470000e-02, 7.000000e-04 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { -2.821100e-01, -2.447000e-01, -2.072900e-01, -1.698900e-01, -1.324800e-01, 3.675200e-01, 8.675200e-01 }, // SINR
          { 9.219858e-01, 9.084507e-01, 8.896552e-01, 8.657095e-01, 8.354839e-01, 1.940000e-02, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { -4.202000e-01, 5.120000e-02, 5.226000e-01, 9.940000e-01, 1.465400e+00 }, // SINR
          { 9.503676e-01, 6.425879e-01, 1.514510e-01, 7.500000e-03, 2.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { -3.087300e-01, -1.668800e-01, -2.502600e-02, 1.168200e-01, 6.168200e-01, 1.116800e+00 }, // SINR
          { 9.595865e-01, 9.082168e-01, 8.176752e-01, 6.815160e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { -8.000000e-01, -2.000000e-01, 5.000000e-01, 1.100000e+00, 1.700000e+00 }, // SINR
          { 9.780534e-01, 6.177184e-01, 3.500000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { -5.088500e-01, -8.853100e-03, 3.207600e-01, 6.503700e-01, 9.799900e-01, 1.309600e+00 }, // SINR
          { 1, 6.747382e-01, 2.725806e-01, 4.760000e-02, 2.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { -4.850500e-01, 1.495000e-02, 3.256400e-01, 6.363300e-01, 9.470100e-01, 1.257700e+00 }, // SINR
          { 1, 7.145833e-01, 3.134282e-01, 6.260000e-02, 4.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { -3.218500e-01, -2.361100e-01, -1.503600e-01, -6.461900e-02, 2.112500e-02, 5.211200e-01, 1.021100e+00 }, // SINR
          { 9.580224e-01, 9.205357e-01, 8.649329e-01, 7.984375e-01, 7.176966e-01, 2.300000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { -5.899000e-01, -1.539700e-01, 2.819500e-01, 7.178700e-01, 1.153800e+00 }, // SINR
          { 9.942308e-01, 8.258065e-01, 2.545455e-01, 9.100000e-03, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { -5.281000e-01, -2.810300e-02, 2.414200e-01, 5.109500e-01, 7.804700e-01, 1.050000e+00 }, // SINR
          { 1, 6.752646e-01, 2.627339e-01, 3.970000e-02, 2.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { -6.140700e-01, -1.140700e-01, 1.509900e-01, 4.160400e-01, 6.811000e-01, 9.461500e-01 }, // SINR
          { 1, 6.440355e-01, 2.219790e-01, 3.140000e-02, 1.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { -5.025700e-01, -2.573300e-03, 2.874000e-01, 5.773600e-01, 8.673300e-01, 1.157300e+00 }, // SINR
          { 1, 6.412500e-01, 1.865699e-01, 1.600000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 888U, // SINR and BLER for CBS 888
        NrEesmErrorModel::DoubleTuple{
          { -6.367800e-01, -2.154600e-01, 2.058600e-01, 6.271800e-01, 1.048500e+00 }, // SINR
          { 1, 8.975694e-01, 2.616944e-01, 7.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { -8.928000e-01, -3.928000e-01, 1.072000e-01, 3.153100e-01, 5.234100e-01, 7.315200e-01, 9.396200e-01 }, // SINR
          { 9.570896e-01, 8.486842e-01, 3.981191e-01, 1.189516e-01, 1.760000e-02, 1.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { -9.168100e-01, -4.168100e-01, 8.318800e-02, 2.700800e-01, 4.569800e-01, 6.438700e-01, 8.307700e-01 }, // SINR
          { 9.855769e-01, 8.070312e-01, 4.783835e-01, 1.760168e-01, 3.640000e-02, 3.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { -1, -1.250000e-01, 7.500000e-01, 1.625000e+00, 2.500000e+00 }, // SINR
          { 9.903846e-01, 7.536550e-01, 1.698241e-01, 4.800000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { -1, -5.000000e-01, 0, 2.257700e-01, 4.515400e-01, 6.773100e-01, 9.030800e-01 }, // SINR
          { 9.865385e-01, 8.853448e-01, 7.799080e-01, 3.503472e-01, 6.050000e-02, 4.000000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { -3.746200e-01, -2.462000e-02, 3.253800e-01, 6.753800e-01 }, // SINR
          { 9.826923e-01, 6.337065e-01, 7.000000e-02, 8.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { -8.000000e-01, -1.000000e-01, 6.000000e-01, 1.300000e+00, 2 }, // SINR
          { 9.411232e-01, 5.887097e-01, 1.322674e-01, 6.300000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { -3.000000e-01, -4.000000e-02, 2.200000e-01, 4.800000e-01, 7.400000e-01 }, // SINR
          { 9.580224e-01, 6.475000e-01, 1.455440e-01, 8.600000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { -3.000000e-01, 0, 2.000000e-01, 5.000000e-01, 7.000000e-01, 9.500000e-01, 1.200000e+00, 1.450000e+00, 1.700000e+00, 1.950000e+00 }, // SINR
          { 9.490741e-01, 7.625740e-01, 5.223029e-01, 1.955928e-01, 6.110000e-02, 3.450000e-02, 9.600000e-03, 1.700000e-03, 6.000000e-04, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { -8.000000e-01, -2.500000e-02, 7.500000e-01, 1.525000e+00, 2.300000e+00 }, // SINR
          { 9.633459e-01, 5.731027e-01, 7.960000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { -2.330800e-01, 6.592000e-02, 3.649200e-01, 6.639200e-01 }, // SINR
          { 9.093310e-01, 3.504155e-01, 2.130000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { -4.707700e-01, -8.800200e-02, 2.947700e-01, 6.775300e-01 }, // SINR
          { 9.961538e-01, 6.901882e-01, 5.530000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { -2.884600e-01, -1.372200e-02, 2.610200e-01, 5.357500e-01, 8.104900e-01 }, // SINR
          { 9.564815e-01, 5.269710e-01, 6.250000e-02, 1.100000e-03, 2.000000e-04 } // BLER
        }
      },
      { 2664U, // SINR and BLER for CBS 2664
        NrEesmErrorModel::DoubleTuple{
          { -9.526700e-01, -4.526700e-01, 4.733100e-02, 4.729200e-01, 8.985200e-01 }, // SINR
          { 9.855769e-01, 8.750000e-01, 6.645078e-01, 1.630000e-02, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, -1.000000e-01, 6.000000e-01, 1.200000e+00 }, // SINR
          { 9.942308e-01, 7.765152e-01, 4.330000e-02, 0 } // BLER
        }
      },
      { 3368U, // SINR and BLER for CBS 3368
        NrEesmErrorModel::DoubleTuple{
          { -3.562800e-01, -5.844200e-02, 2.393900e-01, 7.393900e-01, 1.239400e+00, 1.739400e+00 }, // SINR
          { 9.971154e-01, 8.590604e-01, 2.715517e-01, 6.380000e-02, 3.400000e-03, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, -1.000000e-01, 6.000000e-01, 1.200000e+00, 1.900000e+00 }, // SINR
          { 9.971154e-01, 7.522059e-01, 3.820000e-02, 2.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 3
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { -5.000000e-01, 5.000000e-01, 1.500000e+00, 2.500000e+00, 3.500000e+00 }, // SINR
          { 9.990385e-01, 8.608333e-01, 2.906106e-01, 1.730000e-02, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 1.000000e-01, 8.000000e-01, 1.400000e+00, 2.100000e+00, 2.800000e+00 }, // SINR
          { 9.397482e-01, 5.418455e-01, 1.204327e-01, 4.200000e-03, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { -5.000000e-01, 6.000000e-01, 1.700000e+00, 2.900000e+00 }, // SINR
          { 9.990385e-01, 6.844920e-01, 2.390000e-02, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { -2.000000e-01, 4.000000e-01, 1.100000e+00, 1.700000e+00, 2.300000e+00 }, // SINR
          { 9.807692e-01, 7.795455e-01, 1.983097e-01, 1.090000e-02, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { -1.000000e-01, 6.000000e-01, 1.300000e+00, 2, 2.700000e+00 }, // SINR
          { 9.961538e-01, 8.923611e-01, 3.178662e-01, 1.670000e-02, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { -1.000000e-01, 6.000000e-01, 1.300000e+00, 2, 2.700000e+00 }, // SINR
          { 9.971154e-01, 8.854167e-01, 2.902299e-01, 1.180000e-02, 2.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { -1.417400e-01, 3.582600e-01, 8.073900e-01, 1.256500e+00, 1.705700e+00, 2.154800e+00, 2.654800e+00 }, // SINR
          { 1, 8.295455e-01, 5.694444e-01, 2.827915e-01, 8.300000e-02, 1.300000e-02, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { -1.509200e-01, 3.490800e-01, 7.974600e-01, 1.245800e+00, 1.694200e+00, 2.142600e+00, 2.642600e+00 }, // SINR
          { 1, 8.020186e-01, 5.407839e-01, 2.344907e-01, 5.090000e-02, 5.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 6.247500e-02, 6.881000e-01, 1.313700e+00, 1.939400e+00, 2.565000e+00, 3.190600e+00 }, // SINR
          { 9.990385e-01, 7.062500e-01, 2.490138e-01, 2.630000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { -8.323800e-02, 4.234100e-01, 9.300600e-01, 1.436700e+00, 1.943400e+00, 2.450000e+00 }, // SINR
          { 9.990385e-01, 8.640940e-01, 5.183673e-01, 1.565625e-01, 1.760000e-02, 8.000000e-04 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { -8.586200e-02, 4.871900e-01, 1.060200e+00, 1.633300e+00, 2.206300e+00, 2.779400e+00 }, // SINR
          { 1, 7.778614e-01, 3.188131e-01, 4.820000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { -3.077500e-02, 5.098400e-01, 1.050500e+00, 1.591100e+00, 2.131700e+00, 2.672300e+00 }, // SINR
          { 1, 8.160828e-01, 3.548050e-01, 5.100000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.480000e-01, 6.480000e-01, 1.079300e+00, 1.510600e+00, 1.941800e+00, 2.373100e+00 }, // SINR
          { 1, 7.053571e-01, 2.645042e-01, 4.030000e-02, 2.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 2.695700e-01, 9.349800e-01, 1.600400e+00, 2.265800e+00, 2.931200e+00 }, // SINR
          { 9.187500e-01, 3.533520e-01, 1.450000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 2.155800e-01, 9.334600e-01, 1.651300e+00, 2.369200e+00 }, // SINR
          { 9.807692e-01, 4.903846e-01, 2.620000e-02, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.169100e-01, 6.169100e-01, 1.036700e+00, 1.456600e+00, 1.876400e+00, 2.296200e+00 }, // SINR
          { 1, 7.420058e-01, 2.823661e-01, 3.570000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.975700e-01, 6.975700e-01, 1.084400e+00, 1.471200e+00, 1.858100e+00, 2.244900e+00 }, // SINR
          { 9.990385e-01, 7.573529e-01, 3.140625e-01, 4.780000e-02, 2.700000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 2.552300e-01, 8.769200e-01, 1.498600e+00, 2.120300e+00 }, // SINR
          { 9.485294e-01, 3.783683e-01, 1.120000e-02, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 7.001500e-01, 9.405100e-01, 1.180900e+00, 1.421200e+00, 1.661600e+00, 2.161600e+00 }, // SINR
          { 9.047203e-01, 7.391618e-01, 4.865900e-01, 2.240608e-01, 7.470000e-02, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.502900e-01, 6.502900e-01, 1.010500e+00, 1.370600e+00, 1.730800e+00, 2.091000e+00 }, // SINR
          { 1, 8.290323e-01, 4.276756e-01, 1.036379e-01, 7.300000e-03, 3.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 2.330100e-01, 7.330100e-01, 1.059700e+00, 1.386400e+00, 1.713000e+00, 2.039700e+00 }, // SINR
          { 1, 7.234637e-01, 3.459699e-01, 6.610000e-02, 4.100000e-03, 2.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 3.708900e-01, 8.708900e-01, 1.233800e+00, 1.596700e+00, 1.959600e+00, 2.322500e+00 }, // SINR
          { 9.990385e-01, 7.314286e-01, 2.726782e-01, 3.280000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 4.190100e-01, 9.190100e-01, 1.256500e+00, 1.593900e+00, 1.931400e+00, 2.268800e+00 }, // SINR
          { 9.990385e-01, 7.058011e-01, 2.558435e-01, 3.420000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 4.136600e-01, 9.136600e-01, 1.239000e+00, 1.564300e+00, 1.889700e+00, 2.215000e+00 }, // SINR
          { 1, 6.516497e-01, 2.189130e-01, 2.240000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 4.564400e-01, 9.564400e-01, 1.244200e+00, 1.532000e+00, 1.819700e+00, 2.107500e+00 }, // SINR
          { 9.990385e-01, 6.544872e-01, 2.084718e-01, 2.400000e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 4.836300e-01, 9.836300e-01, 1.237700e+00, 1.491800e+00, 1.745900e+00, 2 }, // SINR
          { 1, 5.737613e-01, 1.785461e-01, 1.960000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 5.477000e-01, 1.047700e+00, 1.258900e+00, 1.470100e+00, 1.681300e+00, 1.892500e+00 }, // SINR
          { 9.951923e-01, 4.442509e-01, 1.301921e-01, 1.940000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 5.130000e-01, 1.013000e+00, 1.206000e+00, 1.399000e+00, 1.592000e+00, 1.785000e+00 }, // SINR
          { 9.990385e-01, 3.821536e-01, 1.101507e-01, 1.640000e-02, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 5.546000e-01, 1.054600e+00, 1.286400e+00, 1.518300e+00, 1.750100e+00 }, // SINR
          { 9.923077e-01, 4.018987e-01, 9.792863e-02, 9.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 5.000000e-01, 1, 1.217400e+00, 1.434700e+00, 1.652100e+00, 1.869500e+00 }, // SINR
          { 9.971154e-01, 7.037293e-01, 3.311518e-01, 7.490000e-02, 8.000000e-03, 4.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 5.787800e-01, 8.733600e-01, 1.167900e+00, 1.462500e+00, 1.757100e+00, 2.257100e+00 }, // SINR
          { 9.971154e-01, 8.666667e-01, 3.764925e-01, 4.290000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 5.625000e-01, 1.062500e+00, 1.280400e+00, 1.498300e+00, 1.716300e+00, 1.934200e+00 }, // SINR
          { 9.971154e-01, 5.487013e-01, 1.655184e-01, 1.790000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 5.805100e-01, 8.602600e-01, 1.140000e+00, 1.419800e+00, 1.699500e+00 }, // SINR
          { 9.932692e-01, 8.649329e-01, 3.500693e-01, 3.250000e-02, 5.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 6.551300e-01, 9.250000e-01, 1.194900e+00, 1.464700e+00, 1.734600e+00 }, // SINR
          { 9.961538e-01, 8.733108e-01, 3.908669e-01, 4.110000e-02, 8.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 7.100000e-01, 9.700000e-01, 1.230000e+00, 1.490000e+00, 1.750000e+00 }, // SINR
          { 9.633459e-01, 6.714660e-01, 1.627756e-01, 7.700000e-03, 3.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 4.950000e-01, 7.898000e-01, 1.084600e+00, 1.379400e+00, 1.674200e+00 }, // SINR
          { 9.971154e-01, 8.583333e-01, 2.793792e-01, 1.220000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 7.605100e-01, 9.718300e-01, 1.183200e+00, 1.394500e+00, 1.605800e+00 }, // SINR
          { 9.321429e-01, 6.035714e-01, 1.626779e-01, 1.350000e-02, 3.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 5.257700e-01, 7.813800e-01, 1.037000e+00, 1.292600e+00, 1.548200e+00 }, // SINR
          { 9.980769e-01, 9.232143e-01, 4.875000e-01, 5.830000e-02, 6.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 6.214000e-01, 1.121400e+00, 1.329000e+00, 1.536600e+00, 1.744200e+00 }, // SINR
          { 1, 2.142249e-01, 2.330000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 5.420000e-01, 1.042000e+00, 1.332700e+00, 1.623300e+00, 1.914000e+00 }, // SINR
          { 9.990385e-01, 6.117788e-01, 7.180000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 2728U, // SINR and BLER for CBS 2728
        NrEesmErrorModel::DoubleTuple{
          { 5.874000e-01, 1.087400e+00, 1.367000e+00, 1.646700e+00 }, // SINR
          { 9.990385e-01, 2.943925e-01, 1.040000e-02, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 5.145500e-01, 1.042000e+00, 1.569500e+00, 2.096900e+00 }, // SINR
          { 9.990385e-01, 6.135266e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 7.646800e-01, 1.127500e+00, 1.490300e+00, 1.853100e+00 }, // SINR
          { 9.826923e-01, 4.599820e-01, 1.280000e-02, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 5.736200e-01, 1, 1.426400e+00, 1.852800e+00 }, // SINR
          { 9.990385e-01, 7.780303e-01, 3.250000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 4
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 1, 1.800000e+00, 2.200000e+00, 2.600000e+00, 3.400000e+00 }, // SINR
          { 1, 3.083942e-01, 6.770000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 6.531000e-01, 1.265400e+00, 1.877700e+00, 2.490000e+00, 3.102300e+00 }, // SINR
          { 9.894231e-01, 8.101266e-01, 3.037861e-01, 2.580000e-02, 9.000000e-04 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 9.961000e-01, 1.516900e+00, 2.037700e+00, 2.558600e+00, 3.079400e+00 }, // SINR
          { 9.466912e-01, 6.472081e-01, 1.600064e-01, 9.300000e-03, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 6.000000e-01, 1.200000e+00, 1.630000e+00, 1.800000e+00, 2.070000e+00, 2.500000e+00, 3.100000e+00 }, // SINR
          { 1, 9.913462e-01, 3.981918e-01, 3.757440e-01, 5.020000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 5.000000e-01, 1.275000e+00, 2.050000e+00, 2.825000e+00, 3.600000e+00 }, // SINR
          { 1, 8.077532e-01, 5.820000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 7.407700e-01, 1.458000e+00, 2.175200e+00, 2.892500e+00, 3.609700e+00, 4.326900e+00 }, // SINR
          { 9.456522e-01, 6.664508e-01, 2.512425e-01, 3.420000e-02, 2.200000e-03, 2.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 8.217700e-01, 1.448300e+00, 2.074800e+00, 2.701300e+00, 3.327900e+00, 3.954400e+00 }, // SINR
          { 9.356618e-01, 7.566568e-01, 3.660201e-01, 7.700000e-02, 5.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 4.386000e-01, 9.386000e-01, 1.438600e+00, 1.706700e+00, 1.974700e+00, 2.242800e+00, 2.510800e+00, 3.010800e+00, 3.510800e+00, 4.010800e+00 }, // SINR
          { 1, 8.818493e-01, 7.814417e-01, 6.330446e-01, 4.438596e-01, 2.666490e-01, 1.283129e-01, 2.070000e-02, 3.000000e-03, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 3.870000e-01, 1.020900e+00, 1.654800e+00, 2.288700e+00, 2.922600e+00, 3.556500e+00 }, // SINR
          { 9.913462e-01, 8.862847e-01, 5.561674e-01, 1.506295e-01, 1.270000e-02, 4.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 5.546200e-01, 1.149700e+00, 1.744800e+00, 2.339900e+00, 2.934900e+00, 3.530000e+00 }, // SINR
          { 9.617537e-01, 8.724662e-01, 4.995079e-01, 1.113090e-01, 7.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 5.828800e-01, 1.167000e+00, 1.751200e+00, 2.335300e+00, 2.919400e+00, 3.503500e+00 }, // SINR
          { 9.699248e-01, 8.775510e-01, 4.819392e-01, 1.005390e-01, 6.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 1.888300e-01, 1.010800e+00, 1.832900e+00, 2.654900e+00, 3.476900e+00 }, // SINR
          { 9.990385e-01, 8.767007e-01, 2.725054e-01, 9.100000e-03, 2.000000e-04 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 8.031500e-01, 1.332600e+00, 1.862000e+00, 2.391500e+00, 2.920900e+00, 3.450400e+00 }, // SINR
          { 9.713740e-01, 8.258065e-01, 3.673692e-01, 6.150000e-02, 3.100000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 9.113000e-01, 1.413800e+00, 1.916300e+00, 2.418800e+00, 2.921300e+00, 3.423800e+00 }, // SINR
          { 9.049296e-01, 6.087740e-01, 1.741690e-01, 1.740000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.005300e+00, 1.505300e+00, 1.978300e+00, 2.451300e+00, 2.924300e+00, 3.397300e+00 }, // SINR
          { 9.082168e-01, 6.987705e-01, 2.348414e-01, 2.640000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 9.621000e-01, 1.462100e+00, 1.939300e+00, 2.416500e+00, 2.893600e+00, 3.370800e+00 }, // SINR
          { 9.303571e-01, 6.988950e-01, 2.400568e-01, 2.920000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 9.607000e-01, 1.460700e+00, 1.924900e+00, 2.389200e+00, 2.853400e+00, 3.317700e+00 }, // SINR
          { 9.847328e-01, 5.980047e-01, 1.545567e-01, 1.480000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 9.645000e-01, 1.464500e+00, 1.914500e+00, 2.364500e+00, 2.814600e+00, 3.264600e+00 }, // SINR
          { 9.555556e-01, 6.354680e-01, 1.597577e-01, 1.090000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 8.037500e-01, 1.320600e+00, 1.837500e+00, 2.354300e+00, 2.871200e+00 }, // SINR
          { 9.971154e-01, 8.835616e-01, 3.497230e-01, 3.010000e-02, 4.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 5.414500e-01, 1.195700e+00, 1.850000e+00, 2.504200e+00, 3.158500e+00 }, // SINR
          { 9.990385e-01, 8.503289e-01, 1.541411e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 9.441000e-01, 1.444100e+00, 1.859400e+00, 2.274700e+00, 2.690100e+00, 3.105400e+00 }, // SINR
          { 9.577206e-01, 6.445707e-01, 1.614516e-01, 1.190000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 8.758500e-01, 1.336700e+00, 1.797500e+00, 2.258400e+00, 2.719200e+00 }, // SINR
          { 9.951923e-01, 8.707770e-01, 3.783482e-01, 4.420000e-02, 7.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.297700e+00, 1.754700e+00, 2.211600e+00, 2.668600e+00, 3.168600e+00 }, // SINR
          { 9.598881e-01, 6.219512e-01, 1.377888e-01, 8.000000e-03, 0 } // BLER
        }
      },
      { 480U, // SINR and BLER for CBS 480
        NrEesmErrorModel::DoubleTuple{
          { 1.011600e+00, 1.511600e+00, 1.870200e+00, 2.228900e+00, 2.587500e+00, 2.946200e+00 }, // SINR
          { 9.826923e-01, 8.880208e-01, 4.879344e-01, 1.140210e-01, 8.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.184500e+00, 1.684500e+00, 1.973400e+00, 2.262300e+00, 2.551100e+00, 2.840000e+00 }, // SINR
          { 9.307554e-01, 6.699219e-01, 2.683902e-01, 4.330000e-02, 2.300000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 6.791000e-01, 1.179100e+00, 1.679100e+00, 1.942800e+00, 2.206500e+00, 2.470100e+00, 2.733800e+00 }, // SINR
          { 9.971154e-01, 8.871528e-01, 7.791411e-01, 3.753698e-01, 9.079903e-02, 8.700000e-03, 4.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.146900e+00, 1.646900e+00, 1.892100e+00, 2.137300e+00, 2.382500e+00, 2.627700e+00 }, // SINR
          { 9.740385e-01, 6.824866e-01, 2.976051e-01, 6.150000e-02, 5.500000e-03, 5.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.375000e+00, 1.875000e+00, 2.113800e+00, 2.352600e+00, 2.591500e+00, 2.830300e+00 }, // SINR
          { 9.255319e-01, 3.149876e-01, 6.470000e-02, 5.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.199800e+00, 1.579600e+00, 1.959500e+00, 2.339300e+00, 2.719200e+00 }, // SINR
          { 9.894231e-01, 7.704545e-01, 1.727335e-01, 4.500000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.250000e+00, 1.750000e+00, 1.964600e+00, 2.179100e+00, 2.393700e+00, 2.608200e+00 }, // SINR
          { 9.759615e-01, 4.340753e-01, 1.343884e-01, 1.630000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.320800e+00, 1.614900e+00, 1.909000e+00, 2.203100e+00, 2.497200e+00 }, // SINR
          { 9.951923e-01, 8.176752e-01, 3.168970e-01, 2.730000e-02, 2.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.250000e+00, 1.750000e+00, 1.981300e+00, 2.212700e+00, 2.444000e+00, 2.675400e+00 }, // SINR
          { 9.742366e-01, 5.652902e-01, 1.900452e-01, 2.880000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.325600e+00, 1.605100e+00, 1.884600e+00, 2.164100e+00, 2.443600e+00 }, // SINR
          { 9.846154e-01, 8.197115e-01, 3.145161e-01, 3.190000e-02, 1.000000e-03 } // BLER
        }
      },
      { 1480U, // SINR and BLER for CBS 1480
        NrEesmErrorModel::DoubleTuple{
          { 1.402600e+00, 1.672300e+00, 1.942100e+00, 2.211800e+00, 2.481500e+00 }, // SINR
          { 9.713740e-01, 7.161017e-01, 2.256261e-01, 1.560000e-02, 3.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.460000e+00, 1.720000e+00, 1.980000e+00, 2.240000e+00, 2.500000e+00 }, // SINR
          { 9.583333e-01, 6.274510e-01, 1.480882e-01, 8.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 9.979000e-01, 1.497900e+00, 1.685600e+00, 1.873300e+00, 2.061000e+00, 2.248700e+00, 2.748700e+00 }, // SINR
          { 9.971154e-01, 8.835034e-01, 5.683333e-01, 1.904223e-01, 3.080000e-02, 2.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.516400e+00, 1.684700e+00, 1.853100e+00, 2.021400e+00, 2.189800e+00, 2.689800e+00 }, // SINR
          { 9.151786e-01, 6.752646e-01, 3.225703e-01, 8.060000e-02, 1.050000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.284600e+00, 1.502600e+00, 1.720700e+00, 1.938700e+00, 2.156700e+00, 2.656700e+00 }, // SINR
          { 9.932692e-01, 9.110915e-01, 5.755682e-01, 1.538929e-01, 1.550000e-02, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.273800e+00, 1.544600e+00, 1.815400e+00, 2.086200e+00, 2.357000e+00 }, // SINR
          { 9.971154e-01, 8.558333e-01, 3.418022e-01, 3.020000e-02, 3.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.243600e+00, 1.587400e+00, 1.931200e+00, 2.274900e+00, 2.618700e+00 }, // SINR
          { 9.971154e-01, 8.023438e-01, 1.153846e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.472800e+00, 1.716700e+00, 1.960600e+00, 2.204500e+00, 2.448400e+00 }, // SINR
          { 9.589552e-01, 6.387500e-01, 1.453993e-01, 5.600000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.373700e+00, 1.873700e+00, 2.282500e+00, 2.691300e+00 }, // SINR
          { 9.694656e-01, 3.695175e-01, 2.500000e-03, 0 } // BLER
        }
      },
      { 3368U, // SINR and BLER for CBS 3368
        NrEesmErrorModel::DoubleTuple{
          { 1.166800e+00, 1.524500e+00, 1.882200e+00, 2.239900e+00, 2.597600e+00 }, // SINR
          { 1, 8.986014e-01, 1.873145e-01, 8.000000e-04, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 9.700000e-01, 1.470400e+00, 1.680100e+00, 1.889900e+00, 2.099600e+00, 2.309300e+00, 2.810000e+00, 3.310000e+00 }, // SINR
          { 9.356884e-01, 6.472081e-01, 4.377148e-01, 2.335343e-01, 1.075881e-01, 4.230000e-02, 7.000000e-03, 9.000000e-04 } // BLER
        }
      }
  },
  { // MCS 5
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 1.600000e+00, 2.400000e+00, 2.800000e+00, 3.200000e+00, 4 }, // SINR
          { 1, 6.864973e-01, 9.786184e-02, 3.000000e-03, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 1.700000e+00, 2.600000e+00, 3.400000e+00, 4.300000e+00, 5.100000e+00 }, // SINR
          { 9.379496e-01, 4.815341e-01, 7.880000e-02, 1.400000e-03, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 1.700000e+00, 2, 2.300000e+00, 2.600000e+00, 2.900000e+00, 3.200000e+00, 3.500000e+00, 3.800000e+00, 4.100000e+00, 4.400000e+00 }, // SINR
          { 9.661654e-01, 8.125000e-01, 5.910138e-01, 3.525910e-01, 1.563275e-01, 4.980000e-02, 1.070000e-02, 3.000000e-03, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 1.800000e+00, 2.300000e+00, 2.500000e+00, 2.800000e+00, 3.300000e+00 }, // SINR
          { 9.980769e-01, 8.598993e-01, 4.063505e-01, 1.097028e-01, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 1.800000e+00, 2.400000e+00, 3, 3.600000e+00 }, // SINR
          { 1, 8.039063e-01, 3.000000e-03, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 2.140400e+00, 2.331000e+00, 2.521600e+00, 2.712200e+00, 2.902800e+00, 3.260200e+00 }, // SINR
          { 9.836538e-01, 8.675000e-01, 3.553371e-01, 1.382013e-01, 1.580000e-02, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 2.178500e+00, 2.535300e+00, 2.892100e+00, 3.248900e+00, 3.605700e+00 }, // SINR
          { 9.791667e-01, 4.965686e-01, 5.710000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 1.250000e+00, 2, 2.800000e+00, 3.500000e+00, 4.300000e+00, 5 }, // SINR
          { 9.990385e-01, 8.775685e-01, 2.542843e-01, 1.200000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 2.150000e+00, 2.505000e+00, 2.860000e+00, 3.215000e+00 }, // SINR
          { 9.990385e-01, 7.787879e-01, 2.420000e-02, 1.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 1.069600e+00, 1.777200e+00, 2.484800e+00, 3.192400e+00, 3.899900e+00, 4.607500e+00 }, // SINR
          { 9.923077e-01, 8.835034e-01, 4.665751e-01, 6.670000e-02, 2.200000e-03, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 1.230000e+00, 1.900000e+00, 2.500000e+00, 3.200000e+00, 3.900000e+00, 4.600000e+00 }, // SINR
          { 9.875000e-01, 8.266129e-01, 4.501779e-01, 6.720000e-02, 2.300000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 1.700000e+00, 2.200000e+00, 2.700000e+00, 3.200000e+00, 3.700000e+00, 4.200000e+00 }, // SINR
          { 9.122340e-01, 7.078729e-01, 2.630480e-01, 3.650000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 2.089300e+00, 3.825000e+00, 4.175100e+00, 4.525300e+00 }, // SINR
          { 9.000000e-01, 2.500000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 2.254500e+00, 3.800000e+00, 4.148900e+00 }, // SINR
          { 9.000000e-01, 1.900000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 2.216244e+00, 3.775000e+00, 4.122700e+00 }, // SINR
          { 9.000000e-01, 1.800000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 2.315500e+00, 3.750000e+00, 4.096500e+00, 4.443100e+00 }, // SINR
          { 9.000000e-01, 1.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 2.245000e+00, 3.700000e+00, 4.044100e+00 }, // SINR
          { 9.000000e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 2.270083e+00, 3.650000e+00, 3.991700e+00 }, // SINR
          { 9.000000e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 2.281300e+00, 2.281333e+00, 2.720900e+00, 3.160400e+00, 3.600000e+00 }, // SINR
          { 9.736842e-01, 9.000000e-01, 3.364362e-01, 7.150000e-02, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 2.200000e+00, 2.400000e+00, 2.700000e+00, 2.900000e+00, 3.100000e+00 }, // SINR
          { 9.586466e-01, 7.326590e-01, 1.539522e-01, 1.710000e-02, 8.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 2.200000e+00, 2.500000e+00, 2.800000e+00, 3.200000e+00 }, // SINR
          { 9.742366e-01, 5.658186e-01, 6.430000e-02, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 2.300000e+00, 2.500000e+00, 2.700000e+00, 2.900000e+00, 3.100000e+00, 3.300000e+00 }, // SINR
          { 9.807692e-01, 7.875767e-01, 3.396667e-01, 5.130000e-02, 2.600000e-03, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.600000e+00, 1.900000e+00, 2.200000e+00, 2.500000e+00, 2.800000e+00, 3.100000e+00, 3.400000e+00, 3.700000e+00, 4 }, // SINR
          { 9.375000e-01, 8.591667e-01, 7.478198e-01, 4.756554e-01, 2.148038e-01, 5.470000e-02, 8.400000e-03, 3.300000e-03, 5.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.600000e+00, 1.900000e+00, 2.200000e+00, 2.500000e+00, 2.800000e+00, 3.100000e+00, 3.400000e+00, 3.700000e+00, 4, 4.300000e+00 }, // SINR
          { 9.375000e-01, 8.591667e-01, 6.759259e-01, 4.458627e-01, 2.137990e-01, 7.620000e-02, 1.770000e-02, 3.300000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 608U, // SINR and BLER for CBS 608
        NrEesmErrorModel::DoubleTuple{
          { 2.230000e+00, 2.500000e+00, 2.800000e+00, 3, 3.300000e+00 }, // SINR
          { 9.687500e-01, 5.821918e-01, 5.190000e-02, 2.500000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 3.150000e+00, 3.467700e+00, 3.785400e+00 }, // SINR
          { 9.000000e-01, 2.160000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.798400e+00, 2.111300e+00, 2.424200e+00, 2.737100e+00, 3.050000e+00, 3.362900e+00, 3.675800e+00 }, // SINR
          { 9.160714e-01, 8.965517e-01, 8.415033e-01, 3.005952e-01, 3.440000e-02, 1.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.594600e+00, 2.950000e+00, 3.258100e+00, 3.566200e+00 }, // SINR
          { 9.000000e-01, 6.030000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 2.270000e+00, 2.500000e+00, 2.700000e+00, 3, 3.200000e+00 }, // SINR
          { 9.512868e-01, 6.912162e-01, 2.203833e-01, 4.500000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 2.153000e+00, 2.451500e+00, 2.750000e+00, 3.048500e+00, 3.346900e+00, 3.645400e+00 }, // SINR
          { 9.971154e-01, 6.237745e-01, 1.738981e-01, 1.710000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.680000e+00, 1.910000e+00, 2.140000e+00, 2.370000e+00, 2.600000e+00, 2.800000e+00, 3.100000e+00, 3.300000e+00 }, // SINR
          { 9.875000e-01, 8.583333e-01, 7.258523e-01, 6.984890e-01, 5.263430e-01, 1.197469e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.972400e+00, 2.261200e+00, 2.550000e+00, 2.838800e+00, 3.127700e+00, 3.416500e+00, 3.705400e+00 }, // SINR
          { 1, 8.715986e-01, 4.736111e-01, 1.019377e-01, 6.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+00, 2.350000e+00, 2.629200e+00, 2.656250e+00, 2.908500e+00, 3.187700e+00 }, // SINR
          { 1, 7.536982e-01, 3.097426e-01, 4.460000e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.150000e+00, 2.419600e+00, 2.689200e+00, 2.958800e+00, 3.228500e+00, 3.498100e+00 }, // SINR
          { 9.429348e-01, 6.419598e-01, 1.829578e-01, 1.780000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.200000e+00, 2.500000e+00, 2.700000e+00, 3, 3.300000e+00 }, // SINR
          { 9.932692e-01, 7.759146e-01, 3.111111e-01, 7.300000e-03, 0 } // BLER
        }
      },
      { 1800U, // SINR and BLER for CBS 1800
        NrEesmErrorModel::DoubleTuple{
          { 2.300000e+00, 2.400000e+00, 2.600000e+00, 2.800000e+00, 3, 3.180000e+00 }, // SINR
          { 9.320652e-01, 7.898773e-01, 3.346561e-01, 4.670000e-02, 2.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 2.031500e+00, 2.272300e+00, 2.513100e+00, 2.656250e+00, 2.753800e+00, 2.946400e+00 }, // SINR
          { 9.903846e-01, 8.923611e-01, 5.110442e-01, 1.507841e-01, 1.256225e-01, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.043500e+00, 2.274600e+00, 2.505800e+00, 2.737000e+00, 2.968100e+00, 3.199300e+00 }, // SINR
          { 9.942308e-01, 9.051724e-01, 5.445279e-01, 3.227848e-01, 7.810000e-02, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 2.200000e+00, 2.400000e+00, 2.600000e+00, 2.800000e+00, 3, 3.200000e+00 }, // SINR
          { 9.020979e-01, 7.326590e-01, 3.944099e-01, 5.120000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+00, 2.100000e+00, 2.350000e+00, 2.600000e+00, 2.850000e+00, 3.100000e+00, 3.350000e+00, 3.600000e+00, 3.850000e+00 }, // SINR
          { 9.219858e-01, 7.875767e-01, 5.691964e-01, 3.489011e-01, 1.608280e-01, 5.910000e-02, 2.400000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 2.100000e+00, 2.470000e+00, 2.600000e+00, 2.830000e+00, 3.200000e+00 }, // SINR
          { 1, 8.388158e-01, 4.055466e-01, 1.137704e-01, 0 } // BLER
        }
      },
      { 2976U, // SINR and BLER for CBS 2976
        NrEesmErrorModel::DoubleTuple{
          { 1.017300e+00, 2.147000e+00, 2.711900e+00, 3.276800e+00, 4.406500e+00 }, // SINR
          { 1, 9.184397e-01, 3.294271e-01, 8.900000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.900000e+00, 2.100000e+00, 2.300000e+00, 2.500000e+00, 2.700000e+00, 2.900000e+00, 3.100000e+00, 3.300000e+00, 3.500000e+00, 3.700000e+00, 3.900000e+00, 4.100000e+00 }, // SINR
          { 9.406934e-01, 8.398693e-01, 7.485294e-01, 5.745516e-01, 3.787764e-01, 2.179376e-01, 9.815574e-02, 1.720000e-02, 4.400000e-03, 8.000000e-04, 2.000000e-04, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.300000e+00, 2.300000e+00, 2.630000e+00, 2.970000e+00, 3.300000e+00 }, // SINR
          { 1, 9.846154e-01, 3.681159e-01, 6.900000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 6
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 2.838100e+00, 3.198200e+00, 3.558200e+00, 3.918300e+00 }, // SINR
          { 9.961538e-01, 6.355198e-01, 6.550000e-02, 1.000000e-04 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 2.250000e+00, 2.800000e+00, 3.300000e+00, 3.900000e+00, 4.400000e+00, 5, 5.550000e+00, 6.100000e+00 }, // SINR
          { 9.574074e-01, 8.412829e-01, 6.035714e-01, 2.560729e-01, 6.340000e-02, 5.600000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 2.533500e+00, 3.158500e+00, 3.575200e+00, 3.783500e+00, 3.991800e+00 }, // SINR
          { 1, 9.145683e-01, 3.040000e-02, 2.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 3, 3.348300e+00, 3.696700e+00, 4.045000e+00 }, // SINR
          { 9.561567e-01, 2.418893e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 2.999200e+00, 3.335900e+00, 3.672600e+00, 4.009300e+00 }, // SINR
          { 9.923077e-01, 5.985915e-01, 1.890000e-02, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 2.871100e+00, 3.150000e+00, 3.429000e+00, 3.707900e+00, 3.986800e+00 }, // SINR
          { 9.942308e-01, 7.643072e-01, 1.165037e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 3.140300e+00, 3.418800e+00, 3.697200e+00, 3.975700e+00 }, // SINR
          { 9.715909e-01, 4.765918e-01, 2.920000e-02, 2.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 3.041800e+00, 3.272500e+00, 3.503200e+00, 3.733900e+00, 3.964600e+00 }, // SINR
          { 9.971154e-01, 8.624161e-01, 3.202532e-01, 2.340000e-02, 2.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 3.022800e+00, 3.252700e+00, 3.482500e+00, 3.712400e+00, 3.942300e+00 }, // SINR
          { 9.913462e-01, 8.368506e-01, 2.703426e-01, 1.330000e-02, 3.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 2.862500e+00, 3.215000e+00, 3.567500e+00, 3.920000e+00 }, // SINR
          { 1, 7.914110e-01, 4.860000e-02, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 2.695100e+00, 3.208600e+00, 3.722000e+00, 4.235500e+00 }, // SINR
          { 9.375000e-01, 3.652457e-01, 1.500000e-02, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 2.800000e+00, 3.330000e+00, 3.600000e+00, 3.870000e+00, 4.400000e+00 }, // SINR
          { 1, 3.073422e-01, 1.450000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 2.200000e+00, 2.870000e+00, 3.200000e+00, 3.530000e+00, 4.200000e+00 }, // SINR
          { 1, 9.636194e-01, 5.825688e-01, 1.036534e-01, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 2.400000e+00, 3.200000e+00, 3.900000e+00, 4.700000e+00 }, // SINR
          { 1, 6.026995e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 2.400000e+00, 3.100000e+00, 3.800000e+00, 4.600000e+00 }, // SINR
          { 1, 7.810606e-01, 5.600000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 2.970000e+00, 3.200000e+00, 3.430000e+00, 3.900000e+00, 4.600000e+00 }, // SINR
          { 1, 7.919207e-01, 6.021127e-01, 4.114078e-01, 9.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 2.970000e+00, 3.200000e+00, 3.430000e+00, 3.900000e+00, 4.600000e+00 }, // SINR
          { 1, 8.673986e-01, 6.021127e-01, 8.950000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 2.331300e+00, 2.831300e+00, 2.894800e+00, 2.958400e+00, 3.021900e+00, 3.085400e+00, 3.585400e+00, 4.085400e+00 }, // SINR
          { 1, 8.054687e-01, 7.732036e-01, 7.335714e-01, 6.970109e-01, 6.505102e-01, 5.860000e-02, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 2.349000e+00, 2.849000e+00, 2.896300e+00, 2.943600e+00, 2.990900e+00, 3.038100e+00, 3.538100e+00, 4.038100e+00 }, // SINR
          { 1, 8.610197e-01, 8.457792e-01, 8.282258e-01, 8.078125e-01, 7.835366e-01, 7.440000e-02, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 2.492800e+00, 3.148400e+00, 3.804100e+00, 4.459700e+00 }, // SINR
          { 9.884615e-01, 4.990196e-01, 1.350000e-02, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 2.400000e+00, 2.870000e+00, 3.100000e+00, 3.330000e+00, 3.800000e+00, 4.600000e+00 }, // SINR
          { 1, 9.673507e-01, 6.617268e-01, 2.973529e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 2.444300e+00, 3.086200e+00, 3.728100e+00, 4.370000e+00 }, // SINR
          { 9.903846e-01, 5.810502e-01, 1.470000e-02, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 2.700000e+00, 3.200000e+00, 3.700000e+00, 4.200000e+00 }, // SINR
          { 9.980769e-01, 5.495690e-01, 7.100000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 2.480500e+00, 3.005900e+00, 3.531300e+00, 4.056700e+00, 4.582100e+00 }, // SINR
          { 9.923077e-01, 7.274011e-01, 9.588353e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 3, 3.500000e+00, 4, 4.500000e+00 }, // SINR
          { 1, 9.001736e-01, 2.432171e-01, 3.500000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.250000e+00, 2.750000e+00, 3.250000e+00, 3.487900e+00, 3.725800e+00, 3.963600e+00, 4.201500e+00 }, // SINR
          { 1, 8.767241e-01, 3.118842e-01, 1.296488e-01, 3.950000e-02, 7.700000e-03, 8.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.349300e+00, 2.864500e+00, 3.379800e+00, 3.895100e+00, 4.410300e+00 }, // SINR
          { 9.990385e-01, 9.402574e-01, 3.182957e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.763000e+00, 3.276100e+00, 3.789100e+00, 4.302100e+00 }, // SINR
          { 9.753788e-01, 3.931327e-01, 6.500000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 2.683800e+00, 3.187100e+00, 3.690500e+00, 4.193800e+00 }, // SINR
          { 9.951923e-01, 6.025943e-01, 2.870000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 2.816700e+00, 3.239700e+00, 3.662600e+00, 4.085600e+00 }, // SINR
          { 9.817308e-01, 5.452128e-01, 3.090000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 2.755100e+00, 3.260300e+00, 3.765400e+00, 4.270600e+00 }, // SINR
          { 9.942308e-01, 5.214286e-01, 9.200000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 2.700900e+00, 3.186500e+00, 3.672100e+00, 4.157700e+00 }, // SINR
          { 1, 6.237805e-01, 2.200000e-02, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 2.536900e+00, 2.885600e+00, 3.234400e+00, 3.583100e+00, 3.931800e+00 }, // SINR
          { 9.642857e-01, 6.854839e-01, 2.186411e-01, 2.430000e-02, 7.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.825000e+00, 3.250000e+00, 3.675000e+00, 4.100000e+00 }, // SINR
          { 9.751908e-01, 5.029880e-01, 3.290000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.600000e+00, 2.975000e+00, 3.350000e+00, 3.725000e+00, 4.100000e+00 }, // SINR
          { 1, 9.055944e-01, 3.433243e-01, 1.570000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.700000e+00, 3.025000e+00, 3.350000e+00, 3.675000e+00, 4 }, // SINR
          { 9.961538e-01, 7.906442e-01, 1.835029e-01, 3.400000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 2.700000e+00, 3.025000e+00, 3.350000e+00, 3.675000e+00, 4 }, // SINR
          { 1, 8.682886e-01, 3.073171e-01, 2.210000e-02, 2.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.300000e+00, 2.800000e+00, 3.300000e+00, 3.800000e+00, 4.300000e+00, 4.800000e+00 }, // SINR
          { 9.903846e-01, 7.529412e-01, 3.361037e-01, 4.540000e-02, 2.200000e-03, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 2.798500e+00, 3.046100e+00, 3.293800e+00, 3.541500e+00, 3.789100e+00, 4.289100e+00 }, // SINR
          { 9.884615e-01, 8.886986e-01, 5.258264e-01, 1.362527e-01, 1.090000e-02, 7.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 2.399800e+00, 2.899800e+00, 3.399800e+00, 3.574700e+00, 3.749600e+00, 3.924400e+00, 4.099300e+00 }, // SINR
          { 1, 7.085635e-01, 3.091299e-01, 8.600000e-02, 1.280000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 2.848100e+00, 3.035600e+00, 3.223000e+00, 3.410400e+00, 3.910400e+00, 4.410400e+00 }, // SINR
          { 9.875000e-01, 9.187500e-01, 6.957418e-01, 3.355438e-01, 3.140000e-02, 9.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 2.315000e+00, 2.815000e+00, 3.315000e+00, 3.694700e+00, 4.074400e+00 }, // SINR
          { 1, 8.066038e-01, 4.122951e-01, 1.600000e-02, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 2.729900e+00, 3.085800e+00, 3.441700e+00, 3.797600e+00, 4.153500e+00 }, // SINR
          { 9.932692e-01, 7.604167e-01, 1.045047e-01, 9.000000e-04, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 2.830800e+00, 2.974700e+00, 3.118500e+00, 3.262400e+00, 3.406200e+00, 3.906200e+00, 4.406200e+00 }, // SINR
          { 9.675573e-01, 8.640940e-01, 6.274510e-01, 3.277202e-01, 1.112222e-01, 2.100000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 7
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 3.747700e+00, 4.084100e+00, 4.420500e+00, 4.756900e+00 }, // SINR
          { 9.075704e-01, 3.344327e-01, 5.320000e-02, 1.000000e-04 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 3.479200e+00, 4.215600e+00, 4.583700e+00, 4.951900e+00 }, // SINR
          { 9.990385e-01, 1.742424e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 3.300000e+00, 3.550000e+00, 3.800000e+00, 4, 4.300000e+00, 4.500000e+00, 4.800000e+00, 5.050000e+00, 5.300000e+00, 5.550000e+00 }, // SINR
          { 9.696970e-01, 8.470395e-01, 8.031250e-01, 6.213235e-01, 3.209288e-01, 1.386589e-01, 2.950000e-02, 2.700000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 2.880000e+00, 3.300000e+00, 3.720000e+00, 4.150000e+00, 4.580000e+00, 5, 5.430000e+00, 5.850000e+00, 6.280000e+00, 6.700000e+00 }, // SINR
          { 9.687500e-01, 8.810345e-01, 6.912568e-01, 4.222408e-01, 1.804957e-01, 5.440000e-02, 1.040000e-02, 1.200000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 3.620500e+00, 3.899300e+00, 4.178000e+00, 4.456800e+00, 4.735600e+00 }, // SINR
          { 9.761450e-01, 6.107820e-01, 7.500000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 3.480300e+00, 3.980300e+00, 4.163600e+00, 4.346900e+00, 4.530200e+00, 4.713500e+00 }, // SINR
          { 9.866412e-01, 4.083601e-01, 9.045125e-02, 7.300000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 3.639900e+00, 3.905500e+00, 4.171200e+00, 4.436800e+00, 4.702500e+00 }, // SINR
          { 9.932692e-01, 7.463450e-01, 1.689597e-01, 4.000000e-03, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 3.277100e+00, 3.866400e+00, 4.161100e+00, 4.455800e+00, 5.045100e+00 }, // SINR
          { 1, 6.510152e-01, 1.865727e-01, 2.910000e-02, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 3.260000e+00, 3.788600e+00, 4.052900e+00, 4.317200e+00, 4.845800e+00 }, // SINR
          { 1, 7.702096e-01, 1.253750e-01, 5.210000e-02, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 3.242200e+00, 3.769100e+00, 4.032600e+00, 4.296100e+00, 4.823000e+00 }, // SINR
          { 1, 7.858232e-01, 2.522455e-01, 5.390000e-02, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 3.575100e+00, 4.041900e+00, 4.275300e+00, 4.508700e+00, 4.975500e+00 }, // SINR
          { 9.980769e-01, 3.408177e-01, 3.470000e-02, 3.600000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 3.673100e+00, 3.905700e+00, 4.138300e+00, 4.603500e+00 }, // SINR
          { 9.847328e-01, 4.438596e-01, 4.136808e-01, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 3.538400e+00, 3.944100e+00, 4.146900e+00, 4.349700e+00, 4.755400e+00 }, // SINR
          { 9.971154e-01, 5.538043e-01, 1.392738e-01, 2.220000e-02, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 3.173000e+00, 3.779500e+00, 4.183900e+00, 4.386100e+00, 4.588200e+00 }, // SINR
          { 1, 9.143357e-01, 6.010000e-02, 3.600000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 3.501200e+00, 3.904200e+00, 4.105600e+00, 4.307100e+00, 4.710100e+00 }, // SINR
          { 1, 6.942935e-01, 1.705007e-01, 3.370000e-02, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 3.300000e+00, 3.900000e+00, 4.500000e+00, 5.100000e+00, 5.700000e+00 }, // SINR
          { 9.406475e-01, 4.011905e-01, 2.130000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 3.807700e+00, 4.089300e+00, 4.230000e+00, 4.370800e+00, 4.652400e+00 }, // SINR
          { 9.507299e-01, 2.778698e-01, 1.079396e-01, 1.660000e-02, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 3.300000e+00, 3.800000e+00, 4, 4.200000e+00, 4.400000e+00, 4.600000e+00 }, // SINR
          { 9.701923e-01, 8.715986e-01, 4.816038e-01, 8.919952e-02, 5.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 3.745700e+00, 4.020600e+00, 4.158000e+00, 4.295400e+00, 4.570300e+00 }, // SINR
          { 9.836538e-01, 3.075980e-01, 1.800574e-01, 1.700000e-02, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 3.814100e+00, 4.069000e+00, 4.196500e+00, 4.324000e+00, 4.578900e+00 }, // SINR
          { 9.574074e-01, 2.397619e-01, 1.035592e-01, 1.010000e-02, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 3.180000e+00, 3.701500e+00, 4.224300e+00, 4.747100e+00, 5.270000e+00, 5.792800e+00 }, // SINR
          { 9.734848e-01, 7.042350e-01, 2.990544e-01, 4.530000e-02, 2.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 3.202900e+00, 3.837600e+00, 4.472300e+00, 5.107000e+00, 5.741700e+00 }, // SINR
          { 9.846154e-01, 7.230114e-01, 1.778169e-01, 6.800000e-03, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 3.267100e+00, 3.873000e+00, 4.478900e+00, 5.084800e+00, 5.690700e+00 }, // SINR
          { 9.740385e-01, 6.845238e-01, 1.542488e-01, 6.100000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 3.140000e+00, 3.644300e+00, 4.061700e+00, 4.479200e+00, 4.896600e+00, 5.314100e+00 }, // SINR
          { 9.564815e-01, 7.981366e-01, 4.085761e-01, 9.362307e-02, 7.700000e-03, 3.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 3.071600e+00, 3.688100e+00, 4.304500e+00, 4.921000e+00, 5.537500e+00 }, // SINR
          { 9.980769e-01, 8.784014e-01, 3.150000e-01, 1.370000e-02, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 3.219800e+00, 3.773700e+00, 4.327600e+00, 4.881500e+00, 5.435400e+00 }, // SINR
          { 9.630682e-01, 5.475427e-01, 6.620000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 808U, // SINR and BLER for CBS 808
        NrEesmErrorModel::DoubleTuple{
          { 3.250000e+00, 3.770800e+00, 4.291700e+00, 4.812500e+00, 5.333300e+00 }, // SINR
          { 9.799618e-01, 6.573834e-01, 1.117832e-01, 2.300000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 3.004000e+00, 3.637500e+00, 4.271000e+00, 4.904400e+00, 5.537900e+00 }, // SINR
          { 1, 8.601974e-01, 1.479953e-01, 9.000000e-04, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 2.900000e+00, 3.400000e+00, 3.900000e+00, 4.400000e+00, 4.900000e+00 }, // SINR
          { 9.990385e-01, 8.994755e-01, 2.927326e-01, 7.400000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 3.020000e+00, 3.543500e+00, 4.063100e+00, 4.582700e+00, 5.102200e+00 }, // SINR
          { 9.961538e-01, 7.848485e-01, 1.272819e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 3.500000e+00, 4, 4.500000e+00, 5 }, // SINR
          { 9.031690e-01, 3.140547e-01, 1.290000e-02, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 3.246500e+00, 3.784600e+00, 4.322600e+00, 4.860700e+00, 5.398800e+00 }, // SINR
          { 9.846154e-01, 6.055425e-01, 5.310000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 3.339700e+00, 3.673800e+00, 4.007800e+00, 4.341900e+00, 4.675900e+00, 5.175900e+00 }, // SINR
          { 9.152098e-01, 7.288136e-01, 4.717593e-01, 2.147436e-01, 6.230000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 3.144900e+00, 3.644900e+00, 3.914300e+00, 4.183600e+00, 4.452900e+00, 4.722300e+00, 5.222300e+00 }, // SINR
          { 1, 7.341954e-01, 4.861641e-01, 2.480392e-01, 9.332123e-02, 2.290000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 3.450000e+00, 3.775000e+00, 4.100000e+00, 4.425000e+00, 4.750000e+00, 5.250000e+00 }, // SINR
          { 9.058099e-01, 6.891711e-01, 3.724265e-01, 1.392423e-01, 3.100000e-02, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 3.005800e+00, 3.505800e+00, 3.756400e+00, 4.007000e+00, 4.257700e+00, 4.508300e+00, 5.008300e+00 }, // SINR
          { 1, 8.716443e-01, 7.097222e-01, 4.592391e-01, 2.402567e-01, 9.304662e-02, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 3.042800e+00, 3.542800e+00, 3.772000e+00, 4.001200e+00, 4.230500e+00, 4.459700e+00, 4.959700e+00 }, // SINR
          { 1, 8.181090e-01, 6.243902e-01, 3.806306e-01, 1.826923e-01, 6.650000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 3.300000e+00, 3.925000e+00, 4.550000e+00, 5.175000e+00, 5.800000e+00 }, // SINR
          { 9.527778e-01, 5.319038e-01, 5.000000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 3.060800e+00, 3.560800e+00, 3.838000e+00, 4.115100e+00, 4.392300e+00, 4.669500e+00, 5.169500e+00 }, // SINR
          { 1, 8.792808e-01, 6.318069e-01, 3.175251e-01, 1.028668e-01, 1.910000e-02, 0 } // BLER
        }
      },
      { 2472U, // SINR and BLER for CBS 2472
        NrEesmErrorModel::DoubleTuple{
          { 3.500000e+00, 3.875000e+00, 4.250000e+00, 4.625000e+00 }, // SINR
          { 9.971154e-01, 6.655928e-01, 3.120000e-02, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 3.058300e+00, 3.877100e+00, 4.695900e+00, 5.514600e+00 }, // SINR
          { 9.971154e-01, 5.839041e-01, 9.000000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 3.800000e+00, 4.050000e+00, 4.300000e+00, 4.550000e+00, 4.800000e+00 }, // SINR
          { 9.298561e-01, 4.899038e-01, 6.260000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 3.100000e+00, 3.772200e+00, 4.108300e+00, 4.444400e+00, 5.116600e+00 }, // SINR
          { 9.751908e-01, 8.589527e-01, 1.661162e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 3.260000e+00, 3.480000e+00, 3.700000e+00, 3.930000e+00, 4.150000e+00, 4.380000e+00, 4.600000e+00, 4.820000e+00, 5.050000e+00 }, // SINR
          { 9.633459e-01, 8.854895e-01, 7.349138e-01, 4.766791e-01, 2.162220e-01, 6.800000e-02, 1.520000e-02, 1.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 8
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 4.398200e+00, 4.752800e+00, 5.107400e+00, 5.462000e+00, 5.816600e+00, 6.171200e+00, 6.525800e+00, 6.880400e+00 }, // SINR
          { 9.942308e-01, 6.949728e-01, 3.968750e-01, 9.755592e-02, 2.610000e-02, 4.800000e-03, 6.000000e-04, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 4.379600e+00, 4.733100e+00, 5.086600e+00, 5.440100e+00, 5.793600e+00, 6.147100e+00, 6.500600e+00 }, // SINR
          { 9.903846e-01, 6.838235e-01, 2.619048e-01, 1.209016e-01, 1.100000e-02, 8.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 4.370600e+00, 4.723500e+00, 5.076400e+00, 5.429300e+00, 5.782200e+00, 6.135100e+00, 6.488000e+00 }, // SINR
          { 9.990385e-01, 6.677632e-01, 3.286554e-01, 1.213768e-01, 1.150000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 4.361600e+00, 4.713900e+00, 5.066200e+00, 5.418500e+00, 5.770800e+00, 6.123100e+00, 6.475400e+00 }, // SINR
          { 1, 7.191011e-01, 3.517361e-01, 7.070000e-02, 1.100000e-02, 1.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 4.343000e+00, 4.694200e+00, 5.045400e+00, 5.396600e+00, 5.747800e+00, 6.099000e+00 }, // SINR
          { 1, 8.965517e-01, 2.552817e-01, 8.450000e-02, 6.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 4.075000e+00, 4.641700e+00, 4.925000e+00, 5.208300e+00, 5.775000e+00, 6.625000e+00 }, // SINR
          { 9.375000e-01, 4.675926e-01, 1.975552e-01, 5.670000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 3.958200e+00, 4.307000e+00, 4.655800e+00, 5.004600e+00, 5.353400e+00, 5.702200e+00, 6.051000e+00 }, // SINR
          { 1, 7.825758e-01, 6.610825e-01, 2.509980e-01, 2.050000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 3.940700e+00, 4.288400e+00, 4.636100e+00, 4.983800e+00, 5.331500e+00, 5.679200e+00, 6.026900e+00, 6.374600e+00 }, // SINR
          { 1, 8.485099e-01, 5.803167e-01, 2.955607e-01, 3.960000e-02, 1.900000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 3.923900e+00, 4.270400e+00, 4.616900e+00, 4.963400e+00, 5.309900e+00, 5.656400e+00, 6.002900e+00 }, // SINR
          { 1, 7.787879e-01, 6.565722e-01, 1.727586e-01, 5.300000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 4, 4.500000e+00, 5, 5.375000e+00, 5.750000e+00, 6.125000e+00, 6.500000e+00 }, // SINR
          { 9.586466e-01, 6.653646e-01, 3.724340e-01, 8.910000e-02, 1.060000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 3.889600e+00, 4.233800e+00, 4.578000e+00, 4.922200e+00, 5.266400e+00, 5.610600e+00 }, // SINR
          { 1, 8.959790e-01, 6.918919e-01, 1.291237e-01, 1.800000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 4.200000e+00, 4.550000e+00, 4.900000e+00, 5.200000e+00, 5.600000e+00 }, // SINR
          { 9.923077e-01, 7.996894e-01, 5.546537e-01, 2.870000e-02, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 4.718750e+00, 5.882700e+00, 6.223500e+00, 6.564200e+00 }, // SINR
          { 9.000000e-01, 6.600000e-03, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 4.200000e+00, 4.450000e+00, 4.700000e+00, 4.950000e+00, 5.200000e+00, 5.400000e+00, 5.700000e+00 }, // SINR
          { 9.980769e-01, 8.455882e-01, 4.500000e-01, 1.136570e-01, 1.170000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.700000e+00, 5, 5.300000e+00, 5.600000e+00 }, // SINR
          { 9.522059e-01, 6.311881e-01, 9.742351e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 4.287657e+00, 5.738500e+00, 6.072300e+00 }, // SINR
          { 9.000000e-01, 1.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.800000e+00, 5.200000e+00, 5.600000e+00 }, // SINR
          { 9.980769e-01, 4.946911e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 4.500436e+00, 5.642300e+00, 5.971500e+00 }, // SINR
          { 9.000000e-01, 3.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 4.506242e+00, 5.594200e+00, 5.921200e+00 }, // SINR
          { 9.000000e-01, 3.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 4.389550e+00, 5.546200e+00, 5.870800e+00 }, // SINR
          { 9.000000e-01, 1.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.650000e+00, 4.900000e+00, 5.150000e+00, 5.400000e+00, 5.650000e+00 }, // SINR
          { 9.761450e-01, 8.046875e-01, 3.580028e-01, 5.550000e-02, 2.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 4.407063e+00, 5.353800e+00, 5.669200e+00 }, // SINR
          { 9.000000e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 4.546200e+00, 5.257700e+00, 5.568500e+00 }, // SINR
          { 9.000000e-01, 1.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 4.356250e+00, 5.161500e+00, 5.467700e+00 }, // SINR
          { 9.000000e-01, 5.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 4.478800e+00, 5.065400e+00, 5.366900e+00, 5.668500e+00, 5.970000e+00 }, // SINR
          { 9.000000e-01, 1.782670e-01, 2.320000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 4.478800e+00, 4.969200e+00, 5.266200e+00, 5.563100e+00, 5.860000e+00 }, // SINR
          { 9.000000e-01, 2.164372e-01, 3.090000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 4.430700e+00, 4.873100e+00, 5.165400e+00, 5.457700e+00, 5.750000e+00 }, // SINR
          { 9.000000e-01, 2.500000e-01, 3.250000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 4.201500e+00, 4.489200e+00, 4.776900e+00, 5.064600e+00, 5.352300e+00, 5.640000e+00 }, // SINR
          { 9.807692e-01, 5.105000e-01, 4.390138e-01, 9.560016e-02, 7.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 4.306100e+00, 4.584600e+00, 4.863100e+00, 5.141500e+00, 5.420000e+00, 5.698500e+00 }, // SINR
          { 9.425182e-01, 6.457286e-01, 2.223940e-01, 2.270000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 4.392300e+00, 4.537700e+00, 4.661500e+00, 4.930800e+00, 5.200000e+00, 5.469200e+00 }, // SINR
          { 9.080357e-01, 9.000000e-01, 5.252058e-01, 1.311198e-01, 1.170000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 4.200000e+00, 4.460000e+00, 4.598000e+00, 4.720000e+00, 4.980000e+00, 5.240000e+00 }, // SINR
          { 9.855769e-01, 8.566667e-01, 4.277872e-01, 7.210000e-02, 3.900000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 4.258500e+00, 4.509200e+00, 4.716400e+00, 4.760000e+00, 5.010800e+00, 5.261500e+00 }, // SINR
          { 9.671053e-01, 7.647929e-01, 3.288961e-01, 4.610000e-02, 2.400000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 4.300000e+00, 4.500000e+00, 4.800000e+00, 5, 5.300000e+00, 5.550000e+00, 5.800000e+00 }, // SINR
          { 9.990385e-01, 9.817308e-01, 7.090278e-01, 3.579060e-01, 3.350000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 4.087700e+00, 4.320000e+00, 4.552300e+00, 4.700631e+00, 4.911900e+00, 5.123200e+00, 5.334500e+00 }, // SINR
          { 9.980769e-01, 9.498175e-01, 6.477273e-01, 2.081271e-01, 5.850000e-02, 2.600000e-03, 9.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 4.323100e+00, 4.546200e+00, 4.682569e+00, 4.747600e+00, 4.921700e+00, 4.949000e+00, 5.095900e+00 }, // SINR
          { 9.642857e-01, 6.967213e-01, 3.731618e-01, 1.999601e-01, 1.065329e-01, 4.160000e-02, 2.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 4.307700e+00, 4.674650e+00, 4.939500e+00, 5.204400e+00, 5.469300e+00 }, // SINR
          { 9.642857e-01, 9.000000e-01, 2.666139e-01, 3.020000e-02, 3.000000e-04 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 3.830800e+00, 4.694463e+00, 4.694500e+00, 5.558100e+00 }, // SINR
          { 1, 9.000000e-01, 6.400000e-01, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 3.353800e+00, 4.694486e+00, 4.694500e+00, 6.035200e+00 }, // SINR
          { 1, 9.000000e-01, 4.870690e-01, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 4.669762e+00, 4.669800e+00, 6.462600e+00 }, // SINR
          { 9.000000e-01, 3.148263e-01, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 4.663453e+00, 4.663500e+00, 6.926900e+00 }, // SINR
          { 9.000000e-01, 4.330205e-01, 0 } // BLER
        }
      }
  },
  { // MCS 9
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 4.600000e+00, 5.270000e+00, 5.600000e+00, 5.930000e+00, 6.600000e+00 }, // SINR
          { 1, 7.376453e-01, 6.387500e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 4.800000e+00, 5.400000e+00, 5.700000e+00, 6, 6.600000e+00 }, // SINR
          { 1, 4.676095e-01, 4.040605e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 4.986800e+00, 5.486800e+00, 5.986800e+00, 6.486800e+00, 6.986800e+00 }, // SINR
          { 9.570896e-01, 6.759259e-01, 1.195446e-01, 1.060000e-02, 3.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 4.700000e+00, 5.300000e+00, 5.600000e+00, 5.900000e+00, 6.500000e+00 }, // SINR
          { 1, 9.732824e-01, 2.286751e-01, 9.347301e-02, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 4.700000e+00, 5.200000e+00, 5.600000e+00, 6, 6.500000e+00, 6.950000e+00, 7.400000e+00, 7.850000e+00, 8.300000e+00 }, // SINR
          { 9.246454e-01, 7.191011e-01, 4.747191e-01, 2.245124e-01, 5.640000e-02, 8.700000e-03, 6.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 4.707200e+00, 5.056000e+00, 5.404800e+00, 5.753600e+00, 6.102400e+00, 6.451200e+00, 6.800000e+00 }, // SINR
          { 1, 7.840909e-01, 5.603070e-01, 2.451456e-01, 7.700000e-03, 9.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 4.690700e+00, 5.038300e+00, 5.385900e+00, 5.733500e+00, 6.081100e+00, 6.428700e+00 }, // SINR
          { 9.699248e-01, 8.415033e-01, 6.159420e-01, 2.584529e-01, 1.510000e-02, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 4.673600e+00, 5.020100e+00, 5.366600e+00, 5.713100e+00, 6.059600e+00, 6.406100e+00, 6.752600e+00 }, // SINR
          { 1, 8.827586e-01, 6.675393e-01, 2.905172e-01, 5.350000e-02, 2.800000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 5.002300e+00, 5.347600e+00, 5.692900e+00, 6.038200e+00, 6.383500e+00, 6.728800e+00 }, // SINR
          { 9.583333e-01, 8.657718e-01, 2.485236e-01, 8.353414e-02, 2.700000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 4.639900e+00, 4.984100e+00, 5.328300e+00, 5.672500e+00, 6.016700e+00, 6.360900e+00, 6.705100e+00 }, // SINR
          { 9.645522e-01, 8.741611e-01, 7.721037e-01, 6.021127e-01, 5.710000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 4.700000e+00, 5.150000e+00, 5.600000e+00, 6, 6.500000e+00 }, // SINR
          { 9.558824e-01, 8.897569e-01, 6.393035e-01, 3.040000e-02, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 4.800000e+00, 5.300000e+00, 5.900000e+00, 6.400000e+00, 7, 7.550000e+00, 8.100000e+00 }, // SINR
          { 9.005282e-01, 6.642670e-01, 2.807650e-01, 7.600000e-02, 7.200000e-03, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 4.700000e+00, 5.300000e+00, 5.800000e+00, 6.400000e+00, 6.900000e+00, 7.450000e+00, 8, 8.550000e+00 }, // SINR
          { 9.246454e-01, 6.642670e-01, 3.317942e-01, 7.600000e-02, 1.230000e-02, 5.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 5.213200e+00, 5.550600e+00, 5.888000e+00, 6.225400e+00, 6.562800e+00 }, // SINR
          { 9.817308e-01, 7.470930e-01, 7.050000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 5.175000e+00, 5.510100e+00, 5.845200e+00, 6.180300e+00, 6.515400e+00, 6.850500e+00, 7.185600e+00, 7.520700e+00 }, // SINR
          { 9.809160e-01, 7.695783e-01, 1.961778e-01, 4.330000e-02, 1.510000e-02, 1.700000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 4.600000e+00, 5.100000e+00, 5.600000e+00, 6.100000e+00, 6.600000e+00, 7.100000e+00, 7.600000e+00, 8.100000e+00 }, // SINR
          { 9.474638e-01, 7.822086e-01, 4.747191e-01, 1.790300e-01, 3.970000e-02, 4.000000e-03, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 4.680000e+00, 5.100000e+00, 5.500000e+00, 5.900000e+00, 6.300000e+00, 6.800000e+00, 7.230000e+00, 7.650000e+00, 8.080000e+00, 8.500000e+00 }, // SINR
          { 9.273050e-01, 7.822086e-01, 5.390295e-01, 2.807650e-01, 1.020259e-01, 1.780000e-02, 2.200000e-03, 1.000000e-04, 1.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 4.380000e+00, 4.900000e+00, 5.400000e+00, 5.900000e+00, 6.500000e+00, 7, 7.530000e+00, 8.050000e+00 }, // SINR
          { 9.721154e-01, 8.640940e-01, 6.042654e-01, 2.807650e-01, 5.640000e-02, 7.200000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 4.850000e+00, 5.300000e+00, 5.800000e+00, 6.200000e+00 }, // SINR
          { 1, 7.245763e-01, 5.200000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 4.480000e+00, 4.900000e+00, 5.300000e+00, 5.700000e+00, 6.200000e+00, 6.600000e+00, 7.020000e+00, 7.450000e+00, 7.870000e+00, 8.300000e+00 }, // SINR
          { 9.640152e-01, 8.640940e-01, 6.642670e-01, 4.022082e-01, 1.402405e-01, 3.970000e-02, 6.400000e-03, 5.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 5.154700e+00, 6.183300e+00, 6.502500e+00, 6.821700e+00 }, // SINR
          { 9.000000e-01, 4.400000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 5, 6.088500e+00, 6.403100e+00, 6.717700e+00 }, // SINR
          { 9.000000e-01, 1.160000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 4.800000e+00, 5.100000e+00, 5.500000e+00, 5.900000e+00, 6.300000e+00, 6.680000e+00, 7.050000e+00, 7.430000e+00, 7.800000e+00, 8.180000e+00 }, // SINR
          { 9.005282e-01, 7.822086e-01, 5.390295e-01, 2.807650e-01, 1.020259e-01, 2.930000e-02, 5.600000e-03, 6.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 5.110300e+00, 5.898700e+00, 6.204200e+00, 6.509700e+00 }, // SINR
          { 9.000000e-01, 2.560000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 5.110300e+00, 5.803800e+00, 6.104800e+00, 6.405800e+00 }, // SINR
          { 9.000000e-01, 4.490000e-02, 2.200000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 4.819800e+00, 5.116200e+00, 5.412600e+00, 5.709000e+00, 6.005400e+00, 6.301800e+00, 6.598200e+00 }, // SINR
          { 1, 8.349359e-01, 7.272727e-01, 1.668883e-01, 1.820000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 4.875000e+00, 5.614100e+00, 5.906000e+00, 6.197800e+00, 6.489700e+00, 6.781500e+00 }, // SINR
          { 9.000000e-01, 5.484914e-01, 1.918960e-01, 3.270000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 4.657300e+00, 4.944600e+00, 5.231900e+00, 5.519200e+00, 5.806500e+00, 6.093800e+00, 6.381200e+00, 6.668500e+00 }, // SINR
          { 1, 8.614865e-01, 7.529412e-01, 6.560914e-01, 2.655263e-01, 4.930000e-02, 2.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 5, 5.330000e+00, 5.500000e+00, 5.670000e+00, 6, 6.400000e+00 }, // SINR
          { 1, 6.610825e-01, 5.220287e-01, 4.180000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 4.870600e+00, 5.139700e+00, 5.408800e+00, 5.677900e+00, 5.947100e+00, 6.216200e+00, 6.485300e+00 }, // SINR
          { 9.980769e-01, 8.853448e-01, 5.084661e-01, 1.281823e-01, 9.800000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 4.950000e+00, 5.210000e+00, 5.470000e+00, 5.730000e+00, 5.990000e+00, 6.250000e+00 }, // SINR
          { 9.836538e-01, 8.251582e-01, 3.883384e-01, 7.840000e-02, 5.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 5, 5.400000e+00, 5.600000e+00, 5.800000e+00, 6.200000e+00 }, // SINR
          { 9.942308e-01, 7.700893e-01, 1.666667e-01, 3.010000e-02, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 4.800400e+00, 5.300400e+00, 5.800400e+00, 6.300400e+00, 6.800400e+00 }, // SINR
          { 9.855769e-01, 7.751524e-01, 3.095966e-01, 2.830000e-02, 8.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 5.078800e+00, 5.311500e+00, 5.327850e+00, 5.506400e+00, 5.685000e+00, 5.863600e+00, 6.042200e+00, 6.220800e+00 }, // SINR
          { 9.509259e-01, 6.838235e-01, 5.661111e-01, 2.485294e-01, 1.323684e-01, 1.070000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 5.100000e+00, 5.500000e+00, 6, 6.400000e+00 }, // SINR
          { 9.961538e-01, 5.346639e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 5.200000e+00, 5.600000e+00, 5.900000e+00, 6.300000e+00 }, // SINR
          { 9.592593e-01, 1.848385e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 4.603200e+00, 5.300800e+00, 5.998400e+00, 6.696000e+00, 7.393600e+00 }, // SINR
          { 1, 9.000000e-01, 1.757650e-01, 1.260000e-02, 5.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 4.800000e+00, 5.400000e+00, 6, 6.600000e+00 }, // SINR
          { 1, 8.632550e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 3240U, // SINR and BLER for CBS 3240
        NrEesmErrorModel::DoubleTuple{
          { 4.900000e+00, 5.300000e+00, 5.570000e+00, 5.700000e+00, 5.830000e+00, 6.100000e+00 }, // SINR
          { 1, 9.157801e-01, 2.870159e-01, 1.230411e-01, 1.750000e-02, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 4.700000e+00, 5.230000e+00, 5.500000e+00, 5.770000e+00, 6.300000e+00 }, // SINR
          { 1, 9.330357e-01, 7.873476e-01, 2.130000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 10
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 5.400000e+00, 5.900000e+00, 6.075000e+00, 6.250000e+00, 6.425000e+00, 6.600000e+00, 7.100000e+00, 7.600000e+00 }, // SINR
          { 9.855769e-01, 7.627246e-01, 5.778409e-01, 3.527159e-01, 1.824275e-01, 7.170000e-02, 2.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 4.900000e+00, 5.400000e+00, 5.900000e+00, 6.400000e+00, 6.900000e+00, 7.400000e+00, 7.900000e+00, 8.400000e+00 }, // SINR
          { 9.817308e-01, 8.741497e-01, 6.379950e-01, 3.101966e-01, 8.020000e-02, 1.150000e-02, 1.400000e-03, 2.000000e-04 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 5.400000e+00, 5.900000e+00, 6.225000e+00, 6.550000e+00, 6.875000e+00, 7.200000e+00 }, // SINR
          { 9.790076e-01, 5.557359e-01, 1.677236e-01, 2.130000e-02, 1.800000e-03, 3.000000e-04 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 5.500000e+00, 6, 6.200000e+00, 6.400000e+00, 6.600000e+00, 6.800000e+00, 7.300000e+00 }, // SINR
          { 9.267857e-01, 4.876894e-01, 2.320772e-01, 7.520000e-02, 1.630000e-02, 2.400000e-03, 2.000000e-04 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 5.301600e+00, 6.051600e+00, 6.801600e+00, 7.551600e+00 }, // SINR
          { 1, 6.925676e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 5.200000e+00, 5.925000e+00, 6.650000e+00, 7.375000e+00 }, // SINR
          { 1, 8.347039e-01, 3.200000e-03, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 5.950000e+00, 6.200000e+00, 6.450000e+00, 6.700000e+00 }, // SINR
          { 9.932692e-01, 6.818783e-01, 5.000000e-02, 2.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 5.776700e+00, 6.050600e+00, 6.324600e+00, 6.598500e+00, 6.872400e+00 }, // SINR
          { 9.809160e-01, 7.097222e-01, 1.589331e-01, 8.300000e-03, 3.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 5.805400e+00, 6.066800e+00, 6.328300e+00, 6.589800e+00, 6.851200e+00 }, // SINR
          { 9.289568e-01, 4.761236e-01, 5.510000e-02, 1.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 5.787500e+00, 6.048100e+00, 6.308700e+00, 6.569400e+00, 6.830000e+00 }, // SINR
          { 9.604779e-01, 6.088517e-01, 9.031676e-02, 2.500000e-03, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 5.723600e+00, 5.994900e+00, 6.266200e+00, 6.537500e+00, 6.808800e+00 }, // SINR
          { 9.723282e-01, 6.165049e-01, 7.060000e-02, 4.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 5.478200e+00, 5.984200e+00, 6.237100e+00, 6.490100e+00, 6.996100e+00 }, // SINR
          { 1, 5.726351e-01, 3.442623e-01, 1.825945e-01, 1.000000e-04 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 5.950000e+00, 6.200000e+00, 6.450000e+00, 6.700000e+00 }, // SINR
          { 1, 8.374183e-01, 1.114106e-01, 8.000000e-04 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.150000e+00, 6.400000e+00, 6.900000e+00 }, // SINR
          { 9.990385e-01, 8.488562e-01, 1.276731e-01, 1.000000e-04 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 5.286800e+00, 5.786800e+00, 6.286800e+00, 6.652600e+00, 7.018300e+00 }, // SINR
          { 1, 7.004076e-01, 2.968384e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 5.286800e+00, 5.786800e+00, 6.286800e+00, 6.646400e+00, 7.006100e+00 }, // SINR
          { 1, 8.233871e-01, 3.724265e-01, 2.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 5.213200e+00, 5.713200e+00, 6.213200e+00, 6.579000e+00, 6.944700e+00 }, // SINR
          { 1, 7.342857e-01, 4.230769e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 5.221400e+00, 5.721400e+00, 6.221400e+00, 6.572900e+00, 6.924400e+00 }, // SINR
          { 1, 8.894558e-01, 4.407439e-01, 2.600000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 5.772800e+00, 6.174000e+00, 6.374600e+00, 6.575300e+00, 6.976500e+00 }, // SINR
          { 9.980769e-01, 2.112647e-01, 9.184696e-02, 2.400000e-02, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 5.660800e+00, 6.127900e+00, 6.595000e+00, 7.062100e+00 }, // SINR
          { 1, 8.344156e-01, 3.100000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 5.616700e+00, 6.030800e+00, 6.237900e+00, 6.444900e+00, 6.859000e+00 }, // SINR
          { 1, 6.100478e-01, 4.985352e-01, 1.465367e-01, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 5.625000e+00, 6.076600e+00, 6.528100e+00, 6.979600e+00 }, // SINR
          { 1, 8.258065e-01, 4.600000e-03, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 5.662400e+00, 6.092400e+00, 6.522300e+00, 6.952200e+00 }, // SINR
          { 1, 8.322581e-01, 8.500000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 6.050000e+00, 6.300000e+00, 6.550000e+00, 6.800000e+00 }, // SINR
          { 9.980769e-01, 6.815160e-01, 3.990000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 5.878100e+00, 6.094500e+00, 6.311000e+00, 6.527400e+00, 6.743800e+00 }, // SINR
          { 1, 8.809524e-01, 2.800000e-01, 1.160000e-02, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 5.997400e+00, 6.272600e+00, 6.547900e+00, 6.823100e+00 }, // SINR
          { 9.942308e-01, 6.243932e-01, 3.920000e-02, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 6.039200e+00, 6.372400e+00, 6.705600e+00, 7.038800e+00 }, // SINR
          { 9.903846e-01, 3.659942e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 6.042000e+00, 6.341600e+00, 6.641200e+00, 6.940800e+00 }, // SINR
          { 9.836538e-01, 3.872324e-01, 3.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 5.955500e+00, 6.251200e+00, 6.547000e+00, 6.842700e+00 }, // SINR
          { 9.951923e-01, 5.614035e-01, 1.060000e-02, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 6.078800e+00, 6.300800e+00, 6.522700e+00, 6.744600e+00 }, // SINR
          { 9.836538e-01, 5.680310e-01, 5.080000e-02, 5.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 5.212200e+00, 5.841500e+00, 6.470800e+00, 7.100100e+00, 7.729400e+00 }, // SINR
          { 9.990385e-01, 8.741497e-01, 9.443099e-02, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 5.474600e+00, 6.011500e+00, 6.548400e+00, 7.085400e+00 }, // SINR
          { 9.522059e-01, 3.153125e-01, 4.600000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 5.296500e+00, 5.824400e+00, 6.180000e+00, 6.352300e+00, 6.530000e+00, 6.880300e+00, 7.408200e+00 }, // SINR
          { 1, 9.503676e-01, 4.800380e-01, 2.046748e-01, 1.599171e-01, 1.000000e-04, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 5.700000e+00, 6.100000e+00, 6.300000e+00, 6.500000e+00, 6.900000e+00 }, // SINR
          { 1, 5.257202e-01, 2.695396e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 5.200000e+00, 5.775000e+00, 6.350000e+00, 6.925000e+00, 7.500000e+00 }, // SINR
          { 1, 9.807692e-01, 2.958528e-01, 1.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 5.263800e+00, 5.764800e+00, 6.265900e+00, 6.766900e+00, 7.267900e+00 }, // SINR
          { 1, 9.980769e-01, 6.048578e-01, 1.030000e-02, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 5.188100e+00, 5.688100e+00, 5.900000e+00, 6.112000e+00, 6.323900e+00, 6.535900e+00, 7.035900e+00 }, // SINR
          { 1, 7.978395e-01, 6.935484e-01, 5.865826e-01, 4.619565e-01, 3.361037e-01, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 5.264500e+00, 5.789200e+00, 6.140000e+00, 6.313900e+00, 6.490000e+00, 6.838600e+00, 7.363300e+00 }, // SINR
          { 1, 9.630682e-01, 4.711896e-01, 1.517107e-01, 1.382450e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 5.292700e+00, 5.780500e+00, 6.110000e+00, 6.268400e+00, 6.430000e+00, 6.756300e+00, 7.244100e+00 }, // SINR
          { 1, 9.661654e-01, 5.285270e-01, 1.854720e-01, 1.777620e-01, 1.000000e-04, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 5.124600e+00, 5.640800e+00, 5.980000e+00, 6.157000e+00, 6.330000e+00, 6.673300e+00, 7.189500e+00 }, // SINR
          { 1, 9.875000e-01, 6.305419e-01, 2.184783e-01, 2.154210e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 5.288700e+00, 5.788700e+00, 6.098500e+00, 6.408300e+00, 6.718100e+00, 7.027900e+00, 7.527900e+00 }, // SINR
          { 1, 7.125000e-01, 5.161290e-01, 3.051559e-01, 1.438002e-01, 5.120000e-02, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 5.247800e+00, 5.747800e+00, 6.166000e+00, 6.584200e+00, 7.002300e+00, 7.420500e+00, 7.920500e+00 }, // SINR
          { 1, 7.383721e-01, 4.647436e-01, 2.035541e-01, 4.910000e-02, 8.900000e-03, 0 } // BLER
        }
      },
      { 3240U, // SINR and BLER for CBS 3240
        NrEesmErrorModel::DoubleTuple{
          { 5.250000e+00, 5.750000e+00, 6.148000e+00, 6.546100e+00, 6.944100e+00, 7.342100e+00, 7.842100e+00 }, // SINR
          { 1, 8.132911e-01, 5.059524e-01, 1.881559e-01, 3.860000e-02, 4.200000e-03, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 5.566200e+00, 6.093800e+00, 6.621400e+00, 7.149000e+00, 7.676600e+00 }, // SINR
          { 9.951923e-01, 4.249161e-01, 7.910000e-02, 4.600000e-03, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 11
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 5.596300e+00, 6.096300e+00, 6.443800e+00, 6.791300e+00, 7.138800e+00, 7.486300e+00, 7.986300e+00 }, // SINR
          { 1, 8.870690e-01, 6.361940e-01, 3.156095e-01, 8.997198e-02, 1.360000e-02, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 6, 6.700000e+00, 6.930000e+00, 7.170000e+00, 7.400000e+00, 8.100000e+00 }, // SINR
          { 1, 9.652256e-01, 1.204151e-01, 1.200000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 6, 6.575000e+00, 7.150000e+00, 7.725000e+00, 8.300000e+00 }, // SINR
          { 9.770992e-01, 6.696891e-01, 1.365239e-01, 4.200000e-03, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 5.823000e+00, 6.327200e+00, 6.831300e+00, 7.335400e+00, 7.839600e+00, 8.343800e+00 }, // SINR
          { 9.913462e-01, 8.306452e-01, 3.080685e-01, 2.920000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 6.008100e+00, 6.508100e+00, 6.821900e+00, 7.135700e+00, 7.449500e+00, 7.763300e+00 }, // SINR
          { 1, 6.256098e-01, 2.622651e-01, 5.140000e-02, 5.700000e-03, 5.000000e-04 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 6.071200e+00, 6.392900e+00, 6.714500e+00, 7.036200e+00, 7.357800e+00, 7.857800e+00 }, // SINR
          { 9.671053e-01, 8.451987e-01, 4.907946e-01, 1.385061e-01, 1.940000e-02, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 6.300000e+00, 6.581200e+00, 6.862500e+00, 7.143700e+00, 7.425000e+00, 7.925000e+00 }, // SINR
          { 9.086879e-01, 7.076503e-01, 3.431572e-01, 9.395292e-02, 1.460000e-02, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 6, 6.468800e+00, 6.937500e+00, 7.406200e+00, 7.875000e+00 }, // SINR
          { 9.725379e-01, 6.368750e-01, 1.146119e-01, 4.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 6.124900e+00, 6.436400e+00, 6.747900e+00, 7.059400e+00, 7.370900e+00, 7.870900e+00 }, // SINR
          { 9.846154e-01, 8.741497e-01, 5.224490e-01, 1.547472e-01, 1.700000e-02, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 6.300000e+00, 6.675000e+00, 7.050000e+00, 7.425000e+00, 7.800000e+00 }, // SINR
          { 9.770992e-01, 6.935484e-01, 1.770099e-01, 1.030000e-02, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 5.983800e+00, 6.302300e+00, 6.620800e+00, 6.939300e+00, 7.257800e+00, 7.757800e+00 }, // SINR
          { 9.951923e-01, 8.996479e-01, 4.990196e-01, 9.241754e-02, 4.400000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 6.204500e+00, 6.443200e+00, 6.681900e+00, 6.920600e+00, 7.159300e+00, 7.659300e+00 }, // SINR
          { 9.913462e-01, 9.084507e-01, 6.559278e-01, 2.735900e-01, 5.800000e-02, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 6.401200e+00, 6.626100e+00, 6.851000e+00, 7.075900e+00, 7.300800e+00, 7.800800e+00 }, // SINR
          { 9.817308e-01, 8.483333e-01, 5.321577e-01, 1.732094e-01, 3.170000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 6.405700e+00, 6.628800e+00, 6.851900e+00, 7.075000e+00, 7.298100e+00, 7.798100e+00 }, // SINR
          { 9.617537e-01, 8.225806e-01, 4.681734e-01, 1.358108e-01, 1.700000e-02, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.525000e+00, 7.150000e+00, 7.775000e+00 }, // SINR
          { 1, 8.501656e-01, 3.680000e-02, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 6.006300e+00, 6.381300e+00, 6.756300e+00, 7.131300e+00, 7.506300e+00 }, // SINR
          { 1, 9.420290e-01, 4.489437e-01, 3.030000e-02, 3.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 6.387900e+00, 6.633700e+00, 6.879600e+00, 7.125400e+00, 7.371200e+00, 7.871200e+00 }, // SINR
          { 9.942308e-01, 9.193262e-01, 5.769231e-01, 1.534022e-01, 1.110000e-02, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 6.464400e+00, 6.694600e+00, 6.924900e+00, 7.155100e+00, 7.385300e+00 }, // SINR
          { 9.154930e-01, 5.797511e-01, 1.611716e-01, 1.420000e-02, 4.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 6, 6.500000e+00, 6.719700e+00, 6.939400e+00, 7.159100e+00, 7.378800e+00 }, // SINR
          { 1, 8.576159e-01, 4.572842e-01, 8.530000e-02, 5.700000e-03, 3.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 6.067600e+00, 6.567600e+00, 6.731800e+00, 6.896000e+00, 7.060100e+00, 7.224300e+00 }, // SINR
          { 1, 6.493590e-01, 2.901376e-01, 7.190000e-02, 7.700000e-03, 5.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 6.200000e+00, 6.700000e+00, 6.862300e+00, 7.024700e+00, 7.187000e+00, 7.349300e+00, 7.849300e+00 }, // SINR
          { 1, 6.194581e-01, 2.542929e-01, 4.650000e-02, 5.800000e-03, 1.200000e-03, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 6.162800e+00, 6.662800e+00, 6.823900e+00, 6.984900e+00, 7.146000e+00, 7.307100e+00, 7.807100e+00 }, // SINR
          { 1, 8.615772e-01, 5.291667e-01, 1.728650e-01, 2.410000e-02, 2.400000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 6.698600e+00, 6.847700e+00, 6.996800e+00, 7.295000e+00, 7.742300e+00 }, // SINR
          { 9.617537e-01, 3.867424e-01, 2.120573e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 6.013800e+00, 6.513800e+00, 6.731800e+00, 6.949900e+00, 7.168000e+00, 7.386000e+00 }, // SINR
          { 1, 7.378571e-01, 2.186411e-01, 1.080000e-02, 3.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 6.578600e+00, 6.741800e+00, 6.904900e+00, 7.068100e+00, 7.231300e+00, 7.731300e+00 }, // SINR
          { 9.276786e-01, 6.759259e-01, 2.755991e-01, 5.080000e-02, 4.500000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 6.443600e+00, 6.739300e+00, 6.887100e+00, 7.034900e+00, 7.330600e+00 }, // SINR
          { 9.583333e-01, 8.716216e-01, 1.191998e-01, 7.956365e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 6.510400e+00, 6.726100e+00, 6.941700e+00, 7.157400e+00, 7.373100e+00 }, // SINR
          { 9.913462e-01, 8.387097e-01, 2.985782e-01, 2.190000e-02, 2.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 6.168200e+00, 6.668200e+00, 6.896300e+00, 7.124400e+00, 7.352500e+00 }, // SINR
          { 1, 8.221154e-01, 2.402008e-01, 7.100000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 6.084600e+00, 6.584600e+00, 6.734500e+00, 6.884300e+00, 7.034200e+00, 7.184100e+00 }, // SINR
          { 1, 8.035714e-01, 3.923611e-01, 7.940000e-02, 6.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 5.815300e+00, 6.315300e+00, 6.815300e+00, 6.958200e+00, 7.101200e+00, 7.244200e+00, 7.387100e+00 }, // SINR
          { 1, 8.640940e-01, 3.944704e-01, 9.805945e-02, 9.300000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 6.502500e+00, 6.771100e+00, 7.039700e+00, 7.308300e+00 }, // SINR
          { 9.611742e-01, 3.868140e-01, 9.900000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.400000e+00, 6.730000e+00, 6.900000e+00, 7.070000e+00, 7.400000e+00 }, // SINR
          { 1, 9.980769e-01, 2.519960e-01, 1.864785e-01, 3.120000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 6.400000e+00, 6.800000e+00, 7.300000e+00, 7.700000e+00 }, // SINR
          { 1, 7.962963e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 5.800000e+00, 6.300000e+00, 6.525000e+00, 6.750000e+00, 6.975000e+00, 7.200000e+00, 7.700000e+00, 8.200000e+00 }, // SINR
          { 9.980769e-01, 7.820122e-01, 6.444724e-01, 4.748134e-01, 3.079490e-01, 1.655716e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 6.265400e+00, 6.704300e+00, 7.143300e+00, 7.582300e+00 }, // SINR
          { 1, 6.841398e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 5.800000e+00, 6.300000e+00, 6.475000e+00, 6.650000e+00, 6.825000e+00, 7, 7.500000e+00, 8 }, // SINR
          { 1, 8.949653e-01, 8.256369e-01, 7.046703e-01, 5.709459e-01, 4.327911e-01, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 6.100000e+00, 6.600000e+00, 6.750000e+00, 6.900000e+00, 7.050000e+00, 7.200000e+00, 7.700000e+00, 8.200000e+00 }, // SINR
          { 1, 6.750000e-01, 5.385021e-01, 4.142157e-01, 2.872159e-01, 1.876866e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.150000e+00, 6.400000e+00, 6.700000e+00, 6.900000e+00, 7.200000e+00, 7.400000e+00, 7.650000e+00, 7.900000e+00, 8.150000e+00, 8.400000e+00, 8.650000e+00, 8.900000e+00, 9.150000e+00 }, // SINR
          { 9.430147e-01, 8.657718e-01, 7.676471e-01, 5.915899e-01, 4.499113e-01, 2.663502e-01, 1.541054e-01, 6.930000e-02, 2.450000e-02, 8.100000e-03, 1.600000e-03, 5.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 6.200000e+00, 6.500000e+00, 6.800000e+00, 7.100000e+00, 7.400000e+00, 7.700000e+00, 8, 8.300000e+00, 8.600000e+00, 8.900000e+00, 9.200000e+00, 9.500000e+00, 9.800000e+00 }, // SINR
          { 9.202128e-01, 7.718373e-01, 5.199795e-01, 2.675159e-01, 8.090000e-02, 7.560000e-02, 2.080000e-02, 4.000000e-03, 9.000000e-04, 2.000000e-04, 2.000000e-04, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 5.950000e+00, 6.400000e+00, 6.850000e+00, 7.300000e+00, 7.750000e+00, 8.200000e+00, 8.650000e+00, 9.100000e+00 }, // SINR
          { 9.280576e-01, 7.676471e-01, 4.907588e-01, 2.042683e-01, 4.910000e-02, 5.700000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 6.348600e+00, 6.730000e+00, 6.916400e+00, 7.110000e+00, 7.484200e+00, 8.052000e+00, 8.619800e+00 }, // SINR
          { 9.932692e-01, 4.191419e-01, 2.514940e-01, 7.500000e-02, 2.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.400000e+00, 6.650000e+00, 6.900000e+00, 7.400000e+00, 8.150000e+00 }, // SINR
          { 1, 7.220670e-01, 4.040605e-01, 1.006198e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 5.800000e+00, 6.250000e+00, 6.475000e+00, 6.700000e+00, 7.150000e+00, 7.825000e+00 }, // SINR
          { 1, 9.329710e-01, 8.362903e-01, 3.692029e-01, 7.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 12
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 6.650000e+00, 7.280000e+00, 7.593800e+00, 7.910000e+00, 8.538900e+00 }, // SINR
          { 9.990385e-01, 8.338710e-01, 7.929448e-01, 3.222010e-01, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 6.482700e+00, 7.342700e+00, 7.677500e+00, 8.202700e+00 }, // SINR
          { 1, 6.699219e-01, 1.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 6.960000e+00, 7.450000e+00, 7.687500e+00, 7.930000e+00, 8.418500e+00 }, // SINR
          { 9.769231e-01, 7.449128e-01, 3.985849e-01, 3.168750e-01, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 7.577900e+00, 7.676046e+00, 8.812900e+00, 1.004790e+01 }, // SINR
          { 9.640152e-01, 6.564103e-01, 9.324434e-02, 1.400000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 7, 7.250000e+00, 7.500000e+00, 7.750000e+00, 8, 8.250000e+00, 8.500000e+00, 8.750000e+00, 9, 9.250000e+00, 9.500000e+00 }, // SINR
          { 9.453704e-01, 8.750000e-01, 5.717489e-01, 3.210227e-01, 1.295196e-01, 3.420000e-02, 6.500000e-03, 5.800000e-03, 1.500000e-03, 1.200000e-03, 2.000000e-04 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 7, 7.200000e+00, 7.400000e+00, 7.600000e+00, 7.800000e+00, 8, 8.200000e+00, 8.400000e+00, 8.600000e+00, 8.800000e+00, 9, 9.200000e+00 }, // SINR
          { 9.365942e-01, 7.833333e-01, 6.717105e-01, 4.355670e-01, 2.366323e-01, 9.111201e-02, 2.920000e-02, 1.100000e-02, 1.700000e-03, 6.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 7.100000e+00, 7.600000e+00, 7.800000e+00, 8.100000e+00, 8.600000e+00 }, // SINR
          { 1, 3.684593e-01, 3.502747e-01, 2.600000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 7.300000e+00, 7.600000e+00, 7.900000e+00, 8.200000e+00, 9.100000e+00 }, // SINR
          { 1, 2.355140e-01, 1.140000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 7.206500e+00, 7.617000e+00, 8.070100e+00, 8.933700e+00 }, // SINR
          { 9.875000e-01, 7.789634e-01, 2.048046e-01, 6.000000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 6.950000e+00, 7.200000e+00, 7.450000e+00, 7.700000e+00, 7.950000e+00, 8.200000e+00, 8.450000e+00, 8.700000e+00, 8.950000e+00 }, // SINR
          { 9.537037e-01, 8.699664e-01, 6.311881e-01, 3.229434e-01, 1.030906e-01, 1.910000e-02, 2.800000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 7.645900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.942308e-01, 9.241071e-01, 5.481602e-01, 1.177773e-01, 5.200000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 7.687700e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.913462e-01, 9.298561e-01, 5.723982e-01, 1.099867e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 7.645300e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.961538e-01, 9.276786e-01, 5.427215e-01, 8.773167e-02, 2.300000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.475500e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.980769e-01, 8.987676e-01, 4.431424e-01, 4.680000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.971154e-01, 8.914931e-01, 3.929128e-01, 2.840000e-02, 5.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.980769e-01, 9.271583e-01, 4.357759e-01, 3.640000e-02, 3.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.980769e-01, 8.827586e-01, 2.915704e-01, 9.500000e-03, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.990385e-01, 9.388489e-01, 3.336640e-01, 8.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 1, 8.977273e-01, 2.527555e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 1, 9.169643e-01, 2.204983e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 1, 9.119718e-01, 1.716621e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.985600e+00, 8.396200e+00 }, // SINR
          { 9.836538e-01, 8.632550e-01, 2.049347e-01, 3.360000e-02, 2.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.985600e+00, 8.396200e+00 }, // SINR
          { 9.689850e-01, 7.618343e-01, 1.721311e-01, 3.010000e-02, 5.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.990385e-01, 1.857407e-01, 6.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.990385e-01, 1.323996e-01, 2.050000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.985600e+00, 8.396200e+00 }, // SINR
          { 9.971154e-01, 8.431373e-01, 1.840278e-01, 1.010000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.990385e-01, 5.501078e-01, 6.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 7.300000e+00, 7.700000e+00, 8, 8.400000e+00 }, // SINR
          { 1, 8.266129e-01, 3.920000e-02, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.985600e+00, 8.396200e+00 }, // SINR
          { 9.923077e-01, 7.932099e-01, 3.904321e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 1, 4.209866e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 6.953552e-01, 2.839888e-01, 2.250000e-02, 1.000000e-03 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 7.740964e-01, 3.083538e-01, 1.120000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00, 8.498900e+00 }, // SINR
          { 1, 6.821809e-01, 3.132678e-01, 1.172535e-01, 2.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 7.626488e-01, 2.420673e-01, 1.430000e-02, 4.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 7.217514e-01, 2.425193e-01, 1.460000e-02, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 7.120166e-01, 2.020000e-01, 4.360000e-02, 7.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 6.375000e-01, 2.254025e-01, 1.630000e-02, 2.000000e-04 } // BLER
        }
      }
  },
  { // MCS 13
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 7.987000e+00, 8.284000e+00, 8.447000e+00, 8.677000e+00, 8.907000e+00, 9.137000e+00 }, // SINR
          { 1, 6.395833e-01, 8.130000e-02, 1.800000e-03, 1.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 7.800000e+00, 8.230000e+00, 8.650000e+00, 9.080000e+00, 9.500000e+00, 9.930000e+00, 1.035000e+01 }, // SINR
          { 9.865385e-01, 8.105346e-01, 4.142157e-01, 7.780000e-02, 5.000000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 8, 8.330000e+00, 8.500000e+00, 8.670000e+00, 9 }, // SINR
          { 1, 9.942308e-01, 4.199670e-01, 3.735119e-01, 6.320000e-02, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 7.970000e+00, 8.280000e+00, 8.430000e+00, 8.590000e+00, 8.900000e+00 }, // SINR
          { 1, 9.932692e-01, 6.096698e-01, 3.415541e-01, 1.824422e-01, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 8.430000e+00, 8.900000e+00, 9.370000e+00, 1.030000e+01 }, // SINR
          { 1, 1.684492e-01, 1.210000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 8.400000e+00, 8.530000e+00, 8.670000e+00, 8.800000e+00, 9.200000e+00, 9.600000e+00, 1.070000e+01 }, // SINR
          { 9.393116e-01, 1.300362e-01, 3.910000e-02, 8.000000e-04, 1.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 8.400000e+00, 8.620000e+00, 8.730000e+00, 8.850000e+00, 9.070000e+00, 9.400000e+00 }, // SINR
          { 1, 9.384191e-01, 7.370000e-02, 1.120000e-02, 7.400000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 7.400000e+00, 8.400000e+00, 9.400000e+00, 1.040000e+01, 1.140000e+01 }, // SINR
          { 1, 6.363065e-01, 1.043441e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 7.600000e+00, 8.500000e+00, 9.400000e+00, 1.030000e+01, 1.120000e+01 }, // SINR
          { 1, 5.799087e-01, 7.380000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.573700e+00, 9.677900e+00, 1.078220e+01, 1.188640e+01 }, // SINR
          { 9.636194e-01, 5.379747e-01, 4.070000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 7.980000e+00, 8.500000e+00, 9, 9.500000e+00, 1.010000e+01, 1.060000e+01 }, // SINR
          { 9.980769e-01, 6.552835e-01, 2.418269e-01, 4.500000e-02, 1.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.449900e+00, 9.430300e+00, 1.041080e+01, 1.139120e+01 }, // SINR
          { 9.649621e-01, 5.095766e-01, 4.010000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.388000e+00, 9.306500e+00, 1.022510e+01, 1.114360e+01 }, // SINR
          { 9.817308e-01, 6.578608e-01, 9.987593e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.264200e+00, 8.402700e+00, 9.058900e+00, 9.853700e+00 }, // SINR
          { 9.671053e-01, 6.362500e-01, 1.220958e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.636194e-01, 6.734293e-01, 1.732759e-01, 8.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.818702e-01, 8.160828e-01, 2.888128e-01, 2.260000e-02, 3.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.884615e-01, 7.399425e-01, 2.034790e-01, 1.040000e-02, 1.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.437500e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.580224e-01, 6.451005e-01, 1.186381e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.732824e-01, 6.815160e-01, 1.147448e-01, 1.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.367000e+00, 9.482300e+00 }, // SINR
          { 9.780534e-01, 6.858289e-01, 1.080139e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 8.100000e+00, 8.800000e+00, 9.500000e+00 }, // SINR
          { 1, 8.959790e-01, 4.340000e-02, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.923077e-01, 6.970109e-01, 5.750000e-02, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.875000e-01, 6.095972e-01, 3.230000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.942308e-01, 6.160287e-01, 2.070000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.903846e-01, 5.900229e-01, 1.510000e-02, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.923077e-01, 5.498927e-01, 1.030000e-02, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.913462e-01, 5.445279e-01, 7.100000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.971154e-01, 5.764840e-01, 6.900000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.961538e-01, 4.699074e-01, 2.500000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 5.543478e-01, 2.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 7.271429e-01, 7.300000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 8.835616e-01, 3.560000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 8.093750e-01, 1.810000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.587700e+00, 8.811300e+00, 9.035000e+00, 9.482300e+00 }, // SINR
          { 1, 9.393116e-01, 2.658228e-01, 3.260000e-02, 2.460000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 7.227654e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 8.509934e-01, 7.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 7.500000e-01, 1.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 8.258065e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 6.980874e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 7.893519e-01, 1.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 7.900000e+00, 8.100000e+00, 8.400000e+00, 8.600000e+00, 8.800000e+00, 9.030000e+00, 9.250000e+00, 9.480000e+00 }, // SINR
          { 9.894231e-01, 9.049296e-01, 4.824144e-01, 1.637987e-01, 3.280000e-02, 1.060000e-02, 1.000000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 14
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 8.500000e+00, 8.800000e+00, 9.100000e+00, 9.400000e+00, 9.700000e+00, 10, 1.030000e+01, 1.060000e+01, 1.090000e+01 }, // SINR
          { 1, 9.990385e-01, 9.205357e-01, 4.425087e-01, 5.580000e-02, 1.450000e-02, 1.100000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 8, 9.400000e+00, 1.080000e+01, 1.220000e+01 }, // SINR
          { 1, 7.492690e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 8.700000e+00, 9.730000e+00, 1.030000e+01, 1.077000e+01, 1.180000e+01 }, // SINR
          { 9.214286e-01, 7.500000e-03, 5.700000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 7.200000e+00, 8.700000e+00, 9.700000e+00, 1.020000e+01, 1.070000e+01 }, // SINR
          { 1, 9.145683e-01, 2.740000e-02, 8.300000e-03, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 8.700000e+00, 9.430000e+00, 9.800000e+00, 1.017000e+01 }, // SINR
          { 9.347826e-01, 2.704741e-01, 8.907126e-02, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 7.400000e+00, 8.700000e+00, 9.600000e+00, 1.010000e+01, 1.050000e+01 }, // SINR
          { 1, 9.334532e-01, 9.465021e-02, 1.000000e-02, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 8.700000e+00, 8.980000e+00, 9.250000e+00, 9.530000e+00, 9.800000e+00, 1.008000e+01, 1.035000e+01 }, // SINR
          { 9.990385e-01, 9.586466e-01, 6.099760e-01, 1.280644e-01, 5.500000e-03, 1.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.964400e+00, 1.119230e+01, 1.242030e+01 }, // SINR
          { 9.507299e-01, 5.300830e-01, 7.300000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.902500e+00, 1.106850e+01, 1.223460e+01, 1.340060e+01 }, // SINR
          { 9.214286e-01, 3.825758e-01, 2.380000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.840600e+00, 1.094470e+01, 1.204890e+01, 1.315300e+01 }, // SINR
          { 9.293478e-01, 4.751873e-01, 3.790000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.778700e+00, 1.082090e+01, 1.186320e+01, 1.290540e+01 }, // SINR
          { 9.614662e-01, 5.231481e-01, 5.470000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 7.756200e+00, 8.736500e+00, 9.716800e+00, 1.069710e+01, 1.167750e+01 }, // SINR
          { 1, 8.801020e-01, 3.466530e-01, 2.360000e-02, 1.000000e-04 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 8.052500e+00, 8.736500e+00, 9.593000e+00, 1.044950e+01, 1.130610e+01 }, // SINR
          { 9.406475e-01, 5.161943e-01, 8.120000e-02, 2.200000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01 }, // SINR
          { 9.031690e-01, 4.288721e-01, 6.760000e-02, 1.900000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01, 1.166740e+01 }, // SINR
          { 9.343525e-01, 5.418432e-01, 9.532520e-02, 3.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01, 1.166740e+01 }, // SINR
          { 1, 8.809524e-01, 3.814006e-01, 3.500000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01, 1.166740e+01 }, // SINR
          { 1, 8.956897e-01, 3.965517e-01, 3.150000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 8.250000e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01 }, // SINR
          { 9.552239e-01, 5.675223e-01, 7.050000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01, 1.166740e+01 }, // SINR
          { 9.498175e-01, 4.960784e-01, 4.880000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.055944e-01, 2.809710e-01, 8.600000e-03, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.406934e-01, 3.285714e-01, 6.400000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.241071e-01, 2.787611e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01 }, // SINR
          { 9.903846e-01, 8.987676e-01, 1.397715e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.835600e+00 }, // SINR
          { 1, 7.078729e-01, 1.462704e-01, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.835600e+00, 1.020190e+01 }, // SINR
          { 1, 8.050314e-01, 1.597398e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.835600e+00 }, // SINR
          { 1, 8.409091e-01, 1.574248e-01, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.102900e+00, 9.469200e+00, 9.835500e+00 }, // SINR
          { 1, 8.616667e-01, 4.780000e-02, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.343525e-01, 9.000000e-01, 9.655383e-02, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.250000e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.512868e-01, 9.000000e-01, 8.716707e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.194450e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.430147e-01, 9.000000e-01, 7.280000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 1, 4.074519e-01, 1.583753e-01, 1.840000e-02, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 1, 3.899540e-01, 2.052117e-01, 1.370000e-02, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.522059e-01, 3.840634e-01, 2.056056e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.636194e-01, 5.793379e-01, 1.805755e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 1, 8.949653e-01, 1.321053e-01, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.420290e-01, 7.127809e-01, 1.807554e-01, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.088542e-01, 5.334728e-01, 6.540000e-02, 3.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.507299e-01, 7.694611e-01, 8.924279e-02, 1.010000e-02, 1.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 1, 8.810345e-01, 8.420000e-02, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.125874e-01, 8.330592e-01, 4.620000e-02, 9.600000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 15
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.038000e+01, 1.057000e+01, 1.075000e+01, 1.113000e+01 }, // SINR
          { 9.531250e-01, 1.193994e-01, 3.300000e-03, 8.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 9, 1.000360e+01, 1.166470e+01, 1.332580e+01, 1.498690e+01 }, // SINR
          { 9.498175e-01, 5.887097e-01, 1.304687e-01, 5.100000e-03, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 9.250000e+00, 1.000360e+01, 1.157180e+01, 1.314010e+01, 1.470830e+01 }, // SINR
          { 9.444444e-01, 5.633260e-01, 9.689289e-02, 2.600000e-03, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 9.102900e+00, 1.000360e+01, 1.154090e+01, 1.307820e+01, 1.461550e+01 }, // SINR
          { 9.000000e-01, 8.093354e-01, 2.468075e-01, 1.090000e-02, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 8.590100e+00, 1.000360e+01, 1.141710e+01, 1.283060e+01, 1.424410e+01, 1.565760e+01 }, // SINR
          { 9.817308e-01, 8.392857e-01, 3.034420e-01, 2.450000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 9.286000e+00, 1.000360e+01, 1.135520e+01, 1.270680e+01, 1.405840e+01 }, // SINR
          { 9.000000e-01, 8.427152e-01, 3.033573e-01, 1.880000e-02, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 8.713900e+00, 1.000360e+01, 1.129330e+01, 1.258300e+01, 1.387270e+01, 1.516240e+01 }, // SINR
          { 9.894231e-01, 8.716216e-01, 3.134328e-01, 2.150000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 9.250000e+00, 1.000360e+01, 1.123140e+01, 1.245920e+01, 1.368700e+01 }, // SINR
          { 9.000000e-01, 7.955247e-01, 1.738904e-01, 3.000000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 9.143667e+00, 1.000360e+01, 1.116950e+01, 1.233540e+01 }, // SINR
          { 9.258929e-01, 4.140625e-01, 2.280000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 9.135600e+00, 1.000360e+01, 1.110760e+01, 1.221160e+01, 1.331560e+01 }, // SINR
          { 9.000000e-01, 6.695026e-01, 1.009496e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 9.125000e+00, 1.000360e+01, 1.104570e+01, 1.208780e+01, 1.312990e+01 }, // SINR
          { 9.000000e-01, 7.038043e-01, 1.386740e-01, 2.300000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 9.085300e+00, 1.000360e+01, 1.092190e+01, 1.184020e+01, 1.275850e+01 }, // SINR
          { 9.903846e-01, 7.761976e-01, 1.900376e-01, 7.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01, 1.318160e+01 }, // SINR
          { 9.894231e-01, 7.875767e-01, 2.709227e-01, 2.280000e-02, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01, 1.318160e+01 }, // SINR
          { 9.537037e-01, 8.101266e-01, 2.917633e-01, 1.820000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 9.473900e+00, 1.000360e+01, 1.053854e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 1, 6.112440e-01, 9.733441e-02, 1.400000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01 }, // SINR
          { 9.652256e-01, 6.407828e-01, 1.086192e-01, 2.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01 }, // SINR
          { 9.942308e-01, 7.087989e-01, 1.258741e-01, 2.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01 }, // SINR
          { 9.942308e-01, 6.165865e-01, 7.650000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01 }, // SINR
          { 9.770992e-01, 7.306034e-01, 9.020000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.751908e-01, 6.237864e-01, 4.150000e-02, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.923077e-01, 6.987705e-01, 5.040000e-02, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 9.503600e+00, 1.000360e+01, 1.051730e+01, 1.079810e+01 }, // SINR
          { 1, 6.280941e-01, 2.130000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 7.212079e-01, 2.860000e-02, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 6.784759e-01, 1.730000e-02, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 9.920100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 5.762332e-01, 7.200000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.818702e-01, 6.505102e-01, 1.110000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 9.503600e+00, 1.000360e+01, 1.059850e+01, 1.079810e+01 }, // SINR
          { 1, 5.985915e-01, 5.000000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.961538e-01, 5.755682e-01, 2.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 9.612800e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 5.616812e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 9.625000e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 5.885417e-01, 9.000000e-04, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 9.750000e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 5.581140e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 1928U, // SINR and BLER for CBS 1928
        NrEesmErrorModel::DoubleTuple{
          { 8.987000e+00, 1.000360e+01, 1.102020e+01, 1.305330e+01 }, // SINR
          { 1, 6.895161e-01, 2.000000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 9.500000e+00, 9.800000e+00, 1.020000e+01, 1.050000e+01, 1.080000e+01, 1.113000e+01 }, // SINR
          { 9.875954e-01, 8.852740e-01, 4.865385e-01, 1.568352e-01, 2.290000e-02, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 9.500000e+00, 9.800000e+00, 1.020000e+01, 1.050000e+01, 1.080000e+01, 1.113000e+01 }, // SINR
          { 9.990385e-01, 9.678030e-01, 6.274631e-01, 1.833942e-01, 1.540000e-02, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 7.210452e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.836538e-01, 8.140823e-01, 2.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 7.551775e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 8.993500e+00, 1.000360e+01, 1.101380e+01, 1.303400e+01 }, // SINR
          { 1, 7.953125e-01, 5.600000e-03, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 9.473900e+00, 1.000360e+01, 1.012800e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 1, 7.602941e-01, 6.525641e-01, 5.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 16
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 9.830000e+00, 1.057000e+01, 1.081000e+01, 1.106000e+01, 1.130000e+01, 1.350000e+01 }, // SINR
          { 1, 9.990385e-01, 3.783582e-01, 4.250000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070000e+01, 11, 1.130000e+01, 1.160000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.280000e+01 }, // SINR
          { 9.055944e-01, 7.090278e-01, 4.057508e-01, 1.574655e-01, 3.720000e-02, 1.530000e-02, 1.700000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.030000e+01, 1.080000e+01, 1.140000e+01, 1.250000e+01 }, // SINR
          { 1, 5.427350e-01, 2.000000e-03, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 1.050000e+01, 1.065000e+01, 1.080000e+01, 1.095000e+01, 1.110000e+01, 1.125000e+01, 1.140000e+01 }, // SINR
          { 9.232143e-01, 8.423203e-01, 1.888138e-01, 6.970000e-02, 1.730000e-02, 2.500000e-03, 6.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 1.037000e+01, 1.080000e+01, 1.133000e+01, 1.230000e+01, 1.380000e+01 }, // SINR
          { 1, 4.499113e-01, 7.870000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 1.030000e+01, 1.107000e+01, 1.140000e+01, 1.183000e+01 }, // SINR
          { 9.836538e-01, 5.686384e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 1.043000e+01, 1.080000e+01, 1.127000e+01, 1.210000e+01, 1.340000e+01 }, // SINR
          { 1, 8.750000e-01, 1.750000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 9.640800e+00, 1.084790e+01, 1.205500e+01, 1.326210e+01 }, // SINR
          { 1, 6.018957e-01, 5.690000e-02, 1.000000e-04 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 9.702700e+00, 1.084790e+01, 1.199310e+01, 1.313830e+01, 1.428350e+01 }, // SINR
          { 9.980769e-01, 5.833333e-01, 5.670000e-02, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 9.764600e+00, 1.084790e+01, 1.193120e+01, 1.301450e+01, 1.409780e+01 }, // SINR
          { 1, 5.817972e-01, 6.680000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.023060e+01, 1.084790e+01, 1.180740e+01, 1.276690e+01, 1.372640e+01 }, // SINR
          { 9.000000e-01, 5.752262e-01, 8.520000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.025000e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.000000e-01, 6.183894e-01, 1.199905e-01, 4.300000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.021290e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.000000e-01, 5.748874e-01, 9.428052e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.017255e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.000000e-01, 6.081731e-01, 9.023312e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.031250e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.000000e-01, 5.680310e-01, 7.480000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.001220e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.990385e-01, 6.129808e-01, 6.830000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.075000e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 4.372852e-01, 2.570000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.001220e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.961538e-01, 4.190199e-01, 1.560000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.001220e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 1, 3.521468e-01, 4.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.081250e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.518006e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.635057e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.044069e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.649135e-01, 2.400000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.053450e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 2.711910e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.053450e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 2.860795e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.063900e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.039568e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.050000e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.159375e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.050000e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.868499e-01, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.053450e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.960094e-01, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.069888e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.406951e-01, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.063402e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.253743e-01, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.051545e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.511733e-01, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.049485e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 4.540179e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.064118e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 4.581835e-01, 8.000000e-04, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.066766e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 3.280848e-01, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.057950e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 2.944965e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.057950e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 3.684593e-01, 1.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.062830e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 2.608471e-01, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.062830e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 3.052885e-01, 0 } // BLER
        }
      }
  },
  { // MCS 17
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 1.080000e+01, 12, 1.260000e+01, 1.320000e+01 }, // SINR
          { 9.315693e-01, 2.510000e-02, 8.700000e-03, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 9.950000e+00, 1.150000e+01, 13, 1.460000e+01 }, // SINR
          { 1, 6.020047e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.005000e+01, 1.103000e+01, 1.150000e+01, 1.202000e+01, 13, 1.440000e+01 }, // SINR
          { 1, 9.605263e-01, 5.868056e-01, 1.000000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 1.080000e+01, 1.157000e+01, 12, 1.233000e+01, 1.310000e+01, 1.420000e+01 }, // SINR
          { 9.790076e-01, 7.588757e-01, 1.557859e-01, 2.000000e-04, 2.000000e-04, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 1.080000e+01, 1.153000e+01, 1.190000e+01, 1.227000e+01 }, // SINR
          { 9.846154e-01, 6.426768e-01, 2.000792e-01, 0 } // BLER
        }
      },
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 1.035000e+01, 1.113000e+01, 1.150000e+01, 1.192000e+01, 1.270000e+01, 1.380000e+01 }, // SINR
          { 1, 8.844828e-01, 4.956055e-01, 1.140000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 11, 1.125000e+01, 1.150000e+01, 1.175000e+01, 12, 1.225000e+01, 1.250000e+01 }, // SINR
          { 9.668561e-01, 8.205128e-01, 7.405523e-01, 2.758152e-01, 3.640000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 1.002380e+01, 1.082790e+01, 1.146870e+01, 1.203400e+01, 1.324010e+01 }, // SINR
          { 1, 8.913793e-01, 3.170426e-01, 1.200000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.197210e+01, 1.311630e+01, 1.426050e+01, 1.540470e+01 }, // SINR
          { 9.022887e-01, 3.385695e-01, 1.480000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.152990e+01, 1.191020e+01, 1.299250e+01, 1.407480e+01 }, // SINR
          { 9.193262e-01, 9.000000e-01, 3.624282e-01, 2.030000e-02, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.178640e+01, 1.274490e+01, 1.370340e+01, 1.466190e+01 }, // SINR
          { 9.058099e-01, 4.366379e-01, 4.090000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.146870e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01, 1.416670e+01 }, // SINR
          { 9.384058e-01, 9.000000e-01, 5.438034e-01, 9.523322e-02, 3.700000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.150000e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01, 1.416670e+01 }, // SINR
          { 9.430147e-01, 9.000000e-01, 5.004941e-01, 6.900000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01, 1.416670e+01 }, // SINR
          { 9.470803e-01, 5.080000e-01, 5.840000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01, 1.416670e+01 }, // SINR
          { 9.356884e-01, 4.702602e-01, 4.740000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01, 1.416670e+01 }, // SINR
          { 9.580224e-01, 5.193878e-01, 4.550000e-02, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.150000e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.169643e-01, 9.000000e-01, 3.593305e-01, 1.660000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01, 1.416670e+01 }, // SINR
          { 9.267857e-01, 3.245501e-01, 6.500000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.143105e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.465580e-01, 9.000000e-01, 2.858770e-01, 3.500000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 11, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.439338e-01, 9.000000e-01, 2.679325e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.150000e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.562044e-01, 9.000000e-01, 2.787611e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 11, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.668561e-01, 9.000000e-01, 2.717275e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.942308e-01, 2.071664e-01, 1.640000e-02, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.562044e-01, 8.665541e-01, 1.984252e-01, 8.100000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.942308e-01, 2.072368e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.951923e-01, 1.332535e-01, 3.100000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.865385e-01, 8.232484e-01, 1.325397e-01, 3.400000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01 }, // SINR
          { 9.809160e-01, 5.209184e-01, 1.330773e-01, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01 }, // SINR
          { 9.636194e-01, 8.767123e-01, 8.370000e-02, 8.600000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01 }, // SINR
          { 9.820076e-01, 9.119718e-01, 7.860000e-02, 5.100000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 9.990385e-01, 8.648649e-01, 8.423478e-02, 2.300000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01 }, // SINR
          { 9.980769e-01, 8.691275e-01, 2.537575e-01, 8.400000e-03, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.166260e+01, 1.249730e+01, 1.333200e+01 }, // SINR
          { 1, 3.212025e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01 }, // SINR
          { 9.913462e-01, 2.129237e-01, 7.470000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01 }, // SINR
          { 9.961538e-01, 1.881559e-01, 1.193834e-01, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01 }, // SINR
          { 1, 9.131206e-01, 2.234513e-01, 2.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01 }, // SINR
          { 9.884615e-01, 1.348013e-01, 2.680000e-02, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.082790e+01, 1.138440e+01, 1.166260e+01, 1.194080e+01, 1.249730e+01 }, // SINR
          { 1, 9.439338e-01, 1.788762e-01, 1.440000e-02, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 18
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 1.113000e+01, 1.170000e+01, 1.227000e+01, 1.340000e+01 }, // SINR
          { 1, 7.649701e-01, 1.342049e-01, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 1.113000e+01, 1.170000e+01, 1.227000e+01, 1.340000e+01 }, // SINR
          { 1, 8.248408e-01, 1.444189e-01, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 1.130000e+01, 1.170000e+01, 1.210000e+01, 1.290000e+01 }, // SINR
          { 1, 6.647135e-01, 3.935759e-01, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 1.207000e+01, 1.231000e+01, 1.243000e+01, 1.256000e+01, 1.280000e+01, 1.390000e+01 }, // SINR
          { 9.384058e-01, 9.202128e-01, 6.050000e-02, 4.540000e-02, 2.100000e-03, 8.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 1.240000e+01, 1.270000e+01, 1.310000e+01 }, // SINR
          { 9.704198e-01, 1.304517e-01, 4.300000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 1.237000e+01, 1.270000e+01, 1.303000e+01 }, // SINR
          { 9.865385e-01, 1.210145e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.080000e+01, 1.170000e+01, 1.227000e+01, 1.250000e+01, 1.283000e+01, 1.340000e+01 }, // SINR
          { 1, 9.620370e-01, 8.783784e-01, 9.300000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 1.042550e+01, 1.167280e+01, 1.292000e+01, 1.416730e+01, 1.541460e+01, 1.666190e+01 }, // SINR
          { 1, 8.384740e-01, 2.143463e-01, 7.300000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.088260e+01, 1.167280e+01, 1.210053e+01, 1.285810e+01, 1.404350e+01 }, // SINR
          { 1, 8.245192e-01, 2.049918e-01, 4.800000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.092380e+01, 1.167280e+01, 1.206210e+01, 1.279620e+01, 1.391970e+01 }, // SINR
          { 1, 8.148734e-01, 2.171280e-01, 4.900000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.067310e+01, 1.167280e+01, 1.267240e+01, 1.367210e+01, 1.467180e+01 }, // SINR
          { 1, 7.860429e-01, 2.165948e-01, 9.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01 }, // SINR
          { 1, 8.000000e-01, 2.430019e-01, 1.390000e-02, 1.000000e-04 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01, 1.517630e+01 }, // SINR
          { 1, 7.641369e-01, 2.058824e-01, 8.300000e-03, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01, 1.517630e+01 }, // SINR
          { 9.089286e-01, 4.116450e-01, 2.980000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01, 1.517630e+01 }, // SINR
          { 9.095745e-01, 3.867424e-01, 2.050000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01, 1.517630e+01 }, // SINR
          { 9.029720e-01, 3.075980e-01, 1.200000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01 }, // SINR
          { 1, 8.204114e-01, 1.871280e-01, 3.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01 }, // SINR
          { 1, 7.947531e-01, 9.650206e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 1, 8.070312e-01, 8.090000e-02, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.120000e+01, 1.150000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.283000e+01, 1.315000e+01 }, // SINR
          { 1, 9.961538e-01, 7.522059e-01, 1.837482e-01, 7.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.855769e-01, 7.551170e-01, 3.390000e-02, 1.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.855769e-01, 7.463235e-01, 2.590000e-02, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.120000e+01, 1.150000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.283000e+01, 1.315000e+01 }, // SINR
          { 1, 9.961538e-01, 7.321429e-01, 1.799215e-01, 6.000000e-03, 1.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.117280e+01, 1.167280e+01, 1.192575e+01, 1.254860e+01 }, // SINR
          { 9.444444e-01, 7.642216e-01, 1.990000e-02, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.117280e+01, 1.167280e+01, 1.227410e+01, 1.254860e+01 }, // SINR
          { 9.866412e-01, 7.795455e-01, 1.520000e-02, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.150000e+01, 1.178000e+01, 1.205000e+01, 1.233000e+01, 1.260000e+01, 1.288000e+01 }, // SINR
          { 1, 9.875000e-01, 6.708115e-01, 1.021303e-01, 1.900000e-03, 2.000000e-04 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.162500e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.000000e-01, 8.026730e-01, 1.040000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.139190e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.000000e-01, 8.113057e-01, 6.400000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.903846e-01, 7.718373e-01, 2.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.160000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.280000e+01 }, // SINR
          { 9.990385e-01, 9.214286e-01, 3.236607e-01, 9.600000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.065370e+01, 1.167280e+01, 1.269190e+01, 1.473010e+01 }, // SINR
          { 1, 7.171788e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 2152U, // SINR and BLER for CBS 2152
        NrEesmErrorModel::DoubleTuple{
          { 1.130000e+01, 1.170000e+01, 12, 1.240000e+01, 1.270000e+01 }, // SINR
          { 1, 9.942308e-01, 7.134831e-01, 1.540000e-02, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 12, 1.230000e+01, 1.250000e+01 }, // SINR
          { 9.352518e-01, 3.165625e-01, 6.600000e-03, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.150000e+01, 1.178000e+01, 1.205000e+01, 1.233000e+01, 1.260000e+01 }, // SINR
          { 1, 8.382353e-01, 2.050081e-01, 4.200000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.176067e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.470803e-01, 9.000000e-01, 9.400000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 12, 1.230000e+01, 1.250000e+01, 1.280000e+01 }, // SINR
          { 9.923077e-01, 5.936047e-01, 3.090000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 12, 1.230000e+01, 1.250000e+01, 1.280000e+01 }, // SINR
          { 1, 9.128521e-01, 2.969484e-01, 3.770000e-02, 8.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.225670e+01, 1.254860e+01, 1.284060e+01 }, // SINR
          { 9.980769e-01, 2.011218e-01, 3.800000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 19
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 1.136280e+01, 1.321120e+01, 1.505960e+01, 1.690810e+01, 1.875650e+01, 2.060500e+01 }, // SINR
          { 9.799618e-01, 8.287338e-01, 3.337731e-01, 2.550000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.321120e+01, 1.337710e+01, 1.357710e+01, 1.377710e+01, 1.397710e+01, 1.417710e+01, 1.437710e+01, 1.457710e+01, 1.477710e+01, 1.493580e+01, 1.497710e+01, 1.666050e+01, 1.838510e+01, 2.010980e+01 }, // SINR
          { 9.066901e-01, 8.792517e-01, 8.501656e-01, 8.054687e-01, 7.500000e-01, 6.991758e-01, 6.311881e-01, 5.738636e-01, 5.135542e-01, 4.471831e-01, 4.284512e-01, 4.040000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 1.157940e+01, 1.321120e+01, 1.484300e+01, 1.647480e+01, 1.810660e+01 }, // SINR
          { 9.855769e-01, 8.152516e-01, 2.774725e-01, 9.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 1.164130e+01, 1.321120e+01, 1.478110e+01, 1.635100e+01, 1.792090e+01 }, // SINR
          { 9.903846e-01, 7.205056e-01, 1.675532e-01, 2.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 1.165840e+01, 1.321120e+01, 1.465730e+01, 1.610340e+01, 1.754950e+01 }, // SINR
          { 9.000000e-01, 6.356250e-01, 1.071739e-01, 1.900000e-03, 0 } // BLER
        }
      },
      { 144U, // SINR and BLER for CBS 144
        NrEesmErrorModel::DoubleTuple{
          { 1.188890e+01, 1.321120e+01, 1.453350e+01, 1.585580e+01, 1.717810e+01, 1.850040e+01 }, // SINR
          { 9.640152e-01, 7.573529e-01, 2.294333e-01, 7.100000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.186290e+01, 1.321120e+01, 1.459540e+01, 1.597960e+01 }, // SINR
          { 9.000000e-01, 5.520386e-01, 3.790000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.187500e+01, 1.321120e+01, 1.447160e+01, 1.573200e+01, 1.699240e+01 }, // SINR
          { 9.000000e-01, 8.012422e-01, 1.953246e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.176080e+01, 1.321120e+01, 1.440970e+01, 1.560820e+01, 1.680670e+01 }, // SINR
          { 9.000000e-01, 7.655325e-01, 1.449192e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.213650e+01, 1.321120e+01, 1.428590e+01, 1.536060e+01, 1.643530e+01 }, // SINR
          { 9.932692e-01, 6.280340e-01, 9.000809e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.818702e-01, 4.789326e-01, 5.720000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.531250e-01, 4.203795e-01, 3.600000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.176080e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.000000e-01, 6.207524e-01, 8.620000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.494485e-01, 5.575658e-01, 5.720000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.884615e-01, 4.146242e-01, 2.110000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.534672e-01, 4.951550e-01, 2.950000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.855769e-01, 5.198980e-01, 1.810000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.289420e+01, 1.321120e+01, 1.352820e+01, 1.416210e+01 }, // SINR
          { 9.980769e-01, 2.352528e-01, 2.460000e-02, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.659091e-01, 2.961449e-01, 2.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.175000e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 1.736496e-01, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.192467e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 1.996417e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 12, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 1.962891e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.234605e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 2.423372e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.262500e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.219601e-01, 1.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.291160e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.308777e-01, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.212910e+01, 1.285050e+01, 1.321120e+01, 1.375230e+01 }, // SINR
          { 1, 8.844828e-01, 8.610000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 13, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 7.520000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.276513e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 8.558468e-02, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.274726e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 6.910000e-02, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.276257e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 5.110000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.269950e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.500000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.263580e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.370000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.267930e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 2.870000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.273250e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 2.375470e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.273630e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.172897e-01, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.279473e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.220760e-01, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.276870e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 6.710000e-02, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 20
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 1.280000e+01, 1.407000e+01, 1.470000e+01, 1.533000e+01 }, // SINR
          { 1, 5.105000e-01, 2.066201e-01, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.330000e+01, 1.365000e+01, 14, 1.435000e+01, 1.470000e+01, 1.505000e+01, 1.540000e+01, 1.575000e+01, 1.610000e+01 }, // SINR
          { 9.211957e-01, 7.104167e-01, 4.325601e-01, 1.855670e-01, 4.720000e-02, 2.750000e-02, 3.000000e-03, 7.000000e-04, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 13, 1.410000e+01, 1.470000e+01, 1.520000e+01 }, // SINR
          { 1, 2.753821e-01, 2.377358e-01, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 1.313810e+01, 1.471970e+01, 1.630130e+01, 1.788280e+01, 1.946440e+01 }, // SINR
          { 1, 4.323630e-01, 2.350000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 152U, // SINR and BLER for CBS 152
        NrEesmErrorModel::DoubleTuple{
          { 1.332380e+01, 1.471970e+01, 1.611560e+01, 1.751140e+01, 1.890730e+01 }, // SINR
          { 9.923077e-01, 3.684402e-01, 1.550000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.296120e+01, 1.471970e+01, 1.617750e+01, 1.763520e+01 }, // SINR
          { 9.000000e-01, 3.148148e-01, 4.600000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.205170e+01, 1.338570e+01, 1.471970e+01, 1.605370e+01, 1.738760e+01 }, // SINR
          { 1, 8.750000e-01, 5.496795e-01, 2.800000e-02, 1.000000e-04 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.296120e+01, 1.471970e+01, 1.599180e+01, 1.726380e+01 }, // SINR
          { 9.000000e-01, 4.024682e-01, 1.340000e-02, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.290625e+01, 1.471970e+01, 1.586800e+01, 1.701620e+01 }, // SINR
          { 9.000000e-01, 2.118465e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.267070e+01, 1.369520e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01, 1.779310e+01 }, // SINR
          { 1, 5.937500e-01, 3.871951e-01, 2.130000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.375000e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 2.216608e-01, 5.400000e-03, 0 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 1.390720e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 1.304404e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.267070e+01, 1.369520e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 1, 8.388158e-01, 2.663502e-01, 3.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.394465e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 1.004740e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.381250e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 1.247515e-01, 4.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 14, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 4.390000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 13, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 6.180000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.385850e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 6.090000e-02, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.350000e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 3.910000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.381830e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 3.110000e-02, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.350000e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 1.720000e-02, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.346970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 1.130000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.371970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 5.400000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.346970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.377050e+01, 1.377053e+01, 1.408690e+01, 1.440330e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 9.000000e-01, 6.888441e-01, 5.700673e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.379553e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 1.800000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.379938e+01, 1.379940e+01, 1.410620e+01, 1.441290e+01 }, // SINR
          { 9.951923e-01, 7.514451e-01, 6.464646e-01, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.384357e+01, 1.384360e+01, 1.413560e+01, 1.442770e+01, 1.471970e+01 }, // SINR
          { 9.990385e-01, 6.994536e-01, 5.818182e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.293460e+01, 1.387110e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 7.825758e-01, 5.308577e-01, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.270740e+01, 1.371970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 8.958333e-01, 6.188424e-01, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.270740e+01, 1.371970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 8.101563e-01, 4.323630e-01, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.373879e+01, 1.373880e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 8.624161e-01, 5.120968e-01, 0 } // BLER
        }
      },
      { 2664U, // SINR and BLER for CBS 2664
        NrEesmErrorModel::DoubleTuple{
          { 1.324060e+01, 1.374060e+01, 1.374063e+01, 1.471970e+01 }, // SINR
          { 9.636194e-01, 7.955975e-01, 3.684402e-01, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.324810e+01, 1.374810e+01, 1.374812e+01, 1.471970e+01 }, // SINR
          { 9.570896e-01, 8.445724e-01, 4.082792e-01, 1.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.270740e+01, 1.371970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 7.880435e-01, 3.148148e-01, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.371970e+01, 1.432600e+01, 1.432760e+01, 1.493230e+01 }, // SINR
          { 9.166667e-01, 3.100000e-03, 2.200000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 21
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 1.420000e+01, 1.450000e+01, 1.480000e+01, 1.510000e+01, 1.540000e+01, 1.570000e+01, 16, 1.630000e+01, 1.660000e+01 }, // SINR
          { 9.555556e-01, 8.784483e-01, 6.564103e-01, 3.521468e-01, 1.167672e-01, 2.020000e-02, 2.500000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 1.483000e+01, 1.512000e+01, 1.527000e+01, 1.541000e+01, 1.570000e+01 }, // SINR
          { 9.485294e-01, 9.474638e-01, 9.835391e-02, 2.380000e-02, 5.000000e-03, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 1.434000e+01, 1.452000e+01, 1.470000e+01, 1.488000e+01, 1.505000e+01, 1.523000e+01, 1.540000e+01, 1.558000e+01, 1.575000e+01, 1.593000e+01, 1.610000e+01, 1.628000e+01 }, // SINR
          { 9.527778e-01, 8.896552e-01, 8.409091e-01, 5.952103e-01, 3.484116e-01, 1.357875e-01, 3.910000e-02, 1.420000e-02, 2.800000e-03, 4.000000e-04, 1.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 1.460000e+01, 15, 1.540000e+01, 1.580000e+01 }, // SINR
          { 9.154930e-01, 5.960648e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 144U, // SINR and BLER for CBS 144
        NrEesmErrorModel::DoubleTuple{
          { 1.360000e+01, 1.433000e+01, 1.470000e+01, 1.507000e+01, 1.580000e+01 }, // SINR
          { 1, 8.906250e-01, 4.975490e-01, 1.370000e-02, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.313830e+01, 1.466810e+01, 1.619790e+01, 1.772770e+01, 1.925750e+01 }, // SINR
          { 1, 8.640940e-01, 1.400838e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.360000e+01, 1.433000e+01, 1.470000e+01, 1.507000e+01, 1.580000e+01 }, // SINR
          { 1, 9.713740e-01, 7.120787e-01, 3.190000e-02, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.350000e+01, 1.423000e+01, 1.460000e+01, 1.497000e+01, 1.570000e+01 }, // SINR
          { 1, 9.990385e-01, 7.033784e-01, 2.746746e-01, 0 } // BLER
        }
      },
      { 256U, // SINR and BLER for CBS 256
        NrEesmErrorModel::DoubleTuple{
          { 1.481245e+01, 1.619790e+01, 1.741820e+01, 1.863850e+01 }, // SINR
          { 9.000000e-01, 5.760000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.495327e+01, 1.619790e+01, 1.729440e+01, 1.839090e+01 }, // SINR
          { 9.000000e-01, 6.020000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.489070e+01, 1.619790e+01, 1.729440e+01, 1.839090e+01 }, // SINR
          { 9.000000e-01, 2.470000e-02, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.489070e+01, 1.619790e+01, 1.729440e+01, 1.839090e+01 }, // SINR
          { 9.000000e-01, 2.610000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 1.460000e+01, 1.480000e+01, 15, 1.520000e+01, 1.540000e+01, 1.560000e+01, 1.580000e+01, 16, 1.620000e+01, 1.640000e+01, 1.660000e+01 }, // SINR
          { 9.045139e-01, 7.960123e-01, 6.772487e-01, 4.733146e-01, 2.504950e-01, 1.067550e-01, 3.330000e-02, 1.140000e-02, 2.500000e-03, 6.000000e-04, 2.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.472460e+01, 1.619790e+01, 1.729440e+01, 1.839090e+01 }, // SINR
          { 9.000000e-01, 1.350000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.488842e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 1.080000e-02, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.475610e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 1.130000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.466400e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 4.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.463540e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.455000e+01, 1.480000e+01, 1.505000e+01, 1.530000e+01, 1.555000e+01, 1.580000e+01, 1.605000e+01, 1.630000e+01 }, // SINR
          { 9.865385e-01, 9.397810e-01, 7.858232e-01, 5.224490e-01, 2.231350e-01, 5.420000e-02, 1.030000e-02, 1.400000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.321220e+01, 1.457290e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 1, 8.362903e-01, 2.610000e-02, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.480000e+01, 15, 1.530000e+01, 1.580000e+01 }, // SINR
          { 1, 2.214411e-01, 2.088843e-01, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.467120e+01, 1.467123e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.406475e-01, 9.000000e-01, 2.500000e-02, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.471577e+01, 1.471580e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 8.093750e-01, 5.300000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.479165e+01, 1.479170e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 6.596154e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.413540e+01, 1.465100e+01, 1.499000e+01, 1.516670e+01, 1.534000e+01, 1.568230e+01 }, // SINR
          { 1, 9.980769e-01, 2.427326e-01, 1.258776e-01, 4.900000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.461335e+01, 1.461340e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 8.018868e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.380000e+01, 1.450000e+01, 15, 1.530000e+01, 1.550000e+01 }, // SINR
          { 1, 9.990385e-01, 5.049603e-01, 1.170000e-02, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 15, 1.560000e+01, 1.620000e+01 }, // SINR
          { 1, 8.362903e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.480000e+01, 1.513000e+01, 1.547000e+01, 1.580000e+01 }, // SINR
          { 1, 1.868499e-01, 1.000000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.475415e+01, 1.475420e+01, 1.619790e+01 }, // SINR
          { 9.000000e-01, 5.927419e-01, 1.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.485415e+01, 1.485420e+01, 1.619790e+01 }, // SINR
          { 9.000000e-01, 4.475524e-01, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 1.480000e+01, 15, 1.520000e+01, 1.560000e+01, 1.620000e+01 }, // SINR
          { 1, 9.961538e-01, 5.720721e-01, 1.169468e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.466220e+01, 1.466220e+01, 1.619790e+01 }, // SINR
          { 9.000000e-01, 6.567259e-01, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.460000e+01, 1.487000e+01, 15, 1.513000e+01, 1.540000e+01, 1.580000e+01 }, // SINR
          { 9.971154e-01, 8.503289e-01, 3.553922e-01, 8.670000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.457290e+01, 1.457290e+01, 1.511460e+01, 1.565620e+01, 1.619790e+01 }, // SINR
          { 9.903846e-01, 9.000000e-01, 4.453671e-01, 1.560945e-01, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.383380e+01, 1.501590e+01, 1.619800e+01, 1.738010e+01 }, // SINR
          { 9.913462e-01, 6.381250e-01, 1.400000e-03, 2.000000e-04 } // BLER
        }
      }
  },
  { // MCS 22
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 1.474380e+01, 1.523740e+01, 1.573110e+01, 1.671850e+01, 1.819950e+01 }, // SINR
          { 1, 8.200637e-01, 3.506181e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.555000e+01, 1.580000e+01, 1.605000e+01, 1.630000e+01 }, // SINR
          { 9.980769e-01, 9.338768e-01, 4.499113e-01, 4.240000e-02, 3.000000e-04 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 1.490000e+01, 1.540000e+01, 1.567500e+01, 1.595000e+01, 1.622500e+01 }, // SINR
          { 1, 8.932292e-01, 2.979412e-01, 8.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 160U, // SINR and BLER for CBS 160
        NrEesmErrorModel::DoubleTuple{
          { 1.532510e+01, 1.573590e+01, 1.614670e+01, 1.696820e+01, 1.820050e+01 }, // SINR
          { 9.567669e-01, 6.349010e-01, 9.478913e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 1.471480e+01, 1.532740e+01, 1.563370e+01, 1.594000e+01, 1.655260e+01, 1.747150e+01 }, // SINR
          { 1, 9.066901e-01, 4.349315e-01, 2.320442e-01, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 1.469540e+01, 1.530230e+01, 1.560580e+01, 1.590930e+01, 1.651620e+01 }, // SINR
          { 1, 9.237589e-01, 6.399254e-01, 2.460938e-01, 1.000000e-04 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.488900e+01, 1.545360e+01, 1.573590e+01, 1.601820e+01, 1.658280e+01 }, // SINR
          { 1, 9.049296e-01, 3.200377e-01, 1.685734e-01, 1.000000e-04 } // BLER
        }
      },
      { 256U, // SINR and BLER for CBS 256
        NrEesmErrorModel::DoubleTuple{
          { 1.491990e+01, 1.546390e+01, 1.573590e+01, 1.600790e+01, 1.655190e+01 }, // SINR
          { 1, 7.330508e-01, 1.717258e-01, 2.480000e-02, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.501200e+01, 1.552720e+01, 1.578480e+01, 1.604240e+01, 1.655760e+01 }, // SINR
          { 1, 4.538530e-01, 2.257194e-01, 5.200000e-03, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.495090e+01, 1.547420e+01, 1.573590e+01, 1.599760e+01, 1.652090e+01 }, // SINR
          { 1, 8.012422e-01, 3.193384e-01, 4.260000e-02, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.515620e+01, 1.565620e+01, 1.599790e+01, 1.633950e+01 }, // SINR
          { 1, 3.212025e-01, 4.900000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.481270e+01, 1.535440e+01, 1.562530e+01, 1.589620e+01, 1.643790e+01 }, // SINR
          { 1, 8.827055e-01, 3.931889e-01, 5.720000e-02, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.523870e+01, 1.551850e+01, 1.579830e+01, 1.635790e+01 }, // SINR
          { 9.836538e-01, 7.384393e-01, 2.781114e-01, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.485540e+01, 1.539150e+01, 1.565950e+01, 1.592750e+01, 1.646360e+01 }, // SINR
          { 1, 9.196429e-01, 7.687126e-01, 8.080000e-02, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.556470e+01, 1.592160e+01, 1.627850e+01, 1.663530e+01 }, // SINR
          { 9.614662e-01, 2.898509e-01, 4.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.558610e+01, 1.593940e+01, 1.629270e+01, 1.664600e+01 }, // SINR
          { 9.798077e-01, 3.994479e-01, 1.000000e-02, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.562260e+01, 1.596980e+01, 1.614340e+01, 1.631700e+01, 1.666420e+01 }, // SINR
          { 9.102113e-01, 6.153382e-01, 2.390000e-02, 9.800000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.562260e+01, 1.583090e+01, 1.603920e+01, 1.645590e+01 }, // SINR
          { 9.500000e-01, 2.849099e-01, 3.280000e-02, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.545590e+01, 1.579990e+01, 1.614400e+01, 1.648800e+01, 1.752000e+01 }, // SINR
          { 9.980769e-01, 6.769737e-01, 1.200000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.532570e+01, 1.564900e+01, 1.582570e+01, 1.597240e+01, 1.629570e+01, 1.676580e+01 }, // SINR
          { 1, 6.834677e-01, 5.267490e-01, 1.000626e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.564740e+01, 1.573630e+01, 1.582530e+01, 1.591420e+01, 1.641420e+01 }, // SINR
          { 9.828244e-01, 8.965517e-01, 7.134831e-01, 4.685185e-01, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.559650e+01, 1.580740e+01, 1.601830e+01, 1.644020e+01 }, // SINR
          { 9.961538e-01, 4.340753e-01, 1.121474e-01, 1.000000e-04 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.533920e+01, 1.573370e+01, 1.612810e+01, 1.652260e+01 }, // SINR
          { 1, 8.767123e-01, 4.050000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.533090e+01, 1.572670e+01, 1.612260e+01, 1.651840e+01 }, // SINR
          { 1, 8.649329e-01, 2.600000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.545590e+01, 1.583090e+01, 1.620590e+01, 1.658090e+01 }, // SINR
          { 9.980769e-01, 4.165296e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.587260e+01, 1.603920e+01, 1.620590e+01, 1.637260e+01, 1.687260e+01 }, // SINR
          { 9.522059e-01, 5.888761e-01, 1.596958e-01, 1.520000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.577400e+01, 1.606940e+01, 1.621720e+01, 1.636490e+01 }, // SINR
          { 9.589552e-01, 9.100000e-03, 7.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.557500e+01, 1.585000e+01, 1.612500e+01, 1.640000e+01 }, // SINR
          { 9.923077e-01, 8.301282e-01, 2.763158e-01, 1.600000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.556730e+01, 1.584300e+01, 1.611880e+01, 1.639450e+01 }, // SINR
          { 9.846154e-01, 7.463873e-01, 6.286946e-01, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.557500e+01, 1.585000e+01, 1.612500e+01, 1.640000e+01 }, // SINR
          { 9.761450e-01, 6.709845e-01, 1.121307e-01, 2.400000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.540000e+01, 1.575000e+01, 1.610000e+01, 1.645000e+01 }, // SINR
          { 9.696970e-01, 3.557961e-01, 5.900000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.470000e+01, 1.553000e+01, 1.590000e+01, 1.637000e+01, 1.720000e+01 }, // SINR
          { 1, 7.385057e-01, 6.586788e-01, 1.350000e-02, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 15, 1.560000e+01, 1.590000e+01, 1.620000e+01, 1.680000e+01 }, // SINR
          { 1, 7.595588e-01, 3.525910e-01, 6.650000e-02, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.510000e+01, 1.553000e+01, 1.597000e+01, 1.640000e+01 }, // SINR
          { 1, 8.657718e-01, 2.328704e-01, 0 } // BLER
        }
      }
  },
  { // MCS 23
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.627230e+01, 1.652500e+01, 1.677780e+01, 1.703050e+01, 1.728320e+01, 1.778320e+01 }, // SINR
          { 9.971154e-01, 9.293478e-01, 4.697955e-01, 6.130000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 1.640000e+01, 1.665000e+01, 1.690000e+01, 1.715000e+01, 1.740000e+01 }, // SINR
          { 9.990385e-01, 8.306452e-01, 2.058824e-01, 8.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 1.620000e+01, 1.650000e+01, 1.680000e+01, 1.710000e+01, 1.740000e+01, 1.770000e+01, 18 }, // SINR
          { 1, 5.497835e-01, 5.456009e-01, 2.439438e-01, 6.370000e-02, 1.060000e-02, 1.000000e-03 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.606000e+01, 1.633000e+01, 1.660000e+01, 1.690000e+01, 1.720000e+01, 1.740000e+01, 1.770000e+01, 1.797000e+01, 1.825000e+01 }, // SINR
          { 1, 8.741554e-01, 6.809211e-01, 3.365385e-01, 1.029657e-01, 3.500000e-02, 4.800000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.665580e+01, 1.690580e+01, 1.715580e+01, 1.740580e+01, 1.765580e+01 }, // SINR
          { 9.501812e-01, 4.810606e-01, 5.850000e-02, 1.300000e-03, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 24
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 1.680000e+01, 1.730000e+01, 1.763000e+01, 1.780000e+01, 1.797000e+01, 1.830000e+01, 1.880000e+01 }, // SINR
          { 1, 9.402174e-01, 2.945804e-01, 2.178879e-01, 1.860000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 1.758985e+01, 2.090370e+01, 2.266300e+01 }, // SINR
          { 9.000000e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.759430e+01, 2.090370e+01, 2.247730e+01 }, // SINR
          { 9.000000e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.758985e+01, 2.090370e+01, 2.235350e+01 }, // SINR
          { 9.000000e-01, 1.200000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 25
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 26
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 1.962500e+01, 2.025000e+01, 2.087500e+01, 2.150000e+01, 2.212500e+01, 2.275000e+01 }, // SINR
          { 9.788462e-01, 7.544118e-01, 2.986936e-01, 3.420000e-02, 1.600000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 27
      { 152U, // SINR and BLER for CBS 152
        NrEesmErrorModel::DoubleTuple{
          { 2.050000e+01, 21, 2.150000e+01, 22, 2.250000e+01, 23 }, // SINR
          { 9.961538e-01, 9.352518e-01, 7.111111e-01, 3.123457e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 2.020000e+01, 2.045000e+01, 2.070000e+01, 2.095000e+01, 2.120000e+01, 2.145000e+01 }, // SINR
          { 9.990385e-01, 9.884615e-01, 8.366013e-01, 3.825758e-01, 6.320000e-02, 0 } // BLER
        }
      },
      { 256U, // SINR and BLER for CBS 256
        NrEesmErrorModel::DoubleTuple{
          { 2.060000e+01, 2.087500e+01, 2.115000e+01, 2.142500e+01, 2.170000e+01 }, // SINR
          { 9.951923e-01, 8.566667e-01, 3.010143e-01, 2.620000e-02, 7.000000e-04 } // BLER
        }
      }
  }
}
};

/**
 * \brief SINR to BLER mapping for MCSs in Table2
 */
static const NrEesmErrorModel::SimulatedBlerFromSINR BlerForSinr2 = {
{ // BG TYPE 1
  { // MCS 0
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 1
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 2
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 5.300000e-01, 1.100000e+00, 1.680000e+00, 2.250000e+00, 2.830000e+00, 3.400000e+00 }, // SINR
          { 9.817308e-01, 8.835616e-01, 5.481602e-01, 1.601911e-01, 1.900000e-02, 1.000000e-03 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 5.300000e-01, 1.100000e+00, 1.680000e+00, 2.250000e+00, 2.830000e+00, 3.400000e+00 }, // SINR
          { 9.855769e-01, 8.470395e-01, 4.655331e-01, 1.021483e-01, 7.300000e-03, 5.000000e-04 } // BLER
        }
      },
      { 4224U, // SINR and BLER for CBS 4224
        NrEesmErrorModel::DoubleTuple{
          { 8.300000e-01, 1.200000e+00, 1.580000e+00, 1.950000e+00, 2.330000e+00, 2.700000e+00, 3.080000e+00, 3.450000e+00, 3.830000e+00 }, // SINR
          { 9.393116e-01, 8.392857e-01, 5.337553e-01, 2.614108e-01, 7.540000e-02, 1.220000e-02, 2.200000e-03, 6.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 8.300000e-01, 1.200000e+00, 1.580000e+00, 1.950000e+00, 2.330000e+00, 2.700000e+00, 3.080000e+00, 3.450000e+00 }, // SINR
          { 9.661654e-01, 8.924825e-01, 6.649485e-01, 3.439208e-01, 1.132671e-01, 2.410000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.150000e+00, 1.400000e+00, 1.650000e+00, 1.900000e+00, 2.200000e+00, 2.400000e+00, 2.700000e+00, 2.900000e+00 }, // SINR
          { 9.163732e-01, 8.486842e-01, 6.861702e-01, 1.869420e-01, 3.660000e-02, 9.000000e-03, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 9.100000e-01, 1.240000e+00, 1.570000e+00, 1.900000e+00, 2.200000e+00, 2.600000e+00, 2.900000e+00 }, // SINR
          { 9.807692e-01, 8.715986e-01, 6.649485e-01, 1.657359e-01, 2.900000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 6144U, // SINR and BLER for CBS 6144
        NrEesmErrorModel::DoubleTuple{
          { 1.100000e+00, 1.400000e+00, 1.700000e+00, 2, 2.300000e+00, 2.600000e+00, 2.900000e+00, 3.200000e+00 }, // SINR
          { 9.216549e-01, 7.067308e-01, 4.635036e-01, 2.830717e-01, 5.900000e-02, 4.600000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.400000e+00, 1.600000e+00, 1.900000e+00, 2.200000e+00, 2.500000e+00, 2.780000e+00, 3.050000e+00 }, // SINR
          { 9.498175e-01, 8.081761e-01, 3.753698e-01, 8.440000e-02, 7.700000e-03, 1.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 7168U, // SINR and BLER for CBS 7168
        NrEesmErrorModel::DoubleTuple{
          { 1.200000e+00, 1.400000e+00, 1.600000e+00, 1.700000e+00, 1.900000e+00, 2.070000e+00, 2.250000e+00, 2.430000e+00, 2.600000e+00, 2.780000e+00, 2.950000e+00, 3.130000e+00, 3.300000e+00 }, // SINR
          { 9.799618e-01, 9.084507e-01, 7.183989e-01, 6.020047e-01, 3.095966e-01, 1.206731e-01, 5.460000e-02, 2.400000e-02, 7.700000e-03, 3.000000e-03, 5.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 8064U, // SINR and BLER for CBS 8064
        NrEesmErrorModel::DoubleTuple{
          { 8.000000e-01, 1.500000e+00, 2.300000e+00, 3.100000e+00 }, // SINR
          { 1, 8.741497e-01, 2.540000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 3
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 2.750000e+00, 3, 3.300000e+00, 3.500000e+00, 3.800000e+00, 4, 4.250000e+00 }, // SINR
          { 9.680451e-01, 8.205128e-01, 5.226337e-01, 2.182642e-01, 7.820000e-02, 1.060000e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.750000e+00, 3, 3.300000e+00, 3.500000e+00, 3.800000e+00, 4, 4.250000e+00, 4.500000e+00 }, // SINR
          { 9.080357e-01, 7.559172e-01, 4.187294e-01, 2.097245e-01, 4.690000e-02, 1.190000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 2.680000e+00, 2.900000e+00, 3.100000e+00, 3.300000e+00, 3.600000e+00, 3.800000e+00, 4.020000e+00, 4.250000e+00 }, // SINR
          { 9.452555e-01, 6.746032e-01, 4.022082e-01, 1.738227e-01, 3.040000e-02, 5.600000e-03, 1.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 2.730000e+00, 3, 3.300000e+00, 3.500000e+00, 3.800000e+00, 4.100000e+00 }, // SINR
          { 9.258929e-01, 6.553030e-01, 2.448930e-01, 7.830000e-02, 6.400000e-03, 3.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 2.900000e+00, 3.300000e+00, 3.600000e+00, 4, 4.380000e+00 }, // SINR
          { 9.855769e-01, 8.179688e-01, 3.088235e-01, 6.250000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.100000e+00, 2.700000e+00, 3.300000e+00, 3.900000e+00, 4.500000e+00, 5.100000e+00, 5.700000e+00 }, // SINR
          { 9.761450e-01, 8.741611e-01, 6.035714e-01, 2.560729e-01, 4.680000e-02, 3.600000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 2.830000e+00, 3.100000e+00, 3.400000e+00, 3.600000e+00, 3.900000e+00, 4.200000e+00 }, // SINR
          { 9.583333e-01, 6.088517e-01, 2.043619e-01, 5.810000e-02, 5.200000e-03, 2.000000e-04 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.880000e+00, 3.100000e+00, 3.300000e+00, 3.500000e+00, 3.800000e+00, 4, 4.230000e+00 }, // SINR
          { 9.289568e-01, 8.217742e-01, 5.224490e-01, 2.432171e-01, 2.660000e-02, 3.500000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.800000e+00, 3.100000e+00, 3.400000e+00, 3.700000e+00, 4, 4.300000e+00 }, // SINR
          { 9.708647e-01, 7.233146e-01, 2.968023e-01, 3.880000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.800000e+00, 3, 3.200000e+00, 3.400000e+00, 3.600000e+00, 3.800000e+00, 4, 4.200000e+00 }, // SINR
          { 9.826923e-01, 8.319805e-01, 5.309917e-01, 2.024960e-01, 4.530000e-02, 6.100000e-03, 2.000000e-04, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 4
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.460000e+00, 4.500000e+00, 4.600000e+00, 4.700000e+00, 4.720000e+00, 4.800000e+00, 4.900000e+00, 4.980000e+00, 5, 5.240000e+00, 5.500000e+00 }, // SINR
          { 9.546296e-01, 9.196429e-01, 8.665541e-01, 7.060811e-01, 4.852099e-01, 4.332192e-01, 2.706103e-01, 1.241362e-01, 5.300000e-02, 4.200000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 4.258500e+00, 4.509200e+00, 4.760000e+00, 5.010800e+00, 5.261500e+00 }, // SINR
          { 9.903846e-01, 8.172468e-01, 2.485149e-01, 1.330000e-02, 2.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 4.100000e+00, 4.700000e+00, 5.300000e+00, 5.900000e+00 }, // SINR
          { 9.178571e-01, 2.702991e-01, 4.700000e-03, 0 } // BLER
        }
      },
      { 4608U, // SINR and BLER for CBS 4608
        NrEesmErrorModel::DoubleTuple{
          { 3.800000e+00, 4.400000e+00, 4.700000e+00, 5, 5.600000e+00 }, // SINR
          { 1, 6.477273e-01, 3.983386e-01, 5.580000e-02, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 4.300000e+00, 4.600000e+00, 4.800000e+00, 5.100000e+00, 5.300000e+00 }, // SINR
          { 9.990385e-01, 7.965839e-01, 2.694149e-01, 4.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 3.850000e+00, 4.200000e+00, 4.550000e+00, 4.900000e+00, 5.200000e+00, 5.600000e+00 }, // SINR
          { 1, 8.809122e-01, 4.163934e-01, 1.534714e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 4, 4.500000e+00, 5, 5.500000e+00 }, // SINR
          { 9.884615e-01, 6.898396e-01, 2.110000e-02, 0 } // BLER
        }
      },
      { 6528U, // SINR and BLER for CBS 6528
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.680000e+00, 4.950000e+00, 5.230000e+00, 5.500000e+00 }, // SINR
          { 9.411232e-01, 6.417910e-01, 2.137712e-01, 2.810000e-02, 8.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 3.800000e+00, 4.200000e+00, 4.600000e+00, 5, 5.400000e+00 }, // SINR
          { 1, 9.865385e-01, 6.458333e-01, 7.860000e-02, 4.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 4.005500e+00, 4.540600e+00, 4.808200e+00, 5.610900e+00 }, // SINR
          { 9.734848e-01, 7.611607e-01, 3.080900e-01, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 5
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 6.033800e+00, 6.332800e+00, 6.631900e+00, 6.930900e+00, 7.230000e+00, 7.730000e+00 }, // SINR
          { 9.932692e-01, 9.151786e-01, 6.105769e-01, 1.890106e-01, 2.160000e-02, 4.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 6.016500e+00, 6.516500e+00, 6.704900e+00, 6.893300e+00, 7.081600e+00, 7.270000e+00, 7.770000e+00 }, // SINR
          { 9.685115e-01, 6.123188e-01, 3.143657e-01, 1.031513e-01, 2.270000e-02, 3.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 5.823100e+00, 6.323100e+00, 6.565400e+00, 6.807700e+00, 7.050000e+00, 7.292300e+00, 7.792300e+00 }, // SINR
          { 9.980769e-01, 8.368506e-01, 5.064741e-01, 1.574248e-01, 2.500000e-02, 2.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 6.363100e+00, 6.655000e+00, 6.946800e+00, 7.238600e+00, 7.530500e+00 }, // SINR
          { 9.837786e-01, 7.942547e-01, 2.870475e-01, 2.550000e-02, 7.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 5.450000e+00, 6.100000e+00, 6.800000e+00, 7.400000e+00, 8.100000e+00, 8.700000e+00 }, // SINR
          { 9.894231e-01, 8.916084e-01, 5.235656e-01, 1.541054e-01, 1.080000e-02, 4.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 6.390000e+00, 6.605800e+00, 6.821600e+00, 7.037400e+00, 7.253200e+00, 7.753200e+00 }, // SINR
          { 9.208633e-01, 6.129227e-01, 2.087500e-01, 2.340000e-02, 1.100000e-03, 2.000000e-04 } // BLER
        }
      },
      { 5760U, // SINR and BLER for CBS 5760
        NrEesmErrorModel::DoubleTuple{
          { 6.300000e+00, 6.800000e+00, 7.300000e+00, 7.800000e+00 }, // SINR
          { 9.923077e-01, 4.146242e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 6.134900e+00, 6.634900e+00, 6.784900e+00, 6.934900e+00, 7.084900e+00, 7.234900e+00, 7.734900e+00 }, // SINR
          { 9.592593e-01, 6.854839e-01, 3.533520e-01, 1.082677e-01, 2.040000e-02, 1.900000e-03, 3.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 6.139500e+00, 6.593000e+00, 7.046400e+00, 7.499800e+00 }, // SINR
          { 9.990385e-01, 7.514620e-01, 2.190000e-02, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 6.163800e+00, 6.511000e+00, 6.858200e+00, 7.205300e+00, 7.552500e+00 }, // SINR
          { 9.990385e-01, 9.402174e-01, 3.798799e-01, 7.300000e-03, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 6
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 6.200000e+00, 7.200000e+00, 8.200000e+00, 9.200000e+00, 1.020000e+01 }, // SINR
          { 9.980769e-01, 8.750000e-01, 1.701624e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 4032U, // SINR and BLER for CBS 4032
        NrEesmErrorModel::DoubleTuple{
          { 6.769200e+00, 7.269200e+00, 7.520500e+00, 7.771800e+00, 8.023100e+00, 8.274400e+00 }, // SINR
          { 9.884615e-01, 6.449005e-01, 2.555894e-01, 3.990000e-02, 2.700000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 6.821000e+00, 7.321000e+00, 7.563500e+00, 7.806100e+00, 8.048700e+00, 8.291200e+00 }, // SINR
          { 9.420956e-01, 5.694444e-01, 1.807554e-01, 2.580000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 6.855400e+00, 7.355400e+00, 7.530800e+00, 7.706200e+00, 7.881600e+00, 8.057000e+00 }, // SINR
          { 9.531250e-01, 5.294421e-01, 2.390684e-01, 7.280000e-02, 1.110000e-02, 9.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 6.872300e+00, 7.372300e+00, 7.653700e+00, 7.935100e+00, 8.216400e+00, 8.497800e+00 }, // SINR
          { 9.476103e-01, 8.404605e-01, 4.063505e-01, 7.710000e-02, 4.700000e-03, 2.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 6.871800e+00, 7.371800e+00, 7.588200e+00, 7.804600e+00, 8.021000e+00, 8.237400e+00, 8.737400e+00 }, // SINR
          { 9.420290e-01, 8.801020e-01, 5.578603e-01, 2.018000e-01, 3.090000e-02, 2.500000e-03, 3.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 6.800000e+00, 7.300000e+00, 7.800000e+00, 8.300000e+00, 8.800000e+00 }, // SINR
          { 1, 9.425182e-01, 2.038835e-01, 1.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 6656U, // SINR and BLER for CBS 6656
        NrEesmErrorModel::DoubleTuple{
          { 7.004100e+00, 7.271600e+00, 7.539000e+00, 7.806400e+00, 8.073900e+00, 8.573900e+00 }, // SINR
          { 9.903846e-01, 8.758503e-01, 4.907946e-01, 8.670000e-02, 3.900000e-03, 8.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 7.036100e+00, 7.472400e+00, 7.908800e+00, 8.345100e+00 }, // SINR
          { 9.807692e-01, 4.678309e-01, 1.050000e-02, 1.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 7.156200e+00, 7.509400e+00, 7.686100e+00, 7.862700e+00, 8.215900e+00 }, // SINR
          { 9.542910e-01, 3.047445e-01, 1.328125e-01, 4.780000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 7
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 7.702700e+00, 8.202700e+00, 8.450300e+00, 8.697800e+00, 8.945400e+00 }, // SINR
          { 1, 8.583333e-01, 2.439202e-01, 8.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 7.725800e+00, 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 1, 9.049296e-01, 2.573980e-01, 4.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 7.725800e+00, 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 1, 9.330357e-01, 2.788462e-01, 4.400000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.601852e-01, 3.724112e-01, 9.500000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.884615e-01, 5.009766e-01, 1.830000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.675573e-01, 4.080645e-01, 6.800000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.855769e-01, 4.761236e-01, 1.010000e-02, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.363700e+00, 8.682600e+00, 9.001500e+00 }, // SINR
          { 9.932692e-01, 4.932692e-01, 8.000000e-03, 0 } // BLER
        }
      },
      { 7552U, // SINR and BLER for CBS 7552
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.299867e+00, 8.363700e+00, 8.576300e+00, 8.788900e+00 }, // SINR
          { 9.951923e-01, 5.369198e-01, 1.240109e-01, 8.300000e-03, 9.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 8.044700e+00, 8.314176e+00, 8.363700e+00, 8.576300e+00, 8.788900e+00 }, // SINR
          { 9.932692e-01, 4.129902e-01, 1.234006e-01, 5.000000e-03, 7.000000e-04 } // BLER
        }
      }
  },
  { // MCS 8
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.649621e-01, 5.136089e-01, 3.570000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.611742e-01, 4.551971e-01, 2.220000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.740385e-01, 4.454225e-01, 1.860000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.790076e-01, 4.283898e-01, 1.420000e-02, 1.000000e-04 } // BLER
        }
      },
      { 5120U, // SINR and BLER for CBS 5120
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.894231e-01, 6.838235e-01, 4.660000e-02, 2.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.855769e-01, 5.720721e-01, 3.410000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.923077e-01, 5.814220e-01, 2.400000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.923077e-01, 5.432692e-01, 1.610000e-02, 1.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.971154e-01, 7.067039e-01, 3.580000e-02, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 8.843500e+00, 9.157600e+00, 9.471700e+00, 9.785800e+00 }, // SINR
          { 9.932692e-01, 5.726351e-01, 1.340000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 9
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.951923e-01, 8.792808e-01, 3.351064e-01, 2.140000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.923077e-01, 7.150838e-01, 1.250000e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.951923e-01, 7.412281e-01, 1.118772e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.971154e-01, 7.333815e-01, 1.081878e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.942308e-01, 7.641369e-01, 9.903133e-02, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 9.980769e-01, 7.097222e-01, 7.160000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 5632U, // SINR and BLER for CBS 5632
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01 }, // SINR
          { 9.980769e-01, 6.536990e-01, 4.360000e-02, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 1, 9.357143e-01, 3.420516e-01, 7.800000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01, 1.088890e+01 }, // SINR
          { 1, 9.397482e-01, 2.602459e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 9.651900e+00, 9.961200e+00, 1.027040e+01, 1.057970e+01 }, // SINR
          { 1, 8.325321e-01, 9.162572e-02, 0 } // BLER
        }
      }
  },
  { // MCS 10
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.509259e-01, 5.225410e-01, 5.610000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01 }, // SINR
          { 9.148936e-01, 3.670520e-01, 2.060000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.555556e-01, 4.508929e-01, 3.290000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.028680e+01, 1.052850e+01, 1.077030e+01, 1.101200e+01, 1.151200e+01 }, // SINR
          { 9.809160e-01, 8.344156e-01, 3.935759e-01, 5.610000e-02, 8.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.638889e-01, 4.513393e-01, 2.200000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01 }, // SINR
          { 9.780534e-01, 4.980315e-01, 2.850000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6144U, // SINR and BLER for CBS 6144
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.990385e-01, 7.993827e-01, 1.207892e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.913462e-01, 6.949728e-01, 7.500000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070600e+01, 1.101200e+01, 1.131800e+01, 1.162400e+01 }, // SINR
          { 9.961538e-01, 7.271429e-01, 6.450000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.033890e+01, 1.056330e+01, 1.078760e+01, 1.101200e+01, 1.151200e+01 }, // SINR
          { 9.608209e-01, 6.614583e-01, 1.791311e-01, 1.450000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 11
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.196360e+01, 1.219070e+01, 1.241790e+01, 1.264510e+01 }, // SINR
          { 9.828244e-01, 5.774887e-01, 6.630000e-02, 7.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.155000e+01, 1.180000e+01, 1.210000e+01, 1.230000e+01, 1.260000e+01, 1.280000e+01, 1.305000e+01 }, // SINR
          { 1, 6.401515e-01, 2.695815e-01, 1.036765e-01, 1.240000e-02, 2.100000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.196360e+01, 1.219070e+01, 1.241790e+01, 1.264510e+01, 1.314510e+01 }, // SINR
          { 9.865385e-01, 6.708115e-01, 9.662327e-02, 2.400000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.196360e+01, 1.219070e+01, 1.241790e+01, 1.264510e+01, 1.314510e+01 }, // SINR
          { 9.932692e-01, 6.606218e-01, 9.226430e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.173000e+01, 12, 1.230000e+01, 1.260000e+01, 1.280000e+01, 1.310000e+01 }, // SINR
          { 1, 3.327836e-01, 6.850000e-02, 7.800000e-03, 9.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.196360e+01, 1.219070e+01, 1.241790e+01, 1.264510e+01 }, // SINR
          { 9.763258e-01, 5.786199e-01, 4.330000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.180220e+01, 1.215040e+01, 1.249860e+01, 1.284690e+01 }, // SINR
          { 1, 5.193878e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 6400U, // SINR and BLER for CBS 6400
        NrEesmErrorModel::DoubleTuple{
          { 1.181210e+01, 1.219080e+01, 1.256940e+01, 1.294800e+01 }, // SINR
          { 1, 6.562500e-01, 4.900000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.163000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.280000e+01, 13 }, // SINR
          { 1, 6.497462e-01, 1.832117e-01, 1.580000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.188790e+01, 1.211510e+01, 1.234220e+01, 1.256940e+01, 1.279660e+01 }, // SINR
          { 9.990385e-01, 8.550000e-01, 1.733815e-01, 2.300000e-03, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 12
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.923077e-01, 8.320064e-01, 3.105037e-01, 2.410000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.865385e-01, 7.962963e-01, 2.366822e-01, 1.170000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.951923e-01, 8.261218e-01, 2.475394e-01, 1.000000e-02, 3.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.942308e-01, 8.409091e-01, 2.485207e-01, 9.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.990385e-01, 8.649329e-01, 2.715517e-01, 1.000000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.990385e-01, 8.128981e-01, 1.842009e-01, 3.800000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 1, 8.707770e-01, 1.973787e-01, 3.600000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 1, 8.279221e-01, 1.474736e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 7168U, // SINR and BLER for CBS 7168
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.990385e-01, 8.750000e-01, 2.142249e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.249360e+01, 1.279060e+01, 1.308760e+01, 1.338460e+01, 1.368150e+01 }, // SINR
          { 9.990385e-01, 8.977273e-01, 2.119932e-01, 2.500000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 13
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.340000e+01, 1.350000e+01, 1.360000e+01, 1.370000e+01, 1.380000e+01, 1.383010e+01, 1.390000e+01, 14, 1.410000e+01, 1.412130e+01, 1.420000e+01, 1.430000e+01, 1.440000e+01, 1.441250e+01, 1.450000e+01, 1.470370e+01 }, // SINR
          { 9.809160e-01, 9.479927e-01, 8.783784e-01, 7.603550e-01, 5.992991e-01, 5.452128e-01, 4.172131e-01, 2.573980e-01, 1.256269e-01, 1.055867e-01, 5.050000e-02, 1.780000e-02, 4.400000e-03, 4.400000e-03, 1.200000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 9.884615e-01, 6.268473e-01, 1.351351e-01, 3.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01, 1.499490e+01 }, // SINR
          { 9.198944e-01, 4.584838e-01, 5.700000e-02, 1.200000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.278649e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 9.000000e-01, 6.213415e-01, 9.919488e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 9.598881e-01, 5.080645e-01, 4.590000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.324770e+01, 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 1, 8.870690e-01, 5.372340e-01, 5.750000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.353890e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01, 1.470370e+01 }, // SINR
          { 9.715909e-01, 5.280083e-01, 4.100000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.270000e+01, 1.310000e+01, 1.350000e+01, 14, 1.440000e+01, 1.483000e+01 }, // SINR
          { 1, 9.971154e-01, 9.361314e-01, 6.772487e-01, 3.478709e-01, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.261860e+01, 1.383010e+01, 1.412130e+01, 1.441250e+01 }, // SINR
          { 9.000000e-01, 6.795213e-01, 5.610000e-02, 0 } // BLER
        }
      },
      { 7808U, // SINR and BLER for CBS 7808
        NrEesmErrorModel::DoubleTuple{
          { 1.270000e+01, 1.310000e+01, 1.360000e+01, 14, 1.440000e+01, 1.483000e+01 }, // SINR
          { 9.942308e-01, 9.642857e-01, 7.529240e-01, 3.960938e-01, 1.133590e-01, 0 } // BLER
        }
      }
  },
  { // MCS 14
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.480000e+01, 1.530000e+01, 1.570000e+01, 1.620000e+01 }, // SINR
          { 9.447464e-01, 5.788288e-01, 8.994762e-02, 3.400000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.450000e+01, 1.490000e+01, 1.520000e+01, 1.560000e+01, 1.590000e+01 }, // SINR
          { 9.343525e-01, 5.669643e-01, 2.064860e-01, 1.400000e-02, 8.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.415000e+01, 1.450000e+01, 1.490000e+01, 1.520000e+01, 1.560000e+01, 1.590000e+01 }, // SINR
          { 1, 8.775510e-01, 4.340986e-01, 1.132777e-01, 3.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.410000e+01, 1.460000e+01, 15, 1.550000e+01, 1.590000e+01 }, // SINR
          { 1, 9.038462e-01, 4.228188e-01, 1.840000e-02, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.480000e+01, 1.530000e+01, 1.590000e+01, 1.640000e+01 }, // SINR
          { 1, 8.567881e-01, 1.193994e-01, 1.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.460000e+01, 1.490000e+01, 1.520000e+01, 1.550000e+01, 1.580000e+01, 1.610000e+01, 1.640000e+01, 1.670000e+01, 17, 1.730000e+01 }, // SINR
          { 9.262590e-01, 7.960123e-01, 5.221193e-01, 2.329020e-01, 7.060000e-02, 2.250000e-02, 4.700000e-03, 8.000000e-04, 4.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 14, 1.450000e+01, 15, 1.540000e+01, 1.590000e+01 }, // SINR
          { 9.980769e-01, 8.121069e-01, 8.309944e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.470000e+01, 1.510000e+01, 1.550000e+01, 1.590000e+01 }, // SINR
          { 9.730769e-01, 4.922481e-01, 2.390000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 14, 1.460000e+01, 1.520000e+01, 1.580000e+01 }, // SINR
          { 1, 8.948276e-01, 6.370000e-02, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.420000e+01, 1.460000e+01, 1.510000e+01, 1.550000e+01, 1.590000e+01 }, // SINR
          { 9.980769e-01, 8.836207e-01, 9.797297e-02, 6.000000e-04, 2.000000e-04 } // BLER
        }
      }
  },
  { // MCS 15
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.560000e+01, 1.583000e+01, 1.590000e+01, 1.607000e+01, 1.630000e+01, 1.660000e+01 }, // SINR
          { 9.531250e-01, 7.747006e-01, 2.305759e-01, 5.920000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.460000e+01, 1.527000e+01, 1.560000e+01, 1.593000e+01, 1.660000e+01 }, // SINR
          { 1, 9.836538e-01, 8.819444e-01, 1.929724e-01, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.510000e+01, 1.545000e+01, 1.580000e+01, 1.615000e+01, 1.650000e+01, 1.685000e+01, 1.720000e+01, 1.755000e+01, 1.790000e+01, 1.825000e+01 }, // SINR
          { 9.894231e-01, 9.415468e-01, 8.144904e-01, 5.516304e-01, 2.818792e-01, 1.168785e-01, 3.300000e-02, 5.600000e-03, 9.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.563000e+01, 1.580000e+01, 1.597000e+01, 1.630000e+01, 1.680000e+01 }, // SINR
          { 1, 6.557692e-01, 5.404412e-01, 3.468407e-01, 1.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.530000e+01, 1.563000e+01, 1.597000e+01, 1.630000e+01 }, // SINR
          { 1, 9.961538e-01, 5.145582e-01, 5.850000e-02, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.563000e+01, 1.580000e+01, 1.597000e+01, 1.630000e+01, 1.680000e+01 }, // SINR
          { 1, 8.200637e-01, 7.406069e-01, 4.917954e-01, 8.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.563000e+01, 1.597000e+01, 1.630000e+01 }, // SINR
          { 1, 3.974763e-01, 2.870000e-02, 1.000000e-04 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.540000e+01, 1.573000e+01, 1.590000e+01, 1.607000e+01, 1.640000e+01, 17 }, // SINR
          { 1, 7.973602e-01, 6.838235e-01, 4.563849e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.540000e+01, 1.570000e+01, 1.580000e+01, 16, 1.630000e+01, 1.670000e+01 }, // SINR
          { 1, 8.558333e-01, 6.668814e-01, 5.600437e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 1.540000e+01, 1.573000e+01, 1.607000e+01, 1.640000e+01 }, // SINR
          { 1, 9.990385e-01, 5.910138e-01, 6.320000e-02, 5.000000e-04 } // BLER
        }
      }
  },
  { // MCS 16
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.580000e+01, 1.710000e+01, 1.840000e+01, 1.970000e+01 }, // SINR
          { 9.687500e-01, 5.009843e-01, 2.280000e-02, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.590000e+01, 1.690000e+01, 1.790000e+01, 1.890000e+01, 1.990000e+01 }, // SINR
          { 9.780534e-01, 6.064593e-01, 9.153226e-02, 2.000000e-03, 0 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 1.560000e+01, 1.690000e+01, 1.820000e+01, 1.950000e+01 }, // SINR
          { 9.961538e-01, 4.575812e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 1.520000e+01, 1.630000e+01, 1.740000e+01, 1.860000e+01, 1.970000e+01 }, // SINR
          { 9.980769e-01, 8.750000e-01, 1.662269e-01, 8.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.540000e+01, 1.650000e+01, 1.750000e+01, 1.850000e+01, 1.960000e+01 }, // SINR
          { 9.980769e-01, 6.801862e-01, 5.830000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 1.640000e+01, 1.740000e+01, 1.840000e+01, 1.940000e+01 }, // SINR
          { 9.614662e-01, 3.350000e-01, 4.000000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.620000e+01, 17, 1.780000e+01, 1.860000e+01, 1.940000e+01 }, // SINR
          { 9.303571e-01, 3.057927e-01, 9.900000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.620000e+01, 17, 1.780000e+01, 1.850000e+01, 1.930000e+01 }, // SINR
          { 9.799618e-01, 5.362395e-01, 3.650000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.545590e+01, 1.635660e+01, 1.725730e+01, 1.815800e+01, 1.905870e+01 }, // SINR
          { 9.961538e-01, 8.843537e-01, 3.052536e-01, 1.920000e-02, 1.000000e-04 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.567690e+01, 1.632850e+01, 1.698010e+01, 1.763170e+01, 1.828330e+01, 1.893490e+01 }, // SINR
          { 1, 7.962963e-01, 3.098894e-01, 3.380000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.570000e+01, 1.650000e+01, 1.720000e+01, 1.790000e+01, 1.870000e+01 }, // SINR
          { 9.932692e-01, 6.978022e-01, 1.054721e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.602390e+01, 1.652390e+01, 1.700290e+01, 1.748180e+01, 1.796070e+01, 1.843970e+01 }, // SINR
          { 9.895038e-01, 6.779101e-01, 2.800773e-01, 5.080000e-02, 3.900000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.575000e+01, 1.635400e+01, 1.695800e+01, 1.756200e+01, 1.816600e+01 }, // SINR
          { 9.942308e-01, 8.566667e-01, 3.352273e-01, 3.810000e-02, 7.000000e-04 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 16, 1.650000e+01, 1.691650e+01, 1.733300e+01, 1.774950e+01, 1.816600e+01 }, // SINR
          { 9.798077e-01, 6.634115e-01, 2.692308e-01, 5.640000e-02, 5.500000e-03, 2.000000e-04 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.606700e+01, 1.656700e+01, 1.696680e+01, 1.736650e+01, 1.776630e+01, 1.816600e+01 }, // SINR
          { 9.846154e-01, 8.066038e-01, 4.368557e-01, 1.158649e-01, 1.390000e-02, 9.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 1.609220e+01, 1.659220e+01, 1.698570e+01, 1.737910e+01, 1.777260e+01, 1.816600e+01 }, // SINR
          { 9.990385e-01, 6.864973e-01, 2.663848e-01, 4.940000e-02, 2.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 1.544400e+01, 1.641220e+01, 1.738040e+01, 1.834850e+01 }, // SINR
          { 1, 8.518212e-01, 3.810000e-02, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.619300e+01, 1.685070e+01, 1.750830e+01, 1.816600e+01 }, // SINR
          { 9.817308e-01, 5.306017e-01, 3.050000e-02, 2.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.604200e+01, 1.657300e+01, 1.710400e+01, 1.763500e+01, 1.816600e+01 }, // SINR
          { 9.971154e-01, 8.750000e-01, 3.176952e-01, 1.930000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.604200e+01, 1.657300e+01, 1.710400e+01, 1.763500e+01, 1.816600e+01 }, // SINR
          { 9.961538e-01, 7.865854e-01, 1.971831e-01, 6.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.598620e+01, 1.648620e+01, 1.690610e+01, 1.732610e+01, 1.774600e+01 }, // SINR
          { 9.826923e-01, 8.221154e-01, 2.907044e-01, 2.120000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.589240e+01, 1.639240e+01, 1.668800e+01, 1.698360e+01, 1.727920e+01, 1.757480e+01 }, // SINR
          { 9.913462e-01, 7.937117e-01, 3.416442e-01, 5.610000e-02, 3.500000e-03, 3.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.605760e+01, 1.655760e+01, 1.695970e+01, 1.736180e+01, 1.776390e+01 }, // SINR
          { 9.503676e-01, 7.847222e-01, 1.744467e-01, 5.500000e-03, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.633920e+01, 1.685690e+01, 1.737460e+01, 1.789240e+01 }, // SINR
          { 9.617537e-01, 3.051205e-01, 3.400000e-03, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.589240e+01, 1.639240e+01, 1.654020e+01, 1.668800e+01, 1.683580e+01, 1.698360e+01, 1.748360e+01 }, // SINR
          { 9.932692e-01, 8.640940e-01, 6.599741e-01, 4.076613e-01, 1.746528e-01, 5.800000e-02, 6.000000e-04 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 1.635500e+01, 1.641280e+01, 1.647060e+01, 1.652840e+01, 1.658610e+01, 1.708610e+01, 1.758610e+01 }, // SINR
          { 9.583333e-01, 9.303571e-01, 8.932292e-01, 8.406863e-01, 7.700893e-01, 5.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.626710e+01, 1.676710e+01, 1.726710e+01, 1.776710e+01 }, // SINR
          { 9.971154e-01, 6.336634e-01, 3.680000e-02, 3.000000e-04 } // BLER
        }
      },
      { 3368U, // SINR and BLER for CBS 3368
        NrEesmErrorModel::DoubleTuple{
          { 1.580000e+01, 1.630000e+01, 1.680000e+01, 1.730000e+01 }, // SINR
          { 9.990385e-01, 8.292484e-01, 8.062901e-02, 1.000000e-04 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.580000e+01, 1.655000e+01, 1.730000e+01, 1.805000e+01 }, // SINR
          { 1, 8.241935e-01, 7.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.610000e+01, 1.660000e+01, 1.680000e+01, 17, 1.720000e+01 }, // SINR
          { 9.951923e-01, 3.374335e-01, 4.700000e-02, 3.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.631920e+01, 1.637660e+01, 1.643390e+01, 1.649130e+01, 1.654860e+01, 1.704860e+01, 1.754860e+01, 1.804860e+01 }, // SINR
          { 9.420290e-01, 9.107143e-01, 8.558333e-01, 7.797256e-01, 6.871622e-01, 1.893769e-01, 2.100000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.581200e+01, 1.631200e+01, 1.633990e+01, 1.636780e+01, 1.639570e+01, 1.642360e+01, 1.692360e+01, 1.742360e+01, 1.792360e+01 }, // SINR
          { 1, 8.975694e-01, 8.733108e-01, 8.269231e-01, 7.854938e-01, 7.341954e-01, 2.042683e-01, 1.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.598620e+01, 1.648620e+01, 1.683770e+01, 1.718930e+01, 1.754080e+01 }, // SINR
          { 1, 8.200637e-01, 1.357759e-01, 1.800000e-03, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.643010e+01, 1.648610e+01, 1.698610e+01, 1.748610e+01, 1.798610e+01 }, // SINR
          { 9.574074e-01, 9.119718e-01, 2.058824e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.660000e+01, 1.680000e+01, 17, 1.750000e+01 }, // SINR
          { 9.798077e-01, 7.699704e-01, 2.934149e-01, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.615110e+01, 1.653810e+01, 1.673150e+01, 1.692500e+01, 1.731200e+01 }, // SINR
          { 1, 7.078729e-01, 4.035714e-01, 6.860000e-02, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.627270e+01, 1.677270e+01, 1.727270e+01, 1.777270e+01 }, // SINR
          { 9.923077e-01, 3.414634e-01, 2.700000e-03, 2.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.654860e+01, 1.688190e+01, 1.704860e+01, 1.721530e+01, 1.754860e+01 }, // SINR
          { 9.626866e-01, 2.439555e-01, 7.100000e-03, 3.100000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 17
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.670000e+01, 1.760000e+01, 1.840000e+01, 1.930000e+01 }, // SINR
          { 1, 8.913793e-01, 9.958678e-02, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.673620e+01, 1.763190e+01, 1.852760e+01, 1.942330e+01, 2.031900e+01 }, // SINR
          { 9.990385e-01, 9.280576e-01, 3.384718e-01, 1.230000e-02, 1.000000e-04 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 1.670000e+01, 1.760000e+01, 1.850000e+01, 1.940000e+01 }, // SINR
          { 1, 6.726190e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 1.670000e+01, 1.740000e+01, 18, 1.870000e+01 }, // SINR
          { 1, 8.716216e-01, 1.100220e-01, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.685000e+01, 1.750000e+01, 1.810000e+01, 1.880000e+01 }, // SINR
          { 1, 7.822086e-01, 5.110000e-02, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 1.690000e+01, 1.750000e+01, 18, 1.860000e+01 }, // SINR
          { 9.980769e-01, 7.076389e-01, 5.560000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.660000e+01, 1.750000e+01, 1.830000e+01, 1.920000e+01 }, // SINR
          { 1, 7.406609e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.690000e+01, 1.750000e+01, 1.810000e+01, 1.860000e+01, 1.920000e+01 }, // SINR
          { 1, 7.955247e-01, 2.810000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.710000e+01, 1.757000e+01, 1.780000e+01, 1.803000e+01, 1.850000e+01 }, // SINR
          { 9.826923e-01, 6.933060e-01, 2.182642e-01, 5.450000e-02, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.750550e+01, 1.773090e+01, 1.795630e+01, 1.818170e+01, 1.840710e+01, 1.890710e+01 }, // SINR
          { 9.352518e-01, 7.947531e-01, 5.508658e-01, 2.831461e-01, 9.708437e-02, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.648610e+01, 1.728960e+01, 1.782520e+01, 1.809300e+01, 1.836090e+01, 1.889650e+01 }, // SINR
          { 1, 9.388489e-01, 8.423203e-01, 6.330000e-02, 1.600000e-02, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.743360e+01, 1.793360e+01, 1.834420e+01, 1.875490e+01 }, // SINR
          { 9.990385e-01, 1.744105e-01, 4.300000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.689060e+01, 1.739060e+01, 1.787010e+01, 1.834960e+01, 1.882910e+01 }, // SINR
          { 1, 8.932292e-01, 2.425193e-01, 3.500000e-03, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 1.700860e+01, 1.750860e+01, 1.780860e+01, 1.810860e+01, 1.840860e+01, 1.870860e+01 }, // SINR
          { 1, 6.945946e-01, 2.255386e-01, 2.000000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.730860e+01, 1.764190e+01, 1.797520e+01, 1.830860e+01, 1.864190e+01 }, // SINR
          { 9.485294e-01, 5.120482e-01, 7.230000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 1.726950e+01, 1.776950e+01, 1.796190e+01, 1.815430e+01, 1.834670e+01, 1.853910e+01 }, // SINR
          { 1, 3.458904e-01, 1.003772e-01, 1.600000e-02, 1.400000e-03, 4.000000e-04 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 1.705240e+01, 1.755240e+01, 1.793800e+01, 1.832350e+01, 1.870910e+01 }, // SINR
          { 1, 6.274272e-01, 8.910000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.705860e+01, 1.755860e+01, 1.785030e+01, 1.814200e+01, 1.843360e+01, 1.872530e+01 }, // SINR
          { 1, 8.181090e-01, 3.716814e-01, 4.730000e-02, 1.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.683230e+01, 1.745140e+01, 1.807050e+01, 1.868950e+01 }, // SINR
          { 1, 7.950311e-01, 1.770000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.738740e+01, 1.770760e+01, 1.802780e+01, 1.834800e+01, 1.866820e+01 }, // SINR
          { 9.951923e-01, 7.772727e-01, 2.142857e-01, 7.500000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.708860e+01, 1.758860e+01, 1.787530e+01, 1.816200e+01, 1.844860e+01 }, // SINR
          { 1, 6.686198e-01, 1.526428e-01, 5.400000e-03, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.701850e+01, 1.751850e+01, 1.781690e+01, 1.811520e+01, 1.841360e+01 }, // SINR
          { 1, 7.827744e-01, 2.028491e-01, 5.800000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.742940e+01, 1.774260e+01, 1.805580e+01, 1.836900e+01, 1.868220e+01 }, // SINR
          { 9.942308e-01, 8.128931e-01, 2.034790e-01, 6.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.743360e+01, 1.774610e+01, 1.805860e+01, 1.837110e+01, 1.868360e+01 }, // SINR
          { 9.884615e-01, 6.604381e-01, 9.706478e-02, 2.600000e-03, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.750390e+01, 1.780470e+01, 1.810550e+01, 1.840620e+01, 1.870700e+01 }, // SINR
          { 9.942308e-01, 7.573529e-01, 1.203394e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 1.755860e+01, 1.785030e+01, 1.814200e+01, 1.843360e+01, 1.872530e+01 }, // SINR
          { 9.846154e-01, 6.207729e-01, 8.050000e-02, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.755860e+01, 1.785030e+01, 1.799610e+01, 1.814190e+01, 1.843360e+01, 1.887110e+01 }, // SINR
          { 9.732824e-01, 2.976471e-01, 2.061582e-01, 4.400000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.740490e+01, 1.776680e+01, 1.794780e+01, 1.812870e+01, 1.849060e+01, 1.903340e+01 }, // SINR
          { 9.990385e-01, 9.443431e-01, 4.637681e-01, 1.083479e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 3624U, // SINR and BLER for CBS 3624
        NrEesmErrorModel::DoubleTuple{
          { 1.773890e+01, 1.805280e+01, 1.836680e+01, 1.868070e+01 }, // SINR
          { 9.540441e-01, 3.118812e-01, 7.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 1.770250e+01, 1.801480e+01, 1.832710e+01, 1.863940e+01 }, // SINR
          { 9.942308e-01, 2.750545e-01, 1.200000e-02, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.755860e+01, 1.789490e+01, 1.806300e+01, 1.823110e+01, 1.856740e+01, 1.907180e+01 }, // SINR
          { 9.788462e-01, 5.459402e-01, 1.202107e-01, 8.200000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.730000e+01, 1.765000e+01, 1.782500e+01, 18, 1.835000e+01, 1.887500e+01 }, // SINR
          { 1, 8.716216e-01, 8.400974e-01, 1.070043e-01, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.772340e+01, 1.799990e+01, 1.813810e+01, 1.827640e+01, 1.855290e+01, 1.979720e+01 }, // SINR
          { 9.836538e-01, 2.823661e-01, 1.745480e-01, 1.520000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.754000e+01, 1.772000e+01, 1.790000e+01, 1.810000e+01, 1.830000e+01, 1.840000e+01, 1.860000e+01 }, // SINR
          { 9.903846e-01, 3.322454e-01, 2.868151e-01, 6.980000e-02, 7.200000e-03, 1.900000e-03, 2.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.711330e+01, 1.761330e+01, 1.810400e+01, 1.859470e+01 }, // SINR
          { 1, 8.897569e-01, 6.900000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.772790e+01, 1.803590e+01, 1.834400e+01, 1.865200e+01 }, // SINR
          { 9.913462e-01, 8.870000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.750490e+01, 1.780550e+01, 1.810610e+01, 1.840670e+01 }, // SINR
          { 1, 5.268595e-01, 2.380000e-02, 1.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.747950e+01, 1.778430e+01, 1.808920e+01, 1.839400e+01 }, // SINR
          { 1, 6.868351e-01, 5.590000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 18
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.854550e+01, 1.895640e+01, 1.936730e+01, 2.018900e+01 }, // SINR
          { 9.855769e-01, 5.770000e-02, 5.400000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.844550e+01, 1.875970e+01, 1.907390e+01, 1.970240e+01 }, // SINR
          { 9.687500e-01, 8.253205e-01, 2.190000e-02, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 1.858990e+01, 1.888350e+01, 1.917710e+01, 1.976430e+01 }, // SINR
          { 9.847328e-01, 2.036062e-01, 7.450000e-02, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.842400e+01, 1.891760e+01, 1.916430e+01, 1.941110e+01, 1.990470e+01 }, // SINR
          { 9.503676e-01, 2.896689e-01, 1.100000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 1.858980e+01, 1.890230e+01, 1.921470e+01, 1.952720e+01 }, // SINR
          { 9.837786e-01, 4.702602e-01, 2.040000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.829110e+01, 1.859600e+01, 1.890100e+01, 1.951080e+01 }, // SINR
          { 9.951923e-01, 8.565436e-01, 6.050000e-02, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.862910e+01, 1.892350e+01, 1.921780e+01, 1.951220e+01 }, // SINR
          { 9.570896e-01, 8.766779e-01, 2.024478e-01, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.847080e+01, 1.895400e+01, 1.919560e+01, 1.943710e+01, 1.992030e+01 }, // SINR
          { 1, 9.456699e-02, 2.700000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.798000e+01, 1.855000e+01, 1.911950e+01, 1.969160e+01 }, // SINR
          { 1, 7.993827e-01, 1.478873e-01, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.795000e+01, 1.861000e+01, 1.927040e+01, 1.993050e+01, 2.059060e+01 }, // SINR
          { 9.903846e-01, 4.302721e-01, 5.030000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+01, 1.870000e+01, 1.890000e+01, 1.910000e+01, 1.930000e+01, 1.950000e+01, 1.970000e+01, 1.990000e+01, 2.010000e+01, 2.030000e+01, 2.050000e+01 }, // SINR
          { 9.626866e-01, 8.673986e-01, 6.690415e-01, 4.030854e-01, 1.842972e-01, 4.290000e-02, 1.120000e-02, 2.100000e-03, 2.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+01, 19, 1.925000e+01, 1.950000e+01, 1.975000e+01 }, // SINR
          { 9.837786e-01, 2.121212e-01, 1.550000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 1.770000e+01, 1.830000e+01, 1.860000e+01, 1.890000e+01, 1.950000e+01 }, // SINR
          { 1, 9.761450e-01, 3.676901e-01, 1.245040e-01, 0 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.867000e+01, 19, 1.923000e+01, 1.980000e+01 }, // SINR
          { 9.980769e-01, 4.555160e-01, 2.640000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 18, 1.847000e+01, 1.870000e+01, 1.893000e+01, 1.940000e+01, 2.010000e+01 }, // SINR
          { 1, 8.402318e-01, 1.138332e-01, 6.220000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.860000e+01, 19, 1.950000e+01, 20 }, // SINR
          { 1, 8.566667e-01, 1.442750e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.830000e+01, 1.870000e+01, 1.910000e+01, 1.950000e+01, 20 }, // SINR
          { 1, 9.734848e-01, 3.106527e-01, 6.400000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.860000e+01, 19, 1.940000e+01, 1.990000e+01 }, // SINR
          { 1, 9.951923e-01, 4.879808e-01, 1.350000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.848000e+01, 1.885000e+01, 1.923000e+01, 1.960000e+01, 1.998000e+01, 2.035000e+01, 2.073000e+01 }, // SINR
          { 1, 9.744318e-01, 7.958075e-01, 3.614672e-01, 7.280000e-02, 8.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.853330e+01, 1.875000e+01, 1.896670e+01, 1.940000e+01 }, // SINR
          { 9.516423e-01, 3.731563e-01, 1.089939e-01, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.820000e+01, 1.861670e+01, 1.882500e+01, 1.903330e+01, 1.945000e+01 }, // SINR
          { 1, 9.375000e-01, 5.879630e-01, 8.497191e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+01, 1.891670e+01, 1.912500e+01, 1.933330e+01, 1.975000e+01 }, // SINR
          { 9.865385e-01, 1.447520e-01, 6.300000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.790000e+01, 1.840000e+01, 1.870000e+01, 19, 1.930000e+01, 1.960000e+01, 2.010000e+01, 2.060000e+01 }, // SINR
          { 9.598881e-01, 7.732036e-01, 5.132653e-01, 2.773179e-01, 1.117475e-01, 3.280000e-02, 2.500000e-03, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.746570e+01, 1.810160e+01, 1.873750e+01, 1.937340e+01, 2.000920e+01, 2.064510e+01 }, // SINR
          { 9.894231e-01, 8.818027e-01, 3.852896e-01, 3.920000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.853000e+01, 1.870000e+01, 1.897000e+01, 1.940000e+01, 20 }, // SINR
          { 1, 8.280255e-01, 7.736280e-01, 4.730000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.850000e+01, 19, 1.916670e+01, 1.933330e+01, 1.950000e+01 }, // SINR
          { 9.564815e-01, 1.888138e-01, 1.850000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.800210e+01, 1.853070e+01, 1.905930e+01, 1.958790e+01, 2.011650e+01, 2.064510e+01 }, // SINR
          { 9.828244e-01, 7.896341e-01, 2.386364e-01, 1.920000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 3824U, // SINR and BLER for CBS 3824
        NrEesmErrorModel::DoubleTuple{
          { 1.871670e+01, 1.890000e+01, 1.908330e+01, 1.945000e+01, 20 }, // SINR
          { 9.706439e-01, 4.412021e-01, 2.200612e-01, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 18, 1.850000e+01, 1.870000e+01, 1.890000e+01, 1.910000e+01, 1.930000e+01 }, // SINR
          { 9.847328e-01, 4.152961e-01, 1.229207e-01, 1.880000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.733010e+01, 1.834060e+01, 1.935110e+01, 2.036150e+01 }, // SINR
          { 1, 8.879310e-01, 3.740000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.797450e+01, 1.850860e+01, 1.904270e+01, 1.957690e+01, 2.011100e+01 }, // SINR
          { 9.753788e-01, 5.450644e-01, 6.020000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.810000e+01, 1.860000e+01, 1.890000e+01, 1.910000e+01, 1.920000e+01, 1.950000e+01 }, // SINR
          { 1, 9.280576e-01, 3.064320e-01, 4.140000e-02, 2.340000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.804100e+01, 1.856180e+01, 1.908260e+01, 1.960340e+01, 2.012430e+01 }, // SINR
          { 9.555556e-01, 7.001366e-01, 7.580000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.767630e+01, 1.817630e+01, 1.858780e+01, 1.899930e+01, 1.941070e+01, 1.982220e+01 }, // SINR
          { 1, 8.666107e-01, 2.732181e-01, 1.260000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.780140e+01, 1.830140e+01, 1.859430e+01, 1.888730e+01, 1.918030e+01, 1.947320e+01 }, // SINR
          { 9.780534e-01, 8.880208e-01, 4.655331e-01, 8.660000e-02, 4.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.777010e+01, 1.827010e+01, 1.856700e+01, 1.886390e+01, 1.916080e+01, 1.945770e+01 }, // SINR
          { 9.932692e-01, 7.470930e-01, 2.480354e-01, 2.340000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.795960e+01, 1.845960e+01, 1.882390e+01, 1.918810e+01, 1.955230e+01 }, // SINR
          { 9.923077e-01, 7.875767e-01, 2.086794e-01, 7.400000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 19
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.890000e+01, 1.970000e+01, 1.997000e+01, 2.023000e+01, 2.050000e+01, 2.120000e+01 }, // SINR
          { 1, 9.534672e-01, 2.886156e-01, 2.450000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.950000e+01, 1.975000e+01, 20, 2.025000e+01, 2.050000e+01 }, // SINR
          { 9.980769e-01, 7.245763e-01, 7.290000e-02, 1.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.970000e+01, 20, 2.050000e+01, 21 }, // SINR
          { 9.661654e-01, 1.967085e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 1.852410e+01, 1.939640e+01, 1.983250e+01, 2.026870e+01, 2.114100e+01 }, // SINR
          { 1, 6.929348e-01, 5.870000e-02, 1.160000e-02, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.920720e+01, 1.960730e+01, 2.000740e+01, 2.040750e+01, 2.080760e+01 }, // SINR
          { 9.990385e-01, 8.409091e-01, 1.661162e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 1.920000e+01, 1.945000e+01, 1.970000e+01, 1.995000e+01, 2.020000e+01, 2.070000e+01, 2.120000e+01 }, // SINR
          { 9.580224e-01, 9.196429e-01, 8.366013e-01, 7.362717e-01, 5.930233e-01, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.902030e+01, 1.961790e+01, 2.021540e+01, 2.081290e+01 }, // SINR
          { 1, 5.524017e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.934380e+01, 1.972440e+01, 2.010500e+01, 2.048560e+01 }, // SINR
          { 9.568015e-01, 3.020335e-01, 3.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 1.927090e+01, 1.966190e+01, 2.005300e+01, 2.044400e+01, 2.083500e+01 }, // SINR
          { 9.837786e-01, 5.537281e-01, 3.640000e-02, 8.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.929520e+01, 1.968270e+01, 2.007030e+01, 2.045780e+01, 2.084530e+01 }, // SINR
          { 9.671053e-01, 4.155738e-01, 1.810000e-02, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.920000e+01, 1.947500e+01, 1.975000e+01, 2.002500e+01, 2.030000e+01, 2.080000e+01 }, // SINR
          { 9.740385e-01, 9.276786e-01, 8.368506e-01, 7.131944e-01, 5.556769e-01, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.926610e+01, 1.976610e+01, 1.989070e+01, 2.001520e+01, 2.013970e+01, 2.026430e+01, 2.076430e+01 }, // SINR
          { 9.586397e-01, 6.489899e-01, 3.567416e-01, 1.421291e-01, 4.360000e-02, 1.080000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.918230e+01, 1.958600e+01, 1.998970e+01, 2.039330e+01, 2.079700e+01 }, // SINR
          { 1, 9.411232e-01, 1.927321e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 1.932370e+01, 1.965920e+01, 1.999470e+01, 2.033030e+01 }, // SINR
          { 9.903846e-01, 5.658186e-01, 3.670000e-02, 2.000000e-04 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 1.960000e+01, 1.977500e+01, 1.995000e+01, 2.012500e+01, 2.030000e+01 }, // SINR
          { 9.923077e-01, 8.176101e-01, 3.056901e-01, 3.430000e-02, 7.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 1.941220e+01, 1.972390e+01, 2.003550e+01, 2.034720e+01, 2.128220e+01 }, // SINR
          { 9.951923e-01, 2.357210e-01, 2.050000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 1.926810e+01, 1.969610e+01, 2.012410e+01, 2.055200e+01 }, // SINR
          { 1, 8.741497e-01, 5.230000e-02, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 1.890000e+01, 1.940000e+01, 1.960000e+01, 1.980000e+01, 20, 2.020000e+01, 2.070000e+01 }, // SINR
          { 1, 8.368506e-01, 7.147790e-01, 5.641593e-01, 4.206811e-01, 2.726782e-01, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.900250e+01, 1.965470e+01, 2.008960e+01, 2.030700e+01, 2.052440e+01, 2.095930e+01 }, // SINR
          { 1, 9.241071e-01, 6.800000e-02, 3.200000e-03, 7.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.910000e+01, 1.960000e+01, 1.985000e+01, 2.010000e+01, 2.035000e+01, 2.060000e+01 }, // SINR
          { 9.075704e-01, 2.875854e-01, 7.080000e-02, 8.900000e-03, 6.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.945590e+01, 1.992390e+01, 2.015790e+01, 2.039190e+01, 2.085990e+01 }, // SINR
          { 1, 1.836957e-01, 7.140000e-02, 1.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.938170e+01, 1.996530e+01, 2.025710e+01, 2.054900e+01 }, // SINR
          { 1, 2.640000e-02, 2.080000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.958250e+01, 1.980600e+01, 2.002940e+01, 2.025290e+01, 2.092320e+01 }, // SINR
          { 9.217626e-01, 3.828593e-01, 7.580000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.903300e+01, 1.953300e+01, 2.003300e+01, 2.052670e+01 }, // SINR
          { 1, 5.548246e-01, 5.990000e-02, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 1.979500e+01, 2.011120e+01, 2.042730e+01, 2.074340e+01, 2.105960e+01 }, // SINR
          { 9.828244e-01, 3.945925e-01, 5.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 1.888000e+01, 1.940000e+01, 1.992000e+01, 2.044550e+01, 2.096630e+01, 2.148710e+01 }, // SINR
          { 1, 5.559211e-01, 2.883295e-01, 4.160000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 19, 1.938000e+01, 1.975000e+01, 2.013000e+01, 2.050000e+01 }, // SINR
          { 1, 9.704198e-01, 3.549860e-01, 6.400000e-03, 2.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.981330e+01, 2.005720e+01, 2.022390e+01, 2.039050e+01, 2.055720e+01 }, // SINR
          { 9.533582e-01, 9.157801e-01, 7.250000e-02, 2.910000e-02, 0 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 1.888300e+01, 1.938300e+01, 1.967470e+01, 1.996630e+01, 2.025800e+01, 2.054970e+01, 2.104970e+01 }, // SINR
          { 1, 8.468543e-01, 7.558824e-01, 6.705729e-01, 5.538043e-01, 4.158416e-01, 0 } // BLER
        }
      },
      { 4032U, // SINR and BLER for CBS 4032
        NrEesmErrorModel::DoubleTuple{
          { 1.935240e+01, 1.969000e+01, 1.986380e+01, 2.003000e+01, 2.037510e+01, 2.088650e+01 }, // SINR
          { 1, 9.855769e-01, 5.680804e-01, 5.289256e-01, 1.000000e-04, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 1.860000e+01, 1.950000e+01, 20, 2.040000e+01, 2.130000e+01 }, // SINR
          { 1, 8.724832e-01, 1.062554e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.911490e+01, 1.928980e+01, 1.946470e+01, 1.963960e+01, 1.981450e+01, 2.031450e+01, 2.081450e+01 }, // SINR
          { 9.425182e-01, 9.033688e-01, 8.665541e-01, 8.128931e-01, 7.412281e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 1.910000e+01, 1.967000e+01, 1.990000e+01, 2.023000e+01, 2.080000e+01 }, // SINR
          { 1, 9.699248e-01, 9.521405e-02, 6.220000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 1.920000e+01, 1.940000e+01, 1.960000e+01, 1.980000e+01, 20, 2.020000e+01 }, // SINR
          { 9.865385e-01, 5.559211e-01, 2.875854e-01, 5.060000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 1.940000e+01, 1.977000e+01, 1.990000e+01, 2.013000e+01, 2.050000e+01, 2.110000e+01 }, // SINR
          { 1, 6.044601e-01, 3.385695e-01, 2.280000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 1.885360e+01, 1.935360e+01, 1.957480e+01, 1.979600e+01, 2.001720e+01, 2.023840e+01, 2.073840e+01, 2.123840e+01 }, // SINR
          { 1, 8.437500e-01, 7.558824e-01, 6.432161e-01, 4.898649e-01, 3.620690e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 1.885000e+01, 1.952000e+01, 1.985000e+01, 2.018000e+01, 2.085000e+01 }, // SINR
          { 1, 4.586331e-01, 3.173804e-01, 6.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 1.960000e+01, 2.013000e+01, 2.040000e+01, 2.067000e+01, 2.120000e+01 }, // SINR
          { 1, 3.597734e-01, 1.170000e-02, 1.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 20
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.013900e+01, 2.112940e+01, 2.211990e+01, 2.311030e+01 }, // SINR
          { 9.903846e-01, 6.305419e-01, 3.340000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.024950e+01, 2.072630e+01, 2.120310e+01, 2.167990e+01, 2.215670e+01, 2.265670e+01 }, // SINR
          { 9.029720e-01, 5.875576e-01, 2.122681e-01, 3.730000e-02, 2.200000e-03, 0 } // BLER
        }
      },
      { 4224U, // SINR and BLER for CBS 4224
        NrEesmErrorModel::DoubleTuple{
          { 1.979900e+01, 2.029900e+01, 2.079900e+01, 2.129900e+01, 2.179900e+01, 2.229900e+01 }, // SINR
          { 9.865385e-01, 7.989130e-01, 3.836858e-01, 7.400000e-02, 4.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 1.987540e+01, 2.052240e+01, 2.116930e+01, 2.181630e+01, 2.246330e+01 }, // SINR
          { 1, 8.415033e-01, 2.858352e-01, 1.530000e-02, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.025300e+01, 2.030880e+01, 2.036460e+01, 2.042040e+01, 2.047620e+01, 2.097620e+01, 2.147620e+01 }, // SINR
          { 9.411232e-01, 9.166667e-01, 8.996479e-01, 8.640940e-01, 8.379032e-01, 1.937596e-01, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.037450e+01, 2.105840e+01, 2.174240e+01, 2.242630e+01, 2.311030e+01 }, // SINR
          { 9.865385e-01, 6.090047e-01, 6.340000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 2.036820e+01, 2.082530e+01, 2.128230e+01, 2.173930e+01, 2.219630e+01, 2.269630e+01 }, // SINR
          { 9.866412e-01, 7.893519e-01, 3.716912e-01, 5.230000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.040810e+01, 2.108360e+01, 2.175920e+01, 2.243470e+01 }, // SINR
          { 9.744318e-01, 5.303347e-01, 2.570000e-02, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.039220e+01, 2.089220e+01, 2.113670e+01, 2.138130e+01, 2.162580e+01, 2.187030e+01, 2.237030e+01 }, // SINR
          { 1, 6.510152e-01, 3.876534e-01, 1.515380e-01, 3.790000e-02, 6.500000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.010000e+01, 2.060000e+01, 2.087500e+01, 2.115000e+01, 2.142500e+01, 2.170000e+01, 2.220000e+01 }, // SINR
          { 1, 7.272727e-01, 4.809886e-01, 2.091625e-01, 5.860000e-02, 1.110000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 21
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 2.010000e+01, 2.110000e+01, 2.160000e+01, 2.210000e+01, 2.310000e+01, 2.460000e+01 }, // SINR
          { 1, 9.680451e-01, 6.048578e-01, 1.441648e-01, 1.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 2.110000e+01, 2.177000e+01, 2.210000e+01, 2.243000e+01, 2.310000e+01 }, // SINR
          { 1, 5.619469e-01, 2.500000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 2.113000e+01, 2.150000e+01, 2.187000e+01, 2.260000e+01 }, // SINR
          { 1, 1.008098e-01, 3.300000e-02, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 2.050000e+01, 2.093000e+01, 2.137000e+01, 2.180000e+01, 2.320000e+01 }, // SINR
          { 9.961538e-01, 9.875000e-01, 6.705729e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 2.120000e+01, 2.145000e+01, 2.170000e+01, 2.195000e+01, 2.220000e+01, 2.245000e+01 }, // SINR
          { 1, 9.951923e-01, 7.298851e-01, 1.307351e-01, 2.600000e-03, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 2.135000e+01, 2.160000e+01, 2.185000e+01, 2.210000e+01, 2.330000e+01 }, // SINR
          { 1, 5.457974e-01, 5.370000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 528U, // SINR and BLER for CBS 528
        NrEesmErrorModel::DoubleTuple{
          { 2.093560e+01, 2.143560e+01, 2.158530e+01, 2.173510e+01, 2.188490e+01, 2.203460e+01, 2.253460e+01 }, // SINR
          { 9.329710e-01, 8.683333e-01, 8.213141e-01, 7.657186e-01, 6.844920e-01, 6.037736e-01, 0 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 1.990000e+01, 2.090000e+01, 2.163000e+01, 22, 2.237000e+01 }, // SINR
          { 1, 9.951923e-01, 3.550000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 2.060000e+01, 2.127000e+01, 2.160000e+01, 2.193000e+01, 2.260000e+01 }, // SINR
          { 1, 5.141700e-01, 4.500000e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 1.968790e+01, 2.079870e+01, 2.190940e+01, 2.302020e+01, 2.413100e+01 }, // SINR
          { 9.990385e-01, 9.474638e-01, 3.000000e-01, 1.800000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.065960e+01, 2.141890e+01, 2.217820e+01, 2.293740e+01, 2.369670e+01 }, // SINR
          { 9.668561e-01, 7.182203e-01, 1.651845e-01, 3.400000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.126270e+01, 2.203000e+01, 2.279740e+01, 2.356480e+01, 2.433210e+01 }, // SINR
          { 9.198944e-01, 4.112903e-01, 2.170000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.097120e+01, 2.164340e+01, 2.231560e+01, 2.298770e+01, 2.365990e+01, 2.433210e+01 }, // SINR
          { 9.836538e-01, 7.888720e-01, 2.566462e-01, 1.590000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 2.010300e+01, 2.116030e+01, 2.221760e+01, 2.327480e+01 }, // SINR
          { 9.990385e-01, 7.463663e-01, 3.430000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 2.035660e+01, 2.085660e+01, 2.126270e+01, 2.166880e+01, 2.207500e+01, 2.248110e+01, 2.298110e+01 }, // SINR
          { 1, 8.750000e-01, 6.002358e-01, 2.170826e-01, 3.370000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 2.087880e+01, 2.137880e+01, 2.160810e+01, 2.183750e+01, 2.206680e+01, 2.229620e+01, 2.279620e+01 }, // SINR
          { 9.980769e-01, 7.981366e-01, 5.997653e-01, 3.645533e-01, 1.781650e-01, 5.850000e-02, 0 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 2.095520e+01, 2.145520e+01, 2.160390e+01, 2.175250e+01, 2.190120e+01, 2.204980e+01, 2.254980e+01 }, // SINR
          { 9.836538e-01, 6.171117e-01, 4.580325e-01, 3.043870e-01, 1.793866e-01, 8.920000e-02, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 2.092320e+01, 2.142320e+01, 2.164890e+01, 2.187450e+01, 2.210010e+01, 2.232580e+01, 2.282580e+01 }, // SINR
          { 9.818702e-01, 6.066351e-01, 3.720674e-01, 1.602810e-01, 5.050000e-02, 9.700000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.098610e+01, 2.143540e+01, 2.188470e+01, 2.233390e+01, 2.278320e+01 }, // SINR
          { 9.818702e-01, 8.168790e-01, 3.337766e-01, 3.370000e-02, 6.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.094130e+01, 2.144130e+01, 2.185220e+01, 2.226310e+01, 2.267410e+01 }, // SINR
          { 9.932692e-01, 4.440559e-01, 9.148551e-02, 3.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.090650e+01, 2.140650e+01, 2.163350e+01, 2.186060e+01, 2.208770e+01, 2.231470e+01 }, // SINR
          { 9.980769e-01, 3.017857e-01, 1.179644e-01, 2.640000e-02, 2.400000e-03, 2.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.090650e+01, 2.140650e+01, 2.163350e+01, 2.186060e+01, 2.208770e+01, 2.231470e+01 }, // SINR
          { 1, 3.183544e-01, 9.111022e-02, 1.440000e-02, 1.200000e-03, 2.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.095250e+01, 2.140660e+01, 2.186060e+01, 2.231470e+01, 2.276880e+01 }, // SINR
          { 9.951923e-01, 8.518212e-01, 2.392857e-01, 3.800000e-03, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 2.109290e+01, 2.139670e+01, 2.170050e+01, 2.230810e+01 }, // SINR
          { 9.659091e-01, 5.706278e-01, 1.085237e-01, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 2.069820e+01, 2.119820e+01, 2.161720e+01, 2.203610e+01, 2.245510e+01 }, // SINR
          { 1, 6.201691e-01, 7.900000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 2.115350e+01, 2.165350e+01, 2.178390e+01, 2.191430e+01, 2.204460e+01, 2.217500e+01 }, // SINR
          { 9.923077e-01, 2.409351e-01, 1.000827e-01, 2.990000e-02, 7.100000e-03, 1.000000e-03 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 2.064150e+01, 2.122310e+01, 2.180470e+01, 2.238620e+01 }, // SINR
          { 9.990385e-01, 5.821918e-01, 7.700000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 2.069620e+01, 2.126870e+01, 2.184120e+01, 2.241360e+01 }, // SINR
          { 9.913462e-01, 4.051587e-01, 1.900000e-03, 0 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.104500e+01, 2.154500e+01, 2.170120e+01, 2.185740e+01, 2.201360e+01, 2.216980e+01 }, // SINR
          { 1, 1.880000e-02, 2.800000e-03, 4.000000e-04, 2.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.067280e+01, 2.105710e+01, 2.144130e+01, 2.182550e+01, 2.220980e+01 }, // SINR
          { 1, 9.064685e-01, 3.513889e-01, 1.570000e-02, 0 } // BLER
        }
      },
      { 4352U, // SINR and BLER for CBS 4352
        NrEesmErrorModel::DoubleTuple{
          { 2.076650e+01, 2.132730e+01, 2.188800e+01, 2.244880e+01 }, // SINR
          { 9.961538e-01, 4.770599e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 2.086030e+01, 2.158640e+01, 2.194950e+01, 2.231260e+01 }, // SINR
          { 1, 1.660620e-01, 9.800000e-03, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.075000e+01, 2.147810e+01, 2.184220e+01, 2.220620e+01 }, // SINR
          { 9.990385e-01, 6.095238e-01, 1.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.118230e+01, 2.140910e+01, 2.163590e+01, 2.208960e+01 }, // SINR
          { 9.751908e-01, 3.549296e-01, 1.928517e-01, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 2.138250e+01, 2.171230e+01, 2.204210e+01, 2.237200e+01, 2.287200e+01 }, // SINR
          { 9.742366e-01, 6.231707e-01, 8.680000e-02, 2.000000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.095540e+01, 2.140910e+01, 2.186270e+01, 2.231640e+01 }, // SINR
          { 9.865385e-01, 4.485035e-01, 1.200000e-02, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.074810e+01, 2.147800e+01, 2.184290e+01, 2.220780e+01, 2.293770e+01 }, // SINR
          { 1, 6.991758e-01, 2.670000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.128900e+01, 2.165940e+01, 2.202980e+01, 2.277070e+01 }, // SINR
          { 9.809160e-01, 3.135910e-01, 3.700000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 22
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 2.200560e+01, 2.246370e+01, 2.276910e+01, 2.292180e+01, 2.307460e+01, 2.338000e+01 }, // SINR
          { 1, 9.255319e-01, 1.120766e-01, 1.690000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 2.242230e+01, 2.278320e+01, 2.296370e+01, 2.314410e+01, 2.350500e+01 }, // SINR
          { 9.630682e-01, 1.262563e-01, 1.090000e-02, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 2.242230e+01, 2.278320e+01, 2.296370e+01, 2.314410e+01 }, // SINR
          { 9.942308e-01, 5.240000e-02, 2.740000e-02, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 2.230000e+01, 2.255000e+01, 2.280000e+01, 2.305000e+01, 2.330000e+01 }, // SINR
          { 9.961538e-01, 7.071823e-01, 7.830000e-02, 1.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 2.225560e+01, 2.263500e+01, 2.282480e+01, 2.301450e+01, 2.339390e+01 }, // SINR
          { 9.980769e-01, 4.908654e-01, 3.130000e-02, 2.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 528U, // SINR and BLER for CBS 528
        NrEesmErrorModel::DoubleTuple{
          { 2.233210e+01, 2.258900e+01, 2.284580e+01, 2.335940e+01 }, // SINR
          { 1, 5.315126e-01, 9.677750e-02, 1.000000e-04 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 2.225560e+01, 2.254020e+01, 2.268250e+01, 2.282470e+01, 2.310930e+01 }, // SINR
          { 9.865385e-01, 6.998626e-01, 5.730000e-02, 4.170000e-02, 1.000000e-04 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 2.208890e+01, 2.253660e+01, 2.298430e+01, 2.343210e+01 }, // SINR
          { 1, 7.765152e-01, 3.200000e-03, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 2.231240e+01, 2.259230e+01, 2.287210e+01, 2.315200e+01 }, // SINR
          { 9.652256e-01, 3.034010e-01, 4.600000e-03, 0 } // BLER
        }
      },
      { 736U, // SINR and BLER for CBS 736
        NrEesmErrorModel::DoubleTuple{
          { 2.170000e+01, 2.220000e+01, 2.245000e+01, 2.270000e+01, 2.295000e+01, 2.320000e+01, 2.370000e+01 }, // SINR
          { 1, 8.674497e-01, 6.801862e-01, 4.226589e-01, 2.081954e-01, 8.560000e-02, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.170000e+01, 2.237000e+01, 2.270000e+01, 2.303000e+01, 2.370000e+01 }, // SINR
          { 1, 8.566667e-01, 9.005705e-02, 4.500000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.160000e+01, 2.180000e+01, 22, 2.220000e+01, 2.240000e+01, 2.260000e+01, 2.280000e+01, 23, 2.320000e+01, 2.340000e+01, 2.360000e+01 }, // SINR
          { 9.903846e-01, 9.466912e-01, 8.293269e-01, 5.996462e-01, 2.964706e-01, 1.292526e-01, 3.940000e-02, 1.170000e-02, 2.600000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 2.129420e+01, 2.179420e+01, 2.193260e+01, 2.207110e+01, 2.220960e+01, 2.234800e+01, 2.284800e+01, 2.334800e+01, 2.384800e+01 }, // SINR
          { 9.826923e-01, 8.766892e-01, 8.368506e-01, 7.881098e-01, 7.164804e-01, 6.472081e-01, 3.014354e-01, 8.000000e-02, 4.000000e-04 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 2.131810e+01, 2.181810e+01, 2.231810e+01, 2.252770e+01, 2.273720e+01, 2.294680e+01, 2.315630e+01, 2.365630e+01, 2.415630e+01 }, // SINR
          { 1, 8.905172e-01, 6.984890e-01, 5.678251e-01, 4.209437e-01, 2.961765e-01, 2.089552e-01, 6.200000e-03, 4.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 2.199000e+01, 2.232000e+01, 2.265000e+01, 2.332000e+01, 2.432000e+01 }, // SINR
          { 1, 5.810502e-01, 3.196203e-01, 1.000000e-04, 0 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 2.180000e+01, 2.250000e+01, 2.297000e+01, 2.320000e+01, 2.343000e+01, 2.390000e+01 }, // SINR
          { 1, 9.244604e-01, 1.010000e-02, 1.700000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 2.164890e+01, 2.258510e+01, 2.305330e+01, 2.352140e+01, 2.445760e+01 }, // SINR
          { 9.555556e-01, 4.671296e-01, 1.858358e-01, 1.600000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.215000e+01, 2.250000e+01, 2.280000e+01, 2.320000e+01, 2.350000e+01, 2.390000e+01 }, // SINR
          { 9.799618e-01, 8.250000e-01, 1.175023e-01, 3.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.042740e+01, 2.178610e+01, 2.269190e+01, 2.314470e+01, 2.359760e+01, 2.450340e+01, 2.586200e+01 }, // SINR
          { 1, 9.425182e-01, 2.460938e-01, 8.740000e-02, 1.400000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.174120e+01, 2.224120e+01, 2.246750e+01, 2.269380e+01, 2.292020e+01, 2.314650e+01, 2.364650e+01, 2.414650e+01 }, // SINR
          { 9.522059e-01, 7.934783e-01, 6.317402e-01, 3.995253e-01, 2.336754e-01, 1.258750e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.168110e+01, 2.261020e+01, 2.307470e+01, 2.353930e+01, 2.446840e+01, 2.586200e+01 }, // SINR
          { 9.961538e-01, 6.350503e-01, 3.062954e-01, 1.520000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.181940e+01, 2.231940e+01, 2.246700e+01, 2.261460e+01, 2.276220e+01, 2.290980e+01, 2.340980e+01, 2.390980e+01 }, // SINR
          { 9.923077e-01, 4.752799e-01, 3.170200e-01, 1.874069e-01, 1.037301e-01, 5.470000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 2.220000e+01, 2.270000e+01, 2.310000e+01, 2.360000e+01 }, // SINR
          { 1, 4.802632e-01, 4.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 2.170000e+01, 2.220000e+01, 2.255000e+01, 2.290000e+01, 2.325000e+01, 2.360000e+01 }, // SINR
          { 1, 3.981191e-01, 5.450000e-02, 2.300000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 2.129350e+01, 2.202300e+01, 2.275250e+01, 2.348210e+01, 2.421160e+01 }, // SINR
          { 9.990385e-01, 8.362903e-01, 5.970000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 2.133930e+01, 2.206120e+01, 2.242210e+01, 2.278310e+01, 2.350500e+01 }, // SINR
          { 1, 7.875767e-01, 4.097896e-01, 1.290000e-02, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 2.175600e+01, 2.240840e+01, 2.273470e+01, 2.306090e+01, 2.371330e+01 }, // SINR
          { 9.713740e-01, 2.272523e-01, 4.440000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.113100e+01, 2.188760e+01, 2.226590e+01, 2.264420e+01, 2.340080e+01 }, // SINR
          { 1, 9.370504e-01, 5.161943e-01, 2.610000e-02, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.138100e+01, 2.209590e+01, 2.245340e+01, 2.281090e+01, 2.352580e+01 }, // SINR
          { 1, 7.406069e-01, 6.608073e-01, 1.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 2.167930e+01, 2.207840e+01, 2.247750e+01, 2.327580e+01 }, // SINR
          { 9.932692e-01, 7.932099e-01, 8.290000e-02, 0 } // BLER
        }
      },
      { 4608U, // SINR and BLER for CBS 4608
        NrEesmErrorModel::DoubleTuple{
          { 2.170000e+01, 22, 2.230000e+01, 2.260000e+01, 2.310000e+01, 2.360000e+01 }, // SINR
          { 9.980769e-01, 9.181655e-01, 6.330446e-01, 2.017685e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.123520e+01, 2.197440e+01, 2.271360e+01, 2.345290e+01 }, // SINR
          { 1, 8.949653e-01, 1.450000e-02, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.124510e+01, 2.198270e+01, 2.235150e+01, 2.272030e+01, 2.345790e+01 }, // SINR
          { 9.990385e-01, 8.168239e-01, 4.820000e-02, 3.700000e-03, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 2.195700e+01, 2.232840e+01, 2.269980e+01, 2.344250e+01 }, // SINR
          { 9.782197e-01, 1.086245e-01, 4.610000e-02, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.190000e+01, 2.217500e+01, 2.245000e+01, 2.272500e+01, 23 }, // SINR
          { 9.429348e-01, 6.443750e-01, 1.806948e-01, 1.190000e-02, 1.000000e-04 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.184850e+01, 2.223070e+01, 2.261300e+01, 2.337740e+01 }, // SINR
          { 1, 8.724832e-01, 3.178392e-01, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.213980e+01, 2.247610e+01, 2.264420e+01, 2.281230e+01, 2.314860e+01 }, // SINR
          { 9.561567e-01, 6.768617e-01, 9.097392e-02, 8.370000e-02, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 23
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 2.294450e+01, 2.335080e+01, 2.416340e+01, 2.497600e+01, 2.578860e+01 }, // SINR
          { 9.708647e-01, 3.926923e-01, 6.690000e-02, 2.000000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 2.260000e+01, 2.320000e+01, 2.367000e+01, 2.390000e+01, 2.413000e+01, 2.460000e+01 }, // SINR
          { 1, 9.770992e-01, 3.200000e-02, 3.700000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 2.260360e+01, 2.326170e+01, 2.391990e+01, 2.523620e+01 }, // SINR
          { 1, 7.996894e-01, 2.100000e-03, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 2.248200e+01, 2.315750e+01, 2.383310e+01, 2.518410e+01 }, // SINR
          { 1, 4.016720e-01, 5.630000e-02, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 2.248200e+01, 2.315750e+01, 2.383310e+01, 2.518410e+01 }, // SINR
          { 1, 3.656609e-01, 6.420000e-02, 0 } // BLER
        }
      },
      { 480U, // SINR and BLER for CBS 480
        NrEesmErrorModel::DoubleTuple{
          { 2.248200e+01, 2.315750e+01, 2.383310e+01, 2.518410e+01 }, // SINR
          { 1, 7.919255e-01, 1.540000e-02, 0 } // BLER
        }
      },
      { 528U, // SINR and BLER for CBS 528
        NrEesmErrorModel::DoubleTuple{
          { 2.257920e+01, 2.302030e+01, 2.324090e+01, 2.357180e+01, 2.390260e+01, 2.423340e+01 }, // SINR
          { 1, 8.690878e-01, 3.652174e-01, 1.407645e-01, 2.400000e-03, 6.000000e-04 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 2.255500e+01, 2.322010e+01, 2.388510e+01, 2.521530e+01 }, // SINR
          { 1, 2.081271e-01, 2.590000e-02, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 2.250520e+01, 2.317740e+01, 2.384960e+01, 2.519400e+01 }, // SINR
          { 1, 3.302872e-01, 7.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 2.320000e+01, 2.387000e+01, 2.420000e+01, 2.453000e+01, 2.520000e+01 }, // SINR
          { 1, 1.110000e-02, 4.100000e-03, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.260000e+01, 2.330000e+01, 2.373000e+01, 2.390000e+01, 2.417000e+01, 2.460000e+01 }, // SINR
          { 1, 9.990385e-01, 6.112440e-01, 7.550000e-02, 8.200000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.260360e+01, 2.326170e+01, 2.391990e+01, 2.523620e+01 }, // SINR
          { 1, 6.243961e-01, 1.390000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.243340e+01, 2.311590e+01, 2.379830e+01, 2.516320e+01 }, // SINR
          { 1, 4.791667e-01, 1.160000e-02, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 2.243340e+01, 2.311590e+01, 2.379830e+01, 2.516320e+01 }, // SINR
          { 1, 3.984375e-01, 1.770000e-02, 0 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 23, 2.320000e+01, 2.340000e+01, 2.360000e+01, 2.380000e+01, 24, 2.420000e+01, 2.440000e+01, 2.460000e+01 }, // SINR
          { 1, 9.990385e-01, 9.104610e-01, 4.210963e-01, 4.370000e-02, 1.320000e-02, 1.000000e-03, 1.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 2.270080e+01, 2.334510e+01, 2.398930e+01, 2.527780e+01 }, // SINR
          { 1, 4.013365e-01, 4.760000e-02, 0 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 2.240000e+01, 2.340000e+01, 2.403000e+01, 2.430000e+01, 2.467000e+01, 2.530000e+01 }, // SINR
          { 1, 9.990385e-01, 1.230000e-02, 1.700000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 2.336590e+01, 2.357950e+01, 2.379310e+01, 2.400670e+01, 2.464740e+01, 2.528820e+01 }, // SINR
          { 9.512868e-01, 7.350000e-02, 1.060000e-02, 9.000000e-04, 2.000000e-04, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.326170e+01, 2.370050e+01, 2.391990e+01, 2.479740e+01, 2.567490e+01 }, // SINR
          { 9.894231e-01, 3.439208e-01, 1.800000e-03, 1.600000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.267650e+01, 2.332420e+01, 2.397190e+01, 2.526740e+01 }, // SINR
          { 1, 8.577303e-01, 9.100000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.323690e+01, 2.367840e+01, 2.389920e+01, 2.411990e+01, 2.456140e+01, 2.522370e+01 }, // SINR
          { 9.675573e-01, 2.820000e-02, 6.800000e-03, 4.000000e-04, 3.000000e-04, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.330000e+01, 2.363000e+01, 2.380000e+01, 2.397000e+01, 2.430000e+01, 2.480000e+01, 2.530000e+01 }, // SINR
          { 9.687500e-01, 1.445627e-01, 5.010000e-02, 1.100000e-03, 5.000000e-04, 2.000000e-04, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.306760e+01, 2.352790e+01, 2.479380e+01, 2.605970e+01 }, // SINR
          { 9.595865e-01, 2.717742e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 23, 2.350000e+01, 2.375000e+01, 24, 2.425000e+01, 2.450000e+01 }, // SINR
          { 9.066901e-01, 2.797222e-01, 5.480000e-02, 4.700000e-03, 4.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 2.167060e+01, 2.290170e+01, 2.341470e+01, 2.351730e+01, 2.392760e+01, 2.444060e+01 }, // SINR
          { 1, 9.329710e-01, 4.142157e-01, 4.007937e-01, 4.070000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 2.207060e+01, 2.325320e+01, 2.384450e+01, 2.473150e+01 }, // SINR
          { 1, 8.994755e-01, 2.500000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 2.204560e+01, 2.323380e+01, 2.372880e+01, 2.382780e+01, 2.442190e+01, 2.501600e+01 }, // SINR
          { 1, 9.075704e-01, 8.216561e-01, 2.080000e-02, 1.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 2.313000e+01, 2.360000e+01, 2.410000e+01, 2.450000e+01, 25, 2.550000e+01 }, // SINR
          { 9.971154e-01, 6.368159e-01, 2.700000e-03, 7.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.275080e+01, 2.333100e+01, 2.352440e+01, 2.371780e+01, 2.391120e+01 }, // SINR
          { 9.826923e-01, 9.246454e-01, 1.618590e-01, 2.070000e-02, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.242060e+01, 2.352540e+01, 2.407780e+01, 2.759070e+01 }, // SINR
          { 9.750000e-01, 7.169944e-01, 3.600000e-03, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 2.180000e+01, 2.280000e+01, 2.280000e+01, 2.330000e+01, 2.380000e+01, 2.430000e+01 }, // SINR
          { 1, 9.325540e-01, 9.325540e-01, 3.050847e-01, 5.800000e-03, 4.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 2.242060e+01, 2.295280e+01, 2.348510e+01, 2.401730e+01, 2.561390e+01 }, // SINR
          { 9.951923e-01, 8.431373e-01, 6.336634e-01, 9.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.222420e+01, 2.315390e+01, 2.361870e+01, 2.408360e+01, 2.501330e+01 }, // SINR
          { 1, 8.640203e-01, 7.185754e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.293460e+01, 2.337920e+01, 2.382370e+01, 2.471270e+01 }, // SINR
          { 9.980769e-01, 7.595029e-01, 2.060000e-02, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 23, 2.340000e+01, 2.360000e+01, 2.380000e+01, 2.420000e+01, 2.480000e+01, 2.540000e+01 }, // SINR
          { 1, 7.648810e-01, 2.323070e-01, 2.060000e-02, 8.000000e-04, 2.000000e-04, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.229560e+01, 2.338780e+01, 2.371870e+01, 2.514170e+01 }, // SINR
          { 9.608209e-01, 7.016575e-01, 9.100000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.220000e+01, 2.340000e+01, 2.383000e+01, 2.427000e+01, 2.470000e+01, 26 }, // SINR
          { 1, 9.990385e-01, 4.229798e-01, 2.300000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.210000e+01, 2.340000e+01, 2.383000e+01, 2.427000e+01, 2.470000e+01, 2.590000e+01 }, // SINR
          { 1, 9.828244e-01, 3.129630e-01, 1.800000e-03, 3.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 24
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 2.424590e+01, 2.474590e+01, 2.485810e+01, 2.497030e+01, 2.508260e+01, 2.519480e+01 }, // SINR
          { 9.971154e-01, 9.128757e-02, 2.900000e-02, 5.900000e-03, 1.000000e-03, 4.000000e-04 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 2.390000e+01, 2.440000e+01, 2.457500e+01, 2.475000e+01, 2.492500e+01, 2.510000e+01, 2.560000e+01 }, // SINR
          { 1, 4.961089e-01, 3.243590e-01, 1.784703e-01, 9.053457e-02, 3.590000e-02, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 2.450000e+01, 2.470000e+01, 2.480000e+01, 25, 2.520000e+01, 2.538000e+01, 2.555000e+01 }, // SINR
          { 9.951923e-01, 8.932292e-01, 6.627604e-01, 1.365489e-01, 8.400000e-03, 4.000000e-04, 4.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 2.437570e+01, 2.474320e+01, 2.492700e+01, 2.511080e+01, 2.547830e+01 }, // SINR
          { 9.846154e-01, 5.793379e-01, 5.520000e-02, 2.580000e-02, 0 } // BLER
        }
      },
      { 480U, // SINR and BLER for CBS 480
        NrEesmErrorModel::DoubleTuple{
          { 2.402160e+01, 2.452160e+01, 2.467950e+01, 2.483750e+01, 2.499540e+01, 2.515330e+01 }, // SINR
          { 1, 4.047428e-01, 1.293590e-01, 2.060000e-02, 2.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 2.441140e+01, 2.477600e+01, 2.495830e+01, 2.514050e+01, 2.550510e+01 }, // SINR
          { 1, 3.025841e-01, 2.323070e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 2.341140e+01, 2.441000e+01, 2.541000e+01, 2.641000e+01 }, // SINR
          { 1, 6.399254e-01, 3.120000e-02, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 2.375000e+01, 2.437500e+01, 2.479170e+01, 25, 2.520830e+01, 2.562500e+01 }, // SINR
          { 1, 9.003497e-01, 2.780000e-02, 6.600000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.364070e+01, 2.437570e+01, 2.462070e+01, 2.486570e+01, 2.511070e+01, 2.584580e+01 }, // SINR
          { 1, 9.012238e-01, 8.205128e-01, 7.710843e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.395270e+01, 2.432450e+01, 2.469630e+01, 2.506810e+01 }, // SINR
          { 1, 8.784014e-01, 8.711817e-02, 3.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.428800e+01, 2.466280e+01, 2.503760e+01, 2.541240e+01, 2.653690e+01 }, // SINR
          { 1, 7.694611e-01, 6.445707e-01, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 2.410000e+01, 2.460000e+01, 2.490000e+01, 2.520000e+01, 2.550000e+01, 2.580000e+01 }, // SINR
          { 9.866412e-01, 7.143855e-01, 8.480000e-02, 1.400000e-03, 2.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 24, 2.430000e+01, 2.460000e+01, 2.490000e+01, 2.520000e+01 }, // SINR
          { 1, 9.325540e-01, 2.672140e-01, 7.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 2.405000e+01, 2.443000e+01, 2.460000e+01, 2.482000e+01, 2.520000e+01, 2.570000e+01 }, // SINR
          { 1, 5.927419e-01, 3.154613e-01, 2.487697e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 2.380000e+01, 2.430000e+01, 2.460000e+01, 2.480000e+01, 2.530000e+01, 2.610000e+01 }, // SINR
          { 1, 9.439338e-01, 5.695701e-01, 1.510000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 2.423000e+01, 2.460000e+01, 2.497000e+01, 2.570000e+01 }, // SINR
          { 1, 4.049521e-01, 1.850000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.420000e+01, 25, 2.580000e+01, 2.650000e+01, 2.730000e+01, 2.808000e+01 }, // SINR
          { 9.379562e-01, 7.385057e-01, 3.611506e-01, 1.176264e-01, 1.450000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.395480e+01, 2.427250e+01, 2.459030e+01, 2.490800e+01, 2.522580e+01, 2.572580e+01, 2.622580e+01, 2.672580e+01 }, // SINR
          { 9.064685e-01, 8.261218e-01, 7.213687e-01, 6.060427e-01, 4.712963e-01, 2.064145e-01, 6.160000e-02, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.369930e+01, 2.419930e+01, 2.444120e+01, 2.468300e+01, 2.492480e+01, 2.516670e+01, 2.566670e+01, 2.616670e+01, 2.666670e+01 }, // SINR
          { 9.443431e-01, 8.293269e-01, 7.196328e-01, 5.862269e-01, 4.605735e-01, 3.362069e-01, 5.070000e-02, 5.500000e-03, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.390000e+01, 2.440000e+01, 2.490000e+01, 2.540000e+01, 2.590000e+01 }, // SINR
          { 9.980769e-01, 4.899038e-01, 7.240000e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.388560e+01, 2.458690e+01, 2.528820e+01, 2.598950e+01, 2.669080e+01 }, // SINR
          { 9.923077e-01, 8.354839e-01, 3.965517e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 2.413330e+01, 2.437860e+01, 2.462400e+01, 2.486930e+01, 2.511460e+01, 2.561460e+01, 2.611460e+01, 2.661460e+01 }, // SINR
          { 9.649621e-01, 9.073427e-01, 7.945313e-01, 6.491117e-01, 4.537367e-01, 2.230000e-02, 1.400000e-03, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 2.423590e+01, 2.465580e+01, 2.507570e+01, 2.549550e+01, 2.591540e+01, 2.641540e+01 }, // SINR
          { 9.494485e-01, 7.514706e-01, 4.586331e-01, 1.775106e-01, 4.120000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 2.443160e+01, 2.467400e+01, 2.491640e+01, 2.515880e+01, 2.540120e+01, 2.590120e+01, 2.640120e+01 }, // SINR
          { 9.780534e-01, 9.148936e-01, 8.054687e-01, 6.452020e-01, 4.487633e-01, 7.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 2.321710e+01, 2.433260e+01, 2.544810e+01, 2.656360e+01 }, // SINR
          { 9.980769e-01, 5.762332e-01, 1.320000e-02, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 2.342650e+01, 2.392650e+01, 2.442650e+01, 2.455500e+01, 2.468350e+01, 2.481190e+01, 2.494040e+01, 2.544040e+01, 2.594040e+01 }, // SINR
          { 1, 8.986014e-01, 6.268382e-01, 5.120482e-01, 3.993711e-01, 2.831461e-01, 2.041396e-01, 4.200000e-03, 0 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.347930e+01, 2.423860e+01, 2.499800e+01, 2.575730e+01, 2.651670e+01 }, // SINR
          { 9.990385e-01, 8.500000e-01, 2.191304e-01, 5.100000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.367370e+01, 2.440530e+01, 2.513690e+01, 2.586840e+01 }, // SINR
          { 9.527778e-01, 4.031746e-01, 1.840000e-02, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 2.416440e+01, 2.445380e+01, 2.474320e+01, 2.503260e+01, 2.532200e+01, 2.582200e+01, 2.632200e+01 }, // SINR
          { 9.836538e-01, 9.066901e-01, 7.279830e-01, 4.442982e-01, 2.013221e-01, 4.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 2.414280e+01, 2.440720e+01, 2.467150e+01, 2.493590e+01, 2.520030e+01, 2.570030e+01 }, // SINR
          { 9.809160e-01, 8.887931e-01, 6.714660e-01, 4.092742e-01, 1.702557e-01, 0 } // BLER
        }
      },
      { 5120U, // SINR and BLER for CBS 5120
        NrEesmErrorModel::DoubleTuple{
          { 2.360880e+01, 2.425700e+01, 2.490530e+01, 2.555350e+01, 2.620170e+01 }, // SINR
          { 9.961538e-01, 8.295455e-01, 1.844714e-01, 4.300000e-03, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.296060e+01, 2.360880e+01, 2.425700e+01, 2.490530e+01, 2.555350e+01 }, // SINR
          { 1, 9.447464e-01, 2.847851e-01, 7.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 2.321060e+01, 2.383100e+01, 2.445150e+01, 2.507200e+01, 2.569240e+01 }, // SINR
          { 1, 9.329710e-01, 2.512450e-01, 2.400000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.349770e+01, 2.415980e+01, 2.482200e+01, 2.548410e+01 }, // SINR
          { 9.884615e-01, 3.263531e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.328330e+01, 2.378330e+01, 2.428330e+01, 2.449780e+01, 2.471240e+01, 2.492690e+01, 2.514140e+01, 2.564140e+01 }, // SINR
          { 1, 8.447712e-01, 8.026730e-01, 5.178571e-01, 2.126897e-01, 5.570000e-02, 8.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.380000e+01, 2.430000e+01, 2.447500e+01, 2.465000e+01, 2.482500e+01, 25, 2.550000e+01 }, // SINR
          { 1, 8.632550e-01, 6.974044e-01, 5.350418e-01, 3.043083e-01, 1.567559e-01, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 25
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 2.457830e+01, 2.507830e+01, 2.541190e+01, 2.574560e+01, 2.607920e+01 }, // SINR
          { 1, 3.677114e-01, 4.020000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 2.460470e+01, 2.501570e+01, 2.542670e+01, 2.583770e+01, 2.624870e+01 }, // SINR
          { 9.971154e-01, 8.724662e-01, 2.870475e-01, 9.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 2.497410e+01, 2.526260e+01, 2.555110e+01, 2.583960e+01, 2.612810e+01 }, // SINR
          { 9.479927e-01, 6.293103e-01, 1.701153e-01, 1.230000e-02, 3.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 2.468800e+01, 2.518800e+01, 2.555740e+01, 2.592670e+01, 2.629610e+01 }, // SINR
          { 1, 4.575893e-01, 3.930000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 504U, // SINR and BLER for CBS 504
        NrEesmErrorModel::DoubleTuple{
          { 2.442670e+01, 2.492670e+01, 2.542670e+01, 2.563220e+01, 2.583770e+01, 2.604330e+01, 2.624880e+01 }, // SINR
          { 9.951923e-01, 5.990566e-01, 4.497331e-01, 1.354839e-01, 1.870000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 2.450460e+01, 2.500460e+01, 2.550460e+01, 2.576010e+01, 2.601560e+01 }, // SINR
          { 1, 2.470530e-01, 7.310000e-02, 3.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 576U, // SINR and BLER for CBS 576
        NrEesmErrorModel::DoubleTuple{
          { 2.470070e+01, 2.520070e+01, 2.570070e+01, 2.582250e+01, 2.594430e+01, 2.606600e+01, 2.618780e+01 }, // SINR
          { 9.980769e-01, 7.463663e-01, 2.560000e-02, 5.000000e-03, 9.000000e-04, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 2.465270e+01, 2.515270e+01, 2.542670e+01, 2.570070e+01, 2.597470e+01, 2.624870e+01 }, // SINR
          { 9.913462e-01, 8.014241e-01, 3.101966e-01, 3.670000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 672U, // SINR and BLER for CBS 672
        NrEesmErrorModel::DoubleTuple{
          { 2.520000e+01, 2.570000e+01, 2.582500e+01, 2.595000e+01, 2.607500e+01, 2.620000e+01 }, // SINR
          { 1, 5.330000e-02, 2.160000e-02, 7.200000e-03, 2.300000e-03, 8.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.474880e+01, 2.524880e+01, 2.574880e+01, 2.624880e+01, 2.640100e+01, 2.655320e+01, 2.670550e+01, 2.685770e+01 }, // SINR
          { 9.971154e-01, 8.511513e-01, 4.300000e-03, 5.000000e-04, 3.000000e-04, 3.000000e-04, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.493280e+01, 2.553570e+01, 2.613860e+01, 2.674150e+01 }, // SINR
          { 9.895038e-01, 3.350923e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.459340e+01, 2.509340e+01, 2.559340e+01, 2.579200e+01, 2.599050e+01, 2.618910e+01, 2.638770e+01 }, // SINR
          { 9.971154e-01, 7.377168e-01, 3.419811e-01, 9.156504e-02, 1.330000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 2.480000e+01, 2.530000e+01, 2.555000e+01, 2.580000e+01, 2.605000e+01, 2.630000e+01, 2.680000e+01, 2.730000e+01 }, // SINR
          { 1, 5.743243e-01, 3.337731e-01, 1.588903e-01, 5.820000e-02, 1.680000e-02, 2.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 2.492670e+01, 2.542670e+01, 2.573500e+01, 2.604330e+01, 2.635150e+01 }, // SINR
          { 9.894231e-01, 4.927326e-01, 4.920000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 2.481020e+01, 2.542670e+01, 2.604330e+01, 2.665980e+01 }, // SINR
          { 1, 4.951362e-01, 3.800000e-03, 0 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 2.530000e+01, 2.563330e+01, 2.580000e+01, 2.596670e+01, 2.630000e+01, 2.680000e+01 }, // SINR
          { 9.980769e-01, 7.827744e-01, 3.466530e-01, 2.044715e-01, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1320U, // SINR and BLER for CBS 1320
        NrEesmErrorModel::DoubleTuple{
          { 2.475260e+01, 2.537550e+01, 2.599850e+01, 2.662140e+01 }, // SINR
          { 1, 7.971698e-01, 1.340000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.473990e+01, 2.536420e+01, 2.598860e+01, 2.661290e+01 }, // SINR
          { 1, 8.912671e-01, 2.620000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.519050e+01, 2.546250e+01, 2.573450e+01, 2.627860e+01 }, // SINR
          { 9.742366e-01, 7.558824e-01, 7.500000e-01, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.554640e+01, 2.604640e+01, 2.629640e+01, 2.654640e+01 }, // SINR
          { 9.715909e-01, 2.210000e-02, 1.290000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.480000e+01, 2.530000e+01, 2.555000e+01, 2.580000e+01, 2.630000e+01, 2.705000e+01 }, // SINR
          { 1, 8.996479e-01, 5.868056e-01, 4.910000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.527240e+01, 2.528800e+01, 2.530360e+01, 2.531910e+01, 2.565240e+01, 2.581910e+01, 2.598580e+01, 2.631910e+01 }, // SINR
          { 9.561567e-01, 9.402174e-01, 9.312500e-01, 9.169643e-01, 4.927326e-01, 3.787538e-01, 1.787234e-01, 9.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 25, 2.550000e+01, 2.575000e+01, 26, 2.625000e+01, 2.650000e+01, 27 }, // SINR
          { 9.075704e-01, 7.111111e-01, 4.362158e-01, 1.921899e-01, 5.920000e-02, 1.320000e-02, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 2.532260e+01, 2.574230e+01, 2.616200e+01, 2.658170e+01 }, // SINR
          { 9.980769e-01, 3.970126e-01, 4.700000e-03, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 2.512860e+01, 2.553090e+01, 2.593330e+01, 2.673800e+01 }, // SINR
          { 9.951923e-01, 5.594714e-01, 1.140000e-02, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 25, 2.550000e+01, 26, 2.650000e+01 }, // SINR
          { 1, 6.714660e-01, 3.500000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 2.510000e+01, 2.560000e+01, 2.610000e+01, 2.660000e+01, 2.685000e+01, 2.710000e+01, 2.735000e+01 }, // SINR
          { 1, 8.439542e-01, 1.023311e-01, 8.400000e-03, 2.300000e-03, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.550000e+01, 2.640000e+01, 2.730000e+01, 2.820000e+01, 2.920000e+01 }, // SINR
          { 9.425182e-01, 5.049801e-01, 5.370000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.470000e+01, 2.550000e+01, 2.630000e+01, 2.710000e+01, 2.790000e+01, 2.870000e+01 }, // SINR
          { 9.980769e-01, 8.818966e-01, 3.520195e-01, 2.880000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 2.550000e+01, 2.630000e+01, 2.710000e+01, 2.790000e+01, 2.870000e+01 }, // SINR
          { 9.826923e-01, 7.463450e-01, 2.012780e-01, 1.050000e-02, 4.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 2.580000e+01, 2.610000e+01, 2.640000e+01, 2.680000e+01, 2.710000e+01, 2.743000e+01, 2.775000e+01, 2.808000e+01, 2.840000e+01, 2.873000e+01, 2.905000e+01 }, // SINR
          { 9.817308e-01, 9.090909e-01, 7.858232e-01, 5.126518e-01, 2.879566e-01, 1.180000e-02, 2.500000e-03, 2.000000e-04, 2.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.450000e+01, 2.570000e+01, 2.690000e+01, 28 }, // SINR
          { 9.951923e-01, 3.594193e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.460000e+01, 2.570000e+01, 2.647000e+01, 2.690000e+01, 2.723000e+01, 28, 2.920000e+01 }, // SINR
          { 1, 9.633459e-01, 6.453046e-01, 1.484929e-01, 1.342345e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 6272U, // SINR and BLER for CBS 6272
        NrEesmErrorModel::DoubleTuple{
          { 2.440000e+01, 2.560000e+01, 2.680000e+01, 28 }, // SINR
          { 9.990385e-01, 5.019763e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.480000e+01, 2.550000e+01, 2.630000e+01, 27 }, // SINR
          { 9.533582e-01, 3.408177e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.480000e+01, 2.547500e+01, 2.615000e+01, 2.682500e+01 }, // SINR
          { 9.828244e-01, 3.266234e-01, 2.100000e-03, 0 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.450000e+01, 2.510000e+01, 2.570000e+01, 2.630000e+01, 2.690000e+01 }, // SINR
          { 1, 8.733108e-01, 1.183333e-01, 6.000000e-04, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 26
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 2.525000e+01, 2.580000e+01, 2.635000e+01, 2.690000e+01 }, // SINR
          { 1, 7.099448e-01, 4.720000e-02, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 2.610000e+01, 2.650000e+01, 2.690000e+01, 2.730000e+01 }, // SINR
          { 9.069149e-01, 5.146169e-01, 3.000000e-03, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 26, 2.680000e+01, 2.760000e+01, 2.840000e+01 }, // SINR
          { 9.366071e-01, 5.554348e-01, 9.300000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.512500e+01, 2.570000e+01, 2.627500e+01, 2.685000e+01 }, // SINR
          { 1, 6.885027e-01, 1.530000e-02, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.530000e+01, 2.610000e+01, 2.690000e+01, 2.780000e+01, 2.860000e+01, 2.943000e+01, 3.025000e+01, 3.108000e+01, 3.190000e+01, 3.273000e+01, 3.355000e+01, 3.438000e+01 }, // SINR
          { 9.990385e-01, 9.664179e-01, 8.224522e-01, 4.118852e-01, 1.212488e-01, 1.330000e-02, 1.600000e-03, 3.000000e-04, 1.000000e-04, 1.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 1032U, // SINR and BLER for CBS 1032
        NrEesmErrorModel::DoubleTuple{
          { 2.570000e+01, 2.608330e+01, 2.627500e+01, 2.646670e+01, 2.685000e+01, 2.742500e+01 }, // SINR
          { 9.476103e-01, 7.573529e-01, 1.710884e-01, 1.126682e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 1128U, // SINR and BLER for CBS 1128
        NrEesmErrorModel::DoubleTuple{
          { 2.580000e+01, 2.630000e+01, 2.680000e+01, 2.730000e+01 }, // SINR
          { 9.636194e-01, 3.590199e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 2.550000e+01, 26, 2.650000e+01, 2.665000e+01, 2.680000e+01, 2.695000e+01 }, // SINR
          { 9.778846e-01, 8.462171e-01, 2.620000e-02, 4.200000e-03, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1288U, // SINR and BLER for CBS 1288
        NrEesmErrorModel::DoubleTuple{
          { 2.580000e+01, 2.660000e+01, 2.750000e+01, 2.840000e+01 }, // SINR
          { 9.990385e-01, 7.407143e-01, 9.400000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.620000e+01, 2.640000e+01, 2.660000e+01, 2.680000e+01, 27, 2.720000e+01 }, // SINR
          { 9.990385e-01, 2.746746e-01, 7.370000e-02, 1.140000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.515000e+01, 2.602500e+01, 2.690000e+01, 2.777500e+01, 2.865000e+01 }, // SINR
          { 1, 8.050314e-01, 2.922454e-01, 3.600000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.620000e+01, 2.640000e+01, 2.660000e+01, 2.680000e+01, 27, 2.720000e+01 }, // SINR
          { 1, 9.990385e-01, 9.751908e-01, 6.268382e-01, 1.380495e-01, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.570000e+01, 2.620000e+01, 2.670000e+01, 2.690000e+01, 2.710000e+01, 2.730000e+01 }, // SINR
          { 9.598881e-01, 6.472081e-01, 8.480000e-02, 5.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.496870e+01, 2.580760e+01, 2.664660e+01, 2.748550e+01 }, // SINR
          { 1, 8.000000e-01, 2.740000e-02, 0 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 2.582390e+01, 2.663170e+01, 2.703560e+01, 2.743950e+01, 2.824730e+01 }, // SINR
          { 1, 6.002358e-01, 2.937935e-01, 6.960000e-02, 0 } // BLER
        }
      },
      { 2600U, // SINR and BLER for CBS 2600
        NrEesmErrorModel::DoubleTuple{
          { 2.530000e+01, 2.620000e+01, 2.710000e+01, 2.790000e+01, 2.880000e+01 }, // SINR
          { 9.678030e-01, 7.279830e-01, 2.356673e-01, 2.750000e-02, 3.000000e-04 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 2.579390e+01, 2.620000e+01, 2.660600e+01, 2.701210e+01, 2.741820e+01 }, // SINR
          { 9.836538e-01, 7.456395e-01, 2.061582e-01, 9.400000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 2.536180e+01, 2.617740e+01, 2.658520e+01, 2.699290e+01, 2.780850e+01 }, // SINR
          { 1, 9.457721e-01, 6.530612e-01, 4.412021e-01, 1.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 2.587930e+01, 2.637930e+01, 2.664770e+01, 2.691610e+01, 2.718450e+01 }, // SINR
          { 1, 1.284474e-01, 1.380000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 3840U, // SINR and BLER for CBS 3840
        NrEesmErrorModel::DoubleTuple{
          { 2.579320e+01, 2.629320e+01, 2.656610e+01, 2.683910e+01, 2.711200e+01, 2.738490e+01 }, // SINR
          { 9.163669e-01, 5.933180e-01, 1.979167e-01, 2.950000e-02, 1.600000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.606440e+01, 2.656440e+01, 2.676910e+01, 2.697390e+01, 2.717870e+01 }, // SINR
          { 9.148936e-01, 1.610115e-01, 3.360000e-02, 3.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4480U, // SINR and BLER for CBS 4480
        NrEesmErrorModel::DoubleTuple{
          { 2.553610e+01, 2.603610e+01, 2.630820e+01, 2.658030e+01, 2.685240e+01 }, // SINR
          { 9.942308e-01, 4.932432e-01, 9.859736e-02, 4.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 4864U, // SINR and BLER for CBS 4864
        NrEesmErrorModel::DoubleTuple{
          { 2.523640e+01, 2.573640e+01, 2.623640e+01, 2.651230e+01, 2.678820e+01, 2.706410e+01 }, // SINR
          { 1, 8.503289e-01, 2.158505e-01, 2.220000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.603220e+01, 2.653220e+01, 2.674670e+01, 2.696120e+01, 2.717570e+01, 2.739020e+01 }, // SINR
          { 9.942308e-01, 1.294964e-01, 2.270000e-02, 1.600000e-03, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 5504U, // SINR and BLER for CBS 5504
        NrEesmErrorModel::DoubleTuple{
          { 2.569670e+01, 2.610970e+01, 2.652270e+01, 2.693570e+01, 2.734870e+01 }, // SINR
          { 9.903846e-01, 6.908967e-01, 8.420000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 5632U, // SINR and BLER for CBS 5632
        NrEesmErrorModel::DoubleTuple{
          { 2.536180e+01, 2.617740e+01, 2.658520e+01, 2.699290e+01, 2.780850e+01 }, // SINR
          { 1, 3.912539e-01, 4.290000e-02, 2.000000e-03, 0 } // BLER
        }
      },
      { 6912U, // SINR and BLER for CBS 6912
        NrEesmErrorModel::DoubleTuple{
          { 2.650070e+01, 2.700070e+01, 2.750070e+01, 2.800070e+01 }, // SINR
          { 9.894231e-01, 5.865826e-01, 3.580000e-02, 0 } // BLER
        }
      },
      { 7680U, // SINR and BLER for CBS 7680
        NrEesmErrorModel::DoubleTuple{
          { 2.591740e+01, 2.641740e+01, 2.691740e+01, 2.741740e+01 }, // SINR
          { 9.971154e-01, 7.544643e-01, 8.340000e-02, 3.000000e-04 } // BLER
        }
      },
      { 8192U, // SINR and BLER for CBS 8192
        NrEesmErrorModel::DoubleTuple{
          { 2.540000e+01, 2.620000e+01, 27, 2.780000e+01 }, // SINR
          { 1, 5.447198e-01, 1.980000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 27
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 2.620000e+01, 2.690000e+01, 2.760000e+01, 2.830000e+01, 29 }, // SINR
          { 1, 9.000000e-01, 1.224515e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 640U, // SINR and BLER for CBS 640
        NrEesmErrorModel::DoubleTuple{
          { 2.635000e+01, 27, 2.760000e+01, 2.830000e+01 }, // SINR
          { 1, 3.517361e-01, 3.800000e-03, 0 } // BLER
        }
      },
      { 4096U, // SINR and BLER for CBS 4096
        NrEesmErrorModel::DoubleTuple{
          { 2.690000e+01, 2.740000e+01, 2.790000e+01, 2.840000e+01 }, // SINR
          { 1, 3.841463e-01, 1.300000e-02, 0 } // BLER
        }
      },
      { 5248U, // SINR and BLER for CBS 5248
        NrEesmErrorModel::DoubleTuple{
          { 2.690000e+01, 2.720000e+01, 2.750000e+01, 2.780000e+01, 2.810000e+01 }, // SINR
          { 9.140071e-01, 2.153253e-01, 2.360000e-02, 7.000000e-04, 0 } // BLER
        }
      }
  }
},
{ // BG TYPE 2
  { // MCS 0
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { -2.498600e+00, -2.130300e+00, -1.762000e+00, -1.393700e+00, -1.025400e+00, -6.570900e-01, -2.887500e-01, 7.958400e-02, 4.479200e-01, 8.162500e-01, 1.184600e+00, 1.552900e+00, 1.921300e+00 }, // SINR
          { 9.466912e-01, 8.896552e-01, 7.975460e-01, 6.634115e-01, 4.941860e-01, 3.452869e-01, 1.930941e-01, 8.440000e-02, 3.000000e-02, 7.600000e-03, 1.600000e-03, 4.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { -2.500000e+00, -1.600000e+00, -7.000000e-01, 3.000000e-01, 1.200000e+00, 2.130000e+00 }, // SINR
          { 9.485294e-01, 7.349138e-01, 3.572946e-01, 4.670000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { -2.700000e+00, -2, -1.300000e+00, -6.000000e-01, 1.000000e-01, 8.000000e-01, 1.500000e+00, 2.200000e+00 }, // SINR
          { 9.659091e-01, 8.664966e-01, 6.237805e-01, 3.227564e-01, 7.840000e-02, 8.800000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { -2.570000e+00, -1.900000e+00, -1.300000e+00, -6.000000e-01, 1.000000e-01, 8.000000e-01, 1.480000e+00 }, // SINR
          { 9.531250e-01, 8.274194e-01, 6.237805e-01, 3.227564e-01, 7.840000e-02, 8.800000e-03, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { -2.430000e+00, -1.800000e+00, -1.100000e+00, -5.000000e-01, 1.000000e-01, 7.000000e-01, 1.330000e+00 }, // SINR
          { 9.498175e-01, 7.970679e-01, 4.761236e-01, 1.862056e-01, 3.810000e-02, 2.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { -2.550000e+00, -1.900000e+00, -1.300000e+00, -6.000000e-01, 1.000000e-01, 7.000000e-01 }, // SINR
          { 9.696970e-01, 8.113057e-01, 5.252058e-01, 1.555143e-01, 1.670000e-02, 9.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { -2.550000e+00, -1.900000e+00, -1.200000e+00, -6.000000e-01, 1.000000e-01, 7.000000e-01 }, // SINR
          { 9.678030e-01, 8.370968e-01, 4.715485e-01, 1.459790e-01, 1.060000e-02, 4.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { -3.300000e+00, -2.300000e+00, -1.300000e+00, -3.000000e-01, 7.000000e-01 }, // SINR
          { 9.942308e-01, 9.217626e-01, 5.252058e-01, 7.000000e-02, 9.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { -2.680000e+00, -2.179200e+00, -2.110900e+00, -2.042500e+00, -1.974200e+00, -1.905900e+00, -1.410000e+00, -9.100000e-01, -4.100000e-01, 9.000000e-02, 5.900000e-01, 1.090000e+00, 1.590000e+00 }, // SINR
          { 9.645522e-01, 8.986014e-01, 8.886986e-01, 8.666667e-01, 8.504902e-01, 8.352273e-01, 6.625648e-01, 4.541815e-01, 2.359551e-01, 8.190000e-02, 1.890000e-02, 2.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { -2.200000e+00, -1.950000e+00, -1.700000e+00, -1.450000e+00, -1.200000e+00, -9.500000e-01, -7.000000e-01, -4.500000e-01, -2.000000e-01, 5.000000e-02, 3.000000e-01, 5.500000e-01, 8.000000e-01, 1.050000e+00, 1.300000e+00, 1.550000e+00, 1.800000e+00 }, // SINR
          { 9.058099e-01, 8.470395e-01, 7.765152e-01, 6.779101e-01, 5.852273e-01, 4.630474e-01, 3.572946e-01, 2.519960e-01, 1.567955e-01, 8.975904e-02, 4.670000e-02, 2.090000e-02, 8.800000e-03, 3.200000e-03, 7.000000e-04, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { -3.442700e+00, -2.028200e+00, -1.597100e+00, 2.484000e-01, 6.071200e-01 }, // SINR
          { 9.990385e-01, 8.958333e-01, 6.750000e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { -3.600000e+00, -2.200000e+00, -1.300000e+00, -9.000000e-01, -4.000000e-01, 5.000000e-01, 1.900000e+00 }, // SINR
          { 1, 9.190647e-01, 4.176325e-01, 2.453973e-01, 2.760000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 152U, // SINR and BLER for CBS 152
        NrEesmErrorModel::DoubleTuple{
          { -3.074700e+00, -1.993600e+00, -1.858000e+00, 1.685900e-01 }, // SINR
          { 9.865385e-01, 8.558333e-01, 7.589286e-01, 0 } // BLER
        }
      },
      { 160U, // SINR and BLER for CBS 160
        NrEesmErrorModel::DoubleTuple{
          { -2.153500e+00, -1.884600e+00, -1.388300e+00, -6.231700e-01, 1.419900e-01 }, // SINR
          { 9.258929e-01, 7.842988e-01, 4.824144e-01, 7.210000e-02, 0 } // BLER
        }
      },
      { 176U, // SINR and BLER for CBS 176
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -1.700000e+00, -1.100000e+00, -5.000000e-01, 1.000000e-01, 7.000000e-01 }, // SINR
          { 9.577068e-01, 7.122905e-01, 2.754881e-01, 3.360000e-02, 1.500000e-03, 4.000000e-04 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { -2.800000e+00, -2.100000e+00, -1.300000e+00, -5.000000e-01, 2.000000e-01 }, // SINR
          { 9.932692e-01, 9.122340e-01, 4.170792e-01, 4.260000e-02, 5.000000e-04 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { -2.179100e+00, -2.010800e+00, -1.842400e+00, -1.674100e+00, -1.505800e+00, -1.010000e+00, -5.100000e-01, -1.000000e-02, 4.900000e-01, 9.900000e-01, 1.490000e+00 }, // SINR
          { 9.064685e-01, 8.495066e-01, 8.089623e-01, 7.514535e-01, 6.885027e-01, 4.296075e-01, 1.949612e-01, 5.600000e-02, 9.500000e-03, 1.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { -2.260000e+00, -1.880000e+00, -1.500000e+00, -1.200000e+00, -8.000000e-01, -4.000000e-01, 0, 3.800000e-01 }, // SINR
          { 9.334532e-01, 7.865854e-01, 5.650000e-01, 3.597301e-01, 1.285971e-01, 2.700000e-02, 3.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { -3.107300e+00, -2.086300e+00, -1.573300e+00, -1.065300e+00, -4.423100e-02 }, // SINR
          { 9.971154e-01, 8.801724e-01, 4.889423e-01, 1.981132e-01, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { -2.200000e+00, -1.850000e+00, -1.500000e+00, -1.200000e+00, -8.000000e-01, -5.000000e-01, -1.000000e-01, 2.500000e-01 }, // SINR
          { 9.066901e-01, 7.820122e-01, 5.233740e-01, 2.635983e-01, 6.370000e-02, 1.500000e-02, 1.400000e-03, 6.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { -2.160000e+00, -1.830000e+00, -1.500000e+00, -1.200000e+00, -8.000000e-01, -5.000000e-01, -2.000000e-01, 1.300000e-01, 4.500000e-01, 7.800000e-01 }, // SINR
          { 9.276786e-01, 7.376453e-01, 5.753425e-01, 3.527159e-01, 1.175987e-01, 3.360000e-02, 8.600000e-03, 1.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -2.025000e+00, -1.750000e+00, -1.475000e+00, -1.200000e+00, -9.300000e-01, -6.500000e-01, -3.800000e-01, -1.000000e-01, 1.700000e-01, 4.500000e-01, 7.200000e-01, 1 }, // SINR
          { 9.267857e-01, 8.508333e-01, 7.837423e-01, 6.738281e-01, 5.264523e-01, 3.989028e-01, 2.495059e-01, 1.527947e-01, 7.180000e-02, 3.020000e-02, 1.150000e-02, 3.800000e-03, 1.000000e-03 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { -2.236300e+00, -2.064800e+00, -1.893300e+00, -1.721800e+00, -1.550400e+00, -1.050000e+00, -5.500000e-01, -5.000000e-02, 4.500000e-01, 9.500000e-01 }, // SINR
          { 9.131206e-01, 8.681973e-01, 8.181090e-01, 7.392241e-01, 6.498724e-01, 3.779674e-01, 1.398045e-01, 2.850000e-02, 3.200000e-03, 6.000000e-04 } // BLER
        }
      },
      { 528U, // SINR and BLER for CBS 528
        NrEesmErrorModel::DoubleTuple{
          { -2.700000e+00, -1.900000e+00, -1, -1.000000e-01, 7.000000e-01 }, // SINR
          { 9.932692e-01, 7.984375e-01, 1.040609e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { -2.600000e+00, -2.100000e+00, -1.500000e+00, -9.000000e-01, -3.000000e-01, 2.800000e-01, 8.500000e-01, 1.430000e+00 }, // SINR
          { 9.846154e-01, 9.064685e-01, 5.753425e-01, 1.582598e-01, 1.410000e-02, 7.600000e-03, 6.000000e-04, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -2, -1.700000e+00, -1.400000e+00, -1.100000e+00, -8.000000e-01, -5.000000e-01, -2.000000e-01, 1.000000e-01 }, // SINR
          { 9.289568e-01, 8.437500e-01, 6.844920e-01, 4.520609e-01, 2.224912e-01, 7.910000e-02, 1.950000e-02, 2.900000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { -2.210000e+00, -1.940000e+00, -1.670000e+00, -1.400000e+00, -1.100000e+00, -9.000000e-01, -6.000000e-01, -3.000000e-01, -3.000000e-02 }, // SINR
          { 9.250000e-01, 8.327922e-01, 6.015258e-01, 4.446864e-01, 2.063525e-01, 9.769167e-02, 2.460000e-02, 4.400000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { -2.400000e+00, -2.150000e+00, -1.900000e+00, -1.650000e+00, -1.400000e+00, -1.100000e+00, -9.000000e-01, -7.000000e-01, -4.000000e-01 }, // SINR
          { 9.320652e-01, 8.951049e-01, 8.031250e-01, 6.077381e-01, 3.092752e-01, 9.204728e-02, 3.030000e-02, 7.200000e-03, 5.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -2, -1.700000e+00, -1.400000e+00, -1.100000e+00, -8.000000e-01, -5.000000e-01, -2.000000e-01 }, // SINR
          { 9.580292e-01, 8.657095e-01, 6.324627e-01, 3.876534e-01, 1.702236e-01, 4.500000e-02, 6.300000e-03, 1.000000e-03 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { -2.300000e+00, -2.050000e+00, -1.800000e+00, -1.550000e+00, -1.300000e+00, -1, -8.000000e-01, -6.000000e-01, -3.000000e-01, -5.000000e-02 }, // SINR
          { 9.267857e-01, 8.665541e-01, 7.801205e-01, 5.302083e-01, 4.586331e-01, 1.794872e-01, 6.900000e-02, 2.130000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { -2.200000e+00, -1.800000e+00, -1.400000e+00, -9.000000e-01, -5.000000e-01, -7.000000e-02, 3.500000e-01 }, // SINR
          { 9.500000e-01, 7.315341e-01, 3.709440e-01, 6.570000e-02, 6.000000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { -1.980000e+00, -1.700000e+00, -1.500000e+00, -1.200000e+00, -9.000000e-01, -6.000000e-01, -3.200000e-01, -4.000000e-02 }, // SINR
          { 9.798077e-01, 6.844920e-01, 4.735130e-01, 1.855670e-01, 3.930000e-02, 3.100000e-03, 1.200000e-03, 0 } // BLER
        }
      },
      { 1320U, // SINR and BLER for CBS 1320
        NrEesmErrorModel::DoubleTuple{
          { -2.250000e+00, -1.900000e+00, -1.600000e+00, -1.200000e+00, -9.000000e-01, -5.000000e-01, -1.500000e-01 }, // SINR
          { 9.990385e-01, 8.035714e-01, 5.374473e-01, 1.789773e-01, 4.640000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { -1.880000e+00, -1.600000e+00, -1.300000e+00, -1, -8.000000e-01, -5.000000e-01, -2.200000e-01 }, // SINR
          { 9.366071e-01, 4.884615e-01, 2.185864e-01, 5.430000e-02, 1.380000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 1672U, // SINR and BLER for CBS 1672
        NrEesmErrorModel::DoubleTuple{
          { -1.750000e+00, -1.500000e+00, -1.300000e+00, -1, -8.000000e-01, -5.000000e-01, -2.500000e-01 }, // SINR
          { 9.095745e-01, 4.183168e-01, 2.258497e-01, 5.300000e-02, 1.490000e-02, 1.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { -2.191200e+00, -2.012800e+00, -1.763100e+00, -1.513300e+00, -1.381000e+00, -1.263600e+00, -1.013800e+00, -8.140000e-01, -7.641000e-01, -5.857100e-01, -4.073200e-01, -2.289300e-01, -5.054700e-02, 1.278400e-01, 3.062300e-01, 4.846200e-01, 6.630100e-01, 8.414000e-01, 1.019800e+00, 1.198200e+00, 1.376600e+00 }, // SINR
          { 9.078014e-01, 8.608333e-01, 7.924383e-01, 7.026099e-01, 6.556122e-01, 6.083333e-01, 4.956055e-01, 4.087621e-01, 3.887615e-01, 3.103554e-01, 2.305759e-01, 1.700405e-01, 1.173923e-01, 7.480000e-02, 4.670000e-02, 2.670000e-02, 1.520000e-02, 7.700000e-03, 3.400000e-03, 1.800000e-03, 9.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { -2.406200e+00, -1.906200e+00, -1.406200e+00, -9.062000e-01, -7.062000e-01, -5.062000e-01, -3.062000e-01, -2.036000e-01, -1.010000e-01, 1.600000e-03, 1.600000e-03, 2.324500e-01, 2.324500e-01, 4.633000e-01, 4.633000e-01, 6.941500e-01, 6.941500e-01, 9.250000e-01, 9.250000e-01, 1.027600e+00, 1.130200e+00, 1.232800e+00, 1.432800e+00 }, // SINR
          { 9.356884e-01, 8.314516e-01, 6.589744e-01, 4.616426e-01, 3.646132e-01, 2.684989e-01, 1.982283e-01, 1.578947e-01, 1.321278e-01, 1.012251e-01, 1.012251e-01, 5.480000e-02, 5.480000e-02, 2.930000e-02, 2.930000e-02, 1.350000e-02, 1.350000e-02, 5.000000e-03, 5.000000e-03, 3.600000e-03, 2.100000e-03, 1.300000e-03, 7.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { -1.780000e+00, -1.500000e+00, -1.200000e+00, -9.000000e-01, -7.000000e-01, -4.000000e-01, -1.300000e-01 }, // SINR
          { 9.122340e-01, 6.864865e-01, 3.586648e-01, 1.129613e-01, 3.960000e-02, 4.100000e-03, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { -2.149300e+00, -1.994400e+00, -1.775400e+00, -1.556400e+00, -1.315400e+00, -1.271500e+00, -1.090800e+00, -1.074500e+00, -9.100500e-01, -7.551400e-01, -6.002300e-01, -4.453300e-01, -2.904200e-01, -1.355200e-01, 1.938600e-02, 1.742900e-01, 3.292000e-01, 4.841100e-01, 6.390200e-01, 7.939300e-01 }, // SINR
          { 9.040493e-01, 8.534768e-01, 7.559524e-01, 6.568878e-01, 5.306017e-01, 5.059761e-01, 4.024682e-01, 3.852273e-01, 2.991706e-01, 2.178325e-01, 1.603954e-01, 1.085613e-01, 6.830000e-02, 4.070000e-02, 2.520000e-02, 1.360000e-02, 7.400000e-03, 3.300000e-03, 1.300000e-03, 8.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { -1.730000e+00, -1.300000e+00, -8.000000e-01, -4.000000e-01, 0 }, // SINR
          { 9.148936e-01, 4.642210e-01, 5.830000e-02, 2.000000e-03, 0 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { -1.830000e+00, -1.300000e+00, -8.000000e-01, -2.000000e-01, 3.000000e-01 }, // SINR
          { 9.093310e-01, 4.325601e-01, 4.100000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { -1.680000e+00, -1.300000e+00, -9.000000e-01, -5.000000e-01, -2.000000e-01, 2.000000e-01 }, // SINR
          { 9.303571e-01, 4.399123e-01, 7.100000e-02, 3.800000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 3240U, // SINR and BLER for CBS 3240
        NrEesmErrorModel::DoubleTuple{
          { -3.405100e+00, -1.965600e+00, -1.245800e+00, -5.260300e-01, 9.135000e-01 }, // SINR
          { 1, 8.285256e-01, 4.143443e-01, 2.100000e-03, 0 } // BLER
        }
      },
      { 3624U, // SINR and BLER for CBS 3624
        NrEesmErrorModel::DoubleTuple{
          { -2.144400e+00, -1.250000e+00, -3.555700e-01, 1.433300e+00 }, // SINR
          { 9.788462e-01, 4.418403e-01, 2.900000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 1
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, 0, 7.000000e-01, 1.400000e+00, 2.100000e+00 }, // SINR
          { 9.330357e-01, 4.284512e-01, 2.370000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { -1.200000e+00, -4.000000e-01, 4.000000e-01, 1.200000e+00, 2.100000e+00 }, // SINR
          { 1, 8.758503e-01, 2.364232e-01, 4.400000e-03, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, 2.000000e-01, 1.200000e+00, 2.100000e+00 }, // SINR
          { 9.828244e-01, 4.431818e-01, 2.600000e-03, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, 0, 7.000000e-01, 1.300000e+00, 2 }, // SINR
          { 9.903846e-01, 5.854358e-01, 3.100000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { -9.037900e-01, -2.847700e-01, 3.342500e-01, 9.532700e-01, 1.572300e+00, 2.191300e+00 }, // SINR
          { 1, 7.507310e-01, 3.761161e-01, 9.209992e-02, 6.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { -5.981000e-01, 9.245000e-02, 7.830000e-01, 1.473500e+00, 2.164100e+00 }, // SINR
          { 9.406934e-01, 6.113095e-01, 1.542848e-01, 9.300000e-03, 4.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { -1.047100e+00, -4.076100e-01, 2.319200e-01, 8.714500e-01, 1.511000e+00, 2.150500e+00 }, // SINR
          { 1, 8.600993e-01, 4.368557e-01, 8.436749e-02, 5.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { -8.000000e-01, -1.000000e-01, 6.000000e-01, 1.400000e+00 }, // SINR
          { 9.856870e-01, 5.810502e-01, 3.350000e-02, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { -1.100000e+00, -3.000000e-01, 4.000000e-01, 1.200000e+00, 2 }, // SINR
          { 9.990385e-01, 8.330645e-01, 1.780410e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { -9.559100e-01, -4.559100e-01, -9.242400e-02, 2.710600e-01, 6.345500e-01, 9.980300e-01, 1.498000e+00 }, // SINR
          { 1, 7.804878e-01, 4.837786e-01, 2.024960e-01, 5.210000e-02, 7.100000e-03, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { -6.029000e-01, -2.192500e-01, 1.644000e-01, 5.480500e-01, 9.317000e-01, 1.431700e+00 }, // SINR
          { 9.102113e-01, 6.825397e-01, 3.423025e-01, 9.662829e-02, 1.280000e-02, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { -8.529000e-01, -3.529000e-01, -1.523200e-02, 3.224400e-01, 6.601000e-01, 9.977700e-01, 1.497800e+00 }, // SINR
          { 1, 7.294034e-01, 4.480634e-01, 1.678380e-01, 3.410000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { -8.281500e-01, -1.514900e-01, 5.251800e-01, 1.201800e+00, 1.878500e+00 }, // SINR
          { 9.846154e-01, 6.875000e-01, 9.423708e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { -4.634300e-01, -3.808400e-01, -2.982400e-01, -2.156500e-01, -1.330600e-01, 3.669400e-01, 8.669400e-01 }, // SINR
          { 9.320652e-01, 9.005282e-01, 8.468543e-01, 7.960938e-01, 7.201705e-01, 2.100000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { -6.572400e-01, -1.572400e-01, 2.808700e-01, 7.189800e-01, 1.157100e+00, 1.595200e+00 }, // SINR
          { 1, 6.522843e-01, 1.895708e-01, 1.310000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { -3.709700e-01, 1.864700e-02, 4.082600e-01, 7.978800e-01, 1.187500e+00 }, // SINR
          { 9.110915e-01, 5.569690e-01, 1.546818e-01, 1.470000e-02, 7.000000e-04 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { -2.821100e-01, -2.447000e-01, -2.072900e-01, -1.698900e-01, -1.324800e-01, 3.675200e-01, 8.675200e-01 }, // SINR
          { 9.219858e-01, 9.084507e-01, 8.896552e-01, 8.657095e-01, 8.354839e-01, 1.940000e-02, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { -4.202000e-01, 5.120000e-02, 5.226000e-01, 9.940000e-01, 1.465400e+00 }, // SINR
          { 9.503676e-01, 6.425879e-01, 1.514510e-01, 7.500000e-03, 2.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { -3.087300e-01, -1.668800e-01, -2.502600e-02, 1.168200e-01, 6.168200e-01, 1.116800e+00 }, // SINR
          { 9.595865e-01, 9.082168e-01, 8.176752e-01, 6.815160e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { -8.000000e-01, -2.000000e-01, 5.000000e-01, 1.100000e+00, 1.700000e+00 }, // SINR
          { 9.780534e-01, 6.177184e-01, 3.500000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { -5.088500e-01, -8.853100e-03, 3.207600e-01, 6.503700e-01, 9.799900e-01, 1.309600e+00 }, // SINR
          { 1, 6.747382e-01, 2.725806e-01, 4.760000e-02, 2.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { -4.850500e-01, 1.495000e-02, 3.256400e-01, 6.363300e-01, 9.470100e-01, 1.257700e+00 }, // SINR
          { 1, 7.145833e-01, 3.134282e-01, 6.260000e-02, 4.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { -3.218500e-01, -2.361100e-01, -1.503600e-01, -6.461900e-02, 2.112500e-02, 5.211200e-01, 1.021100e+00 }, // SINR
          { 9.580224e-01, 9.205357e-01, 8.649329e-01, 7.984375e-01, 7.176966e-01, 2.300000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { -5.899000e-01, -1.539700e-01, 2.819500e-01, 7.178700e-01, 1.153800e+00 }, // SINR
          { 9.942308e-01, 8.258065e-01, 2.545455e-01, 9.100000e-03, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { -5.281000e-01, -2.810300e-02, 2.414200e-01, 5.109500e-01, 7.804700e-01, 1.050000e+00 }, // SINR
          { 1, 6.752646e-01, 2.627339e-01, 3.970000e-02, 2.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { -6.140700e-01, -1.140700e-01, 1.509900e-01, 4.160400e-01, 6.811000e-01, 9.461500e-01 }, // SINR
          { 1, 6.440355e-01, 2.219790e-01, 3.140000e-02, 1.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { -5.025700e-01, -2.573300e-03, 2.874000e-01, 5.773600e-01, 8.673300e-01, 1.157300e+00 }, // SINR
          { 1, 6.412500e-01, 1.865699e-01, 1.600000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 888U, // SINR and BLER for CBS 888
        NrEesmErrorModel::DoubleTuple{
          { -6.367800e-01, -2.154600e-01, 2.058600e-01, 6.271800e-01, 1.048500e+00 }, // SINR
          { 1, 8.975694e-01, 2.616944e-01, 7.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { -8.928000e-01, -3.928000e-01, 1.072000e-01, 3.153100e-01, 5.234100e-01, 7.315200e-01, 9.396200e-01 }, // SINR
          { 9.570896e-01, 8.486842e-01, 3.981191e-01, 1.189516e-01, 1.760000e-02, 1.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { -9.168100e-01, -4.168100e-01, 8.318800e-02, 2.700800e-01, 4.569800e-01, 6.438700e-01, 8.307700e-01 }, // SINR
          { 9.855769e-01, 8.070312e-01, 4.783835e-01, 1.760168e-01, 3.640000e-02, 3.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { -1, -1.250000e-01, 7.500000e-01, 1.625000e+00, 2.500000e+00 }, // SINR
          { 9.903846e-01, 7.536550e-01, 1.698241e-01, 4.800000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { -1, -5.000000e-01, 0, 2.257700e-01, 4.515400e-01, 6.773100e-01, 9.030800e-01 }, // SINR
          { 9.865385e-01, 8.853448e-01, 7.799080e-01, 3.503472e-01, 6.050000e-02, 4.000000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { -3.746200e-01, -2.462000e-02, 3.253800e-01, 6.753800e-01 }, // SINR
          { 9.826923e-01, 6.337065e-01, 7.000000e-02, 8.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { -8.000000e-01, -1.000000e-01, 6.000000e-01, 1.300000e+00, 2 }, // SINR
          { 9.411232e-01, 5.887097e-01, 1.322674e-01, 6.300000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { -3.000000e-01, -4.000000e-02, 2.200000e-01, 4.800000e-01, 7.400000e-01 }, // SINR
          { 9.580224e-01, 6.475000e-01, 1.455440e-01, 8.600000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { -3.000000e-01, 0, 2.000000e-01, 5.000000e-01, 7.000000e-01, 9.500000e-01, 1.200000e+00, 1.450000e+00, 1.700000e+00, 1.950000e+00 }, // SINR
          { 9.490741e-01, 7.625740e-01, 5.223029e-01, 1.955928e-01, 6.110000e-02, 3.450000e-02, 9.600000e-03, 1.700000e-03, 6.000000e-04, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { -8.000000e-01, -2.500000e-02, 7.500000e-01, 1.525000e+00, 2.300000e+00 }, // SINR
          { 9.633459e-01, 5.731027e-01, 7.960000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { -2.330800e-01, 6.592000e-02, 3.649200e-01, 6.639200e-01 }, // SINR
          { 9.093310e-01, 3.504155e-01, 2.130000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { -4.707700e-01, -8.800200e-02, 2.947700e-01, 6.775300e-01 }, // SINR
          { 9.961538e-01, 6.901882e-01, 5.530000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { -2.884600e-01, -1.372200e-02, 2.610200e-01, 5.357500e-01, 8.104900e-01 }, // SINR
          { 9.564815e-01, 5.269710e-01, 6.250000e-02, 1.100000e-03, 2.000000e-04 } // BLER
        }
      },
      { 2664U, // SINR and BLER for CBS 2664
        NrEesmErrorModel::DoubleTuple{
          { -9.526700e-01, -4.526700e-01, 4.733100e-02, 4.729200e-01, 8.985200e-01 }, // SINR
          { 9.855769e-01, 8.750000e-01, 6.645078e-01, 1.630000e-02, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, -1.000000e-01, 6.000000e-01, 1.200000e+00 }, // SINR
          { 9.942308e-01, 7.765152e-01, 4.330000e-02, 0 } // BLER
        }
      },
      { 3368U, // SINR and BLER for CBS 3368
        NrEesmErrorModel::DoubleTuple{
          { -3.562800e-01, -5.844200e-02, 2.393900e-01, 7.393900e-01, 1.239400e+00, 1.739400e+00 }, // SINR
          { 9.971154e-01, 8.590604e-01, 2.715517e-01, 6.380000e-02, 3.400000e-03, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { -7.000000e-01, -1.000000e-01, 6.000000e-01, 1.200000e+00, 1.900000e+00 }, // SINR
          { 9.971154e-01, 7.522059e-01, 3.820000e-02, 2.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 2
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 1, 1.800000e+00, 2.200000e+00, 2.600000e+00, 3.400000e+00 }, // SINR
          { 1, 3.083942e-01, 6.770000e-02, 3.800000e-03, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 6.531000e-01, 1.265400e+00, 1.877700e+00, 2.490000e+00, 3.102300e+00 }, // SINR
          { 9.894231e-01, 8.101266e-01, 3.037861e-01, 2.580000e-02, 9.000000e-04 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 9.961000e-01, 1.516900e+00, 2.037700e+00, 2.558600e+00, 3.079400e+00 }, // SINR
          { 9.466912e-01, 6.472081e-01, 1.600064e-01, 9.300000e-03, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 6.000000e-01, 1.200000e+00, 1.630000e+00, 1.800000e+00, 2.070000e+00, 2.500000e+00, 3.100000e+00 }, // SINR
          { 1, 9.913462e-01, 3.981918e-01, 3.757440e-01, 5.020000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 5.000000e-01, 1.275000e+00, 2.050000e+00, 2.825000e+00, 3.600000e+00 }, // SINR
          { 1, 8.077532e-01, 5.820000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 7.407700e-01, 1.458000e+00, 2.175200e+00, 2.892500e+00, 3.609700e+00, 4.326900e+00 }, // SINR
          { 9.456522e-01, 6.664508e-01, 2.512425e-01, 3.420000e-02, 2.200000e-03, 2.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 8.217700e-01, 1.448300e+00, 2.074800e+00, 2.701300e+00, 3.327900e+00, 3.954400e+00 }, // SINR
          { 9.356618e-01, 7.566568e-01, 3.660201e-01, 7.700000e-02, 5.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 4.386000e-01, 9.386000e-01, 1.438600e+00, 1.706700e+00, 1.974700e+00, 2.242800e+00, 2.510800e+00, 3.010800e+00, 3.510800e+00, 4.010800e+00 }, // SINR
          { 1, 8.818493e-01, 7.814417e-01, 6.330446e-01, 4.438596e-01, 2.666490e-01, 1.283129e-01, 2.070000e-02, 3.000000e-03, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 3.870000e-01, 1.020900e+00, 1.654800e+00, 2.288700e+00, 2.922600e+00, 3.556500e+00 }, // SINR
          { 9.913462e-01, 8.862847e-01, 5.561674e-01, 1.506295e-01, 1.270000e-02, 4.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 5.546200e-01, 1.149700e+00, 1.744800e+00, 2.339900e+00, 2.934900e+00, 3.530000e+00 }, // SINR
          { 9.617537e-01, 8.724662e-01, 4.995079e-01, 1.113090e-01, 7.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 5.828800e-01, 1.167000e+00, 1.751200e+00, 2.335300e+00, 2.919400e+00, 3.503500e+00 }, // SINR
          { 9.699248e-01, 8.775510e-01, 4.819392e-01, 1.005390e-01, 6.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 1.888300e-01, 1.010800e+00, 1.832900e+00, 2.654900e+00, 3.476900e+00 }, // SINR
          { 9.990385e-01, 8.767007e-01, 2.725054e-01, 9.100000e-03, 2.000000e-04 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 8.031500e-01, 1.332600e+00, 1.862000e+00, 2.391500e+00, 2.920900e+00, 3.450400e+00 }, // SINR
          { 9.713740e-01, 8.258065e-01, 3.673692e-01, 6.150000e-02, 3.100000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 9.113000e-01, 1.413800e+00, 1.916300e+00, 2.418800e+00, 2.921300e+00, 3.423800e+00 }, // SINR
          { 9.049296e-01, 6.087740e-01, 1.741690e-01, 1.740000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.005300e+00, 1.505300e+00, 1.978300e+00, 2.451300e+00, 2.924300e+00, 3.397300e+00 }, // SINR
          { 9.082168e-01, 6.987705e-01, 2.348414e-01, 2.640000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 9.621000e-01, 1.462100e+00, 1.939300e+00, 2.416500e+00, 2.893600e+00, 3.370800e+00 }, // SINR
          { 9.303571e-01, 6.988950e-01, 2.400568e-01, 2.920000e-02, 1.200000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 9.607000e-01, 1.460700e+00, 1.924900e+00, 2.389200e+00, 2.853400e+00, 3.317700e+00 }, // SINR
          { 9.847328e-01, 5.980047e-01, 1.545567e-01, 1.480000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 9.645000e-01, 1.464500e+00, 1.914500e+00, 2.364500e+00, 2.814600e+00, 3.264600e+00 }, // SINR
          { 9.555556e-01, 6.354680e-01, 1.597577e-01, 1.090000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 8.037500e-01, 1.320600e+00, 1.837500e+00, 2.354300e+00, 2.871200e+00 }, // SINR
          { 9.971154e-01, 8.835616e-01, 3.497230e-01, 3.010000e-02, 4.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 5.414500e-01, 1.195700e+00, 1.850000e+00, 2.504200e+00, 3.158500e+00 }, // SINR
          { 9.990385e-01, 8.503289e-01, 1.541411e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 9.441000e-01, 1.444100e+00, 1.859400e+00, 2.274700e+00, 2.690100e+00, 3.105400e+00 }, // SINR
          { 9.577206e-01, 6.445707e-01, 1.614516e-01, 1.190000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 8.758500e-01, 1.336700e+00, 1.797500e+00, 2.258400e+00, 2.719200e+00 }, // SINR
          { 9.951923e-01, 8.707770e-01, 3.783482e-01, 4.420000e-02, 7.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.297700e+00, 1.754700e+00, 2.211600e+00, 2.668600e+00, 3.168600e+00 }, // SINR
          { 9.598881e-01, 6.219512e-01, 1.377888e-01, 8.000000e-03, 0 } // BLER
        }
      },
      { 480U, // SINR and BLER for CBS 480
        NrEesmErrorModel::DoubleTuple{
          { 1.011600e+00, 1.511600e+00, 1.870200e+00, 2.228900e+00, 2.587500e+00, 2.946200e+00 }, // SINR
          { 9.826923e-01, 8.880208e-01, 4.879344e-01, 1.140210e-01, 8.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.184500e+00, 1.684500e+00, 1.973400e+00, 2.262300e+00, 2.551100e+00, 2.840000e+00 }, // SINR
          { 9.307554e-01, 6.699219e-01, 2.683902e-01, 4.330000e-02, 2.300000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 6.791000e-01, 1.179100e+00, 1.679100e+00, 1.942800e+00, 2.206500e+00, 2.470100e+00, 2.733800e+00 }, // SINR
          { 9.971154e-01, 8.871528e-01, 7.791411e-01, 3.753698e-01, 9.079903e-02, 8.700000e-03, 4.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.146900e+00, 1.646900e+00, 1.892100e+00, 2.137300e+00, 2.382500e+00, 2.627700e+00 }, // SINR
          { 9.740385e-01, 6.824866e-01, 2.976051e-01, 6.150000e-02, 5.500000e-03, 5.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.375000e+00, 1.875000e+00, 2.113800e+00, 2.352600e+00, 2.591500e+00, 2.830300e+00 }, // SINR
          { 9.255319e-01, 3.149876e-01, 6.470000e-02, 5.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.199800e+00, 1.579600e+00, 1.959500e+00, 2.339300e+00, 2.719200e+00 }, // SINR
          { 9.894231e-01, 7.704545e-01, 1.727335e-01, 4.500000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.250000e+00, 1.750000e+00, 1.964600e+00, 2.179100e+00, 2.393700e+00, 2.608200e+00 }, // SINR
          { 9.759615e-01, 4.340753e-01, 1.343884e-01, 1.630000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.320800e+00, 1.614900e+00, 1.909000e+00, 2.203100e+00, 2.497200e+00 }, // SINR
          { 9.951923e-01, 8.176752e-01, 3.168970e-01, 2.730000e-02, 2.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.250000e+00, 1.750000e+00, 1.981300e+00, 2.212700e+00, 2.444000e+00, 2.675400e+00 }, // SINR
          { 9.742366e-01, 5.652902e-01, 1.900452e-01, 2.880000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.325600e+00, 1.605100e+00, 1.884600e+00, 2.164100e+00, 2.443600e+00 }, // SINR
          { 9.846154e-01, 8.197115e-01, 3.145161e-01, 3.190000e-02, 1.000000e-03 } // BLER
        }
      },
      { 1480U, // SINR and BLER for CBS 1480
        NrEesmErrorModel::DoubleTuple{
          { 1.402600e+00, 1.672300e+00, 1.942100e+00, 2.211800e+00, 2.481500e+00 }, // SINR
          { 9.713740e-01, 7.161017e-01, 2.256261e-01, 1.560000e-02, 3.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.460000e+00, 1.720000e+00, 1.980000e+00, 2.240000e+00, 2.500000e+00 }, // SINR
          { 9.583333e-01, 6.274510e-01, 1.480882e-01, 8.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 9.979000e-01, 1.497900e+00, 1.685600e+00, 1.873300e+00, 2.061000e+00, 2.248700e+00, 2.748700e+00 }, // SINR
          { 9.971154e-01, 8.835034e-01, 5.683333e-01, 1.904223e-01, 3.080000e-02, 2.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.516400e+00, 1.684700e+00, 1.853100e+00, 2.021400e+00, 2.189800e+00, 2.689800e+00 }, // SINR
          { 9.151786e-01, 6.752646e-01, 3.225703e-01, 8.060000e-02, 1.050000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.284600e+00, 1.502600e+00, 1.720700e+00, 1.938700e+00, 2.156700e+00, 2.656700e+00 }, // SINR
          { 9.932692e-01, 9.110915e-01, 5.755682e-01, 1.538929e-01, 1.550000e-02, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.273800e+00, 1.544600e+00, 1.815400e+00, 2.086200e+00, 2.357000e+00 }, // SINR
          { 9.971154e-01, 8.558333e-01, 3.418022e-01, 3.020000e-02, 3.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.243600e+00, 1.587400e+00, 1.931200e+00, 2.274900e+00, 2.618700e+00 }, // SINR
          { 9.971154e-01, 8.023438e-01, 1.153846e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.472800e+00, 1.716700e+00, 1.960600e+00, 2.204500e+00, 2.448400e+00 }, // SINR
          { 9.589552e-01, 6.387500e-01, 1.453993e-01, 5.600000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.373700e+00, 1.873700e+00, 2.282500e+00, 2.691300e+00 }, // SINR
          { 9.694656e-01, 3.695175e-01, 2.500000e-03, 0 } // BLER
        }
      },
      { 3368U, // SINR and BLER for CBS 3368
        NrEesmErrorModel::DoubleTuple{
          { 1.166800e+00, 1.524500e+00, 1.882200e+00, 2.239900e+00, 2.597600e+00 }, // SINR
          { 1, 8.986014e-01, 1.873145e-01, 8.000000e-04, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 9.700000e-01, 1.470400e+00, 1.680100e+00, 1.889900e+00, 2.099600e+00, 2.309300e+00, 2.810000e+00, 3.310000e+00 }, // SINR
          { 9.356884e-01, 6.472081e-01, 4.377148e-01, 2.335343e-01, 1.075881e-01, 4.230000e-02, 7.000000e-03, 9.000000e-04 } // BLER
        }
      }
  },
  { // MCS 3
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 2.838100e+00, 3.198200e+00, 3.558200e+00, 3.918300e+00 }, // SINR
          { 9.961538e-01, 6.355198e-01, 6.550000e-02, 1.000000e-04 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 2.250000e+00, 2.800000e+00, 3.300000e+00, 3.900000e+00, 4.400000e+00, 5, 5.550000e+00, 6.100000e+00 }, // SINR
          { 9.574074e-01, 8.412829e-01, 6.035714e-01, 2.560729e-01, 6.340000e-02, 5.600000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 2.533500e+00, 3.158500e+00, 3.575200e+00, 3.783500e+00, 3.991800e+00 }, // SINR
          { 1, 9.145683e-01, 3.040000e-02, 2.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 3, 3.348300e+00, 3.696700e+00, 4.045000e+00 }, // SINR
          { 9.561567e-01, 2.418893e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 2.999200e+00, 3.335900e+00, 3.672600e+00, 4.009300e+00 }, // SINR
          { 9.923077e-01, 5.985915e-01, 1.890000e-02, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 2.871100e+00, 3.150000e+00, 3.429000e+00, 3.707900e+00, 3.986800e+00 }, // SINR
          { 9.942308e-01, 7.643072e-01, 1.165037e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 3.140300e+00, 3.418800e+00, 3.697200e+00, 3.975700e+00 }, // SINR
          { 9.715909e-01, 4.765918e-01, 2.920000e-02, 2.000000e-04 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 3.041800e+00, 3.272500e+00, 3.503200e+00, 3.733900e+00, 3.964600e+00 }, // SINR
          { 9.971154e-01, 8.624161e-01, 3.202532e-01, 2.340000e-02, 2.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 3.022800e+00, 3.252700e+00, 3.482500e+00, 3.712400e+00, 3.942300e+00 }, // SINR
          { 9.913462e-01, 8.368506e-01, 2.703426e-01, 1.330000e-02, 3.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 2.862500e+00, 3.215000e+00, 3.567500e+00, 3.920000e+00 }, // SINR
          { 1, 7.914110e-01, 4.860000e-02, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 2.695100e+00, 3.208600e+00, 3.722000e+00, 4.235500e+00 }, // SINR
          { 9.375000e-01, 3.652457e-01, 1.500000e-02, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 2.800000e+00, 3.330000e+00, 3.600000e+00, 3.870000e+00, 4.400000e+00 }, // SINR
          { 1, 3.073422e-01, 1.450000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 2.200000e+00, 2.870000e+00, 3.200000e+00, 3.530000e+00, 4.200000e+00 }, // SINR
          { 1, 9.636194e-01, 5.825688e-01, 1.036534e-01, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 2.400000e+00, 3.200000e+00, 3.900000e+00, 4.700000e+00 }, // SINR
          { 1, 6.026995e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 2.400000e+00, 3.100000e+00, 3.800000e+00, 4.600000e+00 }, // SINR
          { 1, 7.810606e-01, 5.600000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 2.970000e+00, 3.200000e+00, 3.430000e+00, 3.900000e+00, 4.600000e+00 }, // SINR
          { 1, 7.919207e-01, 6.021127e-01, 4.114078e-01, 9.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 2.970000e+00, 3.200000e+00, 3.430000e+00, 3.900000e+00, 4.600000e+00 }, // SINR
          { 1, 8.673986e-01, 6.021127e-01, 8.950000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 2.331300e+00, 2.831300e+00, 2.894800e+00, 2.958400e+00, 3.021900e+00, 3.085400e+00, 3.585400e+00, 4.085400e+00 }, // SINR
          { 1, 8.054687e-01, 7.732036e-01, 7.335714e-01, 6.970109e-01, 6.505102e-01, 5.860000e-02, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 2.349000e+00, 2.849000e+00, 2.896300e+00, 2.943600e+00, 2.990900e+00, 3.038100e+00, 3.538100e+00, 4.038100e+00 }, // SINR
          { 1, 8.610197e-01, 8.457792e-01, 8.282258e-01, 8.078125e-01, 7.835366e-01, 7.440000e-02, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 2.492800e+00, 3.148400e+00, 3.804100e+00, 4.459700e+00 }, // SINR
          { 9.884615e-01, 4.990196e-01, 1.350000e-02, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 2.400000e+00, 2.870000e+00, 3.100000e+00, 3.330000e+00, 3.800000e+00, 4.600000e+00 }, // SINR
          { 1, 9.673507e-01, 6.617268e-01, 2.973529e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 2.444300e+00, 3.086200e+00, 3.728100e+00, 4.370000e+00 }, // SINR
          { 9.903846e-01, 5.810502e-01, 1.470000e-02, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 2.700000e+00, 3.200000e+00, 3.700000e+00, 4.200000e+00 }, // SINR
          { 9.980769e-01, 5.495690e-01, 7.100000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 2.480500e+00, 3.005900e+00, 3.531300e+00, 4.056700e+00, 4.582100e+00 }, // SINR
          { 9.923077e-01, 7.274011e-01, 9.588353e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 2.500000e+00, 3, 3.500000e+00, 4, 4.500000e+00 }, // SINR
          { 1, 9.001736e-01, 2.432171e-01, 3.500000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.250000e+00, 2.750000e+00, 3.250000e+00, 3.487900e+00, 3.725800e+00, 3.963600e+00, 4.201500e+00 }, // SINR
          { 1, 8.767241e-01, 3.118842e-01, 1.296488e-01, 3.950000e-02, 7.700000e-03, 8.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.349300e+00, 2.864500e+00, 3.379800e+00, 3.895100e+00, 4.410300e+00 }, // SINR
          { 9.990385e-01, 9.402574e-01, 3.182957e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.763000e+00, 3.276100e+00, 3.789100e+00, 4.302100e+00 }, // SINR
          { 9.753788e-01, 3.931327e-01, 6.500000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 2.683800e+00, 3.187100e+00, 3.690500e+00, 4.193800e+00 }, // SINR
          { 9.951923e-01, 6.025943e-01, 2.870000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 2.816700e+00, 3.239700e+00, 3.662600e+00, 4.085600e+00 }, // SINR
          { 9.817308e-01, 5.452128e-01, 3.090000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 2.755100e+00, 3.260300e+00, 3.765400e+00, 4.270600e+00 }, // SINR
          { 9.942308e-01, 5.214286e-01, 9.200000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 2.700900e+00, 3.186500e+00, 3.672100e+00, 4.157700e+00 }, // SINR
          { 1, 6.237805e-01, 2.200000e-02, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 2.536900e+00, 2.885600e+00, 3.234400e+00, 3.583100e+00, 3.931800e+00 }, // SINR
          { 9.642857e-01, 6.854839e-01, 2.186411e-01, 2.430000e-02, 7.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.825000e+00, 3.250000e+00, 3.675000e+00, 4.100000e+00 }, // SINR
          { 9.751908e-01, 5.029880e-01, 3.290000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.600000e+00, 2.975000e+00, 3.350000e+00, 3.725000e+00, 4.100000e+00 }, // SINR
          { 1, 9.055944e-01, 3.433243e-01, 1.570000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.700000e+00, 3.025000e+00, 3.350000e+00, 3.675000e+00, 4 }, // SINR
          { 9.961538e-01, 7.906442e-01, 1.835029e-01, 3.400000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 2.700000e+00, 3.025000e+00, 3.350000e+00, 3.675000e+00, 4 }, // SINR
          { 1, 8.682886e-01, 3.073171e-01, 2.210000e-02, 2.000000e-04 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 2.300000e+00, 2.800000e+00, 3.300000e+00, 3.800000e+00, 4.300000e+00, 4.800000e+00 }, // SINR
          { 9.903846e-01, 7.529412e-01, 3.361037e-01, 4.540000e-02, 2.200000e-03, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 2.798500e+00, 3.046100e+00, 3.293800e+00, 3.541500e+00, 3.789100e+00, 4.289100e+00 }, // SINR
          { 9.884615e-01, 8.886986e-01, 5.258264e-01, 1.362527e-01, 1.090000e-02, 7.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 2.399800e+00, 2.899800e+00, 3.399800e+00, 3.574700e+00, 3.749600e+00, 3.924400e+00, 4.099300e+00 }, // SINR
          { 1, 7.085635e-01, 3.091299e-01, 8.600000e-02, 1.280000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 2.848100e+00, 3.035600e+00, 3.223000e+00, 3.410400e+00, 3.910400e+00, 4.410400e+00 }, // SINR
          { 9.875000e-01, 9.187500e-01, 6.957418e-01, 3.355438e-01, 3.140000e-02, 9.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 2.315000e+00, 2.815000e+00, 3.315000e+00, 3.694700e+00, 4.074400e+00 }, // SINR
          { 1, 8.066038e-01, 4.122951e-01, 1.600000e-02, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 2.729900e+00, 3.085800e+00, 3.441700e+00, 3.797600e+00, 4.153500e+00 }, // SINR
          { 9.932692e-01, 7.604167e-01, 1.045047e-01, 9.000000e-04, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 2.830800e+00, 2.974700e+00, 3.118500e+00, 3.262400e+00, 3.406200e+00, 3.906200e+00, 4.406200e+00 }, // SINR
          { 9.675573e-01, 8.640940e-01, 6.274510e-01, 3.277202e-01, 1.112222e-01, 2.100000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 4
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 4.398200e+00, 4.752800e+00, 5.107400e+00, 5.462000e+00, 5.816600e+00, 6.171200e+00, 6.525800e+00, 6.880400e+00 }, // SINR
          { 9.942308e-01, 6.949728e-01, 3.968750e-01, 9.755592e-02, 2.610000e-02, 4.800000e-03, 6.000000e-04, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 4.379600e+00, 4.733100e+00, 5.086600e+00, 5.440100e+00, 5.793600e+00, 6.147100e+00, 6.500600e+00 }, // SINR
          { 9.903846e-01, 6.838235e-01, 2.619048e-01, 1.209016e-01, 1.100000e-02, 8.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 4.370600e+00, 4.723500e+00, 5.076400e+00, 5.429300e+00, 5.782200e+00, 6.135100e+00, 6.488000e+00 }, // SINR
          { 9.990385e-01, 6.677632e-01, 3.286554e-01, 1.213768e-01, 1.150000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 4.361600e+00, 4.713900e+00, 5.066200e+00, 5.418500e+00, 5.770800e+00, 6.123100e+00, 6.475400e+00 }, // SINR
          { 1, 7.191011e-01, 3.517361e-01, 7.070000e-02, 1.100000e-02, 1.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 4.343000e+00, 4.694200e+00, 5.045400e+00, 5.396600e+00, 5.747800e+00, 6.099000e+00 }, // SINR
          { 1, 8.965517e-01, 2.552817e-01, 8.450000e-02, 6.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 4.075000e+00, 4.641700e+00, 4.925000e+00, 5.208300e+00, 5.775000e+00, 6.625000e+00 }, // SINR
          { 9.375000e-01, 4.675926e-01, 1.975552e-01, 5.670000e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 3.958200e+00, 4.307000e+00, 4.655800e+00, 5.004600e+00, 5.353400e+00, 5.702200e+00, 6.051000e+00 }, // SINR
          { 1, 7.825758e-01, 6.610825e-01, 2.509980e-01, 2.050000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 3.940700e+00, 4.288400e+00, 4.636100e+00, 4.983800e+00, 5.331500e+00, 5.679200e+00, 6.026900e+00, 6.374600e+00 }, // SINR
          { 1, 8.485099e-01, 5.803167e-01, 2.955607e-01, 3.960000e-02, 1.900000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 3.923900e+00, 4.270400e+00, 4.616900e+00, 4.963400e+00, 5.309900e+00, 5.656400e+00, 6.002900e+00 }, // SINR
          { 1, 7.787879e-01, 6.565722e-01, 1.727586e-01, 5.300000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 4, 4.500000e+00, 5, 5.375000e+00, 5.750000e+00, 6.125000e+00, 6.500000e+00 }, // SINR
          { 9.586466e-01, 6.653646e-01, 3.724340e-01, 8.910000e-02, 1.060000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 3.889600e+00, 4.233800e+00, 4.578000e+00, 4.922200e+00, 5.266400e+00, 5.610600e+00 }, // SINR
          { 1, 8.959790e-01, 6.918919e-01, 1.291237e-01, 1.800000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 4.200000e+00, 4.550000e+00, 4.900000e+00, 5.200000e+00, 5.600000e+00 }, // SINR
          { 9.923077e-01, 7.996894e-01, 5.546537e-01, 2.870000e-02, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 4.718750e+00, 5.882700e+00, 6.223500e+00, 6.564200e+00 }, // SINR
          { 9.000000e-01, 6.600000e-03, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 4.200000e+00, 4.450000e+00, 4.700000e+00, 4.950000e+00, 5.200000e+00, 5.400000e+00, 5.700000e+00 }, // SINR
          { 9.980769e-01, 8.455882e-01, 4.500000e-01, 1.136570e-01, 1.170000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.700000e+00, 5, 5.300000e+00, 5.600000e+00 }, // SINR
          { 9.522059e-01, 6.311881e-01, 9.742351e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 4.287657e+00, 5.738500e+00, 6.072300e+00 }, // SINR
          { 9.000000e-01, 1.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.800000e+00, 5.200000e+00, 5.600000e+00 }, // SINR
          { 9.980769e-01, 4.946911e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 4.500436e+00, 5.642300e+00, 5.971500e+00 }, // SINR
          { 9.000000e-01, 3.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 4.506242e+00, 5.594200e+00, 5.921200e+00 }, // SINR
          { 9.000000e-01, 3.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 4.389550e+00, 5.546200e+00, 5.870800e+00 }, // SINR
          { 9.000000e-01, 1.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 4.400000e+00, 4.650000e+00, 4.900000e+00, 5.150000e+00, 5.400000e+00, 5.650000e+00 }, // SINR
          { 9.761450e-01, 8.046875e-01, 3.580028e-01, 5.550000e-02, 2.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 4.407063e+00, 5.353800e+00, 5.669200e+00 }, // SINR
          { 9.000000e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 4.546200e+00, 5.257700e+00, 5.568500e+00 }, // SINR
          { 9.000000e-01, 1.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 4.356250e+00, 5.161500e+00, 5.467700e+00 }, // SINR
          { 9.000000e-01, 5.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 4.478800e+00, 5.065400e+00, 5.366900e+00, 5.668500e+00, 5.970000e+00 }, // SINR
          { 9.000000e-01, 1.782670e-01, 2.320000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 4.478800e+00, 4.969200e+00, 5.266200e+00, 5.563100e+00, 5.860000e+00 }, // SINR
          { 9.000000e-01, 2.164372e-01, 3.090000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 4.430700e+00, 4.873100e+00, 5.165400e+00, 5.457700e+00, 5.750000e+00 }, // SINR
          { 9.000000e-01, 2.500000e-01, 3.250000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 4.201500e+00, 4.489200e+00, 4.776900e+00, 5.064600e+00, 5.352300e+00, 5.640000e+00 }, // SINR
          { 9.807692e-01, 5.105000e-01, 4.390138e-01, 9.560016e-02, 7.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 4.306100e+00, 4.584600e+00, 4.863100e+00, 5.141500e+00, 5.420000e+00, 5.698500e+00 }, // SINR
          { 9.425182e-01, 6.457286e-01, 2.223940e-01, 2.270000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 4.392300e+00, 4.537700e+00, 4.661500e+00, 4.930800e+00, 5.200000e+00, 5.469200e+00 }, // SINR
          { 9.080357e-01, 9.000000e-01, 5.252058e-01, 1.311198e-01, 1.170000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 4.200000e+00, 4.460000e+00, 4.598000e+00, 4.720000e+00, 4.980000e+00, 5.240000e+00 }, // SINR
          { 9.855769e-01, 8.566667e-01, 4.277872e-01, 7.210000e-02, 3.900000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 4.258500e+00, 4.509200e+00, 4.716400e+00, 4.760000e+00, 5.010800e+00, 5.261500e+00 }, // SINR
          { 9.671053e-01, 7.647929e-01, 3.288961e-01, 4.610000e-02, 2.400000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 4.300000e+00, 4.500000e+00, 4.800000e+00, 5, 5.300000e+00, 5.550000e+00, 5.800000e+00 }, // SINR
          { 9.990385e-01, 9.817308e-01, 7.090278e-01, 3.579060e-01, 3.350000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 4.087700e+00, 4.320000e+00, 4.552300e+00, 4.700631e+00, 4.911900e+00, 5.123200e+00, 5.334500e+00 }, // SINR
          { 9.980769e-01, 9.498175e-01, 6.477273e-01, 2.081271e-01, 5.850000e-02, 2.600000e-03, 9.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 4.323100e+00, 4.546200e+00, 4.682569e+00, 4.747600e+00, 4.921700e+00, 4.949000e+00, 5.095900e+00 }, // SINR
          { 9.642857e-01, 6.967213e-01, 3.731618e-01, 1.999601e-01, 1.065329e-01, 4.160000e-02, 2.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 4.307700e+00, 4.674650e+00, 4.939500e+00, 5.204400e+00, 5.469300e+00 }, // SINR
          { 9.642857e-01, 9.000000e-01, 2.666139e-01, 3.020000e-02, 3.000000e-04 } // BLER
        }
      },
      { 2792U, // SINR and BLER for CBS 2792
        NrEesmErrorModel::DoubleTuple{
          { 3.830800e+00, 4.694463e+00, 4.694500e+00, 5.558100e+00 }, // SINR
          { 1, 9.000000e-01, 6.400000e-01, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 3.353800e+00, 4.694486e+00, 4.694500e+00, 6.035200e+00 }, // SINR
          { 1, 9.000000e-01, 4.870690e-01, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 4.669762e+00, 4.669800e+00, 6.462600e+00 }, // SINR
          { 9.000000e-01, 3.148263e-01, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 4.663453e+00, 4.663500e+00, 6.926900e+00 }, // SINR
          { 9.000000e-01, 4.330205e-01, 0 } // BLER
        }
      }
  },
  { // MCS 5
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 5.596300e+00, 6.096300e+00, 6.443800e+00, 6.791300e+00, 7.138800e+00, 7.486300e+00, 7.986300e+00 }, // SINR
          { 1, 8.870690e-01, 6.361940e-01, 3.156095e-01, 8.997198e-02, 1.360000e-02, 0 } // BLER
        }
      },
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 6, 6.700000e+00, 6.930000e+00, 7.170000e+00, 7.400000e+00, 8.100000e+00 }, // SINR
          { 1, 9.652256e-01, 1.204151e-01, 1.200000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 6, 6.575000e+00, 7.150000e+00, 7.725000e+00, 8.300000e+00 }, // SINR
          { 9.770992e-01, 6.696891e-01, 1.365239e-01, 4.200000e-03, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 5.823000e+00, 6.327200e+00, 6.831300e+00, 7.335400e+00, 7.839600e+00, 8.343800e+00 }, // SINR
          { 9.913462e-01, 8.306452e-01, 3.080685e-01, 2.920000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 6.008100e+00, 6.508100e+00, 6.821900e+00, 7.135700e+00, 7.449500e+00, 7.763300e+00 }, // SINR
          { 1, 6.256098e-01, 2.622651e-01, 5.140000e-02, 5.700000e-03, 5.000000e-04 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 6.071200e+00, 6.392900e+00, 6.714500e+00, 7.036200e+00, 7.357800e+00, 7.857800e+00 }, // SINR
          { 9.671053e-01, 8.451987e-01, 4.907946e-01, 1.385061e-01, 1.940000e-02, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 6.300000e+00, 6.581200e+00, 6.862500e+00, 7.143700e+00, 7.425000e+00, 7.925000e+00 }, // SINR
          { 9.086879e-01, 7.076503e-01, 3.431572e-01, 9.395292e-02, 1.460000e-02, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 6, 6.468800e+00, 6.937500e+00, 7.406200e+00, 7.875000e+00 }, // SINR
          { 9.725379e-01, 6.368750e-01, 1.146119e-01, 4.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 6.124900e+00, 6.436400e+00, 6.747900e+00, 7.059400e+00, 7.370900e+00, 7.870900e+00 }, // SINR
          { 9.846154e-01, 8.741497e-01, 5.224490e-01, 1.547472e-01, 1.700000e-02, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 6.300000e+00, 6.675000e+00, 7.050000e+00, 7.425000e+00, 7.800000e+00 }, // SINR
          { 9.770992e-01, 6.935484e-01, 1.770099e-01, 1.030000e-02, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 5.983800e+00, 6.302300e+00, 6.620800e+00, 6.939300e+00, 7.257800e+00, 7.757800e+00 }, // SINR
          { 9.951923e-01, 8.996479e-01, 4.990196e-01, 9.241754e-02, 4.400000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 6.204500e+00, 6.443200e+00, 6.681900e+00, 6.920600e+00, 7.159300e+00, 7.659300e+00 }, // SINR
          { 9.913462e-01, 9.084507e-01, 6.559278e-01, 2.735900e-01, 5.800000e-02, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 6.401200e+00, 6.626100e+00, 6.851000e+00, 7.075900e+00, 7.300800e+00, 7.800800e+00 }, // SINR
          { 9.817308e-01, 8.483333e-01, 5.321577e-01, 1.732094e-01, 3.170000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 6.405700e+00, 6.628800e+00, 6.851900e+00, 7.075000e+00, 7.298100e+00, 7.798100e+00 }, // SINR
          { 9.617537e-01, 8.225806e-01, 4.681734e-01, 1.358108e-01, 1.700000e-02, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.525000e+00, 7.150000e+00, 7.775000e+00 }, // SINR
          { 1, 8.501656e-01, 3.680000e-02, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 6.006300e+00, 6.381300e+00, 6.756300e+00, 7.131300e+00, 7.506300e+00 }, // SINR
          { 1, 9.420290e-01, 4.489437e-01, 3.030000e-02, 3.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 6.387900e+00, 6.633700e+00, 6.879600e+00, 7.125400e+00, 7.371200e+00, 7.871200e+00 }, // SINR
          { 9.942308e-01, 9.193262e-01, 5.769231e-01, 1.534022e-01, 1.110000e-02, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 6.464400e+00, 6.694600e+00, 6.924900e+00, 7.155100e+00, 7.385300e+00 }, // SINR
          { 9.154930e-01, 5.797511e-01, 1.611716e-01, 1.420000e-02, 4.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 6, 6.500000e+00, 6.719700e+00, 6.939400e+00, 7.159100e+00, 7.378800e+00 }, // SINR
          { 1, 8.576159e-01, 4.572842e-01, 8.530000e-02, 5.700000e-03, 3.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 6.067600e+00, 6.567600e+00, 6.731800e+00, 6.896000e+00, 7.060100e+00, 7.224300e+00 }, // SINR
          { 1, 6.493590e-01, 2.901376e-01, 7.190000e-02, 7.700000e-03, 5.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 6.200000e+00, 6.700000e+00, 6.862300e+00, 7.024700e+00, 7.187000e+00, 7.349300e+00, 7.849300e+00 }, // SINR
          { 1, 6.194581e-01, 2.542929e-01, 4.650000e-02, 5.800000e-03, 1.200000e-03, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 6.162800e+00, 6.662800e+00, 6.823900e+00, 6.984900e+00, 7.146000e+00, 7.307100e+00, 7.807100e+00 }, // SINR
          { 1, 8.615772e-01, 5.291667e-01, 1.728650e-01, 2.410000e-02, 2.400000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 6.698600e+00, 6.847700e+00, 6.996800e+00, 7.295000e+00, 7.742300e+00 }, // SINR
          { 9.617537e-01, 3.867424e-01, 2.120573e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 6.013800e+00, 6.513800e+00, 6.731800e+00, 6.949900e+00, 7.168000e+00, 7.386000e+00 }, // SINR
          { 1, 7.378571e-01, 2.186411e-01, 1.080000e-02, 3.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 6.578600e+00, 6.741800e+00, 6.904900e+00, 7.068100e+00, 7.231300e+00, 7.731300e+00 }, // SINR
          { 9.276786e-01, 6.759259e-01, 2.755991e-01, 5.080000e-02, 4.500000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 6.443600e+00, 6.739300e+00, 6.887100e+00, 7.034900e+00, 7.330600e+00 }, // SINR
          { 9.583333e-01, 8.716216e-01, 1.191998e-01, 7.956365e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 6.510400e+00, 6.726100e+00, 6.941700e+00, 7.157400e+00, 7.373100e+00 }, // SINR
          { 9.913462e-01, 8.387097e-01, 2.985782e-01, 2.190000e-02, 2.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 6.168200e+00, 6.668200e+00, 6.896300e+00, 7.124400e+00, 7.352500e+00 }, // SINR
          { 1, 8.221154e-01, 2.402008e-01, 7.100000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 6.084600e+00, 6.584600e+00, 6.734500e+00, 6.884300e+00, 7.034200e+00, 7.184100e+00 }, // SINR
          { 1, 8.035714e-01, 3.923611e-01, 7.940000e-02, 6.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 5.815300e+00, 6.315300e+00, 6.815300e+00, 6.958200e+00, 7.101200e+00, 7.244200e+00, 7.387100e+00 }, // SINR
          { 1, 8.640940e-01, 3.944704e-01, 9.805945e-02, 9.300000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1192U, // SINR and BLER for CBS 1192
        NrEesmErrorModel::DoubleTuple{
          { 6.502500e+00, 6.771100e+00, 7.039700e+00, 7.308300e+00 }, // SINR
          { 9.611742e-01, 3.868140e-01, 9.900000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.400000e+00, 6.730000e+00, 6.900000e+00, 7.070000e+00, 7.400000e+00 }, // SINR
          { 1, 9.980769e-01, 2.519960e-01, 1.864785e-01, 3.120000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 6.400000e+00, 6.800000e+00, 7.300000e+00, 7.700000e+00 }, // SINR
          { 1, 7.962963e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 5.800000e+00, 6.300000e+00, 6.525000e+00, 6.750000e+00, 6.975000e+00, 7.200000e+00, 7.700000e+00, 8.200000e+00 }, // SINR
          { 9.980769e-01, 7.820122e-01, 6.444724e-01, 4.748134e-01, 3.079490e-01, 1.655716e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 6.265400e+00, 6.704300e+00, 7.143300e+00, 7.582300e+00 }, // SINR
          { 1, 6.841398e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 5.800000e+00, 6.300000e+00, 6.475000e+00, 6.650000e+00, 6.825000e+00, 7, 7.500000e+00, 8 }, // SINR
          { 1, 8.949653e-01, 8.256369e-01, 7.046703e-01, 5.709459e-01, 4.327911e-01, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 6.100000e+00, 6.600000e+00, 6.750000e+00, 6.900000e+00, 7.050000e+00, 7.200000e+00, 7.700000e+00, 8.200000e+00 }, // SINR
          { 1, 6.750000e-01, 5.385021e-01, 4.142157e-01, 2.872159e-01, 1.876866e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.150000e+00, 6.400000e+00, 6.700000e+00, 6.900000e+00, 7.200000e+00, 7.400000e+00, 7.650000e+00, 7.900000e+00, 8.150000e+00, 8.400000e+00, 8.650000e+00, 8.900000e+00, 9.150000e+00 }, // SINR
          { 9.430147e-01, 8.657718e-01, 7.676471e-01, 5.915899e-01, 4.499113e-01, 2.663502e-01, 1.541054e-01, 6.930000e-02, 2.450000e-02, 8.100000e-03, 1.600000e-03, 5.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 6.200000e+00, 6.500000e+00, 6.800000e+00, 7.100000e+00, 7.400000e+00, 7.700000e+00, 8, 8.300000e+00, 8.600000e+00, 8.900000e+00, 9.200000e+00, 9.500000e+00, 9.800000e+00 }, // SINR
          { 9.202128e-01, 7.718373e-01, 5.199795e-01, 2.675159e-01, 8.090000e-02, 7.560000e-02, 2.080000e-02, 4.000000e-03, 9.000000e-04, 2.000000e-04, 2.000000e-04, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 5.950000e+00, 6.400000e+00, 6.850000e+00, 7.300000e+00, 7.750000e+00, 8.200000e+00, 8.650000e+00, 9.100000e+00 }, // SINR
          { 9.280576e-01, 7.676471e-01, 4.907588e-01, 2.042683e-01, 4.910000e-02, 5.700000e-03, 5.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 6.348600e+00, 6.730000e+00, 6.916400e+00, 7.110000e+00, 7.484200e+00, 8.052000e+00, 8.619800e+00 }, // SINR
          { 9.932692e-01, 4.191419e-01, 2.514940e-01, 7.500000e-02, 2.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 5.900000e+00, 6.400000e+00, 6.650000e+00, 6.900000e+00, 7.400000e+00, 8.150000e+00 }, // SINR
          { 1, 7.220670e-01, 4.040605e-01, 1.006198e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 5.800000e+00, 6.250000e+00, 6.475000e+00, 6.700000e+00, 7.150000e+00, 7.825000e+00 }, // SINR
          { 1, 9.329710e-01, 8.362903e-01, 3.692029e-01, 7.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 6
      { 24U, // SINR and BLER for CBS 24
        NrEesmErrorModel::DoubleTuple{
          { 6.650000e+00, 7.280000e+00, 7.593800e+00, 7.910000e+00, 8.538900e+00 }, // SINR
          { 9.990385e-01, 8.338710e-01, 7.929448e-01, 3.222010e-01, 0 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 6.482700e+00, 7.342700e+00, 7.677500e+00, 8.202700e+00 }, // SINR
          { 1, 6.699219e-01, 1.000000e-03, 2.000000e-04 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 6.960000e+00, 7.450000e+00, 7.687500e+00, 7.930000e+00, 8.418500e+00 }, // SINR
          { 9.769231e-01, 7.449128e-01, 3.985849e-01, 3.168750e-01, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 7.577900e+00, 7.676046e+00, 8.812900e+00, 1.004790e+01 }, // SINR
          { 9.640152e-01, 6.564103e-01, 9.324434e-02, 1.400000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 7, 7.250000e+00, 7.500000e+00, 7.750000e+00, 8, 8.250000e+00, 8.500000e+00, 8.750000e+00, 9, 9.250000e+00, 9.500000e+00 }, // SINR
          { 9.453704e-01, 8.750000e-01, 5.717489e-01, 3.210227e-01, 1.295196e-01, 3.420000e-02, 6.500000e-03, 5.800000e-03, 1.500000e-03, 1.200000e-03, 2.000000e-04 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 7, 7.200000e+00, 7.400000e+00, 7.600000e+00, 7.800000e+00, 8, 8.200000e+00, 8.400000e+00, 8.600000e+00, 8.800000e+00, 9, 9.200000e+00 }, // SINR
          { 9.365942e-01, 7.833333e-01, 6.717105e-01, 4.355670e-01, 2.366323e-01, 9.111201e-02, 2.920000e-02, 1.100000e-02, 1.700000e-03, 6.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 7.100000e+00, 7.600000e+00, 7.800000e+00, 8.100000e+00, 8.600000e+00 }, // SINR
          { 1, 3.684593e-01, 3.502747e-01, 2.600000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 7.300000e+00, 7.600000e+00, 7.900000e+00, 8.200000e+00, 9.100000e+00 }, // SINR
          { 1, 2.355140e-01, 1.140000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 7.206500e+00, 7.617000e+00, 8.070100e+00, 8.933700e+00 }, // SINR
          { 9.875000e-01, 7.789634e-01, 2.048046e-01, 6.000000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 6.950000e+00, 7.200000e+00, 7.450000e+00, 7.700000e+00, 7.950000e+00, 8.200000e+00, 8.450000e+00, 8.700000e+00, 8.950000e+00 }, // SINR
          { 9.537037e-01, 8.699664e-01, 6.311881e-01, 3.229434e-01, 1.030906e-01, 1.910000e-02, 2.800000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 7.645900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.942308e-01, 9.241071e-01, 5.481602e-01, 1.177773e-01, 5.200000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 7.687700e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.913462e-01, 9.298561e-01, 5.723982e-01, 1.099867e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 7.645300e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.961538e-01, 9.276786e-01, 5.427215e-01, 8.773167e-02, 2.300000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.475500e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.980769e-01, 8.987676e-01, 4.431424e-01, 4.680000e-02, 1.100000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.971154e-01, 8.914931e-01, 3.929128e-01, 2.840000e-02, 5.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.980769e-01, 9.271583e-01, 4.357759e-01, 3.640000e-02, 3.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.980769e-01, 8.827586e-01, 2.915704e-01, 9.500000e-03, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.990385e-01, 9.388489e-01, 3.336640e-01, 8.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 1, 8.977273e-01, 2.527555e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 1, 9.169643e-01, 2.204983e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 6.342900e+00, 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 1, 9.119718e-01, 1.716621e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.985600e+00, 8.396200e+00 }, // SINR
          { 9.836538e-01, 8.632550e-01, 2.049347e-01, 3.360000e-02, 2.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.985600e+00, 8.396200e+00 }, // SINR
          { 9.689850e-01, 7.618343e-01, 1.721311e-01, 3.010000e-02, 5.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.990385e-01, 1.857407e-01, 6.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.990385e-01, 1.323996e-01, 2.050000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1352U, // SINR and BLER for CBS 1352
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.985600e+00, 8.396200e+00 }, // SINR
          { 9.971154e-01, 8.431373e-01, 1.840278e-01, 1.010000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 9.990385e-01, 5.501078e-01, 6.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 7.300000e+00, 7.700000e+00, 8, 8.400000e+00 }, // SINR
          { 1, 8.266129e-01, 3.920000e-02, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.985600e+00, 8.396200e+00 }, // SINR
          { 9.923077e-01, 7.932099e-01, 3.904321e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.574900e+00, 8.190900e+00, 8.807000e+00 }, // SINR
          { 1, 4.209866e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 6.953552e-01, 2.839888e-01, 2.250000e-02, 1.000000e-03 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 7.740964e-01, 3.083538e-01, 1.120000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00, 8.498900e+00 }, // SINR
          { 1, 6.821809e-01, 3.132678e-01, 1.172535e-01, 2.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 7.626488e-01, 2.420673e-01, 1.430000e-02, 4.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 7.217514e-01, 2.425193e-01, 1.460000e-02, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 7.120166e-01, 2.020000e-01, 4.360000e-02, 7.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 6.958900e+00, 7.369600e+00, 7.574900e+00, 7.882900e+00, 8.190900e+00 }, // SINR
          { 1, 6.375000e-01, 2.254025e-01, 1.630000e-02, 2.000000e-04 } // BLER
        }
      }
  },
  { // MCS 7
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 7.987000e+00, 8.284000e+00, 8.447000e+00, 8.677000e+00, 8.907000e+00, 9.137000e+00 }, // SINR
          { 1, 6.395833e-01, 8.130000e-02, 1.800000e-03, 1.300000e-03, 2.000000e-04 } // BLER
        }
      },
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 7.800000e+00, 8.230000e+00, 8.650000e+00, 9.080000e+00, 9.500000e+00, 9.930000e+00, 1.035000e+01 }, // SINR
          { 9.865385e-01, 8.105346e-01, 4.142157e-01, 7.780000e-02, 5.000000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 8, 8.330000e+00, 8.500000e+00, 8.670000e+00, 9 }, // SINR
          { 1, 9.942308e-01, 4.199670e-01, 3.735119e-01, 6.320000e-02, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 7.970000e+00, 8.280000e+00, 8.430000e+00, 8.590000e+00, 8.900000e+00 }, // SINR
          { 1, 9.932692e-01, 6.096698e-01, 3.415541e-01, 1.824422e-01, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 8.430000e+00, 8.900000e+00, 9.370000e+00, 1.030000e+01 }, // SINR
          { 1, 1.684492e-01, 1.210000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 8.400000e+00, 8.530000e+00, 8.670000e+00, 8.800000e+00, 9.200000e+00, 9.600000e+00, 1.070000e+01 }, // SINR
          { 9.393116e-01, 1.300362e-01, 3.910000e-02, 8.000000e-04, 1.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 8.400000e+00, 8.620000e+00, 8.730000e+00, 8.850000e+00, 9.070000e+00, 9.400000e+00 }, // SINR
          { 1, 9.384191e-01, 7.370000e-02, 1.120000e-02, 7.400000e-03, 4.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 7.400000e+00, 8.400000e+00, 9.400000e+00, 1.040000e+01, 1.140000e+01 }, // SINR
          { 1, 6.363065e-01, 1.043441e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 7.600000e+00, 8.500000e+00, 9.400000e+00, 1.030000e+01, 1.120000e+01 }, // SINR
          { 1, 5.799087e-01, 7.380000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.573700e+00, 9.677900e+00, 1.078220e+01, 1.188640e+01 }, // SINR
          { 9.636194e-01, 5.379747e-01, 4.070000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 7.980000e+00, 8.500000e+00, 9, 9.500000e+00, 1.010000e+01, 1.060000e+01 }, // SINR
          { 9.980769e-01, 6.552835e-01, 2.418269e-01, 4.500000e-02, 1.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.449900e+00, 9.430300e+00, 1.041080e+01, 1.139120e+01 }, // SINR
          { 9.649621e-01, 5.095766e-01, 4.010000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.388000e+00, 9.306500e+00, 1.022510e+01, 1.114360e+01 }, // SINR
          { 9.817308e-01, 6.578608e-01, 9.987593e-02, 9.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.264200e+00, 8.402700e+00, 9.058900e+00, 9.853700e+00 }, // SINR
          { 9.671053e-01, 6.362500e-01, 1.220958e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.636194e-01, 6.734293e-01, 1.732759e-01, 8.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.818702e-01, 8.160828e-01, 2.888128e-01, 2.260000e-02, 3.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.884615e-01, 7.399425e-01, 2.034790e-01, 1.040000e-02, 1.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.437500e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.580224e-01, 6.451005e-01, 1.186381e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.732824e-01, 6.815160e-01, 1.147448e-01, 1.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.367000e+00, 9.482300e+00 }, // SINR
          { 9.780534e-01, 6.858289e-01, 1.080139e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 7.500000e+00, 8.100000e+00, 8.800000e+00, 9.500000e+00 }, // SINR
          { 1, 8.959790e-01, 4.340000e-02, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.923077e-01, 6.970109e-01, 5.750000e-02, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00, 1.015320e+01 }, // SINR
          { 9.875000e-01, 6.095972e-01, 3.230000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.942308e-01, 6.160287e-01, 2.070000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.903846e-01, 5.900229e-01, 1.510000e-02, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.923077e-01, 5.498927e-01, 1.030000e-02, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.913462e-01, 5.445279e-01, 7.100000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.971154e-01, 5.764840e-01, 6.900000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 9.961538e-01, 4.699074e-01, 2.500000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 5.543478e-01, 2.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 7.271429e-01, 7.300000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 8.835616e-01, 3.560000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 8.093750e-01, 1.810000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.587700e+00, 8.811300e+00, 9.035000e+00, 9.482300e+00 }, // SINR
          { 1, 9.393116e-01, 2.658228e-01, 3.260000e-02, 2.460000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 7.227654e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 8.509934e-01, 7.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 7.500000e-01, 1.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 8.258065e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 6.980874e-01, 2.200000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 7.469400e+00, 8.140400e+00, 8.811300e+00, 9.482300e+00 }, // SINR
          { 1, 7.893519e-01, 1.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 7.900000e+00, 8.100000e+00, 8.400000e+00, 8.600000e+00, 8.800000e+00, 9.030000e+00, 9.250000e+00, 9.480000e+00 }, // SINR
          { 9.894231e-01, 9.049296e-01, 4.824144e-01, 1.637987e-01, 3.280000e-02, 1.060000e-02, 1.000000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 8
      { 32U, // SINR and BLER for CBS 32
        NrEesmErrorModel::DoubleTuple{
          { 8.500000e+00, 8.800000e+00, 9.100000e+00, 9.400000e+00, 9.700000e+00, 10, 1.030000e+01, 1.060000e+01, 1.090000e+01 }, // SINR
          { 1, 9.990385e-01, 9.205357e-01, 4.425087e-01, 5.580000e-02, 1.450000e-02, 1.100000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 8, 9.400000e+00, 1.080000e+01, 1.220000e+01 }, // SINR
          { 1, 7.492690e-01, 6.200000e-03, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 8.700000e+00, 9.730000e+00, 1.030000e+01, 1.077000e+01, 1.180000e+01 }, // SINR
          { 9.214286e-01, 7.500000e-03, 5.700000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 7.200000e+00, 8.700000e+00, 9.700000e+00, 1.020000e+01, 1.070000e+01 }, // SINR
          { 1, 9.145683e-01, 2.740000e-02, 8.300000e-03, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 8.700000e+00, 9.430000e+00, 9.800000e+00, 1.017000e+01 }, // SINR
          { 9.347826e-01, 2.704741e-01, 8.907126e-02, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 7.400000e+00, 8.700000e+00, 9.600000e+00, 1.010000e+01, 1.050000e+01 }, // SINR
          { 1, 9.334532e-01, 9.465021e-02, 1.000000e-02, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 8.700000e+00, 8.980000e+00, 9.250000e+00, 9.530000e+00, 9.800000e+00, 1.008000e+01, 1.035000e+01 }, // SINR
          { 9.990385e-01, 9.586466e-01, 6.099760e-01, 1.280644e-01, 5.500000e-03, 1.100000e-03, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.964400e+00, 1.119230e+01, 1.242030e+01 }, // SINR
          { 9.507299e-01, 5.300830e-01, 7.300000e-02, 1.700000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.902500e+00, 1.106850e+01, 1.223460e+01, 1.340060e+01 }, // SINR
          { 9.214286e-01, 3.825758e-01, 2.380000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.840600e+00, 1.094470e+01, 1.204890e+01, 1.315300e+01 }, // SINR
          { 9.293478e-01, 4.751873e-01, 3.790000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.778700e+00, 1.082090e+01, 1.186320e+01, 1.290540e+01 }, // SINR
          { 9.614662e-01, 5.231481e-01, 5.470000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 7.756200e+00, 8.736500e+00, 9.716800e+00, 1.069710e+01, 1.167750e+01 }, // SINR
          { 1, 8.801020e-01, 3.466530e-01, 2.360000e-02, 1.000000e-04 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 8.052500e+00, 8.736500e+00, 9.593000e+00, 1.044950e+01, 1.130610e+01 }, // SINR
          { 9.406475e-01, 5.161943e-01, 8.120000e-02, 2.200000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01 }, // SINR
          { 9.031690e-01, 4.288721e-01, 6.760000e-02, 1.900000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01, 1.166740e+01 }, // SINR
          { 9.343525e-01, 5.418432e-01, 9.532520e-02, 3.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01, 1.166740e+01 }, // SINR
          { 1, 8.809524e-01, 3.814006e-01, 3.500000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01, 1.166740e+01 }, // SINR
          { 1, 8.956897e-01, 3.965517e-01, 3.150000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 8.250000e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01 }, // SINR
          { 9.552239e-01, 5.675223e-01, 7.050000e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01, 1.166740e+01 }, // SINR
          { 9.498175e-01, 4.960784e-01, 4.880000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.055944e-01, 2.809710e-01, 8.600000e-03, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.406934e-01, 3.285714e-01, 6.400000e-03, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 8, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.241071e-01, 2.787611e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01, 1.093470e+01 }, // SINR
          { 9.903846e-01, 8.987676e-01, 1.397715e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.835600e+00 }, // SINR
          { 1, 7.078729e-01, 1.462704e-01, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.835600e+00, 1.020190e+01 }, // SINR
          { 1, 8.050314e-01, 1.597398e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.835600e+00 }, // SINR
          { 1, 8.409091e-01, 1.574248e-01, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.102900e+00, 9.469200e+00, 9.835500e+00 }, // SINR
          { 1, 8.616667e-01, 4.780000e-02, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.343525e-01, 9.000000e-01, 9.655383e-02, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.250000e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.512868e-01, 9.000000e-01, 8.716707e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.194450e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 9.430147e-01, 9.000000e-01, 7.280000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 1, 4.074519e-01, 1.583753e-01, 1.840000e-02, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 1, 3.899540e-01, 2.052117e-01, 1.370000e-02, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.522059e-01, 3.840634e-01, 2.056056e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.636194e-01, 5.793379e-01, 1.805755e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 1, 8.949653e-01, 1.321053e-01, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.420290e-01, 7.127809e-01, 1.807554e-01, 7.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.088542e-01, 5.334728e-01, 6.540000e-02, 3.000000e-04, 2.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.507299e-01, 7.694611e-01, 8.924279e-02, 1.010000e-02, 1.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 8.003800e+00, 8.736500e+00, 9.469200e+00, 1.020190e+01 }, // SINR
          { 1, 8.810345e-01, 8.420000e-02, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 8.736500e+00, 9.225000e+00, 9.469200e+00, 9.713400e+00, 1.020190e+01 }, // SINR
          { 9.125874e-01, 8.330592e-01, 4.620000e-02, 9.600000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 9
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.038000e+01, 1.057000e+01, 1.075000e+01, 1.113000e+01 }, // SINR
          { 9.531250e-01, 1.193994e-01, 3.300000e-03, 8.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 9, 1.000360e+01, 1.166470e+01, 1.332580e+01, 1.498690e+01 }, // SINR
          { 9.498175e-01, 5.887097e-01, 1.304687e-01, 5.100000e-03, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 9.250000e+00, 1.000360e+01, 1.157180e+01, 1.314010e+01, 1.470830e+01 }, // SINR
          { 9.444444e-01, 5.633260e-01, 9.689289e-02, 2.600000e-03, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 9.102900e+00, 1.000360e+01, 1.154090e+01, 1.307820e+01, 1.461550e+01 }, // SINR
          { 9.000000e-01, 8.093354e-01, 2.468075e-01, 1.090000e-02, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 8.590100e+00, 1.000360e+01, 1.141710e+01, 1.283060e+01, 1.424410e+01, 1.565760e+01 }, // SINR
          { 9.817308e-01, 8.392857e-01, 3.034420e-01, 2.450000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 9.286000e+00, 1.000360e+01, 1.135520e+01, 1.270680e+01, 1.405840e+01 }, // SINR
          { 9.000000e-01, 8.427152e-01, 3.033573e-01, 1.880000e-02, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 8.713900e+00, 1.000360e+01, 1.129330e+01, 1.258300e+01, 1.387270e+01, 1.516240e+01 }, // SINR
          { 9.894231e-01, 8.716216e-01, 3.134328e-01, 2.150000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 9.250000e+00, 1.000360e+01, 1.123140e+01, 1.245920e+01, 1.368700e+01 }, // SINR
          { 9.000000e-01, 7.955247e-01, 1.738904e-01, 3.000000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 9.143667e+00, 1.000360e+01, 1.116950e+01, 1.233540e+01 }, // SINR
          { 9.258929e-01, 4.140625e-01, 2.280000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 9.135600e+00, 1.000360e+01, 1.110760e+01, 1.221160e+01, 1.331560e+01 }, // SINR
          { 9.000000e-01, 6.695026e-01, 1.009496e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 9.125000e+00, 1.000360e+01, 1.104570e+01, 1.208780e+01, 1.312990e+01 }, // SINR
          { 9.000000e-01, 7.038043e-01, 1.386740e-01, 2.300000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 9.085300e+00, 1.000360e+01, 1.092190e+01, 1.184020e+01, 1.275850e+01 }, // SINR
          { 9.903846e-01, 7.761976e-01, 1.900376e-01, 7.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01, 1.318160e+01 }, // SINR
          { 9.894231e-01, 7.875767e-01, 2.709227e-01, 2.280000e-02, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01, 1.318160e+01 }, // SINR
          { 9.537037e-01, 8.101266e-01, 2.917633e-01, 1.820000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 9.473900e+00, 1.000360e+01, 1.053854e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 1, 6.112440e-01, 9.733441e-02, 1.400000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01 }, // SINR
          { 9.652256e-01, 6.407828e-01, 1.086192e-01, 2.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01 }, // SINR
          { 9.942308e-01, 7.087989e-01, 1.258741e-01, 2.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01 }, // SINR
          { 9.942308e-01, 6.165865e-01, 7.650000e-02, 8.000000e-04, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01, 1.238710e+01 }, // SINR
          { 9.770992e-01, 7.306034e-01, 9.020000e-02, 9.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.751908e-01, 6.237864e-01, 4.150000e-02, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.923077e-01, 6.987705e-01, 5.040000e-02, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 9.503600e+00, 1.000360e+01, 1.051730e+01, 1.079810e+01 }, // SINR
          { 1, 6.280941e-01, 2.130000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 7.212079e-01, 2.860000e-02, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 6.784759e-01, 1.730000e-02, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 9.920100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 5.762332e-01, 7.200000e-03, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.818702e-01, 6.505102e-01, 1.110000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 9.503600e+00, 1.000360e+01, 1.059850e+01, 1.079810e+01 }, // SINR
          { 1, 5.985915e-01, 5.000000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.961538e-01, 5.755682e-01, 2.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 9.612800e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 5.616812e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 9.625000e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 5.885417e-01, 9.000000e-04, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 9.750000e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 5.581140e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 1928U, // SINR and BLER for CBS 1928
        NrEesmErrorModel::DoubleTuple{
          { 8.987000e+00, 1.000360e+01, 1.102020e+01, 1.305330e+01 }, // SINR
          { 1, 6.895161e-01, 2.000000e-03, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 9.500000e+00, 9.800000e+00, 1.020000e+01, 1.050000e+01, 1.080000e+01, 1.113000e+01 }, // SINR
          { 9.875954e-01, 8.852740e-01, 4.865385e-01, 1.568352e-01, 2.290000e-02, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 9.500000e+00, 9.800000e+00, 1.020000e+01, 1.050000e+01, 1.080000e+01, 1.113000e+01 }, // SINR
          { 9.990385e-01, 9.678030e-01, 6.274631e-01, 1.833942e-01, 1.540000e-02, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 7.210452e-01, 1.700000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 9.209100e+00, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.836538e-01, 8.140823e-01, 2.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.000360e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 9.000000e-01, 7.551775e-01, 1.100000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 8.993500e+00, 1.000360e+01, 1.101380e+01, 1.303400e+01 }, // SINR
          { 1, 7.953125e-01, 5.600000e-03, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 9.473900e+00, 1.000360e+01, 1.012800e+01, 1.079810e+01, 1.159260e+01 }, // SINR
          { 1, 7.602941e-01, 6.525641e-01, 5.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 10
      { 40U, // SINR and BLER for CBS 40
        NrEesmErrorModel::DoubleTuple{
          { 9.830000e+00, 1.057000e+01, 1.081000e+01, 1.106000e+01, 1.130000e+01, 1.350000e+01 }, // SINR
          { 1, 9.990385e-01, 3.783582e-01, 4.250000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 1.040000e+01, 1.070000e+01, 11, 1.130000e+01, 1.160000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.280000e+01 }, // SINR
          { 9.055944e-01, 7.090278e-01, 4.057508e-01, 1.574655e-01, 3.720000e-02, 1.530000e-02, 1.700000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.030000e+01, 1.080000e+01, 1.140000e+01, 1.250000e+01 }, // SINR
          { 1, 5.427350e-01, 2.000000e-03, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 1.050000e+01, 1.065000e+01, 1.080000e+01, 1.095000e+01, 1.110000e+01, 1.125000e+01, 1.140000e+01 }, // SINR
          { 9.232143e-01, 8.423203e-01, 1.888138e-01, 6.970000e-02, 1.730000e-02, 2.500000e-03, 6.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 1.037000e+01, 1.080000e+01, 1.133000e+01, 1.230000e+01, 1.380000e+01 }, // SINR
          { 1, 4.499113e-01, 7.870000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 1.030000e+01, 1.107000e+01, 1.140000e+01, 1.183000e+01 }, // SINR
          { 9.836538e-01, 5.686384e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 1.043000e+01, 1.080000e+01, 1.127000e+01, 1.210000e+01, 1.340000e+01 }, // SINR
          { 1, 8.750000e-01, 1.750000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 9.640800e+00, 1.084790e+01, 1.205500e+01, 1.326210e+01 }, // SINR
          { 1, 6.018957e-01, 5.690000e-02, 1.000000e-04 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 9.702700e+00, 1.084790e+01, 1.199310e+01, 1.313830e+01, 1.428350e+01 }, // SINR
          { 9.980769e-01, 5.833333e-01, 5.670000e-02, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 9.764600e+00, 1.084790e+01, 1.193120e+01, 1.301450e+01, 1.409780e+01 }, // SINR
          { 1, 5.817972e-01, 6.680000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.023060e+01, 1.084790e+01, 1.180740e+01, 1.276690e+01, 1.372640e+01 }, // SINR
          { 9.000000e-01, 5.752262e-01, 8.520000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.025000e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.000000e-01, 6.183894e-01, 1.199905e-01, 4.300000e-03, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.021290e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.000000e-01, 5.748874e-01, 9.428052e-02, 1.500000e-03, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.017255e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.000000e-01, 6.081731e-01, 9.023312e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.031250e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.000000e-01, 5.680310e-01, 7.480000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.001220e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01, 1.335500e+01 }, // SINR
          { 9.990385e-01, 6.129808e-01, 6.830000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.075000e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 4.372852e-01, 2.570000e-02, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.001220e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.961538e-01, 4.190199e-01, 1.560000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.001220e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 1, 3.521468e-01, 4.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.081250e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.518006e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 10, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.635057e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.044069e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.649135e-01, 2.400000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.053450e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 2.711910e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.053450e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 2.860795e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.063900e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.039568e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.050000e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 3.159375e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.050000e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.868499e-01, 1.000000e-04 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.053450e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.960094e-01, 1.000000e-04 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.069888e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.406951e-01, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.063402e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.253743e-01, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.051545e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 1.511733e-01, 0 } // BLER
        }
      },
      { 2088U, // SINR and BLER for CBS 2088
        NrEesmErrorModel::DoubleTuple{
          { 1.049485e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 4.540179e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.064118e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 4.581835e-01, 8.000000e-04, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.066766e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 3.280848e-01, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.057950e+01, 1.084790e+01, 1.168360e+01, 1.251930e+01 }, // SINR
          { 9.000000e-01, 2.944965e-01, 4.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.057950e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 3.684593e-01, 1.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.062830e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 2.608471e-01, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.062830e+01, 1.084790e+01, 1.168360e+01 }, // SINR
          { 9.000000e-01, 3.052885e-01, 0 } // BLER
        }
      }
  },
  { // MCS 11
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 1.113000e+01, 1.170000e+01, 1.227000e+01, 1.340000e+01 }, // SINR
          { 1, 7.649701e-01, 1.342049e-01, 0 } // BLER
        }
      },
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 1.113000e+01, 1.170000e+01, 1.227000e+01, 1.340000e+01 }, // SINR
          { 1, 8.248408e-01, 1.444189e-01, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 1.130000e+01, 1.170000e+01, 1.210000e+01, 1.290000e+01 }, // SINR
          { 1, 6.647135e-01, 3.935759e-01, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 1.207000e+01, 1.231000e+01, 1.243000e+01, 1.256000e+01, 1.280000e+01, 1.390000e+01 }, // SINR
          { 9.384058e-01, 9.202128e-01, 6.050000e-02, 4.540000e-02, 2.100000e-03, 8.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 1.240000e+01, 1.270000e+01, 1.310000e+01 }, // SINR
          { 9.704198e-01, 1.304517e-01, 4.300000e-03, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 1.237000e+01, 1.270000e+01, 1.303000e+01 }, // SINR
          { 9.865385e-01, 1.210145e-01, 3.900000e-03, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.080000e+01, 1.170000e+01, 1.227000e+01, 1.250000e+01, 1.283000e+01, 1.340000e+01 }, // SINR
          { 1, 9.620370e-01, 8.783784e-01, 9.300000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 1.042550e+01, 1.167280e+01, 1.292000e+01, 1.416730e+01, 1.541460e+01, 1.666190e+01 }, // SINR
          { 1, 8.384740e-01, 2.143463e-01, 7.300000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.088260e+01, 1.167280e+01, 1.210053e+01, 1.285810e+01, 1.404350e+01 }, // SINR
          { 1, 8.245192e-01, 2.049918e-01, 4.800000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.092380e+01, 1.167280e+01, 1.206210e+01, 1.279620e+01, 1.391970e+01 }, // SINR
          { 1, 8.148734e-01, 2.171280e-01, 4.900000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.067310e+01, 1.167280e+01, 1.267240e+01, 1.367210e+01, 1.467180e+01 }, // SINR
          { 1, 7.860429e-01, 2.165948e-01, 9.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01 }, // SINR
          { 1, 8.000000e-01, 2.430019e-01, 1.390000e-02, 1.000000e-04 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01, 1.517630e+01 }, // SINR
          { 1, 7.641369e-01, 2.058824e-01, 8.300000e-03, 2.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01, 1.517630e+01 }, // SINR
          { 9.089286e-01, 4.116450e-01, 2.980000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01, 1.517630e+01 }, // SINR
          { 9.095745e-01, 3.867424e-01, 2.050000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01, 1.517630e+01 }, // SINR
          { 9.029720e-01, 3.075980e-01, 1.200000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01 }, // SINR
          { 1, 8.204114e-01, 1.871280e-01, 3.700000e-03, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01, 1.430040e+01 }, // SINR
          { 1, 7.947531e-01, 9.650206e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 1, 8.070312e-01, 8.090000e-02, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.120000e+01, 1.150000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.283000e+01, 1.315000e+01 }, // SINR
          { 1, 9.961538e-01, 7.522059e-01, 1.837482e-01, 7.900000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.855769e-01, 7.551170e-01, 3.390000e-02, 1.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.855769e-01, 7.463235e-01, 2.590000e-02, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.120000e+01, 1.150000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.283000e+01, 1.315000e+01 }, // SINR
          { 1, 9.961538e-01, 7.321429e-01, 1.799215e-01, 6.000000e-03, 1.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.117280e+01, 1.167280e+01, 1.192575e+01, 1.254860e+01 }, // SINR
          { 9.444444e-01, 7.642216e-01, 1.990000e-02, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.117280e+01, 1.167280e+01, 1.227410e+01, 1.254860e+01 }, // SINR
          { 9.866412e-01, 7.795455e-01, 1.520000e-02, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.150000e+01, 1.178000e+01, 1.205000e+01, 1.233000e+01, 1.260000e+01, 1.288000e+01 }, // SINR
          { 1, 9.875000e-01, 6.708115e-01, 1.021303e-01, 1.900000e-03, 2.000000e-04 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.162500e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.000000e-01, 8.026730e-01, 1.040000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.139190e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.000000e-01, 8.113057e-01, 6.400000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.079690e+01, 1.167280e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.903846e-01, 7.718373e-01, 2.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.160000e+01, 1.190000e+01, 1.220000e+01, 1.250000e+01, 1.280000e+01 }, // SINR
          { 9.990385e-01, 9.214286e-01, 3.236607e-01, 9.600000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.065370e+01, 1.167280e+01, 1.269190e+01, 1.473010e+01 }, // SINR
          { 1, 7.171788e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 2152U, // SINR and BLER for CBS 2152
        NrEesmErrorModel::DoubleTuple{
          { 1.130000e+01, 1.170000e+01, 12, 1.240000e+01, 1.270000e+01 }, // SINR
          { 1, 9.942308e-01, 7.134831e-01, 1.540000e-02, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 12, 1.230000e+01, 1.250000e+01 }, // SINR
          { 9.352518e-01, 3.165625e-01, 6.600000e-03, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.150000e+01, 1.178000e+01, 1.205000e+01, 1.233000e+01, 1.260000e+01 }, // SINR
          { 1, 8.382353e-01, 2.050081e-01, 4.200000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.176067e+01, 1.254860e+01, 1.342450e+01 }, // SINR
          { 9.470803e-01, 9.000000e-01, 9.400000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 12, 1.230000e+01, 1.250000e+01, 1.280000e+01 }, // SINR
          { 9.923077e-01, 5.936047e-01, 3.090000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.170000e+01, 12, 1.230000e+01, 1.250000e+01, 1.280000e+01 }, // SINR
          { 1, 9.128521e-01, 2.969484e-01, 3.770000e-02, 8.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.167280e+01, 1.225670e+01, 1.254860e+01, 1.284060e+01 }, // SINR
          { 9.980769e-01, 2.011218e-01, 3.800000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 12
      { 48U, // SINR and BLER for CBS 48
        NrEesmErrorModel::DoubleTuple{
          { 1.136280e+01, 1.321120e+01, 1.505960e+01, 1.690810e+01, 1.875650e+01, 2.060500e+01 }, // SINR
          { 9.799618e-01, 8.287338e-01, 3.337731e-01, 2.550000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.321120e+01, 1.337710e+01, 1.357710e+01, 1.377710e+01, 1.397710e+01, 1.417710e+01, 1.437710e+01, 1.457710e+01, 1.477710e+01, 1.493580e+01, 1.497710e+01, 1.666050e+01, 1.838510e+01, 2.010980e+01 }, // SINR
          { 9.066901e-01, 8.792517e-01, 8.501656e-01, 8.054687e-01, 7.500000e-01, 6.991758e-01, 6.311881e-01, 5.738636e-01, 5.135542e-01, 4.471831e-01, 4.284512e-01, 4.040000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 1.157940e+01, 1.321120e+01, 1.484300e+01, 1.647480e+01, 1.810660e+01 }, // SINR
          { 9.855769e-01, 8.152516e-01, 2.774725e-01, 9.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 1.164130e+01, 1.321120e+01, 1.478110e+01, 1.635100e+01, 1.792090e+01 }, // SINR
          { 9.903846e-01, 7.205056e-01, 1.675532e-01, 2.500000e-03, 1.000000e-04 } // BLER
        }
      },
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 1.165840e+01, 1.321120e+01, 1.465730e+01, 1.610340e+01, 1.754950e+01 }, // SINR
          { 9.000000e-01, 6.356250e-01, 1.071739e-01, 1.900000e-03, 0 } // BLER
        }
      },
      { 144U, // SINR and BLER for CBS 144
        NrEesmErrorModel::DoubleTuple{
          { 1.188890e+01, 1.321120e+01, 1.453350e+01, 1.585580e+01, 1.717810e+01, 1.850040e+01 }, // SINR
          { 9.640152e-01, 7.573529e-01, 2.294333e-01, 7.100000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.186290e+01, 1.321120e+01, 1.459540e+01, 1.597960e+01 }, // SINR
          { 9.000000e-01, 5.520386e-01, 3.790000e-02, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.187500e+01, 1.321120e+01, 1.447160e+01, 1.573200e+01, 1.699240e+01 }, // SINR
          { 9.000000e-01, 8.012422e-01, 1.953246e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.176080e+01, 1.321120e+01, 1.440970e+01, 1.560820e+01, 1.680670e+01 }, // SINR
          { 9.000000e-01, 7.655325e-01, 1.449192e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.213650e+01, 1.321120e+01, 1.428590e+01, 1.536060e+01, 1.643530e+01 }, // SINR
          { 9.932692e-01, 6.280340e-01, 9.000809e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.818702e-01, 4.789326e-01, 5.720000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.531250e-01, 4.203795e-01, 3.600000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.176080e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.000000e-01, 6.207524e-01, 8.620000e-02, 1.000000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.494485e-01, 5.575658e-01, 5.720000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.884615e-01, 4.146242e-01, 2.110000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01, 1.606390e+01 }, // SINR
          { 9.534672e-01, 4.951550e-01, 2.950000e-02, 4.000000e-04, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.855769e-01, 5.198980e-01, 1.810000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.289420e+01, 1.321120e+01, 1.352820e+01, 1.416210e+01 }, // SINR
          { 9.980769e-01, 2.352528e-01, 2.460000e-02, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.226030e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.659091e-01, 2.961449e-01, 2.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.175000e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 1.736496e-01, 5.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.192467e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 1.996417e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 12, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 1.962891e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.234605e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 2.423372e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.262500e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.219601e-01, 1.000000e-04 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.291160e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.308777e-01, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.212910e+01, 1.285050e+01, 1.321120e+01, 1.375230e+01 }, // SINR
          { 1, 8.844828e-01, 8.610000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 13, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 7.520000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.276513e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 8.558468e-02, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.274726e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 6.910000e-02, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.276257e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 5.110000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.269950e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.500000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.263580e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.370000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2408U, // SINR and BLER for CBS 2408
        NrEesmErrorModel::DoubleTuple{
          { 1.267930e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 2.870000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.273250e+01, 1.321120e+01, 1.416210e+01, 1.511300e+01 }, // SINR
          { 9.000000e-01, 2.375470e-01, 3.000000e-04, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.273630e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.172897e-01, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.279473e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 1.220760e-01, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.276870e+01, 1.321120e+01, 1.416210e+01 }, // SINR
          { 9.000000e-01, 6.710000e-02, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 13
      { 56U, // SINR and BLER for CBS 56
        NrEesmErrorModel::DoubleTuple{
          { 1.280000e+01, 1.407000e+01, 1.470000e+01, 1.533000e+01 }, // SINR
          { 1, 5.105000e-01, 2.066201e-01, 0 } // BLER
        }
      },
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.330000e+01, 1.365000e+01, 14, 1.435000e+01, 1.470000e+01, 1.505000e+01, 1.540000e+01, 1.575000e+01, 1.610000e+01 }, // SINR
          { 9.211957e-01, 7.104167e-01, 4.325601e-01, 1.855670e-01, 4.720000e-02, 2.750000e-02, 3.000000e-03, 7.000000e-04, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 13, 1.410000e+01, 1.470000e+01, 1.520000e+01 }, // SINR
          { 1, 2.753821e-01, 2.377358e-01, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 1.313810e+01, 1.471970e+01, 1.630130e+01, 1.788280e+01, 1.946440e+01 }, // SINR
          { 1, 4.323630e-01, 2.350000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 152U, // SINR and BLER for CBS 152
        NrEesmErrorModel::DoubleTuple{
          { 1.332380e+01, 1.471970e+01, 1.611560e+01, 1.751140e+01, 1.890730e+01 }, // SINR
          { 9.923077e-01, 3.684402e-01, 1.550000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.296120e+01, 1.471970e+01, 1.617750e+01, 1.763520e+01 }, // SINR
          { 9.000000e-01, 3.148148e-01, 4.600000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.205170e+01, 1.338570e+01, 1.471970e+01, 1.605370e+01, 1.738760e+01 }, // SINR
          { 1, 8.750000e-01, 5.496795e-01, 2.800000e-02, 1.000000e-04 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.296120e+01, 1.471970e+01, 1.599180e+01, 1.726380e+01 }, // SINR
          { 9.000000e-01, 4.024682e-01, 1.340000e-02, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.290625e+01, 1.471970e+01, 1.586800e+01, 1.701620e+01 }, // SINR
          { 9.000000e-01, 2.118465e-01, 2.700000e-03, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.267070e+01, 1.369520e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01, 1.779310e+01 }, // SINR
          { 1, 5.937500e-01, 3.871951e-01, 2.130000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 336U, // SINR and BLER for CBS 336
        NrEesmErrorModel::DoubleTuple{
          { 1.375000e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 2.216608e-01, 5.400000e-03, 0 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 1.390720e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 1.304404e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.267070e+01, 1.369520e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 1, 8.388158e-01, 2.663502e-01, 3.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.394465e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 1.004740e-01, 6.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.381250e+01, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 1.247515e-01, 4.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 14, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 4.390000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 13, 1.471970e+01, 1.574420e+01, 1.676860e+01 }, // SINR
          { 9.000000e-01, 6.180000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.385850e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 6.090000e-02, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.350000e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 3.910000e-02, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.381830e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 3.110000e-02, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.350000e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 1.720000e-02, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.346970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 1.130000e-02, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.371970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 5.400000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.346970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.377050e+01, 1.377053e+01, 1.408690e+01, 1.440330e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 9.000000e-01, 6.888441e-01, 5.700673e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.379553e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 1.800000e-03, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.379938e+01, 1.379940e+01, 1.410620e+01, 1.441290e+01 }, // SINR
          { 9.951923e-01, 7.514451e-01, 6.464646e-01, 1.000000e-04 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.384357e+01, 1.384360e+01, 1.413560e+01, 1.442770e+01, 1.471970e+01 }, // SINR
          { 9.990385e-01, 6.994536e-01, 5.818182e-01, 5.000000e-04, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.293460e+01, 1.387110e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 7.825758e-01, 5.308577e-01, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.270740e+01, 1.371970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 8.958333e-01, 6.188424e-01, 0 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.270740e+01, 1.371970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 8.101563e-01, 4.323630e-01, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.373879e+01, 1.373880e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 9.000000e-01, 8.624161e-01, 5.120968e-01, 0 } // BLER
        }
      },
      { 2664U, // SINR and BLER for CBS 2664
        NrEesmErrorModel::DoubleTuple{
          { 1.324060e+01, 1.374060e+01, 1.374063e+01, 1.471970e+01 }, // SINR
          { 9.636194e-01, 7.955975e-01, 3.684402e-01, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.324810e+01, 1.374810e+01, 1.374812e+01, 1.471970e+01 }, // SINR
          { 9.570896e-01, 8.445724e-01, 4.082792e-01, 1.000000e-04 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.270740e+01, 1.371970e+01, 1.471970e+01, 1.574420e+01 }, // SINR
          { 1, 7.880435e-01, 3.148148e-01, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.371970e+01, 1.432600e+01, 1.432760e+01, 1.493230e+01 }, // SINR
          { 9.166667e-01, 3.100000e-03, 2.200000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 14
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 1.420000e+01, 1.450000e+01, 1.480000e+01, 1.510000e+01, 1.540000e+01, 1.570000e+01, 16, 1.630000e+01, 1.660000e+01 }, // SINR
          { 9.555556e-01, 8.784483e-01, 6.564103e-01, 3.521468e-01, 1.167672e-01, 2.020000e-02, 2.500000e-03, 3.000000e-04, 0 } // BLER
        }
      },
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 1.483000e+01, 1.512000e+01, 1.527000e+01, 1.541000e+01, 1.570000e+01 }, // SINR
          { 9.485294e-01, 9.474638e-01, 9.835391e-02, 2.380000e-02, 5.000000e-03, 0 } // BLER
        }
      },
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 1.434000e+01, 1.452000e+01, 1.470000e+01, 1.488000e+01, 1.505000e+01, 1.523000e+01, 1.540000e+01, 1.558000e+01, 1.575000e+01, 1.593000e+01, 1.610000e+01, 1.628000e+01 }, // SINR
          { 9.527778e-01, 8.896552e-01, 8.409091e-01, 5.952103e-01, 3.484116e-01, 1.357875e-01, 3.910000e-02, 1.420000e-02, 2.800000e-03, 4.000000e-04, 1.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 1.460000e+01, 15, 1.540000e+01, 1.580000e+01 }, // SINR
          { 9.154930e-01, 5.960648e-01, 1.600000e-03, 0 } // BLER
        }
      },
      { 144U, // SINR and BLER for CBS 144
        NrEesmErrorModel::DoubleTuple{
          { 1.360000e+01, 1.433000e+01, 1.470000e+01, 1.507000e+01, 1.580000e+01 }, // SINR
          { 1, 8.906250e-01, 4.975490e-01, 1.370000e-02, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 1.313830e+01, 1.466810e+01, 1.619790e+01, 1.772770e+01, 1.925750e+01 }, // SINR
          { 1, 8.640940e-01, 1.400838e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.360000e+01, 1.433000e+01, 1.470000e+01, 1.507000e+01, 1.580000e+01 }, // SINR
          { 1, 9.713740e-01, 7.120787e-01, 3.190000e-02, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.350000e+01, 1.423000e+01, 1.460000e+01, 1.497000e+01, 1.570000e+01 }, // SINR
          { 1, 9.990385e-01, 7.033784e-01, 2.746746e-01, 0 } // BLER
        }
      },
      { 256U, // SINR and BLER for CBS 256
        NrEesmErrorModel::DoubleTuple{
          { 1.481245e+01, 1.619790e+01, 1.741820e+01, 1.863850e+01 }, // SINR
          { 9.000000e-01, 5.760000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.495327e+01, 1.619790e+01, 1.729440e+01, 1.839090e+01 }, // SINR
          { 9.000000e-01, 6.020000e-02, 6.000000e-04, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.489070e+01, 1.619790e+01, 1.729440e+01, 1.839090e+01 }, // SINR
          { 9.000000e-01, 2.470000e-02, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.489070e+01, 1.619790e+01, 1.729440e+01, 1.839090e+01 }, // SINR
          { 9.000000e-01, 2.610000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 1.460000e+01, 1.480000e+01, 15, 1.520000e+01, 1.540000e+01, 1.560000e+01, 1.580000e+01, 16, 1.620000e+01, 1.640000e+01, 1.660000e+01 }, // SINR
          { 9.045139e-01, 7.960123e-01, 6.772487e-01, 4.733146e-01, 2.504950e-01, 1.067550e-01, 3.330000e-02, 1.140000e-02, 2.500000e-03, 6.000000e-04, 2.000000e-04, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.472460e+01, 1.619790e+01, 1.729440e+01, 1.839090e+01 }, // SINR
          { 9.000000e-01, 1.350000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.488842e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 1.080000e-02, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.475610e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 1.130000e-02, 1.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.466400e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 4.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.463540e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.430000e+01, 1.455000e+01, 1.480000e+01, 1.505000e+01, 1.530000e+01, 1.555000e+01, 1.580000e+01, 1.605000e+01, 1.630000e+01 }, // SINR
          { 9.865385e-01, 9.397810e-01, 7.858232e-01, 5.224490e-01, 2.231350e-01, 5.420000e-02, 1.030000e-02, 1.400000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.321220e+01, 1.457290e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 1, 8.362903e-01, 2.610000e-02, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.480000e+01, 15, 1.530000e+01, 1.580000e+01 }, // SINR
          { 1, 2.214411e-01, 2.088843e-01, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.467120e+01, 1.467123e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.406475e-01, 9.000000e-01, 2.500000e-02, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.471577e+01, 1.471580e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 8.093750e-01, 5.300000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.479165e+01, 1.479170e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 6.596154e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.413540e+01, 1.465100e+01, 1.499000e+01, 1.516670e+01, 1.534000e+01, 1.568230e+01 }, // SINR
          { 1, 9.980769e-01, 2.427326e-01, 1.258776e-01, 4.900000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.461335e+01, 1.461340e+01, 1.619790e+01, 1.729440e+01 }, // SINR
          { 9.000000e-01, 8.018868e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.380000e+01, 1.450000e+01, 15, 1.530000e+01, 1.550000e+01 }, // SINR
          { 1, 9.990385e-01, 5.049603e-01, 1.170000e-02, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 15, 1.560000e+01, 1.620000e+01 }, // SINR
          { 1, 8.362903e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.480000e+01, 1.513000e+01, 1.547000e+01, 1.580000e+01 }, // SINR
          { 1, 1.868499e-01, 1.000000e-02, 0 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.475415e+01, 1.475420e+01, 1.619790e+01 }, // SINR
          { 9.000000e-01, 5.927419e-01, 1.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.485415e+01, 1.485420e+01, 1.619790e+01 }, // SINR
          { 9.000000e-01, 4.475524e-01, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.440000e+01, 1.480000e+01, 15, 1.520000e+01, 1.560000e+01, 1.620000e+01 }, // SINR
          { 1, 9.961538e-01, 5.720721e-01, 1.169468e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.466220e+01, 1.466220e+01, 1.619790e+01 }, // SINR
          { 9.000000e-01, 6.567259e-01, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.460000e+01, 1.487000e+01, 15, 1.513000e+01, 1.540000e+01, 1.580000e+01 }, // SINR
          { 9.971154e-01, 8.503289e-01, 3.553922e-01, 8.670000e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.457290e+01, 1.457290e+01, 1.511460e+01, 1.565620e+01, 1.619790e+01 }, // SINR
          { 9.903846e-01, 9.000000e-01, 4.453671e-01, 1.560945e-01, 1.000000e-04 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.383380e+01, 1.501590e+01, 1.619800e+01, 1.738010e+01 }, // SINR
          { 9.913462e-01, 6.381250e-01, 1.400000e-03, 2.000000e-04 } // BLER
        }
      }
  },
  { // MCS 15
      { 64U, // SINR and BLER for CBS 64
        NrEesmErrorModel::DoubleTuple{
          { 1.474380e+01, 1.523740e+01, 1.573110e+01, 1.671850e+01, 1.819950e+01 }, // SINR
          { 1, 8.200637e-01, 3.506181e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.555000e+01, 1.580000e+01, 1.605000e+01, 1.630000e+01 }, // SINR
          { 9.980769e-01, 9.338768e-01, 4.499113e-01, 4.240000e-02, 3.000000e-04 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 1.490000e+01, 1.540000e+01, 1.567500e+01, 1.595000e+01, 1.622500e+01 }, // SINR
          { 1, 8.932292e-01, 2.979412e-01, 8.600000e-03, 1.000000e-04 } // BLER
        }
      },
      { 160U, // SINR and BLER for CBS 160
        NrEesmErrorModel::DoubleTuple{
          { 1.532510e+01, 1.573590e+01, 1.614670e+01, 1.696820e+01, 1.820050e+01 }, // SINR
          { 9.567669e-01, 6.349010e-01, 9.478913e-02, 3.000000e-04, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 1.471480e+01, 1.532740e+01, 1.563370e+01, 1.594000e+01, 1.655260e+01, 1.747150e+01 }, // SINR
          { 1, 9.066901e-01, 4.349315e-01, 2.320442e-01, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 1.469540e+01, 1.530230e+01, 1.560580e+01, 1.590930e+01, 1.651620e+01 }, // SINR
          { 1, 9.237589e-01, 6.399254e-01, 2.460938e-01, 1.000000e-04 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.488900e+01, 1.545360e+01, 1.573590e+01, 1.601820e+01, 1.658280e+01 }, // SINR
          { 1, 9.049296e-01, 3.200377e-01, 1.685734e-01, 1.000000e-04 } // BLER
        }
      },
      { 256U, // SINR and BLER for CBS 256
        NrEesmErrorModel::DoubleTuple{
          { 1.491990e+01, 1.546390e+01, 1.573590e+01, 1.600790e+01, 1.655190e+01 }, // SINR
          { 1, 7.330508e-01, 1.717258e-01, 2.480000e-02, 0 } // BLER
        }
      },
      { 304U, // SINR and BLER for CBS 304
        NrEesmErrorModel::DoubleTuple{
          { 1.501200e+01, 1.552720e+01, 1.578480e+01, 1.604240e+01, 1.655760e+01 }, // SINR
          { 1, 4.538530e-01, 2.257194e-01, 5.200000e-03, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 1.495090e+01, 1.547420e+01, 1.573590e+01, 1.599760e+01, 1.652090e+01 }, // SINR
          { 1, 8.012422e-01, 3.193384e-01, 4.260000e-02, 0 } // BLER
        }
      },
      { 368U, // SINR and BLER for CBS 368
        NrEesmErrorModel::DoubleTuple{
          { 1.515620e+01, 1.565620e+01, 1.599790e+01, 1.633950e+01 }, // SINR
          { 1, 3.212025e-01, 4.900000e-03, 0 } // BLER
        }
      },
      { 432U, // SINR and BLER for CBS 432
        NrEesmErrorModel::DoubleTuple{
          { 1.481270e+01, 1.535440e+01, 1.562530e+01, 1.589620e+01, 1.643790e+01 }, // SINR
          { 1, 8.827055e-01, 3.931889e-01, 5.720000e-02, 0 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 1.523870e+01, 1.551850e+01, 1.579830e+01, 1.635790e+01 }, // SINR
          { 9.836538e-01, 7.384393e-01, 2.781114e-01, 0 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 1.485540e+01, 1.539150e+01, 1.565950e+01, 1.592750e+01, 1.646360e+01 }, // SINR
          { 1, 9.196429e-01, 7.687126e-01, 8.080000e-02, 0 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 1.556470e+01, 1.592160e+01, 1.627850e+01, 1.663530e+01 }, // SINR
          { 9.614662e-01, 2.898509e-01, 4.800000e-03, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 1.558610e+01, 1.593940e+01, 1.629270e+01, 1.664600e+01 }, // SINR
          { 9.798077e-01, 3.994479e-01, 1.000000e-02, 1.000000e-04 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 1.562260e+01, 1.596980e+01, 1.614340e+01, 1.631700e+01, 1.666420e+01 }, // SINR
          { 9.102113e-01, 6.153382e-01, 2.390000e-02, 9.800000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 1.562260e+01, 1.583090e+01, 1.603920e+01, 1.645590e+01 }, // SINR
          { 9.500000e-01, 2.849099e-01, 3.280000e-02, 1.000000e-04 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 1.545590e+01, 1.579990e+01, 1.614400e+01, 1.648800e+01, 1.752000e+01 }, // SINR
          { 9.980769e-01, 6.769737e-01, 1.200000e-02, 2.000000e-04, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.532570e+01, 1.564900e+01, 1.582570e+01, 1.597240e+01, 1.629570e+01, 1.676580e+01 }, // SINR
          { 1, 6.834677e-01, 5.267490e-01, 1.000626e-01, 7.000000e-04, 0 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 1.564740e+01, 1.573630e+01, 1.582530e+01, 1.591420e+01, 1.641420e+01 }, // SINR
          { 9.828244e-01, 8.965517e-01, 7.134831e-01, 4.685185e-01, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 1.559650e+01, 1.580740e+01, 1.601830e+01, 1.644020e+01 }, // SINR
          { 9.961538e-01, 4.340753e-01, 1.121474e-01, 1.000000e-04 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 1.533920e+01, 1.573370e+01, 1.612810e+01, 1.652260e+01 }, // SINR
          { 1, 8.767123e-01, 4.050000e-02, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 1.533090e+01, 1.572670e+01, 1.612260e+01, 1.651840e+01 }, // SINR
          { 1, 8.649329e-01, 2.600000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 1.545590e+01, 1.583090e+01, 1.620590e+01, 1.658090e+01 }, // SINR
          { 9.980769e-01, 4.165296e-01, 1.500000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 1.587260e+01, 1.603920e+01, 1.620590e+01, 1.637260e+01, 1.687260e+01 }, // SINR
          { 9.522059e-01, 5.888761e-01, 1.596958e-01, 1.520000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 1.577400e+01, 1.606940e+01, 1.621720e+01, 1.636490e+01 }, // SINR
          { 9.589552e-01, 9.100000e-03, 7.200000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.557500e+01, 1.585000e+01, 1.612500e+01, 1.640000e+01 }, // SINR
          { 9.923077e-01, 8.301282e-01, 2.763158e-01, 1.600000e-02, 1.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 1.556730e+01, 1.584300e+01, 1.611880e+01, 1.639450e+01 }, // SINR
          { 9.846154e-01, 7.463873e-01, 6.286946e-01, 1.000000e-04 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 1.530000e+01, 1.557500e+01, 1.585000e+01, 1.612500e+01, 1.640000e+01 }, // SINR
          { 9.761450e-01, 6.709845e-01, 1.121307e-01, 2.400000e-03, 0 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 1.540000e+01, 1.575000e+01, 1.610000e+01, 1.645000e+01 }, // SINR
          { 9.696970e-01, 3.557961e-01, 5.900000e-03, 0 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 1.470000e+01, 1.553000e+01, 1.590000e+01, 1.637000e+01, 1.720000e+01 }, // SINR
          { 1, 7.385057e-01, 6.586788e-01, 1.350000e-02, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 15, 1.560000e+01, 1.590000e+01, 1.620000e+01, 1.680000e+01 }, // SINR
          { 1, 7.595588e-01, 3.525910e-01, 6.650000e-02, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.510000e+01, 1.553000e+01, 1.597000e+01, 1.640000e+01 }, // SINR
          { 1, 8.657718e-01, 2.328704e-01, 0 } // BLER
        }
      }
  },
  { // MCS 16
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 1.627230e+01, 1.652500e+01, 1.677780e+01, 1.703050e+01, 1.728320e+01, 1.778320e+01 }, // SINR
          { 9.971154e-01, 9.293478e-01, 4.697955e-01, 6.130000e-02, 1.800000e-03, 0 } // BLER
        }
      },
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 1.640000e+01, 1.665000e+01, 1.690000e+01, 1.715000e+01, 1.740000e+01 }, // SINR
          { 9.990385e-01, 8.306452e-01, 2.058824e-01, 8.400000e-03, 1.000000e-04 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 1.620000e+01, 1.650000e+01, 1.680000e+01, 1.710000e+01, 1.740000e+01, 1.770000e+01, 18 }, // SINR
          { 1, 5.497835e-01, 5.456009e-01, 2.439438e-01, 6.370000e-02, 1.060000e-02, 1.000000e-03 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 1.606000e+01, 1.633000e+01, 1.660000e+01, 1.690000e+01, 1.720000e+01, 1.740000e+01, 1.770000e+01, 1.797000e+01, 1.825000e+01 }, // SINR
          { 1, 8.741554e-01, 6.809211e-01, 3.365385e-01, 1.029657e-01, 3.500000e-02, 4.800000e-03, 1.000000e-04, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.665580e+01, 1.690580e+01, 1.715580e+01, 1.740580e+01, 1.765580e+01 }, // SINR
          { 9.501812e-01, 4.810606e-01, 5.850000e-02, 1.300000e-03, 1.000000e-04 } // BLER
        }
      }
  },
  { // MCS 17
      { 80U, // SINR and BLER for CBS 80
        NrEesmErrorModel::DoubleTuple{
          { 1.680000e+01, 1.730000e+01, 1.763000e+01, 1.780000e+01, 1.797000e+01, 1.830000e+01, 1.880000e+01 }, // SINR
          { 1, 9.402174e-01, 2.945804e-01, 2.178879e-01, 1.860000e-02, 7.000000e-04, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 1.758985e+01, 2.090370e+01, 2.266300e+01 }, // SINR
          { 9.000000e-01, 2.900000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 1.759430e+01, 2.090370e+01, 2.247730e+01 }, // SINR
          { 9.000000e-01, 1.400000e-03, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 1.758985e+01, 2.090370e+01, 2.235350e+01 }, // SINR
          { 9.000000e-01, 1.200000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 18
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 19
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 1.962500e+01, 2.025000e+01, 2.087500e+01, 2.150000e+01, 2.212500e+01, 2.275000e+01 }, // SINR
          { 9.788462e-01, 7.544118e-01, 2.986936e-01, 3.420000e-02, 1.600000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 20
      { 88U, // SINR and BLER for CBS 88
        NrEesmErrorModel::DoubleTuple{
          { 2.010000e+01, 2.060000e+01, 2.110000e+01, 2.160000e+01, 2.210000e+01 }, // SINR
          { 1, 3.409704e-01, 1.510000e-02, 6.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 120U, // SINR and BLER for CBS 120
        NrEesmErrorModel::DoubleTuple{
          { 2.010000e+01, 2.077000e+01, 2.110000e+01, 2.143000e+01, 2.210000e+01 }, // SINR
          { 1, 5.918605e-01, 6.500000e-02, 1.000000e-04, 0 } // BLER
        }
      },
      { 152U, // SINR and BLER for CBS 152
        NrEesmErrorModel::DoubleTuple{
          { 2.020000e+01, 2.055000e+01, 2.090000e+01, 2.125000e+01, 2.160000e+01, 2.210000e+01 }, // SINR
          { 9.470803e-01, 6.953552e-01, 2.780837e-01, 4.530000e-02, 2.500000e-03, 0 } // BLER
        }
      },
      { 184U, // SINR and BLER for CBS 184
        NrEesmErrorModel::DoubleTuple{
          { 2.013000e+01, 2.050000e+01, 2.090000e+01, 2.130000e+01, 2.160000e+01, 22, 2.238000e+01, 2.276000e+01 }, // SINR
          { 9.503676e-01, 8.295455e-01, 4.723783e-01, 1.336436e-01, 3.310000e-02, 1.800000e-03, 1.400000e-03, 0 } // BLER
        }
      },
      { 224U, // SINR and BLER for CBS 224
        NrEesmErrorModel::DoubleTuple{
          { 20, 2.037500e+01, 2.075000e+01, 2.112500e+01, 2.150000e+01 }, // SINR
          { 9.790076e-01, 6.844920e-01, 1.410112e-01, 6.300000e-03, 6.000000e-04 } // BLER
        }
      },
      { 256U, // SINR and BLER for CBS 256
        NrEesmErrorModel::DoubleTuple{
          { 2.040000e+01, 2.065000e+01, 2.090000e+01, 2.115000e+01, 2.140000e+01, 2.190000e+01 }, // SINR
          { 9.846154e-01, 8.844178e-01, 5.956573e-01, 2.411398e-01, 5.150000e-02, 0 } // BLER
        }
      },
      { 288U, // SINR and BLER for CBS 288
        NrEesmErrorModel::DoubleTuple{
          { 20, 2.050000e+01, 21, 2.150000e+01, 22, 2.250000e+01 }, // SINR
          { 1, 5.110000e-01, 1.870000e-02, 8.000000e-04, 3.000000e-04, 0 } // BLER
        }
      },
      { 320U, // SINR and BLER for CBS 320
        NrEesmErrorModel::DoubleTuple{
          { 2.020000e+01, 2.055000e+01, 2.090000e+01, 2.125000e+01, 2.160000e+01 }, // SINR
          { 9.685115e-01, 6.243902e-01, 1.174953e-01, 3.500000e-03, 0 } // BLER
        }
      },
      { 352U, // SINR and BLER for CBS 352
        NrEesmErrorModel::DoubleTuple{
          { 2.020000e+01, 2.055000e+01, 2.090000e+01, 2.125000e+01 }, // SINR
          { 9.062500e-01, 3.333333e-01, 1.990000e-02, 1.000000e-04 } // BLER
        }
      },
      { 384U, // SINR and BLER for CBS 384
        NrEesmErrorModel::DoubleTuple{
          { 1.999690e+01, 2.058880e+01, 2.118060e+01, 2.177250e+01 }, // SINR
          { 1, 6.621094e-01, 6.700000e-03, 0 } // BLER
        }
      },
      { 408U, // SINR and BLER for CBS 408
        NrEesmErrorModel::DoubleTuple{
          { 2.003000e+01, 2.040000e+01, 2.080000e+01, 2.110000e+01, 2.150000e+01, 2.190000e+01, 2.228000e+01 }, // SINR
          { 9.208633e-01, 8.741497e-01, 4.720149e-01, 1.921899e-01, 2.600000e-02, 1.500000e-03, 5.000000e-04 } // BLER
        }
      },
      { 456U, // SINR and BLER for CBS 456
        NrEesmErrorModel::DoubleTuple{
          { 2.030000e+01, 2.070000e+01, 2.110000e+01, 2.150000e+01, 2.180000e+01 }, // SINR
          { 9.166667e-01, 5.151822e-01, 9.199438e-02, 3.900000e-03, 1.000000e-04 } // BLER
        }
      },
      { 552U, // SINR and BLER for CBS 552
        NrEesmErrorModel::DoubleTuple{
          { 2.050000e+01, 2.080000e+01, 2.110000e+01, 2.140000e+01 }, // SINR
          { 9.654851e-01, 5.348101e-01, 5.050000e-02, 5.000000e-04 } // BLER
        }
      },
      { 704U, // SINR and BLER for CBS 704
        NrEesmErrorModel::DoubleTuple{
          { 2.030160e+01, 2.057220e+01, 2.084270e+01, 2.111330e+01, 2.138380e+01 }, // SINR
          { 9.923077e-01, 8.240446e-01, 2.622651e-01, 1.430000e-02, 1.000000e-04 } // BLER
        }
      },
      { 768U, // SINR and BLER for CBS 768
        NrEesmErrorModel::DoubleTuple{
          { 2.061370e+01, 2.098050e+01, 2.134730e+01, 2.171410e+01 }, // SINR
          { 9.923077e-01, 6.902174e-01, 9.318740e-02, 1.000000e-03 } // BLER
        }
      },
      { 848U, // SINR and BLER for CBS 848
        NrEesmErrorModel::DoubleTuple{
          { 2.050000e+01, 2.090000e+01, 2.120000e+01, 2.160000e+01, 22, 2.238000e+01 }, // SINR
          { 9.826923e-01, 7.952454e-01, 4.251672e-01, 5.910000e-02, 1.300000e-03, 0 } // BLER
        }
      },
      { 928U, // SINR and BLER for CBS 928
        NrEesmErrorModel::DoubleTuple{
          { 2.014250e+01, 2.082370e+01, 2.150480e+01, 2.218590e+01 }, // SINR
          { 1, 6.963315e-01, 1.300000e-03, 0 } // BLER
        }
      },
      { 984U, // SINR and BLER for CBS 984
        NrEesmErrorModel::DoubleTuple{
          { 2.040000e+01, 2.070000e+01, 21, 2.130000e+01, 2.160000e+01 }, // SINR
          { 9.633459e-01, 6.417910e-01, 1.289744e-01, 3.500000e-03, 0 } // BLER
        }
      },
      { 1064U, // SINR and BLER for CBS 1064
        NrEesmErrorModel::DoubleTuple{
          { 1.992000e+01, 2.040000e+01, 2.090000e+01, 2.130000e+01, 2.180000e+01, 2.230000e+01 }, // SINR
          { 9.923077e-01, 6.337500e-01, 5.120000e-02, 1.600000e-03, 3.000000e-04, 1.000000e-04 } // BLER
        }
      },
      { 1160U, // SINR and BLER for CBS 1160
        NrEesmErrorModel::DoubleTuple{
          { 2.038010e+01, 2.083270e+01, 2.128520e+01, 2.173780e+01 }, // SINR
          { 9.942308e-01, 3.433243e-01, 1.200000e-03, 0 } // BLER
        }
      },
      { 1256U, // SINR and BLER for CBS 1256
        NrEesmErrorModel::DoubleTuple{
          { 2.037500e+01, 2.082830e+01, 2.128160e+01, 2.173490e+01 }, // SINR
          { 1, 8.690878e-01, 5.140000e-02, 0 } // BLER
        }
      },
      { 1416U, // SINR and BLER for CBS 1416
        NrEesmErrorModel::DoubleTuple{
          { 2.040030e+01, 2.085000e+01, 2.129960e+01, 2.174930e+01 }, // SINR
          { 9.961538e-01, 4.089806e-01, 2.800000e-03, 0 } // BLER
        }
      },
      { 1544U, // SINR and BLER for CBS 1544
        NrEesmErrorModel::DoubleTuple{
          { 2.040550e+01, 2.085440e+01, 2.130340e+01, 2.175230e+01 }, // SINR
          { 1, 8.176101e-01, 3.950000e-02, 0 } // BLER
        }
      },
      { 1736U, // SINR and BLER for CBS 1736
        NrEesmErrorModel::DoubleTuple{
          { 2.039390e+01, 2.084450e+01, 2.129510e+01, 2.174570e+01 }, // SINR
          { 1, 4.062500e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 1864U, // SINR and BLER for CBS 1864
        NrEesmErrorModel::DoubleTuple{
          { 2.030000e+01, 2.080000e+01, 2.105000e+01, 2.130000e+01 }, // SINR
          { 9.020979e-01, 1.189573e-01, 3.800000e-03, 0 } // BLER
        }
      },
      { 2024U, // SINR and BLER for CBS 2024
        NrEesmErrorModel::DoubleTuple{
          { 2.040000e+01, 2.070000e+01, 21, 2.130000e+01, 2.160000e+01 }, // SINR
          { 9.855769e-01, 7.175141e-01, 1.689008e-01, 1.020000e-02, 5.000000e-04 } // BLER
        }
      },
      { 2216U, // SINR and BLER for CBS 2216
        NrEesmErrorModel::DoubleTuple{
          { 2.040000e+01, 2.065000e+01, 2.090000e+01, 2.115000e+01, 2.140000e+01 }, // SINR
          { 9.809160e-01, 6.925676e-01, 2.084718e-01, 2.290000e-02, 8.000000e-04 } // BLER
        }
      },
      { 2280U, // SINR and BLER for CBS 2280
        NrEesmErrorModel::DoubleTuple{
          { 2.007310e+01, 2.053640e+01, 2.076810e+01, 2.099980e+01, 2.146310e+01, 2.215810e+01 }, // SINR
          { 1, 8.020186e-01, 7.680723e-01, 3.638649e-01, 2.000000e-04, 0 } // BLER
        }
      },
      { 2536U, // SINR and BLER for CBS 2536
        NrEesmErrorModel::DoubleTuple{
          { 2.020000e+01, 2.070000e+01, 2.090000e+01, 2.110000e+01, 2.130000e+01, 2.150000e+01 }, // SINR
          { 9.971154e-01, 8.192675e-01, 3.915895e-01, 7.370000e-02, 4.300000e-03, 1.000000e-04 } // BLER
        }
      },
      { 2856U, // SINR and BLER for CBS 2856
        NrEesmErrorModel::DoubleTuple{
          { 2.062980e+01, 2.092540e+01, 2.122100e+01, 2.151660e+01, 2.181220e+01 }, // SINR
          { 1, 8.940972e-01, 1.938462e-01, 3.000000e-03, 1.000000e-04 } // BLER
        }
      },
      { 3104U, // SINR and BLER for CBS 3104
        NrEesmErrorModel::DoubleTuple{
          { 2.051700e+01, 2.102220e+01, 2.152740e+01, 2.203260e+01 }, // SINR
          { 9.393116e-01, 2.638598e-01, 1.900000e-03, 0 } // BLER
        }
      },
      { 3496U, // SINR and BLER for CBS 3496
        NrEesmErrorModel::DoubleTuple{
          { 1.965000e+01, 2.020000e+01, 2.070000e+01, 2.130000e+01, 2.190000e+01, 2.240000e+01 }, // SINR
          { 1, 8.758621e-01, 2.028491e-01, 9.000000e-04, 3.000000e-04, 0 } // BLER
        }
      },
      { 3752U, // SINR and BLER for CBS 3752
        NrEesmErrorModel::DoubleTuple{
          { 1.990000e+01, 2.050000e+01, 2.093000e+01, 2.120000e+01, 2.137000e+01, 2.180000e+01, 2.250000e+01 }, // SINR
          { 1, 9.734848e-01, 4.146440e-01, 1.844624e-01, 7.780000e-02, 8.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 21
      { 96U, // SINR and BLER for CBS 96
        NrEesmErrorModel::DoubleTuple{
          { 2.110000e+01, 2.160000e+01, 22, 2.250000e+01, 2.290000e+01 }, // SINR
          { 1, 6.717105e-01, 6.010000e-02, 5.000000e-04, 0 } // BLER
        }
      },
      { 128U, // SINR and BLER for CBS 128
        NrEesmErrorModel::DoubleTuple{
          { 2.080000e+01, 2.130000e+01, 2.180000e+01, 2.230000e+01, 2.280000e+01 }, // SINR
          { 1, 9.903846e-01, 3.617479e-01, 1.000000e-03, 0 } // BLER
        }
      },
      { 160U, // SINR and BLER for CBS 160
        NrEesmErrorModel::DoubleTuple{
          { 2.070000e+01, 2.160000e+01, 2.210000e+01, 2.250000e+01 }, // SINR
          { 1, 2.356673e-01, 1.090000e-02, 0 } // BLER
        }
      },
      { 192U, // SINR and BLER for CBS 192
        NrEesmErrorModel::DoubleTuple{
          { 2.060000e+01, 2.153000e+01, 22, 2.247000e+01, 2.340000e+01 }, // SINR
          { 1, 2.240000e-02, 3.300000e-03, 2.000000e-04, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 2.040000e+01, 2.133000e+01, 2.180000e+01, 2.227000e+01 }, // SINR
          { 1, 7.581361e-01, 6.770000e-02, 0 } // BLER
        }
      },
      { 272U, // SINR and BLER for CBS 272
        NrEesmErrorModel::DoubleTuple{
          { 2.050000e+01, 2.140000e+01, 2.190000e+01, 2.230000e+01 }, // SINR
          { 1, 2.046010e-01, 3.200000e-03, 0 } // BLER
        }
      }
  },
  { // MCS 22
      { 104U, // SINR and BLER for CBS 104
        NrEesmErrorModel::DoubleTuple{
          { 2.190000e+01, 2.230000e+01, 2.260000e+01, 2.280000e+01, 2.290000e+01, 2.320000e+01 }, // SINR
          { 1, 9.713740e-01, 1.618404e-01, 3.770000e-02, 3.300000e-03, 0 } // BLER
        }
      },
      { 208U, // SINR and BLER for CBS 208
        NrEesmErrorModel::DoubleTuple{
          { 2.226620e+01, 2.283390e+01, 2.340150e+01, 2.396910e+01 }, // SINR
          { 9.971154e-01, 3.111111e-01, 2.300000e-03, 0 } // BLER
        }
      },
      { 240U, // SINR and BLER for CBS 240
        NrEesmErrorModel::DoubleTuple{
          { 2.100600e+01, 2.216900e+01, 2.255670e+01, 2.294430e+01, 2.333200e+01 }, // SINR
          { 1, 9.298561e-01, 7.242188e-01, 1.140000e-02, 0 } // BLER
        }
      },
      { 288U, // SINR and BLER for CBS 288
        NrEesmErrorModel::DoubleTuple{
          { 2.220000e+01, 2.260000e+01, 23, 2.340000e+01 }, // SINR
          { 1, 8.282258e-01, 4.160000e-02, 0 } // BLER
        }
      }
  },
  { // MCS 23
    { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
  },
  { // MCS 24
      { 72U, // SINR and BLER for CBS 72
        NrEesmErrorModel::DoubleTuple{
          { 2.380000e+01, 2.420000e+01, 2.460000e+01, 25, 2.530000e+01, 2.568000e+01, 2.605000e+01, 2.643000e+01, 2.680000e+01, 2.718000e+01, 2.755000e+01, 2.793000e+01, 2.830000e+01, 2.868000e+01 }, // SINR
          { 9.923077e-01, 9.694656e-01, 8.809122e-01, 6.824866e-01, 4.893822e-01, 3.757353e-01, 1.770099e-01, 6.950000e-02, 1.870000e-02, 3.900000e-03, 6.000000e-04, 1.000000e-04, 1.000000e-04, 0 } // BLER
        }
      },
      { 112U, // SINR and BLER for CBS 112
        NrEesmErrorModel::DoubleTuple{
          { 2.280000e+01, 2.380000e+01, 2.447000e+01, 2.480000e+01, 2.513000e+01, 2.580000e+01 }, // SINR
          { 1, 9.474638e-01, 3.091299e-01, 1.484929e-01, 2.000000e-04, 0 } // BLER
        }
      }
  },
  { // MCS 25
      { 256U, // SINR and BLER for CBS 256
        NrEesmErrorModel::DoubleTuple{
          { 25, 2.550000e+01, 26, 2.650000e+01, 27 }, // SINR
          { 9.759615e-01, 6.387500e-01, 1.095395e-01, 2.400000e-03, 0 } // BLER
        }
      }
  }
}
};

std::vector<std::string>
NrEesmErrorModel::m_bgTypeName = { "BG1" , "BG2" };

NrEesmErrorModel::NrEesmErrorModel () : NrErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

NrEesmErrorModel::~NrEesmErrorModel ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrEesmErrorModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrEesmErrorModel")
    .SetParent<NrErrorModel> ()
    .AddConstructor<NrEesmErrorModel> ()
    .AddAttribute ("McsTable",
                   "Type of the NR Table to use in NR EESM Error Model",
                   EnumValue (NrEesmErrorModel::McsTable1),
                   MakeEnumAccessor (&NrEesmErrorModel::GetMcsTable,
                                     &NrEesmErrorModel::SetMcsTable),
                   MakeEnumChecker (NrEesmErrorModel::McsTable1, "McsTable1",
                                    NrEesmErrorModel::McsTable2, "McsTable2"))
    .AddAttribute ("HarqMethod",
                   "HARQ method for PHY abstraction of HARQ",
                   EnumValue (NrEesmErrorModel::HarqCc),
                   MakeEnumAccessor (&NrEesmErrorModel::GetHarqMethod,
                                     &NrEesmErrorModel::SetHarqMethod),
                   MakeEnumChecker (NrEesmErrorModel::HarqCc, "HarqCc",
                                    NrEesmErrorModel::HarqIr, "HarqIr"))
  ;
  return tid;
}

TypeId
NrEesmErrorModel::GetInstanceTypeId() const
{
  return NrEesmErrorModel::GetTypeId ();
}

double
NrEesmErrorModel::SinrEff (const SpectrumValue& sinr, const std::vector<int>& map, uint8_t mcs)
{
  NS_LOG_FUNCTION (sinr << &map << (uint8_t) mcs);
  NS_ASSERT (m_betaTable != nullptr);
  NS_ABORT_MSG_IF (map.size () == 0,
                   " Error: number of allocated RBs cannot be 0 - EESM method - SinrEff function");

  double SINR = 0.0;
  double SINRsum = 0.0;
  SpectrumValue sinrCopy = sinr;

  double beta = m_betaTable->at (mcs);

  for (uint32_t i = 0; i < map.size (); i++)
    {
      double sinrLin = sinrCopy[map.at (i)];
      SINR = exp ( -sinrLin / beta );
      SINRsum += SINR;
    }

  SINR = -beta * log ( SINRsum / map.size () );

  NS_LOG_INFO (" Effective SINR = " << SINR);

  return SINR;
}


const std::vector<double> &
NrEesmErrorModel::GetSinrDbVectorFromSimulatedValues (NrEesmErrorModel::GraphType graphType,
                                                      uint8_t mcs, uint32_t cbSizeIndex) const
{
  return std::get<0> (m_simulatedBlerFromSINR->at (graphType).at (mcs).at (cbSizeIndex));
}

const std::vector<double> &
NrEesmErrorModel::GetBLERVectorFromSimulatedValues (NrEesmErrorModel::GraphType graphType,
                                                    uint8_t mcs, uint32_t cbSizeIndex) const
{
  return std::get<1> (m_simulatedBlerFromSINR->at (graphType).at (mcs).at (cbSizeIndex));
}

double
NrEesmErrorModel::MappingSinrBler (double sinr, uint8_t mcs, uint32_t cbSizeBit)
{
  NS_LOG_FUNCTION (sinr << (uint8_t) mcs << (uint32_t) cbSizeBit);
  NS_ABORT_MSG_IF (mcs > GetMaxMcs (), "MCS out of range [0..27/28]: " << static_cast<uint8_t> (mcs));

  // use cbSize to obtain the index of CBSIZE in the map, jointly with mcs and sinr. take the
  // lowest CBSIZE simulated including this CB for removing CB size quatization
  // errors. sinr is also lower-bounded.
  double bler = 0.0;
  double sinr_db = 10 * log10 (sinr);
  GraphType bg_type = GetBaseGraphType (cbSizeBit, mcs);

  // Get the index of CBSIZE in the map
  NS_LOG_INFO ("For sinr " << sinr << " and mcs " << static_cast<uint8_t>(mcs) <<
                " CbSizebit " << cbSizeBit << " we got bg type " << m_bgTypeName[bg_type]);
  auto cbMap = m_simulatedBlerFromSINR->at (bg_type).at (mcs);
  auto cbIt = cbMap.upper_bound (cbSizeBit);

  if (cbIt != cbMap.begin ())
    {
      cbIt--;
    }

  if (sinr_db < GetSinrDbVectorFromSimulatedValues (bg_type, mcs, cbIt->first).front ())
    {
      bler = 1.0;
    }
  else if (sinr_db > GetSinrDbVectorFromSimulatedValues (bg_type, mcs, cbIt->first).back ())
    {
      bler = 0.0;
    }
  else
    {
      // Get the index of SINR in the vector
      auto sinrIt = std::upper_bound (GetSinrDbVectorFromSimulatedValues (bg_type, mcs, cbIt->first).begin (),
                                      GetSinrDbVectorFromSimulatedValues (bg_type, mcs, cbIt->first).end (),
                                      sinr_db);

      if (sinrIt != GetSinrDbVectorFromSimulatedValues (bg_type, mcs, cbIt->first).begin ())
        {
          sinrIt--;
        }

      auto sinr_index = std::distance (GetSinrDbVectorFromSimulatedValues (bg_type, mcs, cbIt->first).begin (),
                                       sinrIt);
      bler = GetBLERVectorFromSimulatedValues (bg_type, mcs, cbIt->first).at (sinr_index);
    }

  NS_LOG_LOGIC ("SINR effective: " << sinr << " BLER:" << bler);
  return bler;
}

NrEesmErrorModel::GraphType
NrEesmErrorModel::GetBaseGraphType (uint32_t tbSizeBit, uint8_t mcs) const
{
  NS_ASSERT (m_mcsEcrTable != nullptr);
  double ecr = m_mcsEcrTable->at (mcs);

  GraphType bg_type = FIRST;
  if (tbSizeBit <= 292 || ecr <= 0.25 || (tbSizeBit <= 3824 && ecr <= 0.67))
    {
      bg_type = SECOND;
    }
  return bg_type;
}

// codeblock segmentation and CRC attachment as per TS 38.212 (EESM method and LDPC coding)
Ptr<NrErrorModelOutput>
NrEesmErrorModel::GetTbDecodificationStats (const SpectrumValue& sinr, const std::vector<int>& map,
                                            uint32_t size, uint8_t mcs,
                                            const NrErrorModelHistory &sinrHistory)
{
  return GetTbBitDecodificationStats (sinr, map, size * 8, mcs, sinrHistory);
}

static std::string
PrintMap (const std::vector<int> &map)
{
  std::stringstream ss;

  for (const auto &v : map)
    {
      ss << v << ", ";
    }

  return ss.str ();

}
Ptr<NrErrorModelOutput>
NrEesmErrorModel::GetTbBitDecodificationStats (const SpectrumValue& sinr,
                                               const std::vector<int>& map,
                                               uint32_t sizeBit, uint8_t mcs,
                                               const NrErrorModelHistory &sinrHistory)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (mcs > GetMaxMcs ());

  double tbSinr = SinrEff (sinr, map, mcs);
  double SINR = tbSinr;

  NS_LOG_DEBUG (" mcs " << static_cast<uint8_t>(mcs) << " TBSize in bit " << sizeBit <<
                " history elements: " << sinrHistory.size () << " SINR of the tx: " <<
                tbSinr << std::endl << "MAP: " << PrintMap (map) << std::endl <<
                "SINR: " << sinr);

  double Reff = 0.0;

  if (sinrHistory.size () > 0)
    {

      if (m_harqMethod == HarqCc)
        {
          // HARQ CHASE COMBINING: update SINReff, but not ECR after retx
          // repetition of coded bits

          // make a vector of history that contains the last tx (but without modifying
          // sinrHistory, as it will be modified by the caller when it will be the time)
          Ptr<NrEesmErrorModelOutput> last = Create<NrEesmErrorModelOutput> (0.0);
          last->m_map = map;
          last->m_sinr = sinr;

          NrErrorModelHistory total = sinrHistory;
          total.push_back (last);

          // evaluate SINR_eff over "total", as per Chase Combining

          NS_ASSERT (sinr.GetSpectrumModel()->GetNumBands() == sinr.GetValuesN());

          SpectrumValue sinr_sum (sinr.GetSpectrumModel());
          uint32_t historySize = static_cast<uint32_t> (total.size ());
          uint32_t maxRBUsed = 0;
          for (uint32_t i = 0; i < historySize; ++i)
            {
              Ptr<NrEesmErrorModelOutput> output = DynamicCast<NrEesmErrorModelOutput> (total.at(i));
              maxRBUsed = std::max (maxRBUsed, static_cast<uint32_t> (output->m_map.size ()));
            }

          std::vector<int> map_sum;
          map_sum.reserve (maxRBUsed);

          for (uint32_t i = 0 ; i < maxRBUsed; ++i)
            {
              sinr_sum[i] = 0;
              map_sum.push_back (static_cast<int> (i));
            }

          /* combine at the bit level. Example:
           * SINR{1}=[0 0 10 20 10 0 0];
           * SINR{2}=[1 2 1 2 1 0 3];
           * SINR{3}=[5 0 0 0 0 0 0];
           *
           * map{1}=[2 3 4];
           * map{2}=[0 1 2 3 4 6];
           * map{3}=[0];
           *
           * MAP_SUM = [0 1 2 3 4 5]
           * SINR_SUM = [16 27 16 17 26 18]
           *
           * (the value at SINR_SUM[0] is SINR{1}[2] + SINR{2}[0] + SINR{3}[0])
           */
          for (uint32_t i = 0; i < historySize; ++i)
            {
              Ptr<NrEesmErrorModelOutput> output = DynamicCast<NrEesmErrorModelOutput> (total.at(i));
              uint32_t size = output->m_map.size ();
              for (uint32_t j = 0 ; j < maxRBUsed; ++j)
                {
                  sinr_sum[j] += output->m_sinr [ output->m_map [ j % size ] ];
                }
            }

          NS_LOG_INFO ("\tHISTORY:");
          for (const auto & element : total)
            {
              Ptr<NrEesmErrorModelOutput> output = DynamicCast<NrEesmErrorModelOutput> (element);
              NS_LOG_INFO ("\tMAP:" << PrintMap (output->m_map));
              NS_LOG_INFO ("\tSINR: " << output->m_sinr);
            }

          NS_LOG_INFO ("MAP_SUM: " << PrintMap (map_sum));
          NS_LOG_INFO ("SINR_SUM: " << sinr_sum);

          // compute effective SINR with the sinr_sum vector and map_sum RB map
          SINR = SinrEff (sinr_sum, map_sum, mcs);
        }
      else
        {
          // HARQ INCREMENTAL REDUNDANCY: update SINReff and ECR after retx
          // no repetition of coded bits

          NS_ASSERT (m_harqMethod == HarqIr);
          std::vector<int> map_sum = map;

          // evaluate SINR_eff over "total", as per Incremental Redundancy.
          // combine at the bit level.
          SpectrumValue sinr_sum = sinr;
          double SINReff_previousTx = DynamicCast<NrEesmErrorModelOutput> (sinrHistory.back ())->m_sinrEff;
          NS_LOG_INFO ("\tHISTORY:");
          NS_LOG_INFO ("\tSINReff: " << SINReff_previousTx);

          for (uint32_t i = 0; i < map.size (); i++)
             {
                sinr_sum[map.at(i)] += SINReff_previousTx;
             }

          NS_LOG_INFO ("MAP_SUM: " << PrintMap (map_sum));
          NS_LOG_INFO ("SINR_SUM: " << sinr_sum);

          // compute equivalent effective code rate after retransmissions
          uint32_t codeBitsSum = 0;
          uint32_t infoBits = DynamicCast<NrEesmErrorModelOutput> (sinrHistory.front ())->m_infoBits;  // information bits of the first TB

          for (const Ptr<NrErrorModelOutput> & output : sinrHistory)
            {
              Ptr<NrEesmErrorModelOutput> sinrHistorytemp = DynamicCast<NrEesmErrorModelOutput> (output);
              NS_ASSERT (sinrHistorytemp != nullptr);

              NS_LOG_DEBUG (" Effective SINR " << sinrHistorytemp->m_sinrEff << " codeBits " << sinrHistorytemp->m_codeBits <<
                            " infoBits: " << sinrHistorytemp->m_infoBits);

              codeBitsSum += sinrHistorytemp->m_codeBits;
            }

          codeBitsSum += sizeBit / m_mcsEcrTable->at (mcs);;
          Reff = infoBits / static_cast<double> (codeBitsSum);

          NS_LOG_INFO (" Reff " << Reff << " HARQ history (previous) " << sinrHistory.size ());

          // compute effective SINR with the sinr_sum vector and map_sum RB map
          SINR = SinrEff (sinr_sum, map_sum, mcs);
        }
    }

  NS_LOG_DEBUG (" SINR after processing all retx (if any): " << SINR << " SINR last tx" << tbSinr);

  // selection of LDPC base graph type (1 or 2), as per TS 38.212
  GraphType bg_type = GetBaseGraphType (sizeBit, mcs);
  NS_LOG_INFO ("BG type selection: " << bg_type);

  uint16_t Kcb;
  uint8_t Kb;
  // estimate CB size (according to Section 5.2.2 of TS 38.212)
  if (bg_type == FIRST)
    {
      Kcb = 8448;  // max size of a codeblock (including CRC) for BG1
      Kb = 22;
    }
  else
    {
      NS_ASSERT (bg_type == SECOND);
      Kcb = 3840;  // max size of a codeblock (including CRC) for BG2
      if (sizeBit >= 640)
        {
          Kb = 10;
        }
      else if (sizeBit >= 560)
        {
          Kb = 9;
        }
      else if (sizeBit >= 192)
        {
          Kb = 8;
        }
      else
        {
          Kb = 6;
        }
    }

  uint32_t B = sizeBit; // TBS in bits
  uint32_t L = 0;
  uint32_t C = 0; // no. of codeblocks
  uint32_t B1 = 0;

  if (B <= Kcb)
    {
      // only one codeblock
      L = 0;
      C = 1;
      B1 = B;
    }
  else
    {
      L = 24;
      C = ceil ((double)B / ((double)(Kcb - L)));
      B1 = B + C * L;
    }

  // Zc = minimum Z in all sets of lifting sizes table such that Kb * Z >= K1
  uint32_t K1 = B1 / C;

  // returns an iterator pointing to the first element in the range [first,last)
  // which compares greater than the third parameter.
  std::vector<uint16_t>::const_iterator ZcIt = std::upper_bound (LiftingSizeTableBG.begin (),
                                                                 LiftingSizeTableBG.end (),
                                                                 (static_cast<double> (K1) / Kb) + 0.001);
  uint16_t Zc = *ZcIt;
  uint32_t K;

  if (bg_type == FIRST)
    {
      K = Zc * 22;  // no. of bits in each code block
    }
  else  // bg_type==2
    {
      NS_ASSERT (bg_type == SECOND);
      K = Zc * 10;  // no. of bits in each code block
    }

  NS_LOG_INFO ("EESMErrorModel: TBS of " << B << " needs of " << B1 <<
               " bits distributed in " << C << " CBs of " << K << " bits");

  // PHY abstraction for HARQ-IR retx -> get closest ECR to Reff from the
  // available ones that belong to the same modulation order
  uint8_t mcs_eq = mcs;
  if ((sinrHistory.size () > 0) && (m_harqMethod == HarqIr))
    {
      uint8_t ModOrder = m_mcsMTable->at (mcs);

      for (uint8_t mcsindex = GetMaxMcs (); mcsindex >= 0; mcsindex--)
        {
          if ((m_mcsMTable->at (mcsindex) == ModOrder) &&
              (m_mcsEcrTable->at (mcsindex) > Reff))
            {
              mcs_eq--;
            }
        }
     }

  NS_LOG_DEBUG (" MCS of tx " << mcs <<
                " Equivalent MCS for PHY abstraction (just for HARQ-IR) " << mcs_eq);

  double errorRate = 1.0;
  if (C != 1)
    {
      double cbler = MappingSinrBler (SINR, mcs_eq, K);
      errorRate = 1.0 - pow (1.0 - cbler, C);
    }
  else
    {
      errorRate = MappingSinrBler (SINR, mcs_eq, K);
    }

  NS_LOG_DEBUG ("Calculated Error rate " << errorRate);
  NS_ASSERT (m_mcsEcrTable != nullptr);

  Ptr<NrEesmErrorModelOutput> ret = Create<NrEesmErrorModelOutput> (errorRate);
  ret->m_sinr = sinr;
  ret->m_map = map;
  ret->m_sinrEff = SINR;
  ret->m_infoBits = sizeBit;
  ret->m_codeBits = sizeBit / m_mcsEcrTable->at (mcs);

  return ret;
}

double
NrEesmErrorModel::GetSpectralEfficiencyForCqi (uint8_t cqi)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_spectralEfficiencyForCqi != nullptr);
  NS_ABORT_MSG_UNLESS (cqi >= 0 && cqi <= 15, "CQI must be in [0..15] = " << cqi);

  return m_spectralEfficiencyForCqi->at (cqi);
}

double
NrEesmErrorModel::GetSpectralEfficiencyForMcs (uint8_t mcs)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_spectralEfficiencyForMcs != nullptr);
  NS_ABORT_IF (mcs > GetMaxMcs ());

  return m_spectralEfficiencyForMcs->at (mcs);
}

uint32_t
NrEesmErrorModel::GetPayloadSize (uint32_t usefulSc, uint8_t mcs, uint32_t rbNum) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_mcsEcrTable != nullptr);
  const uint32_t rscElement = usefulSc * rbNum;
  double Rcode = m_mcsEcrTable->at (mcs);
  uint8_t Qm = m_mcsMTable->at (mcs);

  const double spectralEfficiency = rscElement * Qm * Rcode;

  return static_cast<uint32_t> (std::floor (spectralEfficiency / 8));
}

uint32_t
NrEesmErrorModel::GetMaxCbSize (uint32_t tbSize, uint8_t mcs) const
{
  GraphType bgType = GetBaseGraphType (tbSize * 8, mcs);
  if (bgType == FIRST)
    {
      return LiftingSizeTableBG.back () * 22 / 8;  // return CBsize in bytes
    }
  NS_ASSERT (bgType == SECOND);
  return LiftingSizeTableBG.back () * 10 / 8;     // return CBsize in bytes
}

uint8_t
NrEesmErrorModel::GetMaxMcs ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_mcsEcrTable != nullptr);
  return static_cast<uint8_t> (m_mcsEcrTable->size () - 1);
}

void
NrEesmErrorModel::SetMcsTable (NrEesmErrorModel::McsTable input)
{
  NS_LOG_FUNCTION (this << input);

  if (input == McsTable1)
    {
      m_betaTable = &BetaTable1;
      m_mcsEcrTable = &McsEcrTable1;
      m_simulatedBlerFromSINR = &BlerForSinr1;
      m_mcsMTable = &McsMTable1;
      m_spectralEfficiencyForMcs = &SpectralEfficiencyForMcs1;
      m_spectralEfficiencyForCqi = &SpectralEfficiencyForCqi1;
    }
  else
    {
      NS_ASSERT (input == McsTable2);
      m_betaTable = &BetaTable2;
      m_mcsEcrTable = &McsEcrTable2;
      m_simulatedBlerFromSINR = &BlerForSinr2;
      m_mcsMTable = &McsMTable2;
      m_spectralEfficiencyForMcs = &SpectralEfficiencyForMcs2;
      m_spectralEfficiencyForCqi = &SpectralEfficiencyForCqi2;
    }
}

NrEesmErrorModel::McsTable
NrEesmErrorModel::GetMcsTable () const
{
  NS_LOG_FUNCTION (this);
  return m_mcsTable;
}

void
NrEesmErrorModel::SetHarqMethod (NrEesmErrorModel::HarqMethod input)
{
  NS_LOG_FUNCTION (this << input);

  if (input == HarqCc)
    {
      m_harqMethod = HarqCc;
    }
  else
    {
      NS_ASSERT (input == HarqIr);
      m_harqMethod = HarqIr;
    }
}

NrEesmErrorModel::HarqMethod
NrEesmErrorModel::GetHarqMethod () const
{
  NS_LOG_FUNCTION (this);
  return m_harqMethod;
}


} // namespace ns3

