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
 * $Id: marcdisp.h,v 1.7 2002-12-16 13:13:53 adam Exp $
 */

#ifndef MARCDISP_H
#define MARCDISP_H

#include <yaz/yconfig.h>
#include <stdio.h>
#include <yaz/wrbuf.h>

YAZ_BEGIN_CDECL

typedef struct yaz_marc_t_ *yaz_marc_t;

/* create handler */
YAZ_EXPORT yaz_marc_t yaz_marc_create(void);
/* destroy */
YAZ_EXPORT void yaz_marc_destroy(yaz_marc_t mt);

/* set XML mode YAZ_MARC_LINE, YAZ_MARC_SIMPLEXML, ... */
YAZ_EXPORT void yaz_marc_xml(yaz_marc_t mt, int xmlmode);
#define YAZ_MARC_LINE      0
#define YAZ_MARC_SIMPLEXML 1
#define YAZ_MARC_OAIMARC   2
#define YAZ_MARC_MARCXML   3

/* set debug level, 0=none, 1=more, 2=even more, .. */
YAZ_EXPORT void yaz_marc_debug(yaz_marc_t mt, int level);

/* decode MARC in buf of size bsize. Returns >0 on success; <=0 on failure.
   On success, result in *result with size *rsize. */
YAZ_EXPORT int yaz_marc_decode_buf (yaz_marc_t mt, const char *buf, int bsize,
                                    char **result, int *rsize);

/* decode MARC in buf of size bsize. Returns >0 on success; <=0 on failure.
   On success, result in WRBUF */
YAZ_EXPORT int yaz_marc_decode_wrbuf (yaz_marc_t mt, const char *buf,
                                      int bsize, WRBUF wrbuf);

/* old functions (depricated) */
YAZ_EXPORT int marc_display (const char *buf, FILE *outf);
YAZ_EXPORT int marc_display_ex (const char *buf, FILE *outf, int debug);
YAZ_EXPORT int marc_display_exl (const char *buf, FILE *outf, int debug,
                                 int length);
YAZ_EXPORT int marc_display_wrbuf (const char *buf, WRBUF wr, int debug,
				   int bsize);
YAZ_EXPORT int yaz_marc_decode(const char *buf, WRBUF wr,
                               int debug, int bsize, int xml);


/* like atoi except that it reads exactly len characters */
YAZ_EXPORT int atoi_n (const char *buf, int len);

/* MARC control characters */
#define ISO2709_RS 035
#define ISO2709_FS 036
#define ISO2709_IDFS 037

YAZ_END_CDECL

#endif
