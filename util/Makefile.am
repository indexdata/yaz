## Copyright (C) 1994-2001, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.10 2001-05-16 07:37:39 adam Exp $

noinst_LTLIBRARIES = libutil.la

#lib_LTLIBRARIES = libyazthread.la

bin_SCRIPTS = yaz-comp

EXTRA_DIST = yaz-comp cvs-date.tcl

LIBS =

INCLUDES =-I$(top_srcdir)/include

libyazthread_la_INCLUDES=-I$(top_srcdir)/include

noinst_PROGRAMS = marcdump

marcdump_LDADD = libutil.la 

marcdump_SOURCES = marcdump.c

libutil_la_SOURCES=options.c log.c marcdisp.c oid.c wrbuf.c nmemsdup.c \
   xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c 

#libyazthread_la_SOURCES=nmemthread.c xmalloc.c log.c
