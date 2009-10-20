/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/** \file query-charset.c
    \brief converts General Terms in RPN queries
 */

#include <stdio.h>
#include <assert.h>

#include <yaz/query-charset.h>
#include <yaz/wrbuf.h>

static int yaz_query_charset_convert_buf(char *buf_in, int len_in,
                                         char **buf_out, int *len_out,
                                         ODR o, WRBUF wrbuf, yaz_iconv_t cd)
{
    int r = 0;
    wrbuf_rewind(wrbuf);
    wrbuf_iconv_write(wrbuf, cd, buf_in, len_in);
    wrbuf_iconv_reset(wrbuf, cd);

    *len_out = wrbuf_len(wrbuf);
    if (*len_out == 0)
    {   /* we assume conversion failed */
        *buf_out = buf_in;
        *len_out = len_in;
        r = -1;
    }
    else
    {
        /* conversion OK */
        *buf_out = (char*) odr_malloc(o, *len_out);
        memcpy(*buf_out, wrbuf_buf(wrbuf), *len_out);
    }
    return r;
}

static int yaz_query_charset_convert_term(Z_Term *q,
                                          ODR o, WRBUF wrbuf, yaz_iconv_t cd)
{
    int r = 0;
    switch(q->which)
    {
    case Z_Term_general:
        r = yaz_query_charset_convert_buf(
            (char *) q->u.general->buf, q->u.general->len,
            (char **) &q->u.general->buf, &q->u.general->len, o, wrbuf, cd);
        break;
    }
    return r;
}

static int yaz_query_charset_convert_operand(Z_Operand *q,
                                             ODR o, WRBUF wrbuf, yaz_iconv_t cd)
{
    int r = 0;
    switch(q->which)
    {
    case Z_Operand_APT:
        r = yaz_query_charset_convert_term(q->u.attributesPlusTerm->term,
                                              o, wrbuf, cd);
        break;
    case Z_Operand_resultSetId:
        break;
    case Z_Operand_resultAttr:
        break;
    }
    return r;
}

static int yaz_query_charset_convert_structure(Z_RPNStructure *q,
                                               ODR o, WRBUF wrbuf,
                                               yaz_iconv_t cd)
{
    int r = 0;
    switch(q->which)
    {
    case Z_RPNStructure_simple:
        r = yaz_query_charset_convert_operand(q->u.simple, o, wrbuf, cd);
        break;
    case Z_RPNStructure_complex:
        r = yaz_query_charset_convert_structure(q->u.complex->s1, o, wrbuf, cd);
        if (r == 0)
            r = yaz_query_charset_convert_structure(
                q->u.complex->s2, o, wrbuf, cd);
        break;
    }
    return r;
}

int yaz_query_charset_convert_rpnquery_check(Z_RPNQuery *q,
                                             ODR o, yaz_iconv_t cd)
{
    int r = 0;
    WRBUF wrbuf = wrbuf_alloc();
    r = yaz_query_charset_convert_structure(q->RPNStructure, o, wrbuf, cd);
    wrbuf_destroy(wrbuf);
    return r;
}

void yaz_query_charset_convert_rpnquery(Z_RPNQuery *q,
                                        ODR o, yaz_iconv_t cd)
{
    WRBUF wrbuf = wrbuf_alloc();
    yaz_query_charset_convert_structure(q->RPNStructure, o, wrbuf, cd);
    wrbuf_destroy(wrbuf);
}

void yaz_query_charset_convert_apt(Z_AttributesPlusTerm *apt,
                                   ODR o, yaz_iconv_t cd)
{
    WRBUF wrbuf = wrbuf_alloc();
    yaz_query_charset_convert_term(apt->term, o, wrbuf, cd);
    wrbuf_destroy(wrbuf);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

