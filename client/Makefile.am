## Copyright (C) 1995-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.13 2002-02-03 09:04:55 adam Exp $

bin_PROGRAMS=yaz-client

EXTRA_DIST = default.bib

yaz_client_SOURCES=client.c admin.c admin.h tabcomplete.c tabcomplete.h

yaz_client_LDADD = ../lib/libyaz.la $(READLINE_LIBS)

INCLUDES=-I$(top_srcdir)/include
