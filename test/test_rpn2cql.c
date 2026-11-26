/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <yaz/test.h>
#include <yaz/log.h>
#include <yaz/rpn2cql.h>
#include <yaz/wrbuf.h>
#include <yaz/pquery.h>
#include <yaz/diagsrw.h>

static int compare2(cql_transform_t ct, const char *pqf, const char *cql,
                    int expected_error)
{
    int ret = 0;
    ODR odr = odr_createmem(ODR_ENCODE);
    WRBUF w = wrbuf_alloc();
    Z_RPNQuery *q = p_query_rpn(odr, pqf);

    if (q)
    {
        int r = cql_transform_rpn2cql_wrbuf(ct, w, q);

        if (r != 0)
        {
            const char *addinfo = 0;
            int err = cql_transform_error(ct, &addinfo);
            /* transform error */
            yaz_log(YLOG_LOG, "%s -> Error %d", pqf, r);
            if (err == 0)
                ;
            else if (err == expected_error)
            {
                if (addinfo && cql && !strcmp(addinfo, cql))
                    ret = 1;
                else if (!addinfo && !cql)
                    ret = 1;
            }
        }
        else if (r == 0)
        {
            yaz_log(YLOG_LOG, "%s -> %s", pqf, wrbuf_cstr(w));
            if (!expected_error)
                ret = 1;
            else if (cql && !strcmp(wrbuf_cstr(w), cql))
            {
                ret = 1;
            }
            else
            {
                yaz_log(YLOG_WARN, " expected: %s", cql ? cql : "null");
                yaz_log(YLOG_WARN, " got:      %s", wrbuf_cstr(w));
            }
        }
    }
    wrbuf_destroy(w);
    odr_destroy(odr);
    return ret;
}

static int compare(cql_transform_t ct, const char *pqf, const char *cql)
{
    return compare2(ct, pqf, cql, 0);
}

static void tst1(void)
{
    cql_transform_t ct = cql_transform_create();

    YAZ_CHECK(compare(ct, "abc", "abc"));
    YAZ_CHECK(compare(ct, "\"a b c\"", "\"a b c\""));
    YAZ_CHECK(compare(ct, "@and a b", "a and b"));
    YAZ_CHECK(compare(ct, "@or a @and b c", "a or (b and c)"));
    YAZ_CHECK(compare(ct, "@or @and a b @and c d", "(a and b) or (c and d)"));
    YAZ_CHECK(compare(ct, "@or @or a b @or c d", "(a or b) or (c or d)"));
    YAZ_CHECK(compare(ct, "@and @and a b @and c d", "(a and b) and (c and d)"));

    YAZ_CHECK(compare(ct, "@attr 1=field abc", "field=abc"));
    YAZ_CHECK(compare2(ct, "@attr 1=4 abc", "4", 114)); /* should fail */

    cql_transform_define_pattern(ct, "index.title", "1=4");
    YAZ_CHECK(compare(ct, "@attr 1=4 abc", "title=abc"));

    cql_transform_define_pattern(ct, "index.foo", "1=bar");
    YAZ_CHECK(compare(ct, "@attr 1=bar abc", "foo=abc"));
    YAZ_CHECK(compare(ct, "@attr bib1 1=bar abc", "foo=abc"));

    cql_transform_define_pattern(ct, "index.author", "bib1 1=1003 4=2");
    YAZ_CHECK(compare(ct, "@attr 4=2 @attr bib1 1=1003 abc", "author=abc"));

    cql_transform_close(ct);
}

static void tst2(void)
{
    WRBUF w = wrbuf_alloc();
    cql_transform_t ct = 0;
    const char *srcdir = getenv("srcdir");
    if (srcdir)
    {
        wrbuf_puts(w, srcdir);
        wrbuf_puts(w, "/");
    }
    wrbuf_puts(w, "../etc/pqf.properties");

    ct = cql_transform_open_fname(wrbuf_cstr(w));
    YAZ_CHECK(compare(ct, "@attr 1=4 abc", "dc.title=abc"));
    YAZ_CHECK(compare(ct, "@attr 1=4 @attr 4=108 abc", "dc.title=/exact abc"));
    YAZ_CHECK(compare(ct, "@attr 1=4 @attr 3=1 @attr 6=1 abc", "dc.title=abc"));
    YAZ_CHECK(compare(ct, "@attr 1=4 @attr 4=1 @attr 6=1 abc", "dc.title=abc"));
    YAZ_CHECK(compare(ct, "@attr 1=1016 abc", "abc"));
    /* Date tests */
    YAZ_CHECK(compare(ct, "@attr 1=30 @attr 2=1 1980", "dc.date<1980"));
    YAZ_CHECK(compare(ct, "@attr 1=30 @attr 2=2 1980", "dc.date<=1980"));
    YAZ_CHECK(compare(ct, "@attr 1=30 @attr 2=3 1980", "dc.date=1980"));
    YAZ_CHECK(compare(ct, "@attr 1=30 @attr 2=4 1980", "dc.date>=1980"));
    YAZ_CHECK(compare(ct, "@attr 1=30 @attr 2=5 1980", "dc.date>1980"));
    YAZ_CHECK(compare(ct, "@and @attr 1=30 @attr 2=4 234 @attr 1=30 @attr 2=2 1990", "dc.date>=234 and dc.date<=1990"));

    /* Truncation */
    YAZ_CHECK(compare(ct, "@attr 5=1 water", "water*"));
    YAZ_CHECK(compare(ct, "@attr 5=2 water", "*water"));
    YAZ_CHECK(compare(ct, "@attr 5=3 water", "*water*"));
    YAZ_CHECK(compare(ct, "@attr 5=100 water", "water"));
    YAZ_CHECK(compare(ct, "@attr 5=102 water", "water"));
    YAZ_CHECK(compare(ct, "@attr 5=104 water", "water"));

    YAZ_CHECK(compare(ct, "@attr 5=102 wat.*er", "wat*er"));
    YAZ_CHECK(compare(ct, "@attr 5=104 wat?er", "wat*er"));

    YAZ_CHECK(compare(ct, "@attr 5=102 wat.er", "wat?er"));
    YAZ_CHECK(compare(ct, "@attr 5=104 wat#er", "wat?er"));
    YAZ_CHECK(compare(ct, "@attr 5=102 wat?er", "wat\\?er"));
    YAZ_CHECK(compare(ct, "@attr 5=104 wat*er", "wat\\*er"));
    YAZ_CHECK(compare(ct, "@attr 5=102 wat#er", "wat#er"));

    /* \. is 'eaten' by PQF parser */
    YAZ_CHECK(compare(ct, "@attr 5=102 wat\\.er", "wat?er"));

    /* Escape sequences */
    /* note: escape sequences that survive after PQF parse below */
    YAZ_CHECK(compare(ct, "@attr 5=102 wat\\\\?er", "wat\\?er"));
    YAZ_CHECK(compare(ct, "@attr 5=104 wat\\\\?er", "wat\\?er"));

    YAZ_CHECK(compare(ct, "@attr 5=102 wat\\\\*er", "wat\\*er"));
    YAZ_CHECK(compare(ct, "@attr 5=104 wat\\\\*er", "wat\\*er"));

    YAZ_CHECK(compare(ct, "wat\\\\#er", "wat#er"));
    YAZ_CHECK(compare(ct, "@attr 5=100 wat\\\\#er", "wat#er"));
    YAZ_CHECK(compare(ct, "@attr 5=102 wat\\\\#er", "wat#er"));
    YAZ_CHECK(compare(ct, "@attr 5=104 wat\\\\#er", "wat#er"));
    YAZ_CHECK(compare(ct, "@attr 5=102 wat\\\\.er", "wat.er"));
    YAZ_CHECK(compare(ct, "@attr 5=104 wat\\\\.er", "wat.er"));

    /* Quoting */
    YAZ_CHECK(compare(ct, "@attr 5=100 \"\"", "\"\""));
    YAZ_CHECK(compare(ct, "@attr 5=1 \"\"", "\"*\""));
    YAZ_CHECK(compare(ct, "@attr 5=2 \"\"", "\"*\""));
    YAZ_CHECK(compare(ct, "@attr 5=3 \"\"", "\"**\""));
    YAZ_CHECK(compare(ct, "@attr 5=102 \"\"", "\"\""));
    YAZ_CHECK(compare(ct, "@attr 5=104 \"\"", "\"\""));

    YAZ_CHECK(compare(ct, "@attr 5=1 \"water basket\"", "\"water basket*\""));
    YAZ_CHECK(compare(ct, "@attr 5=2 \"water basket\"", "\"*water basket\""));
    YAZ_CHECK(compare(ct, "@attr 5=3 \"water basket\"", "\"*water basket*\""));

    /* Other */
    YAZ_CHECK(compare(ct, "@attr 2=103 @attr 1=_ALLRECORDS 1", "cql.allRecords=1"));
    YAZ_CHECK(compare2(ct, "@attr 1=500 abc", "500", 114));
    YAZ_CHECK(compare2(ct, "@attr 5=99 x", "99", 120));
    cql_transform_close(ct);
    wrbuf_destroy(w);
}

/* CQL to PQF: index of index_len characters, relation all with modifiers,
   term of so many words */
static int tst_cql2pqf(cql_transform_t ct, int index_len,
                       const char *modifier, int modifiers,
                       int words, WRBUF pqf)
{
    CQL_parser cp = cql_parser_create();
    WRBUF w = wrbuf_alloc();
    int i, r;

    for (i = 0; i < index_len; i++)
        wrbuf_puts(w, "x");
    wrbuf_puts(w, " all");
    for (i = 0; i < modifiers; i++)
        wrbuf_puts(w, modifier);
    wrbuf_puts(w, " \"");
    for (i = 0; i < words; i++)
        wrbuf_puts(w, i ? " a" : "a");
    wrbuf_puts(w, "\"");
    r = cql_parser_string(cp, wrbuf_cstr(w));
    YAZ_CHECK_EQ(r, 0);
    wrbuf_rewind(pqf);
    if (r == 0)
        r = cql_transform(ct, cql_parser_result(cp), wrbuf_vp_puts, pqf);
    wrbuf_destroy(w);
    cql_parser_destroy(cp);
    return r;
}

/* index and relation modifiers are written for each word of a term, so the
   PQF is limited: in the length of a name that is passed on (*) and in the
   number of terms and relation modifiers */
static void tst_cql2pqf_limits(void)
{
    cql_transform_t ct = cql_transform_create();
    WRBUF pqf = wrbuf_alloc();
    const char *addinfo;

    cql_transform_define_pattern(ct, "set.pass", "http://pass");
    cql_transform_define_pattern(ct, "set", "http://pass");
    cql_transform_define_pattern(ct, "index.pass.*", "1=*");
    cql_transform_define_pattern(ct, "relation.all", "4=6");
    cql_transform_define_pattern(ct, "relationModifier.relevant", "2=102");
    cql_transform_define_pattern(ct, "structure.*", "4=1");
    cql_transform_define_pattern(ct, "position.any", "3=3");

    YAZ_CHECK_EQ(tst_cql2pqf(ct, 1, "/relevant", 1, 2, pqf), 0);
    YAZ_CHECK(!strcmp(wrbuf_cstr(pqf), "@attr 4=6 @attr 4=1 "
                      "@and @attr 3=3 @attr 1=x @attr 2=102 \"a\" "
                      "@attr 3=3 @attr 1=x @attr 2=102 \"a\" "));

    YAZ_CHECK_EQ(tst_cql2pqf(ct, 255, "", 0, 2, pqf), 0);
    YAZ_CHECK(wrbuf_len(pqf) > 2 * 255);
    YAZ_CHECK_EQ(tst_cql2pqf(ct, 256, "", 0, 2, pqf), YAZ_SRW_UNSUPP_INDEX);
    YAZ_CHECK_EQ(cql_transform_error(ct, &addinfo), YAZ_SRW_UNSUPP_INDEX);
    YAZ_CHECK(addinfo && strlen(addinfo) == 256);
    YAZ_CHECK_EQ(tst_cql2pqf(ct, 48000, "", 0, 2, pqf), YAZ_SRW_UNSUPP_INDEX);

    YAZ_CHECK_EQ(tst_cql2pqf(ct, 1, "", 0, YAZ_PQF_MAX_NODES, pqf), 0);
    YAZ_CHECK_EQ(tst_cql2pqf(ct, 1, "", 0, YAZ_PQF_MAX_NODES + 1, pqf),
                 YAZ_SRW_QUERY_SYNTAX_ERROR);
    YAZ_CHECK_EQ(cql_transform_error(ct, &addinfo),
                 YAZ_SRW_QUERY_SYNTAX_ERROR);
    YAZ_CHECK(addinfo && !strcmp(addinfo, "query complexity limit exceeded"));

    YAZ_CHECK_EQ(tst_cql2pqf(ct, 1, "/relevant", 63,
                             YAZ_PQF_MAX_NODES / 64, pqf), 0);
    YAZ_CHECK_EQ(tst_cql2pqf(ct, 1, "/relevant", 63,
                             YAZ_PQF_MAX_NODES / 64 + 1, pqf),
                 YAZ_SRW_QUERY_SYNTAX_ERROR);
    /* Stay within the parser's modifier-list limit so conversion rejects
       the expansion, rather than parsing rejecting the query first. */
    YAZ_CHECK_EQ(tst_cql2pqf(ct, 255, "/relevant", CQL_MAX_DEPTH, 2000, pqf),
                 YAZ_SRW_QUERY_SYNTAX_ERROR);
    YAZ_CHECK(wrbuf_len(pqf) < 100000);

    /* not lost when the clause has a modifier with a value */
    YAZ_CHECK_EQ(tst_cql2pqf(ct, 1, "/pass.y=1", 1,
                             YAZ_PQF_MAX_NODES / 2, pqf), 0);
    YAZ_CHECK_EQ(tst_cql2pqf(ct, 1, "/pass.y=1", 1,
                             YAZ_PQF_MAX_NODES / 2 + 1, pqf),
                 YAZ_SRW_QUERY_SYNTAX_ERROR);

    wrbuf_destroy(pqf);
    cql_transform_close(ct);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst1();
    tst2();
    tst_cql2pqf_limits();
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

