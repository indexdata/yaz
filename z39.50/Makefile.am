## $Id: Makefile.am,v 1.1 2000-02-28 12:28:34 adam Exp $

INCLUDES=-I../include

noinst_LIBRARIES = libasn.a

EXTRA_DIST=datetime.asn esupdate.asn univres.asn z3950v3.asn z.tcl

libasn_a_SOURCES = z-accdes1.c z-accform1.c z-acckrb1.c z-core.c \
 z-diag1.c z-espec1.c z-estask.c z-exp.c z-grs.c z-opac.c z-uifr1.c \
 z-rrf1.c z-rrf2.c z-sum.c z-sutrs.c \
 zes-expi.c zes-exps.c zes-order.c zes-pquery.c zes-psched.c \
 zes-pset.c zes-update0.c z-date.c z-univ.c zes-update.c

z-accdes1.c z-accform1.c z-acckrb1.c z-core.c \
 z-diag1.c z-espec1.c z-estask.c z-exp.c z-grs.c z-opac.c z-uifr1.c \
 z-rrf1.c z-rrf2.c z-sum.c z-sutrs.c \
 zes-expi.c zes-exps.c zes-order.c zes-pquery.c zes-psched.c \
 zes-pset.c zes-update0.c \
 ../include/yaz/z-accdes1.h: z.tcl z3950v3.asn ../util/yc.tcl
	../util/yc.tcl -d z.tcl -i yaz -I ../include $(YCFLAGS) z3950v3.asn

z-date.c ../include/yaz/z-date.h: z.tcl datetime.asn ../util/yc.tcl
	../util/yc.tcl -d z.tcl -i yaz -I ../include $(YCFLAGS) datetime.asn

z-univ.c ../include/yaz/z-univ.h: z.tcl univres.asn
	../util/yc.tcl -d z.tcl -i yaz -I ../include $(YCFLAGS) univres.asn

zes-update.c ../include/yaz/zes-update.h: z.tcl esupdate.asn
	../util/yc.tcl -d z.tcl -i yaz -I ../include $(YCFLAGS) esupdate.asn
