## Copyright (C) 1994-2004, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.3 2004-10-01 11:43:43 adam Exp $

check_PROGRAMS = tsticonv tstnmem tstmatchstr tstwrbuf tstodr tstccl
check_SCRIPTS = tstcql.sh tstmarc.sh

TESTS = $(check_PROGRAMS) $(check_SCRIPTS)

EXTRA_DIST = tstodr.asn tstodrcodec.c tstodrcodec.h cqlsample \
 $(check_SCRIPTS) \
 marc1 marc1.xml marc2 marc2.xml marc3 marc3.xml

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

LDADD = ../src/libyaz.la

tsticonv_SOURCES = tsticonv.c

tstnmem_SOURCES = tstnmem.c

tstmatchstr_SOURCES = tstmatchstr.c

tstwrbuf_SOURCES = tstwrbuf.c

tstodr_SOURCES = tstodrcodec.c tstodrcodec.h tstodr.c

tstccl_SOURCES = tstccl.c

