## $Id: Makefile.am,v 1.17 2004-05-01 23:32:21 adam Exp $

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c

EXTRA_DIST=dummy-records dummy-words dummy-grs

if ISTHR
extra=../src/libyazthread.la
endif

yaz_ztest_LDADD=$(extra) ../src/libyaz.la $(LIBTHREAD)

AM_CFLAGS=@CFLAGSTHREADS@

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)
