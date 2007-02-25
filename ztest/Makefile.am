## $Id: Makefile.am,v 1.21 2007-02-25 14:46:51 adam Exp $

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c

EXTRA_DIST=dummy-records dummy-words dummy-grs ztest.pem config1.xml

yaz_ztest_LDADD=../src/libyaz.la

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)
