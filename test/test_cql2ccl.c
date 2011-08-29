/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
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


static int tst_query(const char *cql, const char *expected_ccl)
{
    int ret = 0;
    CQL_parser cp = cql_parser_create();
    int r = cql_parser_string(cp, cql);
    
    if (r)
    {
        yaz_log(YLOG_WARN, "cql: parse error: %s", cql);
    }
    else
    { /* cql parse OK */
        WRBUF w = wrbuf_alloc();
        r = cql_to_ccl(cql_parser_result(cp), wrbuf_vp_puts, w);

        if (expected_ccl && r == 0 && strcmp(wrbuf_cstr(w), expected_ccl) == 0)
        {
            ret = 1;
        }
        else if (!expected_ccl)
        {
            if (r)
                ret = 1; /* expected conversion error, OK */
            else
                yaz_log(YLOG_WARN, "cql: expected conversion error: %s", cql);
        }
        else
        {
            yaz_log(YLOG_WARN, "cql: diff: %s", cql);
            yaz_log(YLOG_WARN, " expected %s", expected_ccl);
            yaz_log(YLOG_WARN, " got      %s", wrbuf_cstr(w));
        }
        wrbuf_destroy(w);
    }
    cql_parser_destroy(cp);
    return ret;
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

    YAZ_CHECK(tst_query("\\\\", "\"\\\"\""));
    YAZ_CHECK(tst_query("\\\"",   "\"\\\"\""));

    YAZ_CHECK(tst_query("\\*", "\"*\""));
    YAZ_CHECK(tst_query("\"\\*\"", "\"*\""));
    YAZ_CHECK(tst_query("\\#", "\"#\""));
    YAZ_CHECK(tst_query("\"\\#\"", "\"#\""));

    YAZ_CHECK(tst_query("title=x", "title=\"x\""));
    YAZ_CHECK(tst_query("title=x or author=y",
                        "(title=\"x\" or author=\"y\")"));


    YAZ_CHECK(tst_query("title all x", "title=x"));
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

