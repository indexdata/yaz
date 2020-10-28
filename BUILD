# Bazel BUILD file for YAZ
#
# bazel  build //:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("util.bzl", "cplush")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]

INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

genrule(
    name = "oidtoc",
    srcs = [ "src/oid.csv" ],
    outs = [ "src/oid_std.c", "include/yaz/oid_std.h" ],
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
    outs = [ "src/diagbib1.c", "include/yaz/diagbib1.h" ],
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/bib1.csv) $(location src/diagbib1.c) $(location include/yaz/diagbib1.h) bib1 diagbib1_str",
    tools = [ "src/csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "diagsrw",
    srcs = [ "src/srw.csv" ],
    outs = [ "src/diagsrw.c", "include/yaz/diagsrw.h" ],
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/srw.csv) $(location src/diagsrw.c) $(location include/yaz/diagsrw.h) srw",
    tools = [ "src/csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "diagsru_update",
    srcs = [ "src/sru_update.csv" ],
    outs = [ "src/diagsru_update.c", "include/yaz/diagsru_update.h" ],
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/sru_update.csv) $(location src/diagsru_update.c) $(location include/yaz/diagsru_update.h) sru_update",
    tools = [ "src/csvtodiag.tcl" ],
    visibility = [ "//visibility:public" ],
)

genrule(
    name = "z3950",
    srcs = [ "src/z3950v3.asn", "src/z.tcl" ],
    outs = cplush(["z-core", "z-diag1", "z-exp", "z-sutrs", "z-opac", "z-sum", "z-grs", "z-estask", "z-rrf1", "z-rrf2", "z-accform1", "z-accdes1", "z-acckrb1", "zes-pset", "zes-pquery", "zes-psched", "zes-order", "zes-update0", "zes-exps", "zes-expi", "z-uifr1", "z-espec1"]),
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

cc_library(
    name = "yaz",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["src/oid_std.c", "src/marc8.c", "src/marc8r.c", "src/iso5426.c", "src/diagbib1.c", "src/diagsrw.c", "src/diagsru_update.c" ] + glob(["src/*.c"]),
    hdrs = ["include/yaz/diagbib1.h", "include/yaz/diagsrw.h", "include/yaz/diagsru_update.h", "include/yaz/z-core.h", "include/yaz/z-date.h", "include/yaz/z-univ.h", "include/yaz/zes-update.h", "include/yaz/zes-admin.h", "include/yaz/z-charneg.h", "include/yaz/z-mterm2.h", "include/yaz/z-oclcui.h", "include/yaz/z-facet-1.h", "include/yaz/ill-core.h", "include/yaz/oclc-ill-req-ext.h", "include/yaz/item-req.h" ] + glob(["src/*.h", "include/*.h", "include/yaz/*.h"]),
    visibility = ["//main:__pkg__"],
)

cc_binary(
    name = "yaz-client",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT + [ "-lreadline" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = glob(["client/admin.h", "client/admin.c", "client/tabcomplete.h", "client/tabcomplete.c", "client/client.c", "client/fhistory.c", "client/fhistory.h"]),
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
    srcs = glob(["ztest/dummy-opac.c", "ztest/read-grs.c", "ztest/read-marc.c", "ztest/ztest.c", "ztest/ztest.h"]),
    deps = [
        ":yaz",
    ],
)

cc_binary(
    name = "cclsh",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT + [ "-lreadline" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = glob(["util/cclsh.c"]),
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
    srcs = glob(["util/cql2pqf.c"]),
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
    srcs = glob(["util/cql2xcql.c"]),
    deps = [
        ":yaz",
    ],
)

