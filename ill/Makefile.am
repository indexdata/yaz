## $Id: Makefile.am,v 1.6 2001-03-21 15:02:39 adam Exp $

INCLUDES=-I$(top_srcdir)/include

EXTRA_DIST=ill9702.asn item-req.asn ill.tcl

noinst_LTLIBRARIES=libill.la

LIBS =

libill_la_SOURCES=ill-core.c item-req.c ill-get.c

$(srcdir)/ill-core.c \
$(top_srcdir)/include/yaz/ill-core.h: \
$(srcdir)/ill.tcl $(srcdir)/ill9702.asn $(top_srcdir)/util/yaz-comp
	cd $(srcdir); ../util/yaz-comp -d ill.tcl -i yaz -I ../include $(YCFLAGS) ill9702.asn

$(srcdir)/item-req.c \
$(top_srcdir)/include/yaz/item-req.h: \
$(srcdir)/ill.tcl $(srcdir)/item-req.asn $(top_srcdir)/util/yaz-comp
	cd $(srcdir); ../util/yaz-comp -d ill.tcl -i yaz -I ../include $(YCFLAGS) item-req.asn
