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
 * $Id: unix.h,v 1.3 2004-10-15 00:18:59 adam Exp $
 * UNIX socket COMSTACK. By Morten Bøgeskov.
 */
/**
 * \file unix.h
 * \brief Header for UNIX domain socket COMSTACK
 */

#ifndef UNIX_H
#define UNIX_H

#ifndef WIN32

#include <yaz/comstack.h>
#include <yaz/oid.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT int completeWAIS(const unsigned char *buf, int len);
YAZ_EXPORT struct sockaddr_un *unix_strtoaddr(const char *str);
YAZ_EXPORT COMSTACK unix_type(int s, int blocking, int protocol, void *vp);

YAZ_END_CDECL

#endif

#endif
