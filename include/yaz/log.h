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
 * $Id: log.h,v 1.8 2003-01-06 08:20:27 adam Exp $
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <yaz/yconfig.h>
#include <yaz/xmalloc.h>

YAZ_BEGIN_CDECL

#define LOG_FATAL  0x0001
#define LOG_DEBUG  0x0002
#define LOG_WARN   0x0004
#define LOG_LOG    0x0008
#define LOG_ERRNO  0x0010     /* append strerror to message */
#define LOG_FILE   0x0020
#define LOG_APP    0x0040     /* For application level events such as new-connection */
#define LOG_MALLOC 0x0080     /* debugging mallocs */

#define LOG_ALL   0xff7f

#define LOG_DEFAULT_LEVEL (LOG_FATAL | LOG_ERRNO | LOG_LOG | LOG_WARN)

#define logf yaz_log

YAZ_EXPORT void yaz_log_init(int level, const char *prefix, const char *name);
YAZ_EXPORT void yaz_log_init_file (const char *fname);
YAZ_EXPORT void yaz_log_init_level (int level);
YAZ_EXPORT void yaz_log_init_prefix (const char *prefix);
YAZ_EXPORT void yaz_log_init_prefix2 (const char *prefix);

YAZ_EXPORT void yaz_log(int level, const char *fmt, ...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 2, 3)))
#endif
	;
YAZ_EXPORT int yaz_log_mask_str (const char *str);
YAZ_EXPORT int yaz_log_mask_str_x (const char *str, int level);
YAZ_EXPORT FILE *yaz_log_file(void);

YAZ_EXPORT void log_event_start (void (*func)(int level, const char *msg, void *info),
	void *info);
YAZ_EXPORT void log_event_end (void (*func)(int level, const char *msg, void *info),
	void *info);

YAZ_END_CDECL

#endif
