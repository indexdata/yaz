## $Id: Makefile.am,v 1.6 2000-06-19 09:49:35 adam Exp $

lib_LTLIBRARIES = libyaz.la

bin_SCRIPTS = yaz-config

EXTRA_DIST = yaz-config.in

# If the line below is omitted the LINK variable is
# not set in Makefile.in and it will NOT work ..
EXTRA_libyaz_la_SOURCES=x.c

# No real sources
libyaz_la_SOURCES=

LDFLAGS=-version-info 0:0:0

libyaz_la_LIBADD=../odr/libodr.la ../$(ASNMODULE)/libasn.la \
 ../comstack/libcomstack.la ../server/libserver.la \
 ../zutil/libzutil.la ../util/libutil.la ../ccl/libccl.la \
 ../retrieval/libret.la $(ILLLIB)
