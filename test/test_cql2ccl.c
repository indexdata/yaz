/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/wrbuf.h>
#include <yaz/cql.h>
#include <yaz/log.h>
#include <yaz/test.h>

static int tst_query_s(const char *cql, const char *expected_ccl,
                       const char *expected_keys)
{
    int ret = 1;
    CQL_parser cp = cql_parser_create();
    int r = cql_parser_string(cp, cql);

    if (r)
    {
        yaz_log(YLOG_WARN, "cql: parse error: %s", cql);
        ret = 0;
    }
    else
    { /* cql parse OK */
        WRBUF w = wrbuf_alloc();
        r = cql_to_ccl(cql_parser_result(cp), wrbuf_vp_puts, w);

        if (expected_ccl && r == 0 && strcmp(wrbuf_cstr(w), expected_ccl) == 0)
            ;
        else if (!expected_ccl)
        {
            if (!r)
            {
                ret = 0;
                yaz_log(YLOG_WARN, "cql: diff: %s", cql);
                yaz_log(YLOG_WARN, " exp error");
                yaz_log(YLOG_WARN, " got ccl %s", wrbuf_cstr(w));
            }
        }
        else
        {
            ret = 0;
            yaz_log(YLOG_WARN, "cql: diff: %s", cql);
            yaz_log(YLOG_WARN, " exp ccl %s", expected_ccl);
            if (!r)
                yaz_log(YLOG_WARN, " got ccl %s", wrbuf_cstr(w));
            else
                yaz_log(YLOG_WARN, " got error");
        }
        wrbuf_rewind(w);
        r = cql_sortby_to_sortkeys(cql_parser_result(cp),
                                   wrbuf_vp_puts, w);
        if (expected_keys && !r && !strcmp(wrbuf_cstr(w), expected_keys))
            ;
        else if (!expected_keys)
        {
            if (!r)
            {
                ret = 0;
                yaz_log(YLOG_WARN, "cql: diff: %s", cql);
                yaz_log(YLOG_WARN, " exp error");
                yaz_log(YLOG_WARN, " got sortkeys %s", wrbuf_cstr(w));
            }
        }
        else
        {
            ret = 0;
            yaz_log(YLOG_WARN, "cql: diff: %s", cql);
            yaz_log(YLOG_WARN, " exp sortkeys %s", expected_keys);
            if (!r)
                yaz_log(YLOG_WARN, " got sortkeys %s", wrbuf_cstr(w));
            else
                yaz_log(YLOG_WARN, " got error");
        }
        wrbuf_destroy(w);
    }
    cql_parser_destroy(cp);
    return ret;
}

static int tst_query(const char *cql, const char *expected_ccl)
{
    return tst_query_s(cql, expected_ccl, "");
}

static void tst(void)
{
    YAZ_CHECK(tst_query("\"\"", "\"\""));
    YAZ_CHECK(tst_query("x", "\"x\""));
    YAZ_CHECK(tst_query("\"x\"", "\"x\""));
    YAZ_CHECK(tst_query("\"xy\"", "\"xy\""));

    YAZ_CHECK(tst_query("?", "#"));
    YAZ_CHECK(tst_query("?a?", "#\"a\"#"));
    YAZ_CHECK(tst_query("?a", "#\"a\""));
    YAZ_CHECK(tst_query("a?", "\"a\"#"));
    YAZ_CHECK(tst_query("\"?\"", "#"));
    YAZ_CHECK(tst_query("\\?", "\"?\""));
    YAZ_CHECK(tst_query("\"\\?\"", "\"?\""));

    YAZ_CHECK(tst_query("*", "?"));
    YAZ_CHECK(tst_query("*a*", "?\"a\"?"));
    YAZ_CHECK(tst_query("*a", "?\"a\""));
    YAZ_CHECK(tst_query("a*", "\"a\"?"));
    YAZ_CHECK(tst_query("\"*\"", "?"));
    YAZ_CHECK(tst_query("\\*", "\"*\""));
    YAZ_CHECK(tst_query("\"\\*\"", "\"*\""));

    YAZ_CHECK(tst_query("a b", "\"a\" \"b\""));
    YAZ_CHECK(tst_query("ab bc", "\"ab\" \"bc\""));

    YAZ_CHECK(tst_query("\\\\", "\"\\\\\""));
    YAZ_CHECK(tst_query("\\\"", "\"\\\"\""));
    YAZ_CHECK(tst_query("\\x" , "\"x\""));

    YAZ_CHECK(tst_query("\\*", "\"*\""));
    YAZ_CHECK(tst_query("\"\\*\"", "\"*\""));
    YAZ_CHECK(tst_query("\\#", "\"#\""));
    YAZ_CHECK(tst_query("\"\\#\"", "\"#\""));

    YAZ_CHECK(tst_query("title=x", "title=\"x\""));
    YAZ_CHECK(tst_query("title=x or author=y",
                        "(title=\"x\") or (author=\"y\")"));
    YAZ_CHECK(tst_query("title=x or author=y and date=z",
                        "((title=\"x\") or (author=\"y\")) and (date=\"z\")"));

    YAZ_CHECK(tst_query("title all \"\"", "title=\"\""));

    YAZ_CHECK(tst_query("title all x", "title=\"x\""));
    YAZ_CHECK(tst_query("title all x y", "title=\"x\" and title=\"y\""));
    YAZ_CHECK(tst_query("title all \"x y\"", "title=\"x\" and title=\"y\""));

    YAZ_CHECK(tst_query("title any x", "title=\"x\""));
    YAZ_CHECK(tst_query("title any x y", "title=\"x\" or title=\"y\""));
    YAZ_CHECK(tst_query("title any \"x y\"", "title=\"x\" or title=\"y\""));

    YAZ_CHECK(tst_query("title = \"x y\"", "title=\"x y\""));
    YAZ_CHECK(tst_query("title = x y", "title=\"x\" \"y\""));

    YAZ_CHECK(tst_query("title = x y z",  "title=\"x\" \"y\" \"z\""));

    YAZ_CHECK(tst_query("dc.title=encyclopedia prox dinosaurs",
                        "(dc.title=\"encyclopedia\") % (\"dinosaurs\")"));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance<=3 dinosaurs",
                        "(dc.title=\"encyclopedia\") %3 (\"dinosaurs\")"));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance<=3/unit=word "
                        "dinosaurs",
                        "(dc.title=\"encyclopedia\") %3 (\"dinosaurs\")"));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance<=3/unit=phrase "
                        "dinosaurs", 0));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance<=3/a=b "
                        "dinosaurs", 0));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/a=b dinosaurs", 0));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance<3 dinosaurs",
                        "(dc.title=\"encyclopedia\") %2 (\"dinosaurs\")"));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance=3 dinosaurs", 0));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance>3 dinosaurs", 0));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance>=3 dinosaurs", 0));
    YAZ_CHECK(tst_query_s("a sortby title", "\"a\"",
                          "title,,1,0,highValue"));
    YAZ_CHECK(tst_query_s("a sortby dc.title", "\"a\"",
                          "title,dc,1,0,highValue"));
    YAZ_CHECK(tst_query_s("a sortby title/ascending", "\"a\"",
                          "title,,1,0,highValue"));
    YAZ_CHECK(tst_query_s("a sortby title/descending", "\"a\"",
                          "title,,0,0,highValue"));
    YAZ_CHECK(tst_query_s("a sortby title/ignoreCase", "\"a\"",
                          "title,,1,0,highValue"));
    YAZ_CHECK(tst_query_s("a sortby title/respectCase", "\"a\"",
                          "title,,1,1,highValue"));
    YAZ_CHECK(tst_query_s("a sortby title/missingOmit", "\"a\"",
                          "title,,1,0,omit"));
    YAZ_CHECK(tst_query_s("a sortby title/missingFail", "\"a\"",
                          "title,,1,0,abort"));
    YAZ_CHECK(tst_query_s("a sortby title/missingLow", "\"a\"",
                          "title,,1,0,lowValue"));
    YAZ_CHECK(tst_query_s("a sortby title/missingHigh", "\"a\"",
                          "title,,1,0,highValue"));
    YAZ_CHECK(tst_query_s("a sortby title/sort.missingHigh", "\"a\"",
                          "title,,1,0,highValue"));
    YAZ_CHECK(tst_query_s("a sortby title/bogus", "\"a\"", 0));

    YAZ_CHECK(tst_query_s("a sortby dc.year dc.author", "\"a\"",
                          "year,dc,1,0,highValue author,dc,1,0,highValue"));

}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst();
    YAZ_CHECK_TERM;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

