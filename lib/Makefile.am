## $Id: Makefile.am,v 1.29 2003-04-17 23:24:00 adam Exp $

if ISTHR
extra=libyazthread.la
endif

if ISSSL
ssllib=libyazssl.la
endif

bin_SCRIPTS = yaz-config

lib_LTLIBRARIES = libyaz.la $(extra) $(ssllib) libyazmalloc.la

# If the line below is omitted the LINK variable is
# not set in Makefile.in and it will NOT work ..
EXTRA_libyaz_la_SOURCES=x.c

YAZ_VERSION_INFO=2:0:0

# No real sources
libyaz_la_SOURCES=
libyaz_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

libyazthread_la_LIBADD=nmem.lo statserv.lo eventl.lo
libyazthread_la_SOURCES=
libyazthread_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

libyazmalloc_la_LIBADD=xmalloc.lo
libyazmalloc_la_SOURCES=
libyazmalloc_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

libyazssl_la_LIBADD=tcpip.lo comstack.lo
libyazssl_la_SOURCES=
libyazssl_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

AM_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/server
THREADED_FLAGS = @CFLAGSTHREADS@
XMALLOC_FLAGS = -DTRACE_XMALLOC=2 

SSL_FLAGS= -I$(top_srcdir)/include $(SSL_DEFS) $(SSL_CFLAGS)

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

comstack.lo: $(top_srcdir)/comstack/comstack.c
	$(LTCOMPILE) $(SSL_FLAGS) -c $(top_srcdir)/comstack/comstack.c

comstack.o: $(top_srcdir)/comstack/comstack.c
	$(COMPILE) $(SSL_FLAGS) -c $(top_srcdir)/comstack/comstack.c

tcpip.lo: $(top_srcdir)/comstack/tcpip.c
	$(LTCOMPILE) $(SSL_FLAGS) -c $(top_srcdir)/comstack/tcpip.c

tcpip.o: $(top_srcdir)/comstack/tcpip.c
	$(COMPILE) $(SSL_FLAGS) -c $(top_srcdir)/comstack/tcpip.c

libyaz_la_LIBADD=../odr/libodr.la \
 ../comstack/libcomstack.la ../server/libserver.la \
 ../util/libutil.la ../ccl/libccl.la ../cql/libcql.la ../zutil/libzutil.la \
 ../ill/libill.la ../z39.50/libz39.50.la

