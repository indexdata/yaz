/*
 * Copyright (C) 1995-2006, Index Data ApS
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The name of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * $Id: yaz-iconv.h,v 1.9 2006-04-19 23:15:39 adam Exp $
 */
/**
 * \file yaz-iconv.h
 * \brief Header for YAZ iconv interface
 */

#ifndef YAZ_ICONV_H
#define YAZ_ICONV_H

#include <stddef.h>
#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

/** \brief yaz_iconv handle (similar to iconv_t) */
typedef struct yaz_iconv_struct *yaz_iconv_t;

/** \brief error code: unknown */
#define YAZ_ICONV_UNKNOWN 1
/** \brief error code: Not sufficient room for output buffer */
#define YAZ_ICONV_E2BIG 2
/** \brief error code: Invalid sequence */
#define YAZ_ICONV_EILSEQ 3
/** \brief error code: An incomplete multibyte sequence is in input buffer */
#define YAZ_ICONV_EINVAL 4

/** \brief just like iconv_open(3) */
YAZ_EXPORT yaz_iconv_t yaz_iconv_open (const char *tocode,
                                       const char *fromcode);
/** \brief just like iconv(3) */
YAZ_EXPORT size_t yaz_iconv (yaz_iconv_t cd, char **inbuf, size_t *inbytesleft,
                             char **outbuf, size_t *outbytesleft);
/** \brief returns last error - like errno for iconv(3) */
YAZ_EXPORT int yaz_iconv_error (yaz_iconv_t cd);

/** \brief just like iconv_close(3) */
YAZ_EXPORT int yaz_iconv_close (yaz_iconv_t cd);

/** \brief tests whether conversion is handled by YAZ' iconv or system iconv */
YAZ_EXPORT int yaz_iconv_isbuiltin(yaz_iconv_t cd);

YAZ_EXPORT int yaz_matchstr(const char *s1, const char *s2);

YAZ_EXPORT int yaz_strcmp_del(const char *a, const char *b, const char *b_del);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

