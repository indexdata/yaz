/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief Danmarc2 character set encoding
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <yaz/xmalloc.h>
#include "iconv-p.h"

#define MAX_COMP 4

struct encoder_data
{
    unsigned long comp[MAX_COMP];
    size_t sz;
    unsigned long base_char;
    int dia;
};

static size_t write1(yaz_iconv_t cd, unsigned long x,
                     char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = (unsigned char *) *outbuf;
    if (x == '@' || x == '*')
    {
        if (*outbytesleft < 2)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
            return (size_t)(-1);
        }
        *outp++ = '@';
        (*outbytesleft)--;
        *outp++ = (unsigned char) x;
        (*outbytesleft)--;
    }
    else if (x <= 0xff)
    {  /* latin-1 range */
        if (*outbytesleft < 1)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
            return (size_t)(-1);
        }
        *outp++ = (unsigned char) x;
        (*outbytesleft)--;
    }
    else if (x <= 0xffff)
    {
        if (*outbytesleft < 6)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
            return (size_t)(-1);
        }
        switch (x)
        {
        case 0xa733:
            *outp++ = '@';
            *outp++ = 0xe5;
            (*outbytesleft) -= 2;
            break;
        case 0xa732:
            *outp++ = '@';
            *outp++ = 0xc5;
            (*outbytesleft) -= 2;
            break;
        default:
            /* full unicode, emit @XXXX */
            sprintf(*outbuf, "@%04lX", x);
            outp += 5;
            (*outbytesleft) -= 5;
            break;
        }
    }
    else
    { /* can not be encoded in Danmarc2 */
        yaz_iconv_set_errno(cd, YAZ_ICONV_EILSEQ);
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}

static size_t flush_danmarc(yaz_iconv_t cd, yaz_iconv_encoder_t e,
                            char **outbuf, size_t *outbytesleft)
{
    struct encoder_data *w = (struct encoder_data *) e->data;
    /* see if this is a combining thing that can be mapped to Latin-1 */
    if (w->base_char && w->sz == 1)
    {
        unsigned long y;
        if (yaz_iso_8859_1_lookup_x12(w->base_char, w->comp[0], &y))
        {
            w->base_char = y;
            w->sz = 0;
        }
    }
    /* combining characters in reverse */
    while (w->sz > 0)
    {
        unsigned long x = w->comp[w->sz - 1];
        if (w->dia)
            x = yaz_danmarc_swap_to_danmarc(x);
        size_t r = write1(cd, x, outbuf, outbytesleft);
        if (r)
            return r;
        w->sz--;
    }
    /* then base char */
    if (w->base_char)
    {
        size_t r = write1(cd, w->base_char, outbuf, outbytesleft);
        if (r)
            return r; /* if we fail base_char is still there.. */
        w->base_char = 0;
    }
    return 0;
}

static size_t write_danmarc(yaz_iconv_t cd, yaz_iconv_encoder_t e,
			    unsigned long x,
			    char **outbuf, size_t *outbytesleft)
{
    struct encoder_data *w = (struct encoder_data *) e->data;

    /* check for combining characters */
    if (yaz_danmarc_is_combining(x))
    {
        w->comp[w->sz++] = x;
        if (w->sz < MAX_COMP)
            return 0;
    }
    /* x is NOT a combining character. Flush previous sequence */
    size_t r = flush_danmarc(cd, e, outbuf, outbytesleft);
    if (r)
        return r;
    w->base_char = x;
    return 0;
}

static void init_danmarc(yaz_iconv_encoder_t e)
{
    struct encoder_data *w = (struct encoder_data *) e->data;
    w->base_char = 0;
    w->sz = 0;
}

static void destroy_danmarc(yaz_iconv_encoder_t e)
{
    xfree(e->data);
}

yaz_iconv_encoder_t yaz_danmarc_encoder(const char *tocode,
                                        yaz_iconv_encoder_t e)

{
    if (!yaz_matchstr(tocode, "danmarc"))
    {
        struct encoder_data *data = (struct encoder_data *)
            xmalloc(sizeof(*data));
        data->dia = 0;
        e->data = data;
        e->write_handle = write_danmarc;
        e->flush_handle = flush_danmarc;
        e->init_handle = init_danmarc;
        e->destroy_handle = destroy_danmarc;
        return e;
    }
    if (!yaz_matchstr(tocode, "danmarc2"))
    {
        struct encoder_data *data = (struct encoder_data *)
            xmalloc(sizeof(*data));
        data->dia = 1;
        e->data = data;
        e->write_handle = write_danmarc;
        e->flush_handle = flush_danmarc;
        e->init_handle = init_danmarc;
        e->destroy_handle = destroy_danmarc;
        return e;
    }
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

