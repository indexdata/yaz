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
 * $Id: log.h,v 1.20 2004-11-18 15:18:13 heikki Exp $
 */

/**
 * \file log.h
 * \brief The old, deprecated header for log.c
 */

#ifndef LOG_H
#define LOG_H

#include <yaz/ylog.h>
#warning "use of log.h is deprecated, use logf.h instead"

#include <stdio.h>
#include <yaz/yconfig.h>
#include <yaz/xmalloc.h>

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
 /*     LOG_FILE   discontinued */
#define LOG_APP    YLOG_APP   /* Deprecated */
 /*     LOG_MALLOC discontinued */
#define LOG_NOTIME YLOG_NOTIME /* do not output date and time */
#define LOG_APP2   YLOG_APP2 /* deprecated */
#define LOG_APP3   YLOG_APP3 /* deprecated */
#define LOG_FLUSH  YLOG_FLUSH 
 /*     LOG_LOGLVL so new that nobody should be using it.  */

#define LOG_ALL    YLOG_ALL

#define LOG_DEFAULT_LEVEL YLOG_DEFAULT_LEVEL



#endif
