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
 * $Id: tpath.h,v 1.4 2002-04-05 12:46:07 adam Exp $
 *
 */

#ifndef TPATH_H
#define TPATH_H

#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT FILE *yaz_fopen(const char *path, const char *name,
                           const char *mode, const char *base);
YAZ_EXPORT FILE *yaz_path_fopen(const char *path, const char *name,
                           const char *mode);

YAZ_EXPORT int yaz_fclose(FILE *f);

YAZ_EXPORT int yaz_is_abspath (const char *p);

YAZ_END_CDECL

#endif
