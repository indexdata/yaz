## Copyright (C) 1994-2001, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.12 2002-08-27 14:02:13 adam Exp $

noinst_LTLIBRARIES = libutil.la

#lib_LTLIBRARIES = libyazthread.la

bin_SCRIPTS = yaz-comp

EXTRA_DIST = yaz-comp cvs-date.tcl

LIBS =

AM_CPPFLAGS=-I$(top_srcdir)/include

noinst_PROGRAMS = marcdump yaziconv

marcdump_LDADD = libutil.la 
marcdump_SOURCES = marcdump.c

yaziconv_LDADD = libutil.la 
yaziconv_SOURCES = siconvtst.c

libutil_la_SOURCES=options.c log.c marcdisp.c oid.c wrbuf.c nmemsdup.c \
   xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c siconv.c

#libyazthread_la_SOURCES=nmemthread.c xmalloc.c log.c
