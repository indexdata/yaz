#!/bin/sh
# $Id: buildconf.sh,v 1.16 2003-05-06 12:09:10 adam Exp $
set -x
aclocal
libtoolize --automake --force 
automake --add-missing 
automake --add-missing 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi
