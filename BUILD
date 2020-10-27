load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "yaz",
    includes = [ "include" ],
    copts = [ "-pthread", "-I/usr/include/libxml2" ],
    linkopts = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = glob(["src/*.c"]),
    hdrs = glob(["src/*.h", "include/*.h", "include/yaz/*.h"]),
    visibility = ["//main:__pkg__"],
)

cc_binary(
    name = "yaz-client",
    includes = [ "include" ],
    copts = [ "-pthread", "-I/usr/include/libxml2" ],
    linkopts = [ "-pthread", "-lgnutls", "-lexslt", "-lxslt", "-lxml2", "-lreadline" ],
    local_defines = [ "HAVE_CONFIG_H" ],
    srcs = glob(["client/admin.h", "client/admin.c", "client/tabcomplete.h", "client/tabcomplete.c", "client/client.c", "client/fhistory.c", "client/fhistory.h"]),
    deps = [
        ":yaz",
    ],
)

