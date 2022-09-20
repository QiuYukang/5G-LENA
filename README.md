# 3GPP NR ns-3 module #

This is an [ns-3](https://www.nsnam.org "ns-3 Website") NR module for the
simulation of NR non-standalone cellular networks. ns-3 is used as a base 
simulator, on top of which can be added our NR module as plug-in.

## Brand new installation of ns-3 

To download a working copy of the ns-3, you can do the following:

```
git clone https://gitlab.com/nsnam/ns-3-dev.git
cd ns-3-dev
```

### Test the ns-3 installation
To test the installation, you can run:

```
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

## Brand new installation of the NR module

Once you have installed and tested the ns-3, you can install the nr module:

```
cd contrib
git clone https://gitlab.com/cttc-lena/nr.git
cd nr
```

Notice that we have two parallel git repositories, ns-3 and NR. If you run 
`git status` inside of the ns-3, you will notice that the contrib/nr 
directory will be listed as "Untracked files". This is normal.


### Checkout the latest NR release branch

Switch to the latest NR release branch. For example, for NR Relase 2.2 run 
from the NR folder:

```
git checkout 5g-lena-v2.2.y
```

### Checkout the recommended ns-3 release branch

`Check in the NR RELEASE_NOTES.md which is the recommended ns-3 release`, 
and then checkout the corresponding ns-3 release branch. 

For example, if the NR RELEASE_NOTES.md indicates that the recommended ns-3 
release is "36.1" you can run:

```
cd ../..
git checkout ns-3.36.1
```

### Test ns-3 + nr installation

Let's configure the ns-3 project again (after downloading NR and 
switching to the specific NR and ns-3 release branches):

```
./ns3 configure --enable-examples --enable-tests
```

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is not the case, then most probably the previous
point failed. 

To compile the ns-3 with NR you can run the following command:

```
./ns3
```

If that command returns successfully, _Welcome to the NR world !_


## ns-3 and NR prerequisites

Notice that sqlite development package and semaphore.h are required (otherwise
you will get an error, e.g: `fatal error: ns3/sqlite-output.h`). In this case
you should install libc6-dev:

```
sudo apt-get install libc6-dev
```

that will provide semaphore.h and/or sqlite:

```
apt-get install sqlite sqlite3 libsqlite3-dev
```

For more details, related to the prerequisites for ns-3 please visit: `https://www.nsnam.org/wiki/Installation#Ubuntu.2FDebian.2FMint`.
After the installation of the missing packages run again `./ns3 configure --enable-tests --enable-examples`.
You should see: `SQLite stats support: enabled`


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

To move to the latest NR release, if the latest release is 2.2 you will need to do
from the `nr` folder:  

```
git checkout 5g-lena-v2.2.y
```

Then, you should check in the NR RELEASE_NOTES.md what is the recommended ns-3 version, 
then move to the `ns-3` folder and run:

```
git checkout ns-3.*
```

Replace * by the specific ns-3 release number. Then you can follow the steps listed in 
section "Test ns3 + NR installation" to test your upgraded NR.

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
git submodule sync --recursive
git submodule update --init --recursive
python3 doc/m.css/documentation/doxygen.py doc/doxygen-mcss.conf --debug
```

You will find the doxygen documentation inside `doc/doc/html/`.
Please note that you may need to initialize the m.css submodule, and
to install some packages like python3.

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

To see the features, please go to the [official webpage](https://cttc-lena.gitlab.io/5g-lena-website/features/).

## Papers

An updated list of published papers that are based on the outcome of this
module is available
[here](https://cttc-lena.gitlab.io/5g-lena-website/papers/).

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
