#!/bin/sh
# $Id: buildconf.sh,v 1.1 2001-02-21 09:03:34 adam Exp $
aclocal
libtoolize --force
automake -a
autoconf
