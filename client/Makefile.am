## Copyright (C) 1995-2004, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.22 2004-04-28 22:44:58 adam Exp $

bin_PROGRAMS=yaz-client 
EXTRA_PROGRAMS=bertorture

EXTRA_DIST = default.bib

COMMON=admin.c admin.h tabcomplete.c tabcomplete.h
yaz_client_SOURCES=client.c $(COMMON)

yaz_client_LDADD = ../src/libyaz.la $(READLINE_LIBS) $(SSL_LIBS)
bertorture_LDADD = ../src/libyaz.la $(SSL_LIBS)

bertorture_SOURCES=bertorture.c

AM_CPPFLAGS=-I$(top_srcdir)/include $(SSL_CFLAGS) $(SSL_DEFS)

