#!/bin/sh
# $Id: buildconf.sh,v 1.11 2002-05-13 14:13:37 adam Exp $
aclocal 
libtoolize --automake --force 
automake -a 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi
#util/cvs-date.tcl include/yaz/yaz-date.h
