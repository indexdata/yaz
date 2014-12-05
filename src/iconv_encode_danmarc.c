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

static size_t write_danmarc(yaz_iconv_t cd, yaz_iconv_encoder_t en,
			    unsigned long x,
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
    else if (x <= 255)
    {  /* latin-1 range */
        if (*outbytesleft < 1)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
            return (size_t)(-1);
        }
        *outp++ = (unsigned char) x;
        (*outbytesleft)--;
    }
    else
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
    *outbuf = (char *) outp;
    return 0;
}

yaz_iconv_encoder_t yaz_danmarc_encoder(const char *tocode,
                                        yaz_iconv_encoder_t e)

{
    if (!yaz_matchstr(tocode, "danmarc"))
    {
        e->write_handle = write_danmarc;
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

