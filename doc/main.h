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
/** \mainpage 5G-LENA

 * This is a documentation for 5G-LENA. To see high-level feature
 * overview, project goals or read the developer blog, head over to the
 * [project homepage](https://cttc-lena.gitlab.io/5g-lena-website/).
 *
 * \section mainpage-whats-new What's new?
 *
 * Curious about what was added or improved recently? Check out the \ref changes file.
 * We also maintain the \ref releasenotes in a separate file.
 *
 * \section mainpage-getting-started Getting started
 *
 * The best way to get started is to read the thorough
 * \ref getting-started "guide to download, build, install and start using 5G-LENA".
 *
 *\section mainpage-contributions Contributions
 *
 *If you want to contribute to 5G-LENA, if you spotted a bug, need a feature or
 * have an awesome idea, you can get a copy of the sources from Gitlab and start
 * right away! There is a guide about
 * [NR Coding style and best practices] (https://gitlab.com/cttc-lena/nr/-/blob/master/doc/development-guidelines.md)
 * which you should follow to keep the project as consistent and maintainable as possible.
 *
 *\section mainpage-other-links Users group and Bug/Issues Reporting
 *
 * For users Q&A about 5G-LENA module you can join our 5G-LENA users group.
 * For reporting an issue or a bug you should do it through NR Gitlab. Here are the links:
 *
 * - 5G-LENA users group: https://groups.google.com/g/5g-lena-users/
 * - NR module bug/issues reporting: https://gitlab.com/cttc-lena/nr/-/issues
 *
 *
 * \section mainpage-contact Contact & Support page
 *
 * See the [Contact & Support page](https://cttc-lena.gitlab.io/5g-lena-website/contact/)
 * on the project website for further information.
 *
 * \section mainpage-license License
 *
 * 5G-LENA is licensed under the GPLv2 license:
 *
 * >
 * >     Copyright (c) 2018 CTTC
 * >
 * >     This program is free software; you can redistribute it and/or modify
 * >     it under the terms of the GNU General Public License version 2 as
 * >     published by the Free Software Foundation;
 * >
 * >     This program is distributed in the hope that it will be useful,
 * >     but WITHOUT ANY WARRANTY; without even the implied warranty of
 * >     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * >     GNU General Public License for more details.
 * >
 * >     You should have received a copy of the GNU General Public License
 * >     along with this program; if not, write to the Free Software
 * >     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * >
 *
 */

/**
 * \dir examples
 * In this directory, we store some example scripts that can be useful
 * to learn how to configure, run, and get output from the simulations.
 */

/**
 * \dir model
 * In this directory, we store all implementation files for our 5G-LENA module.
 */

/**
 * \dir helper
 * In this directory, we store the implementation of the helper files. Use them
 * in your simulation to setup and connect all the 5G-LENA components.
 */

/**
 * \dir test
 * In this directory we store all the test suites for 5G-LENA.
 */

/**
 * \namespace ns3
 *
 * All our files are stored under the ns3 namespace. In your script, remember to
 * use such namespace or to use the prefix "ns3::" for the classes.
 */
