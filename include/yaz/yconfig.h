/*
 * Copyright (c) 1995-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: yconfig.h,v 1.6 2003-02-18 14:28:52 adam Exp $
 */

#ifndef YCONFIG_H
#define YCONFIG_H

#ifndef YAZ_EXPORT
# ifdef WIN32
#  define YAZ_EXPORT __declspec(dllexport)
# else
#  define YAZ_EXPORT
# endif
#endif

#ifndef WIN32
# ifndef O_BINARY
#  define O_BINARY 0
# endif
#endif

#ifdef __cplusplus
#define YAZ_BEGIN_CDECL extern "C" {
#define YAZ_END_CDECL }
#else
#define YAZ_BEGIN_CDECL 
#define YAZ_END_CDECL 
#endif

#endif
