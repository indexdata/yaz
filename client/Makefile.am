## Copyright (C) 1995-2003, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.21 2004-04-28 12:10:51 adam Exp $

if ISSSL
extra=yaz-client-ssl
endif

bin_PROGRAMS=yaz-client $(extra)
EXTRA_PROGRAMS=bertorture

EXTRA_DIST = default.bib

COMMON=admin.c admin.h tabcomplete.c tabcomplete.h
yaz_client_SOURCES=client.c $(COMMON)
yaz_client_ssl_SOURCES=$(COMMON)

yaz_client_LDADD = ../src/libyaz.la $(READLINE_LIBS)
yaz_client_ssl_LDADD = ssl-client.lo ../src/libyazssl.la ../src/libyaz.la $(READLINE_LIBS) $(SSL_LIBS)
bertorture_LDADD = ../src/libyaz.la

bertorture_SOURCES=bertorture.c

AM_CPPFLAGS=-I$(top_srcdir)/include

ssl-client.lo: client.c
	$(LTCOMPILE) $(SSL_CFLAGS) $(SSL_DEFS) -c $(srcdir)/client.c -o ssl-client.lo
