## Copyright (C) 1995-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.10 2001-03-21 15:02:39 adam Exp $

bin_PROGRAMS=yaz-client

EXTRA_DIST = default.bib

yaz_client_SOURCES=client.c admin.c admin.h

yaz_client_LDADD = ../lib/libyaz.la $(READLINE_LIBS)

INCLUDES=-I$(top_srcdir)/include -DCCL2RPN=1
