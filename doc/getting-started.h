/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Natale Patriciello <natale.patriciello\gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/** \page getting-started Getting started
\brief Get started with 5G-LENA in matter of minutes.

\tableofcontents

The first thing to do is take some confidence with the ns-3 environment.
We cannot really help you here, but you can find on the Web a lot of information
and tutorials. So, we will download and build the 5G-LENA project, but then
we will point out some ns-3 tutorials before entering the NR domain.

\note
Many of these instructions are copied from the README file. If you find an
inconsistency, please open a support ticket!

\section getting-started-ns3 Download the ns-3 part of the 5G-LENA project

We try to keep in sync with the latest advancements in ns-3-dev. However, in
between our upstreamed patched are submitted and accepted, there will be a time
in which it will be necessary to use patches "on top of" of ns-3-dev. Technically,
these patches are maintained as a "branch" of the ns-3-dev development, and are
hosted internally at CTTC.

\note
If you don't have the permission to see the repository, it is probably due
to the fact that you did not requested it. Even though 5G LENA is GPLv2-licensed,
the access to the code is restricted.

\subsection download-ns3 Download a brand new ns-3-dev repository
To download a working copy of the ns-3-dev repository with the latest changes
to support the NR module, you can do the following:

\code{.sh}
$ git clone git@gitlab.com:cttc-lena/ns-3-dev.git
$ cd ns-3-dev
$ git checkout nr
\endcode

Provide your username and password when asked.

In case you are already using the git mirror of ns-3-dev, hosted at GitHub or GitLab,
add the branch that make ns-3-dev supporting the NR module is easy as doing:

\code{.sh}
$ cd your-local-ns-3-dev-git
$ git remote add mirror-gitlab-cttc git@gitlab.com:cttc-lena/ns-3-dev.git
$ git fetch -p --all   # fetch all the branches and update the refs
$ git checkout nr      # checkout a local "nr" branch that points to mirror-gitlab-cttc/nr
\endcode

\subsection test-ns3 Test the installation

To test the installation, after following one of the previous point, you can do
a simple configuration and compile test (more options for that later):

\code{.sh}
$ ./waf configure --enable-examples --enable-tests
$ ./waf
\endcode

A success for both previous commands indicates an overall success.

\section getting-started-nr Download the 5G-LENA core project

As a precondition to the following steps, you must have a working local git
repository. If that is the case, then, your local git repo is ready to include
our nr module:

\code{.sh}
cd src
git clone git@gitlab.com:cttc-lena/nr.git
cd ..
\endcode

Please note that the src/nr directory will be listed as "Untracked files" every
time you do a git status command. Ignore it, as the directory lives as an
independent module. As a result, we have now two parallel repository, but one
lives inside the other. We are working to be able to put nr inside the
contrib/ directory, as per standard ns-3 rules.

To test the resulting repository, let's configure the project again:
\code{.sh}
$ ./waf configure --enable-examples --enable-tests
\endcode

If the NR module is recognized correctly, you should see "nr" in the list of
built modules. If that is not the case, then most probably the previous
point failed. Otherwise, you could compile it:

\code{sh}
$ ./waf
\endcode

If that command returns successfully, Welcome to the NR world !

\section getting-started-tutorial ns-3 tutorials

If it is the first time you work with the ns-3 environment, we recommend to take
things slowly (but steady) and going forward through simple steps.
The ns-3 documentation <https://www.nsnam.org/documentation/> is divided into
two categories: the reference manual for the ns-3 core, and a separate model
library. We suggest to read the following:

- The ns-3 core tutorial: <https://www.nsnam.org/docs/tutorial/html/index.html>
- The ns-3 core manual: <https://www.nsnam.org/docs/manual/html/index.html>
- The LTE documentation: <https://www.nsnam.org/docs/models/html/lte.html>

*/
