## $Id: Makefile.am,v 1.17 2001-11-19 09:16:37 adam Exp $

if ISTHR
extra=libyazthread.la
endif

lib_LTLIBRARIES = libyaz.la $(extra) libyazmalloc.la

# If the line below is omitted the LINK variable is
# not set in Makefile.in and it will NOT work ..
EXTRA_libyaz_la_SOURCES=x.c

# No real sources
libyaz_la_SOURCES=

libyazthread_la_SOURCES=nmem.c statserv.c eventl.c

libyazmalloc_la_SOURCES=xmalloc.c

$(srcdir)/nmem.c: $(top_srcdir)/util/nmem.c
	cp $(top_srcdir)/util/nmem.c $(srcdir)

$(srcdir)/statserv.c: $(top_srcdir)/server/statserv.c
	cp $(top_srcdir)/server/statserv.c $(srcdir)

$(srcdir)/eventl.c: $(top_srcdir)/server/eventl.c
	cp $(top_srcdir)/server/eventl.c $(srcdir)

$(srcdir)/xmalloc.c: $(top_srcdir)/util/xmalloc.c
	cp $(top_srcdir)/util/xmalloc.c $(srcdir)

LDFLAGS=-version-info 1:0:0

AM_CFLAGS=-DTRACE_XMALLOC=2 -I$(top_srcdir)/include -I$(top_srcdir)/server @CFLAGSTHREADS@

bin_SCRIPTS = yaz-config

libyaz_la_LIBADD=../odr/libodr.la \
 ../comstack/libcomstack.la ../server/libserver.la \
 ../util/libutil.la ../retrieval/libret.la \
 $(EXTRALIB)

