#!/bin/sh
# $Id: buildconf.sh,v 1.19 2004-04-30 19:34:05 adam Exp $
set -x
aclocal -I .
libtoolize --automake --force 
automake --add-missing 
automake --add-missing 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi

MESSAGE=" 

Build the Makefiles with the configure command.
  ./configure [--someoption=somevalue ...]

For help on options or configuring run
  ./configure --help

Build and install binaries with the usual
  make
  make check
  make install

Build distribution tarball with
  make dist

Verify distribution tarball with
  make distcheck

Or just build the Debian packages without configuring
  dpkg-buildpackage -rfakeroot

When building from a CVS checkout, you need these Debian tools:
  bison, docbook-utils, docbook, docbook-xml, docbook-dsssl, jade, jadetex,
  libxml2-dev, libssl-dev, libreadline4-dev, libwrap0-dev, any tcl
"
