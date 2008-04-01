/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2008 Index Data
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
#include <yaz/nmem.h>
#include <yaz/snprintf.h>
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


/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
