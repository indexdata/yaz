## $Id: Makefile.am,v 1.15 2003-10-27 12:21:45 adam Exp $

if ISSSL
sslbin=yaz-ztest-ssl
endif

bin_PROGRAMS=yaz-ztest $(sslbin)

yaz_ztest_SOURCES=ztest.c read-grs.c read-marc.c
yaz_ztest_ssl_SOURCES=$(yaz_ztest_SOURCES)

EXTRA_DIST=dummy-records dummy-words dummy-grs

if ISTHR
extra=../src/libyazthread.la
endif

yaz_ztest_LDADD=$(extra) ../src/libyaz.la \
 $(LIBTHREAD)

yaz_ztest_ssl_LDADD=$(extra) ../src/libyazssl.la ../src/libyaz.la \
 $(SSL_LIBS) $(LIBTHREAD)

AM_CFLAGS=@CFLAGSTHREADS@

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)
