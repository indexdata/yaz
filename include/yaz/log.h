/*
 * Copyright (c) 1995-2004, Index Data.
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
 * $Id: log.h,v 1.26 2004-12-09 09:37:00 adam Exp $
 */

/**
 * \file log.h
 * \brief The old, deprecated header for log.c
 */

#ifndef LOG_H
#define LOG_H

#include <yaz/ylog.h>

#ifndef YAZ_USE_OLD_LOG
/* #warning "use of log.h is deprecated, use ylog.h instead" */
/* if this warning gets on your nerves, run configure like this:
 * CFLAGS="-Wall -g -D YAZ_USE_OLD_LOG" ./configure
 */
#endif

YAZ_BEGIN_CDECL

/* The old LOG_ bit names are here for compatibility only. They may 
 * conflict with bits defined in syslog.h, or other places. 'LOG'
 * really is not such a good name. YLOG must be more unique
 */
#define LOG_FATAL  YLOG_FATAL
#define LOG_DEBUG  YLOG_DEBUG
#define LOG_WARN   YLOG_WARN
#define LOG_LOG    YLOG_LOG /* Deprecated, use the modern dynamic log levels*/
#define LOG_ERRNO  YLOG_ERRNO 
#define LOG_FILE   0x00000020 /* Deprecated - not in ylog.h at all*/
#define LOG_APP    0x00000040 /* Deprecated - not in ylog.h at all*/
#define LOG_MALLOC YLOG_MALLOC /* deprecated */
#define LOG_NOTIME YLOG_NOTIME /* do not output date and time */
#define LOG_APP2   0x00000200 /* Deprecated - not in ylog.h at all*/
#define LOG_APP3   0x00000400 /* Deprecated - not in ylog.h at all*/
#define LOG_FLUSH  YLOG_FLUSH 
 /*     LOG_LOGLVL is a new one in ylog.h. So new that no log.h users should
  *     use it */

#define LOG_ALL    YLOG_ALL

#define LOG_DEFAULT_LEVEL YLOG_DEFAULT_LEVEL


/* logf is deprecated, as it conflicts with a math function */
#define logf yaz_log

YAZ_END_CDECL

#endif
