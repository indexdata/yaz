/*
 * Copyright (c) 1995-2004, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: yaz-version.h,v 1.38 2004-02-23 09:30:17 adam Exp $
 */

/*
 * Current software version.
 */
#ifndef YAZ_VERSION

#include <yaz/yconfig.h>

#define YAZ_VERSION "2.0.13"
#define YAZ_VERSIONL 0x02000D

#define YAZ_DATE 1

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

YAZ_BEGIN_CDECL

YAZ_EXPORT unsigned long yaz_version(char *version_str, char *sys_str);

YAZ_END_CDECL

#endif

