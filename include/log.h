/*
 * Copyright (c) 1995-1998, Index Data.
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
 * $Log: log.h,v $
 * Revision 1.13  1998-10-13 16:11:11  adam
 * Added printf-format check for logf when using GNUC.
 *
 * Revision 1.12  1997/09/04 07:59:02  adam
 * Added include of xmalloc.h.
 *
 * Revision 1.11  1997/09/01 08:49:48  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.10  1997/05/14 06:53:40  adam
 * C++ support.
 *
 * Revision 1.9  1997/05/01 15:06:42  adam
 * Added log_mask_str_x routine.
 *
 * Revision 1.8  1996/05/01 12:45:00  quinn
 * *** empty log message ***
 *
 * Revision 1.7  1996/02/05  12:24:26  adam
 * Implemented log_event_{start,end}-functions.
 *
 * Revision 1.6  1995/10/10  16:27:06  quinn
 * *** empty log message ***
 *
 * Revision 1.5  1995/09/29  17:12:03  quinn
 * Smallish
 *
 * Revision 1.4  1995/09/27  15:02:47  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.3  1995/06/19  12:38:25  quinn
 * Reorganized include-files. Added small features.
 *
 * Revision 1.2  1995/05/16  08:50:31  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/30  09:39:41  quinn
 * Moved .h files to include directory
 *
 * Revision 1.4  1994/09/28  13:07:22  adam
 * Added log_mask_str.
 *
 * Revision 1.3  1994/08/18  08:18:45  quinn
 * Added prefix to log_init.
 *
 * Revision 1.2  1994/08/17  14:27:46  quinn
 * added LOG_ERRNO
 *
 * Revision 1.1  1994/08/17  13:22:52  quinn
 * First version
 *
 */

#ifndef LOG_H
#define LOG_H

#include <yconfig.h>
#include <xmalloc.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_FATAL 0x0001
#define LOG_DEBUG 0x0002
#define LOG_WARN  0x0004
#define LOG_LOG   0x0008
#define LOG_ERRNO 0x0010     /* append strerror to message */
#define LOG_FILE  0x0020

#define LOG_ALL   0xffff

#define LOG_DEFAULT_LEVEL (LOG_FATAL | LOG_ERRNO | LOG_LOG | LOG_WARN)

YAZ_EXPORT void log_init(int level, const char *prefix, const char *name);
YAZ_EXPORT void logf(int level, const char *fmt, ...)
#ifdef __GNUC__
	__attribute__ ((format (printf, 2, 3)))
#endif
	;
YAZ_EXPORT int log_mask_str (const char *str);
YAZ_EXPORT int log_mask_str_x (const char *str, int level);
YAZ_EXPORT FILE *log_file(void);

YAZ_EXPORT void log_event_start (void (*func)(int level, const char *msg, void *info),
	void *info);
YAZ_EXPORT void log_event_end (void (*func)(int level, const char *msg, void *info),
	void *info);

#ifdef __cplusplus
}
#endif

#endif
