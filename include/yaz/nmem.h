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
 * $Id: nmem.h,v 1.11 2003-03-18 13:34:35 adam Exp $
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

typedef struct nmem_mutex *NMEM_MUTEX;
YAZ_EXPORT void nmem_mutex_create(NMEM_MUTEX *);
YAZ_EXPORT void nmem_mutex_enter(NMEM_MUTEX);
YAZ_EXPORT void nmem_mutex_leave(NMEM_MUTEX);
YAZ_EXPORT void nmem_mutex_destroy(NMEM_MUTEX *);

typedef struct nmem_control *NMEM;

YAZ_EXPORT void nmem_reset(NMEM n);
YAZ_EXPORT int nmem_total(NMEM n);
YAZ_EXPORT char *nmem_strdup (NMEM mem, const char *src);
YAZ_EXPORT char *nmem_strdupn (NMEM mem, const char *src, size_t n);
YAZ_EXPORT int *nmem_intdup (NMEM mem, int v);
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

YAZ_EXPORT void nmem_print_list (void);

#else

YAZ_EXPORT NMEM nmem_create(void);
YAZ_EXPORT void nmem_destroy(NMEM n);
YAZ_EXPORT void *nmem_malloc(NMEM n, int size);

#define nmem_print_list()

#endif

YAZ_EXPORT void nmem_init (void);
YAZ_EXPORT void nmem_exit (void);
YAZ_EXPORT int yaz_errno (void);
YAZ_EXPORT void yaz_set_errno (int v);
YAZ_EXPORT void yaz_strerror(char *buf, int max);

YAZ_END_CDECL

#endif
