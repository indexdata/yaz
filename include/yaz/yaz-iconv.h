/*
 * Copyright (c) 1995-2003, Index Data.
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
 * $Id: yaz-iconv.h,v 1.3 2003-02-12 15:06:43 adam Exp $
 */

#ifndef YAZ_ICONV_H
#define YAZ_ICONV_H

YAZ_BEGIN_CDECL

typedef struct yaz_iconv_struct *yaz_iconv_t;
#define YAZ_ICONV_UNKNOWN 1
#define YAZ_ICONV_E2BIG 2
#define YAZ_ICONV_EILSEQ 3
#define YAZ_ICONV_EINVAL 4

YAZ_EXPORT yaz_iconv_t yaz_iconv_open (const char *tocode,
                                       const char *fromcode);
YAZ_EXPORT size_t yaz_iconv (yaz_iconv_t cd, char **inbuf, size_t *inbytesleft,
                             char **outbuf, size_t *outbytesleft);
YAZ_EXPORT int yaz_iconv_error (yaz_iconv_t cd);

YAZ_EXPORT int yaz_iconv_close (yaz_iconv_t cd);

YAZ_EXPORT int yaz_iconv_isbuiltin(yaz_iconv_t cd);

YAZ_EXPORT int yaz_matchstr(const char *s1, const char *s2);

YAZ_EXPORT int yaz_strcmp_del(const char *a, const char *b, const char *b_del);

YAZ_END_CDECL

#endif
