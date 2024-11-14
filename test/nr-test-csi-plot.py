#! /usr/bin/env python

# Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# SPDX-License-Identifier: GPL-2.0-only

import json
import statistics
from collections import deque

from matplotlib import pyplot as plt

try:
    with open("nr-csi-test-output.json", "r") as f:
        contents = json.load(f)
except:
    print("Failed to open nr-csi-test-output.json. Run `test-runner --suite=nr-csi-test` first.")
    exit(1)


def plot_test_case(testCase, contents):
    if contents[testCase]["csiFb"] is None:
        print("Test-case was skipped due to its test-runner fullness:", testCase)
        return

    measurements = list(
        map(
            lambda x: (
                x["ts"],
                x["mcs"],
                x["corrupted"],
                x["assignedRbgs"],
                x["rank"],
                x["cellid"],
            ),
            contents[testCase]["rxTb"],
        )
    )

    thr_measurements = list(map(lambda x: (x["ts"], x["thr"]), contents[testCase]["ueThr"]))

    cqi_measurements = {}
    for ue in contents[testCase]["csiFb"].keys():
        cqi_measurements[ue] = list(
            map(lambda x: (x["ts"], x["sbCqi"]), contents[testCase]["csiFb"][ue])
        )

    # Determine which of the two UEs is subjected to interference (highest score = highest interference)
    scores = []
    for ue in cqi_measurements.keys():
        score = 0
        for subband in range(len(cqi_measurements[ue][0][1])):
            interferedBand = statistics.harmonic_mean(
                map(lambda x: x[1][subband], cqi_measurements[ue])
            )
            nonInterferedBand = cqi_measurements[ue][0][1][subband]
            score += nonInterferedBand - interferedBand
        scores.append((score, ue))

    ueCqiManagerToUeId = {}
    for ueId, (score, ueCqiManager) in enumerate(sorted(scores, reverse=True)):
        ueCqiManagerToUeId[ueCqiManager] = ueId

    app_state = {}
    for app in contents[testCase]["appState"].keys():
        app_state[app] = list(
            map(lambda x: (x["ts"], x["state"]), contents[testCase]["appState"][app])
        )

    # Remove interferer with single sample (it is the UE0 traffic)
    app_state = dict(filter(lambda x: len(x[1]) > 6, app_state.items()))

    for app in app_state:
        last_measurement_ts = measurements[-1][0]
        last_reported_state = 0 if app_state[app][-1][1] else 1
        app_state[app].append((last_measurement_ts, last_reported_state))

    x_axis = list(map(lambda x: int(x[0]), filter(lambda x: int(x[-1]) == 1, measurements)))
    y_mcs = list(map(lambda x: int(x[1]), filter(lambda x: int(x[-1]) == 1, measurements)))
    y_tbError = list(map(lambda x: int(x[2]), filter(lambda x: int(x[-1]) == 1, measurements)))
    y_allocatedRbgs = list(
        map(lambda x: int(x[3]), filter(lambda x: int(x[-1]) == 1, measurements))
    )
    y_rank = list(map(lambda x: int(x[4]), filter(lambda x: int(x[-1]) == 1, measurements)))
    y_tbErrorRate = []
    y_tbErrorRateWindow = []
    erroredTbs = 0
    erroredTbsWindow = deque()
    for i, tb in enumerate(y_tbError):
        if len(erroredTbsWindow) >= 10:
            erroredTbsWindow.popleft()
        erroredTbs += tb
        erroredTbsWindow.append(tb)
        y_tbErrorRate.append(float(erroredTbs) / (i + 1))
        y_tbErrorRateWindow.append(float(sum(erroredTbsWindow)) / len(erroredTbsWindow))

    x_axis_gaps = list(
        map(
            lambda i: (
                (x_axis[i], x_axis[i + 1] - x_axis[i])
                if (x_axis[i + 1] - x_axis[i]) > 1000000
                else None
            ),
            range(len(x_axis) - 1),
        )
    )
    x_axis_gaps = list(filter(lambda x: x is not None, x_axis_gaps))

    # x_axis2 = list(map(lambda x: int(x[0]), filter(lambda x: int(x[-1]) == 2, measurements)))
    # y_mcs2 = list(map(lambda x: int(x[1]), filter(lambda x: int(x[-1]) == 2, measurements)))
    # x_axis2_gaps = list(
    #    map(
    #        lambda i: (
    #            (x_axis2[i], x_axis2[i + 1] - x_axis2[i])
    #            if (x_axis2[i + 1] - x_axis2[i]) > 1000000
    #            else None
    #        ),
    #        range(len(x_axis2) - 1),
    #    )
    # )
    # x_axis2_gaps = list(filter(lambda x: x is not None, x_axis2_gaps))

    fig, axis = plt.subplots(7, 2, squeeze=False, figsize=(15, 10), sharex=True)
    # axis[0][1].scatter(x_axis2, y_mcs2, label=f"MCS2", color="red")

    plot_entries = [
        {
            "yAxis": y_mcs,
            "yRange": [0, 27],
            "label": "MCS",
            "plotType": "scatter",
            "yTicks": list(range(0, 31, 5)),
            "extra": {"s": 5},
        },
        {
            "yAxis": y_rank,
            "yRange": [0, 4],
            "label": "Rank",
            "plotType": "scatter",
            "yTicks": list(range(1, 5)),
            "extra": {"s": 5},
        },
        {
            "yAxis": y_allocatedRbgs,
            "yRange": [0, 106],
            "label": "Allocated RBGs",
            "plotType": "scatter",
            "extra": {"s": 5},
        },
        {
            "yAxis": y_tbError,
            "yRange": [0, 1],
            "label": "TB error",
            "plotType": "scatter",
            "extra": {"s": 5},
        },
        {
            "yAxis": y_tbErrorRate,
            "yRange": [0, 1.1],
            "label": "TB error rate",
            "plotType": "plot",
            "yTicks": [x / 10 for x in range(11)],
        },
        {
            "yAxis": y_tbErrorRateWindow,
            "yRange": [0, 1.1],
            "label": "TB error rate (sliding window)",
            "plotType": "plot",
            "yTicks": [x / 10 for x in range(11)],
        },
    ]
    for i, entry in enumerate(plot_entries):
        plotFunc = getattr(axis[i][1], entry["plotType"])
        plotFunc(
            x_axis,
            entry["yAxis"],
            label=entry["label"],
            **({} if "extra" not in entry else entry["extra"]),
        )
        if "yTicks" in entry:
            axis[i][1].set_yticks(entry["yTicks"])
            axis[i][1].grid()
        axis[i][1].set_ylim(entry["yRange"])
        axis[i][1].legend()
    for i, interferer in enumerate(app_state):
        x_axis = list(map(lambda x: int(x[0]), app_state[interferer]))
        y_axis = list(map(lambda x: not bool(x[1]), app_state[interferer]))

        axis[1 + 2 * i][0].step(x_axis, y_axis, label=f"Interferer {interferer}")
        axis[1 + 2 * i][0].legend()

    for i, ue in enumerate(reversed(ueCqiManagerToUeId)):
        x_axis = list(map(lambda x: int(x[0]), cqi_measurements[ue]))

        for j in range(len(cqi_measurements[ue][0][1])):
            y_axis = list(map(lambda x: int(x[1][j]) + 16 * j, cqi_measurements[ue]))
            axis[i * 2][0].plot(x_axis, y_axis, label=f"UE {ueCqiManagerToUeId[ue]} SB {j}")
        axis[i * 2][0].legend()

    axis[6][1].plot(
        list(map(lambda x: int(x[0]), thr_measurements)),
        list(map(lambda x: float(x[1] if x[1] else 0.0), thr_measurements)),
        label=f"Throughput (Mbps)",
    )
    axis[6][1].set_yticks([x for x in range(0, 450, 50)])
    axis[6][1].legend()

    fig.suptitle(testCase)
    axis[0][0].title.set_text("Interferers and sub-band CQI")
    axis[0][1].title.set_text("Measurements of interfered UE0")
    output_file_name = testCase
    output_file_name = output_file_name.replace("\n", "__")
    output_file_name = output_file_name.replace(", ", "__")
    output_file_name = output_file_name.replace("=", "-")
    output_file_name = output_file_name.replace(" interference", "")
    output_file_name = output_file_name.replace("3gpp sub-band CQI clamping", "3gppClamping")
    output_file_name = output_file_name.replace("CSI feedback source", "CsiSource")
    output_file_name = output_file_name.replace("MCS computation based on", "McsSource")
    output_file_name = output_file_name.replace("Average allocated RBG", "avg")
    output_file_name = output_file_name.replace("CSI-RS", "CsiRs")
    output_file_name = output_file_name.replace("CSI-IM", "CsiIm")
    output_file_name = output_file_name.replace(" ", "")
    output_file_name += ".png"
    fig.savefig(output_file_name)


# For each CSI test case, plot the resulting figure
for testCase in contents.keys():
    plot_test_case(testCase, contents)
