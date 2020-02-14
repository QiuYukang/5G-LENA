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
entire GPL header with the following SPDX identifier: `// GPL-2.0-only`.

Please note that we do not use the convention of putting authors, as we don't
have a clear definition of an author. Moreover, even if we had it, it would be
useless as our users write to the generic 5G-LENA email address, or open a bug
in the gitlab interface (and then we will assign it internally).

Header inclusion
----------------

In a `.h` file, please include only the headers strictly needed by the unit,
and forward-declare everything else. An example is the following:

```
#include "MyBossClass.h"
#include "Mac.h"

class Spectrum;
class Phy;

class MyPreciousClass : public MyBossClass // Cannot be forward-declared
{
public:
  MyPreciousClass ();

private:
  Ptr<Spectrum> m_spectrum;  //!< Pointer to Spectrum: can be forward-declared
  Mac m_mac;                 //!< Instance to Mac: cannot be forward-declared
  std::shared_ptr<Phy> m_phy;//!< Another pointer: Can be forward-declared
};
```

A typical cleaning task is to take one random `.h` file, and check that the
includes are really needed. If not, then, move the include to the `.cc` file,
and forward-declare the type in the header file. Of course, you may get
compiler errors: but we are sure you can fix them.

*Why the forward-declaration is so important?* Well, the reasons are two:

1) Reduce compile times;
2) Break cyclic references when two definitions both uses each other.
