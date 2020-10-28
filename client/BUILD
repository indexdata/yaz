# Bazel BUILD file for yaz-client
#
# bazel  build //client:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

LIBS_EXT = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ]
INCLUDES_EXT = [ "-I/usr/include/libxml2" ]

cc_binary(
    name = "yaz-client",
    includes = [ "include" ],
    copts = [ "-pthread" ] + INCLUDES_EXT,
    linkopts = LIBS_EXT + [ "-lreadline" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = [
	 "admin.h", "admin.c", "tabcomplete.h", "tabcomplete.c",
	 "client.c", "fhistory.c", "fhistory.h",
	 ],
    deps = [
        "//:yaz",
    ],
)

