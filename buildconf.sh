#!/bin/sh
# $Id: buildconf.sh,v 1.15 2003-05-06 11:37:18 adam Exp $
set -x
aclocal
libtoolize --automake --force 
automake --add-missing 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi
