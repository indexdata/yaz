## Copyright (C) 1994-2003, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.22 2003-06-11 11:09:22 adam Exp $

noinst_LTLIBRARIES = libutil.la

check_PROGRAMS = tsticonv tstnmem tstmatchstr tstwrbuf

TESTS = $(check_PROGRAMS)

bin_SCRIPTS = yaz-asncomp

EXTRA_DIST = yaz-asncomp cvs-date.tcl \
 charconv.tcl charconv.sgm charconv_cjk.xml

AM_CPPFLAGS=-I$(top_srcdir)/include

noinst_PROGRAMS = marcdump yaziconv

# MARC dumper utility
marcdump_LDADD = libutil.la 
marcdump_SOURCES = marcdump.c

# YAZ Iconv utility
yaziconv_LDADD = libutil.la 
yaziconv_SOURCES = yaziconv.c

# Tests..
tsticonv_LDADD = libutil.la
tsticonv_SOURCES = tsticonv.c

tstnmem_LDADD = libutil.la
tstnmem_SOURCES = tstnmem.c

tstmatchstr_LDADD = libutil.la
tstmatchstr_SOURCES = tstmatchstr.c

tstwrbuf_LDADD = libutil.la
tstwrbuf_SOURCES = tstwrbuf.c

# MARC8 conversion is generated from charconv.sgm
marc8.c: charconv.sgm charconv.tcl
	cd $(srcdir); ./charconv.tcl -p marc8 -s 50 charconv.sgm -o marc8.c

libutil_la_SOURCES=version.c options.c log.c marcdisp.c oid.c wrbuf.c \
   nmemsdup.c xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c \
   siconv.c marc8.c

