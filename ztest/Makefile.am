## $Id: Makefile.am,v 1.7 2001-10-06 08:16:30 adam Exp $

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c

EXTRA_DIST=dummy-records dummy-words dummy-grs

if ISTHR
extra=../lib/libyazthread.la
endif
yaz_ztest_LDADD=$(extra) ../lib/libyaz.la $(LIBTHREAD)

CFLAGS=@CFLAGSTHREADS@

INCLUDES=-I$(top_srcdir)/include
