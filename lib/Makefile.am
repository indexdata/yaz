## $Id: Makefile.am,v 1.10 2001-04-05 13:08:48 adam Exp $

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

LDFLAGS=-version-info 1:0:0

nmem.c:
	ln -s $(srcdir)/../util/nmem.c

statserv.c:
	ln -s $(srcdir)/../server/statserv.c

INCLUDES =-I$(srcdir)/../include -I$(srcdir)/../server  -D_REENTRANT=1 -DHAVE_PTHREAD_H=1

bin_SCRIPTS = yaz-config

libyaz_la_LIBADD=../odr/libodr.la \
 ../comstack/libcomstack.la ../server/libserver.la \
 ../zutil/libzutil.la ../util/libutil.la ../ccl/libccl.la \
 ../retrieval/libret.la $(EXTRALIB)

