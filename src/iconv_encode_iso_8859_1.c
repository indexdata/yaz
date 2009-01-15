/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief ISO-8859-1 encoding / decoding
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <yaz/xmalloc.h>
#include "iconv-p.h"

struct encoder_data
{
    unsigned long compose_char;
};



static struct {
    unsigned long x1, x2;
    unsigned y;
} latin1_comb[] = {
    { 'A', 0x0300, 0xc0}, /* LATIN CAPITAL LETTER A WITH GRAVE */
    { 'A', 0x0301, 0xc1}, /* LATIN CAPITAL LETTER A WITH ACUTE */
    { 'A', 0x0302, 0xc2}, /* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
    { 'A', 0x0303, 0xc3}, /* LATIN CAPITAL LETTER A WITH TILDE */
    { 'A', 0x0308, 0xc4}, /* LATIN CAPITAL LETTER A WITH DIAERESIS */
    { 'A', 0x030a, 0xc5}, /* LATIN CAPITAL LETTER A WITH RING ABOVE */
    /* no need for 0xc6      LATIN CAPITAL LETTER AE */
    { 'C', 0x0327, 0xc7}, /* LATIN CAPITAL LETTER C WITH CEDILLA */
    { 'E', 0x0300, 0xc8}, /* LATIN CAPITAL LETTER E WITH GRAVE */
    { 'E', 0x0301, 0xc9}, /* LATIN CAPITAL LETTER E WITH ACUTE */
    { 'E', 0x0302, 0xca}, /* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
    { 'E', 0x0308, 0xcb}, /* LATIN CAPITAL LETTER E WITH DIAERESIS */
    { 'I', 0x0300, 0xcc}, /* LATIN CAPITAL LETTER I WITH GRAVE */
    { 'I', 0x0301, 0xcd}, /* LATIN CAPITAL LETTER I WITH ACUTE */
    { 'I', 0x0302, 0xce}, /* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
    { 'I', 0x0308, 0xcf}, /* LATIN CAPITAL LETTER I WITH DIAERESIS */
    { 'N', 0x0303, 0xd1}, /* LATIN CAPITAL LETTER N WITH TILDE */
    { 'O', 0x0300, 0xd2}, /* LATIN CAPITAL LETTER O WITH GRAVE */
    { 'O', 0x0301, 0xd3}, /* LATIN CAPITAL LETTER O WITH ACUTE */
    { 'O', 0x0302, 0xd4}, /* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
    { 'O', 0x0303, 0xd5}, /* LATIN CAPITAL LETTER O WITH TILDE */
    { 'O', 0x0308, 0xd6}, /* LATIN CAPITAL LETTER O WITH DIAERESIS */
    /* omitted:    0xd7      MULTIPLICATION SIGN */
    /* omitted:    0xd8      LATIN CAPITAL LETTER O WITH STROKE */
    { 'U', 0x0300, 0xd9}, /* LATIN CAPITAL LETTER U WITH GRAVE */
    { 'U', 0x0301, 0xda}, /* LATIN CAPITAL LETTER U WITH ACUTE */
    { 'U', 0x0302, 0xdb}, /* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
    { 'U', 0x0308, 0xdc}, /* LATIN CAPITAL LETTER U WITH DIAERESIS */
    { 'Y', 0x0301, 0xdd}, /* LATIN CAPITAL LETTER Y WITH ACUTE */
    /* omitted:    0xde      LATIN CAPITAL LETTER THORN */
    /* omitted:    0xdf      LATIN SMALL LETTER SHARP S */
    { 'a', 0x0300, 0xe0}, /* LATIN SMALL LETTER A WITH GRAVE */
    { 'a', 0x0301, 0xe1}, /* LATIN SMALL LETTER A WITH ACUTE */
    { 'a', 0x0302, 0xe2}, /* LATIN SMALL LETTER A WITH CIRCUMFLEX */
    { 'a', 0x0303, 0xe3}, /* LATIN SMALL LETTER A WITH TILDE */
    { 'a', 0x0308, 0xe4}, /* LATIN SMALL LETTER A WITH DIAERESIS */
    { 'a', 0x030a, 0xe5}, /* LATIN SMALL LETTER A WITH RING ABOVE */
    /* omitted:    0xe6      LATIN SMALL LETTER AE */
    { 'c', 0x0327, 0xe7}, /* LATIN SMALL LETTER C WITH CEDILLA */
    { 'e', 0x0300, 0xe8}, /* LATIN SMALL LETTER E WITH GRAVE */
    { 'e', 0x0301, 0xe9}, /* LATIN SMALL LETTER E WITH ACUTE */
    { 'e', 0x0302, 0xea}, /* LATIN SMALL LETTER E WITH CIRCUMFLEX */
    { 'e', 0x0308, 0xeb}, /* LATIN SMALL LETTER E WITH DIAERESIS */
    { 'i', 0x0300, 0xec}, /* LATIN SMALL LETTER I WITH GRAVE */
    { 'i', 0x0301, 0xed}, /* LATIN SMALL LETTER I WITH ACUTE */
    { 'i', 0x0302, 0xee}, /* LATIN SMALL LETTER I WITH CIRCUMFLEX */
    { 'i', 0x0308, 0xef}, /* LATIN SMALL LETTER I WITH DIAERESIS */
    /* omitted:    0xf0      LATIN SMALL LETTER ETH */
    { 'n', 0x0303, 0xf1}, /* LATIN SMALL LETTER N WITH TILDE */
    { 'o', 0x0300, 0xf2}, /* LATIN SMALL LETTER O WITH GRAVE */
    { 'o', 0x0301, 0xf3}, /* LATIN SMALL LETTER O WITH ACUTE */
    { 'o', 0x0302, 0xf4}, /* LATIN SMALL LETTER O WITH CIRCUMFLEX */
    { 'o', 0x0303, 0xf5}, /* LATIN SMALL LETTER O WITH TILDE */
    { 'o', 0x0308, 0xf6}, /* LATIN SMALL LETTER O WITH DIAERESIS */
    /* omitted:    0xf7      DIVISION SIGN */
    /* omitted:    0xf8      LATIN SMALL LETTER O WITH STROKE */
    { 'u', 0x0300, 0xf9}, /* LATIN SMALL LETTER U WITH GRAVE */
    { 'u', 0x0301, 0xfa}, /* LATIN SMALL LETTER U WITH ACUTE */
    { 'u', 0x0302, 0xfb}, /* LATIN SMALL LETTER U WITH CIRCUMFLEX */
    { 'u', 0x0308, 0xfc}, /* LATIN SMALL LETTER U WITH DIAERESIS */
    { 'y', 0x0301, 0xfd}, /* LATIN SMALL LETTER Y WITH ACUTE */
    /* omitted:    0xfe      LATIN SMALL LETTER THORN */
    { 'y', 0x0308, 0xff}, /* LATIN SMALL LETTER Y WITH DIAERESIS */
    
    { 0, 0, 0}
};

int yaz_iso_8859_1_lookup_y(unsigned long v,
                            unsigned long *x1, unsigned long *x2)
{
    if (v >= 0xc0 && v <= 0xff) /* optimization. min and max .y values */
    {
        int i;
        for (i = 0; latin1_comb[i].x1; i++)
        {
            if (v == latin1_comb[i].y)
            {
                *x1 = latin1_comb[i].x1;
                *x2 = latin1_comb[i].x2;
                return 1;
            }
        }
    }
    return 0;
}

int yaz_iso_8859_1_lookup_x12(unsigned long x1, unsigned long x2,
                              unsigned long *y)
{
    /* For MARC8s we try to get a Latin-1 page code out of it */
    int i;
    for (i = 0; latin1_comb[i].x1; i++)
        if (x2 == latin1_comb[i].x2 && x1 == latin1_comb[i].x1)
        {
            *y = latin1_comb[i].y;
            return 1;
        }
    return 0;
}

static size_t write_iso_8859_1(yaz_iconv_t cd, yaz_iconv_encoder_t e,
                               unsigned long x,
                               char **outbuf, size_t *outbytesleft)
{
    struct encoder_data *w = (struct encoder_data *) e->data;
    /* list of two char unicode sequence that, when combined, are
       equivalent to single unicode chars that can be represented in
       ISO-8859-1/Latin-1.
       Regular iconv on Linux at least does not seem to convert these,
       but since MARC-8 to UTF-8 generates these composed sequence
       we get a better chance of a successful MARC-8 -> ISO-8859-1
       conversion */
    unsigned char *outp = (unsigned char *) *outbuf;

    if (w->compose_char)
    {
        int i;
        for (i = 0; latin1_comb[i].x1; i++)
            if (w->compose_char == latin1_comb[i].x1 && x == latin1_comb[i].x2)
            {
                x = latin1_comb[i].y;
                break;
            }
        if (*outbytesleft < 1)
        {  /* no room. Retain compose_char and bail out */
            yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
            return (size_t)(-1);
        }
        if (!latin1_comb[i].x1) 
        {   /* not found. Just write compose_char */
            *outp++ = (unsigned char) w->compose_char;
            (*outbytesleft)--;
            *outbuf = (char *) outp;
        }
        /* compose_char used so reset it. x now holds current char */
        w->compose_char = 0;
    }

    if (x > 32 && x < 127 && w->compose_char == 0)
    {
        w->compose_char = x;
        return 0;
    }
    else if (x > 255 || x < 1)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EILSEQ);
        return (size_t) -1;
    }
    else if (*outbytesleft < 1)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
        return (size_t)(-1);
    }
    *outp++ = (unsigned char) x;
    (*outbytesleft)--;
    *outbuf = (char *) outp;
    return 0;
}

static size_t flush_iso_8859_1(yaz_iconv_t cd, yaz_iconv_encoder_t e,
                               char **outbuf, size_t *outbytesleft)
{
    struct encoder_data *w = (struct encoder_data *) e->data;
    if (w->compose_char)
    {
        unsigned char *outp = (unsigned char *) *outbuf;
        if (*outbytesleft < 1)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
            return (size_t)(-1);
        }
        *outp++ = (unsigned char) w->compose_char;
        (*outbytesleft)--;
        *outbuf = (char *) outp;
        w->compose_char = 0;
    }
    return 0;
}


void init_iso_8859_1(yaz_iconv_encoder_t e)
{
    struct encoder_data *w = (struct encoder_data *) e->data;
    w->compose_char = 0;
}

void destroy_iso_8859_1(yaz_iconv_encoder_t e)
{
    xfree(e->data);
}

yaz_iconv_encoder_t yaz_iso_8859_1_encoder(const char *tocode,
                                           yaz_iconv_encoder_t e)
    
{
    if (!yaz_matchstr(tocode, "iso88591"))
    {
        struct encoder_data *data = (struct encoder_data *)
            xmalloc(sizeof(*data));
        e->data = data;
        e->write_handle = write_iso_8859_1;
        e->flush_handle = flush_iso_8859_1;
        e->init_handle = init_iso_8859_1;
        e->destroy_handle = destroy_iso_8859_1;
        return e;
    }
    return 0;
}

static unsigned long read_ISO8859_1(yaz_iconv_t cd, 
                                    yaz_iconv_decoder_t d,
                                    unsigned char *inp,
                                    size_t inbytesleft, size_t *no_read)
{
    unsigned long x = inp[0];
    *no_read = 1;
    return x;
}

yaz_iconv_decoder_t yaz_iso_8859_1_decoder(const char *fromcode,
                                           yaz_iconv_decoder_t d)
    
{
    if (!yaz_matchstr(fromcode, "iso88591"))
    {
        d->read_handle = read_ISO8859_1;
        return d;
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

