/*
 * Copyright (c) 1995-2000, Index Data.
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
 * $Log: wrbuf.h,v $
 * Revision 1.4  2001-10-23 21:00:19  adam
 * Old Z39.50 codecs gone. Added ZOOM. WRBUF MARC display util.
 *
 * Revision 1.3  2001/10/15 19:36:48  adam
 * New function marc_display_wrbuf.
 *
 * Revision 1.2  2000/02/28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.8  1999/11/03 09:05:56  adam
 * Implemented wrbuf_puts.
 *
 * Revision 1.7  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.6  1997/10/31 12:20:08  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 * Revision 1.5  1997/09/17 12:10:32  adam
 * YAZ version 1.4.
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

#define wrbuf_len(b) ((b)->pos)
#define wrbuf_buf(b) ((b)->buf)

#define wrbuf_putc(b, c) \
    (((b)->pos >= (b)->size ? wrbuf_grow(b, 1) : 0),  \
    (b)->buf[(b)->pos++] = (c), 0)

YAZ_EXPORT int marc_display_wrbuf (const char *buf, WRBUF wr, int debug,
				   int bsize);

YAZ_END_CDECL

#endif
