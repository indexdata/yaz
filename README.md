# YAZ toolkit

Copyright (C) 1995-2026 Index Data.
See the file LICENSE for details.

The primary output of the source here is the
[YAZ](https://www.indexdata.com/yaz) library, which contains support
functions for implementing the server or client role of Z39.50 and SRU.

## Documentation

For more information about YAZ refer to the documentation in sub
directory `doc` or
[online](https://www.indexdata.com/yaz/doc).

## Cloning

It's easiest to compile this software by using the source tar provided
for official releases.
Refer to ["release downloads"](https://ftp.indexdata.com/pub/yaz/).

If you want to clone and compile from Git, it's more complicated. Firstly, you need to clone with submodules.
You need autoconf tools - such
as autoconf, automake, libtool. For compilation besides the obvious
C compiler (gcc, clang) and `make` you also need xsltproc, tcl, docbook xml.
The `buildconf.sh` script creates `configure` and the
Makefile templates.

For Debian based systems, read `debian/control` and install
what's listed in `Build-Depends`. For RPM based systems, read
`yaz.spec` and what's listed in `BuildRequires`.

    $ git clone --recursive https://github.com/indexdata/yaz.git
    $ cd yaz
    $ ./buildconf.sh
    $ ./configure
    $ make
    $ make check

On Windows, use an MSVC command prompt and the settings in `win/makefile`:

    > cd win
    > nmake all
    > nmake check

The `check` target builds and runs the C tests listed in
`test/Makefile.am`, including their generated helpers and test variants.
It runs every test, prints a pass/fail summary, and fails if any test fails.
Test output and diagnostic logs are written in `test/`. Shell and Python
integration tests are not included in this target.

## Help

To get more information or assistance, send mail to info@indexdata.com
Or use the [YAZ discussions](https://github.com/indexdata/yaz/discussions)

