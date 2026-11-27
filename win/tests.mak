# This file is part of the YAZ toolkit.
# Copyright (C) Index Data

# Keep this list in step with check_PROGRAMS in test/Makefile.am.
# test_odrcodec.c is generated support for test_odr, not a test program.
TEST_PROGRAMS = \
    $(BINDIR)\test_charneg.exe \
    $(BINDIR)\test_client_record.exe \
    $(BINDIR)\test_ccl.exe \
    $(BINDIR)\test_comstack.exe \
    $(BINDIR)\test_cql2ccl.exe \
    $(BINDIR)\test_daemon.exe \
    $(BINDIR)\test_daemon_no_groups.exe \
    $(BINDIR)\test_daemon_no_passwd.exe \
    $(BINDIR)\test_errno.exe \
    $(BINDIR)\test_statserv_startup.exe \
    $(BINDIR)\test_embed_record.exe \
    $(BINDIR)\test_eventl.exe \
    $(BINDIR)\test_filepath.exe \
    $(BINDIR)\test_file_glob.exe \
    $(BINDIR)\test_iconv.exe \
    $(BINDIR)\test_icu.exe \
    $(BINDIR)\test_gfs_requests.exe \
    $(BINDIR)\test_json.exe \
    $(BINDIR)\test_libstemmer.exe \
    $(BINDIR)\test_log.exe \
    $(BINDIR)\test_log_thread.exe \
    $(BINDIR)\test_match_glob.exe \
    $(BINDIR)\test_matchstr.exe \
    $(BINDIR)\test_mutex.exe \
    $(BINDIR)\test_nmem.exe \
    $(BINDIR)\test_odr.exe \
    $(BINDIR)\test_odrstack.exe \
    $(BINDIR)\test_oid.exe \
    $(BINDIR)\test_options.exe \
    $(BINDIR)\test_otherinfo.exe \
    $(BINDIR)\test_pquery.exe \
    $(BINDIR)\test_query_charset.exe \
    $(BINDIR)\test_record_conv.exe \
    $(BINDIR)\test_rpn2cql.exe \
    $(BINDIR)\test_rpn2solr.exe \
    $(BINDIR)\test_retrieval.exe \
    $(BINDIR)\test_search_present.exe \
    $(BINDIR)\test_scan_limits.exe \
    $(BINDIR)\test_shared_ptr.exe \
    $(BINDIR)\test_soap1.exe \
    $(BINDIR)\test_soap2.exe \
    $(BINDIR)\test_solr.exe \
    $(BINDIR)\test_sortspec.exe \
    $(BINDIR)\test_sru_limits.exe \
    $(BINDIR)\test_timing.exe \
    $(BINDIR)\test_tpath.exe \
    $(BINDIR)\test_url.exe \
    $(BINDIR)\test_wrbuf.exe \
    $(BINDIR)\test_xmalloc.exe \
    $(BINDIR)\test_xml_include.exe \
    $(BINDIR)\test_xmlquery.exe \
    $(BINDIR)\test_zgdu.exe \
    $(BINDIR)\test_gfs_vhosts.exe \
    $(BINDIR)\test_marc_read_iso2709.exe \
    $(BINDIR)\test_marc_read_sax.exe \
    $(BINDIR)\test_present_limits.exe \
    $(BINDIR)\test_sru_update.exe \
    $(BINDIR)\test_zoom_scan.exe \
    $(BINDIR)\test_zoom_send.exe

# Use the library objects in an archive so tests can exercise private APIs.
# The linker also omits implementations embedded or replaced by a test.
TEST_LIB = $(OBJDIR)\yaz_test.lib
!if $(HAVE_ICU)
TEST_ICU_OBJS = $(ICU_DLL_OBJS)
TEST_ICU_LIBS = $(ICU_LIBS) $(LIBSTEMMER_LIB)
!else
TEST_ICU_OBJS =
TEST_ICU_LIBS =
!endif
TEST_LIB_OBJS = $(DLL_OBJS) $(COND_DLL_OBJS) $(TEST_ICU_OBJS)
TEST_LINK_LIBS = $(LINK_LIBS) $(TEST_ICU_LIBS)
# OID constants are local definitions in this archive, rather than DLL imports.
TEST_COPT = $(COPT) /DYAZ_DLL /I"$(TESTDIR)"

LINK_TEST = $(LINK) $(LNKOPT) $(CLIENT_LINK_OPTIONS) $(TEST_LINK_LIBS) $(TEST_LIB)

testprog: dirs generate $(TEST_LIB) $(TEST_PROGRAMS)

$(TEST_LIB): $(TEST_LIB_OBJS) $(WINDIR)\tests.mak
	lib.exe /nologo /out:$@ @<<
$(TEST_LIB_OBJS)
<<

# Build ordinary tests directly from their C source. This also works on a
# clean tree, without relying on NMAKE to chain two inference rules.
{$(TESTDIR)}.c{$(BINDIR)}.exe:
	$(CPP) $(TEST_COPT) $<
	$(LINK_TEST) $(OBJDIR)\$(@B).obj /out:$@

$(TEST_PROGRAMS): $(TEST_LIB) $(WINDIR)\tests.mak

$(BINDIR)\test_client_record.exe: $(OBJDIR)\test_client_record.obj \
    $(OBJDIR)\test_client_admin.obj $(OBJDIR)\test_client_tabcomplete.obj \
    $(OBJDIR)\test_client_fhistory.obj
	$(LINK_TEST) $(OBJDIR)\test_client_record.obj $(OBJDIR)\test_client_admin.obj \
        $(OBJDIR)\test_client_tabcomplete.obj $(OBJDIR)\test_client_fhistory.obj /out:$@

$(OBJDIR)\test_client_admin.obj: $(CLIENTDIR)\admin.c $(WINDIR)\tests.mak
	$(CPP) $(TEST_COPT) /Fo$@ $(CLIENTDIR)\admin.c

$(OBJDIR)\test_client_tabcomplete.obj: $(CLIENTDIR)\tabcomplete.c $(WINDIR)\tests.mak
	$(CPP) $(TEST_COPT) /Fo$@ $(CLIENTDIR)\tabcomplete.c

$(OBJDIR)\test_client_fhistory.obj: $(CLIENTDIR)\fhistory.c $(WINDIR)\tests.mak
	$(CPP) $(TEST_COPT) /Fo$@ $(CLIENTDIR)\fhistory.c

$(BINDIR)\test_odr.exe: $(OBJDIR)\test_odr.obj $(OBJDIR)\test_odrcodec.obj
	$(LINK_TEST) $(OBJDIR)\test_odr.obj $(OBJDIR)\test_odrcodec.obj /out:$@

$(OBJDIR)\test_odr.obj $(OBJDIR)\test_odrcodec.obj: $(TESTDIR)\test_odrcodec.h

!if $(HAVE_TCL)
$(TESTDIR)\test_odrcodec.c $(TESTDIR)\test_odrcodec.h: \
    $(TESTDIR)\tstodr.asn $(SRCDIR)\yaz-asncomp
	@cd $(TESTDIR)
	$(TCL) ../src/yaz-asncomp tstodr.asn
	@cd $(WINDIR)
!endif

$(OBJDIR)\test_daemon_no_groups.obj: $(TESTDIR)\test_daemon.c $(SRCDIR)\daemon.c
	$(CPP) $(TEST_COPT) /DTEST_DAEMON_NO_GROUPS /Fo$@ $(TESTDIR)\test_daemon.c

$(OBJDIR)\test_daemon_no_passwd.obj: $(TESTDIR)\test_daemon.c $(SRCDIR)\daemon.c
	$(CPP) $(TEST_COPT) /DTEST_DAEMON_NO_PASSWD /Fo$@ $(TESTDIR)\test_daemon.c

$(BINDIR)\test_daemon_no_groups.exe: $(OBJDIR)\test_daemon_no_groups.obj
	$(LINK_TEST) $(OBJDIR)\test_daemon_no_groups.obj /out:$@

$(BINDIR)\test_daemon_no_passwd.exe: $(OBJDIR)\test_daemon_no_passwd.obj
	$(LINK_TEST) $(OBJDIR)\test_daemon_no_passwd.obj /out:$@

$(BINDIR)\test_daemon.exe: $(SRCDIR)\daemon.c
$(OBJDIR)\test_client_record.obj: $(CLIENTDIR)\client.c
$(BINDIR)\test_gfs_vhosts.exe $(BINDIR)\test_statserv_startup.exe: $(SRCDIR)\statserv.c

$(OBJDIR)\test_url_transport.obj: $(SRCDIR)\url.c
	$(CPP) $(TEST_COPT) /Dcs_create_host2=test_url_create_host2 /Fo$@ $(SRCDIR)\url.c

$(BINDIR)\test_url.exe: $(OBJDIR)\test_url.obj $(OBJDIR)\test_url_transport.obj
	$(LINK_TEST) $(OBJDIR)\test_url.obj $(OBJDIR)\test_url_transport.obj /out:$@

$(BINDIR)\test_eventl.exe: $(SRCDIR)\eventl.c

# Run in test/ so relative fixture paths work. The runner reports every test
# and returns failure if any test fails, including a crash or missing DLL.
# NMAKE handles SET internally; invoke the runner on its own command line.
check: all
	set PATH=$(ICONV_DIR)\bin;$(LIBXML2_DIR)\bin;$(LIBXSLT_DIR)\bin;$(ICU_BIN);$(PATH)
	$(TCL) $(WINDIR)\run-tests.tcl "$(TESTDIR)" $(TEST_PROGRAMS)

test: check
