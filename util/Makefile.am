## Copyright (C) 1994-2003, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.16 2003-02-18 14:28:52 adam Exp $

noinst_LTLIBRARIES = libutil.la

#lib_LTLIBRARIES = libyazthread.la

bin_SCRIPTS = yaz-comp

EXTRA_DIST = yaz-comp cvs-date.tcl charconv.tcl charconv.sgm

AM_CPPFLAGS=-I$(top_srcdir)/include

noinst_PROGRAMS = marcdump yaziconv

marcdump_LDADD = libutil.la 
marcdump_SOURCES = marcdump.c

yaziconv_LDADD = libutil.la 
yaziconv_SOURCES = siconvtst.c

marc8.c: charconv.sgm charconv.tcl
	cd $(srcdir); ./charconv.tcl -p marc8 -s 50 charconv.sgm marc8.c

libutil_la_SOURCES=version.c options.c log.c marcdisp.c oid.c wrbuf.c \
   nmemsdup.c xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c \
   siconv.c marc8.c

