## Copyright (C) 1994-2004, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.2 2004-03-10 16:34:30 adam Exp $

check_PROGRAMS = tsticonv tstnmem tstmatchstr tstwrbuf tstodr tstccl
check_SCRIPTS = tstcql.sh

TESTS = $(check_PROGRAMS) $(check_SCRIPTS)

EXTRA_DIST = tstodr.asn tstodrcodec.c tstodrcodec.h cqlsample $(check_SCRIPTS)

YAZCOMP = $(top_srcdir)/util/yaz-asncomp
YAZCOMPLINE = $(YAZCOMP) -d z.tcl -i yaz -I../include $(YCFLAGS)

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)

dist-hook:
	mkdir -p $(distdir)/cql
	cp $(srcdir)/cql/*.err $(distdir)/cql
	cp $(srcdir)/cql/*.out $(distdir)/cql

# Rule for generating codecs for our small ASN.1 spec
tstodrcodec.c tstodrcodec.h: tstodr.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMP) tstodr.asn

# Tests..
tsticonv_LDADD = ../src/libyaz.la
tsticonv_SOURCES = tsticonv.c

tstnmem_LDADD = ../src/libyaz.la
tstnmem_SOURCES = tstnmem.c

tstmatchstr_LDADD = ../src/libyaz.la
tstmatchstr_SOURCES = tstmatchstr.c

tstwrbuf_LDADD = ../src/libyaz.la
tstwrbuf_SOURCES = tstwrbuf.c

tstodr_LDADD = ../src/libyaz.la
tstodr_SOURCES = tstodrcodec.c tstodrcodec.h tstodr.c

tstccl_LDADD = ../src/libyaz.la
tstccl_SOURCES = tstccl.c

