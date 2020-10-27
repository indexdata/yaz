# Bazel BUILD file for YAZ
#
# bazel  build //:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]

INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

genrule(
    name = "oidtoc",
    srcs = [ "src/oid.csv" ],
    outs = [ "src/oid_std.c", "include/yaz/oid_std.h" ],
    cmd = "tclsh $(location src/oidtoc.tcl) $(location src/oid.csv) $(location src/oid_std.c) $(location include/yaz/oid_std.h)",
    tools = [ "src/oidtoc.tcl" ],
)

genrule(
    name = "marc8",
    srcs = [ "src/codetables.xml" ],
    outs = [ "src/marc8.c" ],
    cmd = "tclsh $(location src/charconv.tcl)  -p marc8 $(location src/codetables.xml) -o $(location src/marc8.c)",
    tools = [ "src/charconv.tcl" ],
)

genrule(
    name = "marc8r",
    srcs = [ "src/codetables.xml" ],
    outs = [ "src/marc8r.c" ],
    cmd = "tclsh $(location src/charconv.tcl) -r -p marc8r $(location src/codetables.xml) -o $(location src/marc8r.c)",
    tools = [ "src/charconv.tcl" ],
)

genrule(
    name = "iso5426",
    srcs = [ "src/codetables-iso5426.xml" ],
    outs = [ "src/iso5426.c" ],
    cmd = "tclsh $(location src/charconv.tcl) -p iso5426 $(location src/codetables-iso5426.xml) -o $(location src/iso5426.c)",
    tools = [ "src/charconv.tcl" ],
)

genrule(
    name = "diagbib1",
    srcs = [ "src/bib1.csv" ],
    outs = [ "src/diagbib1.c", "include/yaz/diagbib1.h" ],
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/bib1.csv) $(location src/diagbib1.c) $(location include/yaz/diagbib1.h) bib1 diagbib1_str",
    tools = [ "src/csvtodiag.tcl" ],
)

genrule(
    name = "diagsrw",
    srcs = [ "src/srw.csv" ],
    outs = [ "src/diagsrw.c", "include/yaz/diagsrw.h" ],
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/srw.csv) $(location src/diagsrw.c) $(location include/yaz/diagsrw.h) srw",
    tools = [ "src/csvtodiag.tcl" ],
)

genrule(
    name = "diagsru_update",
    srcs = [ "src/sru_update.csv" ],
    outs = [ "src/diagsru_update.c", "include/yaz/diagsru_update.h" ],
    cmd = "tclsh $(location src/csvtodiag.tcl) $(location src/sru_update.csv) $(location src/diagsru_update.c) $(location include/yaz/diagsru_update.h) sru_update",
    tools = [ "src/csvtodiag.tcl" ],
)

cc_library(
    name = "yaz",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = glob(["src/*.c"]),
    hdrs = glob(["src/*.h", "include/*.h", "include/yaz/*.h"]),
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

