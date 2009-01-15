/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#include <stdlib.h>
#include <stdio.h>

#include <yaz/query-charset.h>
#include <yaz/copy_types.h>
#include <yaz/pquery.h>
#include <yaz/querytowrbuf.h>
#include <yaz/test.h>

enum query_charset_status {
    NO_ERROR,
    PQF_FAILED,
    MATCH,
    NO_MATCH,
    CONV_FAILED
};

enum query_charset_status t(yaz_iconv_t cd, 
                            const char *pqf, const char *expect_pqf)
{
    YAZ_PQF_Parser parser = yaz_pqf_create();
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_RPNQuery *rpn;
    enum query_charset_status status = NO_ERROR;

    YAZ_CHECK(parser);

    YAZ_CHECK(odr);

    rpn = yaz_pqf_parse(parser, odr, pqf);

    yaz_pqf_destroy(parser);

    if (!rpn)
        status = PQF_FAILED;
    else
    {
        WRBUF w = wrbuf_alloc();
        Z_RPNQuery *r2 = yaz_copy_z_RPNQuery(rpn, odr);

        YAZ_CHECK(r2);
        YAZ_CHECK(r2 != rpn);
        yaz_query_charset_convert_rpnquery(rpn, odr, cd);
        yaz_rpnquery_to_wrbuf(w, rpn);
        if (!expect_pqf || strcmp(expect_pqf, wrbuf_cstr(w)) == 0)
            status = MATCH;
        else
        {
            status = NO_MATCH;
            printf("Result: %s\n", wrbuf_cstr(w));
        }
        wrbuf_destroy(w);
    }
    odr_destroy(odr);
    return status;
}

static void tst(void)
{
    yaz_iconv_t cd = yaz_iconv_open("iso-8859-1", "utf-8");

    YAZ_CHECK(cd);
    if (!cd)
        return;

    YAZ_CHECK_EQ(t(cd, "@attr 1=4 bad query", 0), PQF_FAILED);
    YAZ_CHECK_EQ(t(cd, "@attr 1=4 ok", "@attrset Bib-1 @attr 1=4 ok"), MATCH);

    /* m followed by latin smaller letter ae */
    YAZ_CHECK_EQ(t(cd, "@attr 1=4 m\xc3\xa6", "@attrset Bib-1 @attr 1=4 m\xe6"), MATCH);

    yaz_iconv_close(cd);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
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

