/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief Danmarc2 character set decoding
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <yaz/log.h>
#include <yaz/xmalloc.h>
#include "iconv-p.h"

#define MAX_COMP 4

struct decoder_data {
    unsigned long comp[MAX_COMP];
    size_t no_read[MAX_COMP];
    size_t sz;
};

static unsigned long read_useq(yaz_iconv_t cd,
                               yaz_iconv_decoder_t d,
                               unsigned char *inp,
                               size_t inbytesleft, size_t *no_read)
{
    static unsigned long u_seq[] = {
        0xA7, 0x2b9,
        0xAE, 0x2bc,
        0xB0, 0x2bb,
        0xB7, 0x2ba,
        0xD8, 0x2014, /* Note 13 */
        0xD9, 0x2191, /* Note 14 */
        0xDA, 0x2193, /* Note 15 */
        0xE0, 0x309,
        0xEB, 0xfe20, /* Note 20 */
        0xEC, 0xfe21, /* Note 21 */
        0xF7, 0x326,
        0xF8, 0x31c,
        0xF9, 0x32e,
        0xFA, 0xfe22, /* Note 26 */
        /* 0xFB Note 27 */
        0xFC, 0x308,
        0xFD, 0xf0fd,
        0xFE, 0xf0fe,
        0xFF, 0xf0ff,
        0};
    int i;
    unsigned long x;

    if (inbytesleft < 4)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL);
        *no_read = 0;
        return 0;
    }
    sscanf((const char *) inp+2, "%2lx", &x);
    *no_read = 4;
    for (i = 0; u_seq[i]; i += 2)
        if (x == u_seq[i])
            return u_seq[i+1];
    return x;
}

static unsigned long read_danmarc(yaz_iconv_t cd,
                                  yaz_iconv_decoder_t d,
                                  unsigned char *inp,
                                  size_t inbytesleft, size_t *no_read)
{
    unsigned long x;

    if (inbytesleft < 1)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL);
        *no_read = 0;
        return 0;
    }
    x = inp[0];
    if (x != '@')
    {
        *no_read = 1;
        return x;
    }
    if (inbytesleft < 2)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL);
        *no_read = 0;
        return 0;
    }
    switch (inp[1])
    {
    case '@':
    case '*':
    case 0xa4: /* CURRENCY SIGN */
        x = inp[1];
        *no_read = 2;
        break;
    case 0xe5: /* LATIN SMALL LETTER A WITH RING ABOVE */
        x = 0xa733;
        *no_read = 2;
        break;
    case 0xc5: /* LATIN CAPITAL LETTER A WITH RING ABOVE */
        x = 0xa732;
        *no_read = 2;
        break;
    default:
        if (inp[1] == 'U')
            return read_useq(cd, d, inp, inbytesleft, no_read);
        if (inbytesleft < 5)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL);
            *no_read = 0;
            return 0;
        }
        sscanf((const char *) inp+1, "%4lx", &x);
        *no_read = 5;
    }
    return x;
}

static unsigned long swap_seq[] = {
    0x5e, 0x302,
    0x5f, 0x332,
    0x60, 0x300,
    0xa8, 0x308,
    0xaf, 0x304,
    0xb4, 0x301,
    0xb8, 0x327,
    0x02c7, 0x30c,
    0x02d8, 0x306,
    0x02da, 0x30a,
    0x02db, 0x328,
    0x02dd, 0x30b,
    0
};

unsigned long yaz_danmarc_swap_to_danmarc(unsigned long x)
{
    int i;
    for (i = 0; swap_seq[i]; i += 2)
        if (swap_seq[i+1] == x)
            return swap_seq[i];
    return x;
}

static unsigned long yaz_danmarc_swap_to_utf8(unsigned long x)
{
    int i;
    for (i = 0; swap_seq[i]; i += 2)
        if (swap_seq[i] == x)
            return swap_seq[i+1];
    return x;
}

static unsigned long read_danmarc_comb(yaz_iconv_t cd,
                                       yaz_iconv_decoder_t d,
                                       unsigned char *inp,
                                       size_t inbytesleft, size_t *no_read)
{
    struct decoder_data *data = (struct decoder_data *) d->data;
    unsigned long x;

    if (data->sz)
    {
        *no_read = data->no_read[--data->sz];
        return data->comp[data->sz];
    }
    while (1)
    {
        x = read_danmarc(cd, d, inp, inbytesleft, no_read);
        if (x)
            x = yaz_danmarc_swap_to_utf8(x);
        if (data->sz >= MAX_COMP)
            break;
        if (!yaz_danmarc_is_combining(x))
            break;
        data->no_read[data->sz] = *no_read;
        data->comp[data->sz++] = x;
        inp += *no_read;
        inbytesleft -= *no_read;
    }
    return x;
}

static size_t init_danmarc(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                           unsigned char *inp,
                           size_t inbytesleft, size_t *no_read)
{
    struct decoder_data *data = (struct decoder_data *) d->data;
    data->sz = 0;
    return 0;
}

void destroy_danmarc(yaz_iconv_decoder_t d)
{
    struct decoder_data *data = (struct decoder_data *) d->data;
    xfree(data);
}

yaz_iconv_decoder_t yaz_danmarc_decoder(const char *fromcode,
                                        yaz_iconv_decoder_t d)

{
    if (!yaz_matchstr(fromcode, "danmarc"))
    {
        struct decoder_data *data = (struct decoder_data *)
            xmalloc(sizeof(*data));
        d->data = data;
        d->read_handle = read_danmarc_comb;
        d->init_handle = init_danmarc;
        d->destroy_handle = destroy_danmarc;
        return d;
    }
    return 0;
}

int yaz_danmarc_is_combining(unsigned long x)
{
    /* https://en.wikipedia.org/wiki/Combining_character */
    int ranges[] = {
        0x300, 0x36F,
        0x1AB0, 0x1AFF,
        0x1DC0, 0x1DFF,
        0x20D0, 0x20FF,
        0xFE20, 0xFE2F,
        0};
    int i = 0;
    while (ranges[i] != 0 && x >= ranges[i])
    {
        if (x <= ranges[i+1])
            return 1;
        i += 2;
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

