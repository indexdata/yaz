/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
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

    q = yaz_pqf_parse(parser, o, wrbuf_cstr(wr));

    wrbuf_destroy(wr);
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

    q = yaz_pqf_scan(parser, o, &setp, wrbuf_cstr(wr));

    wrbuf_destroy(wr);
    yaz_pqf_destroy(parser);
    return q;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

