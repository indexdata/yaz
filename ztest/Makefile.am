## $Id: Makefile.am,v 1.16 2004-04-28 22:45:00 adam Exp $

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c

EXTRA_DIST=dummy-records dummy-words dummy-grs

if ISTHR
extra=../src/libyazthread.la
endif

yaz_ztest_LDADD=$(extra) ../src/libyaz.la $(SSL_LIBS) $(LIBTHREAD)

AM_CFLAGS=@CFLAGSTHREADS@

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)
