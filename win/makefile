# This file is part of the YAZ toolkit.
# Copyright (C) Index Data

# Parameters

DEBUG=0   # 0 for release, 1 for debug
BARCH = 64 # 32=win32/x86 or 64=x64

# TCL
TCL=tclsh
#TCL="C:\Tcl\bin\tclsh85.exe"
HAVE_TCL=1

# iconv charcter conversion utility
HAVE_ICONV=0
ICONV_DIR = c:\iconv-1.9.2.win$(BARCH)

# icu charcter conversion utility
# get icu libraries from http://www.icu-project.org
HAVE_ICU=1
ICU_VER=77
ICU_BASE = c:\icu4c-$(ICU_VER)_1-Win$(BARCH)-MSVC2022
!if $(BARCH) == 32
ICU_LIB = $(ICU_BASE)\lib
ICU_BIN = $(ICU_BASE)\bin
!elseif $(BARCH) == 64
ICU_LIB = $(ICU_BASE)\lib64
ICU_BIN = $(ICU_BASE)\bin64
!else
!error BARCH must be 32 or 64
!endif
ICU_INCLUDE = $(ICU_BASE)\include

# libxslt
HAVE_LIBXSLT=1
LIBXSLT_DIR=c:\libxslt-1.1.28.win$(BARCH)

# libxml2
HAVE_LIBXML2=1
LIBXML2_DIR=c:\libxml2-2.9.2.win$(BARCH)

# get WIN32 binaries libxml2 & iconv & zlib from here:
#  http://www.zlatkovic.com/libxml.en.html

# bison
HAVE_BISON=1
BISON=bison

default: all

all: dirs generate dll client ztest yazicu zoomsh utilprog \
 testprog iconv icu libxml2 libxslt yaz_url

NSIS="c:\program files (x86)\nsis\makensis.exe"
HHC="c:\program files (x86)\html help workshop\hhc.exe"

YAZ4J_DIR="..\..\yaz4j"

YAZPATH=$(MAKEDIR)\..


dist32: yaz.nsi version.nsi distclean
	nmake make32
	nmake nsis32

dist64: yaz.nsi version.nsi distclean
	nmake make64 yaz4j
	nmake nsis64

make32:
	nmake DEBUG=0 BARCH=32 all

make64:
	nmake DEBUG=0 BARCH=64 all

distclean:
	nmake DEBUG=1 clean
	nmake DEBUG=0 clean

nsis32: version.nsi
	$(NSIS) /DVSARCH=x86 yaz.nsi

nsis64: version.nsi
	$(NSIS) /DVSARCH=x64 yaz.nsi

nsishelp:
	$(NSIS)

# Directories
# The current directory is supposed to be something like
# ..../yaz/win, everything is relative to that
ROOTDIR=..   # The home of yaz

LIBDIR=$(ROOTDIR)\lib       # We produce .lib, .exp etc there
BINDIR=$(ROOTDIR)\bin       # We produce exes and dlls there
WINDIR=$(ROOTDIR)\win       # all these Win make things
SRCDIR=$(ROOTDIR)\src       # for the case we move them under src
INCLDIR=$(SRCDIR)           # our includes
DOCDIR=$(ROOTDIR)\doc       # where the doc is
JAVADIR=$(ROOTDIR)\java
M4DIR=$(ROOTDIR)\m4

# where we store intermediate files
!if $(DEBUG)
OBJDIR=$(WINDIR)\dobj
!else
OBJDIR=$(WINDIR)\obj
!endif

CLIENTDIR=$(ROOTDIR)\CLIENT
ZTESTDIR=$(ROOTDIR)\ZTEST
ZOOMDIR=$(ROOTDIR)\ZOOM
UTILDIR=$(ROOTDIR)\UTIL
TESTDIR=$(ROOTDIR)\TEST

# TMPDIR=$(ROOTDIR)\win\tmp
# TMP=$(TMPDIR)

# Targets - what to make

!if $(DEBUG)
YAZ_DLL=$(BINDIR)\yaz5d.dll
YAZ_IMPLIB=$(LIBDIR)\yaz5d.lib
YAZ_ICU_DLL=$(BINDIR)\yaz_icu5d.dll
YAZ_ICU_IMPLIB=$(LIBDIR)\yaz_icu5d.lib
YAZ_COND_DLL=$(BINDIR)\yaz_cond5d.dll
YAZ_COND_IMPLIB=$(LIBDIR)\yaz_cond5d.lib
!else
YAZ_DLL=$(BINDIR)\yaz5.dll
YAZ_IMPLIB=$(LIBDIR)\yaz5.lib
YAZ_ICU_DLL=$(BINDIR)\yaz_icu5.dll
YAZ_ICU_IMPLIB=$(LIBDIR)\yaz_icu5.lib
YAZ_COND_DLL=$(BINDIR)\yaz_cond5.dll
YAZ_COND_IMPLIB=$(LIBDIR)\yaz_cond5.lib
!endif

CLIENT=$(BINDIR)\yaz-client.exe
YAZ_ICU=$(BINDIR)\yaz-icu.exe
YAZ_URL=$(BINDIR)\yaz-url.exe
ZOOMSH=$(BINDIR)\zoomsh.exe
ZTEST=$(BINDIR)\yaz-ztest.exe

ZOOMTST1=$(BINDIR)\zoomtst1.exe
ZOOMTST2=$(BINDIR)\zoomtst2.exe
ZOOMTST3=$(BINDIR)\zoomtst3.exe
ZOOMTST4=$(BINDIR)\zoomtst4.exe
ZOOMTST5=$(BINDIR)\zoomtst5.exe
ZOOMTST6=$(BINDIR)\zoomtst6.exe
ZOOMTST7=$(BINDIR)\zoomtst7.exe
ZOOMTST8=$(BINDIR)\zoomtst8.exe
ZOOMTST9=$(BINDIR)\zoomtst9.exe
ZOOMTST10=$(BINDIR)\zoomtst10.exe

CQL2PQF=$(BINDIR)\cql2pqf.exe
CQL2XCQL=$(BINDIR)\cql2xcql.exe
YAZ_MARCDUMP=$(BINDIR)\yaz-marcdump.exe
YAZ_ICONV=$(BINDIR)\yaz-iconv.exe
YAZ_RECORD_CONV=$(BINDIR)\yaz-record-conv.exe
YAZ_JSON_PARSE=$(BINDIR)\yaz-json-parse.exe

TSTLOG=$(BINDIR)\test_log.exe
TST_TIMING=$(BINDIR)\test_timing.exe
TEST_MUTEX=$(BINDIR)\test_mutex.exe

# shortcut names defined here
dll: dirs generate $(YAZ_DLL) $(YAZ_COND_DLL)
client: dirs generate $(CLIENT)
ztest: dirs generate $(ZTEST)

zoomsh: $(ZOOMSH) $(ZOOMTST1) $(ZOOMTST2) $(ZOOMTST3) \
 $(ZOOMTST4) $(ZOOMTST5) $(ZOOMTST6) $(ZOOMTST7) $(ZOOMTST8) $(ZOOMTST9) \
 $(ZOOMTST10)

yaz_url: $(YAZ_URL)

utilprog: $(CQL2PQF) $(CQL2XCQL) $(YAZ_MARCDUMP) $(YAZ_ICONV) $(YAZ_RECORD_CONV) $(YAZ_JSON_PARSE)
testprog: $(TSTLOG) $(TST_TIMING) $(TEST_MUTEX)

htmlhelp: $(DOCDIR)\htmlhelp.chm

# Modules and conditional flags, etc

!if $(HAVE_ICONV)
ICONV_DEF= \
  /D HAVE_ICONV_H=1 \
  /I"$(ICONV_DIR)\include"
ICONV_LIB= \
  "$(ICONV_DIR)\lib\iconv.lib"
iconv: $(BINDIR)\iconv.dll

$(BINDIR)\iconv.dll:
  copy "$(ICONV_DIR)\bin\iconv.dll" $(BINDIR)
!else
ICONV_DEF= \
  /D HAVE_ICONV_H=0
ICONV_LIB=
iconv:

!endif

!if $(HAVE_ICU)
ICU_DEF= \
  /D YAZ_HAVE_ICU=1 /D HAVE_ICU_H=1 /I"$(ICU_INCLUDE)"
ICU_LIBS= \
  $(ICU_LIB)\icudt.lib $(ICU_LIB)\icuin.lib $(ICU_LIB)\icuuc.lib
icu: $(BINDIR)\icudt$(ICU_VER).dll $(BINDIR)\icuin$(ICU_VER).dll $(BINDIR)\icuuc$(ICU_VER).dll
yazicu: dirs generate $(YAZ_ICU)

$(BINDIR)\icudt$(ICU_VER).dll:
  copy "$(ICU_BIN)\icudt$(ICU_VER).dll" $(BINDIR)
$(BINDIR)\icuin$(ICU_VER).dll:
  copy "$(ICU_BIN)\icuin$(ICU_VER).dll" $(BINDIR)
$(BINDIR)\icuuc$(ICU_VER).dll:
  copy "$(ICU_BIN)\icuuc$(ICU_VER).dll" $(BINDIR)
!else
ICU_DEF= /D YAZ_HAVE_ICU=0
ICU_LIBS=
icu:
yazicu:
!endif

###

!if $(HAVE_LIBXML2)
LIBXML2_LIB="$(LIBXML2_DIR)\lib\libxml2.lib"
LIBXML2_DEF=/D YAZ_HAVE_XML2=1 /I"$(LIBXML2_DIR)\include\libxml2"
libxml2: $(BINDIR)\libxml2.dll

$(BINDIR)\libxml2.dll:
  copy "$(LIBXML2_DIR)\bin\libxml2.dll" $(BINDIR)
!else
LIBXML2_LIB=
LIBXML2_DEF=/D YAZ_HAVE_XML2=0
libxml2:

!endif

!if $(HAVE_LIBXSLT)
LIBXSLT_LIB="$(LIBXSLT_DIR)\lib\libxslt.lib"
LIBXSLT_DEF= \
	/D HAVE_XSLTSAVERESULTTOSTRING=1 \
	/D YAZ_HAVE_XSLT=1 \
	/I"$(LIBXSLT_DIR)\include"
libxslt: $(BINDIR)\libxslt.dll

$(BINDIR)\libxslt.dll:
  copy "$(LIBXSLT_DIR)\lib\libxslt.dll" $(BINDIR)

!else
LIBXSLT_LIB=
LIBXSLT_DEF=/D YAZ_HAVE_XSLT=0
libxslt:
!endif


### C and CPP compiler  (the same thing)
# Note: $(CPP) has already been defined in the environment
# (if you set things up right!)

COMMON_C_OPTIONS=          \
  /nologo /W3 /EHsc /FD /c   \
  $(ICONV_DEF)             \
   $(ICU_DEF)             \
  $(LIBXML2_DEF)           \
  $(LIBXSLT_DEF)           \
  /D "_CRT_SECURE_NO_DEPRECATE" \
  /D "_CRT_NONSTDC_NO_DEPRECATE" \
  /D "_WINDOWS"            \
  /D "WIN32"               \
  /FR"$(OBJDIR)\\"         \
  /Fo"$(OBJDIR)\\"         \
  /Fd"$(OBJDIR)\\"

COMMON_C_INCLUDES= \
  /I"$(INCLDIR)" \
  /I"$(SRCDIR)\libstemmer_c\include"

DEBUG_C_OPTIONS=  \
  /D "_DEBUG"      \
  /MDd  /Od /Zi /Gm

RELEASE_C_OPTIONS=  \
  /D "NDEBUG"        \
  /MD /O2

MVN_ARGS=-Dyaz.path="$(YAZPATH)" -Dyaz.cflags="-I$(YAZPATH)\src"
# without >log below, mvn install does NOT work
yaz4j: $(JAVADIR)
	copy $(BINDIR)\*.dll $(YAZ4J_DIR)
	cd $(YAZ4J_DIR)
	cmd /C mvn $(MVN_ARGS) clean package
	copy /Y target\native\yaz4j*.dll "$(MAKEDIR)\..\bin"
	copy /Y target\yaz4j*.jar "$(MAKEDIR)\..\java"

### Linker options
LINK=link.exe

LINK_LIBS= kernel32.lib ws2_32.lib advapi32.lib \
	   $(ICONV_LIB) $(LIBXML2_LIB) $(LIBXSLT_LIB)

COMMON_LNK_OPTIONS= /nologo /subsystem:windows /incremental:no

DEBUG_LNK_OPTIONS= /debug

RELEASE_LNK_OPTIONS=

DLL_LINK_OPTIONS= /dll
CLIENT_LINK_OPTIONS = /subsystem:console

COMMON_TCL_OPTIONS= yaz-asncomp -I$(INCLDIR) -i yaz

# Final opt variables
!if $(DEBUG)
COPT=   $(COMMON_C_OPTIONS)   $(DEBUG_C_OPTIONS)     $(COMMON_C_INCLUDES)
LNKOPT= $(COMMON_LNK_OPTIONS) $(DEBUG_LNK_OPTIONS)   $(LNK_LIBS)
TCLOPT= $(COMMON_TCL_OPTIONS)

!else
COPT=   $(COMMON_C_OPTIONS)   $(RELEASE_C_OPTIONS)   $(COMMON_C_INCLUDES)
LNKOPT= $(COMMON_LNK_OPTIONS) $(RELEASE_LNK_OPTIONS) $(LNK_LIBS)
TCLOPT= $(COMMON_TCL_OPTIONS)
!endif

LINK_PROGRAM = $(LINK) $(LNKOPT) \
		$(CLIENT_LINK_OPTIONS) $(LINK_LIBS) $(YAZ_IMPLIB)

LINK_DLL = $(LINK) $(LNKOPT) \
		$(LINK_LIBS) $(DLL_LINK_OPTIONS)

# Source and object modules

# Note: Ordinary source files are not specified here at
# all, make finds them in suitable dirs. The object modules
# need to be specified, though

YAZ_CLIENT_OBJS= \
   $(OBJDIR)\client.obj \
   $(OBJDIR)\tabcomplete.obj \
   $(OBJDIR)\admin.obj \
   $(OBJDIR)\fhistory.obj

YAZ_ICU_OBJS= $(OBJDIR)\yaz-icu.obj
YAZ_URL_OBJS= $(OBJDIR)\yaz-url.obj
COND_DLL_OBJS= $(OBJDIR)\condvar.obj

ZTEST_OBJS= \
    $(OBJDIR)\dummy-opac.obj \
    $(OBJDIR)\read-marc.obj \
    $(OBJDIR)\read-grs.obj \
    $(OBJDIR)\ztest.obj

YAZ_ZOOMSH_OBJS = \
   $(OBJDIR)\zoomsh.obj

YAZ_ZOOMTST1_OBJS = \
   $(OBJDIR)\zoomtst1.obj
YAZ_ZOOMTST2_OBJS = \
   $(OBJDIR)\zoomtst2.obj
YAZ_ZOOMTST3_OBJS = \
   $(OBJDIR)\zoomtst3.obj
YAZ_ZOOMTST4_OBJS = \
   $(OBJDIR)\zoomtst4.obj
YAZ_ZOOMTST5_OBJS = \
   $(OBJDIR)\zoomtst5.obj
YAZ_ZOOMTST6_OBJS = \
   $(OBJDIR)\zoomtst6.obj
YAZ_ZOOMTST7_OBJS = \
   $(OBJDIR)\zoomtst7.obj
YAZ_ZOOMTST8_OBJS = \
   $(OBJDIR)\zoomtst8.obj
YAZ_ZOOMTST9_OBJS = \
   $(OBJDIR)\zoomtst9.obj
YAZ_ZOOMTST10_OBJS = \
   $(OBJDIR)\zoomtst10.obj

YAZ_CQL2PQF_OBJS = \
   $(OBJDIR)\cql2pqf.obj
YAZ_CQL2XCQL_OBJS = \
   $(OBJDIR)\cql2xcql.obj
YAZ_MARCDUMP_OBJS = \
   $(OBJDIR)\marcdump.obj
YAZ_ICONV_OBJS = \
   $(OBJDIR)\yaziconv.obj
YAZ_RECORD_CONV_OBJS = \
   $(OBJDIR)\yaz-record-conv.obj
YAZ_JSON_PARSE_OBJS = \
   $(OBJDIR)\json-parse.obj

TSTLOG_OBJS = \
   $(OBJDIR)\test_log.obj

TST_TIMING_OBJS = \
   $(OBJDIR)\test_timing.obj

TEST_MUTEX_OBJS = \
   $(OBJDIR)\test_mutex.obj

MISC_OBJS= \
   $(OBJDIR)\diag_map.obj \
   $(OBJDIR)\base64.obj \
   $(OBJDIR)\version.obj \
   $(OBJDIR)\oid_std.obj \
   $(OBJDIR)\eventl.obj \
   $(OBJDIR)\requestq.obj \
   $(OBJDIR)\seshigh.obj \
   $(OBJDIR)\statserv.obj \
   $(OBJDIR)\tcpdchk.obj \
   $(OBJDIR)\cclerrms.obj \
   $(OBJDIR)\cclfind.obj \
   $(OBJDIR)\cclptree.obj \
   $(OBJDIR)\cclqfile.obj \
   $(OBJDIR)\cclqual.obj \
   $(OBJDIR)\cclstr.obj \
   $(OBJDIR)\ccltoken.obj \
   $(OBJDIR)\cclxmlconfig.obj \
   $(OBJDIR)\ccl_stop_words.obj \
   $(OBJDIR)\comstack.obj \
   $(OBJDIR)\tcpip.obj \
   $(OBJDIR)\ber_any.obj \
   $(OBJDIR)\ber_bit.obj \
   $(OBJDIR)\ber_bool.obj \
   $(OBJDIR)\ber_int.obj \
   $(OBJDIR)\ber_len.obj \
   $(OBJDIR)\ber_null.obj \
   $(OBJDIR)\ber_oct.obj \
   $(OBJDIR)\ber_oid.obj \
   $(OBJDIR)\ber_tag.obj \
   $(OBJDIR)\dumpber.obj \
   $(OBJDIR)\errno.obj \
   $(OBJDIR)\odr.obj \
   $(OBJDIR)\odr_any.obj \
   $(OBJDIR)\odr_bit.obj \
   $(OBJDIR)\odr_bool.obj \
   $(OBJDIR)\odr_choice.obj \
   $(OBJDIR)\odr_cons.obj \
   $(OBJDIR)\odr_enum.obj \
   $(OBJDIR)\odr_int.obj \
   $(OBJDIR)\odr_mem.obj \
   $(OBJDIR)\odr_null.obj \
   $(OBJDIR)\odr_oct.obj \
   $(OBJDIR)\odr_oid.obj \
   $(OBJDIR)\odr_seq.obj \
   $(OBJDIR)\odr_tag.obj \
   $(OBJDIR)\odr_use.obj \
   $(OBJDIR)\odr_util.obj \
   $(OBJDIR)\atoin.obj \
   $(OBJDIR)\log.obj \
   $(OBJDIR)\malloc_info.obj \
   $(OBJDIR)\marcdisp.obj \
   $(OBJDIR)\marc_read_sax.obj \
   $(OBJDIR)\marc_read_json.obj \
   $(OBJDIR)\marc_read_xml.obj \
   $(OBJDIR)\marc_read_iso2709.obj \
   $(OBJDIR)\marc_read_line.obj \
   $(OBJDIR)\nmem.obj \
   $(OBJDIR)\nmemsdup.obj \
   $(OBJDIR)\oid_db.obj \
   $(OBJDIR)\oid_util.obj \
   $(OBJDIR)\options.obj \
   $(OBJDIR)\readconf.obj \
   $(OBJDIR)\tpath.obj \
   $(OBJDIR)\wrbuf.obj \
   $(OBJDIR)\wrbuf_sha1.obj \
   $(OBJDIR)\xmalloc.obj \
   $(OBJDIR)\matchstr.obj \
   $(OBJDIR)\siconv.obj \
   $(OBJDIR)\iso5428.obj \
   $(OBJDIR)\utf8.obj \
   $(OBJDIR)\ucs4.obj \
   $(OBJDIR)\advancegreek.obj \
   $(OBJDIR)\snprintf.obj \
   $(OBJDIR)\marc8.obj \
   $(OBJDIR)\marc8r.obj \
   $(OBJDIR)\iso5426.obj \
   $(OBJDIR)\record_conv.obj \
   $(OBJDIR)\retrieval.obj \
   $(OBJDIR)\test.obj \
   $(OBJDIR)\diagbib1.obj \
   $(OBJDIR)\diagsru_update.obj \
   $(OBJDIR)\diagsrw.obj \
   $(OBJDIR)\diag-entry.obj \
   $(OBJDIR)\zget.obj \
   $(OBJDIR)\logrpn.obj \
   $(OBJDIR)\querytowrbuf.obj \
   $(OBJDIR)\pquery.obj \
   $(OBJDIR)\yaz-ccl.obj \
   $(OBJDIR)\otherinfo.obj \
   $(OBJDIR)\sortspec.obj \
   $(OBJDIR)\charneg.obj \
   $(OBJDIR)\grs1disp.obj \
   $(OBJDIR)\opac_to_xml.obj \
   $(OBJDIR)\xml_get.obj \
   $(OBJDIR)\xml_add.obj \
   $(OBJDIR)\xml_match.obj \
   $(OBJDIR)\xml_to_opac.obj \
   $(OBJDIR)\zgdu.obj \
   $(OBJDIR)\soap.obj \
   $(OBJDIR)\solr.obj \
   $(OBJDIR)\solrtransform.obj \
   $(OBJDIR)\sru_facet.obj \
   $(OBJDIR)\srw.obj \
   $(OBJDIR)\srwutil.obj \
   $(OBJDIR)\zoom-c.obj \
   $(OBJDIR)\zoom-event.obj \
   $(OBJDIR)\zoom-memcached.obj \
   $(OBJDIR)\zoom-record-cache.obj \
   $(OBJDIR)\zoom-z3950.obj \
   $(OBJDIR)\zoom-sru.obj \
   $(OBJDIR)\zoom-query.obj \
   $(OBJDIR)\record_render.obj \
   $(OBJDIR)\facet.obj \
   $(OBJDIR)\zoom-opt.obj \
   $(OBJDIR)\zoom-socket.obj \
   $(OBJDIR)\initopt.obj \
   $(OBJDIR)\init_diag.obj \
   $(OBJDIR)\init_globals.obj \
   $(OBJDIR)\xmlquery.obj \
   $(OBJDIR)\xmlerror.obj \
   $(OBJDIR)\mime.obj \
   $(OBJDIR)\cookie.obj \
   $(OBJDIR)\cql.obj \
   $(OBJDIR)\cql2ccl.obj \
   $(OBJDIR)\cql_sortkeys.obj \
   $(OBJDIR)\cqlstdio.obj \
   $(OBJDIR)\cqlstring.obj \
   $(OBJDIR)\cqltransform.obj \
   $(OBJDIR)\cqlutil.obj \
   $(OBJDIR)\cqlstrer.obj \
   $(OBJDIR)\rpn2cql.obj \
   $(OBJDIR)\rpn2solr.obj \
   $(OBJDIR)\xcqlutil.obj \
   $(OBJDIR)\elementset.obj \
   $(OBJDIR)\timing.obj \
   $(OBJDIR)\uri.obj \
   $(OBJDIR)\query-charset.obj \
   $(OBJDIR)\tokenizer.obj \
   $(OBJDIR)\copy_types.obj \
   $(OBJDIR)\http.obj \
   $(OBJDIR)\match_glob.obj \
   $(OBJDIR)\poll.obj \
   $(OBJDIR)\daemon.obj \
   $(OBJDIR)\iconv_encode_danmarc.obj \
   $(OBJDIR)\iconv_encode_iso_8859_1.obj \
   $(OBJDIR)\iconv_encode_marc8.obj \
   $(OBJDIR)\iconv_decode_marc8.obj \
   $(OBJDIR)\iconv_encode_wchar.obj \
   $(OBJDIR)\iconv_decode_iso5426.obj \
   $(OBJDIR)\iconv_decode_danmarc.obj \
   $(OBJDIR)\mutex.obj \
   $(OBJDIR)\thread_create.obj \
   $(OBJDIR)\spipe.obj \
   $(OBJDIR)\gettimeofday.obj \
   $(OBJDIR)\json.obj \
   $(OBJDIR)\sc.obj \
   $(OBJDIR)\xml_include.obj \
   $(OBJDIR)\file_glob.obj \
   $(OBJDIR)\thread_id.obj \
   $(OBJDIR)\dirent.obj \
   $(OBJDIR)\url.obj \
   $(OBJDIR)\proxunit.obj \
   $(OBJDIR)\backtrace.obj

Z3950_OBJS= \
   $(OBJDIR)\z-date.obj\
   $(OBJDIR)\z-univ.obj\
   $(OBJDIR)\zes-update.obj\
   $(OBJDIR)\zes-admin.obj \
   $(OBJDIR)\z-accdes1.obj \
   $(OBJDIR)\z-accform1.obj \
   $(OBJDIR)\z-acckrb1.obj \
   $(OBJDIR)\z-core.obj \
   $(OBJDIR)\z-diag1.obj \
   $(OBJDIR)\z-espec1.obj \
   $(OBJDIR)\z-estask.obj \
   $(OBJDIR)\z-exp.obj \
   $(OBJDIR)\z-grs.obj \
   $(OBJDIR)\z-opac.obj \
   $(OBJDIR)\z-uifr1.obj \
   $(OBJDIR)\z-rrf1.obj \
   $(OBJDIR)\z-rrf2.obj \
   $(OBJDIR)\z-sum.obj \
   $(OBJDIR)\z-sutrs.obj \
   $(OBJDIR)\zes-expi.obj \
   $(OBJDIR)\zes-exps.obj \
   $(OBJDIR)\zes-order.obj \
   $(OBJDIR)\zes-pquery.obj \
   $(OBJDIR)\zes-psched.obj \
   $(OBJDIR)\zes-pset.obj \
   $(OBJDIR)\zes-update0.obj \
   $(OBJDIR)\prt-ext.obj \
   $(OBJDIR)\z-charneg.obj \
   $(OBJDIR)\z-mterm2.obj \
   $(OBJDIR)\z-oclcui.obj \
   $(OBJDIR)\z-facet-1.obj

ILL_OBJS= \
   $(OBJDIR)\ill-get.obj\
   $(OBJDIR)\ill-core.obj\
   $(OBJDIR)\item-req.obj

STEMMER_OBJS = \
   $(OBJDIR)\libstemmer.obj \
   $(OBJDIR)\api.obj \
   $(OBJDIR)\utilities.obj \
   $(OBJDIR)\stem_UTF_8_porter.obj \
   $(OBJDIR)\stem_UTF_8_english.obj

DLL_OBJS =\
   $(MISC_OBJS) \
   $(Z3950_OBJS) \
   $(ILL_OBJS)

ICU_DLL_OBJS =\
   $(OBJDIR)\icu_chain.obj \
   $(OBJDIR)\icu_utf16.obj \
   $(OBJDIR)\icu_utf8.obj \
   $(OBJDIR)\icu_transform.obj \
   $(OBJDIR)\icu_casemap.obj \
   $(OBJDIR)\icu_tokenizer.obj \
   $(OBJDIR)\icu_sortkey.obj \
   $(OBJDIR)\stemmer.obj \
   $(STEMMER_OBJS)

# Generated C and H files

Z3950_C_DIR=$(SRCDIR)
ILL_C_DIR=$(SRCDIR)
#!!! Should be moved to OBJ, but that requires too much trickery

# Files generated from datetime.asn
DATETIME_H_FILES = $(INCLDIR)\yaz\z-date.h
DATETIME_C_FILES = $(Z3950_C_DIR)\z-date.c

# Files generated from univres.asn
UNIVRES_H_FILES = $(INCLDIR)\yaz\z-univ.h
UNIVRES_C_FILES = $(Z3950_C_DIR)\z-univ.c

# Files generated from esupdate.asn
ESUPDATE_H_FILES = $(INCLDIR)\yaz\zes-update.h
ESUPDATE_C_FILES = $(Z3950_C_DIR)\zes-update.c

# Files generated from esadmin.asn
ESADMIN_H_FILES = $(INCLDIR)\yaz\zes-admin.h
ESADMIN_C_FILES = $(Z3950_C_DIR)\zes-admin.c

# Files generated from esadmin.asn
CHARNEG_H_FILES = $(INCLDIR)\yaz\z-charneg.h
CHARNEG_C_FILES = $(Z3950_C_DIR)\z-charneg.c

# Files generated from mterm2.asn
MTERM2_H_FILES = $(INCLDIR)\yaz\z-mterm2.h
MTERM2_C_FILES = $(Z3950_C_DIR)\z-mterm2.c

# Files generated from mterm2.asn
OCLCUI_H_FILES = $(INCLDIR)\yaz\z-oclcui.h
OCLCUI_C_FILES = $(Z3950_C_DIR)\z-oclcui.c

# Files generated from facet.asn
FACET_H_FILES = $(INCLDIR)\yaz\z-facet-1.h
FACET_C_FILES = $(Z3950_C_DIR)\z-facet-1.c

# Files created from z3950v3.asn
Z3950V3_H_FILES= \
   $(INCLDIR)\yaz\z-accdes1.h \
   $(INCLDIR)\yaz\z-core.h

Z3950V3_C_FILES= \
   $(Z3950_C_DIR)\z-accdes1.c \
   $(Z3950_C_DIR)\z-accform1.c \
   $(Z3950_C_DIR)\z-acckrb1.c \
   $(Z3950_C_DIR)\z-core.c \
   $(Z3950_C_DIR)\z-diag1.c \
   $(Z3950_C_DIR)\z-espec1.c \
   $(Z3950_C_DIR)\z-estask.c \
   $(Z3950_C_DIR)\z-exp.c \
   $(Z3950_C_DIR)\z-grs.c \
   $(Z3950_C_DIR)\z-opac.c \
   $(Z3950_C_DIR)\z-uifr1.c \
   $(Z3950_C_DIR)\z-rrf1.c \
   $(Z3950_C_DIR)\z-rrf2.c \
   $(Z3950_C_DIR)\z-sum.c \
   $(Z3950_C_DIR)\z-sutrs.c \
   $(Z3950_C_DIR)\zes-expi.c \
   $(Z3950_C_DIR)\zes-exps.c \
   $(Z3950_C_DIR)\zes-order.c \
   $(Z3950_C_DIR)\zes-pquery.c \
   $(Z3950_C_DIR)\zes-psched.c \
   $(Z3950_C_DIR)\zes-pset.c \
   $(Z3950_C_DIR)\zes-update0.c

# Files generated from ill9702.asn
ILL_CORE_H_FILES= \
   $(INCLDIR)\yaz\ill-core.h

ILL_CORE_C_FILES= \
   $(ILL_C_DIR)\ill-core.c

# Files generated from itemreq.asn
ITEM_REQ_H_FILES= \
   $(INCLDIR)\yaz\item-req.h

ITEM_REQ_C_FILES= \
   $(ILL_C_DIR)\item-req.c

# Combined..
DATETIME_FILES = $(DATETIME_H_FILES) $(DATETIME_C_FILES)
UNIVRES_FILES = $(UNIVRES_H_FILES) $(UNIVRES_C_FILES)
ESUPDATE_FILES = $(ESUPDATE_H_FILES) $(ESUPDATE_C_FILES)
ESADMIN_FILES = $(ESADMIN_H_FILES) $(ESADMIN_C_FILES)
Z3950V3_FILES= $(Z3950V3_C_FILES) $(Z3950V3_H_FILES)
ILL_CORE_FILES= $(ILL_CORE_C_FILES) $(ILL_CORE_H_FILES)
ITEM_REQ_FILES= $(ITEM_REQ_C_FILES) $(ITEM_REQ_H_FILES)
CHARNEG_FILES = $(CHARNEG_C_FILES) $(CHARNEG_H_FILES)
MTERM2_FILES = $(MTERM2_C_FILES) $(MTERM2_H_FILES)
OCLCUI_FILES = $(OCLCUI_C_FILES) $(OCLCUI_H_FILES)
FACET_FILES = $(FACET_C_FILES) $(FACET_H_FILES)

GENERATED_C_FILES= \
   $(Z3950V3_C_FILES)  \
   $(ESUPDATE_C_FILES) \
   $(UNIVRES_C_FILES)  \
   $(DATETIME_C_FILES) \
   $(ESADMIN_C_FILES) \
   $(CHARNEG_C_FILES) \
   $(MTERM2_C_FILES) \
   $(OCLCUI_C_FILES) \
   $(FACET_C_FILES) \
   $(ILL_CORE_C_FILES) \
   $(ITEM_REQ_C_FILES)

GENERATED_H_FILES= \
   $(Z3950V3_H_FILES)  \
   $(ESUPDATE_H_FILES) \
   $(UNIVRES_H_FILES)  \
   $(DATETIME_H_FILES) \
   $(ESADMIN_H_FILES) \
   $(CHARNEG_H_FILES) \
   $(MTERM2_H_FILES) \
   $(OCLCUI_H_FILES) \
   $(FACET_H_FILES) \
   $(ILL_CORE_H_FILES) \
   $(ITEM_REQ_H_FILES)

generate: \
	$(GENERATED_H_FILES) \
	$(GENERATED_C_FILES) \
	$(SRCDIR)\diagsrw.c \
	$(SRCDIR)\diagbib1.c \
	$(SRCDIR)\diagsru_update.c \
	$(INCLDIR)\yaz\yaz-version.h

# Compiling

# Note: This defines where to look for the necessary
# source files. Funny way of doing it, but it works.

# yaz client
{$(CLIENTDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< /D"_CONSOLE"

# Ztest
{$(ZTESTDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $< /D"_CONSOLE"

# Server
{$(SERVERDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $<

# Various YAZ source directories
{$(SRCDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) /D"YAZ_DLL" $<

{$(ZOOMDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $<

{$(UTILDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $<

{$(TESTDIR)}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $<

{$(SRCDIR)\libstemmer_c\libstemmer}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $<

{$(SRCDIR)\libstemmer_c\runtime}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $<

{$(SRCDIR)\libstemmer_c\src_c}.c{$(OBJDIR)}.obj:
	$(CPP) $(COPT) $<

# ASN-generated files

!if $(HAVE_TCL)

$(Z3950V3_FILES): $(SRCDIR)\z3950v3.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl z3950v3.asn
	@cd $(WINDIR)

$(DATETIME_FILES): $(SRCDIR)\datetime.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl datetime.asn
	@cd $(WINDIR)

$(UNIVRES_FILES): $(SRCDIR)\univres.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl univres.asn
	@cd $(WINDIR)

$(ESUPDATE_FILES): $(SRCDIR)\esupdate.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl esupdate.asn
	@cd $(WINDIR)

$(ESADMIN_FILES): $(SRCDIR)\esadmin.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl esadmin.asn
	@cd $(WINDIR)

$(CHARNEG_FILES): $(SRCDIR)\charneg-3.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl charneg-3.asn
	@cd $(WINDIR)

$(MTERM2_FILES): $(SRCDIR)\mterm2.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl mterm2.asn
	@cd $(WINDIR)

$(OCLCUI_FILES): $(SRCDIR)\oclcui.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl oclcui.asn
	@cd $(WINDIR)

$(FACET_FILES): $(SRCDIR)\facet.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d z.tcl facet.asn
	@cd $(WINDIR)

$(ILL_CORE_FILES): $(SRCDIR)\ill9702.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d ill.tcl ill9702.asn
	@cd $(WINDIR)

$(ITEM_REQ_FILES): $(SRCDIR)\item-req.asn
	@cd $(SRCDIR)
	$(TCL) $(TCLOPT) -d ill.tcl item-req.asn
	@cd $(WINDIR)

$(SRCDIR)\marc8.c: $(SRCDIR)\codetables.xml $(SRCDIR)\charconv.tcl
	@cd $(SRCDIR)
	$(TCL) charconv.tcl -p marc8 codetables.xml -o marc8.c

$(SRCDIR)\marc8r.c: $(SRCDIR)\codetables.xml $(SRCDIR)\charconv.tcl
	@cd $(SRCDIR)
	$(TCL) charconv.tcl -r -p marc8r codetables.xml -o marc8r.c

$(SRCDIR)\iso5426.c: $(SRCDIR)\codetables-iso5426.xml $(SRCDIR)\charconv.tcl
	@cd $(SRCDIR)
	$(TCL) charconv.tcl -r -p iso5426 codetables-iso5426.xml -o iso5426.c

$(SRCDIR)\oid_std.c: $(SRCDIR)\oid.csv
	$(TCL) $(SRCDIR)/oidtoc.tcl $(SRCDIR)\oid.csv $(SRCDIR)\oid_std.c $(INCLDIR)\yaz\oid_std.h

$(SRCDIR)\diagbib1.c: $(SRCDIR)\bib1.csv
	@cd $(SRCDIR)
	$(TCL) csvtodiag.tcl bib1.csv diagbib1.c $(INCLDIR)\yaz\diagbib1.h bib1 diagbib1_str

$(SRCDIR)\diagsrw.c: $(SRCDIR)\srw.csv
	@cd $(SRCDIR)
	$(TCL) csvtodiag.tcl srw.csv diagsrw.c $(INCLDIR)\yaz\diagsrw.h srw

$(SRCDIR)\diagsru_update.c: $(SRCDIR)\sru_update.csv
	@cd $(SRCDIR)
	$(TCL) csvtodiag.tcl sru_update.csv diagsru_update.c $(INCLDIR)\yaz\diagsru_update.h sru_update

$(INCLDIR)\yaz\yaz-version.h: $(ROOTDIR)/IDMETA
	@cd $(M4DIR)
	$(TCL) mk_version.tcl $(ROOTDIR)/IDMETA $(INCLDIR)\yaz\yaz-version.h

version.nsi: $(ROOTDIR)/IDMETA
	$(TCL) $(M4DIR)/mk_version.tcl $(ROOTDIR)/IDMETA version.nsi


!endif

!if $(HAVE_BISON)
$(SRCDIR)\cql.c: $(SRCDIR)\cql.y
	@cd $(SRCDIR)
	$(BISON) -y -p cql_ -o cql.c cql.y

!endif

# Resources
# The RC compiler (resource files)
RSC=rc.exe

YAZ_RC=$(WINDIR)\yaz.rc
YAZ_RES=$(OBJDIR)\yaz.res
YAZ_ICU_RES=$(OBJDIR)\yaz_icu.res

!if $(DEBUG)
RSOPT=/d_DEBUG
!else
RSOPT=/d_NDEBUG
!endif

$(YAZ_RES): $(YAZ_RC)
	$(RSC) $(RSOPT) /I$(INCLDIR) /fo"$(YAZ_RES)" $(YAZ_RC)

$(YAZ_ICU_RES): $(YAZ_RC)
	$(RSC) $(RSOPT) /I$(INCLDIR) /DICU=1 /fo"$(YAZ_ICU_RES)" $(YAZ_RC)

# Linking

$(YAZ_DLL) $(YAZ_IMPLIB): "$(BINDIR)" $(DLL_OBJS) $(YAZ_RES)
	$(LINK_DLL) \
		$(DLL_OBJS) \
		$(YAZ_RES) \
		/out:$@ \
		/implib:"$(YAZ_IMPLIB)"

$(YAZ_ICU_DLL) $(YAZ_ICU_IMPLIB): "$(BINDIR)" $(ICU_DLL_OBJS) $(YAZ_ICU_RES)
	$(LINK_DLL) $(ICU_LIBS) $(YAZ_IMPLIB)\
		$(ICU_DLL_OBJS) \
		$(YAZ_ICU_RES) \
		/out:$@ \
		/implib:"$(YAZ_ICU_IMPLIB)"

$(YAZ_COND_DLL) $(YAZ_COND_IMPLIB): "$(BINDIR)" $(COND_DLL_OBJS) $(YAZ_COND_RES)
	$(LINK_DLL) $(COND_LIB) $(YAZ_IMPLIB)\
		$(COND_DLL_OBJS) \
		$(YAZ_COND_RES) \
		/out:$@ \
		/implib:"$(YAZ_COND_IMPLIB)"

$(CLIENT) : "$(BINDIR)" $(YAZ_CLIENT_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_CLIENT_OBJS) /out:$@

$(YAZ_ICU) : "$(BINDIR)" $(YAZ_ICU_OBJS) $(YAZ_ICU_DLL)
	$(LINK_PROGRAM) $(ICU_LIBS) $(YAZ_ICU_IMPLIB) $(YAZ_ICU_OBJS) /out:$@

$(YAZ_URL) : "$(BINDIR)" $(YAZ_URL_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_URL_OBJS) /out:$@

$(ZOOMSH) : "$(BINDIR)" $(YAZ_ZOOMSH_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMSH_OBJS) /out:$@

$(ZOOMTST1) : "$(BINDIR)" $(YAZ_ZOOMTST1_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST1_OBJS) /out:$@

$(ZOOMTST2) : "$(BINDIR)" $(YAZ_ZOOMTST2_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST2_OBJS) /out:$@

$(ZOOMTST3) : "$(BINDIR)" $(YAZ_ZOOMTST3_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST3_OBJS) /out:$@

$(ZOOMTST4) : "$(BINDIR)" $(YAZ_ZOOMTST4_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST4_OBJS) /out:$@

$(ZOOMTST5) : "$(BINDIR)" $(YAZ_ZOOMTST5_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST5_OBJS) /out:$@

$(ZOOMTST6) : "$(BINDIR)" $(YAZ_ZOOMTST6_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST6_OBJS) /out:$@

$(ZOOMTST7) : "$(BINDIR)" $(YAZ_ZOOMTST7_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST7_OBJS) /out:$@

$(ZOOMTST8) : "$(BINDIR)" $(YAZ_ZOOMTST8_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST8_OBJS) /out:$@

$(ZOOMTST9) : "$(BINDIR)" $(YAZ_ZOOMTST9_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST9_OBJS) /out:$@

$(ZOOMTST10) : "$(BINDIR)" $(YAZ_ZOOMTST10_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ZOOMTST10_OBJS) /out:$@

$(CQL2PQF) : "$(BINDIR)" $(YAZ_CQL2PQF_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_CQL2PQF_OBJS) /out:$@

$(CQL2XCQL) : "$(BINDIR)" $(YAZ_CQL2XCQL_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_CQL2XCQL_OBJS) /out:$@

$(ZTEST) : "$(BINDIR)" $(ZTEST_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM)	$(ZTEST_OBJS) /out:$@

$(YAZ_MARCDUMP) : "$(BINDIR)" $(YAZ_MARCDUMP_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_MARCDUMP_OBJS) /out:$@

$(YAZ_ICONV) : "$(BINDIR)" $(YAZ_ICONV_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_ICONV_OBJS) /out:$@

$(YAZ_RECORD_CONV) : "$(BINDIR)" $(YAZ_RECORD_CONV_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_RECORD_CONV_OBJS) /out:$@

$(YAZ_JSON_PARSE) : "$(BINDIR)" $(YAZ_JSON_PARSE_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(YAZ_JSON_PARSE_OBJS) /out:$@

$(TSTLOG) : "$(BINDIR)" $(TSTLOG_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(TSTLOG_OBJS) /out:$@

$(TST_TIMING) : "$(BINDIR)" $(TST_TIMING_OBJS) $(YAZ_DLL)
	$(LINK_PROGRAM) $(TST_TIMING_OBJS) /out:$@

$(TEST_MUTEX) : "$(BINDIR)" $(TEST_MUTEX_OBJS) $(YAZ_COND_DLL)
	$(LINK_PROGRAM) $(YAZ_COND_IMPLIB) $(TEST_MUTEX_OBJS) /out:$@

# Other rules

$(DOCDIR)\htmlhelp.chm: $(DOCDIR)\htmlhelp.hhp
	@cd $(DOCDIR)
	-$(HHC) htmlhelp.hhp
	@cd $(WINDIR)

clean:
	-del $(BINDIR)\*.exe
	-del $(BINDIR)\*.dll
	-del $(BINDIR)\*.pdb
	-del $(TMPDIR)\*.
	-del $(LIBDIR)\*.LIB
	-del $(OBJDIR)\*.OBJ

realclean: clean
	-del $(SRCDIR)\marc8.c
	-del $(SRCDIR)\cql.c
	-del $(SRCDIR)\z-*.c
	-del $(SRCDIR)\ill-core.c
	-del $(SRCDIR)\item-req.c
	-del $(INCLDIR)\yaz\ill-core.h
	-del $(INCLDIR)\yaz\item-req.h
	-del $(INCLDIR)\yaz\z-accdes1.h
	-del $(INCLDIR)\yaz\z-core.h
	-del $(DATETIME_H_FILES)
	-del $(UNIVRES_H_FILES)
	-del $(ESUPDATE_H_FILES)

# Because DOS del will only accept one file name to delete,
# the _H_ files work only on sets that have just one file.
# Z3950_H_FILES had to be spelled out. One more point for MS!

# check directories and create if needed
dirs: $(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) $(TMPDIR)

$(OBJDIR) $(WINDIR) $(LIBDIR) $(BINDIR) $(TMPDIR) $(JAVADIR):
	if not exist "$@/$(NUL)" mkdir "$@"


# Explicit dependencies

# force recompilation of everything, if makefile changed

$(Z3950_OBJS): $(GENERATED_C_FILES) $(GENERATED_H_FILES)

$(ILL_OBJS): $(ILL_CORE_FILES) $(ITEM_REQ_FILES)

# makes sure we generate before compiling anything, as the
# new proto.h refers to the generated files, and is included
# in various places

