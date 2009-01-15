/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief UCS4 decoding and encoding
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "iconv-p.h"

static unsigned long read_UCS4(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                               unsigned char *inp,
                               size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    
    if (inbytesleft < 4)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL); /* incomplete input */
        *no_read = 0;
    }
    else
    {
        x = (inp[0]<<24) | (inp[1]<<16) | (inp[2]<<8) | inp[3];
        *no_read = 4;
    }
    return x;
}

static unsigned long read_UCS4LE(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                                 unsigned char *inp,
                                 size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    
    if (inbytesleft < 4)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL); /* incomplete input */
        *no_read = 0;
    }
    else
    {
        x = (inp[3]<<24) | (inp[2]<<16) | (inp[1]<<8) | inp[0];
        *no_read = 4;
    }
    return x;
}

static size_t write_UCS4(yaz_iconv_t cd, yaz_iconv_encoder_t en,
                         unsigned long x,
                         char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = (unsigned char *) *outbuf;
    if (*outbytesleft >= 4)
    {
        *outp++ = (unsigned char) (x>>24);
        *outp++ = (unsigned char) (x>>16);
        *outp++ = (unsigned char) (x>>8);
        *outp++ = (unsigned char) x;
        (*outbytesleft) -= 4;
    }
    else
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}

static size_t write_UCS4LE(yaz_iconv_t cd, yaz_iconv_encoder_t en,
                           unsigned long x,
                           char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = (unsigned char *) *outbuf;
    if (*outbytesleft >= 4)
    {
        *outp++ = (unsigned char) x;
        *outp++ = (unsigned char) (x>>8);
        *outp++ = (unsigned char) (x>>16);
        *outp++ = (unsigned char) (x>>24);
        (*outbytesleft) -= 4;
    }
    else
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}


yaz_iconv_encoder_t yaz_ucs4_encoder(const char *tocode,
                                     yaz_iconv_encoder_t e)
    
{
    if (!yaz_matchstr(tocode, "UCS4"))
        e->write_handle = write_UCS4;
    else if (!yaz_matchstr(tocode, "UCS4LE"))
        e->write_handle = write_UCS4LE;
    else
        return 0;
    return e;
}

yaz_iconv_decoder_t yaz_ucs4_decoder(const char *tocode,
                                     yaz_iconv_decoder_t d)
    
{
    if (!yaz_matchstr(tocode, "UCS4"))
        d->read_handle = read_UCS4;
    else if (!yaz_matchstr(tocode, "UCS4LE"))
        d->read_handle = read_UCS4LE;
    else
        return 0;
    return d;
}



/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

