/*
 * Copyright (c) 1995-1997, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: yaz-util.h,v $
 * Revision 1.6  1997-10-27 13:52:46  adam
 * Header yaz-util includes all YAZ utility header files.
 *
 * Revision 1.5  1997/09/04 07:58:36  adam
 * Added prototype for atoi_n.
 *
 * Revision 1.4  1997/09/01 08:49:54  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.3  1997/05/14 06:53:54  adam
 * C++ support.
 *
 * Revision 1.2  1996/02/20 17:58:09  adam
 * Added const to yaz_matchstr.
 *
 * Revision 1.1  1996/02/20  16:32:49  quinn
 * Created util file.
 */

#ifndef YAZ_UTIL_H
#define YAZ_UTIL_H

#include <yconfig.h>

#include <xmalloc.h>
#include <log.h>
#include <tpath.h>
#include <options.h>
#include <wrbuf.h>
#include <nmem.h>
#include <readconf.h>

#ifdef __cplusplus
extern "C" {
#endif

YAZ_EXPORT int yaz_matchstr(const char *s1, const char *s2);
YAZ_EXPORT int atoi_n (const char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
