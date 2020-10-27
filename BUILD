# Bazel BUILD file for YAZ
#
# bazel  build //:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]

INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

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

