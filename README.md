# 3GPP NR ns-3 module #

This is an [ns-3](https://www.nsnam.org "ns-3 Website") 3GPP NR module for the
simulation of NR non-standalone cellular networks.

## Installation for an authorized developer

We try to keep in sync with the latest advancements in ns-3-dev. However, in
between our upstreamed patched are submitted and accepted, there will be a time
in which it will be necessary to use a branch of ns-3-dev, which contains the
"soon-to-be-merged" patches. These will be always present in the "nr" branch of
our copy of [ns-3-dev](https://gitlab.cttc.es/ns3-new-radio/ns-3-dev/tree/nr).

To add it to your existing local git repo, you can do:

```
$ cd your-local-ns-3-dev-git
$ git remote add mirror-gitlab-cttc git@gitlab.cttc.es:ns3-new-radio/ns-3-dev.git
$ git fetch -p --all   # fetch all the branches and update the refs
$ git checkout nr      # checkout a local "nr" branch that points to mirror-gitlab-cttc/nr
```

Or you can clone it entirely (not recommended, except for new installations):

```
$ git clone git@gitlab.cttc.es:ns3-new-radio/ns-3-dev.git
```

Note that on a computer that is outside the CTTC intranet, you must use
the HTTPS address:
[https://gitlab.cttc.es/ns3-new-radio/ns-3-dev.git](https://gitlab.cttc.es/ns3-new-radio/ns-3-dev.git).

Then, your local git repo is ready to include our nr module:

```
$ cd src
$ git clone git@gitlab.cttc.es:ns3-new-radio/nr.git
$ cd ..
```

Again, if you are outside the CTTC intranet, you must use the HTTPS address:
[https://gitlab.cttc.es/ns3-new-radio/nr.git](https://gitlab.cttc.es/ns3-new-radio/nr.git).

Please note that the src/nr directory will be listed as "Untracked files" every
time you do a `git status` command. Ignore it, as the directory lives as an
independent module.

Then, let's configure and build:

```
$ CXX="ccache g++" ./waf configure --enable-examples --enable-tests --disable-python --disable-gtk -d debug
$ ./waf
```

We are using ccache, as compiling is costly and disk space is cheap. We also
disable python (an useless waste of time, and its bindings as well) and GTK
(source of plenty of valgrind errors). Then, use de `debug` configuration,
but remember that for running extensive campaigns, `optimized` configuration is
preferred.

Welcome to the NR world!

## Features

## Papers

## Future work

## Simulation campaigns

Simulation campaigns are stored in another repository, as they are not part of
our release. If you have the permissions, please check
[https://gitlab.cttc.es/ns3-new-radio/sim-campaigns](https://gitlab.cttc.es/ns3-new-radio/sim-campaigns).

## About

## Authors ##

In alphabetical order:

- Biljana Bojovic
- Lorenza Giupponi
- Sandra Lagen
- Natale Patriciello

Inspired by [mmWave module by NYU/UniPD] (https://github.com/nyuwireless-unipd/ns3-mmwave)

## License ##

This software is licensed under the terms of the GNU GPLv2, as like as ns-3.
See the LICENSE file for more details.
