/*
 * Copyright (c) 1995-1997, Index Data.
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
 * $log$
 */

#ifndef NMEM_H
#define NMEM_H
#include <yconfig.h>
#ifdef __cplusplus
extern "C" {
#endif

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
YAZ_EXPORT void *nmem_malloc(NMEM n, int size);
YAZ_EXPORT int nmem_total(NMEM n);
YAZ_EXPORT NMEM nmem_create(void);
YAZ_EXPORT void nmem_destroy(NMEM n);
YAZ_EXPORT char *nmem_strdup (NMEM mem, const char *src);

YAZ_EXPORT void nmem_init (void);
YAZ_EXPORT void nmem_exit (void);
#ifdef __cplusplus
}
#endif

#endif
