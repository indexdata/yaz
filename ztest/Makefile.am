## $Id: Makefile.am,v 1.6 2001-10-05 13:55:17 adam Exp $

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c

EXTRA_DIST=dummy-records dummy-words dummy-grs

if ISTHR
extra=../lib/libyazthread.la
endif
yaz_ztest_LDADD=$(extra) ../lib/libyaz.la $(LIBTHREAD)

INCLUDES=-I$(top_srcdir)/include @CFLAGSTHREADS@
