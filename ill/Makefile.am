## $Id: Makefile.am,v 1.10 2003-05-27 21:12:22 adam Exp $

AM_CPPFLAGS=-I$(top_srcdir)/include

tabdatadir=$(pkgdatadir)/ill
tabdata_DATA=ill9702.asn item-req.asn ill.tcl
EXTRA_DIST=$(tabdata_DATA)

noinst_LTLIBRARIES=libill.la

libill_la_SOURCES=ill-core.c item-req.c ill-get.c

$(srcdir)/ill-core.c \
$(top_srcdir)/include/yaz/ill-core.h: \
$(srcdir)/ill.tcl $(srcdir)/ill9702.asn $(top_srcdir)/util/yaz-asncomp
	cd $(srcdir); ../util/yaz-asncomp -d ill.tcl -i yaz -I ../include $(YCFLAGS) ill9702.asn

$(srcdir)/item-req.c \
$(top_srcdir)/include/yaz/item-req.h: \
$(srcdir)/ill.tcl $(srcdir)/item-req.asn $(top_srcdir)/util/yaz-asncomp
	cd $(srcdir); ../util/yaz-asncomp -d ill.tcl -i yaz -I ../include $(YCFLAGS) item-req.asn
