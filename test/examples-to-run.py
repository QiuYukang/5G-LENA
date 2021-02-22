#! /usr/bin/env python
# -*- coding: utf-8 -*-
## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# A list of C++ examples to run in order to ensure that they remain
# buildable and runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run, do_valgrind_run).
#
# See test.py for more information.
cpp_examples = [
    ("cttc-3gpp-channel-example", "True", "True"),
    ("cttc-3gpp-channel-nums", "True", "True"),
    ("cttc-3gpp-channel-nums-fdm", "True", "True"),
    ("cttc-3gpp-channel-simple-fdm", "True", "True"),
    ("cttc-3gpp-channel-simple-ran", "True", "True"),
    ("cttc-3gpp-indoor-calibration", "True", "True"),
    ("cttc-nr-demo", "True", "True"),
    ("cttc-lte-ca-demo", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|DL|DL|S|UL|UL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|DL|DL|DL|S|UL|DL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|UL|DL|DL|DL|DL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|DL|DL|DL|DL|DL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|DL|DL|DL|DL|DL|DL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|UL|DL|S|UL|UL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|UL|DL|S|UL|UL|UL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"F|F|F|F|F|F|F|F|F|F|\"", "True", "True"),
    ("cttc-realistic-beamforming", "True", "True"),
    ("cttc-nr-notching", "True", "True"),
    ("cttc-fh-compression", "True", "True"),
    ("cttc-error-model", "True", "True"),
    ("cttc-error-model-amc", "True", "True"),
    ("cttc-error-model-comparison", "True", "True"),
    ("cttc-channel-randomness", "True", "True"),
    ("rem-beam-example", "True", "True"),
    ("rem-example", "True", "True"),
    ("lena-lte-comparison-campaign --trafficScenario=1 --simulator=LENA --technology=LTE --numRings=0 --ueNumPergNb=1 --calibration=false --freqScenario=0", "True", "True"),
    ("lena-lte-comparison-campaign --trafficScenario=1 --simulator=5GLENA --technology=LTE --numRings=0 --ueNumPergNb=1 --calibration=false --freqScenario=0", "True", "True"),
    ]

# A list of Python examples to run in order to ensure that they remain
# runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run).
#
# See test.py for more information.
python_examples = []
