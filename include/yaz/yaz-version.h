/*
 * Copyright (c) 1995-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: yaz-version.h,v 1.25 2003-03-25 09:57:26 adam Exp $
 */

/*
 * Current software version.
 */
#ifndef YAZ_VERSION

#include <yaz/yconfig.h>

#define YAZ_VERSION "2.0.1"
#define YAZ_VERSIONL 0x020001

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

