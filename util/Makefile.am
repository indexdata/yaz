## Copyright (C) 1994-2003, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.17 2003-04-23 20:34:08 adam Exp $

noinst_LTLIBRARIES = libutil.la

TESTS = tsticonv tstnmem

bin_SCRIPTS = yaz-comp

EXTRA_DIST = yaz-comp cvs-date.tcl charconv.tcl charconv.sgm

AM_CPPFLAGS=-I$(top_srcdir)/include

noinst_PROGRAMS = marcdump yaziconv

EXTRA_PROGRAMS = tsticonv tstnmem

marcdump_LDADD = libutil.la 
marcdump_SOURCES = marcdump.c

yaziconv_LDADD = libutil.la 
yaziconv_SOURCES = yaziconv.c

tsticonv_LDADD = libutil.la
tsticonv_SOURCES = tsticonv.c

tstnmem_LDADD = libutil.la
tstnmem_SOURCES = tstnmem.c

marc8.c: charconv.sgm charconv.tcl
	cd $(srcdir); ./charconv.tcl -p marc8 -s 50 charconv.sgm marc8.c

libutil_la_SOURCES=version.c options.c log.c marcdisp.c oid.c wrbuf.c \
   nmemsdup.c xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c \
   siconv.c marc8.c

