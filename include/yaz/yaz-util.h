/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: yaz-util.h,v 1.5 2002-10-04 19:06:36 adam Exp $
 */

#ifndef YAZ_UTIL_H
#define YAZ_UTIL_H

#include <yaz/yconfig.h>
#include <yaz/xmalloc.h>
#include <yaz/log.h>
#include <yaz/tpath.h>
#include <yaz/options.h>
#include <yaz/wrbuf.h>
#include <yaz/nmem.h>
#include <yaz/readconf.h>
#include <yaz/marcdisp.h>

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

YAZ_EXPORT int yaz_matchstr(const char *s1, const char *s2);

YAZ_END_CDECL

#endif
