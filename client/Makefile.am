## Copyright (C) 1995-2004, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.25 2004-10-10 22:44:34 adam Exp $


bin_PROGRAMS=yaz-client 
EXTRA_PROGRAMS=bertorture

EXTRA_DIST = default.bib

COMMON=admin.c admin.h tabcomplete.c tabcomplete.h
yaz_client_SOURCES=client.c $(COMMON)

AM_CPPFLAGS=-I$(top_srcdir)/include $(SSL_CFLAGS)

yaz_client_LDADD = ../src/libyaz.la $(READLINE_LIBS) 
bertorture_LDADD = ../src/libyaz.la 

bertorture_SOURCES=bertorture.c


