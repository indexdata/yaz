/*
 * Copyright (C) 1995-2005, Index Data ApS
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
 * $Id: xmalloc.h,v 1.6 2005-01-16 21:51:49 adam Exp $
 */
/**
 * \file xmalloc.h
 * \brief Header for malloc interface.
 */

#ifndef XMALLOC_H
#define XMALLOC_H

#include <stddef.h>

#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

#define xrealloc(o, x) xrealloc_f(o, x, __FILE__, __LINE__)
#define xmalloc(x) xmalloc_f(x, __FILE__, __LINE__)
#define xcalloc(x,y) xcalloc_f(x,y, __FILE__, __LINE__)
#define xfree(x) xfree_f(x, __FILE__, __LINE__)
#define xstrdup(s) xstrdup_f(s, __FILE__, __LINE__)
#define xmalloc_trav(s) xmalloc_trav_f(s, __FILE__, __LINE__)
    
YAZ_EXPORT void *xrealloc_f (void *o, size_t size, const char *file, int line);
YAZ_EXPORT void *xmalloc_f (size_t size, const char *file, int line);
YAZ_EXPORT void *xcalloc_f (size_t nmemb, size_t size,
			    const char *file, int line);
YAZ_EXPORT char *xstrdup_f (const char *p, const char *file, int line);
YAZ_EXPORT void xfree_f (void *p, const char *file, int line);
YAZ_EXPORT void xmalloc_trav_f(const char *s, const char *file, int line);

YAZ_END_CDECL

#endif
