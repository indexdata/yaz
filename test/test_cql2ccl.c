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
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/unit dinosaurs", 0));
    YAZ_CHECK(tst_query("dc.title=encyclopedia prox/distance dinosaurs", 0));
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

/* Keep large term counts below the boolean depth limit. */
static void tst_balanced_terms(WRBUF w, int terms)
{
    if (terms == 1)
        wrbuf_puts(w, "a");
    else
    {
        wrbuf_puts(w, "(");
        tst_balanced_terms(w, terms / 2);
        wrbuf_puts(w, " or ");
        tst_balanced_terms(w, terms - terms / 2);
        wrbuf_puts(w, ")");
    }
}

/* relation modifiers count once for each term that they apply to */
static int tst_modifiers(CQL_parser cp, int modifiers, int terms)
{
    WRBUF w = wrbuf_alloc();
    int i, r;

    wrbuf_puts(w, "dc.title =");
    for (i = 0; i < modifiers; i++)
        wrbuf_puts(w, "/m");
    wrbuf_puts(w, " (");
    tst_balanced_terms(w, terms);
    wrbuf_puts(w, ")");
    r = cql_parser_string(cp, wrbuf_cstr(w));
    wrbuf_destroy(w);
    return r;
}

static void tst_limits(void)
{
    CQL_parser cp = cql_parser_create();

    YAZ_CHECK(tst_modifiers(cp, 64, CQL_MAX_MODIFIERS / 64 + 1) != 0);
    YAZ_CHECK(tst_modifiers(cp, 1, CQL_MAX_MODIFIERS + 1) != 0);
    /* the count starts over for each query */
    YAZ_CHECK(tst_modifiers(cp, 64, CQL_MAX_MODIFIERS / 64) == 0);
    cql_parser_destroy(cp);

    /* terms without modifiers do not count towards the modifier budget */
    cp = cql_parser_create();
    YAZ_CHECK(tst_modifiers(cp, 0, 2 * CQL_MAX_MODIFIERS) == 0);
    cql_parser_destroy(cp);
}

/* parses head, num times repeat, tail, num times close */
static int tst_parse(const char *head, const char *repeat, const char *tail,
                     const char *close, int num)
{
    CQL_parser cp = cql_parser_create();
    WRBUF w = wrbuf_alloc();
    int i, r;

    wrbuf_puts(w, head);
    for (i = 0; i < num; i++)
        wrbuf_puts(w, repeat);
    wrbuf_puts(w, tail);
    for (i = 0; i < num; i++)
        wrbuf_puts(w, close);
    r = cql_parser_string(cp, wrbuf_cstr(w));
    wrbuf_destroy(w);
    cql_parser_destroy(cp);
    return r;
}

static void tst_depth(void)
{
    /* nested boolean operators */
    YAZ_CHECK(tst_parse("a", " or a", "", "", CQL_MAX_DEPTH) == 0);
    YAZ_CHECK(tst_parse("a", " or a", "", "", CQL_MAX_DEPTH + 1) != 0);
    YAZ_CHECK(tst_parse("a", " or a", "", "", 100000) != 0);
    YAZ_CHECK(tst_parse("", "a or (", "a", ")", CQL_MAX_DEPTH) == 0);
    YAZ_CHECK(tst_parse("", "a or (", "a", ")", CQL_MAX_DEPTH + 1) != 0);
    /* the deeper operand counts */
    YAZ_CHECK(tst_parse("a", " or (a and a)", "", "", CQL_MAX_DEPTH - 1) == 0);
    YAZ_CHECK(tst_parse("a", " or (a and a)", "", "", CQL_MAX_DEPTH) != 0);

    /* modifiers for relation, boolean and sort key */
    YAZ_CHECK(tst_parse("a =", "/x", " b", "", CQL_MAX_DEPTH) == 0);
    YAZ_CHECK(tst_parse("a =", "/x", " b", "", CQL_MAX_DEPTH + 1) != 0);
    YAZ_CHECK(tst_parse("a =", "/x", " b", "", 300000) != 0);
    YAZ_CHECK(tst_parse("a and", "/x=1", " b", "", CQL_MAX_DEPTH) == 0);
    YAZ_CHECK(tst_parse("a and", "/x=1", " b", "", CQL_MAX_DEPTH + 1) != 0);
    YAZ_CHECK(tst_parse("a sortby t", "/x", "", "", CQL_MAX_DEPTH) == 0);
    YAZ_CHECK(tst_parse("a sortby t", "/x", "", "", CQL_MAX_DEPTH + 1) != 0);
}

/* parse query with a number of nested prefix assignments */
static int tst_prefixes(const char *prefix, int no, const char *query,
                        const char *suffix)
{
    CQL_parser cp = cql_parser_create();
    WRBUF w = wrbuf_alloc();
    int i, r;

    for (i = 0; i < no; i++)
        wrbuf_puts(w, prefix);
    wrbuf_puts(w, query);
    for (i = 0; i < no; i++)
        wrbuf_puts(w, suffix);
    r = cql_parser_string(cp, wrbuf_cstr(w));
    wrbuf_destroy(w);
    cql_parser_destroy(cp);
    return r;
}

static void tst_prefix_limit(void)
{
    /* at most 20 prefix assignments may be in effect for a clause */
    YAZ_CHECK(tst_prefixes(">u ", 20, "a", "") == 0);
    YAZ_CHECK(tst_prefixes(">u ", 21, "a", "") != 0);
    YAZ_CHECK(tst_prefixes(">p=u ", 20, "p.a=b", "") == 0);
    YAZ_CHECK(tst_prefixes(">p=u ", 21, "p.a=b", "") != 0);
    YAZ_CHECK(tst_prefixes(">u (", 20, "a", ")") == 0);
    YAZ_CHECK(tst_prefixes(">u (", 21, "a", ")") != 0);
    YAZ_CHECK(tst_prefixes(">u ", 3000, "a or b", "") != 0);
    /* assignments that are not nested are not limited */
    YAZ_CHECK(tst_prefixes("(>u a) or ", 200, "a", "") == 0);
    YAZ_CHECK(tst_prefixes(">u (>p=u a) or (", 19, "a", ")") == 0);
    YAZ_CHECK(tst_prefixes(">u (>p=u a) or (", 20, "a", ")") != 0);
}

/* one parser handle used for several queries */
static void tst_reuse(void)
{
    CQL_parser cp = cql_parser_create();
    WRBUF w = wrbuf_alloc();

    YAZ_CHECK(cql_parser_string(cp, "a and b") == 0);
    YAZ_CHECK(cql_parser_result(cp) != 0);
    YAZ_CHECK(cql_to_ccl(cql_parser_result(cp), wrbuf_vp_puts, w) == 0);
    YAZ_CHECK(!strcmp(wrbuf_cstr(w), "(\"a\") and (\"b\")"));

    wrbuf_rewind(w);
    YAZ_CHECK(cql_parser_string(cp, "c sortby title") == 0);
    YAZ_CHECK(cql_parser_result(cp) != 0);
    YAZ_CHECK(cql_to_ccl(cql_parser_result(cp), wrbuf_vp_puts, w) == 0);
    YAZ_CHECK(!strcmp(wrbuf_cstr(w), "\"c\""));

    YAZ_CHECK(cql_parser_string(cp, "d and") != 0);
    YAZ_CHECK(cql_parser_result(cp) == 0);

    wrbuf_rewind(w);
    YAZ_CHECK(cql_parser_string(cp, "e") == 0);
    YAZ_CHECK(cql_parser_result(cp) != 0);
    YAZ_CHECK(cql_to_ccl(cql_parser_result(cp), wrbuf_vp_puts, w) == 0);
    YAZ_CHECK(!strcmp(wrbuf_cstr(w), "\"e\""));

    wrbuf_destroy(w);
    cql_parser_destroy(cp);
}

static int tst_sortkeys_buf(const char *cql, int max, int expected_r,
                            const char *expected_keys)
{
    char out[40];
    int r = -2;
    CQL_parser cp = cql_parser_create();

    memset(out, 'X', sizeof(out));
    out[sizeof(out) - 1] = '\0';
    if (cql_parser_string(cp, cql) == 0)
        r = cql_sortby_to_sortkeys_buf(cql_parser_result(cp), out, max);
    cql_parser_destroy(cp);
    return r == expected_r && !strcmp(out, expected_keys);
}

static void tst_sortkeys(void)
{
    /* "title,,1,0,highValue" is 20 characters + terminator */
    YAZ_CHECK(tst_sortkeys_buf("a", 21, 0, ""));
    YAZ_CHECK(tst_sortkeys_buf("a sortby title", 21, 0,
                               "title,,1,0,highValue"));
    /* does not fit: error and no unterminated result */
    YAZ_CHECK(tst_sortkeys_buf("a sortby title", 20, -1, ""));
    YAZ_CHECK(tst_sortkeys_buf("a sortby title date", 30, -1, ""));
}

static void tst_xcql_buf(void)
{
    CQL_parser cp = cql_parser_create();
    WRBUF expected = wrbuf_alloc();
    char out[2048];

    YAZ_CHECK(cql_parser_string(cp, "a or b") == 0);
    struct cql_node *cn = cql_parser_result(cp);
    cql_to_xml(cn, wrbuf_vp_puts, expected);
    int len = (int) wrbuf_len(expected);
    YAZ_CHECK(len > 0 && len < (int) sizeof(out));
    if (len > 0 && len < (int) sizeof(out))
    {
        YAZ_CHECK_EQ(cql_to_xml_buf(cn, out, len + 1), len);
        YAZ_CHECK(!strcmp(out, wrbuf_cstr(expected)));

        /* Room for the content but not its terminator: clear partial output. */
        memset(out, 'X', sizeof(out));
        YAZ_CHECK_EQ(cql_to_xml_buf(cn, out, len), -1);
        YAZ_CHECK_EQ(out[0], '\0');
        YAZ_CHECK_EQ(out[len], 'X');

        out[0] = 'X';
        YAZ_CHECK_EQ(cql_to_xml_buf(cn, out, 0), -1);
        YAZ_CHECK_EQ(out[0], 'X');
    }
    wrbuf_destroy(expected);
    cql_parser_destroy(cp);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst();
    tst_limits();
    tst_depth();
    tst_prefix_limit();
    tst_reuse();
    tst_sortkeys();
    tst_xcql_buf();
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

