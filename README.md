# 3GPP NR ns-3 module #

This is an [ns-3](https://www.nsnam.org "ns-3 Website") 3GPP NR module for the
simulation of NR non-standalone cellular networks. Ns-3 is used as a base,
on top of which we will add our module as plug-in (with limitations that will
be discussed below).

## Installation for an authorized developer

We try to keep in sync with the latest advancements in ns-3-dev. However, in
between our upstreamed patched are submitted and accepted, there will be a time
in which it will be necessary to use patches "on top of" of ns-3-dev. Technically,
these patches are maintained as a "branch" of the ns-3-dev development, and are
hosted internally at CTTC. The repository is
[ns-3-dev](https://gitlab.cttc.es/ns3-new-radio/ns-3-dev/tree/nr).

### Brand new installation of ns-3-dev repository

To download a working copy of the ns-3-dev repository with the latest changes
to support the NR module, you can do the following:

```
$ git clone git@gitlab.cttc.es:ns3-new-radio/ns-3-dev.git
```

Note that on a computer that is outside the CTTC intranet, you must use
the HTTPS address:
[https://gitlab.cttc.es/ns3-new-radio/ns-3-dev.git](https://gitlab.cttc.es/ns3-new-radio/ns-3-dev.git)
instead of the address `git@gitlab.cttc.es:ns3-new-radio/ns-3-dev.git`.
Provide your username and password when asked.

### Adding the repository to an existing installation

In case you are already using the git mirror of ns-3-dev, hosted at GitHub or GitLab,
add the branch that make ns-3-dev supporting the NR module is easy as doing:

```
$ cd your-local-ns-3-dev-git
$ git remote add mirror-gitlab-cttc git@gitlab.cttc.es:ns3-new-radio/ns-3-dev.git
$ git fetch -p --all   # fetch all the branches and update the refs
$ git checkout nr      # checkout a local "nr" branch that points to mirror-gitlab-cttc/nr
```

Again, use the HTTPS address if you are outside the CTTC intranet. 


### Test the installation
To test the installation, after following one of the previous point, you can do
a simple configuration and compile test (more options for that later):

```
$ ./waf configure --enable-examples --enable-tests
$ ./waf
```

A success for both previous commands indicates an overall success.

### Brand new installation of the NR module

As a precondition to the following steps, you must have a working local git
repository. If that is the case, then, your local git repo is ready to include
our nr module:

```
$ cd src
$ git clone git@gitlab.cttc.es:ns3-new-radio/nr.git
$ cd ..
```

Again, if you are outside the CTTC intranet, you must use the HTTPS address:
[https://gitlab.cttc.es/ns3-new-radio/nr.git](https://gitlab.cttc.es/ns3-new-radio/nr.git).

Please note that the src/nr directory will be listed as "Untracked files" every
time you do a `git status` command. Ignore it, as the directory lives as an
independent module. As a result, we have now two parallel repository, but one
lives inside the other.

### Test the NR installation

Let's configure the project:

```
$ ./waf configure --enable-examples --enable-tests
```

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is not the case, then most probably the previous
point failed. Otherwise, you could compile it:

```
$ ./waf
```

If that command returns successfully, Welcome to the NR world !

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
