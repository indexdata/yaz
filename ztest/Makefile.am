## $Id: Makefile.am,v 1.11 2002-04-15 09:44:44 adam Exp $

if ISSSL
sslbin=yaz-ztest-ssl
endif

bin_PROGRAMS=yaz-ztest $(sslbin)

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c
yaz_ztest_ssl_SOURCES=$(yaz_ztest_SOURCES)

EXTRA_DIST=dummy-records dummy-words dummy-grs

if ISTHR
extra=../lib/libyazthread.la
endif

yaz_ztest_LDADD=$(extra) ../lib/libyaz.la \
 $(LIBTHREAD)

yaz_ztest_ssl_LDADD=$(extra) ../lib/libyazssl.la ../lib/libyaz.la \
 $(SSL_LIBS) $(LIBTHREAD)

AM_CFLAGS=@CFLAGSTHREADS@

AM_CPPFLAGS=-I$(top_srcdir)/include
