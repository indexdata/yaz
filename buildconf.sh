#!/bin/sh
# $Id: buildconf.sh,v 1.3 2001-05-16 07:37:39 adam Exp $
aclocal || exit 1
libtoolize --force >/dev/null 2>&1 || exit 2
automake -a >/dev/null 2>&1 || exit 3
autoconf || exit 4
if [ -f config.cache ]; then
	rm config.cache
fi
util/cvs-date.tcl include/yaz/yaz-date.h
