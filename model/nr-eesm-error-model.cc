/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2018 CTTC Sandra Lagen  <sandra.lagen@cttc.es>
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
  0.08, 0.1, 0.11, 0.15, 0.19, 0.24, 0.3, 0.37, 0.44, 0.51, // ECRs of MCSs
  // 16QAM (M=4)
  0.3, 0.33, 0.37, 0.42, 0.48, 0.54, 0.6, // ECRs of MCSs
  // 64QAM (M=6)
  0.43, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.89, 0.92 // ECRs of MCSs
};

/**
 * \brief Table of ECR of the standard MCSs: 28 MCSs as per Table2 in TS38.214
 */
static const std::vector<double> McsEcrTable2 = {
  // QPSK (M=2)
  0.11, 0.18, 0.30, 0.43, 0.58, // ECRs of MCSs
  // 16QAM (M=4)
  0.36, 0.42, 0.47, 0.54, 0.60, 0.64, // ECRs of MCSs
  // 64QAM (M=6)
  0.45, 0.50, 0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, // ECRs of MCSs
  // 256QAM (M=8)
  0.66, 0.69, 0.73, 0.77, 0.82, 0.86, 0.89, 0.92 // ECRs of MCSs
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
  0.2344, 0.3066, 0.377, 0.4902, 0.616, 0.7402, 0.877, 1.0273, 1.1758, 1.3262, // SEs of MCSs
  // 16QAM (M=4)
  1.3281, 1.4766, 1.6953, 1.9141, 2.1602, 2.4063, 2.5703, // SEs of MCSs
  // 64QAM (M=6)
  2.5664, 2.7305, 3.0293, 3.3223, 3.6094, 3.9023, 4.2129, 4.5234, 4.8164,
  5.1152, 5.3320, 5.5547  // SEs of MCSs
};

/**
 * \brief Table of SE of the standard MCSs: 28 (0 to 27) MCSs as per Table2 in TS38.214
 */
static const std::vector<double> SpectralEfficiencyForMcs2 = {
  // QPSK (M=2)
  0.2344, 0.3770, 0.6016, 0.8770, 1.1758, // SEs of MCSs
  // 16QAM (M=4)
  1.4766, 1.6953, 1.9141, 2.1602, 2.4063, 2.5703, // SEs of MCSs
  // 64QAM (M=6)
  2.7305, 3.0293, 3.3223, 3.6094, 3.9023, 4.2129, 4.5234, 4.8164, 5.1152, // SEs of MCSs
  // 256QAM (M=8)
  5.3320, 5.5547, 5.8906, 6.2266, 6.5703, 6.9141, 7.1602, 7.4063 // SEs of MCSs
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
  0.15, 0.37, 0.87,
  1.47, 1.91, 2.40,
  2.73, 3.32, 3.90, 4.52, 5.11,
  5.55, 6.22, 6.91, 7.40
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
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 5
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 6
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 7
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 8
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 9
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 10
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 11
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 12
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 13
            { 3752U, // SINR and BLER for CBS 3752
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 0.999038, 0.914007, 0.324289, 0.0077 } // BLER
              }
            },
            { 3840U, // SINR and BLER for CBS 3840
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.90493, 0.257398, 0.0044 } // BLER
              }
            },
            { 4096U, // SINR and BLER for CBS 4096
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.933036, 0.278846, 0.0044 } // BLER
              }
            },
            { 4480U, // SINR and BLER for CBS 4480
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.960185, 0.372411, 0.0095 } // BLER
              }
            },
            { 4864U, // SINR and BLER for CBS 4864
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.988462, 0.500977, 0.0183 } // BLER
              }
            },
            { 5248U, // SINR and BLER for CBS 5248
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.967557, 0.408065, 0.0068 } // BLER
              }
            },
            { 5504U, // SINR and BLER for CBS 5504
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.985577, 0.476124, 0.0101 } // BLER
              }
            },
            { 6272U, // SINR and BLER for CBS 6272
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.993269, 0.493269, 0.008 } // BLER
              }
            },
            { 6912U, // SINR and BLER for CBS 6912
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.995192, 0.53692, 0.0083 } // BLER
              }
            },
            { 7552U, // SINR and BLER for CBS 7552
              NrEesmErrorModel::DoubleTuple{
                { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
                { 1, 1, 0.993269, 0.41299, 0.005 } // BLER
              }
            }
        },
        { // MCS 14
            { 3752U, // SINR and BLER for CBS 3752
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.964962, 0.513609, 0.0357, 0.0001 } // BLER
              }
            },
            { 3840U, // SINR and BLER for CBS 3840
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.961174, 0.455197, 0.0222, 0.0001 } // BLER
              }
            },
            { 4096U, // SINR and BLER for CBS 4096
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.974038, 0.445423, 0.0186, 0.0001 } // BLER
              }
            },
            { 4480U, // SINR and BLER for CBS 4480
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.979008, 0.42839, 0.0142, 0.0001 } // BLER
              }
            },
            { 4864U, // SINR and BLER for CBS 4864
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.989423, 0.683824, 0.0466, 0.0002 } // BLER
              }
            },
            { 5120U, // SINR and BLER for CBS 5120
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.985577, 0.572072, 0.0341, 0.0001 } // BLER
              }
            },
            { 5504U, // SINR and BLER for CBS 5504
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.992308, 0.581422, 0.024, 0.0001 } // BLER
              }
            },
            { 6272U, // SINR and BLER for CBS 6272
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.992308, 0.543269, 0.0161, 0.0001 } // BLER
              }
            },
            { 6912U, // SINR and BLER for CBS 6912
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.997115, 0.706704, 0.0358, 0 } // BLER
              }
            },
            { 7680U, // SINR and BLER for CBS 7680
              NrEesmErrorModel::DoubleTuple{
                { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
                { 1, 0.993269, 0.572635, 0.0134, 0 } // BLER
              }
            }
        },
        { // MCS 15
            { 3752U, // SINR and BLER for CBS 3752
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 0.995192, 0.879281, 0.335106, 0.0214, 0.0001 } // BLER
              }
            },
            { 3840U, // SINR and BLER for CBS 3840
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 0.992308, 0.715084, 0.125, 0.0017, 0 } // BLER
              }
            },
            { 4096U, // SINR and BLER for CBS 4096
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 0.995192, 0.741228, 0.111877, 0.0013, 0 } // BLER
              }
            },
            { 4480U, // SINR and BLER for CBS 4480
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 0.997115, 0.733382, 0.108188, 0.0006, 0 } // BLER
              }
            },
            { 4864U, // SINR and BLER for CBS 4864
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 0.994231, 0.764137, 0.0990313, 0.0006, 0.0001 } // BLER
              }
            },
            { 5248U, // SINR and BLER for CBS 5248
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 0.998077, 0.709722, 0.0716, 0.0002, 0 } // BLER
              }
            },
            { 5504U, // SINR and BLER for CBS 5504
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 0.998077, 0.653699, 0.0436, 0, 0 } // BLER
              }
            },
            { 5632U, // SINR and BLER for CBS 5632
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 1, 0.935714, 0.342052, 0.0078, 0 } // BLER
              }
            },
            { 6912U, // SINR and BLER for CBS 6912
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 1, 0.939748, 0.260246, 0.0022, 0 } // BLER
              }
            },
            { 7680U, // SINR and BLER for CBS 7680
              NrEesmErrorModel::DoubleTuple{
                { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
                { 1, 0.832532, 0.0916257, 0, 0 } // BLER
              }
            }
        },
        { // MCS 16
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 17
            { 3752U, // SINR and BLER for CBS 3752
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 0.994231, 0.844771, 0.261719 } // BLER
              }
            },
            { 3840U, // SINR and BLER for CBS 3840
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 0.989423, 0.762649, 0.151419 } // BLER
              }
            },
            { 4096U, // SINR and BLER for CBS 4096
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 0.996154, 0.830357, 0.213771 } // BLER
              }
            },
            { 4480U, // SINR and BLER for CBS 4480
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 0.994231, 0.732955, 0.125749 } // BLER
              }
            },
            { 4864U, // SINR and BLER for CBS 4864
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 1, 0.832258, 0.182148 } // BLER
              }
            },
            { 5248U, // SINR and BLER for CBS 5248
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 0.996154, 0.770958, 0.0965306 } // BLER
              }
            },
            { 5504U, // SINR and BLER for CBS 5504
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 0.990385, 0.724306, 0.0749 } // BLER
              }
            },
            { 6016U, // SINR and BLER for CBS 6016
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 1, 0.938869, 0.353992 } // BLER
              }
            },
            { 6912U, // SINR and BLER for CBS 6912
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 1, 0.966418, 0.358404 } // BLER
              }
            },
            { 7680U, // SINR and BLER for CBS 7680
              NrEesmErrorModel::DoubleTuple{
                { 10.3822, 10.6883, 10.9944, 11.3005, 11.6066 }, // SINR
                { 1, 1, 1, 0.828226, 0.0935477 } // BLER
              }
            }
        },
        { // MCS 18
            { 3752U, // SINR and BLER for CBS 3752
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 0.997115, 0.927536, 0.464416, 0.0497 } // BLER
              }
            },
            { 3840U, // SINR and BLER for CBS 3840
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 0.995192, 0.845395, 0.283146, 0.0126 } // BLER
              }
            },
            { 4096U, // SINR and BLER for CBS 4096
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 1, 0.958955, 0.482008, 0.041 } // BLER
              }
            },
            { 4480U, // SINR and BLER for CBS 4480
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 1, 0.946168, 0.458786, 0.0242 } // BLER
              }
            },
            { 4864U, // SINR and BLER for CBS 4864
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 1, 0.958955, 0.426768, 0.0223 } // BLER
              }
            },
            { 5248U, // SINR and BLER for CBS 5248
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 1, 0.964286, 0.446743, 0.0196 } // BLER
              }
            },
            { 5504U, // SINR and BLER for CBS 5504
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 1, 0.951852, 0.395768, 0.0133 } // BLER
              }
            },
            { 6272U, // SINR and BLER for CBS 6272
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 1, 0.888889, 0.231618, 0.0036 } // BLER
              }
            },
            { 6400U, // SINR and BLER for CBS 6400
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 1, 0.996154, 0.710635, 0.0583 } // BLER
              }
            },
            { 7680U, // SINR and BLER for CBS 7680
              NrEesmErrorModel::DoubleTuple{
                { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
                { 1, 1, 0.984615, 0.482613, 0.0154 } // BLER
              }
            }
        }
      },
      { // BG TYPE 2
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
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 5
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 6
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 7
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 8
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 9
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 10
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 11
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 12
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 13
            { 24U, // SINR and BLER for CBS 24
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 9.1308, 10.7921, 12.4535, 14.1148 }, // SINR
                { 0.991346, 0.887931, 0.496569, 0.10002, 0.0053 } // BLER
              }
            },
            { 32U, // SINR and BLER for CBS 32
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 9.0689, 10.6683, 12.2678, 13.8672 }, // SINR
                { 0.956481, 0.680921, 0.207578, 0.0135, 0 } // BLER
              }
            },
            { 40U, // SINR and BLER for CBS 40
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 9.007, 10.5445, 12.0821, 13.6196 }, // SINR
                { 0.98187, 0.787348, 0.306402, 0.0266, 0.0003 } // BLER
              }
            },
            { 56U, // SINR and BLER for CBS 56
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.9451, 10.4207, 11.8964, 13.372 }, // SINR
                { 0.95709, 0.629926, 0.119189, 0.0039, 0.0001 } // BLER
              }
            },
            { 64U, // SINR and BLER for CBS 64
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.8832, 10.2969, 11.7107, 13.1244 }, // SINR
                { 0.978846, 0.734914, 0.199881, 0.0072, 0.0001 } // BLER
              }
            },
            { 80U, // SINR and BLER for CBS 80
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.8213, 10.1731, 11.525, 12.8768 }, // SINR
                { 0.963619, 0.609524, 0.0947218, 0.0024, 0 } // BLER
              }
            },
            { 88U, // SINR and BLER for CBS 88
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.7594, 10.0493, 11.3393, 12.6292 }, // SINR
                { 0.923561, 0.488942, 0.0507, 0.0004, 0 } // BLER
              }
            },
            { 96U, // SINR and BLER for CBS 96
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.6975, 9.9255, 11.1536, 12.3816 }, // SINR
                { 0.975191, 0.628713, 0.102056, 0.0028, 0 } // BLER
              }
            },
            { 112U, // SINR and BLER for CBS 112
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.6356, 9.8017, 10.9679, 12.134 }, // SINR
                { 0.95463, 0.526639, 0.0687, 0.0012, 0 } // BLER
              }
            },
            { 120U, // SINR and BLER for CBS 120
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.5737, 9.6779, 10.7822, 11.8864 }, // SINR
                { 0.963619, 0.537975, 0.0407, 0.0002, 0 } // BLER
              }
            },
            { 192U, // SINR and BLER for CBS 192
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.5118, 9.5541, 10.5965, 11.6388 }, // SINR
                { 0.984615, 0.68516, 0.101504, 0.001, 0 } // BLER
              }
            },
            { 208U, // SINR and BLER for CBS 208
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.4499, 9.4303, 10.4108, 11.3912 }, // SINR
                { 0.964962, 0.509577, 0.0401, 0.0002, 0 } // BLER
              }
            },
            { 224U, // SINR and BLER for CBS 224
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.388, 9.3065, 10.2251, 11.1436 }, // SINR
                { 0.981731, 0.657861, 0.0998759, 0.0009, 0 } // BLER
              }
            },
            { 240U, // SINR and BLER for CBS 240
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.2642, 9.0589, 9.8537, 10.6484 }, // SINR
                { 0.967105, 0.63625, 0.122096, 0.0039, 0 } // BLER
              }
            },
            { 272U, // SINR and BLER for CBS 272
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.963619, 0.673429, 0.173276, 0.0085, 0.0001 } // BLER
              }
            },
            { 304U, // SINR and BLER for CBS 304
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.98187, 0.816083, 0.288813, 0.0226, 0.0003 } // BLER
              }
            },
            { 336U, // SINR and BLER for CBS 336
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.988462, 0.739943, 0.203479, 0.0104, 0.0001 } // BLER
              }
            },
            { 368U, // SINR and BLER for CBS 368
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.958022, 0.645101, 0.118638, 0.0027, 0 } // BLER
              }
            },
            { 384U, // SINR and BLER for CBS 384
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.973282, 0.681516, 0.114745, 0.0019, 0.0001 } // BLER
              }
            },
            { 432U, // SINR and BLER for CBS 432
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.978053, 0.685829, 0.108014, 0.0027, 0 } // BLER
              }
            },
            { 456U, // SINR and BLER for CBS 456
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.984615, 0.694595, 0.0838341, 0.0012, 0.0001 } // BLER
              }
            },
            { 552U, // SINR and BLER for CBS 552
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.992308, 0.697011, 0.0575, 0, 0 } // BLER
              }
            },
            { 704U, // SINR and BLER for CBS 704
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.9875, 0.609597, 0.0323, 0.0002, 0 } // BLER
              }
            },
            { 768U, // SINR and BLER for CBS 768
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.994231, 0.616029, 0.0207, 0, 0 } // BLER
              }
            },
            { 848U, // SINR and BLER for CBS 848
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.990385, 0.590023, 0.0151, 0, 0 } // BLER
              }
            },
            { 928U, // SINR and BLER for CBS 928
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.992308, 0.549893, 0.0103, 0, 0 } // BLER
              }
            },
            { 984U, // SINR and BLER for CBS 984
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.991346, 0.544528, 0.0071, 0, 0 } // BLER
              }
            },
            { 1064U, // SINR and BLER for CBS 1064
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.997115, 0.576484, 0.0069, 0, 0 } // BLER
              }
            },
            { 1160U, // SINR and BLER for CBS 1160
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 0.996154, 0.469907, 0.0025, 0, 0 } // BLER
              }
            },
            { 1256U, // SINR and BLER for CBS 1256
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.554348, 0.0024, 0.0001, 0 } // BLER
              }
            },
            { 1416U, // SINR and BLER for CBS 1416
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.727143, 0.0073, 0, 0 } // BLER
              }
            },
            { 1544U, // SINR and BLER for CBS 1544
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.883562, 0.0356, 0.0001, 0 } // BLER
              }
            },
            { 1736U, // SINR and BLER for CBS 1736
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.809375, 0.0181, 0.0001, 0 } // BLER
              }
            },
            { 1864U, // SINR and BLER for CBS 1864
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.91521, 0.0326, 0.0001, 0 } // BLER
              }
            },
            { 2024U, // SINR and BLER for CBS 2024
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.722765, 0.0027, 0, 0 } // BLER
              }
            },
            { 2216U, // SINR and BLER for CBS 2216
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.850993, 0.0078, 0.0001, 0 } // BLER
              }
            },
            { 2280U, // SINR and BLER for CBS 2280
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.75, 0.0016, 0.0001, 0 } // BLER
              }
            },
            { 2536U, // SINR and BLER for CBS 2536
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.825806, 0.0022, 0, 0 } // BLER
              }
            },
            { 2856U, // SINR and BLER for CBS 2856
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.698087, 0.0009, 0, 0 } // BLER
              }
            },
            { 3104U, // SINR and BLER for CBS 3104
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.789352, 0.0005, 0.0001, 0 } // BLER
              }
            },
            { 3496U, // SINR and BLER for CBS 3496
              NrEesmErrorModel::DoubleTuple{
                { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
                { 1, 0.811709, 0.0009, 0, 0 } // BLER
              }
            }
        },
        { // MCS 14
            { 24U, // SINR and BLER for CBS 24
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 10.4596, 12.1827, 13.9059, 15.629 }, // SINR
                { 0.934353, 0.585616, 0.141827, 0.0094, 0.0001 } // BLER
              }
            },
            { 32U, // SINR and BLER for CBS 32
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 10.3668, 11.997, 13.6273, 15.2576 }, // SINR
                { 0.952206, 0.645202, 0.158417, 0.0058, 0 } // BLER
              }
            },
            { 48U, // SINR and BLER for CBS 48
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 10.2739, 11.8113, 13.3488, 14.8862 }, // SINR
                { 0.963619, 0.683155, 0.179255, 0.0098, 0.0001 } // BLER
              }
            },
            { 64U, // SINR and BLER for CBS 64
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 10.243, 11.7494, 13.2559, 14.7624 }, // SINR
                { 0.894965, 0.429111, 0.0481, 0.0006, 0 } // BLER
              }
            },
            { 72U, // SINR and BLER for CBS 72
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 10.1501, 11.5637, 12.9774, 14.391 }, // SINR
                { 0.942029, 0.525826, 0.0687, 0.0008, 0 } // BLER
              }
            },
            { 88U, // SINR and BLER for CBS 88
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 10.0882, 11.4399, 12.7917, 14.1434 }, // SINR
                { 0.8625, 0.347796, 0.0226, 0.0001, 0 } // BLER
              }
            },
            { 96U, // SINR and BLER for CBS 96
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 10.0263, 11.3161, 12.606, 13.8958 }, // SINR
                { 0.908854, 0.44788, 0.0442, 0.0002, 0 } // BLER
              }
            },
            { 112U, // SINR and BLER for CBS 112
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.9644, 11.1923, 12.4203, 13.6482 }, // SINR
                { 0.95073, 0.530083, 0.073, 0.0017, 0 } // BLER
              }
            },
            { 128U, // SINR and BLER for CBS 128
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.9025, 11.0685, 12.2346, 13.4006 }, // SINR
                { 0.921429, 0.382576, 0.0238, 0.0002, 0 } // BLER
              }
            },
            { 192U, // SINR and BLER for CBS 192
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.8406, 10.9447, 12.0489, 13.153 }, // SINR
                { 0.929348, 0.475187, 0.0379, 0.0007, 0 } // BLER
              }
            },
            { 208U, // SINR and BLER for CBS 208
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.7787, 10.8209, 11.8632, 12.9054 }, // SINR
                { 0.961466, 0.523148, 0.0547, 0.001, 0 } // BLER
              }
            },
            { 224U, // SINR and BLER for CBS 224
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.7168, 10.6971, 11.6775, 12.6578 }, // SINR
                { 0.880102, 0.346653, 0.0236, 0.0001, 0 } // BLER
              }
            },
            { 240U, // SINR and BLER for CBS 240
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.593, 10.4495, 11.3061, 12.1626 }, // SINR
                { 0.940647, 0.516194, 0.0812, 0.0022, 0 } // BLER
              }
            },
            { 272U, // SINR and BLER for CBS 272
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.903169, 0.428872, 0.0676, 0.0019, 0 } // BLER
              }
            },
            { 304U, // SINR and BLER for CBS 304
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.934353, 0.541843, 0.0953252, 0.0033, 0.0001 } // BLER
              }
            },
            { 336U, // SINR and BLER for CBS 336
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.880952, 0.381401, 0.035, 0.0008, 0 } // BLER
              }
            },
            { 368U, // SINR and BLER for CBS 368
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.89569, 0.396552, 0.0315, 0.0002, 0 } // BLER
              }
            },
            { 384U, // SINR and BLER for CBS 384
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.955224, 0.567522, 0.0705, 0.0015, 0 } // BLER
              }
            },
            { 432U, // SINR and BLER for CBS 432
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.949818, 0.496078, 0.0488, 0.0006, 0 } // BLER
              }
            },
            { 456U, // SINR and BLER for CBS 456
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.905594, 0.280971, 0.0086, 0, 0 } // BLER
              }
            },
            { 552U, // SINR and BLER for CBS 552
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.940693, 0.328571, 0.0064, 0, 0 } // BLER
              }
            },
            { 704U, // SINR and BLER for CBS 704
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.924107, 0.278761, 0.0028, 0, 0 } // BLER
              }
            },
            { 768U, // SINR and BLER for CBS 768
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.898768, 0.139771, 0.0002, 0, 0 } // BLER
              }
            },
            { 848U, // SINR and BLER for CBS 848
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.903169, 0.14627, 0.0009, 0, 0 } // BLER
              }
            },
            { 928U, // SINR and BLER for CBS 928
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.915493, 0.15974, 0.0002, 0, 0 } // BLER
              }
            },
            { 984U, // SINR and BLER for CBS 984
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.932482, 0.157425, 0.0001, 0, 0 } // BLER
              }
            },
            { 1064U, // SINR and BLER for CBS 1064
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.903169, 0.0881, 0.0003, 0.0001, 0 } // BLER
              }
            },
            { 1160U, // SINR and BLER for CBS 1160
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.934353, 0.0965538, 0, 0, 0 } // BLER
              }
            },
            { 1256U, // SINR and BLER for CBS 1256
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.951287, 0.0871671, 0, 0, 0 } // BLER
              }
            },
            { 1416U, // SINR and BLER for CBS 1416
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.943015, 0.0728, 0, 0, 0 } // BLER
              }
            },
            { 1544U, // SINR and BLER for CBS 1544
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.980769, 0.158375, 0, 0, 0 } // BLER
              }
            },
            { 1736U, // SINR and BLER for CBS 1736
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.984615, 0.205212, 0, 0, 0 } // BLER
              }
            },
            { 1864U, // SINR and BLER for CBS 1864
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.988462, 0.205606, 0.0001, 0, 0 } // BLER
              }
            },
            { 2024U, // SINR and BLER for CBS 2024
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.997115, 0.180576, 0.0001, 0, 0 } // BLER
              }
            },
            { 2216U, // SINR and BLER for CBS 2216
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.9875, 0.132105, 0, 0, 0 } // BLER
              }
            },
            { 2280U, // SINR and BLER for CBS 2280
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.995192, 0.180755, 0.0002, 0, 0 } // BLER
              }
            },
            { 2536U, // SINR and BLER for CBS 2536
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.985577, 0.0654, 0, 0, 0 } // BLER
              }
            },
            { 2856U, // SINR and BLER for CBS 2856
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.997115, 0.0892428, 0, 0, 0 } // BLER
              }
            },
            { 3104U, // SINR and BLER for CBS 3104
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 1, 0.0842, 0, 0, 0 } // BLER
              }
            },
            { 3496U, // SINR and BLER for CBS 3496
              NrEesmErrorModel::DoubleTuple{
                { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
                { 0.997115, 0.0462, 0, 0, 0 } // BLER
              }
            }
        },
        { // MCS 15
            { 24U, // SINR and BLER for CBS 24
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.7266, 13.4496, 15.1726, 16.8956 }, // SINR
                { 0.968985, 0.68078, 0.209799, 0.017, 0.0006 } // BLER
              }
            },
            { 40U, // SINR and BLER for CBS 40
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.6647, 13.3258, 14.9869, 16.648 }, // SINR
                { 0.949818, 0.58871, 0.130469, 0.0051, 0 } // BLER
              }
            },
            { 56U, // SINR and BLER for CBS 56
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.5718, 13.1401, 14.7083, 16.2766 }, // SINR
                { 0.944444, 0.563326, 0.0968929, 0.0026, 0 } // BLER
              }
            },
            { 72U, // SINR and BLER for CBS 72
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.5409, 13.0782, 14.6155, 16.1528 }, // SINR
                { 0.809335, 0.246807, 0.0109, 0, 0 } // BLER
              }
            },
            { 80U, // SINR and BLER for CBS 80
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.4171, 12.8306, 14.2441, 15.6576 }, // SINR
                { 0.839286, 0.303442, 0.0245, 0.0006, 0 } // BLER
              }
            },
            { 96U, // SINR and BLER for CBS 96
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.3552, 12.7068, 14.0584, 15.41 }, // SINR
                { 0.842715, 0.303357, 0.0188, 0, 0 } // BLER
              }
            },
            { 112U, // SINR and BLER for CBS 112
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.2933, 12.583, 13.8727, 15.1624 }, // SINR
                { 0.871622, 0.313433, 0.0215, 0.0003, 0 } // BLER
              }
            },
            { 128U, // SINR and BLER for CBS 128
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.2314, 12.4592, 13.687, 14.9148 }, // SINR
                { 0.795525, 0.17389, 0.003, 0, 0 } // BLER
              }
            },
            { 184U, // SINR and BLER for CBS 184
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.1695, 12.3354, 13.5013, 14.6672 }, // SINR
                { 0.925893, 0.414062, 0.0228, 0, 0 } // BLER
              }
            },
            { 208U, // SINR and BLER for CBS 208
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.1076, 12.2116, 13.3156, 14.4196 }, // SINR
                { 0.669503, 0.10095, 0.0014, 0, 0 } // BLER
              }
            },
            { 224U, // SINR and BLER for CBS 224
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 11.0457, 12.0878, 13.1299, 14.172 }, // SINR
                { 0.703804, 0.138674, 0.0023, 0, 0 } // BLER
              }
            },
            { 240U, // SINR and BLER for CBS 240
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.9219, 11.8402, 12.7585, 13.6768 }, // SINR
                { 0.776198, 0.190038, 0.0076, 0.0001, 0 } // BLER
              }
            },
            { 272U, // SINR and BLER for CBS 272
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.787577, 0.270923, 0.0228, 0.0006, 0.0001 } // BLER
              }
            },
            { 304U, // SINR and BLER for CBS 304
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.810127, 0.291763, 0.0182, 0.0002, 0 } // BLER
              }
            },
            { 336U, // SINR and BLER for CBS 336
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.611244, 0.0973344, 0.0014, 0, 0 } // BLER
              }
            },
            { 368U, // SINR and BLER for CBS 368
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.640783, 0.108619, 0.0025, 0.0001, 0 } // BLER
              }
            },
            { 384U, // SINR and BLER for CBS 384
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.708799, 0.125874, 0.0023, 0.0001, 0 } // BLER
              }
            },
            { 432U, // SINR and BLER for CBS 432
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.616587, 0.0765, 0.0008, 0, 0 } // BLER
              }
            },
            { 456U, // SINR and BLER for CBS 456
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.730603, 0.0902, 0.0009, 0.0001, 0 } // BLER
              }
            },
            { 552U, // SINR and BLER for CBS 552
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.623786, 0.0415, 0.0001, 0, 0 } // BLER
              }
            },
            { 704U, // SINR and BLER for CBS 704
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.69877, 0.0504, 0.0001, 0, 0 } // BLER
              }
            },
            { 768U, // SINR and BLER for CBS 768
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.628094, 0.0213, 0, 0, 0 } // BLER
              }
            },
            { 848U, // SINR and BLER for CBS 848
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.721208, 0.0286, 0, 0, 0 } // BLER
              }
            },
            { 928U, // SINR and BLER for CBS 928
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.678476, 0.0173, 0, 0, 0 } // BLER
              }
            },
            { 984U, // SINR and BLER for CBS 984
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.576233, 0.0072, 0, 0, 0 } // BLER
              }
            },
            { 1064U, // SINR and BLER for CBS 1064
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.65051, 0.0111, 0.0001, 0, 0 } // BLER
              }
            },
            { 1160U, // SINR and BLER for CBS 1160
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.598592, 0.005, 0, 0, 0 } // BLER
              }
            },
            { 1256U, // SINR and BLER for CBS 1256
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.575568, 0.0023, 0.0001, 0.0001, 0 } // BLER
              }
            },
            { 1416U, // SINR and BLER for CBS 1416
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.561681, 0.0016, 0, 0, 0 } // BLER
              }
            },
            { 1544U, // SINR and BLER for CBS 1544
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.588542, 0.0009, 0, 0, 0 } // BLER
              }
            },
            { 1736U, // SINR and BLER for CBS 1736
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.558114, 0.0007, 0, 0, 0 } // BLER
              }
            },
            { 1864U, // SINR and BLER for CBS 1864
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.689516, 0.0009, 0, 0, 0 } // BLER
              }
            },
            { 1928U, // SINR and BLER for CBS 1928
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.846854, 0.0093, 0, 0, 0 } // BLER
              }
            },
            { 2216U, // SINR and BLER for CBS 2216
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.744186, 0.0022, 0, 0, 0 } // BLER
              }
            },
            { 2280U, // SINR and BLER for CBS 2280
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.721045, 0.0017, 0, 0, 0 } // BLER
              }
            },
            { 2536U, // SINR and BLER for CBS 2536
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.814082, 0.0026, 0.0001, 0, 0 } // BLER
              }
            },
            { 2856U, // SINR and BLER for CBS 2856
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.755178, 0.0011, 0, 0, 0 } // BLER
              }
            },
            { 3104U, // SINR and BLER for CBS 3104
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.795312, 0.0006, 0, 0, 0 } // BLER
              }
            },
            { 3496U, // SINR and BLER for CBS 3496
              NrEesmErrorModel::DoubleTuple{
                { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
                { 0.760294, 0.0005, 0, 0, 0 } // BLER
              }
            }
        },
        { // MCS 16
          { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
        },
        { // MCS 17
            { 24U, // SINR and BLER for CBS 24
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 12.5911, 14.3543, 16.1175, 17.8807 }, // SINR
                { 0.945255, 0.614183, 0.167667, 0.0078, 0.0002 } // BLER
              }
            },
            { 40U, // SINR and BLER for CBS 40
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 12.5292, 14.2305, 15.9318, 17.6331 }, // SINR
                { 0.927536, 0.572309, 0.0969934, 0.0022, 0 } // BLER
              }
            },
            { 56U, // SINR and BLER for CBS 56
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 12.4363, 14.0448, 15.6532, 17.2617 }, // SINR
                { 0.914894, 0.501476, 0.07, 0.0014, 0 } // BLER
              }
            },
            { 72U, // SINR and BLER for CBS 72
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 12.3435, 13.8591, 15.3747, 16.8903 }, // SINR
                { 0.938869, 0.502941, 0.0595, 0.0007, 0 } // BLER
              }
            },
            { 88U, // SINR and BLER for CBS 88
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 12.2816, 13.7353, 15.189, 16.6427 }, // SINR
                { 0.934783, 0.522358, 0.0586, 0.0006, 0 } // BLER
              }
            },
            { 104U, // SINR and BLER for CBS 104
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 12.1578, 13.4877, 14.8176, 16.1475 }, // SINR
                { 0.940693, 0.549569, 0.0713, 0.0013, 0 } // BLER
              }
            },
            { 120U, // SINR and BLER for CBS 120
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 12.0959, 13.3639, 14.6319, 15.8999 }, // SINR
                { 0.951642, 0.492218, 0.0341, 0.0002, 0 } // BLER
              }
            },
            { 184U, // SINR and BLER for CBS 184
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 12.034, 13.2401, 14.4462, 15.6523 }, // SINR
                { 0.891379, 0.317043, 0.012, 0, 0 } // BLER
              }
            },
            { 208U, // SINR and BLER for CBS 208
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.9721, 13.1163, 14.2605, 15.4047 }, // SINR
                { 0.902289, 0.33857, 0.0148, 0.0002, 0 } // BLER
              }
            },
            { 224U, // SINR and BLER for CBS 224
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.9102, 12.9925, 14.0748, 15.1571 }, // SINR
                { 0.919326, 0.362428, 0.0203, 0, 0 } // BLER
              }
            },
            { 240U, // SINR and BLER for CBS 240
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.7864, 12.7449, 13.7034, 14.6619 }, // SINR
                { 0.90581, 0.436638, 0.0409, 0.0006, 0 } // BLER
              }
            },
            { 272U, // SINR and BLER for CBS 272
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.938406, 0.543803, 0.0952332, 0.0037, 0 } // BLER
              }
            },
            { 304U, // SINR and BLER for CBS 304
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.943015, 0.500494, 0.069, 0.0018, 0 } // BLER
              }
            },
            { 336U, // SINR and BLER for CBS 336
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.94708, 0.508, 0.0584, 0.0008, 0 } // BLER
              }
            },
            { 368U, // SINR and BLER for CBS 368
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.935688, 0.47026, 0.0474, 0.0004, 0 } // BLER
              }
            },
            { 384U, // SINR and BLER for CBS 384
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.958022, 0.519388, 0.0455, 0.0003, 0.0001 } // BLER
              }
            },
            { 432U, // SINR and BLER for CBS 432
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.916964, 0.35933, 0.0166, 0, 0 } // BLER
              }
            },
            { 456U, // SINR and BLER for CBS 456
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.926786, 0.32455, 0.0065, 0.0002, 0 } // BLER
              }
            },
            { 552U, // SINR and BLER for CBS 552
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.946558, 0.285877, 0.0035, 0, 0 } // BLER
              }
            },
            { 704U, // SINR and BLER for CBS 704
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.943934, 0.267932, 0.0012, 0, 0 } // BLER
              }
            },
            { 768U, // SINR and BLER for CBS 768
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.956204, 0.278761, 0.0002, 0, 0 } // BLER
              }
            },
            { 848U, // SINR and BLER for CBS 848
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.966856, 0.271727, 0.0011, 0, 0 } // BLER
              }
            },
            { 928U, // SINR and BLER for CBS 928
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.958022, 0.207166, 0.0006, 0, 0 } // BLER
              }
            },
            { 984U, // SINR and BLER for CBS 984
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.959586, 0.198425, 0.0005, 0, 0 } // BLER
              }
            },
            { 1064U, // SINR and BLER for CBS 1064
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.970644, 0.207237, 0.0004, 0, 0 } // BLER
              }
            },
            { 1160U, // SINR and BLER for CBS 1160
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.958333, 0.133253, 0.0001, 0, 0 } // BLER
              }
            },
            { 1256U, // SINR and BLER for CBS 1256
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.967105, 0.13254, 0.0001, 0, 0 } // BLER
              }
            },
            { 1416U, // SINR and BLER for CBS 1416
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.980916, 0.133077, 0.0001, 0, 0 } // BLER
              }
            },
            { 1544U, // SINR and BLER for CBS 1544
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.969697, 0.0837, 0, 0, 0 } // BLER
              }
            },
            { 1736U, // SINR and BLER for CBS 1736
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.982008, 0.0786, 0, 0, 0 } // BLER
              }
            },
            { 1864U, // SINR and BLER for CBS 1864
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.990385, 0.0842348, 0, 0, 0 } // BLER
              }
            },
            { 2024U, // SINR and BLER for CBS 2024
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.999038, 0.253758, 0.0004, 0, 0 } // BLER
              }
            },
            { 2088U, // SINR and BLER for CBS 2088
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.998077, 0.321203, 0.0001, 0, 0 } // BLER
              }
            },
            { 2280U, // SINR and BLER for CBS 2280
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 0.996154, 0.212924, 0.0001, 0, 0 } // BLER
              }
            },
            { 2536U, // SINR and BLER for CBS 2536
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 1, 0.188156, 0, 0, 0 } // BLER
              }
            },
            { 2856U, // SINR and BLER for CBS 2856
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 1, 0.223451, 0.0001, 0, 0 } // BLER
              }
            },
            { 3104U, // SINR and BLER for CBS 3104
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 1, 0.134801, 0, 0, 0 } // BLER
              }
            },
            { 3496U, // SINR and BLER for CBS 3496
              NrEesmErrorModel::DoubleTuple{
                { 10.8279, 11.6626, 12.4973, 13.332, 14.1667 }, // SINR
                { 1, 0.178876, 0, 0, 0 } // BLER
              }
            }
        },
        { // MCS 18
            { 32U, // SINR and BLER for CBS 32
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 13.4462, 15.2196, 16.9931, 18.7665 }, // SINR
                { 0.971591, 0.777273, 0.291282, 0.0226, 0.0006 } // BLER
              }
            },
            { 48U, // SINR and BLER for CBS 48
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 13.3533, 15.0339, 16.7145, 18.3951 }, // SINR
                { 0.932065, 0.633578, 0.136697, 0.0032, 0.0002 } // BLER
              }
            },
            { 64U, // SINR and BLER for CBS 64
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 13.2914, 14.9101, 16.5288, 18.1475 }, // SINR
                { 0.934353, 0.532038, 0.0663, 0.0006, 0 } // BLER
              }
            },
            { 80U, // SINR and BLER for CBS 80
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 13.1676, 14.6625, 16.1574, 17.6523 }, // SINR
                { 0.919065, 0.467831, 0.0576, 0.0005, 0.0001 } // BLER
              }
            },
            { 96U, // SINR and BLER for CBS 96
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 13.1057, 14.5387, 15.9717, 17.4047 }, // SINR
                { 0.90493, 0.434708, 0.0346, 0.0001, 0 } // BLER
              }
            },
            { 112U, // SINR and BLER for CBS 112
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 13.0438, 14.4149, 15.786, 17.1571 }, // SINR
                { 0.90035, 0.380255, 0.0261, 0.0001, 0 } // BLER
              }
            },
            { 128U, // SINR and BLER for CBS 128
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.9819, 14.2911, 15.6003, 16.9095 }, // SINR
                { 0.844771, 0.213497, 0.0042, 0, 0 } // BLER
              }
            },
            { 192U, // SINR and BLER for CBS 192
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.92, 14.1673, 15.4146, 16.6619 }, // SINR
                { 0.838474, 0.214346, 0.0073, 0.0002, 0 } // BLER
              }
            },
            { 208U, // SINR and BLER for CBS 208
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.8581, 14.0435, 15.2289, 16.4143 }, // SINR
                { 0.824519, 0.204992, 0.0048, 0, 0 } // BLER
              }
            },
            { 224U, // SINR and BLER for CBS 224
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.7962, 13.9197, 15.0432, 16.1667 }, // SINR
                { 0.814873, 0.217128, 0.0049, 0, 0 } // BLER
              }
            },
            { 240U, // SINR and BLER for CBS 240
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.6724, 13.6721, 14.6718, 15.6715 }, // SINR
                { 0.786043, 0.216595, 0.0094, 0.0001, 0 } // BLER
              }
            },
            { 272U, // SINR and BLER for CBS 272
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.8, 0.243002, 0.0139, 0.0001, 0 } // BLER
              }
            },
            { 304U, // SINR and BLER for CBS 304
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.764137, 0.205882, 0.0083, 0.0002, 0.0001 } // BLER
              }
            },
            { 320U, // SINR and BLER for CBS 320
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.908929, 0.411645, 0.0298, 0.0002, 0 } // BLER
              }
            },
            { 368U, // SINR and BLER for CBS 368
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.909574, 0.386742, 0.0205, 0.0002, 0 } // BLER
              }
            },
            { 384U, // SINR and BLER for CBS 384
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.902972, 0.307598, 0.012, 0.0002, 0 } // BLER
              }
            },
            { 432U, // SINR and BLER for CBS 432
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.820411, 0.187128, 0.0037, 0.0001, 0 } // BLER
              }
            },
            { 456U, // SINR and BLER for CBS 456
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.794753, 0.0965021, 0.0006, 0, 0 } // BLER
              }
            },
            { 552U, // SINR and BLER for CBS 552
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.807031, 0.0809, 0.0001, 0, 0 } // BLER
              }
            },
            { 704U, // SINR and BLER for CBS 704
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.783537, 0.0556, 0, 0, 0 } // BLER
              }
            },
            { 768U, // SINR and BLER for CBS 768
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.755117, 0.0339, 0.0001, 0, 0 } // BLER
              }
            },
            { 848U, // SINR and BLER for CBS 848
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.746324, 0.0259, 0.0001, 0, 0 } // BLER
              }
            },
            { 928U, // SINR and BLER for CBS 928
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.784299, 0.0256, 0, 0, 0 } // BLER
              }
            },
            { 984U, // SINR and BLER for CBS 984
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.764222, 0.0199, 0, 0, 0 } // BLER
              }
            },
            { 1064U, // SINR and BLER for CBS 1064
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.779545, 0.0152, 0, 0, 0 } // BLER
              }
            },
            { 1160U, // SINR and BLER for CBS 1160
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.740607, 0.0096, 0, 0, 0 } // BLER
              }
            },
            { 1256U, // SINR and BLER for CBS 1256
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.802673, 0.0104, 0, 0, 0 } // BLER
              }
            },
            { 1416U, // SINR and BLER for CBS 1416
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.811306, 0.0064, 0, 0, 0 } // BLER
              }
            },
            { 1544U, // SINR and BLER for CBS 1544
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.771837, 0.0026, 0.0001, 0.0001, 0 } // BLER
              }
            },
            { 1736U, // SINR and BLER for CBS 1736
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.744913, 0.0015, 0, 0, 0 } // BLER
              }
            },
            { 1864U, // SINR and BLER for CBS 1864
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.717179, 0.0002, 0, 0, 0 } // BLER
              }
            },
            { 2024U, // SINR and BLER for CBS 2024
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.696721, 0.0006, 0, 0, 0 } // BLER
              }
            },
            { 2152U, // SINR and BLER for CBS 2152
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.964286, 0.0257, 0, 0, 0 } // BLER
              }
            },
            { 2280U, // SINR and BLER for CBS 2280
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.880068, 0.0052, 0, 0, 0 } // BLER
              }
            },
            { 2536U, // SINR and BLER for CBS 2536
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.94708, 0.0094, 0, 0, 0 } // BLER
              }
            },
            { 2856U, // SINR and BLER for CBS 2856
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.956801, 0.0056, 0, 0, 0 } // BLER
              }
            },
            { 3104U, // SINR and BLER for CBS 3104
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.964962, 0.0036, 0, 0, 0 } // BLER
              }
            },
            { 3496U, // SINR and BLER for CBS 3496
              NrEesmErrorModel::DoubleTuple{
                { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
                { 0.967803, 0.0038, 0, 0, 0 } // BLER
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
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 3
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 4
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 5
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 6
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 7
        { 3752U, // SINR and BLER for CBS 3752
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 0.999038, 0.914007, 0.324289, 0.0077 } // BLER
          }
        },
        { 3840U, // SINR and BLER for CBS 3840
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.90493, 0.257398, 0.0044 } // BLER
          }
        },
        { 4096U, // SINR and BLER for CBS 4096
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.933036, 0.278846, 0.0044 } // BLER
          }
        },
        { 4480U, // SINR and BLER for CBS 4480
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.960185, 0.372411, 0.0095 } // BLER
          }
        },
        { 4864U, // SINR and BLER for CBS 4864
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.988462, 0.500977, 0.0183 } // BLER
          }
        },
        { 5248U, // SINR and BLER for CBS 5248
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.967557, 0.408065, 0.0068 } // BLER
          }
        },
        { 5504U, // SINR and BLER for CBS 5504
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.985577, 0.476124, 0.0101 } // BLER
          }
        },
        { 6272U, // SINR and BLER for CBS 6272
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.993269, 0.493269, 0.008 } // BLER
          }
        },
        { 6912U, // SINR and BLER for CBS 6912
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.995192, 0.53692, 0.0083 } // BLER
          }
        },
        { 7552U, // SINR and BLER for CBS 7552
          NrEesmErrorModel::DoubleTuple{
            { 7.4068, 7.7258, 8.0447, 8.3637, 8.6826 }, // SINR
            { 1, 1, 0.993269, 0.41299, 0.005 } // BLER
          }
        }
    },
    { // MCS 8
        { 3752U, // SINR and BLER for CBS 3752
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.964962, 0.513609, 0.0357, 0.0001 } // BLER
          }
        },
        { 3840U, // SINR and BLER for CBS 3840
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.961174, 0.455197, 0.0222, 0.0001 } // BLER
          }
        },
        { 4096U, // SINR and BLER for CBS 4096
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.974038, 0.445423, 0.0186, 0.0001 } // BLER
          }
        },
        { 4480U, // SINR and BLER for CBS 4480
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.979008, 0.42839, 0.0142, 0.0001 } // BLER
          }
        },
        { 4864U, // SINR and BLER for CBS 4864
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.989423, 0.683824, 0.0466, 0.0002 } // BLER
          }
        },
        { 5120U, // SINR and BLER for CBS 5120
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.985577, 0.572072, 0.0341, 0.0001 } // BLER
          }
        },
        { 5504U, // SINR and BLER for CBS 5504
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.992308, 0.581422, 0.024, 0.0001 } // BLER
          }
        },
        { 6272U, // SINR and BLER for CBS 6272
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.992308, 0.543269, 0.0161, 0.0001 } // BLER
          }
        },
        { 6912U, // SINR and BLER for CBS 6912
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.997115, 0.706704, 0.0358, 0 } // BLER
          }
        },
        { 7680U, // SINR and BLER for CBS 7680
          NrEesmErrorModel::DoubleTuple{
            { 8.5294, 8.8435, 9.1576, 9.4717, 9.7858 }, // SINR
            { 1, 0.993269, 0.572635, 0.0134, 0 } // BLER
          }
        }
    },
    { // MCS 9
        { 3752U, // SINR and BLER for CBS 3752
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 0.995192, 0.879281, 0.335106, 0.0214, 0.0001 } // BLER
          }
        },
        { 3840U, // SINR and BLER for CBS 3840
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 0.992308, 0.715084, 0.125, 0.0017, 0 } // BLER
          }
        },
        { 4096U, // SINR and BLER for CBS 4096
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 0.995192, 0.741228, 0.111877, 0.0013, 0 } // BLER
          }
        },
        { 4480U, // SINR and BLER for CBS 4480
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 0.997115, 0.733382, 0.108188, 0.0006, 0 } // BLER
          }
        },
        { 4864U, // SINR and BLER for CBS 4864
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 0.994231, 0.764137, 0.0990313, 0.0006, 0.0001 } // BLER
          }
        },
        { 5248U, // SINR and BLER for CBS 5248
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 0.998077, 0.709722, 0.0716, 0.0002, 0 } // BLER
          }
        },
        { 5504U, // SINR and BLER for CBS 5504
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 0.998077, 0.653699, 0.0436, 0, 0 } // BLER
          }
        },
        { 5632U, // SINR and BLER for CBS 5632
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 1, 0.935714, 0.342052, 0.0078, 0 } // BLER
          }
        },
        { 6912U, // SINR and BLER for CBS 6912
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 1, 0.939748, 0.260246, 0.0022, 0 } // BLER
          }
        },
        { 7680U, // SINR and BLER for CBS 7680
          NrEesmErrorModel::DoubleTuple{
            { 9.6519, 9.9612, 10.2704, 10.5797, 10.8889 }, // SINR
            { 1, 0.832532, 0.0916257, 0, 0 } // BLER
          }
        }
    },
    { // MCS 10
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 11
        { 3752U, // SINR and BLER for CBS 3752
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 0.997115, 0.927536, 0.464416, 0.0497 } // BLER
          }
        },
        { 3840U, // SINR and BLER for CBS 3840
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 0.995192, 0.845395, 0.283146, 0.0126 } // BLER
          }
        },
        { 4096U, // SINR and BLER for CBS 4096
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 1, 0.958955, 0.482008, 0.041 } // BLER
          }
        },
        { 4480U, // SINR and BLER for CBS 4480
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 1, 0.946168, 0.458786, 0.0242 } // BLER
          }
        },
        { 4864U, // SINR and BLER for CBS 4864
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 1, 0.958955, 0.426768, 0.0223 } // BLER
          }
        },
        { 5248U, // SINR and BLER for CBS 5248
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 1, 0.964286, 0.446743, 0.0196 } // BLER
          }
        },
        { 5504U, // SINR and BLER for CBS 5504
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 1, 0.951852, 0.395768, 0.0133 } // BLER
          }
        },
        { 6272U, // SINR and BLER for CBS 6272
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 1, 0.888889, 0.231618, 0.0036 } // BLER
          }
        },
        { 6400U, // SINR and BLER for CBS 6400
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 1, 0.996154, 0.710635, 0.0583 } // BLER
          }
        },
        { 7680U, // SINR and BLER for CBS 7680
          NrEesmErrorModel::DoubleTuple{
            { 11.1307, 11.4335, 11.7364, 12.0393, 12.3422 }, // SINR
            { 1, 1, 0.984615, 0.482613, 0.0154 } // BLER
          }
        }
    }
  },
  { // BG TYPE 2
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
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 5
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 6
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 7
        { 24U, // SINR and BLER for CBS 24
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 9.1308, 10.7921, 12.4535, 14.1148 }, // SINR
            { 0.991346, 0.887931, 0.496569, 0.10002, 0.0053 } // BLER
          }
        },
        { 32U, // SINR and BLER for CBS 32
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 9.0689, 10.6683, 12.2678, 13.8672 }, // SINR
            { 0.956481, 0.680921, 0.207578, 0.0135, 0 } // BLER
          }
        },
        { 40U, // SINR and BLER for CBS 40
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 9.007, 10.5445, 12.0821, 13.6196 }, // SINR
            { 0.98187, 0.787348, 0.306402, 0.0266, 0.0003 } // BLER
          }
        },
        { 56U, // SINR and BLER for CBS 56
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.9451, 10.4207, 11.8964, 13.372 }, // SINR
            { 0.95709, 0.629926, 0.119189, 0.0039, 0.0001 } // BLER
          }
        },
        { 64U, // SINR and BLER for CBS 64
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.8832, 10.2969, 11.7107, 13.1244 }, // SINR
            { 0.978846, 0.734914, 0.199881, 0.0072, 0.0001 } // BLER
          }
        },
        { 80U, // SINR and BLER for CBS 80
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.8213, 10.1731, 11.525, 12.8768 }, // SINR
            { 0.963619, 0.609524, 0.0947218, 0.0024, 0 } // BLER
          }
        },
        { 88U, // SINR and BLER for CBS 88
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.7594, 10.0493, 11.3393, 12.6292 }, // SINR
            { 0.923561, 0.488942, 0.0507, 0.0004, 0 } // BLER
          }
        },
        { 96U, // SINR and BLER for CBS 96
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.6975, 9.9255, 11.1536, 12.3816 }, // SINR
            { 0.975191, 0.628713, 0.102056, 0.0028, 0 } // BLER
          }
        },
        { 112U, // SINR and BLER for CBS 112
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.6356, 9.8017, 10.9679, 12.134 }, // SINR
            { 0.95463, 0.526639, 0.0687, 0.0012, 0 } // BLER
          }
        },
        { 120U, // SINR and BLER for CBS 120
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.5737, 9.6779, 10.7822, 11.8864 }, // SINR
            { 0.963619, 0.537975, 0.0407, 0.0002, 0 } // BLER
          }
        },
        { 192U, // SINR and BLER for CBS 192
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.5118, 9.5541, 10.5965, 11.6388 }, // SINR
            { 0.984615, 0.68516, 0.101504, 0.001, 0 } // BLER
          }
        },
        { 208U, // SINR and BLER for CBS 208
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.4499, 9.4303, 10.4108, 11.3912 }, // SINR
            { 0.964962, 0.509577, 0.0401, 0.0002, 0 } // BLER
          }
        },
        { 224U, // SINR and BLER for CBS 224
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.388, 9.3065, 10.2251, 11.1436 }, // SINR
            { 0.981731, 0.657861, 0.0998759, 0.0009, 0 } // BLER
          }
        },
        { 240U, // SINR and BLER for CBS 240
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.2642, 9.0589, 9.8537, 10.6484 }, // SINR
            { 0.967105, 0.63625, 0.122096, 0.0039, 0 } // BLER
          }
        },
        { 272U, // SINR and BLER for CBS 272
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.963619, 0.673429, 0.173276, 0.0085, 0.0001 } // BLER
          }
        },
        { 304U, // SINR and BLER for CBS 304
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.98187, 0.816083, 0.288813, 0.0226, 0.0003 } // BLER
          }
        },
        { 336U, // SINR and BLER for CBS 336
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.988462, 0.739943, 0.203479, 0.0104, 0.0001 } // BLER
          }
        },
        { 368U, // SINR and BLER for CBS 368
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.958022, 0.645101, 0.118638, 0.0027, 0 } // BLER
          }
        },
        { 384U, // SINR and BLER for CBS 384
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.973282, 0.681516, 0.114745, 0.0019, 0.0001 } // BLER
          }
        },
        { 432U, // SINR and BLER for CBS 432
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.978053, 0.685829, 0.108014, 0.0027, 0 } // BLER
          }
        },
        { 456U, // SINR and BLER for CBS 456
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.984615, 0.694595, 0.0838341, 0.0012, 0.0001 } // BLER
          }
        },
        { 552U, // SINR and BLER for CBS 552
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.992308, 0.697011, 0.0575, 0, 0 } // BLER
          }
        },
        { 704U, // SINR and BLER for CBS 704
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.9875, 0.609597, 0.0323, 0.0002, 0 } // BLER
          }
        },
        { 768U, // SINR and BLER for CBS 768
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.994231, 0.616029, 0.0207, 0, 0 } // BLER
          }
        },
        { 848U, // SINR and BLER for CBS 848
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.990385, 0.590023, 0.0151, 0, 0 } // BLER
          }
        },
        { 928U, // SINR and BLER for CBS 928
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.992308, 0.549893, 0.0103, 0, 0 } // BLER
          }
        },
        { 984U, // SINR and BLER for CBS 984
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.991346, 0.544528, 0.0071, 0, 0 } // BLER
          }
        },
        { 1064U, // SINR and BLER for CBS 1064
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.997115, 0.576484, 0.0069, 0, 0 } // BLER
          }
        },
        { 1160U, // SINR and BLER for CBS 1160
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 0.996154, 0.469907, 0.0025, 0, 0 } // BLER
          }
        },
        { 1256U, // SINR and BLER for CBS 1256
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.554348, 0.0024, 0.0001, 0 } // BLER
          }
        },
        { 1416U, // SINR and BLER for CBS 1416
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.727143, 0.0073, 0, 0 } // BLER
          }
        },
        { 1544U, // SINR and BLER for CBS 1544
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.883562, 0.0356, 0.0001, 0 } // BLER
          }
        },
        { 1736U, // SINR and BLER for CBS 1736
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.809375, 0.0181, 0.0001, 0 } // BLER
          }
        },
        { 1864U, // SINR and BLER for CBS 1864
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.91521, 0.0326, 0.0001, 0 } // BLER
          }
        },
        { 2024U, // SINR and BLER for CBS 2024
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.722765, 0.0027, 0, 0 } // BLER
          }
        },
        { 2216U, // SINR and BLER for CBS 2216
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.850993, 0.0078, 0.0001, 0 } // BLER
          }
        },
        { 2280U, // SINR and BLER for CBS 2280
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.75, 0.0016, 0.0001, 0 } // BLER
          }
        },
        { 2536U, // SINR and BLER for CBS 2536
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.825806, 0.0022, 0, 0 } // BLER
          }
        },
        { 2856U, // SINR and BLER for CBS 2856
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.698087, 0.0009, 0, 0 } // BLER
          }
        },
        { 3104U, // SINR and BLER for CBS 3104
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.789352, 0.0005, 0.0001, 0 } // BLER
          }
        },
        { 3496U, // SINR and BLER for CBS 3496
          NrEesmErrorModel::DoubleTuple{
            { 7.4694, 8.1404, 8.8113, 9.4823, 10.1532 }, // SINR
            { 1, 0.811709, 0.0009, 0, 0 } // BLER
          }
        }
    },
    { // MCS 8
        { 24U, // SINR and BLER for CBS 24
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 10.4596, 12.1827, 13.9059, 15.629 }, // SINR
            { 0.934353, 0.585616, 0.141827, 0.0094, 0.0001 } // BLER
          }
        },
        { 32U, // SINR and BLER for CBS 32
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 10.3668, 11.997, 13.6273, 15.2576 }, // SINR
            { 0.952206, 0.645202, 0.158417, 0.0058, 0 } // BLER
          }
        },
        { 48U, // SINR and BLER for CBS 48
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 10.2739, 11.8113, 13.3488, 14.8862 }, // SINR
            { 0.963619, 0.683155, 0.179255, 0.0098, 0.0001 } // BLER
          }
        },
        { 64U, // SINR and BLER for CBS 64
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 10.243, 11.7494, 13.2559, 14.7624 }, // SINR
            { 0.894965, 0.429111, 0.0481, 0.0006, 0 } // BLER
          }
        },
        { 72U, // SINR and BLER for CBS 72
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 10.1501, 11.5637, 12.9774, 14.391 }, // SINR
            { 0.942029, 0.525826, 0.0687, 0.0008, 0 } // BLER
          }
        },
        { 88U, // SINR and BLER for CBS 88
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 10.0882, 11.4399, 12.7917, 14.1434 }, // SINR
            { 0.8625, 0.347796, 0.0226, 0.0001, 0 } // BLER
          }
        },
        { 96U, // SINR and BLER for CBS 96
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 10.0263, 11.3161, 12.606, 13.8958 }, // SINR
            { 0.908854, 0.44788, 0.0442, 0.0002, 0 } // BLER
          }
        },
        { 112U, // SINR and BLER for CBS 112
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.9644, 11.1923, 12.4203, 13.6482 }, // SINR
            { 0.95073, 0.530083, 0.073, 0.0017, 0 } // BLER
          }
        },
        { 128U, // SINR and BLER for CBS 128
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.9025, 11.0685, 12.2346, 13.4006 }, // SINR
            { 0.921429, 0.382576, 0.0238, 0.0002, 0 } // BLER
          }
        },
        { 192U, // SINR and BLER for CBS 192
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.8406, 10.9447, 12.0489, 13.153 }, // SINR
            { 0.929348, 0.475187, 0.0379, 0.0007, 0 } // BLER
          }
        },
        { 208U, // SINR and BLER for CBS 208
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.7787, 10.8209, 11.8632, 12.9054 }, // SINR
            { 0.961466, 0.523148, 0.0547, 0.001, 0 } // BLER
          }
        },
        { 224U, // SINR and BLER for CBS 224
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.7168, 10.6971, 11.6775, 12.6578 }, // SINR
            { 0.880102, 0.346653, 0.0236, 0.0001, 0 } // BLER
          }
        },
        { 240U, // SINR and BLER for CBS 240
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.593, 10.4495, 11.3061, 12.1626 }, // SINR
            { 0.940647, 0.516194, 0.0812, 0.0022, 0 } // BLER
          }
        },
        { 272U, // SINR and BLER for CBS 272
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.903169, 0.428872, 0.0676, 0.0019, 0 } // BLER
          }
        },
        { 304U, // SINR and BLER for CBS 304
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.934353, 0.541843, 0.0953252, 0.0033, 0.0001 } // BLER
          }
        },
        { 336U, // SINR and BLER for CBS 336
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.880952, 0.381401, 0.035, 0.0008, 0 } // BLER
          }
        },
        { 368U, // SINR and BLER for CBS 368
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.89569, 0.396552, 0.0315, 0.0002, 0 } // BLER
          }
        },
        { 384U, // SINR and BLER for CBS 384
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.955224, 0.567522, 0.0705, 0.0015, 0 } // BLER
          }
        },
        { 432U, // SINR and BLER for CBS 432
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.949818, 0.496078, 0.0488, 0.0006, 0 } // BLER
          }
        },
        { 456U, // SINR and BLER for CBS 456
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.905594, 0.280971, 0.0086, 0, 0 } // BLER
          }
        },
        { 552U, // SINR and BLER for CBS 552
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.940693, 0.328571, 0.0064, 0, 0 } // BLER
          }
        },
        { 704U, // SINR and BLER for CBS 704
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.924107, 0.278761, 0.0028, 0, 0 } // BLER
          }
        },
        { 768U, // SINR and BLER for CBS 768
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.898768, 0.139771, 0.0002, 0, 0 } // BLER
          }
        },
        { 848U, // SINR and BLER for CBS 848
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.903169, 0.14627, 0.0009, 0, 0 } // BLER
          }
        },
        { 928U, // SINR and BLER for CBS 928
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.915493, 0.15974, 0.0002, 0, 0 } // BLER
          }
        },
        { 984U, // SINR and BLER for CBS 984
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.932482, 0.157425, 0.0001, 0, 0 } // BLER
          }
        },
        { 1064U, // SINR and BLER for CBS 1064
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.903169, 0.0881, 0.0003, 0.0001, 0 } // BLER
          }
        },
        { 1160U, // SINR and BLER for CBS 1160
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.934353, 0.0965538, 0, 0, 0 } // BLER
          }
        },
        { 1256U, // SINR and BLER for CBS 1256
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.951287, 0.0871671, 0, 0, 0 } // BLER
          }
        },
        { 1416U, // SINR and BLER for CBS 1416
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.943015, 0.0728, 0, 0, 0 } // BLER
          }
        },
        { 1544U, // SINR and BLER for CBS 1544
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.980769, 0.158375, 0, 0, 0 } // BLER
          }
        },
        { 1736U, // SINR and BLER for CBS 1736
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.984615, 0.205212, 0, 0, 0 } // BLER
          }
        },
        { 1864U, // SINR and BLER for CBS 1864
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.988462, 0.205606, 0.0001, 0, 0 } // BLER
          }
        },
        { 2024U, // SINR and BLER for CBS 2024
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.997115, 0.180576, 0.0001, 0, 0 } // BLER
          }
        },
        { 2216U, // SINR and BLER for CBS 2216
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.9875, 0.132105, 0, 0, 0 } // BLER
          }
        },
        { 2280U, // SINR and BLER for CBS 2280
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.995192, 0.180755, 0.0002, 0, 0 } // BLER
          }
        },
        { 2536U, // SINR and BLER for CBS 2536
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.985577, 0.0654, 0, 0, 0 } // BLER
          }
        },
        { 2856U, // SINR and BLER for CBS 2856
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.997115, 0.0892428, 0, 0, 0 } // BLER
          }
        },
        { 3104U, // SINR and BLER for CBS 3104
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 1, 0.0842, 0, 0, 0 } // BLER
          }
        },
        { 3496U, // SINR and BLER for CBS 3496
          NrEesmErrorModel::DoubleTuple{
            { 8.7365, 9.4692, 10.2019, 10.9347, 11.6674 }, // SINR
            { 0.997115, 0.0462, 0, 0, 0 } // BLER
          }
        }
    },
    { // MCS 9
        { 24U, // SINR and BLER for CBS 24
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.7266, 13.4496, 15.1726, 16.8956 }, // SINR
            { 0.968985, 0.68078, 0.209799, 0.017, 0.0006 } // BLER
          }
        },
        { 40U, // SINR and BLER for CBS 40
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.6647, 13.3258, 14.9869, 16.648 }, // SINR
            { 0.949818, 0.58871, 0.130469, 0.0051, 0 } // BLER
          }
        },
        { 56U, // SINR and BLER for CBS 56
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.5718, 13.1401, 14.7083, 16.2766 }, // SINR
            { 0.944444, 0.563326, 0.0968929, 0.0026, 0 } // BLER
          }
        },
        { 72U, // SINR and BLER for CBS 72
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.5409, 13.0782, 14.6155, 16.1528 }, // SINR
            { 0.809335, 0.246807, 0.0109, 0, 0 } // BLER
          }
        },
        { 80U, // SINR and BLER for CBS 80
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.4171, 12.8306, 14.2441, 15.6576 }, // SINR
            { 0.839286, 0.303442, 0.0245, 0.0006, 0 } // BLER
          }
        },
        { 96U, // SINR and BLER for CBS 96
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.3552, 12.7068, 14.0584, 15.41 }, // SINR
            { 0.842715, 0.303357, 0.0188, 0, 0 } // BLER
          }
        },
        { 112U, // SINR and BLER for CBS 112
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.2933, 12.583, 13.8727, 15.1624 }, // SINR
            { 0.871622, 0.313433, 0.0215, 0.0003, 0 } // BLER
          }
        },
        { 128U, // SINR and BLER for CBS 128
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.2314, 12.4592, 13.687, 14.9148 }, // SINR
            { 0.795525, 0.17389, 0.003, 0, 0 } // BLER
          }
        },
        { 184U, // SINR and BLER for CBS 184
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.1695, 12.3354, 13.5013, 14.6672 }, // SINR
            { 0.925893, 0.414062, 0.0228, 0, 0 } // BLER
          }
        },
        { 208U, // SINR and BLER for CBS 208
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.1076, 12.2116, 13.3156, 14.4196 }, // SINR
            { 0.669503, 0.10095, 0.0014, 0, 0 } // BLER
          }
        },
        { 224U, // SINR and BLER for CBS 224
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 11.0457, 12.0878, 13.1299, 14.172 }, // SINR
            { 0.703804, 0.138674, 0.0023, 0, 0 } // BLER
          }
        },
        { 240U, // SINR and BLER for CBS 240
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.9219, 11.8402, 12.7585, 13.6768 }, // SINR
            { 0.776198, 0.190038, 0.0076, 0.0001, 0 } // BLER
          }
        },
        { 272U, // SINR and BLER for CBS 272
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.787577, 0.270923, 0.0228, 0.0006, 0.0001 } // BLER
          }
        },
        { 304U, // SINR and BLER for CBS 304
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.810127, 0.291763, 0.0182, 0.0002, 0 } // BLER
          }
        },
        { 336U, // SINR and BLER for CBS 336
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.611244, 0.0973344, 0.0014, 0, 0 } // BLER
          }
        },
        { 368U, // SINR and BLER for CBS 368
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.640783, 0.108619, 0.0025, 0.0001, 0 } // BLER
          }
        },
        { 384U, // SINR and BLER for CBS 384
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.708799, 0.125874, 0.0023, 0.0001, 0 } // BLER
          }
        },
        { 432U, // SINR and BLER for CBS 432
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.616587, 0.0765, 0.0008, 0, 0 } // BLER
          }
        },
        { 456U, // SINR and BLER for CBS 456
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.730603, 0.0902, 0.0009, 0.0001, 0 } // BLER
          }
        },
        { 552U, // SINR and BLER for CBS 552
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.623786, 0.0415, 0.0001, 0, 0 } // BLER
          }
        },
        { 704U, // SINR and BLER for CBS 704
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.69877, 0.0504, 0.0001, 0, 0 } // BLER
          }
        },
        { 768U, // SINR and BLER for CBS 768
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.628094, 0.0213, 0, 0, 0 } // BLER
          }
        },
        { 848U, // SINR and BLER for CBS 848
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.721208, 0.0286, 0, 0, 0 } // BLER
          }
        },
        { 928U, // SINR and BLER for CBS 928
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.678476, 0.0173, 0, 0, 0 } // BLER
          }
        },
        { 984U, // SINR and BLER for CBS 984
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.576233, 0.0072, 0, 0, 0 } // BLER
          }
        },
        { 1064U, // SINR and BLER for CBS 1064
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.65051, 0.0111, 0.0001, 0, 0 } // BLER
          }
        },
        { 1160U, // SINR and BLER for CBS 1160
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.598592, 0.005, 0, 0, 0 } // BLER
          }
        },
        { 1256U, // SINR and BLER for CBS 1256
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.575568, 0.0023, 0.0001, 0.0001, 0 } // BLER
          }
        },
        { 1416U, // SINR and BLER for CBS 1416
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.561681, 0.0016, 0, 0, 0 } // BLER
          }
        },
        { 1544U, // SINR and BLER for CBS 1544
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.588542, 0.0009, 0, 0, 0 } // BLER
          }
        },
        { 1736U, // SINR and BLER for CBS 1736
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.558114, 0.0007, 0, 0, 0 } // BLER
          }
        },
        { 1864U, // SINR and BLER for CBS 1864
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.689516, 0.0009, 0, 0, 0 } // BLER
          }
        },
        { 1928U, // SINR and BLER for CBS 1928
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.846854, 0.0093, 0, 0, 0 } // BLER
          }
        },
        { 2216U, // SINR and BLER for CBS 2216
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.744186, 0.0022, 0, 0, 0 } // BLER
          }
        },
        { 2280U, // SINR and BLER for CBS 2280
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.721045, 0.0017, 0, 0, 0 } // BLER
          }
        },
        { 2536U, // SINR and BLER for CBS 2536
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.814082, 0.0026, 0.0001, 0, 0 } // BLER
          }
        },
        { 2856U, // SINR and BLER for CBS 2856
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.755178, 0.0011, 0, 0, 0 } // BLER
          }
        },
        { 3104U, // SINR and BLER for CBS 3104
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.795312, 0.0006, 0, 0, 0 } // BLER
          }
        },
        { 3496U, // SINR and BLER for CBS 3496
          NrEesmErrorModel::DoubleTuple{
            { 10.0036, 10.7981, 11.5926, 12.3871, 13.1816 }, // SINR
            { 0.760294, 0.0005, 0, 0, 0 } // BLER
          }
        }
    },
    { // MCS 10
      { 0U, NrEesmErrorModel::DoubleTuple{ { 0.0 } , { 0.0 } } }
    },
    { // MCS 11
        { 32U, // SINR and BLER for CBS 32
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 13.4462, 15.2196, 16.9931, 18.7665 }, // SINR
            { 0.971591, 0.777273, 0.291282, 0.0226, 0.0006 } // BLER
          }
        },
        { 48U, // SINR and BLER for CBS 48
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 13.3533, 15.0339, 16.7145, 18.3951 }, // SINR
            { 0.932065, 0.633578, 0.136697, 0.0032, 0.0002 } // BLER
          }
        },
        { 64U, // SINR and BLER for CBS 64
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 13.2914, 14.9101, 16.5288, 18.1475 }, // SINR
            { 0.934353, 0.532038, 0.0663, 0.0006, 0 } // BLER
          }
        },
        { 80U, // SINR and BLER for CBS 80
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 13.1676, 14.6625, 16.1574, 17.6523 }, // SINR
            { 0.919065, 0.467831, 0.0576, 0.0005, 0.0001 } // BLER
          }
        },
        { 96U, // SINR and BLER for CBS 96
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 13.1057, 14.5387, 15.9717, 17.4047 }, // SINR
            { 0.90493, 0.434708, 0.0346, 0.0001, 0 } // BLER
          }
        },
        { 112U, // SINR and BLER for CBS 112
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 13.0438, 14.4149, 15.786, 17.1571 }, // SINR
            { 0.90035, 0.380255, 0.0261, 0.0001, 0 } // BLER
          }
        },
        { 128U, // SINR and BLER for CBS 128
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.9819, 14.2911, 15.6003, 16.9095 }, // SINR
            { 0.844771, 0.213497, 0.0042, 0, 0 } // BLER
          }
        },
        { 192U, // SINR and BLER for CBS 192
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.92, 14.1673, 15.4146, 16.6619 }, // SINR
            { 0.838474, 0.214346, 0.0073, 0.0002, 0 } // BLER
          }
        },
        { 208U, // SINR and BLER for CBS 208
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.8581, 14.0435, 15.2289, 16.4143 }, // SINR
            { 0.824519, 0.204992, 0.0048, 0, 0 } // BLER
          }
        },
        { 224U, // SINR and BLER for CBS 224
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.7962, 13.9197, 15.0432, 16.1667 }, // SINR
            { 0.814873, 0.217128, 0.0049, 0, 0 } // BLER
          }
        },
        { 240U, // SINR and BLER for CBS 240
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.6724, 13.6721, 14.6718, 15.6715 }, // SINR
            { 0.786043, 0.216595, 0.0094, 0.0001, 0 } // BLER
          }
        },
        { 272U, // SINR and BLER for CBS 272
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.8, 0.243002, 0.0139, 0.0001, 0 } // BLER
          }
        },
        { 304U, // SINR and BLER for CBS 304
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.764137, 0.205882, 0.0083, 0.0002, 0.0001 } // BLER
          }
        },
        { 320U, // SINR and BLER for CBS 320
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.908929, 0.411645, 0.0298, 0.0002, 0 } // BLER
          }
        },
        { 368U, // SINR and BLER for CBS 368
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.909574, 0.386742, 0.0205, 0.0002, 0 } // BLER
          }
        },
        { 384U, // SINR and BLER for CBS 384
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.902972, 0.307598, 0.012, 0.0002, 0 } // BLER
          }
        },
        { 432U, // SINR and BLER for CBS 432
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.820411, 0.187128, 0.0037, 0.0001, 0 } // BLER
          }
        },
        { 456U, // SINR and BLER for CBS 456
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.794753, 0.0965021, 0.0006, 0, 0 } // BLER
          }
        },
        { 552U, // SINR and BLER for CBS 552
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.807031, 0.0809, 0.0001, 0, 0 } // BLER
          }
        },
        { 704U, // SINR and BLER for CBS 704
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.783537, 0.0556, 0, 0, 0 } // BLER
          }
        },
        { 768U, // SINR and BLER for CBS 768
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.755117, 0.0339, 0.0001, 0, 0 } // BLER
          }
        },
        { 848U, // SINR and BLER for CBS 848
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.746324, 0.0259, 0.0001, 0, 0 } // BLER
          }
        },
        { 928U, // SINR and BLER for CBS 928
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.784299, 0.0256, 0, 0, 0 } // BLER
          }
        },
        { 984U, // SINR and BLER for CBS 984
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.764222, 0.0199, 0, 0, 0 } // BLER
          }
        },
        { 1064U, // SINR and BLER for CBS 1064
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.779545, 0.0152, 0, 0, 0 } // BLER
          }
        },
        { 1160U, // SINR and BLER for CBS 1160
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.740607, 0.0096, 0, 0, 0 } // BLER
          }
        },
        { 1256U, // SINR and BLER for CBS 1256
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.802673, 0.0104, 0, 0, 0 } // BLER
          }
        },
        { 1416U, // SINR and BLER for CBS 1416
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.811306, 0.0064, 0, 0, 0 } // BLER
          }
        },
        { 1544U, // SINR and BLER for CBS 1544
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.771837, 0.0026, 0.0001, 0.0001, 0 } // BLER
          }
        },
        { 1736U, // SINR and BLER for CBS 1736
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.744913, 0.0015, 0, 0, 0 } // BLER
          }
        },
        { 1864U, // SINR and BLER for CBS 1864
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.717179, 0.0002, 0, 0, 0 } // BLER
          }
        },
        { 2024U, // SINR and BLER for CBS 2024
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.696721, 0.0006, 0, 0, 0 } // BLER
          }
        },
        { 2152U, // SINR and BLER for CBS 2152
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.964286, 0.0257, 0, 0, 0 } // BLER
          }
        },
        { 2280U, // SINR and BLER for CBS 2280
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.880068, 0.0052, 0, 0, 0 } // BLER
          }
        },
        { 2536U, // SINR and BLER for CBS 2536
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.94708, 0.0094, 0, 0, 0 } // BLER
          }
        },
        { 2856U, // SINR and BLER for CBS 2856
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.956801, 0.0056, 0, 0, 0 } // BLER
          }
        },
        { 3104U, // SINR and BLER for CBS 3104
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.964962, 0.0036, 0, 0, 0 } // BLER
          }
        },
        { 3496U, // SINR and BLER for CBS 3496
          NrEesmErrorModel::DoubleTuple{
            { 11.6728, 12.5486, 13.4245, 14.3004, 15.1763 }, // SINR
            { 0.967803, 0.0038, 0, 0, 0 } // BLER
          }
        }
    }
  }
};

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

  if (sinrHistory.size () > 0)
    {
      // evaluate SINR_eff: as per Chase Combining

      // first step: create map_sum as the sum of all the allocated RBs in
      // different transmissions...
      std::vector<int> map_sum;
      for (const auto & output : sinrHistory)
        {
          Ptr<NrEesmErrorModelOutput> eesmOutput = DynamicCast<NrEesmErrorModelOutput> (output);
          map_sum.insert (map_sum.end (), eesmOutput->m_map.begin (), eesmOutput->m_map.end ());
        }
      // sort the resulting map
      std::sort (map_sum.begin (), map_sum.end ());
      // Rremove duplicate elements
      auto pte = std::unique (map_sum.begin (), map_sum.end ());
      // dups now in [pte, map_sum.end()]
      map_sum.erase (pte, map_sum.end ());

      // second step: create sinr_sum (vector of retransmission's SINR sum)
      SpectrumValue sinr_sum;
      for (const auto & output : sinrHistory)
        {
          Ptr<NrEesmErrorModelOutput> eesmOutput = DynamicCast<NrEesmErrorModelOutput> (output);
          sinr_sum += eesmOutput->m_sinr;
        }

      // compute effective SINR with the sinr_sum vector and map_sum RB map
      SINR = SinrEff (sinr_sum, map_sum, mcs);
    }

  NS_LOG_DEBUG (" SINR after retx " << SINR << " SINR last tx" << tbSinr <<
                " HARQ " << sinrHistory.size ());

  // selection of LDPC base graph type (1 or 2), as per TS 38.212
  GraphType bg_type = GetBaseGraphType (sizeBit, mcs);

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
               " bits distributed in " << C << " CBs of " << K );

  double errorRate = 1.0;

  if (C != 1)
    {
      double cbler = MappingSinrBler (SINR, mcs, K);
      errorRate = 1.0 - pow (1.0 - cbler, C);
    }
  else
    {
      errorRate = MappingSinrBler (SINR, mcs, K);
    }

  NS_LOG_LOGIC (" Error rate " << errorRate);
  NS_ASSERT (m_mcsEcrTable != nullptr);

  Ptr<NrEesmErrorModelOutput> ret = Create<NrEesmErrorModelOutput> (errorRate);
  ret->m_sinr = sinr;
  ret->m_map = map;
  ret->m_sinrEff = SINR;
  ret->m_infoBits = sizeBit;
  ret->m_codeBits = (sizeBit) / m_mcsEcrTable->at (mcs); // CC combining
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

  NS_LOG_INFO (" mcs:" << mcs << " subcarriers" << usefulSc <<
               " rsc element:" << rscElement);

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

} // namespace ns3

