/*
 * Copyright (c) 1995, Index Data.
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
 * $Log: dmalloc.h,v $
 * Revision 1.5  1995-10-16 13:51:43  quinn
 * Changes to provide Especs to the backend.
 *
 * Revision 1.4  1995/09/29  17:12:02  quinn
 * Smallish
 *
 * Revision 1.3  1995/09/27  15:02:47  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/05/16  08:50:30  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/30  09:39:40  quinn
 * Moved .h files to include directory
 *
 * Revision 1.1  1995/03/27  08:35:18  quinn
 * Created util library
 * Added memory debugging module. Imported options-manager
 *
 *
 */

#ifndef DMALLOC_H
#define DMALLOC_H

#ifdef DEBUG_MALLOC

#ifdef malloc
#undef malloc
#endif
#ifdef free
#undef free
#endif
#ifdef realloc
#undef realloc
#endif
#define malloc(n) d_malloc(__FILE__, __LINE__, (n))
#define free(p) d_free(__FILE__, __LINE__, (p))
#define realloc(p, n) d_realloc(__FILE__, __LINE__, (p), (n))

void *d_malloc(char *file, int line, int nbytes);
void d_free(char *file, int line, char *ptr);
void *d_realloc(char *file, int line, char *ptr, int nbytes);

#endif

#endif
