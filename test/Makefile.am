## Copyright (C) 1994-2003, Index Data
## All rights reserved.
## $Id: Makefile.am,v 1.1 2003-10-27 12:21:38 adam Exp $

check_PROGRAMS = tsticonv tstnmem tstmatchstr tstwrbuf tstodr tstccl

TESTS = $(check_PROGRAMS)

EXTRA_DIST = tstodr.asn tstodrcodec.c tstodrcodec.h

YAZCOMP = $(top_srcdir)/util/yaz-asncomp
YAZCOMPLINE = $(YAZCOMP) -d z.tcl -i yaz -I../include $(YCFLAGS)

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS)

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

