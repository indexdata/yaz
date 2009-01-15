/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
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
#include <ctype.h>
#include <stdio.h>
#include <yaz/log.h>
#include <yaz/xmalloc.h>
#include "iconv-p.h"

struct decoder_data {
    unsigned long x_back;
};

static unsigned long read_danmarc(yaz_iconv_t cd, 
                                  yaz_iconv_decoder_t d,
                                  unsigned char *inp,
                                  size_t inbytesleft, size_t *no_read)
{
    struct decoder_data *data = (struct decoder_data *) d->data;
    unsigned long x = inp[0];

    if (data->x_back)
    {
        *no_read = 1;
        x = data->x_back;
        data->x_back = 0;
        return x;
    }

    if (x == '@')
    {
        if (inbytesleft < 2)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL);
            *no_read = 0;
            return 0;
        }
        switch(inp[1])
        {
        case '@':
        case '*':
        case 0xa4: /* CURRENCY SIGN */
            x = inp[1];
            *no_read = 2;
            break;
        case 0xe5: /* LATIN SMALL LETTER A WITH RING ABOVE */
            x = 'a';
            data->x_back = 'a';
            *no_read = 1;
            break;
        case 0xc5: /* LATIN CAPITAL LETTER A WITH RING ABOVE */
            x = 'A';
            data->x_back = 'a';
            *no_read = 1;
            break;
        default:
            if (inbytesleft < 5)
            {
                yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL);
                *no_read = 0;
                return 0;
            }
            else
            {
                unsigned long v;
                sscanf((const char *) inp+1, "%4lx", &v);
                *no_read = 5;
                x = v;
            }
        }
    }
    else
        *no_read = 1;
    return x;
}


static size_t init_danmarc(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                           unsigned char *inp,
                           size_t inbytesleft, size_t *no_read)
{
    struct decoder_data *data = (struct decoder_data *) d->data;
    data->x_back = 0;
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
        data->x_back = 0;
        d->read_handle = read_danmarc;
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

