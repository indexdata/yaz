## $Id: Makefile.am,v 1.5 2000-04-05 07:39:54 adam Exp $

INCLUDES=-I$(srcdir)/../include

EXTRA_DIST=ill9702.asn item-req.asn ill.tcl

noinst_LTLIBRARIES=libill.la

LIBS =

libill_la_SOURCES=ill-core.c item-req.c ill-get.c

ill-core.c ../include/yaz/ill-core.h: ill.tcl ill9702.asn ../util/yaz-comp
	../util/yaz-comp -d ill.tcl -i yaz -I ../include $(YCFLAGS) ill9702.asn

item-req.c ../include/yaz/item-req.h: ill.tcl item-req.asn ../util/yaz-comp
	../util/yaz-comp -d ill.tcl -i yaz -I ../include $(YCFLAGS) item-req.asn
