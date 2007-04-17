## $Id: Makefile.am,v 1.22 2007-04-17 20:26:19 adam Exp $

bin_PROGRAMS=yaz-ztest

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c

EXTRA_DIST=dummy-records dummy-words dummy-grs ztest.pem config1.xml

yaz_ztest_LDADD=../src/libyaz_server.la ../src/libyaz.la $(PTHREAD_LIBS)

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)
