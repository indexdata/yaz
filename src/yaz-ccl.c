/*
 * Copyright (c) 1996-2004, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: yaz-ccl.c,v 1.2 2004-10-15 00:19:01 adam Exp $
 */
/**
 * \file yaz-ccl.c
 * \brief Implements CCL node tree to RPN converson.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <yaz/yaz-ccl.h>
#include <yaz/pquery.h>

Z_RPNQuery *ccl_rpn_query (ODR o, struct ccl_rpn_node *p)
{
    YAZ_PQF_Parser parser = yaz_pqf_create();
    WRBUF wr = wrbuf_alloc();
    Z_RPNQuery *q;

    ccl_pquery(wr, p);

    q = yaz_pqf_parse(parser, o, wrbuf_buf(wr));

    wrbuf_free(wr, 1);
    yaz_pqf_destroy(parser);
    return q;
}

Z_AttributesPlusTerm *ccl_scan_query (ODR o, struct ccl_rpn_node *p)
{
    YAZ_PQF_Parser parser = yaz_pqf_create();
    WRBUF wr = wrbuf_alloc();
    Z_AttributesPlusTerm *q;
    Odr_oid *setp;

    ccl_pquery(wr, p);

    q = yaz_pqf_scan(parser, o, &setp, wrbuf_buf(wr));

    wrbuf_free(wr, 1);
    yaz_pqf_destroy(parser);
    return q;
}

