#!/bin/sh
# $Id: buildconf.sh,v 1.5 2001-10-23 22:39:53 adam Exp $
aclocal || exit 1
libtoolize --automake --force || exit 2
automake -a || exit 3
autoconf || exit 4
if [ -f config.cache ]; then
	rm config.cache
fi
util/cvs-date.tcl include/yaz/yaz-date.h
