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
 * $Log: nmem.h,v $
 * Revision 1.2  2000-02-28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.10  1998/10/19 15:24:20  adam
 * New nmem utility, nmem_transfer, that transfer blocks from one
 * NMEM to another.
 *
 * Revision 1.9  1998/10/13 16:00:17  adam
 * Implemented nmem_critical_{enter,leave}.
 *
 * Revision 1.8  1998/07/20 12:35:59  adam
 * Added more memory diagnostics (when NMEM_DEBUG is 1).
 *
 * Revision 1.7  1997/10/31 12:20:08  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 */

#ifndef NMEM_H
#define NMEM_H
#include <yaz/yconfig.h>

#define NMEM_DEBUG 0

#ifndef NMEM_DEBUG
#define NMEM_DEBUG 0
#endif

YAZ_BEGIN_CDECL

typedef struct nmem_block
{
    char *buf;              /* memory allocated in this block */
    int size;               /* size of buf */
    int top;                /* top of buffer */
    struct nmem_block *next;
} nmem_block;

typedef struct nmem_control
{
    int total;
    nmem_block *blocks;
    struct nmem_control *next;
} nmem_control;

typedef struct nmem_control *NMEM;

YAZ_EXPORT void nmem_reset(NMEM n);
YAZ_EXPORT int nmem_total(NMEM n);
YAZ_EXPORT char *nmem_strdup (NMEM mem, const char *src);
YAZ_EXPORT void nmem_transfer (NMEM dst, NMEM src);

YAZ_EXPORT void nmem_critical_enter (void);
YAZ_EXPORT void nmem_critical_leave (void);

#if NMEM_DEBUG

YAZ_EXPORT NMEM nmem_create_f(const char *file, int line);
YAZ_EXPORT void nmem_destroy_f(const char *file, int line, NMEM n);
YAZ_EXPORT void *nmem_malloc_f(const char *file, int line, NMEM n, int size);
#define nmem_create() nmem_create_f(__FILE__, __LINE__)
#define nmem_destroy(x) nmem_destroy_f(__FILE__, __LINE__, (x))
#define nmem_malloc(x, y) nmem_malloc_f(__FILE__, __LINE__, (x), (y))

#else

YAZ_EXPORT NMEM nmem_create(void);
YAZ_EXPORT void nmem_destroy(NMEM n);
YAZ_EXPORT void *nmem_malloc(NMEM n, int size);

#endif

YAZ_EXPORT void nmem_init (void);
YAZ_EXPORT void nmem_exit (void);

YAZ_END_CDECL

#endif
