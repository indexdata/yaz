## $Id: Makefile.am,v 1.3 2000-04-05 07:39:55 adam Exp $

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c

EXTRA_DIST=dummy-records dummy-words dummy-grs

yaz_ztest_LDADD=../lib/libyaz.la

INCLUDES=-I$(srcdir)/../include
