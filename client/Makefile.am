## Copyright (C) 1995-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.12 2002-01-29 20:17:41 ja7 Exp $

bin_PROGRAMS=yaz-client

EXTRA_DIST = default.bib

yaz_client_SOURCES=client.c admin.c admin.h tabcomplete.c

yaz_client_LDADD = ../lib/libyaz.la $(READLINE_LIBS)

INCLUDES=-I$(top_srcdir)/include
