#!/bin/sh
# $Id: buildconf.sh,v 1.12 2002-05-13 18:34:53 adam Exp $
aclocal 
libtoolize --automake --force 
automake -a 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi
