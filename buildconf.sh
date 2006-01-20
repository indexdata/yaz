#!/bin/sh
# $Id: buildconf.sh,v 1.22 2006-01-20 10:34:50 adam Exp $
set -x
aclocal -I .
libtoolize --automake --force 
automake --add-missing 
automake --add-missing 
autoconf
set -
if [ -f config.cache ]; then
	rm config.cache
fi

enable_configure=false
enable_help=true
sh_flags=""
conf_flags=""
case $1 in
    -d)
	sh_flags="-g -Wall -ansi"
	enable_configure=true
	enable_help=false
	shift
	;;
    -c)
	sh_flags=""
	enable_configure=true
	enable_help=false
	shift
	;;
esac

if $enable_configure; then
    if test -n "$sh_flags"; then
	CFLAGS="$sh_flags" ./configure $*
    else
	./configure $*
    fi
fi
if $enable_help; then
    cat <<EOF

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
  autoconf, automake, bison, gcc, libtools,
  docbook-utils, docbook, docbook-xml, docbook-dsssl, jade, jadetex,
  libxml2-dev, libssl-dev, libreadline4-dev, libwrap0-dev, any tcl
EOF
fi
