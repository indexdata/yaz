# $Id: Makefile.am,v 1.1 2000-02-28 11:20:05 adam Exp $

INCLUDES=-I../include

EXTRA_DIST=ill9702.asn item-req.asn ill.tcl

noinst_LIBRARIES=libill.a

libill_a_SOURCES=ill-core.c item-req.c ill-get.c

ill-core.c ../include/yaz/ill-core.h: ill.tcl ill9702.asn ../util/yc.tcl
	../util/yc.tcl -d ill.tcl -i yaz -I ../include $(YCFLAGS) ill9702.asn

item-req.c ../include/yaz/item-req.h: ill.tcl item-req.asn ../util/yc.tcl
	../util/yc.tcl -d ill.tcl -i yaz -I ../include $(YCFLAGS) item-req.asn

#dist-hook: ill-core.c item-req.c

