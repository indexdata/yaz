/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include <yaz/log.h>
#include <yaz/wrbuf.h>
#include <yaz/querytowrbuf.h>
#include <yaz/pquery.h>
#include <yaz/test.h>

int expect_pqf(const char *pqf, const char *expect_pqf, int expect_error)
{
    YAZ_PQF_Parser parser = yaz_pqf_create();
    int res = 0;
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_RPNQuery *rpn;

    if (!parser)
        return 0;

    if (!odr)
        return 0;

    rpn = yaz_pqf_parse(parser, odr, pqf);

    if (!rpn)
    {
        const char *msg;
        size_t offset;
        int got_error = yaz_pqf_error (parser, &msg, &offset);

        if (expect_error == got_error)
            res = 1;
    }
    else if (expect_error == YAZ_PQF_ERROR_NONE)
    {
        WRBUF wrbuf = wrbuf_alloc();

        if (wrbuf)
        {
            yaz_rpnquery_to_wrbuf(wrbuf, rpn);

            if (!strcmp(wrbuf_cstr(wrbuf), expect_pqf))
            {
                res = 1;
            }
            wrbuf_destroy(wrbuf);
        }
    }
    yaz_pqf_destroy(parser);
    odr_destroy(odr);
    return res;
}

static void tst(void)
{
    YAZ_CHECK(expect_pqf("a", "@attrset Bib-1 a", YAZ_PQF_ERROR_NONE));
    YAZ_CHECK(expect_pqf("@attr 1=4 a", "@attrset Bib-1 @attr 1=4 a", YAZ_PQF_ERROR_NONE));
    YAZ_CHECK(expect_pqf("@attr 1=title a", "@attrset Bib-1 @attr 1=title a", YAZ_PQF_ERROR_NONE));
    YAZ_CHECK(expect_pqf("a b", "", YAZ_PQF_ERROR_EXTRA));
    YAZ_CHECK(expect_pqf("@and a", "", YAZ_PQF_ERROR_MISSING));
    YAZ_CHECK(expect_pqf("@attr p=q a", "", YAZ_PQF_ERROR_BAD_INTEGER));
    YAZ_CHECK(expect_pqf("@prox 0 0 0 0 k 0 a b",
                         "@attrset Bib-1 @prox 0 0 0 0 k 0 a b",
                         YAZ_PQF_ERROR_NONE));
    YAZ_CHECK(expect_pqf("@prox 0 0 0 0 3 0 a b", "",
                         YAZ_PQF_ERROR_PROXIMITY));
    YAZ_CHECK(expect_pqf("@attr 1=12345678901 x", "@attrset Bib-1 @attr 1=12345678901 x", YAZ_PQF_ERROR_NONE));
    YAZ_CHECK(expect_pqf("@attr 1=1234567890.1 x", "@attrset Bib-1 @attr 1=1234567890.1 x", YAZ_PQF_ERROR_NONE));
}

static void tst_term_create(void)
{
    ODR o = odr_createmem(ODR_ENCODE);
    const char data[] = { 'a', '\0', 'b' };
    Z_Term *term = z_Term_create(o, Z_Term_general, data, sizeof(data));

    YAZ_CHECK(term);
    YAZ_CHECK_EQ(term->which, Z_Term_general);
    YAZ_CHECK_EQ(term->u.general->len, sizeof(data));
    YAZ_CHECK(memcmp(term->u.general->buf, data, sizeof(data)) == 0);
    YAZ_CHECK(!z_Term_create(o, Z_Term_general, "", (size_t) INT_MAX + 1));
    YAZ_CHECK_EQ(odr_geterror(o), OLENOV);
    odr_reset(o);
    YAZ_CHECK(!z_Term_create(o, Z_Term_general, "", (size_t) -1));
    YAZ_CHECK_EQ(odr_geterror(o), OLENOV);
    odr_destroy(o);
}

static void check_complexity(YAZ_PQF_Parser parser, ODR odr,
                             WRBUF query, int allowed)
{
    Z_RPNQuery *rpn = yaz_pqf_parse(parser, odr, wrbuf_cstr(query));
    const char *msg;
    size_t offset;
    int error = yaz_pqf_error(parser, &msg, &offset);
    YAZ_CHECK_EQ(rpn != 0, allowed);
    YAZ_CHECK_EQ(error, allowed ? YAZ_PQF_ERROR_NONE : YAZ_PQF_ERROR_COMPLEXITY);
    YAZ_CHECK(offset <= wrbuf_len(query));
    if (!allowed)
    {
        YAZ_CHECK(!strcmp(msg, "query complexity limit exceeded"));
        YAZ_CHECK(offset > 0);
    }
    odr_reset(odr);
    /* Both parser entry points must apply the same bounds. */
    YAZ_CHECK_EQ(p_query_rpn(odr, wrbuf_cstr(query)) != 0, allowed);
    odr_reset(odr);
    /* A rejected query must not poison later uses of the same parser. */
    YAZ_CHECK(yaz_pqf_parse(parser, odr, "@and a b") != 0);
    YAZ_CHECK_EQ(yaz_pqf_error(parser, &msg, &offset), YAZ_PQF_ERROR_NONE);
    odr_reset(odr);
}

static void balanced_query(WRBUF query, unsigned int nodes)
{
    if (nodes == 1)
        wrbuf_puts(query, "x ");
    else
    {
        unsigned int left = (nodes - 1) / 2;
        if (!(left & 1))
            left--;
        wrbuf_puts(query, "@and ");
        balanced_query(query, left);
        balanced_query(query, nodes - 1 - left);
    }
}

static void tst_complexity(void)
{
    static const char *prefix[] = {
        "@and ", "@or ", "@not ", "@prox 0 0 0 0 k 0 ",
        "@term general ", "@attr 1=4 "
    };
    YAZ_PQF_Parser parser = yaz_pqf_create();
    ODR odr = odr_createmem(ODR_ENCODE);
    WRBUF query = wrbuf_alloc();
    size_t kind;
    int extra, right;
    unsigned int i;
    for (kind = 0; kind < sizeof(prefix) / sizeof(*prefix); kind++)
        for (right = 0; right <= (kind < 4); right++)
            for (extra = 0; extra <= 1; extra++)
            {
                unsigned int nesting = YAZ_PQF_MAX_DEPTH - 1 + extra;
                wrbuf_rewind(query);
                for (i = 0; i < nesting; i++)
                {
                    wrbuf_puts(query, prefix[kind]);
                    if (right)
                        wrbuf_puts(query, "x ");
                }
                wrbuf_puts(query, "x ");
                if (kind < 4 && !right)
                    for (i = 0; i < nesting; i++)
                        wrbuf_puts(query, "x ");
                check_complexity(parser, odr, query, !extra);
            }
    /* A very deep, incomplete remote query must fail at the bound. */
    wrbuf_rewind(query);
    for (i = 0; i < 20000; i++)
        wrbuf_puts(query, "@and ");
    check_complexity(parser, odr, query, 0);

    /* Shallow balanced trees exercise the independent total-work bound. */
    wrbuf_rewind(query);
    balanced_query(query, YAZ_PQF_MAX_NODES - 1);
    check_complexity(parser, odr, query, 1);
    for (extra = 0; extra <= 1; extra++)
    {
        wrbuf_rewind(query);
        for (i = 0; i <= (unsigned int) extra; i++)
            wrbuf_puts(query, "@term general ");
        balanced_query(query, YAZ_PQF_MAX_NODES - 1);
        check_complexity(parser, odr, query, !extra);
    }
    wrbuf_destroy(query);
    odr_destroy(odr);
    yaz_pqf_destroy(parser);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst();
    tst_term_create();
    tst_complexity();
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

