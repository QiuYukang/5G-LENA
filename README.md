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

Then, your local git repo is ready to include our nr module:

```
$ git submodule add git@gitlab.cttc.es:ns3-new-radio/nr.git
$ cd ..
```

Then, configure and build. Welcome to the NR world!

PS: The submodule thing is still to be worked. Probably it will change.

## Features

## Papers

## Future work

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
