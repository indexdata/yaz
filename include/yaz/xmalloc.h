/*
 * Copyright (c) 1995-1999, Index Data.
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
 * $Log: xmalloc.h,v $
 * Revision 1.1  1999-11-30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.6  1999/07/13 13:24:53  adam
 * Updated memory debugging memory allocatation routines.
 *
 * Revision 1.5  1998/07/20 12:36:22  adam
 * Minor changes.
 *
 * Revision 1.4  1997/10/31 12:20:08  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 * Revision 1.3  1997/09/01 08:49:54  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.2  1997/05/14 06:53:53  adam
 * C++ support.
 *
 * Revision 1.1  1995/11/01 11:55:41  quinn
 * Added xmalloc.c
 *
 * Revision 1.8  1995/10/16  14:03:07  quinn
 * Changes to support element set names and espec1
 *
 * Revision 1.7  1994/10/05  10:15:18  quinn
 * Added xrealloc.
 *
 * Revision 1.6  1994/09/26  16:31:24  adam
 * Minor changes. xmalloc declares xcalloc now.
 *
 * Revision 1.5  1994/09/19  15:46:34  quinn
 * Added stdlib.h
 *
 * Revision 1.4  1994/08/18  08:22:27  adam
 * Res.h modified. xmalloc now declares xstrdup.
 *
 * Revision 1.3  1994/08/17  15:34:15  adam
 * Initial version of resource manager.
 *
 * Revision 1.2  1994/08/17  14:09:48  quinn
 * Small changes
 *
 * Revision 1.1  1994/08/17  13:39:07  adam
 * Added xmalloc header.
 *
 * Revision 1.1  1994/08/16  16:16:02  adam
 * bfile header created.
 *
 */

#ifndef XMALLOC_H
#define XMALLOC_H

#include <sys/types.h>
#include <stdlib.h>

#include <yaz/yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
