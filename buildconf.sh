#!/bin/sh
# $Id: buildconf.sh,v 1.10 2002-03-16 11:46:18 adam Exp $
aclocal 
libtoolize --automake --force 
automake -a 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi
util/cvs-date.tcl include/yaz/yaz-date.h
