Development guidelines       {#devguidelines}
======================

When in doubt when writing something, please refer to this page to get
some hints. DISCLAIMER: These are not rules. You can override them using the
force (not git --force)

### Table of Contents

* [GPL header](#GPL-header)
* [Header inclusion](#header-inclusion)
* [API design](#api-design)

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

1. Reduce compile times;
2. Break cyclic references when two definitions both uses each other.

API design
----------

This is heavily inspired by [this article by M. Ettrich](https://doc.qt.io/archives/qq/qq13-apis.html).

An API is the interface of our code towards the other. As good people, we do not
want others to suffer... so, your new and shiny API must be:


* **minimal**: A minimal API is one that has as few public members per class
and as few classes as possible. This makes it easier to understand, remember,
debug, and change the API.

* **complete**: A complete API means the expected functionality should be there.
This can conflict with keeping it minimal. Also, if a member function is in
the wrong class, many potential users of the function won't find it.

* **simple**: As with other design work, you should apply the principle of
least surprise. Make common tasks easy. Rare tasks should be possible but not
the focus. Solve the specific problem; don't make the solution overly general
when this is not needed.

* **intuitive**: As with anything else on a computer, an API should be intuitive.
Different experience and background leads to different perceptions on what is
intuitive and what isn't. An API is intuitive if a semi-experienced user gets
away without reading the documentation, and if a programmer who doesn't know
the API can understand code written using it.

* **memorizable**: To make the API easy to remember, choose a consistent and
precise naming convention. Use recognizable patterns and concepts, and avoid
abbreviations.

* **readable**: Code is written once, but read (and debugged and changed)
many times. Readable code may sometimes take longer to write, but saves
time throughout the product's life cycle.

* **correct**: The code must do what the interface says. For example, if a class
has a public *Set\** method, it must be possible to call the method as many
times as the user wishes, expecting the method to setup everything else in a
correct way. If this action is not possible, then the value is not an Attribute
(a property) but is a characteristic, immutable, and hence has to be set in the
constructor.

For C++:

###### Pointers vs. References

Which is best for out-parameters, pointers or references?

```
void getHsv(int *h, int *s, int *v) const
void getHsv(int &h, int &s, int &v) const
```

Most C++ books recommend references whenever possible, according to the
general perception that references are "safer and nicer" than pointers.
In contrast, smart people tend to prefer pointers because they make the user
code more readable. Compare:

```
 color.getHsv(&h, &s, &v);
 color.getHsv(h, s, v);
```

Only the first line makes it clear that there's a high probability that h, s,
and v will be modified by the function call.

That said, compilers really don't like out parameters, so you should avoid
them in new APIs. Instead, return a small struct:

```
struct Hsv { int hue, saturation, value };
Hsv getHsv() const;
```

Lastly, don't think that Ptr<> class is equivalent to a pointer.

```
static void MyInit (Ptr<SomeClass> v)
{
    v = CreateObject<SomeClass> ();
}

int main ()
{
    Ptr<SomeClass> a;
    MyInit (a);

    std::cout << a->m_someValue << std::endl; // How to get a nice SEGFAULT

    return 0;
}
```

This introduces the next point...

###### Passing by const-ref vs. Passing by value

* If the type is bigger than 64 bytes, pass by const-ref.

* If the type has a non-trivial copy-constructor or a non-trivial destructor,
pass by const-ref to avoid executing these methods. *This includes smart pointers*.

* All other types should usually be passed by value.

Example:

```
void SetK2 (uint32_t k2);
void SetCategory (char cat);
void SetName(const std::string &name);
void SetAlarm (const Ptr<Time> &alarm); // const-ref is much faster than running copy-constructor and destructor
void SetAlarm (const Time &time);
```

and therefore:

```
static void MyInit (Ptr<SomeClass> *v)
{
    *v = CreateObject<SomeClass> ();
}

int main ()
{
    Ptr<SomeClass> a;
    MyInit (&a);  // See? a is going to be modified

    std::cout << a->m_someValue << std::endl; // Everything works!

    return 0;
}
```

###### Avoiding virtual functions

There is no much to say here; virtual functions are slow, and often are not
needed (see [this article](https://medium.com/humans-create-software/composition-over-inheritance-cb6f88070205)).

###### const correctness

Put const everywhere. Always. When something is broken, check why. If it makes
sense, then remove the const keyword.

For everything else, please read [here](https://wiki.qt.io/API_Design_Principles)
