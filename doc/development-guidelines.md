Development guidelines       {#devguidelines}
======================

When in doubt when writing something, please refer to this page to get
some hints. DISCLAIMER: These are not rules. You can override them using the
force (not git --force)

GPL header
----------

At the beginning of every file, please put:

```
/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
```
Please note that (1) in the future we may remove the (stupid) line that is
indicating to Emacs what is the style of the file, and (2) we may replace the
entire GPL header with the following SPDX identifier:

// GPL-2.0-only
