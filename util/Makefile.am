## Copyright (C) 1994-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.7 2000-05-18 11:57:05 adam Exp $

noinst_LTLIBRARIES = libutil.la

bin_SCRIPTS = yaz-comp

EXTRA_DIST = yaz-comp

LIBS =

INCLUDES =-I$(srcdir)/../include

noinst_PROGRAMS = marcdump

marcdump_LDADD = libutil.la 

marcdump_SOURCES = marcdump.c

libutil_la_SOURCES=options.c log.c marcdisp.c oid.c wrbuf.c nmemsdup.c \
   xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c 
