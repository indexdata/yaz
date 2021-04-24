# Bazel BUILD file for YAZ documentation
#
# Build library:
#  bazel  build //doc:all

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//src:util.bzl", "listToString")

REFFILES = ["yaz-client-man.xml", "yaz-ztest-man.xml", "yaz-config-man.xml",
        "yaz-man.xml", "zoomsh-man.xml", "yaz-asncomp-man.xml", 
        "yaz-marcdump-man.xml", "yaz-iconv-man.xml", "yaz-log-man.xml",
        "yaz-illclient-man.xml", "yaz-icu-man.xml", "yaz-url-man.xml",
        "bib1-attr-man.xml", "yaz-json-parse-man.xml",
	"yaz-record-conv-man.xml"]

genrule(
    name = "std-oid-table",
    srcs = [ "//src:oid.csv" ],
    outs = [ "std-oid-table.xml" ],
    cmd = "tclsh $(location //src:oidtoc.tcl) $(location //src:oid.csv) $(location std-oid-table.xml)",
    tools = [ "//src:oidtoc.tcl" ],
)

genrule(
    name = "bib1-diag-table",
    srcs = [ "//src:bib1.csv" ],
    outs = [ "bib1-diag-table.xml" ],
    cmd = "tclsh $(location //src:csvtodiag.tcl) $(location //src:bib1.csv) $(location bib1-diag-table.xml) bib1-diag-table",
    tools = [ "//src:csvtodiag.tcl" ],
)

genrule(
    name = "srw-diag-table",
    srcs = [ "//src:srw.csv" ],
    outs = [ "srw-diag-table.xml" ],
    cmd = "tclsh $(location //src:csvtodiag.tcl) $(location //src:srw.csv) $(location srw-diag-table.xml) srw-diag-table",
    tools = [ "//src:csvtodiag.tcl" ],
)

genrule(
    name = "yaz-client-1",
    srcs = [ "yaz-client-man.xml" ],
    outs = [ "yaz-client.1" ],
    cmd = "xsltproc -o $(location yaz-client.1) $(location common/id.man.xsl) $(location yaz-client-man.xml) 2>/dev/null",
    tools = [ "common/id.man.xsl"],
)

genrule(
    name = "manref",
    srcs = REFFILES + [
       "gfs-options.xml", "gfs-synopsis.xml", "gfs-virtual.xml",
       "local.ent", "entities.ent"
    ],
    outs = [ "manref.xml" ],
    cmd = "rm -f $(location manref.xml);"
    + "for i in " + listToString(REFFILES, " ") + "; do"
    + " xsltproc --path .:doc $(location common/stripref.xsl) $$i |"
    +"    sed 1d >>$@;"
    + "done",
    tools = [ "common/stripref.xsl", "common/common.ent"],
)

genrule(
    name = "html",
    srcs = REFFILES + [
       "gfs-options.xml", "gfs-synopsis.xml", "gfs-virtual.xml",
       "manref.xml", "book.xml", "local.ent", "entities.ent",
       "std-oid-table.xml", "bib1-diag-table.xml", "srw-diag-table.xml",
       "common/id.png", "common/style1.css"
    ],
    outs = [ "html.tar.gz" ],
    cmd = "xsltproc --path $(RULEDIR) $(location common/id.htmlhelp.xsl) $(location book.xml);"
    + "mkdir common;"
    + "cp $(location common/id.png) common;"
    + "cp $(location common/style1.css) common;"
    + "tar czf $(location html.tar.gz) *.html common",
    tools = [ "common/id.htmlhelp.xsl", "common/common.ent"],
)
