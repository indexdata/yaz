# Bazel BUILD file for YAZ utilities
#
# bazel  build //util:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]
INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

LIBS_ICU = [ "-licui18n", "-licuuc", "-licudata" ]
INCLUDES_ICU = []


cc_binary(
    name = "cclsh",
    includes = [ "src" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT + [ "-lreadline" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["cclsh.c"],
    deps = [
        "//src:yaz",
    ],
)

cc_binary(
    name = "cql2pqf",
    includes = [ "src" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["cql2pqf.c"],
    deps = [
        "//src:yaz",
    ],
)

cc_binary(
    name = "cql2xcql",
    includes = [ "src" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["cql2xcql.c"],
    deps = [
        "//src:yaz",
    ],
)

cc_binary(
    name = "yaz-marcdump",
    includes = [ "src" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["marcdump.c"],
    deps = [
        "//src:yaz",
    ],
)

cc_binary(
    name = "yaz-json-parse",
    includes = [ "src" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["json-parse.c"],
    deps = [
        "//src:yaz",
    ],
)

cc_binary(
    name = "yaz-illclient",
    includes = [ "src" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["yaz-illclient.c"],
    deps = [
        "//src:yaz",
    ],
)

cc_binary(
    name = "yaz-record-conv",
    includes = [ "src" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["yaz-record-conv.c"],
    deps = [
        "//src:yaz",
    ],
)

cc_binary(
    name = "yaz-icu",
    includes = [ "src" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H", "YAZ_HAVE_ICU=1" ],
    srcs = ["yaz-icu.c"],
    deps = [
        "//src:yaz_icu",
    ],
)

