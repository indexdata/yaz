#!/bin/sh
# $Id: buildconf.sh,v 1.4 2001-10-23 21:00:19 adam Exp $
set -x
libtoolize --force || exit 2
aclocal || exit 1
automake -a || exit 3
autoconf || exit 4
if [ -f config.cache ]; then
	rm config.cache
fi
util/cvs-date.tcl include/yaz/yaz-date.h
