#!/bin/sh
# $Id: buildconf.sh,v 1.13 2002-07-25 12:52:52 adam Exp $
aclocal
libtoolize --automake --force 
automake -a 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi
