## Copyright (C) 1995-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.17 2003-02-18 10:37:08 adam Exp $

if ISSSL
extra=yaz-client-ssl
endif

bin_PROGRAMS=yaz-client $(extra)

EXTRA_DIST = default.bib

yaz_client_SOURCES=client.c admin.c admin.h tabcomplete.c tabcomplete.h
yaz_client_ssl_SOURCES=$(yaz_client_SOURCES)

yaz_client_LDADD = ../lib/libyaz.la $(READLINE_LIBS)
yaz_client_ssl_LDADD = ../lib/libyazssl.la ../lib/libyaz.la $(READLINE_LIBS) 

AM_CPPFLAGS=-I$(top_srcdir)/include
