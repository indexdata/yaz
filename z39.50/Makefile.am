## $Id: Makefile.am,v 1.3 2000-03-14 09:22:15 ian Exp $

INCLUDES=-I../include

noinst_LIBRARIES = libasn.a

EXTRA_DIST=datetime.asn esupdate.asn univres.asn z3950v3.asn z.tcl esadmin.asn

libasn_a_SOURCES = z-accdes1.c z-accform1.c z-acckrb1.c z-core.c \
 z-diag1.c z-espec1.c z-estask.c z-exp.c z-grs.c z-opac.c z-uifr1.c \
 z-rrf1.c z-rrf2.c z-sum.c z-sutrs.c \
 zes-expi.c zes-exps.c zes-order.c zes-pquery.c zes-psched.c \
 zes-pset.c zes-update0.c z-date.c z-univ.c zes-update.c zes-admin.c

z-accdes1.c z-accform1.c z-acckrb1.c z-core.c \
 z-diag1.c z-espec1.c z-estask.c z-exp.c z-grs.c z-opac.c z-uifr1.c \
 z-rrf1.c z-rrf2.c z-sum.c z-sutrs.c \
 zes-expi.c zes-exps.c zes-order.c zes-pquery.c zes-psched.c \
 zes-pset.c zes-update0.c \
 ../include/yaz/z-accdes1.h: z.tcl z3950v3.asn ../util/yaz-comp
	../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) z3950v3.asn

z-date.c ../include/yaz/z-date.h: z.tcl datetime.asn ../util/yaz-comp
	../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) datetime.asn

z-univ.c ../include/yaz/z-univ.h: z.tcl univres.asn ../util/yaz-comp
	../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) univres.asn

zes-update.c ../include/yaz/zes-update.h: z.tcl esupdate.asn ../util/yaz-comp
	../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) esupdate.asn

zes-admin.c ../include/yaz/zes-admin.h: z.tcl esadmin.asn ../util/yaz-comp
	../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) esadmin.asn
