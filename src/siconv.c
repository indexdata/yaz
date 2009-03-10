/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief Implements simple ICONV
 *
 * This implements an interface similar to that of iconv and
 * is used by YAZ to interface with iconv (if present).
 * For systems where iconv is not present, this layer
 * provides a few important conversions: UTF-8, MARC-8, Latin-1.
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#if HAVE_ICONV_H
#include <iconv.h>
#endif

#include <yaz/xmalloc.h>
#include <yaz/errno.h>
#include "iconv-p.h"

struct yaz_iconv_struct {
    int my_errno;
    int init_flag;
#if 0
    size_t (*init_handle)(yaz_iconv_t cd, unsigned char *inbuf,
                            size_t inbytesleft, size_t *no_read);
    unsigned long (*read_handle)(yaz_iconv_t cd, unsigned char *inbuf,
                                 size_t inbytesleft, size_t *no_read);
#endif
    size_t no_read_x;
    unsigned long unget_x;
#if HAVE_ICONV_H
    iconv_t iconv_cd;
#endif
    struct yaz_iconv_encoder_s encoder;
    struct yaz_iconv_decoder_s decoder;
};


int yaz_iconv_isbuiltin(yaz_iconv_t cd)
{
    return cd->decoder.read_handle && cd->encoder.write_handle;
}


static int prepare_encoders(yaz_iconv_t cd, const char *tocode)
{
    if (yaz_marc8_encoder(tocode, &cd->encoder))
        return 1;
    if (yaz_utf8_encoder(tocode, &cd->encoder))
        return 1;
    if (yaz_ucs4_encoder(tocode, &cd->encoder))
        return 1;
    if (yaz_iso_8859_1_encoder(tocode, &cd->encoder))
        return 1;
    if (yaz_iso_5428_encoder(tocode, &cd->encoder))
        return 1;
    if (yaz_advancegreek_encoder(tocode, &cd->encoder))
        return 1;
    if (yaz_wchar_encoder(tocode, &cd->encoder))
        return 1;
    return 0;
}

static int prepare_decoders(yaz_iconv_t cd, const char *tocode)
{
    if (yaz_marc8_decoder(tocode, &cd->decoder))
        return 1;
    if (yaz_iso5426_decoder(tocode, &cd->decoder))
        return 1;
    if (yaz_utf8_decoder(tocode, &cd->decoder))
        return 1;
    if (yaz_ucs4_decoder(tocode, &cd->decoder))
        return 1;
    if (yaz_iso_8859_1_decoder(tocode, &cd->decoder))
        return 1;
    if (yaz_iso_5428_decoder(tocode, &cd->decoder))
        return 1;
    if (yaz_advancegreek_decoder(tocode, &cd->decoder))
        return 1;
    if (yaz_wchar_decoder(tocode, &cd->decoder))
        return 1;
    if (yaz_danmarc_decoder(tocode, &cd->decoder))
        return 1;
    return 0;
}

yaz_iconv_t yaz_iconv_open(const char *tocode, const char *fromcode)
{
    yaz_iconv_t cd = (yaz_iconv_t) xmalloc (sizeof(*cd));

    cd->encoder.data = 0;
    cd->encoder.write_handle = 0;
    cd->encoder.flush_handle = 0;
    cd->encoder.init_handle = 0;
    cd->encoder.destroy_handle = 0;

    cd->decoder.data = 0;
    cd->decoder.read_handle = 0;
    cd->decoder.init_handle = 0;
    cd->decoder.destroy_handle = 0;

    cd->my_errno = YAZ_ICONV_UNKNOWN;

    /* a useful hack: if fromcode has leading @,
       the library not use YAZ's own conversions .. */
    if (fromcode[0] == '@')
        fromcode++;
    else
    {
        prepare_encoders(cd, tocode);
        prepare_decoders(cd, fromcode);
    }
    if (cd->decoder.read_handle && cd->encoder.write_handle)
    {
#if HAVE_ICONV_H
        cd->iconv_cd = (iconv_t) (-1);
#endif
        ;
    }
    else
    {
#if HAVE_ICONV_H
        cd->iconv_cd = iconv_open(tocode, fromcode);
        if (cd->iconv_cd == (iconv_t) (-1))
        {
            yaz_iconv_close(cd);
            return 0;
        }
#else
        yaz_iconv_close(cd);
        return 0;
#endif
    }
    cd->init_flag = 1;
    return cd;
}

size_t yaz_iconv(yaz_iconv_t cd, char **inbuf, size_t *inbytesleft,
                 char **outbuf, size_t *outbytesleft)
{
    char *inbuf0 = 0;
    size_t r = 0;

#if HAVE_ICONV_H
    if (cd->iconv_cd != (iconv_t) (-1))
    {
        size_t r =
            iconv(cd->iconv_cd, inbuf, inbytesleft, outbuf, outbytesleft);
        if (r == (size_t)(-1))
        {
            switch (yaz_errno())
            {
            case E2BIG:
                cd->my_errno = YAZ_ICONV_E2BIG;
                break;
            case EINVAL:
                cd->my_errno = YAZ_ICONV_EINVAL;
                break;
            case EILSEQ:
                cd->my_errno = YAZ_ICONV_EILSEQ;
                break;
            default:
                cd->my_errno = YAZ_ICONV_UNKNOWN;
            }
        }
        return r;
    }
#endif

    if (inbuf)
        inbuf0 = *inbuf;

    if (cd->init_flag)
    {
        cd->my_errno = YAZ_ICONV_UNKNOWN;
        
        if (cd->encoder.init_handle)
            (*cd->encoder.init_handle)(&cd->encoder);
        
        cd->unget_x = 0;
        cd->no_read_x = 0;

        if (cd->decoder.init_handle)
        {
            size_t no_read = 0;
            size_t r = (cd->decoder.init_handle)(
                cd, &cd->decoder,
                inbuf ? (unsigned char *) *inbuf : 0,
                inbytesleft ? *inbytesleft : 0, 
                &no_read);
            if (r)
            {
                if (cd->my_errno == YAZ_ICONV_EINVAL)
                    return r;
                cd->init_flag = 0;
                return r;
            }
            if (inbytesleft)
                *inbytesleft -= no_read;
            if (inbuf)
                *inbuf += no_read;
        }
    }
    cd->init_flag = 0;

    if (!inbuf || !*inbuf)
    {
        if (outbuf && *outbuf)
        {
            if (cd->unget_x)
                r = (*cd->encoder.write_handle)(cd, &cd->encoder,
                                                cd->unget_x, outbuf, outbytesleft);
            if (cd->encoder.flush_handle)
                r = (*cd->encoder.flush_handle)(cd, &cd->encoder,
                                                outbuf, outbytesleft);
        }
        if (r == 0)
            cd->init_flag = 1;
        cd->unget_x = 0;
        return r;
    }
    while (1)
    {
        unsigned long x;
        size_t no_read;

        if (cd->unget_x)
        {
            x = cd->unget_x;
            no_read = cd->no_read_x;
        }
        else
        {
            if (*inbytesleft == 0)
            {
                r = *inbuf - inbuf0;
                break;
            }
            x = (*cd->decoder.read_handle)(
                cd, &cd->decoder, 
                (unsigned char *) *inbuf, *inbytesleft, &no_read);
            if (no_read == 0)
            {
                r = (size_t)(-1);
                break;
            }
        }
        if (x)
        {
            r = (*cd->encoder.write_handle)(cd, &cd->encoder,
                                            x, outbuf, outbytesleft);
            if (r)
            {
                /* unable to write it. save it because read_handle cannot
                   rewind .. */
                if (cd->my_errno == YAZ_ICONV_E2BIG)
                {
                    cd->unget_x = x;
                    cd->no_read_x = no_read;
                    break;
                }
            }
            cd->unget_x = 0;
        }
        *inbytesleft -= no_read;
        (*inbuf) += no_read;
    }
    return r;
}

int yaz_iconv_error(yaz_iconv_t cd)
{
    return cd->my_errno;
}

int yaz_iconv_close(yaz_iconv_t cd)
{
#if HAVE_ICONV_H
    if (cd->iconv_cd != (iconv_t) (-1))
        iconv_close(cd->iconv_cd);
#endif
    if (cd->encoder.destroy_handle)
        (*cd->encoder.destroy_handle)(&cd->encoder);
    if (cd->decoder.destroy_handle)
        (*cd->decoder.destroy_handle)(&cd->decoder);
    xfree(cd);
    return 0;
}

void yaz_iconv_set_errno(yaz_iconv_t cd, int no)
{
    cd->my_errno = no;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

