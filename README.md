<!--
Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)

SPDX-License-Identifier: GPL-2.0-only
-->

[![REUSE status](https://api.reuse.software/badge/gitlab.com/cttc-lena/nr)](https://api.reuse.software/info/gitlab.com/cttc-lena/nr)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.7780747.svg)](https://doi.org/10.5281/zenodo.7780747)

# 3GPP NR ns-3 module #

This is an [ns-3](https://www.nsnam.org "ns-3 Website") NR module for the
simulation of 3GPP NR non-standalone cellular networks. ns-3 is used as a base
simulator, on top of which can be added our NR module as plug-in.

## ns-3 + NR prerequisites

### ns-3 prerequisites:

Make sure to install all [ns-3 prerequisites](https://www.nsnam.org/docs/installation/html/quick-start.html#prerequisites)

### NR prerequisites:

Install libc6-dev (it provides `semaphore.h` header file).  The Ubuntu
package name is:

```
sudo apt-get install libc6-dev
```

Install sqlite (enables optional examples `lena-lte-comparison`,
`cttc-nr-3gpp-calibration` and `cttc-realistic-beamforming`):

```
apt-get install sqlite sqlite3 libsqlite3-dev
```

Install eigen3 (enables optional MIMO features):

```
apt-get install libeigen3-dev
```

Notice that ns-3 and nr prerequisites are required to use all nr features.
Otherwise, you will get a warning at configuration time
and/or an error message during compilation or execution.

## ns-3 + nr installation

###  Download ns-3:

```
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev
```

### Download the NR module:

```
cd contrib
git clone https://gitlab.com/cttc-lena/nr.git
```

Notice that since these are two independent git repositories, when you run
`git status` inside of the ns-3, you will notice that the contrib/nr
directory will be listed as "Untracked files". This is normal.

### Switch to the latest NR release branch:

Checkout the latest NR release branch (usually the branch with the highest version
number, to list git branches run `git branch -r`). For example, for NR Release v3.0 run:

```
cd nr
git checkout 5g-lena-v3.0.y
```

### Switch to the recommended ns-3 release branch:

Check in the [NR RELEASE_NOTES.md Supported platforms](https://gitlab.com/cttc-lena/nr/-/blob/master/RELEASE_NOTES.md#supported-platforms) which is the recommended ns-3 release, and then check out the corresponding ns-3 release branch.
For example, if the NR RELEASE_NOTES.md indicates that the recommended ns-3 release is ".41" you can run:

```
cd ../..
git checkout -b ns-3.41-branch ns-3.41
```

For a quicker reference we provide a table with the supported versions of ns-3-dev
for each NR release.

| NR version     | ns-3 version | Build system  | Release date       |
| :------------: | :-----------:| :-----------: |--------------------|
| 5g-lena-v3.0.y | ns-3.41      | cmake         | February 16, 2024  |
| 5g-lena-v2.6.y | ns-3.40      | cmake         | November 30, 2023  |
| 5g-lena-v2.5.y | ns-3.39      | cmake         | July 26, 2023      |
| 5g-lena-v2.4.y | ns-3.38      | cmake         | April 5, 2023      |
| 5g-lena-v2.3.y | ns-3.37      | cmake         | November 23, 2022  |
| 5g-lena-v2.2.y | ns-3.36.1    | cmake         | June 03, 2022      |
| 5g-lena-v2.1.y | ns-3.36      | cmake         | May 06, 2022       |
| 5g-lena-v2.0.y | ns-3.36      | cmake         | April 21, 2022     |
| 5g-lena-v1.3.y | ns-3.35      | waf           | April 7, 2022      |
| 5g-lena-v1.2.y | ns-3-dev     | waf           | June 4, 2021       |
| 5g-lena-v1.1.y | ns-3-dev     | waf           | March 2, 2021      |
| 5g-lena-v1.0.y | ns-3-dev     | waf           | September 16, 2020 |
| 5g-lena-v0.4.y | ns-3-dev     | waf           | February 13 2020   |
| 5g-lena-v0.3.y | ns-3-dev     | waf           | August 27 2019     |
| 5g-lena-v0.2.y | ns-3-dev     | waf           | February 1 2019    |


### Test ns-3 + nr installation:

Let's configure the ns-3 + NR project:

```
./ns3 configure --enable-examples --enable-tests
```

In the output you should see: `SQLite support: ON`.

If that is not the case, return to "ns-3 and NR prerequisites" section, and install all prerequisites. After the installation of the missing packages run again `./ns3 configure --enable-tests --enable-examples`.

To compile the ns-3 with NR you can run the following command:

```
./ns3 build
```

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is the case, _Welcome to the NR world !_

Further instructions on how to run the examples are given in the [Getting Started instructions](https://cttc-lena.gitlab.io/nr/html/getting-started.html).

## Upgrading 5G-LENA

We assume that your work lives in a separate branch, and that the 'master'
branch of the NR repository is left untouched as the first time you downloaded
it. If it is not the case, then please move all your work in a separate branch.

A vanilla 'master' branch can be updated by simply running:

```
cd ns-3-dev/contrib/nr    # or src/nr if the module lives under src/
git checkout master
git pull
```

## Documentation

We maintain two sources of documentation:

1. [The user manual](https://cttc-lena.gitlab.io/nr/nrmodule.pdf) describes the models and their assumptions; as
we developed the module while the 3GPP standard was not fully available, some parts
are not modeling precisely the procedures as indicated by the standard.

2. [The Doxygen](https://cttc-lena.gitlab.io/nr/html/), you will find details about design
and user usage of any class of the module, as well as description and
images for the examples and the tests.



### Building documentation

If you would like to build the documentation on your own, you can follow the
instructions from this section.

- To build the user manual, navigate to the nr folder and then:

```
cd doc
make latexpdf
```

And you will find the PDF user manual in the directory build/latex. Please note
that you may have to install some requirements to build the documentation; you
can find the list of packages for any Ubuntu-based distribution in the file
`.gitlab-ci.yml`.

- To build the doxygen documentation, please do from the nr folder:

```
cd doc
doxygen doxygen.conf
```

You will find the doxygen documentation inside `doc/doc/html/`.

## Contributions are welcome!

As you may know, to fund, design, develop and then maintain an
open source software for a novel communication technology is a
challenging and time expensive task. For this, we would like to
foster collaborations with researchers and companies around the globe:

- If you identify a bug, please let us know through the Gitlab issue page;
- If you have a development plan that you can share, please get in touch with us.
We may be able to provide useful suggestions with your design and then maybe your
contribution can be more integrated more efficiently and be useful to let the project grow;
- If you plan to share your code, as the GPLv2 permits, please help us to
integrate it so that the work you have done does not become outdated
and then impossible to merge;
- The more we are, the better we can do!

## Features

To see the features, please go to the [official webpage](https://5g-lena.cttc.es/features/).

## Papers

An updated list of published papers that are based on the outcome of this
module is available
[here](https://5g-lena.cttc.es/papers/).

## About

The [OpenSim](https://www.cttc.cat/open-simulations-opensim/) research group in CTTC is a group of highly skilled researchers, with expertise in the area of cellular networks, O-RAN, radio resource management, ML/AI based network management, with a focus on the following research lines:

- Developing models, algorithms, and architectures for next-generation virtualized open radio access networks
- Designing, implementing, validating, and evaluating 5G and beyond extensions in ns-3, including licensed/unlicensed/shared-based access and vehicular communications
- Deriving 3GPP/IEEE technologies coexistence and spectrum sharing strategies
- Designing radio resource, interference, and spectrum management techniques.

## Collaborations

Contact us if you think that we could collaborate! We are interested in research projects with companies or other research institutions. Our [OpenSim](https://www.cttc.cat/open-simulations-opensim/) research group has gained a vast experience in industrial research projects through many successful projects with some of the top companies in the telecom industry. We also organize secondments for excellent and motivated MSc/PhD students. We can organize tutorials for the academy or industry.

## Authors ##

In alphabetical order:

- Zoraze Ali
- Biljana Bojovic
- Lorenza Giupponi
- Katerina Koutlia
- Sandra Lagen
- Natale Patriciello

Inspired by [mmWave module by NYU/UniPD](https://github.com/nyuwireless-unipd/ns3-mmwave)

## License ##

This software is licensed under the terms of the GNU GPLv2, as like as ns-3.
See the LICENSE file for more details.
