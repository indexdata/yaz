#!/bin/sh
# $Id: buildconf.sh,v 1.2 2001-02-26 22:52:54 adam Exp $
aclocal || exit 1
libtoolize --force >/dev/null 2>&1 || exit 2
automake -a >/dev/null 2>&1 || exit 3
autoconf || exit 4
if [ -f config.cache ]; then
	rm config.cache
fi
