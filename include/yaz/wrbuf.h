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
 * 2. The names of Index Data or the individual authors may not be used to
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
 * $Id: wrbuf.h,v 1.6 2002-10-22 10:05:36 adam Exp $
 *
 */

#ifndef WRBUF_H
#define WRBUF_H

#include <yaz/xmalloc.h>

YAZ_BEGIN_CDECL

typedef struct wrbuf
{
    char *buf;
    int pos;
    int size;
} wrbuf, *WRBUF;

YAZ_EXPORT WRBUF wrbuf_alloc(void);
YAZ_EXPORT void wrbuf_free(WRBUF b, int free_buf);
YAZ_EXPORT void wrbuf_rewind(WRBUF b);
YAZ_EXPORT int wrbuf_grow(WRBUF b, int minsize);
YAZ_EXPORT int wrbuf_write(WRBUF b, const char *buf, int size);
YAZ_EXPORT int wrbuf_puts(WRBUF b, const char *buf);
YAZ_EXPORT void wrbuf_printf(WRBUF b, const char *fmt, ...);

#define wrbuf_len(b) ((b)->pos)
#define wrbuf_buf(b) ((b)->buf)

#define wrbuf_putc(b, c) \
    (((b)->pos >= (b)->size ? wrbuf_grow(b, 1) : 0),  \
    (b)->buf[(b)->pos++] = (c), 0)

YAZ_EXPORT int marc_display_wrbuf (const char *buf, WRBUF wr, int debug,
				   int bsize);

YAZ_EXPORT int yaz_marc_decode (const char *buf, WRBUF wr, int debug,
                                int bsize, int xml);

YAZ_END_CDECL

#endif
