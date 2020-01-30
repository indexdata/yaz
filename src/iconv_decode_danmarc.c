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

static unsigned long read_danmarc(yaz_iconv_t cd,
                                  yaz_iconv_decoder_t d,
                                  unsigned char *inp,
                                  size_t inbytesleft, size_t *no_read)
{
    unsigned long x = inp[0];

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
        if (x < 0x300 || x > 0x36F || x == 0x303 || data->sz >= MAX_COMP)
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


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

