/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief UTF-8 encoding / decoding
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "iconv-p.h"

static size_t init_utf8(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                        unsigned char *inp,
                        size_t inbytesleft, size_t *no_read)
{
    if (!inp || inp[0] != 0xef)
    {
        *no_read = 0;
        return 0;
    }
    if (inbytesleft < 3)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL);
        return (size_t) -1;
    }
    if (inp[1] != 0xbb && inp[2] == 0xbf)
        *no_read = 3;
    else
        *no_read = 0;
    return 0;
}

unsigned long yaz_read_UTF8_char(unsigned char *inp,
                                 size_t inbytesleft, size_t *no_read,
                                 int *error)
{
    unsigned long x = 0;

    *no_read = 0; /* by default */
    if (inp[0] <= 0x7f)
    {
        x = inp[0];
        *no_read = 1;
    }
    else if (inp[0] <= 0xbf || inp[0] >= 0xfe)
    {
        *error = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xdf && inbytesleft >= 2)
    {
        if ((inp[1] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x1f) << 6) | (inp[1] & 0x3f);
            if (x >= 0x80)
                *no_read = 2;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xef && inbytesleft >= 3)
    {
        if ((inp[1] & 0xc0) == 0x80 && (inp[2] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x0f) << 12) | ((inp[1] & 0x3f) << 6) |
                (inp[2] & 0x3f);
            if (x >= 0x800)
                *no_read = 3;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }            
    else if (inp[0] <= 0xf7 && inbytesleft >= 4)
    {
        if ((inp[1] & 0xc0) == 0x80 && (inp[2] & 0xc0) == 0x80
            && (inp[3] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x07) << 18) | ((inp[1] & 0x3f) << 12) |
                ((inp[2] & 0x3f) << 6) | (inp[3] & 0x3f);
            if (x >= 0x10000)
                *no_read = 4;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xfb && inbytesleft >= 5)
    {
        if ((inp[1] & 0xc0) == 0x80 && (inp[2] & 0xc0) == 0x80
            && (inp[3] & 0xc0) == 0x80 && (inp[4] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x03) << 24) | ((inp[1] & 0x3f) << 18) |
                ((inp[2] & 0x3f) << 12) | ((inp[3] & 0x3f) << 6) |
                (inp[4] & 0x3f);
            if (x >= 0x200000)
                *no_read = 5;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }
    else if (inp[0] <= 0xfd && inbytesleft >= 6)
    {
        if ((inp[1] & 0xc0) == 0x80 && (inp[2] & 0xc0) == 0x80
            && (inp[3] & 0xc0) == 0x80 && (inp[4] & 0xc0) == 0x80
            && (inp[5] & 0xc0) == 0x80)
        {
            x = ((inp[0] & 0x01) << 30) | ((inp[1] & 0x3f) << 24) |
                ((inp[2] & 0x3f) << 18) | ((inp[3] & 0x3f) << 12) |
                ((inp[4] & 0x3f) << 6) | (inp[5] & 0x3f);
            if (x >= 0x4000000)
                *no_read = 6;
            else
                *error = YAZ_ICONV_EILSEQ;
        }
        else
            *error = YAZ_ICONV_EILSEQ;
    }
    else
        *error = YAZ_ICONV_EINVAL;  /* incomplete sentence */

    return x;
}

static unsigned long read_utf8(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                               unsigned char *inp,
                               size_t inbytesleft, size_t *no_read)
{
    int err = 0;
    int r = yaz_read_UTF8_char(inp, inbytesleft, no_read, &err);
    yaz_iconv_set_errno(cd, err);
    return r;
}


static size_t write_UTF8(yaz_iconv_t cd, yaz_iconv_encoder_t en,
                             unsigned long x,
                             char **outbuf, size_t *outbytesleft)
{
    int err = 0;
    int r = yaz_write_UTF8_char(x, outbuf, outbytesleft, &err);
    yaz_iconv_set_errno(cd, err);
    return r;
}

size_t yaz_write_UTF8_char(unsigned long x,
                           char **outbuf, size_t *outbytesleft,
                           int *error)
{
    unsigned char *outp = (unsigned char *) *outbuf;

    if (x <= 0x7f && *outbytesleft >= 1)
    {
        *outp++ = (unsigned char) x;
        (*outbytesleft)--;
    } 
    else if (x <= 0x7ff && *outbytesleft >= 2)
    {
        *outp++ = (unsigned char) ((x >> 6) | 0xc0);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 2;
    }
    else if (x <= 0xffff && *outbytesleft >= 3)
    {
        *outp++ = (unsigned char) ((x >> 12) | 0xe0);
        *outp++ = (unsigned char) (((x >> 6) & 0x3f) | 0x80);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 3;
    }
    else if (x <= 0x1fffff && *outbytesleft >= 4)
    {
        *outp++ = (unsigned char) ((x >> 18) | 0xf0);
        *outp++ = (unsigned char) (((x >> 12) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 6)  & 0x3f) | 0x80);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 4;
    }
    else if (x <= 0x3ffffff && *outbytesleft >= 5)
    {
        *outp++ = (unsigned char) ((x >> 24) | 0xf8);
        *outp++ = (unsigned char) (((x >> 18) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 12) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 6)  & 0x3f) | 0x80);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 5;
    }
    else if (*outbytesleft >= 6)
    {
        *outp++ = (unsigned char) ((x >> 30) | 0xfc);
        *outp++ = (unsigned char) (((x >> 24) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 18) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 12) & 0x3f) | 0x80);
        *outp++ = (unsigned char) (((x >> 6)  & 0x3f) | 0x80);
        *outp++ = (unsigned char) ((x & 0x3f) | 0x80);
        (*outbytesleft) -= 6;
    }
    else 
    {
        *error = YAZ_ICONV_E2BIG;  /* not room for output */
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}

yaz_iconv_encoder_t yaz_utf8_encoder(const char *tocode,
                                     yaz_iconv_encoder_t e)
    
{
    if (!yaz_matchstr(tocode, "UTF8"))
    {
        e->write_handle = write_UTF8;
        return e;
    }
    return 0;
}

yaz_iconv_decoder_t yaz_utf8_decoder(const char *fromcode,
                                     yaz_iconv_decoder_t d)
{
    if (!yaz_matchstr(fromcode, "UTF8"))
    {
        d->init_handle = init_utf8;
        d->read_handle = read_utf8;
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

