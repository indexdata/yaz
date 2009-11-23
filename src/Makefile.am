## This file is part of the YAZ toolkit.
## Copyright (C) 1995-2009 Index Data

YAZ_VERSION_INFO=3:0:0

lib_LTLIBRARIES = libyaz.la libyaz_server.la libyaz_icu.la

dist-hook:
	test -f $(srcdir)/cql.c || exit 1

tabdatadir = $(pkgdatadir)/z39.50
tabdata_DATA=datetime.asn esupdate.asn univres.asn z3950v3.asn z.tcl \
 esadmin.asn charneg-3.asn mterm2.asn oclcui.asn

illdatadir=$(pkgdatadir)/ill
illdata_DATA=ill9702.asn item-req.asn ill.tcl oclc-ill-req-ext.asn

EXTRA_DIST=$(tabdata_DATA) $(illdata_DATA) \
 charconv.tcl codetables.xml codetables-iso5426.xml \
 csvtodiag.tcl csvtobib1.tcl csvtosrw.tcl bib1.csv srw.csv \
 csvtosru_update.tcl sru_update.csv mk_version.tcl \
 oidtoc.tcl oid.csv sc_test.c

YAZCOMP=$(top_srcdir)/util/yaz-asncomp
YAZCOMP_Z = $(YAZCOMP) -d $(srcdir)/z.tcl -i yaz -I$(top_srcdir)/include
YAZCOMP_I = $(YAZCOMP) -d $(srcdir)/ill.tcl -i yaz -I$(top_srcdir)/include

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS) $(SSL_CFLAGS) 
libyaz_la_LIBADD = $(SSL_LIBS)
libyaz_server_la_CFLAGS = $(PTHREAD_CFLAGS)
libyaz_server_la_LIBADD = libyaz.la
libyaz_icu_la_CPPFLAGS = $(AM_CPPFLAGS) $(ICU_CPPFLAGS)
libyaz_icu_la_LIBADD = libyaz.la $(ICU_LIBS)

AM_YFLAGS=-p cql_

# MARC8->UTF8 conversion is generated from codetables.xml
marc8.c: charconv.tcl codetables.xml
	$(TCLSH) $(srcdir)/charconv.tcl -p marc8 $(srcdir)/codetables.xml -o $@

# UTF-8->MARC8 conversion is generated from codetables.xml
marc8r.c: charconv.tcl codetables.xml
	$(TCLSH) $(srcdir)/charconv.tcl -r -p marc8r $(srcdir)/codetables.xml -o $@

# ISO5426->UTF8 conversion is generated from codetables-iso5426.xml
iso5426.c: charconv.tcl codetables-iso5426.xml
	$(TCLSH) $(srcdir)/charconv.tcl -p iso5426 $(srcdir)/codetables-iso5426.xml -o $@

# Generate OID database from CSV
oid_std.c $(top_srcdir)/include/yaz/oid_std.h: oidtoc.tcl oid.csv
	$(TCLSH) $(srcdir)/oidtoc.tcl $(srcdir) $(srcdir)/oid.csv oid_std.c oid_std.h

# Generate diagnostics from CSVs
diagbib1.c $(top_srcdir)/include/yaz/diagbib1.h: csvtobib1.tcl bib1.csv
	$(TCLSH) $(srcdir)/csvtobib1.tcl $(srcdir)

diagsrw.c $(top_srcdir)/include/yaz/diagsrw.h: csvtosrw.tcl srw.csv
	$(TCLSH) $(srcdir)/csvtosrw.tcl $(srcdir)

diagsru_update.c $(top_srcdir)/include/yaz/diagsru_update.h: csvtosru_update.tcl sru_update.csv
	$(TCLSH) $(srcdir)/csvtosru_update.tcl $(srcdir)

# Auto-generated C-files
GEN_FILES = oid_std.c \
  marc8.c marc8r.c iso5426.c \
  diagbib1.c diagsrw.c diagsru_update.c \
  z-accdes1.c z-accform1.c z-acckrb1.c z-core.c \
  z-diag1.c z-espec1.c z-estask.c z-exp.c z-grs.c z-mterm2.c z-opac.c \
  z-uifr1.c z-rrf1.c z-rrf2.c z-sum.c z-sutrs.c z-oclcui.c\
  zes-expi.c zes-exps.c zes-order.c zes-pquery.c zes-psched.c \
  zes-pset.c zes-update0.c z-date.c z-univ.c zes-update.c zes-admin.c \
  z-charneg.c \
  ill-core.c item-req.c oclc-ill-req-ext.c

libyaz_la_SOURCES=version.c options.c log.c \
 $(GEN_FILES) \
  marcdisp.c marc_read_xml.c marc_read_iso2709.c marc_read_line.c \
  wrbuf.c oid_db.c errno.c \
  nmemsdup.c xmalloc.c readconf.c tpath.c nmem.c matchstr.c atoin.c \
  siconv.c iconv-p.h utf8.c ucs4.c iso5428.c advancegreek.c \
  odr_bool.c ber_bool.c ber_len.c ber_tag.c odr_util.c \
  odr_null.c ber_null.c odr_int.c ber_int.c odr_tag.c odr_cons.c \
  odr_seq.c odr_oct.c ber_oct.c odr_bit.c ber_bit.c odr_oid.c \
  ber_oid.c odr_use.c odr_choice.c odr_any.c ber_any.c odr.c odr_mem.c \
  dumpber.c odr_enum.c odr-priv.h libxml2_error.c \
  comstack.c tcpip.c waislen.c unix.c \
  prt-ext.c \
  ill-get.c \
  zget.c yaz-ccl.c diag-entry.c diag-entry.h \
  logrpn.c \
  otherinfo.c pquery.c sortspec.c charneg.c initopt.c \
  zoom-c.c zoom-socket.c zoom-opt.c zoom-p.h \
  grs1disp.c zgdu.c soap.c srw.c srwutil.c uri.c \
  opacdisp.c cclfind.c ccltoken.c cclerrms.c cclqual.c cclptree.c cclp.h \
  cclqfile.c cclstr.c cclxmlconfig.c ccl_stop_words.c \
  cql.y cqlstdio.c cqltransform.c cqlutil.c xcqlutil.c cqlstring.c rpn2cql.c \
  cqlstrer.c querytowrbuf.c \
  tcpdchk.c \
  test.c timing.c \
  xmlquery.c xmlerror.c http.c \
  mime.c mime.h oid_util.c tokenizer.c \
  record_conv.c retrieval.c elementset.c snprintf.c query-charset.c \
  copy_types.c match_glob.c poll.c daemon.c \
  iconv_encode_marc8.c iconv_encode_iso_8859_1.c iconv_encode_wchar.c \
  iconv_decode_marc8.c iconv_decode_iso5426.c iconv_decode_danmarc.c sc.c

libyaz_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

libyaz_server_la_SOURCES = statserv.c seshigh.c eventl.c \
  requestq.c eventl.h session.h mutex.c

libyaz_server_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

libyaz_icu_la_SOURCES = icu_I18N.c
libyaz_icu_la_LDFLAGS=-version-info $(YAZ_VERSION_INFO)

# Rules for Z39.50 V3
z-accdes1.c \
z-accform1.c \
z-acckrb1.c \
z-diag1.c \
z-espec1.c \
z-estask.c \
z-exp.c \
z-grs.c \
z-opac.c \
z-uifr1.c \
z-rrf1.c \
z-rrf2.c \
z-sum.c \
z-sutrs.c \
zes-expi.c \
zes-exps.c \
zes-order.c \
zes-pquery.c \
zes-psched.c \
zes-pset.c \
zes-update0.c \
$(top_srcdir)/include/yaz/z-accdes1.h \
$(top_srcdir)/include/yaz/z-core.h: z-core.c

z-core.c: $(srcdir)/z.tcl $(srcdir)/z3950v3.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_Z) $(srcdir)/z3950v3.asn

# Date extension
z-date.c \
$(top_srcdir)/include/yaz/z-date.h: $(srcdir)/z.tcl $(srcdir)/datetime.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_Z) $(srcdir)/datetime.asn

# UNIverse extension
z-univ.c \
$(top_srcdir)/include/yaz/z-univ.h: \
$(srcdir)/z.tcl $(srcdir)/univres.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_Z) $(srcdir)/univres.asn

# New Update extended service
zes-update.c \
$(top_srcdir)/include/yaz/zes-update.h: \
$(srcdir)/z.tcl $(srcdir)/esupdate.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_Z) $(srcdir)/esupdate.asn

# Admin extended service
zes-admin.c \
$(top_srcdir)/include/yaz/zes-admin.h: \
$(srcdir)/z.tcl $(srcdir)/esadmin.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_Z) $(srcdir)/esadmin.asn

# Charset negotiation
z-charneg.c \
$(top_srcdir)/include/yaz/z-charneg.h: \
$(srcdir)/z.tcl $(srcdir)/charneg-3.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_Z) $(srcdir)/charneg-3.asn

# UserInfoFormat-multipleSearchTerms-2
z-mterm2.c \
$(top_srcdir)/include/yaz/z-mterm2.h: \
$(srcdir)/z.tcl $(srcdir)/mterm2.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_Z) $(srcdir)/mterm2.asn

# UserInfoFormat-multipleSearchTerms-2
z-oclcui.c \
$(top_srcdir)/include/yaz/z-oclcui.h: \
$(srcdir)/z.tcl $(srcdir)/oclcui.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_Z) $(srcdir)/oclcui.asn

# ILL protocol
ill-core.c \
$(top_srcdir)/include/yaz/ill-core.h: \
$(srcdir)/ill.tcl $(srcdir)/ill9702.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_I) $(srcdir)/ill9702.asn

# OCLC ILL Request Extension
oclc-ill-req-ext.c \
$(top_srcdir)/include/yaz/oclc-ill-req-ext.h: \
$(srcdir)/ill.tcl $(srcdir)/oclc-ill-req-ext.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_I) $(srcdir)/oclc-ill-req-ext.asn

# Item Request
item-req.c \
$(top_srcdir)/include/yaz/item-req.h: \
$(srcdir)/ill.tcl $(srcdir)/item-req.asn $(YAZCOMP)
	$(TCLSH) $(YAZCOMP_I) $(srcdir)/item-req.asn

