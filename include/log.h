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
 * $Log: log.h,v $
 * Revision 1.2  1995-05-16 08:50:31  quinn
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

#define LOG_FATAL 0x0001
#define LOG_DEBUG 0x0002
#define LOG_WARN  0x0004
#define LOG_LOG   0x0008
#define LOG_ERRNO 0x0010     /* apend strerror to message */

#define LOG_ALL   0xffff

#define LOG_DEFAULT_LEVEL (LOG_FATAL | LOG_ERRNO | LOG_LOG | LOG_WARN)

void log_init(int level, const char *prefix, const char *name);
void logf(int level, const char *fmt, ...);
int log_mask_str (const char *str);

#endif
