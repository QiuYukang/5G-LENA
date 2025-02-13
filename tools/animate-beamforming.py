#! /usr/bin/env python3

# Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# SPDX-License-Identifier: GPL-2.0-only

import matplotlib.pyplot as plt
import numpy as np
from matplotlib import gridspec
from matplotlib.animation import FuncAnimation

log = """
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.41308e-14 txTheta 60 rxTheta 60 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.06388e-14 txTheta 60 rxTheta 60 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.57877e-14 txTheta 60 rxTheta 60 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.35083e-15 txTheta 60 rxTheta 60 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.43918e-15 txTheta 60 rxTheta 60 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.46211e-14 txTheta 60 rxTheta 60 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.53348e-13 txTheta 60 rxTheta 60 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.10966e-13 txTheta 60 rxTheta 60 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.93308e-14 txTheta 60 rxTheta 90 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.34609e-13 txTheta 60 rxTheta 90 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.57264e-13 txTheta 60 rxTheta 90 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.48956e-14 txTheta 60 rxTheta 90 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.85409e-15 txTheta 60 rxTheta 90 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.82624e-14 txTheta 60 rxTheta 90 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.69142e-14 txTheta 60 rxTheta 90 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.89271e-14 txTheta 60 rxTheta 90 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.29035e-14 txTheta 60 rxTheta 120 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.27702e-14 txTheta 60 rxTheta 120 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.74805e-15 txTheta 60 rxTheta 120 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.46216e-15 txTheta 60 rxTheta 120 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.06265e-14 txTheta 60 rxTheta 120 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.86879e-14 txTheta 60 rxTheta 120 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.58722e-14 txTheta 60 rxTheta 120 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.25453e-14 txTheta 60 rxTheta 120 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.09465e-14 txTheta 60 rxTheta 60 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.60122e-14 txTheta 60 rxTheta 60 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.12757e-14 txTheta 60 rxTheta 60 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.38116e-15 txTheta 60 rxTheta 60 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.93118e-15 txTheta 60 rxTheta 60 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.78501e-14 txTheta 60 rxTheta 60 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.2702e-13 txTheta 60 rxTheta 60 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.69278e-13 txTheta 60 rxTheta 60 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.3197e-13 txTheta 60 rxTheta 90 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.40771e-13 txTheta 60 rxTheta 90 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.32885e-13 txTheta 60 rxTheta 90 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.13389e-14 txTheta 60 rxTheta 90 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.57422e-15 txTheta 60 rxTheta 90 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.1006e-14 txTheta 60 rxTheta 90 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.32908e-15 txTheta 60 rxTheta 90 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.8013e-14 txTheta 60 rxTheta 90 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.66123e-15 txTheta 60 rxTheta 120 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.35477e-15 txTheta 60 rxTheta 120 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.7159e-15 txTheta 60 rxTheta 120 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.83194e-16 txTheta 60 rxTheta 120 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.77122e-15 txTheta 60 rxTheta 120 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.44659e-14 txTheta 60 rxTheta 120 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.45873e-15 txTheta 60 rxTheta 120 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.08565e-15 txTheta 60 rxTheta 120 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.30357e-14 txTheta 60 rxTheta 60 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.61296e-14 txTheta 60 rxTheta 60 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.27213e-14 txTheta 60 rxTheta 60 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.81998e-15 txTheta 60 rxTheta 60 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.3821e-15 txTheta 60 rxTheta 60 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.64266e-14 txTheta 60 rxTheta 60 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.88212e-13 txTheta 60 rxTheta 60 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.75012e-13 txTheta 60 rxTheta 60 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.80917e-14 txTheta 60 rxTheta 90 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.03915e-13 txTheta 60 rxTheta 90 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.23982e-13 txTheta 60 rxTheta 90 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.3973e-14 txTheta 60 rxTheta 90 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.32051e-15 txTheta 60 rxTheta 90 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.77273e-15 txTheta 60 rxTheta 90 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.32144e-14 txTheta 60 rxTheta 90 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.89485e-14 txTheta 60 rxTheta 90 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.09036e-15 txTheta 60 rxTheta 120 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.32516e-15 txTheta 60 rxTheta 120 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.11308e-16 txTheta 60 rxTheta 120 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.63135e-16 txTheta 60 rxTheta 120 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.66188e-15 txTheta 60 rxTheta 120 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.38788e-14 txTheta 60 rxTheta 120 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.26057e-15 txTheta 60 rxTheta 120 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.06949e-14 txTheta 60 rxTheta 120 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.43102e-14 txTheta 60 rxTheta 60 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.02582e-14 txTheta 60 rxTheta 60 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.10006e-14 txTheta 60 rxTheta 60 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.01533e-14 txTheta 60 rxTheta 60 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.24938e-15 txTheta 60 rxTheta 60 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.90473e-14 txTheta 60 rxTheta 60 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.11949e-13 txTheta 60 rxTheta 60 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.05466e-12 txTheta 60 rxTheta 60 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.29666e-14 txTheta 60 rxTheta 90 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.47264e-13 txTheta 60 rxTheta 90 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.83342e-13 txTheta 60 rxTheta 90 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.32589e-14 txTheta 60 rxTheta 90 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.98722e-15 txTheta 60 rxTheta 90 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.51674e-15 txTheta 60 rxTheta 90 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.67585e-14 txTheta 60 rxTheta 90 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.90907e-14 txTheta 60 rxTheta 90 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.48495e-15 txTheta 60 rxTheta 120 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.02988e-15 txTheta 60 rxTheta 120 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.81198e-16 txTheta 60 rxTheta 120 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.79487e-16 txTheta 60 rxTheta 120 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.01045e-16 txTheta 60 rxTheta 120 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.81478e-15 txTheta 60 rxTheta 120 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.10504e-15 txTheta 60 rxTheta 120 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.50895e-14 txTheta 60 rxTheta 120 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.67164e-13 txTheta 60 rxTheta 60 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.20231e-13 txTheta 60 rxTheta 60 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.12644e-13 txTheta 60 rxTheta 60 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.93967e-14 txTheta 60 rxTheta 60 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.81068e-14 txTheta 60 rxTheta 60 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.99852e-13 txTheta 60 rxTheta 60 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.24875e-12 txTheta 60 rxTheta 60 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.76761e-12 txTheta 60 rxTheta 60 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.6592e-13 txTheta 60 rxTheta 90 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.05308e-13 txTheta 60 rxTheta 90 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.54129e-13 txTheta 60 rxTheta 90 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.00351e-14 txTheta 60 rxTheta 90 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.3713e-15 txTheta 60 rxTheta 90 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.53748e-15 txTheta 60 rxTheta 90 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.04897e-14 txTheta 60 rxTheta 90 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.7318e-13 txTheta 60 rxTheta 90 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.23542e-15 txTheta 60 rxTheta 120 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.86191e-15 txTheta 60 rxTheta 120 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.17592e-15 txTheta 60 rxTheta 120 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.18785e-16 txTheta 60 rxTheta 120 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.03199e-15 txTheta 60 rxTheta 120 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.63335e-15 txTheta 60 rxTheta 120 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.29858e-14 txTheta 60 rxTheta 120 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.05644e-14 txTheta 60 rxTheta 120 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.23042e-13 txTheta 60 rxTheta 60 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.42147e-13 txTheta 60 rxTheta 60 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.00808e-13 txTheta 60 rxTheta 60 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.26842e-13 txTheta 60 rxTheta 60 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.39265e-13 txTheta 60 rxTheta 60 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.90755e-13 txTheta 60 rxTheta 60 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.27705e-12 txTheta 60 rxTheta 60 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.71161e-11 txTheta 60 rxTheta 60 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.12303e-13 txTheta 60 rxTheta 90 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.4839e-13 txTheta 60 rxTheta 90 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.45069e-14 txTheta 60 rxTheta 90 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.92052e-15 txTheta 60 rxTheta 90 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.08732e-14 txTheta 60 rxTheta 90 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.58301e-14 txTheta 60 rxTheta 90 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.98301e-13 txTheta 60 rxTheta 90 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.06977e-12 txTheta 60 rxTheta 90 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.51701e-14 txTheta 60 rxTheta 120 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.17681e-14 txTheta 60 rxTheta 120 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.04161e-15 txTheta 60 rxTheta 120 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.89214e-15 txTheta 60 rxTheta 120 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.80368e-15 txTheta 60 rxTheta 120 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.33189e-14 txTheta 60 rxTheta 120 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.70318e-14 txTheta 60 rxTheta 120 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.25453e-13 txTheta 60 rxTheta 120 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.17799e-14 txTheta 60 rxTheta 60 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.58669e-14 txTheta 60 rxTheta 60 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.68059e-14 txTheta 60 rxTheta 60 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.78226e-14 txTheta 60 rxTheta 60 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.62544e-15 txTheta 60 rxTheta 60 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.01053e-14 txTheta 60 rxTheta 60 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.37126e-13 txTheta 60 rxTheta 60 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.78347e-12 txTheta 60 rxTheta 60 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.61035e-14 txTheta 60 rxTheta 90 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.54557e-13 txTheta 60 rxTheta 90 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.04702e-12 txTheta 60 rxTheta 90 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.79505e-14 txTheta 60 rxTheta 90 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.7845e-15 txTheta 60 rxTheta 90 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.36028e-14 txTheta 60 rxTheta 90 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.82423e-14 txTheta 60 rxTheta 90 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.64414e-13 txTheta 60 rxTheta 90 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.16173e-14 txTheta 60 rxTheta 120 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.16065e-14 txTheta 60 rxTheta 120 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.18759e-15 txTheta 60 rxTheta 120 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.14127e-16 txTheta 60 rxTheta 120 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.4367e-15 txTheta 60 rxTheta 120 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.40036e-14 txTheta 60 rxTheta 120 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.52308e-15 txTheta 60 rxTheta 120 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.24872e-14 txTheta 60 rxTheta 120 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.03374e-13 txTheta 60 rxTheta 60 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.7848e-13 txTheta 60 rxTheta 60 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.64208e-13 txTheta 60 rxTheta 60 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.34922e-14 txTheta 60 rxTheta 60 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.53945e-14 txTheta 60 rxTheta 60 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.10284e-15 txTheta 60 rxTheta 60 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.38306e-14 txTheta 60 rxTheta 60 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.05315e-13 txTheta 60 rxTheta 60 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.22514e-12 txTheta 60 rxTheta 90 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.23866e-11 txTheta 60 rxTheta 90 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.59751e-11 txTheta 60 rxTheta 90 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.67781e-13 txTheta 60 rxTheta 90 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.87433e-13 txTheta 60 rxTheta 90 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.18634e-13 txTheta 60 rxTheta 90 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.10284e-14 txTheta 60 rxTheta 90 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.53762e-13 txTheta 60 rxTheta 90 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.50095e-13 txTheta 60 rxTheta 120 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.23123e-13 txTheta 60 rxTheta 120 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.73725e-14 txTheta 60 rxTheta 120 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.25358e-15 txTheta 60 rxTheta 120 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.97491e-15 txTheta 60 rxTheta 120 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.36074e-14 txTheta 60 rxTheta 120 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.46701e-15 txTheta 60 rxTheta 120 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.95649e-14 txTheta 60 rxTheta 120 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.25863e-10 txTheta 90 rxTheta 60 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.56984e-10 txTheta 90 rxTheta 60 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.10367e-10 txTheta 90 rxTheta 60 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.45283e-12 txTheta 90 rxTheta 60 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.24239e-12 txTheta 90 rxTheta 60 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.31288e-11 txTheta 90 rxTheta 60 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.24248e-11 txTheta 90 rxTheta 60 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.41977e-11 txTheta 90 rxTheta 60 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.59497e-07 txTheta 90 rxTheta 90 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.41299e-06 txTheta 90 rxTheta 90 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.86648e-07 txTheta 90 rxTheta 90 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.40008e-08 txTheta 90 rxTheta 90 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.74703e-08 txTheta 90 rxTheta 90 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.83703e-09 txTheta 90 rxTheta 90 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.58682e-08 txTheta 90 rxTheta 90 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.82414e-10 txTheta 90 rxTheta 90 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.77407e-10 txTheta 90 rxTheta 120 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.40949e-10 txTheta 90 rxTheta 120 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.91289e-11 txTheta 90 rxTheta 120 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.34664e-12 txTheta 90 rxTheta 120 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.66826e-11 txTheta 90 rxTheta 120 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.09137e-10 txTheta 90 rxTheta 120 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.29636e-11 txTheta 90 rxTheta 120 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.28192e-11 txTheta 90 rxTheta 120 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.67253e-12 txTheta 90 rxTheta 60 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.12786e-12 txTheta 90 rxTheta 60 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.30784e-11 txTheta 90 rxTheta 60 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.08431e-12 txTheta 90 rxTheta 60 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.73336e-12 txTheta 90 rxTheta 60 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.21945e-12 txTheta 90 rxTheta 60 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.27734e-11 txTheta 90 rxTheta 60 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.84783e-11 txTheta 90 rxTheta 60 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.2959e-09 txTheta 90 rxTheta 90 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.65773e-09 txTheta 90 rxTheta 90 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.14699e-09 txTheta 90 rxTheta 90 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.4575e-10 txTheta 90 rxTheta 90 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.11466e-11 txTheta 90 rxTheta 90 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.60656e-12 txTheta 90 rxTheta 90 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.63008e-11 txTheta 90 rxTheta 90 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.12204e-11 txTheta 90 rxTheta 90 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.14212e-11 txTheta 90 rxTheta 120 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.99161e-12 txTheta 90 rxTheta 120 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.39881e-11 txTheta 90 rxTheta 120 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.29245e-12 txTheta 90 rxTheta 120 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.44733e-11 txTheta 90 rxTheta 120 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.77286e-11 txTheta 90 rxTheta 120 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.4765e-12 txTheta 90 rxTheta 120 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.72492e-12 txTheta 90 rxTheta 120 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.44675e-12 txTheta 90 rxTheta 60 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.02709e-11 txTheta 90 rxTheta 60 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.94249e-12 txTheta 90 rxTheta 60 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.81657e-13 txTheta 90 rxTheta 60 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.2605e-12 txTheta 90 rxTheta 60 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.20941e-12 txTheta 90 rxTheta 60 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.23598e-12 txTheta 90 rxTheta 60 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.88571e-11 txTheta 90 rxTheta 60 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.69912e-08 txTheta 90 rxTheta 90 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.46191e-08 txTheta 90 rxTheta 90 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.68457e-08 txTheta 90 rxTheta 90 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.97199e-10 txTheta 90 rxTheta 90 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.21764e-09 txTheta 90 rxTheta 90 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.20009e-10 txTheta 90 rxTheta 90 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.94099e-10 txTheta 90 rxTheta 90 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.43101e-11 txTheta 90 rxTheta 90 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.09122e-11 txTheta 90 rxTheta 120 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.36468e-11 txTheta 90 rxTheta 120 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.79191e-13 txTheta 90 rxTheta 120 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.69915e-13 txTheta 90 rxTheta 120 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.05945e-12 txTheta 90 rxTheta 120 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.44264e-12 txTheta 90 rxTheta 120 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.4197e-12 txTheta 90 rxTheta 120 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.36339e-12 txTheta 90 rxTheta 120 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.42795e-12 txTheta 90 rxTheta 60 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.19984e-12 txTheta 90 rxTheta 60 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.11964e-11 txTheta 90 rxTheta 60 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.13733e-12 txTheta 90 rxTheta 60 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.7753e-13 txTheta 90 rxTheta 60 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.26894e-12 txTheta 90 rxTheta 60 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.81637e-11 txTheta 90 rxTheta 60 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.02498e-10 txTheta 90 rxTheta 60 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.24018e-09 txTheta 90 rxTheta 90 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.00291e-08 txTheta 90 rxTheta 90 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.92461e-08 txTheta 90 rxTheta 90 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.48315e-09 txTheta 90 rxTheta 90 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.07037e-10 txTheta 90 rxTheta 90 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.65148e-10 txTheta 90 rxTheta 90 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.79318e-10 txTheta 90 rxTheta 90 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.64659e-11 txTheta 90 rxTheta 90 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.47012e-12 txTheta 90 rxTheta 120 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.20344e-12 txTheta 90 rxTheta 120 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.31106e-12 txTheta 90 rxTheta 120 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.20235e-13 txTheta 90 rxTheta 120 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.50452e-13 txTheta 90 rxTheta 120 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.06856e-12 txTheta 90 rxTheta 120 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.43328e-13 txTheta 90 rxTheta 120 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.63569e-12 txTheta 90 rxTheta 120 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.38856e-11 txTheta 90 rxTheta 60 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.43721e-11 txTheta 90 rxTheta 60 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.56083e-11 txTheta 90 rxTheta 60 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.99158e-12 txTheta 90 rxTheta 60 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.0774e-11 txTheta 90 rxTheta 60 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.89244e-11 txTheta 90 rxTheta 60 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.3937e-10 txTheta 90 rxTheta 60 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.77202e-10 txTheta 90 rxTheta 60 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.68424e-08 txTheta 90 rxTheta 90 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.05078e-07 txTheta 90 rxTheta 90 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.37394e-08 txTheta 90 rxTheta 90 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.51159e-09 txTheta 90 rxTheta 90 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.014e-09 txTheta 90 rxTheta 90 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.34781e-10 txTheta 90 rxTheta 90 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.17857e-09 txTheta 90 rxTheta 90 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.66939e-11 txTheta 90 rxTheta 90 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.16224e-11 txTheta 90 rxTheta 120 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.69275e-11 txTheta 90 rxTheta 120 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.78499e-12 txTheta 90 rxTheta 120 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.10579e-13 txTheta 90 rxTheta 120 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.47022e-13 txTheta 90 rxTheta 120 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.81861e-13 txTheta 90 rxTheta 120 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.82492e-12 txTheta 90 rxTheta 120 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.76238e-12 txTheta 90 rxTheta 120 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.20221e-11 txTheta 90 rxTheta 60 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.12336e-10 txTheta 90 rxTheta 60 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.51636e-11 txTheta 90 rxTheta 60 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.10871e-11 txTheta 90 rxTheta 60 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.24163e-11 txTheta 90 rxTheta 60 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.16015e-11 txTheta 90 rxTheta 60 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.14842e-10 txTheta 90 rxTheta 60 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.77642e-09 txTheta 90 rxTheta 60 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.89351e-08 txTheta 90 rxTheta 90 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.19518e-07 txTheta 90 rxTheta 90 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.72464e-08 txTheta 90 rxTheta 90 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.56532e-09 txTheta 90 rxTheta 90 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.42194e-09 txTheta 90 rxTheta 90 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.03899e-09 txTheta 90 rxTheta 90 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.12197e-09 txTheta 90 rxTheta 90 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.56534e-10 txTheta 90 rxTheta 90 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.65247e-12 txTheta 90 rxTheta 120 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.05435e-11 txTheta 90 rxTheta 120 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.79381e-11 txTheta 90 rxTheta 120 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.34259e-13 txTheta 90 rxTheta 120 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.33351e-13 txTheta 90 rxTheta 120 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.55057e-12 txTheta 90 rxTheta 120 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.72042e-12 txTheta 90 rxTheta 120 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.9134e-11 txTheta 90 rxTheta 120 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.1676e-10 txTheta 90 rxTheta 60 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.36241e-10 txTheta 90 rxTheta 60 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.66421e-10 txTheta 90 rxTheta 60 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.62972e-12 txTheta 90 rxTheta 60 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.42415e-12 txTheta 90 rxTheta 60 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.29759e-12 txTheta 90 rxTheta 60 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.41091e-11 txTheta 90 rxTheta 60 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.90421e-11 txTheta 90 rxTheta 60 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.8571e-07 txTheta 90 rxTheta 90 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.63029e-06 txTheta 90 rxTheta 90 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.39162e-06 txTheta 90 rxTheta 90 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.46784e-07 txTheta 90 rxTheta 90 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.32597e-08 txTheta 90 rxTheta 90 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.94321e-08 txTheta 90 rxTheta 90 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.57568e-08 txTheta 90 rxTheta 90 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.98153e-09 txTheta 90 rxTheta 90 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.79737e-10 txTheta 90 rxTheta 120 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.4356e-10 txTheta 90 rxTheta 120 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.08702e-10 txTheta 90 rxTheta 120 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.97576e-13 txTheta 90 rxTheta 120 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.2784e-13 txTheta 90 rxTheta 120 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.51378e-12 txTheta 90 rxTheta 120 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.05687e-12 txTheta 90 rxTheta 120 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.50486e-12 txTheta 90 rxTheta 120 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.51506e-10 txTheta 90 rxTheta 60 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.54698e-10 txTheta 90 rxTheta 60 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.01728e-10 txTheta 90 rxTheta 60 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.86498e-12 txTheta 90 rxTheta 60 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.24383e-11 txTheta 90 rxTheta 60 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.18513e-11 txTheta 90 rxTheta 60 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.97269e-11 txTheta 90 rxTheta 60 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.10227e-10 txTheta 90 rxTheta 60 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.35586e-06 txTheta 90 rxTheta 90 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.39907e-06 txTheta 90 rxTheta 90 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.70387e-06 txTheta 90 rxTheta 90 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.55823e-07 txTheta 90 rxTheta 90 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.05871e-07 txTheta 90 rxTheta 90 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.65865e-08 txTheta 90 rxTheta 90 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.80668e-08 txTheta 90 rxTheta 90 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.19037e-09 txTheta 90 rxTheta 90 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.49248e-10 txTheta 90 rxTheta 120 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.45107e-10 txTheta 90 rxTheta 120 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.33779e-10 txTheta 90 rxTheta 120 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.14067e-12 txTheta 90 rxTheta 120 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.7245e-11 txTheta 90 rxTheta 120 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.92418e-11 txTheta 90 rxTheta 120 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.65329e-11 txTheta 90 rxTheta 120 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.33797e-11 txTheta 90 rxTheta 120 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.64999e-14 txTheta 120 rxTheta 60 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.57428e-14 txTheta 120 rxTheta 60 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.28199e-14 txTheta 120 rxTheta 60 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.85175e-15 txTheta 120 rxTheta 60 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.16323e-15 txTheta 120 rxTheta 60 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.20512e-14 txTheta 120 rxTheta 60 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.19966e-13 txTheta 120 rxTheta 60 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.64278e-13 txTheta 120 rxTheta 60 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.20993e-14 txTheta 120 rxTheta 90 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.57519e-13 txTheta 120 rxTheta 90 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.67337e-13 txTheta 120 rxTheta 90 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.54857e-14 txTheta 120 rxTheta 90 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.73504e-15 txTheta 120 rxTheta 90 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.94489e-14 txTheta 120 rxTheta 90 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.45773e-14 txTheta 120 rxTheta 90 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.4102e-14 txTheta 120 rxTheta 90 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.09393e-14 txTheta 120 rxTheta 120 tx sector -90 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.05331e-14 txTheta 120 rxTheta 120 tx sector -90 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.66866e-15 txTheta 120 rxTheta 120 tx sector -90 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.72387e-15 txTheta 120 rxTheta 120 tx sector -90 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.23032e-14 txTheta 120 rxTheta 120 tx sector -90 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.18105e-13 txTheta 120 rxTheta 120 tx sector -90 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.81874e-14 txTheta 120 rxTheta 120 tx sector -90 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.0926e-14 txTheta 120 rxTheta 120 tx sector -90 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.55276e-14 txTheta 120 rxTheta 60 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.25036e-14 txTheta 120 rxTheta 60 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.60631e-15 txTheta 120 rxTheta 60 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.65356e-15 txTheta 120 rxTheta 60 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.74913e-15 txTheta 120 rxTheta 60 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.40994e-14 txTheta 120 rxTheta 60 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.92604e-14 txTheta 120 rxTheta 60 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.8612e-13 txTheta 120 rxTheta 60 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.42465e-13 txTheta 120 rxTheta 90 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.91728e-13 txTheta 120 rxTheta 90 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.72867e-13 txTheta 120 rxTheta 90 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.24896e-14 txTheta 120 rxTheta 90 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.36663e-15 txTheta 120 rxTheta 90 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.12631e-14 txTheta 120 rxTheta 90 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.49715e-15 txTheta 120 rxTheta 90 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.4305e-14 txTheta 120 rxTheta 90 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.44442e-15 txTheta 120 rxTheta 120 tx sector -67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.35962e-15 txTheta 120 rxTheta 120 tx sector -67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.77424e-15 txTheta 120 rxTheta 120 tx sector -67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.32806e-16 txTheta 120 rxTheta 120 tx sector -67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.91105e-15 txTheta 120 rxTheta 120 tx sector -67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.71159e-14 txTheta 120 rxTheta 120 tx sector -67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.67651e-15 txTheta 120 rxTheta 120 tx sector -67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.68614e-15 txTheta 120 rxTheta 120 tx sector -67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.5486e-14 txTheta 120 rxTheta 60 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.21658e-14 txTheta 120 rxTheta 60 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.88273e-15 txTheta 120 rxTheta 60 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.17394e-15 txTheta 120 rxTheta 60 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.07787e-15 txTheta 120 rxTheta 60 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.30636e-14 txTheta 120 rxTheta 60 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.47835e-13 txTheta 120 rxTheta 60 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.14237e-13 txTheta 120 rxTheta 60 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.80759e-14 txTheta 120 rxTheta 90 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.13212e-13 txTheta 120 rxTheta 90 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.28465e-13 txTheta 120 rxTheta 90 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.41155e-14 txTheta 120 rxTheta 90 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.50289e-15 txTheta 120 rxTheta 90 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.8524e-15 txTheta 120 rxTheta 90 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.04368e-14 txTheta 120 rxTheta 90 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.42817e-14 txTheta 120 rxTheta 90 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.21722e-15 txTheta 120 rxTheta 120 tx sector -45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.38422e-15 txTheta 120 rxTheta 120 tx sector -45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.42751e-16 txTheta 120 rxTheta 120 tx sector -45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.81233e-16 txTheta 120 rxTheta 120 tx sector -45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.86394e-15 txTheta 120 rxTheta 120 tx sector -45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.6427e-14 txTheta 120 rxTheta 120 tx sector -45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.10178e-15 txTheta 120 rxTheta 120 tx sector -45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.18364e-15 txTheta 120 rxTheta 120 tx sector -45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.17235e-14 txTheta 120 rxTheta 60 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.54449e-14 txTheta 120 rxTheta 60 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.62189e-14 txTheta 120 rxTheta 60 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.73346e-15 txTheta 120 rxTheta 60 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.46176e-15 txTheta 120 rxTheta 60 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.08764e-14 txTheta 120 rxTheta 60 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.23554e-13 txTheta 120 rxTheta 60 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.09521e-13 txTheta 120 rxTheta 60 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.30893e-14 txTheta 120 rxTheta 90 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.59216e-13 txTheta 120 rxTheta 90 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.91496e-13 txTheta 120 rxTheta 90 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.34398e-14 txTheta 120 rxTheta 90 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.1341e-15 txTheta 120 rxTheta 90 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.45075e-15 txTheta 120 rxTheta 90 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.29372e-14 txTheta 120 rxTheta 90 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.31868e-14 txTheta 120 rxTheta 90 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.79e-15 txTheta 120 rxTheta 120 tx sector -22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.34357e-15 txTheta 120 rxTheta 120 tx sector -22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.37982e-16 txTheta 120 rxTheta 120 tx sector -22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.65552e-16 txTheta 120 rxTheta 120 tx sector -22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.18418e-16 txTheta 120 rxTheta 120 tx sector -22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.52271e-15 txTheta 120 rxTheta 120 tx sector -22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.39167e-15 txTheta 120 rxTheta 120 tx sector -22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.14806e-14 txTheta 120 rxTheta 120 tx sector -22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.09349e-13 txTheta 120 rxTheta 60 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.43015e-14 txTheta 120 rxTheta 60 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.9329e-14 txTheta 120 rxTheta 60 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.11684e-14 txTheta 120 rxTheta 60 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.91629e-14 txTheta 120 rxTheta 60 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.14163e-13 txTheta 120 rxTheta 60 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.12176e-12 txTheta 120 rxTheta 60 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.28774e-12 txTheta 120 rxTheta 60 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.40563e-13 txTheta 120 rxTheta 90 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.10163e-13 txTheta 120 rxTheta 90 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.61154e-13 txTheta 120 rxTheta 90 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.84137e-15 txTheta 120 rxTheta 90 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.33314e-15 txTheta 120 rxTheta 90 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.79917e-15 txTheta 120 rxTheta 90 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.0057e-14 txTheta 120 rxTheta 90 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.30897e-13 txTheta 120 rxTheta 90 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.61241e-15 txTheta 120 rxTheta 120 tx sector 0 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.16537e-15 txTheta 120 rxTheta 120 tx sector 0 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.98702e-15 txTheta 120 rxTheta 120 tx sector 0 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.84577e-16 txTheta 120 rxTheta 120 tx sector 0 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.56949e-15 txTheta 120 rxTheta 120 tx sector 0 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.82769e-15 txTheta 120 rxTheta 120 tx sector 0 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.59197e-14 txTheta 120 rxTheta 120 tx sector 0 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.86403e-14 txTheta 120 rxTheta 120 tx sector 0 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.92132e-13 txTheta 120 rxTheta 60 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.07492e-13 txTheta 120 rxTheta 60 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.50823e-13 txTheta 120 rxTheta 60 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.48199e-14 txTheta 120 rxTheta 60 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.9406e-13 txTheta 120 rxTheta 60 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.12245e-13 txTheta 120 rxTheta 60 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.80328e-12 txTheta 120 rxTheta 60 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.32757e-11 txTheta 120 rxTheta 60 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.70266e-13 txTheta 120 rxTheta 90 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.14297e-13 txTheta 120 rxTheta 90 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.06583e-14 txTheta 120 rxTheta 90 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.13275e-15 txTheta 120 rxTheta 90 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.61816e-14 txTheta 120 rxTheta 90 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.23292e-14 txTheta 120 rxTheta 90 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.4395e-13 txTheta 120 rxTheta 90 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.13665e-13 txTheta 120 rxTheta 90 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.20274e-14 txTheta 120 rxTheta 120 tx sector 22.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.70535e-14 txTheta 120 rxTheta 120 tx sector 22.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.937e-15 txTheta 120 rxTheta 120 tx sector 22.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.93731e-15 txTheta 120 rxTheta 120 tx sector 22.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.96971e-15 txTheta 120 rxTheta 120 tx sector 22.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.23018e-14 txTheta 120 rxTheta 120 tx sector 22.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.83065e-14 txTheta 120 rxTheta 120 tx sector 22.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.4129e-13 txTheta 120 rxTheta 120 tx sector 22.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.60697e-14 txTheta 120 rxTheta 60 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.52745e-14 txTheta 120 rxTheta 60 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.9288e-14 txTheta 120 rxTheta 60 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.34556e-14 txTheta 120 rxTheta 60 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.41345e-15 txTheta 120 rxTheta 60 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.42048e-14 txTheta 120 rxTheta 60 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.67178e-13 txTheta 120 rxTheta 60 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.35713e-12 txTheta 120 rxTheta 60 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.09936e-14 txTheta 120 rxTheta 90 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.69754e-13 txTheta 120 rxTheta 90 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.04038e-12 txTheta 120 rxTheta 90 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.70366e-14 txTheta 120 rxTheta 90 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.0167e-15 txTheta 120 rxTheta 90 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.32459e-14 txTheta 120 rxTheta 90 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.0536e-14 txTheta 120 rxTheta 90 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.47342e-13 txTheta 120 rxTheta 90 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.68204e-14 txTheta 120 rxTheta 120 tx sector 45 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.69537e-14 txTheta 120 rxTheta 120 tx sector 45 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.89808e-15 txTheta 120 rxTheta 120 tx sector 45 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.75746e-16 txTheta 120 rxTheta 120 tx sector 45 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.57497e-15 txTheta 120 rxTheta 120 tx sector 45 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.66307e-14 txTheta 120 rxTheta 120 tx sector 45 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.61121e-15 txTheta 120 rxTheta 120 tx sector 45 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.47527e-14 txTheta 120 rxTheta 120 tx sector 45 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.28754e-14 txTheta 120 rxTheta 60 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.41899e-13 txTheta 120 rxTheta 60 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.56341e-13 txTheta 120 rxTheta 60 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.176e-14 txTheta 120 rxTheta 60 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.17642e-14 txTheta 120 rxTheta 60 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 3.10139e-15 txTheta 120 rxTheta 60 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.74158e-14 txTheta 120 rxTheta 60 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 6.20043e-13 txTheta 120 rxTheta 60 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.4651e-12 txTheta 120 rxTheta 90 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.33851e-11 txTheta 120 rxTheta 90 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.67127e-11 txTheta 120 rxTheta 90 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 8.82659e-13 txTheta 120 rxTheta 90 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.02643e-13 txTheta 120 rxTheta 90 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.23378e-13 txTheta 120 rxTheta 90 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 7.3142e-14 txTheta 120 rxTheta 90 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.33701e-13 txTheta 120 rxTheta 90 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.16083e-13 txTheta 120 rxTheta 120 tx sector 67.5 rx sector -90
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.94504e-13 txTheta 120 rxTheta 120 tx sector 67.5 rx sector -67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 9.49692e-14 txTheta 120 rxTheta 120 tx sector 67.5 rx sector -45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.18421e-15 txTheta 120 rxTheta 120 tx sector 67.5 rx sector -22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 4.20165e-15 txTheta 120 rxTheta 120 tx sector 67.5 rx sector 0
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 2.81702e-14 txTheta 120 rxTheta 120 tx sector 67.5 rx sector 22.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 5.82908e-15 txTheta 120 rxTheta 120 tx sector 67.5 rx sector 45
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [LOGIC]  Rx power: 1.568e-14 txTheta 120 rxTheta 120 tx sector 67.5 rx sector 67.5
+0.000000002s -1 IdealBeamformingAlgorithm:GetBeamformingVectors(): [DEBUG] Beamforming vectors with max power 5.39907e-06 for gNB with node id: 0 (0:0:25) and UE with node id: 1 (100:150:25) are txTheta 90 tx sector 67.5 rxTheta 90 rx sector -67.5
"""


def parse_beamforming_logs(logs):
    import re

    data = []
    gnbPos = None
    uePos = None
    for line in logs.splitlines():
        match = re.findall(r"\((\d+:\d+:\d+)\)", line)
        if match:
            gnbPos, uePos = match
            continue
        match = re.findall(
            ".*Rx power: (.*) txTheta (.*) rxTheta (.*) tx sector (.*) rx sector (.*)", line
        )
        if match:
            tabulatedData = {}
            for i, label in enumerate(["RxPower", "TxTheta", "RxTheta", "TxSector", "RxSector"]):
                tabulatedData[label] = float(match[0][i])
            data.append(tabulatedData)
    return gnbPos, uePos, data


gnbPos, uePos, data = parse_beamforming_logs(log)

if not gnbPos or not uePos or not data:
    raise Exception("Could not parse beamforming logs. This was tested with CellScanBeamforming")

fig = plt.figure(figsize=(10, 6))
gs = gridspec.GridSpec(1, 2, width_ratios=[3, 2])

# Axes settings for 3D projection plot
ax = fig.add_subplot(gs[0], projection="3d")
ax.set_xlabel("X")
ax.set_ylabel("Y")
ax.set_zlabel("Z")
ax.set_box_aspect([1, 1, 1])
ax.set_xlim([-10, 200])
ax.set_ylim([-200, 200])
ax.set_zlim([0, 50])

# And settings for RxPower plot
axdata = fig.add_subplot(gs[1])

# UE and gNB beam surfaces, these represent the beams with elipsoids
surfaces = [None, None]

# Sort beam pairs by received power in ascending order (last == highest)
data = list(sorted(data, key=lambda x: x["RxPower"]))

# Filter just highest 100 samples to cut down on render time
data = data[-100:]


def update(frame):
    global surfaces  # We need to modify these global variables to keep track of shapes and update them
    print(f"Rendering frame {frame}/{len(data)}")
    axdata.scatter(list(range(frame)), list(map(lambda x: data[x]["RxPower"], range(frame))))
    axdata.set_title(f'RxPower {data[frame]["RxPower"]}W')

    for centerTxt in [gnbPos, uePos]:
        # The "tip" of the ellipsoid lies here:
        C = np.array(list(map(float, centerTxt.split(":"))))

        # Ellipsoid radii (major, intermediate, minor)
        a, b, c = 10, 2, 1

        # Orientation in spherical coordinates (degrees)
        theta_deg = 45  # azimuth (xy plane)
        phi_deg = 30  # elevation from z-axis

        isGnb = centerTxt == gnbPos
        if isGnb:
            phi_deg, theta_deg = [data[frame]["TxTheta"], data[frame]["TxSector"]]
            phi_deg += 180
        else:
            phi_deg, theta_deg = [data[frame]["RxTheta"], data[frame]["RxSector"]]
            phi_deg += 180
            theta_deg -= 90

        # Convert to radians
        theta = np.radians(theta_deg)
        phi = np.radians(phi_deg)

        # Compute the direction vector (unit vector)
        dx = np.sin(phi) * np.cos(theta)
        dy = np.sin(phi) * np.sin(theta)
        dz = np.cos(phi)
        direction = np.array([dx, dy, dz])

        # Center of ellipsoid so that the major axis tip is at C
        center = C - a * direction

        # Parametrize the unrotated ellipsoid
        u = np.linspace(0, 2 * np.pi, 60)
        v = np.linspace(0, np.pi, 30)
        x = a * np.cos(u)[:, None] * np.sin(v)
        y = b * np.sin(u)[:, None] * np.sin(v)
        z = c * np.cos(v)[None, :] * np.ones_like(x)

        # Flatten and stack
        points = np.stack([x, y, z], axis=-1).reshape(-1, 3)

        # Rotation: align x-axis (major axis) to direction
        def rotation_matrix_from_vectors(vec1, vec2):
            a = vec1 / np.linalg.norm(vec1)
            b = vec2 / np.linalg.norm(vec2)
            v = np.cross(a, b)
            c = np.dot(a, b)
            if np.allclose(v, 0):
                return np.eye(3) if c > 0 else -np.eye(3)
            s = np.linalg.norm(v)
            kmat = np.array([[0, -v[2], v[1]], [v[2], 0, -v[0]], [-v[1], v[0], 0]])
            return np.eye(3) + kmat + kmat @ kmat * ((1 - c) / s**2)

        # Rotate from x-axis to direction
        R = rotation_matrix_from_vectors(np.array([1, 0, 0]), direction)

        # Apply rotation and translate to new center
        rotated_points = (R @ points.T).T + center

        # Reshape
        x_rot = rotated_points[:, 0].reshape(x.shape)
        y_rot = rotated_points[:, 1].reshape(y.shape)
        z_rot = rotated_points[:, 2].reshape(z.shape)

        # Plot
        if not surfaces[isGnb]:
            ax.scatter(*C, color="red", s=60, label=("gNB" if isGnb else "UE"))
        if surfaces[isGnb]:
            surfaces[isGnb].remove()
        surfaces[isGnb] = ax.plot_surface(
            x_rot, y_rot, z_rot, rstride=2, cstride=2, alpha=0.6, color="skyblue", edgecolor="k"
        )


# ax.view_init(azim=90, elev=0)

anim = FuncAnimation(fig, update, frames=len(data), interval=20, blit=False, repeat=True)
anim.save("beamforming.mp4")
# plt.show()
