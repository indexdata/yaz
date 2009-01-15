/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief WCHAR_T iconv encoding / decoding
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#if HAVE_WCHAR_H
#include <wchar.h>
#endif

#include <yaz/xmalloc.h>
#include "iconv-p.h"

struct encoder_data
{
    unsigned long compose_char;
};

#if HAVE_WCHAR_H
static size_t write_wchar_t(yaz_iconv_t cd, yaz_iconv_encoder_t en,
			    unsigned long x,
			    char **outbuf, size_t *outbytesleft)
{
    unsigned char *outp = (unsigned char *) *outbuf;

    if (*outbytesleft >= sizeof(wchar_t))
    {
        wchar_t wch = x;
        memcpy(outp, &wch, sizeof(wch));
        outp += sizeof(wch);
        (*outbytesleft) -= sizeof(wch);
    }
    else
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
        return (size_t)(-1);
    }
    *outbuf = (char *) outp;
    return 0;
}
#endif

yaz_iconv_encoder_t yaz_wchar_encoder(const char *tocode,
				      yaz_iconv_encoder_t e)
    
{
#if HAVE_WCHAR_H
    if (!yaz_matchstr(tocode, "wchar_t"))
    {
        e->write_handle = write_wchar_t;
        return e;
    }
#endif
    return 0;
}

#if HAVE_WCHAR_H
static unsigned long read_wchar_t(yaz_iconv_t cd, yaz_iconv_decoder_t d,
                                  unsigned char *inp,
                                  size_t inbytesleft, size_t *no_read)
{
    unsigned long x = 0;
    
    if (inbytesleft < sizeof(wchar_t))
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EINVAL); /* incomplete input */
        *no_read = 0;
    }
    else
    {
        wchar_t wch;
        memcpy(&wch, inp, sizeof(wch));
        x = wch;
        *no_read = sizeof(wch);
    }
    return x;
}
#endif

yaz_iconv_decoder_t yaz_wchar_decoder(const char *fromcode,
				      yaz_iconv_decoder_t d)
    
{
#if HAVE_WCHAR_H
    if (!yaz_matchstr(fromcode, "wchar_t"))
    {
        d->read_handle = read_wchar_t;
        return d;
    }
#endif
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

