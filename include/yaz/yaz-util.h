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
 * $Id: yaz-util.h,v 1.13 2004-12-10 10:42:33 heikki Exp $
 */
/**
 * \file yaz-util.h
 * \brief Header for common YAZ utilities
 */

#ifndef YAZ_UTIL_H
#define YAZ_UTIL_H

#include <yaz/yconfig.h>
#include <yaz/yaz-version.h>
#include <yaz/xmalloc.h>

/* [y]log.h trickery */
/* if [y]log.h has been included, do not worry about it */
/* else warn here, and make sure log.h does not warn */

#ifndef YLOG_H 

#ifndef YAZ_USE_OLD_LOG
#warning "yaz-util.h is deprecated - include the files directly, and use ylog.h"
/* If you get tired of this message, configure your program like this */
/* CFLAGS="-Wall -g -D YAZ_USE_OLD_LOG" ./configure   */
#define YAZ_USE_OLD_LOG
/* do not complain of the old log.h */
#endif

#include <yaz/log.h>  
#endif

#include <yaz/tpath.h>
#include <yaz/options.h>
#include <yaz/wrbuf.h>
#include <yaz/nmem.h>
#include <yaz/readconf.h>
#include <yaz/marcdisp.h>
#include <yaz/yaz-iconv.h>

#endif
