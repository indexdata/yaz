# Bazel BUILD file for YAZ
#
# bazel  build //:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("util.bzl", "cplush", "plush", "plusc")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]

INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

Z3950_FILES = ["z-core", "z-diag1", "z-exp", "z-sutrs", "z-opac","z-sum", "z-grs", "z-estask", "z-rrf1", "z-rrf2", "z-accform1", "z-accdes1", "z-acckrb1", "zes-pset", "zes-pquery", "zes-psched", "zes-order", "zes-update0", "zes-exps", "zes-expi", "z-uifr1", "z-espec1"]

genrule(
    name = "oidtoc",
    srcs = [ "src/oid.csv" ],
    outs = cplush([ "oid_std"]),
    cmd = "tclsh $(location src/oidtoc.tcl) $(location src/oid.csv) $(location src/oid_std.c) $(location include/yaz/oid_std.h)",
    tools = [ "src/oidtoc.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "marc8",
    srcs = [ "src/codetables.xml" ],
    outs = [ "src/marc8.c" ],
    cmd = "tclsh $(location src/charconv.tcl)  -p marc8 $(location src/codetables.xml) -o $(location src/marc8.c)",
    tools = [ "src/charconv.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "marc8r",
    srcs = [ "src/codetables.xml" ],
    outs = [ "src/marc8r.c" ],
    cmd = "tclsh $(location src/charconv.tcl) -r -p marc8r $(location src/codetables.xml) -o $(location src/marc8r.c)",
    tools = [ "src/charconv.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "iso5426",
    srcs = [ "src/codetables-iso5426.xml" ],
    outs = [ "src/iso5426.c" ],
    cmd = "tclsh $(location src/charconv.tcl) -p iso5426 $(location src/codetables-iso5426.xml) -o $(location src/iso5426.c)",
    tools = [ "src/charconv.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "diagbib1",
    srcs = [ "src/bib1.csv" ],
    outs = cplush([ "diagbib1" ]),
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/bib1.csv) $(location src/diagbib1.c) $(location include/yaz/diagbib1.h) bib1 diagbib1_str",
    tools = [ "src/csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "diagsrw",
    srcs = [ "src/srw.csv" ],
    outs = cplush([ "diagsrw" ]),
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/srw.csv) $(location src/diagsrw.c) $(location include/yaz/diagsrw.h) srw",
    tools = [ "src/csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "diagsru_update",
    srcs = [ "src/sru_update.csv" ],
    outs = cplush([ "diagsru_update" ]),
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/sru_update.csv) $(location src/diagsru_update.c) $(location include/yaz/diagsru_update.h) sru_update",
    tools = [ "src/csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "z3950",
    srcs = [ "src/z3950v3.asn", "src/z.tcl" ],
    outs = cplush(Z3950_FILES),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/z-core.c)\" -I \"2 $(location include/yaz/z-core.h)\" $(location src/z3950v3.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "datetime",
    srcs = [ "src/datetime.asn", "src/z.tcl" ],
    outs = cplush(["z-date"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/z-date.c)\" -I \"2 $(location include/yaz/z-date.h)\" $(location src/datetime.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "univres",
    srcs = [ "src/univres.asn", "src/z.tcl" ],
    outs = cplush(["z-univ"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/z-univ.c)\" -I \"2 $(location include/yaz/z-univ.h)\" $(location src/univres.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "esupdate",
    srcs = [ "src/esupdate.asn", "src/z.tcl" ],
    outs = cplush(["zes-update"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/zes-update.c)\" -I \"2 $(location include/yaz/zes-update.h)\" $(location src/esupdate.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "esadmin",
    srcs = [ "src/esadmin.asn", "src/z.tcl" ],
    outs = cplush(["zes-admin"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/zes-admin.c)\" -I \"2 $(location include/yaz/zes-admin.h)\" $(location src/esadmin.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "charneg",
    srcs = [ "src/charneg-3.asn", "src/z.tcl" ],
    outs = cplush(["z-charneg"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/z-charneg.c)\" -I \"2 $(location include/yaz/z-charneg.h)\" $(location src/charneg-3.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "mterm2",
    srcs = [ "src/mterm2.asn", "src/z.tcl" ],
    outs = cplush(["z-mterm2"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/z-mterm2.c)\" -I \"2 $(location include/yaz/z-mterm2.h)\" $(location src/mterm2.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "oclcui",
    srcs = [ "src/oclcui.asn", "src/z.tcl" ],
    outs = cplush(["z-oclcui"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/z-oclcui.c)\" -I \"2 $(location include/yaz/z-oclcui.h)\" $(location src/oclcui.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "facet",
    srcs = [ "src/facet.asn", "src/z.tcl" ],
    outs = cplush(["z-facet-1"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/z.tcl) -i yaz -C \"1 $(location src/z-facet-1.c)\" -I \"2 $(location include/yaz/z-facet-1.h)\" $(location src/facet.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "ill-core",
    srcs = [ "src/ill9702.asn", "src/ill.tcl" ],
    outs = cplush(["ill-core"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/ill.tcl) -i yaz -C \"1 $(location src/ill-core.c)\" -I \"2 $(location include/yaz/ill-core.h)\" $(location src/ill9702.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "oclc-ill-req-ext",
    srcs = [ "src/oclc-ill-req-ext.asn", "src/ill.tcl" ],
    outs = cplush(["oclc-ill-req-ext"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/ill.tcl) -i yaz -C \"1 $(location src/oclc-ill-req-ext.c)\" -I \"2 $(location include/yaz/oclc-ill-req-ext.h)\" $(location src/oclc-ill-req-ext.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "item-req",
    srcs = [ "src/item-req.asn", "src/ill.tcl" ],
    outs = cplush(["item-req"]),
    cmd = "$(location util/yaz-asncomp) -d $(location src/ill.tcl) -i yaz -C \"1 $(location src/item-req.c)\" -I \"2 $(location include/yaz/item-req.h)\" $(location src/item-req.asn)",
    tools = [ "util/yaz-asncomp" ],
)

genrule(
    name = "cql",
    srcs = [ "src/cql.y" ],
    outs = [ "src/cql.c" ],
    cmd = "bison -p cql_ $(location src/cql.y) -o $(location src/cql.c)",
)

cc_library(
    name = "yaz",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = plusc(Z3950_FILES) + plusc(["oid_std", "marc8",
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
	 ]),
    hdrs = plush(Z3950_FILES) + plush([
	 "oid_std", "diagbib1", "diagsrw",
	 "diagsru_update", "z-date", "z-univ", "zes-update", "zes-admin",
	 "z-charneg", "z-mterm2", "z-oclcui", "z-facet-1", "ill-core",
	 "oclc-ill-req-ext", "item-req"
	 ])
	 + glob(["src/*.h", "include/*.h", "include/yaz/*.h"]),
    visibility = ["//main:__pkg__"],
)

cc_library(
    name = "yaz_server",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = plusc(["statserv", "seshigh", "eventl", "requestq"]),
    hdrs = ["src/eventl.h", "src/session.h"],
    visibility = ["//main:__pkg__"],
    deps = [
        ":yaz",
    ],
)

cc_library(
    name = "yaz_icu",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = plusc([
	 "icu_chain", "icu_utf16", "icu_utf8", "stemmer",
	 "icu_transform", "icu_casemap", "icu_tokenizer", "icu_sortkey"
	 ]),
    hdrs = [],
    visibility = ["//main:__pkg__"],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "yaz-client",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT + [ "-lreadline" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = [
	 "client/admin.h", "client/admin.c", "client/tabcomplete.h",
	 "client/tabcomplete.c", "client/client.c", "client/fhistory.c",
	 "client/fhistory.h",
	 ],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "yaz-ztest",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = [
	 "ztest/dummy-opac.c", "ztest/read-grs.c",
	 "ztest/read-marc.c", "ztest/ztest.c", "ztest/ztest.h",
	 ],
    deps = [
        ":yaz_server",
    ],
)

cc_binary(
    name = "cclsh",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT + [ "-lreadline" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["util/cclsh.c"],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "cql2pqf",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["util/cql2pqf.c"],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "cql2xcql",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["util/cql2xcql.c"],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "yaz-marcdump",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["util/marcdump.c"],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "yaz-json-parse",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["util/json-parse.c"],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "yaz-illclient",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["util/yaz-illclient.c"],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "yaz-record-conv",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["util/yaz-record-conv.c"],
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "yaz-icu",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["util/yaz-icu.c"],
    deps = [
        ":yaz_icu",
    ],
)

