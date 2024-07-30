<!--
Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)

SPDX-License-Identifier: GPL-2.0-only
-->
# ns-3 NR module with V2X extensions

This is an [ns-3](https://www.nsnam.org "ns-3 Website") NR module for the simulation of NR V2X.
ns-3 is used as a base, on top of which we add NR module with V2X extensions as plug-in.

## License ##

This software is licensed under the terms of the GNU GPLv2, as like as ns-3.
See the LICENSE file for more details.

## Features

To see the features, please go to the [official webpage](https://5g-lena.cttc.es/features/).

## Papers

An updated list of published papers that are based on the outcome of this
module is available
[here](https://5g-lena.cttc.es/papers/).

## About

The [OpenSim](https://www.cttc.cat/open-simulations-opensim/) research group in CTTC is a group of highly skilled researchers,
with expertise in the area of cellular networks, O-RAN, radio resource management,
ML/AI based network management, with a focus on the following research lines:

- Developing models, algorithms, and architectures for next-generation virtualized open radio access networks
- Designing, implementing, validating, and evaluating 5G and beyond extensions in ns-3, including licensed/unlicensed/shared-based access and vehicular communications
- Deriving 3GPP/IEEE technologies coexistence and spectrum sharing strategies
- Designing radio resource, interference, and spectrum management techniques.

## About NR V2X

The implementation of NR V2X is divided between ns-3 LTE (RLC and above) and
5G-LENA NR (MAC and PHY) modules, and it is contained in separate branches.
Therefore, to be able to use this code one has to use CTTC customized LTE module
of ns-3, and a specific branch in the nr module.

## ns-3 + nr prerequisites

### ns-3 prerequisites:

Make sure to install all [ns-3 prerequisites](https://www.nsnam.org/docs/installation/html/quick-start.html#prerequisites)

### nr prerequisites: {#prerequisites}

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

## Downloading the NR V2X development version

### 1. Download ns-3 with extensions for V2X:

```
git clone https://gitlab.com/cttc-lena/ns-3-dev.git
```

### 2.  Switch to the ns-3 development branch with V2X extensions:

```
cd ns-3-dev
$ git checkout -b v2x-lte-dev origin/v2x-lte-dev
```

### 3. Download nr with extensions for V2X:

```
cd contrib
git clone https://gitlab.com/cttc-lena/nr.git
```

### 4. Switch to the development branch with V2X extensions:

```
cd nr
git checkout -b nr-v2x-dev origin/nr-v2x-dev
```

## Downloading an NR V2X release

Alternatively, one can use a released version.  In the same way
as the development version, the selected ns-3 branch release must
be paired with the same released version of nr's v2x branch.

To check out the correct tags, consult the following table:

| NR V2X git branch | ns-3 V2X git tag  | Build system| ns-3 version|
| ----------------- | ----------------- | ----------- | ----------- |
| v2x-v1.1          | ns-3-dev-v2x-v1.1 | cmake       | ns-3.42     |
| v2x-v1.0          | ns-3-dev-v2x-v1.0 | cmake       | ns-3.42     |
| v2x-v0.4          | ns-3-dev-v2x-v0.4 | cmake       | ns-3.41     |
| v2x-v0.3          | ns-3-dev-v2x-v0.3 | cmake       | ns-3.40     |
| v2x-v0.2          | ns-3-dev-v2x-v0.2 | cmake       | ns-3.36     |
| v2x-v0.1          | ns-3-dev-v2x-v0.1 | waf         | ns-3.35     |

### 1. Download ns-3 with extensions for V2X:

```
git clone https://gitlab.com/cttc-lena/ns-3-dev.git
```

### 2.  Switch to the desired ns-3 release branch with V2X extensions:

See above table; this will be of the form "ns3-dev-v2x-v#.#"

```
cd ns-3-dev
git checkout tags/ns-3-dev-v2x-v1.0 -b ns-3-dev-v2x-v1.0-branch
```

### 3. Download nr with extensions for V2X:

```
cd contrib
git clone https://gitlab.com/cttc-lena/nr.git
```

### 4. Switch to the desired release branch with V2X extensions:

See above table; this will be of the form "v2x-v#.#" and must match the version in 2) above.

```
cd nr
git checkout tags/v2x-v1.0 -b v2x-v1.0-branch
```

## Configuring and building NR V2X extensions

Let's configure the ns-3 + NR project with V2X extensions:

```
cd ../..
./ns3 configure --enable-tests --enable-examples
```

In the output you should see: `SQLite stats support: enabled`. If that is not the case, return to "ns-3 and NR prerequisites" section, and install all prerequisites. After the installation of the missing packages run again `./ns3 configure --enable-tests --enable-examples`.

In the output you should see:

```
SQLite support                : ON
Eigen3 support                : ON
```

If that is not the case, return to "ns-3 and nr prerequisites" section, and install all prerequisites.
After the installation of the missing packages run again `./ns3 configure --enable-tests --enable-examples`.

To compile the ns-3 with nr you can run the following command:

```
./ns3 build
```

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is the case, _Welcome to the NR V2X world !_

## Run NR V2X examples:

There are only two provided V2X examples, which can be run as follows:
cttc-nr-traffic-ngmn-mixed    cttc-nr-v2x-demo-simple       
nr-v2x-west-to-east-highway   ns2-mobility-trace  

```
$ ./ns3 run cttc-nr-v2x-demo-simple
$ ./ns3 run nr-v2x-west-to-east-highway
```

The `cttc-nr-v2x-demo-simple` example derives from the `cttc-nr-demo` downlink example
documented [here](https://cttc-lena.gitlab.io/nr/cttc-nr-demo-tutorial.pdf).

However, the configuration is a bit more involved than what can be observed in the downlink
tutorial program.

Note: when writing or modifying examples, be aware that
our code is stored under the ns3 namespace. You will either
need to use such namespace or add the prefix "ns3::" for
classes, structs and enums.

## Upgrading 5G-LENA

We assume that your work lives in a separate branch, and that the 'master' and
'nr-v2x-dev' and release branches of the NR repository are left untouched as
the first time you downloaded it.
If it is not the case, then please move all your work in a separate branch.

A vanilla 'master' and 'nr-v2x-dev' branches can be updated by simply running:

```
$ cd ns-3-dev/contrib/nr    # or src/nr if the module lives under src/
$ git checkout master
$ git pull
$ git checkout nr-v2x-dev
$ git pull
```
At each release NR V2X release, we will incorporate into the nr-v2x-dev branch all the work that
is meant to be released.

### Building NR V2X documentation

To build the NR V2X documentation on your own, you can follow the
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

## Collaborations

Contact us if you think that we could collaborate! We are interested in research projects with companies or other research institutions. Our [OpenSim](https://www.cttc.cat/open-simulations-opensim/) research group has gained a vast experience in industrial research projects through many successful projects with some of the top companies in the telecom industry. We also organize secondments for excellent and motivated MSc/PhD students. We can organize tutorials for the academy or industry.

## Authors ##

In alphabetical order:

- Zoraze Ali
- Biljana Bojovic
- Lorenza Giupponi
- Katerina Koutlia
- Sandra Lagen
- Tom Henderson
- Natale Patriciello

Inspired by [mmWave module by NYU/UniPD](https://github.com/nyuwireless-unipd/ns3-mmwave)

## License ##

This software is licensed under the terms of the [GNU GPLv2-only](https://spdx.org/licenses/GPL-2.0-only),
which matches that of ns-3.  See the LICENSE file for more details.
