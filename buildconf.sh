#!/bin/sh
# $Id: buildconf.sh,v 1.7 2001-11-06 17:03:50 adam Exp $
aclocal 
libtoolize --automake --force || exit 2
automake -a  || exit 3
autoconf || exit 4
if [ -f config.cache ]; then
	rm config.cache
fi
util/cvs-date.tcl include/yaz/yaz-date.h
