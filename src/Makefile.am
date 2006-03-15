## This file is part of the YAZ toolkit.
## Copyright (C) 1994-2005, Index Data, All rights reserved.
## $Id: Makefile.am,v 1.29 2006-03-15 13:32:05 adam Exp $

YAZ_VERSION_INFO=2:0:0

lib_LTLIBRARIES = libyaz.la libyazthread.la
# noinst_PROGRAMS = cqlparse

dist-hook:
	test -f $(srcdir)/cql.c || exit 1

tabdatadir = $(pkgdatadir)/z39.50
tabdata_DATA=datetime.asn esupdate.asn univres.asn z3950v3.asn z.tcl \
 esadmin.asn charneg-3.asn mterm2.asn oclcui.asn

illdatadir=$(pkgdatadir)/ill
illdata_DATA=ill9702.asn item-req.asn ill.tcl

EXTRA_DIST=$(tabdata_DATA) $(illdata_DATA) \
 charconv.tcl codetables.xml \
 csvtodiag.tcl csvtobib1.tcl csvtosrw.tcl bib1.csv srw.csv

YAZCOMP = ../util/yaz-asncomp
YAZCOMPLINE = $(YAZCOMP) -d z.tcl -i yaz -I../include $(YCFLAGS)

AM_CFLAGS=$(THREAD_CFLAGS)
AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS) $(SSL_CFLAGS) 

AM_YFLAGS=-p cql_

# MARC8 conversion is generated from codetables.xml
$(srcdir)/marc8.c: charconv.tcl codetables.xml
	cd $(srcdir); ./charconv.tcl -p marc8 codetables.xml -o marc8.c

# Generate diagnostics from CSVs
$(top_srcdir)/include/yaz/diagbib1.h $(srcdir)/diagbib1.c: csvtobib1.tcl bib1.csv
	cd $(srcdir); ./csvtobib1.tcl

$(top_srcdir)/include/yaz/diagsrw.h $(srcdir)/diagsrw.c: csvtosrw.tcl srw.csv
	cd $(srcdir); ./csvtosrw.tcl

libyaz_la_SOURCES=version.c options.c log.c marcdisp.c oid.c wrbuf.c \
  nmemsdup.c xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c \
  siconv.c marc8.c \
  odr_bool.c ber_bool.c ber_len.c ber_tag.c odr_util.c \
  odr_null.c ber_null.c odr_int.c ber_int.c odr_tag.c odr_cons.c \
  odr_seq.c odr_oct.c ber_oct.c odr_bit.c ber_bit.c odr_oid.c \
  ber_oid.c odr_use.c odr_choice.c odr_any.c ber_any.c odr.c odr_mem.c \
  dumpber.c odr_enum.c odr-priv.h \
  comstack.c tcpip.c waislen.c unix.c \
  z-accdes1.c z-accform1.c z-acckrb1.c z-core.c \
  z-diag1.c z-espec1.c z-estask.c z-exp.c z-grs.c z-mterm2.c z-opac.c \
  z-uifr1.c z-rrf1.c z-rrf2.c z-sum.c z-sutrs.c z-oclcui.c\
  zes-expi.c zes-exps.c zes-order.c zes-pquery.c zes-psched.c \
  zes-pset.c zes-update0.c z-date.c z-univ.c zes-update.c zes-admin.c \
  z-charneg.c \
  prt-ext.c \
  ill-core.c item-req.c ill-get.c \
  zget.c yaz-ccl.c diag-entry.c diag-entry.h diagbib1.c diagsrw.c logrpn.c \
  otherinfo.c pquery.c sortspec.c z3950oid.c charneg.c initopt.c \
  zoom-c.c zoom-opt.c zoom-p.h grs1disp.c zgdu.c soap.c srw.c srwutil.c \
  opacdisp.c cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c \
  cclqfile.c cclstr.c \
  cql.y cqlstdio.c cqltransform.c cqlutil.c xcqlutil.c cqlstring.c \
  cqlstrer.c querytowrbuf.c \
  eventl.c seshigh.c statserv.c requestq.c tcpdchk.c \
  eventl.h service.c service.h session.h test.c \
  xmlquery.c \
  mime.c mime.h

libyaz_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

# This library is only built and installed for backwards
# compatibility (older libs depend on it). The libyaz.la is now threaded
# by itself..
libyazthread_la_SOURCES=version.c
libyazthread_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

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
$(top_srcdir)/include/yaz/z-accdes1.h \
$(top_srcdir)/include/yaz/z-core.h: \
$(srcdir)/z.tcl $(srcdir)/z3950v3.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMPLINE) z3950v3.asn

# Date extension
$(srcdir)/z-date.c \
$(top_srcdir)/include/yaz/z-date.h: $(srcdir)/z.tcl $(srcdir)/datetime.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMPLINE) datetime.asn

# UNIverse extension
$(srcdir)/z-univ.c \
$(top_srcdir)/include/yaz/z-univ.h: \
$(srcdir)/z.tcl $(srcdir)/univres.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMPLINE) univres.asn

# New Update extended service
$(srcdir)/zes-update.c \
$(top_srcdir)/include/yaz/zes-update.h: \
$(srcdir)/z.tcl $(srcdir)/esupdate.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMPLINE) esupdate.asn

# Admin extended service
$(srcdir)/zes-admin.c \
$(top_srcdir)/include/yaz/zes-admin.h: \
$(srcdir)/z.tcl $(srcdir)/esadmin.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMPLINE) esadmin.asn

# Charset negotiation
$(srcdir)/z-charneg.c: $(srcdir)/z.tcl $(srcdir)/charneg-3.asn
	cd $(srcdir); $(YAZCOMPLINE) $(YCFLAGS) charneg-3.asn

# UserInfoFormat-multipleSearchTerms-2
$(srcdir)/z-mterm2.c: $(srcdir)/z.tcl $(srcdir)/mterm2.asn
	cd $(srcdir); $(YAZCOMPLINE) mterm2.asn

# UserInfoFormat-multipleSearchTerms-2
$(srcdir)/z-oclcui.c: $(srcdir)/z.tcl $(srcdir)/oclcui.asn
	cd $(srcdir); $(YAZCOMPLINE) oclcui.asn

# ILL protocol
$(srcdir)/ill-core.c \
$(top_srcdir)/include/yaz/ill-core.h: \
$(srcdir)/ill.tcl $(srcdir)/ill9702.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMP) -d ill.tcl -i yaz -I ../include $(YCFLAGS) ill9702.asn

# Item Request
$(srcdir)/item-req.c \
$(top_srcdir)/include/yaz/item-req.h: \
$(srcdir)/ill.tcl $(srcdir)/item-req.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMP) -d ill.tcl -i yaz -I ../include $(YCFLAGS) item-req.asn

# cqlparse_SOURCES = cqlstd.y

