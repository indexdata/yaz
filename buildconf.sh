#!/bin/sh
# $Id: buildconf.sh,v 1.6 2001-10-23 22:44:32 adam Exp $
aclocal || exit 1
libtoolize --automake --force || exit 2
automake -a >/dev/null 2>&1 || exit 3
autoconf || exit 4
if [ -f config.cache ]; then
	rm config.cache
fi
util/cvs-date.tcl include/yaz/yaz-date.h
