#!/bin/sh
# $Id: buildconf.sh,v 1.14 2002-08-02 08:20:31 adam Exp $
set -x
aclocal
libtoolize --automake --force 
automake -a 
autoconf
if [ -f config.cache ]; then
	rm config.cache
fi
