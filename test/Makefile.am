## This file is part of the YAZ toolkit.
## Copyright (C) 1995-2010 Index Data

check_PROGRAMS = test_xmalloc test_iconv test_nmem test_matchstr test_wrbuf \
 test_odr test_ccl test_log \
 test_soap1 test_soap2 test_odrstack test_log_thread test_xmlquery test_pquery \
 test_comstack test_filepath test_record_conv test_retrieval test_tpath \
 test_timing test_query_charset test_oid test_icu test_match_glob \
 test_rpn2cql test_json test_xml_include test_file_glob

check_SCRIPTS = tstmarc.sh tstmarccol.sh tstcql2xcql.sh tstcql2pqf.sh tsticu.sh

TESTS = $(check_PROGRAMS) $(check_SCRIPTS)

EXTRA_DIST = tstodr.asn tstodrcodec.c tstodrcodec.h cql2xcqlsample \
 cql2pqf-order.txt cql2pqfsample \
 $(check_SCRIPTS) \
 marc1.marc marc1.xml marc1.chr marc1.xml.marc \
 marc2.marc marc2.xml marc2.chr marc2.xml.marc \
 marc3.marc marc3.xml marc3.chr marc3.xml.marc \
 marc4.marc marc4.xml marc4.chr marc4.xml.marc \
 marc5.marc marc5.xml marc5.chr marc5.xml.marc \
 marc6.marc marc6.xml marc6.chr marc6.xml.marc \
 marc7.marc marc7.xml marc7.chr marc7.xml.marc \
 marc8.marc marc8.xml marc8.chr marc8.xml.marc \
 marc9.marc marc9.xml marc9.chr marc9.xml.marc \
 marccol1.u8.marc marccol1.u8.1.lst marccol1.u8.2.lst \
 marccol2.u8.marc marccol2.u8.1.lst marccol2.u8.2.lst \
 marccol3.u8.marc marccol3.u8.1.lst marccol3.u8.2.lst \
 marccol4.u8.marc marccol4.u8.1.lst marccol4.u8.2.lst \
 marccol5.u8.marc marccol5.u8.1.lst marccol5.u8.2.lst \
 tsticu-0.xml tsticu-0.input tsticu-0.output \
 tsticu-1.xml tsticu-1.input tsticu-1.output \
 test_record_conv.xsl test_xml_include.xml

YAZCOMP = ../util/yaz-asncomp
YAZCOMPLINE = $(YAZCOMP) -d z.tcl -i yaz -I../include $(YCFLAGS)

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS) $(ICU_CPPFLAGS)

dist-hook:
	mkdir -p $(distdir)/cql
	cp $(srcdir)/cql/*.err $(srcdir)/cql/*.out $(distdir)/cql
	mkdir -p $(distdir)/cql2pqf
	cp $(srcdir)/cql2pqf/*.err $(srcdir)/cql2pqf/*.out $(distdir)/cql2pqf

# Rule for generating codecs for our small ASN.1 spec
test_odrcodec.c test_odrcodec.h: tstodr.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMP) tstodr.asn

LDADD = ../src/libyaz.la
test_icu_LDADD = ../src/libyaz_icu.la $(ICU_LIBS)
AM_CFLAGS = $(PTHREAD_CFLAGS)


CONFIG_CLEAN_FILES=*.log

test_xmalloc_SOURCES = test_xmalloc.c
test_iconv_SOURCES = test_iconv.c
test_nmem_SOURCES = test_nmem.c
test_matchstr_SOURCES = test_matchstr.c
test_wrbuf_SOURCES = test_wrbuf.c
test_odr_SOURCES = test_odrcodec.c test_odrcodec.h test_odr.c
test_odrstack_SOURCES = test_odrstack.c
test_ccl_SOURCES = test_ccl.c
test_log_SOURCES = test_log.c
test_soap1_SOURCES = test_soap1.c
test_soap2_SOURCES = test_soap2.c
test_log_thread_SOURCES = test_log_thread.c
test_xmlquery_SOURCES = test_xmlquery.c
test_pquery_SOURCES = test_pquery.c
test_comstack_SOURCES = test_comstack.c
test_filepath_SOURCES = test_filepath.c
test_oid_SOURCES = test_oid.c
test_record_conv_SOURCES = test_record_conv.c
test_retrieval_SOURCES = test_retrieval.c
test_tpath_SOURCES = test_tpath.c
test_timing_SOURCES = test_timing.c
test_query_charset_SOURCES = test_query_charset.c
test_icu_SOURCES = test_icu.c
test_match_glob_SOURCES = test_match_glob.c
test_rpn2cql_SOURCES = test_rpn2cql.c
test_json_SOURCES = test_json.c
test_xml_include_SOURCES = test_xml_include.c
test_file_glob_SOURCES = test_file_glob.c
