## $Id: Makefile.am,v 1.8 2001-11-19 09:16:37 adam Exp $

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c

EXTRA_DIST=dummy-records dummy-words dummy-grs

if ISTHR
extra=../lib/libyazthread.la
endif
yaz_ztest_LDADD=$(extra) ../lib/libyaz.la $(LIBTHREAD)

AM_CFLAGS=@CFLAGSTHREADS@

INCLUDES=-I$(top_srcdir)/include
