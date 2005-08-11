## Copyright (C) 1994-2004, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.7 2005-08-11 14:00:43 adam Exp $

check_PROGRAMS = tsticonv tstnmem tstmatchstr tstwrbuf tstodr tstccl tstlog \
 tstsoap1 tstsoap2 tstodrstack
check_SCRIPTS = tstcql.sh tstmarc.sh

TESTS = $(check_PROGRAMS) $(check_SCRIPTS)

EXTRA_DIST = tstodr.asn tstodrcodec.c tstodrcodec.h cqlsample \
 $(check_SCRIPTS) \
 marc1 marc1.xml marc2 marc2.xml marc3 marc3.xml marc4 marc4.xml

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

tstodrstack_SOURCES = tstodrstack.c

tstccl_SOURCES = tstccl.c

tstlog_SOURCES = tstlog.c

tstsoap1_SOURCES = tstsoap1.c

tstsoap2_SOURCES = tstsoap2.c

