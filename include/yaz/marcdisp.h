/*
 * Copyright (c) 1995-2002, Index Data.
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
 * $Id: marcdisp.h,v 1.6 2002-12-03 10:03:27 adam Exp $
 */

#ifndef MARCDISP_H
#define MARCDISP_H

#include <yaz/yconfig.h>
#include <yaz/wrbuf.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT int marc_display (const char *buf, FILE *outf);
YAZ_EXPORT int marc_display_ex (const char *buf, FILE *outf, int debug);
YAZ_EXPORT int marc_display_exl (const char *buf, FILE *outf, int debug,
                                 int length);
YAZ_EXPORT int atoi_n (const char *buf, int len);

YAZ_EXPORT int marc_display_wrbuf (const char *buf, WRBUF wr, int debug,
				   int bsize);

YAZ_EXPORT int yaz_marc_decode (const char *buf, WRBUF wr, int debug,
                                int bsize, int xml);

#define YAZ_MARC_LINE    0
#define YAZ_MARC_XML     1
#define YAZ_MARC_OAIMARC 2
#define YAZ_MARC_MARCXML 3

#define ISO2709_RS 035
#define ISO2709_FS 036
#define ISO2709_IDFS 037

YAZ_END_CDECL

#endif
