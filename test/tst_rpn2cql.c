/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2008 Index Data
 * See the file LICENSE for details.
 */

#include <stdlib.h>
#include <stdio.h>

#include <yaz/test.h>
#include <yaz/log.h>
#include <yaz/rpn2cql.h>
#include <yaz/wrbuf.h>
#include <yaz/pquery.h>

static int compare(cql_transform_t ct, const char *pqf, const char *cql)
{
    int ret = 0;
    ODR odr = odr_createmem(ODR_ENCODE);
    WRBUF w = wrbuf_alloc();
    Z_RPNQuery *q = p_query_rpn(odr, pqf);
    
    if (q)
    {
        int r = cql_transform_rpn2cql(ct, wrbuf_vputs, w, q);

        if (r != 0)
        {
            /* transform error */
            yaz_log(YLOG_LOG, "%s -> Error %d", pqf, r);
            if (!cql) /* also expected error? */
                ret = 1;
        }
        else if (r == 0)
        {
            yaz_log(YLOG_LOG, "%s -> %s", pqf, wrbuf_cstr(w));
            if (cql && !strcmp(wrbuf_cstr(w), cql))
                ret = 1;
        }
    }
    wrbuf_destroy(w);
    odr_destroy(odr);
    return ret;
}

static void tst(void)
{
    cql_transform_t ct = cql_transform_create();    
    YAZ_CHECK(compare(ct, "abc", "abc"));
    YAZ_CHECK(compare(ct, "@and a b", "a and b"));
    cql_transform_close(ct);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

