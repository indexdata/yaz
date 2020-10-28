# Bazel BUILD file for YAZ
#
# bazel  build //zoom:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//:util.bzl", "cplush", "c_dir", "h_dir")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]
INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

cc_binary(
    name = "zoomtst1",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst1.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst2",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst2.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst3",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst3.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst4",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst4.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst5",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst5.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst6",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst6.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst7",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst7.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst8",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst8.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst9",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst9.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst10",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst10.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomtst11",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomtst11.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoomsh1",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT + [ "-lreadline" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoomsh.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoom-benchmark",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoom-benchmark.c"],
    deps = [
        "//:yaz",
    ],
)

cc_binary(
    name = "zoom-ka",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = ["zoom-ka.c"],
    deps = [
        "//:yaz",
    ],
)

