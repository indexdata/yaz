## Copyright (C) 1994-2000, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.8 2000-11-23 10:58:33 adam Exp $

noinst_LTLIBRARIES = libutil.la

#lib_LTLIBRARIES = libyazthread.la

bin_SCRIPTS = yaz-comp

EXTRA_DIST = yaz-comp

LIBS =

INCLUDES =-I$(srcdir)/../include

libyazthread_la_INCLUDES=-I$(srcdir)/../include

noinst_PROGRAMS = marcdump

marcdump_LDADD = libutil.la 

marcdump_SOURCES = marcdump.c

libutil_la_SOURCES=options.c log.c marcdisp.c oid.c wrbuf.c nmemsdup.c \
   xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c 

#libyazthread_la_SOURCES=nmemthread.c xmalloc.c log.c
