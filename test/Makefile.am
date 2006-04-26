## Copyright (C) 1994-2006, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.14 2006-04-26 09:40:43 adam Exp $

check_PROGRAMS = tsticonv tstnmem tstmatchstr tstwrbuf tstodr tstccl tstlog \
 tstsoap1 tstsoap2 tstodrstack tstlogthread tstxmlquery tstpquery \
 tst_filepath
check_SCRIPTS = tstcql.sh tstmarciso.sh tstmarcxml.sh

TESTS = $(check_PROGRAMS) $(check_SCRIPTS)

EXTRA_DIST = tstodr.asn tstodrcodec.c tstodrcodec.h cqlsample \
 $(check_SCRIPTS) \
 marc1 marc1.xml marc1.chr marc1.xml.marc \
 marc2 marc2.xml marc2.chr marc2.xml.marc \
 marc3 marc3.xml marc3.chr marc3.xml.marc \
 marc4 marc4.xml marc4.chr marc4.xml.marc \
 marc5 marc5.xml marc5.chr marc5.xml.marc \
 marc6 marc6.xml marc6.chr marc6.xml.marc

YAZCOMP = ../util/yaz-asncomp
YAZCOMPLINE = $(YAZCOMP) -d z.tcl -i yaz -I../include $(YCFLAGS)

AM_CFLAGS=$(THREAD_CFLAGS)

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
tstlogthread_SOURCES = tstlogthread.c
tstxmlquery_SOURCES = tstxmlquery.c
tstpquery_SOURCES = tstpquery.c
tst_filepath_SOURCES = tst_filepath.c

