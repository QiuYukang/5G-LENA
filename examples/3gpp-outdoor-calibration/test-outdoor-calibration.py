#! /usr/bin/env python3

# Copyright (c) 2025 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# SPDX-License-Identifier: GPL-2.0-only

import glob
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from statistics import mean
from types import SimpleNamespace

from scipy.stats import combine_pvalues, ttest_1samp

NUM_RUNS = 10
num_ue = 10
OUTPUT_FILENAME = "stdout"  # This is just an output text file
CURR_PATH = os.path.abspath(os.path.dirname(__file__))
NR_PATH = os.path.abspath(CURR_PATH + "/../..")
NS3_PATH = os.path.abspath(NR_PATH + "/../..")


REFERENCE_VALUES_PER_SIMULATION_CONFIG = [
    # Scenario 1
    (
        # Config
        {
            "trafficScenario": 0,  # just to make validation happy, but unused
            "configurationType": "calibrationConf",
            "technology": "NR",
            "nrConfigurationScenario": "DenseAmimoIntel",
            "bfMethod": "KroneckerQuasiOmniBeamforming",
            "errorModelType": "ns3::NrEesmIrT1",
            "operationMode": "TDD",
            "numRings": 1,
            "ftpLambda": 1.7,
            "ueNumPergNb": num_ue,
            "appGenerationTime": 10,
            "pattern": "F|F|F|F|UL",
        },
        {
            "meanUPT": 41.44,
            "medianUPT": 45.45,
        },
    ),
]
# Create a results directory
nr_parent = os.path.dirname(NR_PATH).removeprefix(NS3_PATH)
CAMPAIGN_PATH = f"{NS3_PATH}/build/{nr_parent}/nr/examples/3gpp-outdoor-calibration/"
os.makedirs(CAMPAIGN_PATH, exist_ok=True)
del nr_parent


def create_test_cases():
    # Create a list with all the parameter combinations to simulate in the campaign
    test_cases = []
    for config, _ in REFERENCE_VALUES_PER_SIMULATION_CONFIG:
        for rngRun in range(1, NUM_RUNS + 1):
            test_cases.append({"RngRun": rngRun})
            test_cases[-1].update(config)
    return test_cases


def get_tag_from_config(config):
    return (
        "-".join(list(map(lambda x: f"{x[0]}_{x[1]}", config.items())))
        .replace("::", "__")
        .replace("|", "")
        .replace('"', "")
        .replace("Beamforming", "bf")
        .replace("operationMode", "opMode")
        .replace("appGenerationTime", "appGenTime")
        .replace("Configuration", "Conf")
        .replace("technology", "tech")
        .replace("Scenario", "Scen")
        .replace("configurationType", "confType")
        .replace("calibrationConf", "calibConf")
    )


def dispatch_simulation(*args):
    # Create a subdirectory for simulation based on run
    simulation_config_tag = get_tag_from_config(args[0])
    output_directory = os.path.join(CAMPAIGN_PATH, simulation_config_tag, str(args[0]["RngRun"]))
    output_file_path = os.path.join(output_directory, OUTPUT_FILENAME)
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    # If the output file does not exist, run a simulation to get it
    if not os.path.exists(output_file_path):
        # dict to args
        simulation_args = " ".join(list(map(lambda x: f"--{x[0]}={x[1]}", args[0].items())))
        simulation_args.replace("=True", "=1").replace("=False", "=0")
        # run sim
        sim_cmd = f"{sys.executable} ns3 run cttc-nr-3gpp-calibration-user --cwd={output_directory} -- {simulation_args}"
        sim_cmd = re.findall(r'(?:".*?"|\S)+', sim_cmd)

        env = os.environ.copy()
        env["OMP_NUM_THREADS"] = "1"
        try:
            with open(output_file_path, "w") as f:
                res = subprocess.run(sim_cmd, cwd=NS3_PATH, env=env, stdout=f, stderr=f)
        except Exception as e:
            return f"RngRun {args[0]['RngRun']} failed"
        if res.returncode != 0:
            return f"RngRun {args[0]['RngRun']} failed"
        return None


def retrieve_metrics_from_output_files(output_filenames):
    # Metrics we are going to collect as text post-processed by the calibration example

    metrics_patterns = {
        "meanUPT": r"Mean user perceived throughput:\s*([0-9.]+)",
        "95pctUPT": r"95tile UPT:\s*([0-9.]+)",
        "medianUPT": r"Median UPT:\s*([0-9.]+)",
        "5pctUPT": r" 5tile UPT:\s*([0-9.]+)",
    }

    # Open each file, then for each metric, extract results with regex matching
    results = {"meanUPT": [], "medianUPT": [], "95pctUPT": [], "5pctUPT": []}
    for output_file in output_filenames:
        with open(output_file, "r") as f:
            file_contents = f.read()
            for metric, pattern in metrics_patterns.items():
                match = re.search(pattern, file_contents)
                if match:
                    results[metric].append(float(match.group(1)))
                else:
                    print(f"{metric} not found in {output_file}")
    return results


def main():
    # Create test cases
    test_cases = create_test_cases()

    # Ensure the program is built before running
    subprocess.run(
        [sys.executable, "ns3", "build", "cttc-nr-3gpp-calibration-user"],
        cwd=NS3_PATH,
        capture_output=True,
        env=os.environ.copy(),
    )

    # Execute simulations in parallel
    num_threads = min(20, os.cpu_count() - 1)

    # If we have psutil available, we actually make a sane estimate based on memory
    try:
        import psutil

        # We know ring 1 has 21 cells with 10 UEs each use 6GB
        memory_required = 6 * 1024 * 1024 * 1024
        num_threads = psutil.virtual_memory().available // memory_required
    except ModuleNotFoundError:
        pass

    exec_result = None
    with ThreadPoolExecutor(max_workers=num_threads) as pool:
        # For debugging purposes
        # exec_result = []
        # for test_case in test_cases:
        # exec_result.append(dispatch_simulation(test_case))
        exec_result = pool.map(dispatch_simulation, test_cases)
        exec_result = list(filter(lambda x: x is not None, exec_result))

    if exec_result:
        for res in exec_result:
            print(res)
            exit(-1)

    # List output files
    output_filenames = glob.glob(f"{CAMPAIGN_PATH}/**/{OUTPUT_FILENAME}", recursive=True)

    # This is where the actual regression test begins
    failed_calibration_log = ""
    for config, reference_values in REFERENCE_VALUES_PER_SIMULATION_CONFIG:
        simulation_config_tag = get_tag_from_config(config)
        # Filter output files to just those containing the tag corresponding to the reference values
        # being tested
        config_output_filenames = list(
            filter(lambda x: simulation_config_tag in x, output_filenames)
        )

        # Extract results from simulation output files
        results = retrieve_metrics_from_output_files(config_output_filenames)
        # We perform a t-test to evaluate if our simulation results match the expected 3GPP reference values.
        # The null hypothesis assumes our results follow a t-distribution with mean equal to the 3GPP reference values.
        # A p-value less than 0.05 indicates statistical significance, meaning our simulation results
        # significantly differ from the 3GPP reference values (rejecting the null hypothesis).
        # Note: This statistical approach has limitations. While a p-value can indicate when results
        # differ significantly from reference values, it cannot definitively prove they are equal.
        # Given the limited statistical information available in the 3GPP study, this is our best
        # available method for comparison.

        for metric in results:
            if metric not in reference_values:
                continue
            ttest_result = ttest_1samp(
                results[metric],
                reference_values[metric],
            )

            if ttest_result.pvalue < 0.05:
                failed_calibration_log += (
                    f"{simulation_config_tag} failed calibration at {metric} "
                    f"with p-value {ttest_result.pvalue:.3f} and "
                    f"p-statistic {ttest_result.statistic:.3f}: "
                    f"got {mean(results[metric]):.3f} while expecting {reference_values[metric]:.3f}\n"
                )
    if failed_calibration_log:
        print(failed_calibration_log)
        return -1
    else:
        print("Calibration passed")
        return 0


if __name__ == "__main__":
    exit(main())
