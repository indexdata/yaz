## This file is part of the YAZ toolkit.
## Copyright (C) Index Data

check_PROGRAMS = test_ccl test_comstack test_cql2ccl \
 test_embed_record test_filepath test_file_glob \
 test_iconv test_icu test_json \
 test_libstemmer test_log test_log_thread \
 test_match_glob test_matchstr test_mutex \
 test_nmem test_odr test_odrstack test_oid test_options \
 test_pquery test_query_charset \
 test_record_conv test_rpn2cql test_rpn2solr test_retrieval \
 test_shared_ptr test_soap1 test_soap2 test_solr test_sortspec \
 test_timing test_tpath test_wrbuf \
 test_xmalloc test_xml_include test_xmlquery test_zgdu

check_SCRIPTS = test_marc.sh test_marccol.sh test_cql2xcql.sh \
	test_cql2pqf.sh test_icu.sh

TESTS = $(check_PROGRAMS) $(check_SCRIPTS)

EXTRA_DIST = tstodr.asn test_odrcodec.c test_odrcodec.h cql2xcqlsample \
 cql2pqf-order.txt cql2pqfsample \
 $(check_SCRIPTS) \
 marccol1.u8.marc marccol1.u8.1.lst marccol1.u8.2.lst \
 marccol2.u8.marc marccol2.u8.1.lst marccol2.u8.2.lst \
 marccol3.u8.marc marccol3.u8.1.lst marccol3.u8.2.lst \
 marccol4.u8.marc marccol4.u8.1.lst marccol4.u8.2.lst \
 marccol5.u8.marc marccol5.u8.1.lst marccol5.u8.2.lst \
 test_icu.0.xml test_icu.0.input test_icu.0.output \
 test_icu.1.xml test_icu.1.input test_icu.1.output \
 test_icu.2.xml test_icu.2.input test_icu.2.output \
 test_icu.3.xml test_icu.3.input test_icu.3.output \
 test_icu.4.xml test_icu.4.input test_icu.4.output \
 test_record_conv.xsl test_xml_include.xml

YAZCOMP = ../util/yaz-asncomp
YAZCOMPLINE = $(YAZCOMP) -d z.tcl -i yaz -I../include $(YCFLAGS)

AM_CPPFLAGS=-I$(top_srcdir)/include $(XML2_CFLAGS) $(ICU_CPPFLAGS)

dist-hook:
	mkdir -p $(distdir)/cql
	cp $(srcdir)/cql/*.err $(srcdir)/cql/*.out $(distdir)/cql
	mkdir -p $(distdir)/cql2pqf
	cp $(srcdir)/cql2pqf/*.err $(srcdir)/cql2pqf/*.out $(distdir)/cql2pqf
	cp $(srcdir)/marc[0-9]*.chr $(distdir)
	cp $(srcdir)/marc[0-9]*.marc $(distdir)
	cp $(srcdir)/marc[0-9]*.marc $(distdir)
	cp $(srcdir)/marc[0-9]*.xml $(distdir)
	cp $(srcdir)/marc[0-9]*.json $(distdir)
	cp $(srcdir)/marc[0-9]*.json.marc $(distdir)
	cp $(srcdir)/*.xml.marc $(distdir)
	cp $(srcdir)/xml2marc[0-9]*.xml $(distdir)
	cp $(srcdir)/tmarc[0-9]*.xml $(distdir)
	cp $(srcdir)/xml2tmarc[0-9]*.xml $(distdir)

# Rule for generating codecs for our small ASN.1 spec
test_odrcodec.c test_odrcodec.h: tstodr.asn $(YAZCOMP)
	cd $(srcdir); $(YAZCOMP) tstodr.asn

LDADD = ../src/libyaz.la
test_icu_LDADD = ../src/libyaz_icu.la ../src/libyaz.la $(ICU_LIBS)
test_libstemmer_LDADD = ../src/libyaz_icu.la ../src/libyaz.la $(ICU_LIBS)

CONFIG_CLEAN_FILES=*.log

test_cql2ccl_SOURCES = test_cql2ccl.c
test_xmalloc_SOURCES = test_xmalloc.c
test_iconv_SOURCES = test_iconv.c
test_nmem_SOURCES = test_nmem.c
test_matchstr_SOURCES = test_matchstr.c
test_wrbuf_SOURCES = test_wrbuf.c
test_odr_SOURCES = test_odrcodec.c test_odrcodec.h test_odr.c
test_odrstack_SOURCES = test_odrstack.c
test_ccl_SOURCES = test_ccl.c
test_log_SOURCES = test_log.c
test_mutex_SOURCES = test_mutex.c
test_soap1_SOURCES = test_soap1.c
test_soap2_SOURCES = test_soap2.c
test_solr_SOURCES = test_solr.c
test_sortspec_SOURCES = test_sortspec.c
test_log_thread_SOURCES = test_log_thread.c
test_xmlquery_SOURCES = test_xmlquery.c
test_options_SOURCES = test_options.c
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
test_rpn2solr_SOURCES = test_rpn2solr.c
test_json_SOURCES = test_json.c
test_xml_include_SOURCES = test_xml_include.c
test_file_glob_SOURCES = test_file_glob.c
test_shared_ptr_SOURCES = test_shared_ptr.c
test_libstemmer_SOURCES = test_libstemmer.c
test_embed_record_SOURCES = test_embed_record.c
test_zgdu_SOURCES = test_zgdu.c
