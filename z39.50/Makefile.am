## $Id: Makefile.am,v 1.9 2002-02-12 21:11:49 adam Exp $

INCLUDES=-I$(top_srcdir)/include

noinst_LTLIBRARIES = libz39.50.la

tabdatadir = $(pkgdatadir)/z39.50
tabdata_DATA=datetime.asn esupdate.asn univres.asn z3950v3.asn z.tcl \
 esadmin.asn charneg-3.asn
EXTRA_DIST=$(tabdata_DATA)

LIBS =

YAZCOMP = $(top_srcdir)/util/yaz-comp

libz39_50_la_SOURCES = z-accdes1.c z-accform1.c z-acckrb1.c z-core.c \
 z-diag1.c z-espec1.c z-estask.c z-exp.c z-grs.c z-opac.c z-uifr1.c \
 z-rrf1.c z-rrf2.c z-sum.c z-sutrs.c \
 zes-expi.c zes-exps.c zes-order.c zes-pquery.c zes-psched.c \
 zes-pset.c zes-update0.c z-date.c z-univ.c zes-update.c zes-admin.c \
 z-charneg.c \
 prt-ext.c

# Rules for Z39.50 V3
$(srcdir)/z-accdes1.c \
$(srcdir)/z-accform1.c \
$(srcdir)/z-acckrb1.c \
$(srcdir)/z-core.c \
$(srcdir)/z-diag1.c \
$(srcdir)/z-espec1.c \
$(srcdir)/z-estask.c \
$(srcdir)/z-exp.c \
$(srcdir)/z-grs.c \
$(srcdir)/z-opac.c \
$(srcdir)/z-uifr1.c \
$(srcdir)/z-rrf1.c \
$(srcdir)/z-rrf2.c \
$(srcdir)/z-sum.c \
$(srcdir)/z-sutrs.c \
$(srcdir)/zes-expi.c \
$(srcdir)/zes-exps.c \
$(srcdir)/zes-order.c \
$(srcdir)/zes-pquery.c \
$(srcdir)/zes-psched.c \
$(srcdir)/zes-pset.c \
$(srcdir)/zes-update0.c \
$(top_srcdir)/include/z-accdes1.h \
$(top_srcdir)/include/z-core.h: \
$(srcdir)/z.tcl $(srcdir)/z3950v3.asn $(YAZCOMP)
	cd $(srcdir); ../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) z3950v3.asn

# Date extension
$(srcdir)/z-date.c \
$(top_srcdir)/include/yaz/z-date.h: $(srcdir)/z.tcl $(srcdir)/datetime.asn $(YAZCOMP)
	cd $(srcdir); ../util/yaz-comp -d z.tcl -i yaz -I../include $(YCFLAGS) datetime.asn

# UNIverse extension
$(srcdir)/z-univ.c \
$(top_srcdir)/include/yaz/z-univ.h: \
$(srcdir)/z.tcl $(srcdir)/univres.asn $(YAZCOMP)
	cd $(srcdir); ../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) univres.asn

# New Update extended service
$(srcdir)/zes-update.c \
$(top_srcdir)/include/yaz/zes-update.h: \
$(srcdir)/z.tcl $(srcdir)/esupdate.asn $(YAZCOMP)
	cd $(srcdir); ../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) esupdate.asn

# Admin extended service
$(srcdir)/zes-admin.c \
$(top_srcdir)/include/yaz/zes-admin.h: \
$(srcdir)/z.tcl $(srcdir)/esadmin.asn $(YAZCOMP)
	cd $(srcdir); ../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) esadmin.asn

# Charset negotiation
$(srcdir)/z-charneg.c: $(srcdir)/z.tcl $(srcdir)/charneg-3.asn
	cd $(srcdir); ../util/yaz-comp -d z.tcl -i yaz -I ../include $(YCFLAGS) charneg-3.asn
