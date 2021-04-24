# Bazel BUILD file for YAZ library
#
# Build library:
#  bazel build //src:all
# Build everything:
#  bazel build //...

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("util.bzl", "cplush", "c_dir", "h_dir")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]
INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

LIBS_ICU = [ "-licui18n", "-licuuc", "-licudata" ]
INCLUDES_ICU = []

Z3950_FILES = ["z-core", "z-diag1", "z-exp", "z-sutrs", "z-opac",
    "z-sum", "z-grs", "z-estask", "z-rrf1", "z-rrf2", "z-accform1",
    "z-accdes1", "z-acckrb1", "zes-pset", "zes-pquery", "zes-psched",
    "zes-order", "zes-update0", "zes-exps", "zes-expi", "z-uifr1",
    "z-espec1"]

genrule(
    name = "oidtoc",
    srcs = [ "oid.csv" ],
    outs = cplush([ "oid_std"]),
    cmd = "tclsh $(location oidtoc.tcl) $(location oid.csv) $(location oid_std.c) $(location yaz/oid_std.h)",
    tools = [ "oidtoc.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "marc8",
    srcs = [ "codetables.xml" ],
    outs = [ "marc8.c" ],
    cmd = "tclsh $(location charconv.tcl)  -p marc8 $(location codetables.xml) -o $(location marc8.c)",
    tools = [ "charconv.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "marc8r",
    srcs = [ "codetables.xml" ],
    outs = [ "marc8r.c" ],
    cmd = "tclsh $(location charconv.tcl) -r -p marc8r $(location codetables.xml) -o $(location marc8r.c)",
    tools = [ "charconv.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "iso5426",
    srcs = [ "codetables-iso5426.xml" ],
    outs = [ "iso5426.c" ],
    cmd = "tclsh $(location charconv.tcl) -p iso5426 $(location codetables-iso5426.xml) -o $(location iso5426.c)",
    tools = [ "charconv.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "diagbib1",
    srcs = [ "bib1.csv" ],
    outs = cplush([ "diagbib1" ]),
    cmd = "tclsh $(location csvtodiag.tcl) $(location bib1.csv) $(location diagbib1.c) $(location yaz/diagbib1.h) bib1 diagbib1_str",
    tools = [ "csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "diagsrw",
    srcs = [ "srw.csv" ],
    outs = cplush([ "diagsrw" ]),
    cmd = "tclsh $(location csvtodiag.tcl) $(location srw.csv) $(location diagsrw.c) $(location yaz/diagsrw.h) srw",
    tools = [ "csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "diagsru_update",
    srcs = [ "sru_update.csv" ],
    outs = cplush([ "diagsru_update" ]),
    cmd = "tclsh $(location csvtodiag.tcl) $(location sru_update.csv) $(location diagsru_update.c) $(location yaz/diagsru_update.h) sru_update",
    tools = [ "csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "z3950",
    srcs = [ "z3950v3.asn", "z.tcl" ],
    outs = cplush(Z3950_FILES),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location z-core.c)\" -I \"2 $(location yaz/z-core.h)\" $(location z3950v3.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "datetime",
    srcs = [ "datetime.asn", "z.tcl" ],
    outs = cplush(["z-date"]),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location z-date.c)\" -I \"2 $(location yaz/z-date.h)\" $(location datetime.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "univres",
    srcs = [ "univres.asn", "z.tcl" ],
    outs = cplush(["z-univ"]),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location z-univ.c)\" -I \"2 $(location yaz/z-univ.h)\" $(location univres.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "esupdate",
    srcs = [ "esupdate.asn", "z.tcl" ],
    outs = cplush(["zes-update"]),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location zes-update.c)\" -I \"2 $(location yaz/zes-update.h)\" $(location esupdate.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "esadmin",
    srcs = [ "esadmin.asn", "z.tcl" ],
    outs = cplush(["zes-admin"]),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location zes-admin.c)\" -I \"2 $(location yaz/zes-admin.h)\" $(location esadmin.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "charneg",
    srcs = [ "charneg-3.asn", "z.tcl" ],
    outs = cplush(["z-charneg"]),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location z-charneg.c)\" -I \"2 $(location yaz/z-charneg.h)\" $(location charneg-3.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "mterm2",
    srcs = [ "mterm2.asn", "z.tcl" ],
    outs = cplush(["z-mterm2"]),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location z-mterm2.c)\" -I \"2 $(location yaz/z-mterm2.h)\" $(location mterm2.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "oclcui",
    srcs = [ "oclcui.asn", "z.tcl" ],
    outs = cplush(["z-oclcui"]),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location z-oclcui.c)\" -I \"2 $(location yaz/z-oclcui.h)\" $(location oclcui.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "facet",
    srcs = [ "facet.asn", "z.tcl" ],
    outs = cplush(["z-facet-1"]),
    cmd = "$(location yaz-asncomp) -d $(location z.tcl) -i yaz -C \"1 $(location z-facet-1.c)\" -I \"2 $(location yaz/z-facet-1.h)\" $(location facet.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "ill-core",
    srcs = [ "ill9702.asn", "ill.tcl" ],
    outs = cplush(["ill-core"]),
    cmd = "$(location yaz-asncomp) -d $(location ill.tcl) -i yaz -C \"1 $(location ill-core.c)\" -I \"2 $(location yaz/ill-core.h)\" $(location ill9702.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "oclc-ill-req-ext",
    srcs = [ "oclc-ill-req-ext.asn", "ill.tcl" ],
    outs = cplush(["oclc-ill-req-ext"]),
    cmd = "$(location yaz-asncomp) -d $(location ill.tcl) -i yaz -C \"1 $(location oclc-ill-req-ext.c)\" -I \"2 $(location yaz/oclc-ill-req-ext.h)\" $(location oclc-ill-req-ext.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "item-req",
    srcs = [ "item-req.asn", "ill.tcl" ],
    outs = cplush(["item-req"]),
    cmd = "$(location yaz-asncomp) -d $(location ill.tcl) -i yaz -C \"1 $(location item-req.c)\" -I \"2 $(location yaz/item-req.h)\" $(location item-req.asn)",
    tools = [ "yaz-asncomp" ],
)

genrule(
    name = "cql",
    srcs = [ "cql.y" ],
    outs = [ "cql.c" ],
    cmd = "bison -p cql_ $(location cql.y) -o $(location cql.c)",
)

cc_library(
    name = "yaz",
    includes = [ "." ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = c_dir(".", Z3950_FILES + ["oid_std", "marc8",
	 "marc8r", "iso5426", "diagbib1", "diagsrw", "diagsru_update",
	 "cql", "z-date", "z-univ", "zes-update", "zes-admin",
	 "z-charneg", "z-mterm2", "z-oclcui", "z-facet-1", "ill-core",
	 "oclc-ill-req-ext", "item-req", "base64", "version", "options",
	 "log", "cookie", "marcdisp", "marc_read_json", "marc_read_xml",
	 "marc_read_iso2709",  "marc_read_line", "wrbuf", "wrbuf_sha1",
	 "malloc_info", "oid_db", "errno", "nmemsdup", "xmalloc", "readconf",
	 "tpath", "nmem", "matchstr", "atoin", "siconv", "utf8", "ucs4",
	 "iso5428", "advancegreek", "odr_bool", "ber_bool", "ber_len",
	 "ber_tag", "odr_util", "facet", "odr_null", "ber_null", "odr_int",
	 "ber_int", "odr_tag", "odr_cons", "odr_seq", "odr_oct", "ber_oct",
	 "odr_bit", "ber_bit", "odr_oid", "ber_oid", "odr_use", "odr_choice",
	 "odr_any", "ber_any", "odr", "odr_mem", "dumpber", "odr_enum",
	 "comstack", "tcpip", "unix", "prt-ext", "proxunit",
	 "ill-get", "zget", "yaz-ccl", "diag-entry", "logrpn", "otherinfo",
	 "pquery", "sortspec", "charneg", "initopt", "init_diag",
	 "init_globals", "zoom-c", "zoom-memcached", "zoom-z3950", "zoom-sru",
	 "zoom-query", "zoom-record-cache", "zoom-event", "record_render",
	 "zoom-socket", "zoom-opt", "sru_facet",
	 "grs1disp", "zgdu", "soap", "srw", "srwutil", "uri", "solr",
	 "diag_map", "opac_to_xml", "xml_add", "xml_match", "xml_to_opac",
	 "cclfind", "ccltoken", "cclerrms", "cclqual", "cclptree", "cclqfile",
	 "cclstr", "cclxmlconfig", "ccl_stop_words", "cqlstdio",
	 "cqltransform", "cqlutil", "xcqlutil", "cqlstring", "cql_sortkeys",
	 "cql2ccl", "rpn2cql", "rpn2solr", "solrtransform", "cqlstrer",
	 "querytowrbuf", "tcpdchk", "test", "timing", "xml_get", "xmlquery",
	 "xmlerror", "http", "mime", "oid_util", "tokenizer",
	 "record_conv", "retrieval", "elementset", "snprintf",
	 "query-charset", "copy_types", "match_glob", "poll", "daemon",
	 "iconv_encode_danmarc", "iconv_encode_marc8",
	 "iconv_encode_iso_8859_1", "iconv_encode_wchar",
	 "iconv_decode_marc8", "iconv_decode_iso5426",
	 "iconv_decode_danmarc", "sc", "json", "xml_include", "file_glob",
	 "dirent", "mutex", "condvar", "thread_id", "gettimeofday",
	 "thread_create", "spipe", "url", "backtrace"
	 ])
	 + h_dir(".", ["cclp", "iconv-p", "mime", "mutex-p",
	   "odr-priv", "sru-p", "zoom-p", "config", "diag-entry"
	 ])
	 ,
    hdrs = h_dir("yaz", Z3950_FILES + [
	 "oid_std", "diagbib1", "diagsrw",
	 "diagsru_update", "z-date", "z-univ", "zes-update", "zes-admin",
	 "z-charneg", "z-mterm2", "z-oclcui", "z-facet-1", "ill-core",
	 "oclc-ill-req-ext", "item-req"
	 ])
	 + glob(["yaz/*.h"]),
    visibility = [ "//visibility:public" ],
)

exports_files(glob(["*.csv", "*.tcl"]))

cc_library(
    name = "yaz_server",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = c_dir(".", ["statserv", "seshigh", "eventl", "requestq"])
         + h_dir(".", ["eventl", "session"]),
    hdrs = [],
    visibility = [ "//visibility:public" ],
    deps = [
        ":yaz",
    ],
)

cc_library(
    name = "yaz_icu",
    includes = [ "include", "libstemmer_c/include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT + INCLUDES_ICU,
    linkopts = LIBS_EXT + LIBS_ICU,
    local_defines = [ "HAVE_CONFIG_H", "YAZ_HAVE_ICU=1" ],
    srcs = c_dir(".", [
	 "icu_chain", "icu_utf16", "icu_utf8", "stemmer",
	 "icu_transform", "icu_casemap", "icu_tokenizer", "icu_sortkey"
	 ]) + [
	 "libstemmer_c/include/libstemmer.h",
	 "libstemmer_c/libstemmer/libstemmer.c",
	 "libstemmer_c/libstemmer/modules.h",
	 "libstemmer_c/runtime/api.c",
	 "libstemmer_c/runtime/api.h",
	 "libstemmer_c/runtime/header.h",
	 "libstemmer_c/runtime/utilities.c",
	 "libstemmer_c/src_c/stem_UTF_8_porter.c",
	 "libstemmer_c/src_c/stem_UTF_8_porter.h",
	 "libstemmer_c/src_c/stem_UTF_8_english.c",
	 "libstemmer_c/src_c/stem_UTF_8_english.h",
    ],
    hdrs = [],
    visibility = [ "//visibility:public" ],
    deps = [
        ":yaz",
    ],
)
