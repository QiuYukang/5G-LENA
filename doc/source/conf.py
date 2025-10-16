# Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
# %SPDX-License-Identifier: GPL-2.0-only

import os
import sys

sys.path.insert(0, os.path.abspath("extensions"))

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.doctest",
    "sphinx.ext.todo",
    "sphinx.ext.coverage",
    "sphinx.ext.imgmath",
    "sphinx.ext.ifconfig",
    "sphinx.ext.autodoc",
    "sphinx.ext.autosectionlabel",
]

latex_engine = "xelatex"
latex_elements = {
    "preamble": r"""
                 \usepackage{amsmath}
                 """
}
todo_include_todos = True
templates_path = ["_templates"]
source_suffix = ".rst"
master_doc = "nr-module"
exclude_patterns = []
add_function_parentheses = True
numfig = True
# add_module_names = True
# modindex_common_prefix = []
html_theme = "sphinx_rtd_theme"
html_css_files = [
    "custom.css",
]
project = "NR Module"
copyright = "2025"
author = "OpenSim CTTC/CERCA"

version = "4.1.1"
release = "4.1.1"
