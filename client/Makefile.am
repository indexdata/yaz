## Copyright (C) 1995-2003, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.20 2003-10-27 12:21:23 adam Exp $

if ISSSL
extra=yaz-client-ssl
endif

bin_PROGRAMS=yaz-client $(extra)
EXTRA_PROGRAMS=bertorture

EXTRA_DIST = default.bib

yaz_client_SOURCES=client.c admin.c admin.h tabcomplete.c tabcomplete.h
yaz_client_ssl_SOURCES=$(yaz_client_SOURCES)

yaz_client_LDADD = ../src/libyaz.la $(READLINE_LIBS)
yaz_client_ssl_LDADD = ../src/libyazssl.la ../src/libyaz.la $(READLINE_LIBS) $(SSL_LIBS)
bertorture_LDADD = ../src/libyaz.la

bertorture_SOURCES=bertorture.c

AM_CPPFLAGS=-I$(top_srcdir)/include
