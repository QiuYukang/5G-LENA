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
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|DL|DL|S|UL|UL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|DL|DL|DL|S|UL|DL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|UL|DL|DL|DL|DL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|DL|DL|DL|DL|DL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|DL|DL|DL|DL|DL|DL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|UL|DL|S|UL|UL|DL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"DL|S|UL|UL|UL|DL|S|UL|UL|UL|\"", "True", "True"),
    ("cttc-nr-cc-bwp-demo --tddPattern=\"F|F|F|F|F|F|F|F|F|F|\"", "True", "True"),
    ]

# A list of Python examples to run in order to ensure that they remain
# runnable over time.  Each tuple in the list contains
#
#     (example_name, do_run).
#
# See test.py for more information.
python_examples = []
