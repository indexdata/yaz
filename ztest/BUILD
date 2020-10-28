# Bazel BUILD file for YAZ
#
# bazel  build //ztest:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]
INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

cc_binary(
    name = "yaz-ztest",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT,
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = [
	 "dummy-opac.c", "read-grs.c", "read-marc.c", "ztest.c", "ztest.h",
	 ],
    deps = [
        "//:yaz_server",
    ],
)

