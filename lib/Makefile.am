## $Id: Makefile.am,v 1.18 2001-11-19 20:43:39 adam Exp $

if ISTHR
extra=libyazthread.la
endif

LDFLAGS=-version-info 1:0:0

bin_SCRIPTS = yaz-config

lib_LTLIBRARIES = libyaz.la $(extra) libyazmalloc.la

# If the line below is omitted the LINK variable is
# not set in Makefile.in and it will NOT work ..
EXTRA_libyaz_la_SOURCES=x.c

# No real sources
libyaz_la_SOURCES=

libyazthread_la_LIBADD=nmem.lo statserv.lo eventl.lo
libyazthread_la_SOURCES=

libyazmalloc_la_LIBADD=xmalloc.lo
libyazmalloc_la_SOURCES=

THREADED_FLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/server @CFLAGSTHREADS@

XMALLOC_FLAGS = -DTRACE_XMALLOC=2 -I$(top_srcdir)/include

nmem.lo: $(top_srcdir)/util/nmem.c
	$(LTCOMPILE) $(THREADED_FLAGS) -c $(top_srcdir)/util/nmem.c

nmem.o: $(top_srcdir)/util/nmem.c
	$(COMPILE) $(THREADED_FLAGS) -c $(top_srcdir)/util/nmem.c

statserv.lo: $(top_srcdir)/server/statserv.c
	$(LTCOMPILE) $(THREADED_FLAGS) -c $(top_srcdir)/server/statserv.c

statserv.o: $(top_srcdir)/server/statserv.c
	$(COMPILE) $(THREADED_FLAGS) -c $(top_srcdir)/server/statserv.c

eventl.lo: $(top_srcdir)/server/eventl.c
	$(LTCOMPILE) $(THREADED_FLAGS) -c $(top_srcdir)/server/eventl.c

eventl.o: $(top_srcdir)/server/eventl.c
	$(COMPILE) $(THREADED_FLAGS) -c $(top_srcdir)/server/eventl.c

xmalloc.lo: $(top_srcdir)/util/xmalloc.c
	$(LTCOMPILE) $(XMALLOC_FLAGS) -c $(top_srcdir)/util/xmalloc.c

xmalloc.o: $(top_srcdir)/util/xmalloc.c
	$(COMPILE) $(XMALLOC_FLAGS) -c $(top_srcdir)/util/xmalloc.c


libyaz_la_LIBADD=../odr/libodr.la \
 ../comstack/libcomstack.la ../server/libserver.la \
 ../util/libutil.la ../retrieval/libret.la \
 $(EXTRALIB)

