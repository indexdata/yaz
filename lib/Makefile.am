## $Id: Makefile.am,v 1.12 2001-05-07 12:01:00 adam Exp $

if ISTHR
extra=libyazthread.la
endif

lib_LTLIBRARIES = libyaz.la $(extra)

# If the line below is omitted the LINK variable is
# not set in Makefile.in and it will NOT work ..
EXTRA_libyaz_la_SOURCES=x.c

# No real sources
libyaz_la_SOURCES=

libyazthread_la_SOURCES=nmem.c statserv.c

$(srcdir)/nmem.c: $(top_srcdir)/util/nmem.c
	cp $(top_srcdir)/util/nmem.c $(srcdir)

$(srcdir)/statserv.c: $(top_srcdir)/server/statserv.c
	cp $(top_srcdir)/server/statserv.c $(srcdir)

LDFLAGS=-version-info 1:0:0

INCLUDES =-I$(top_srcdir)/include -I$(top_srcdir)/server  -D_REENTRANT=1 -DHAVE_PTHREAD_H=1

bin_SCRIPTS = yaz-config

libyaz_la_LIBADD=../odr/libodr.la \
 ../comstack/libcomstack.la ../server/libserver.la \
 ../zutil/libzutil.la ../util/libutil.la ../retrieval/libret.la \
 $(EXTRALIB)

