/*
 * Copyright (c) 1995-2004, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: yaz-version.h,v 1.40 2004-03-01 17:33:02 adam Exp $
 */

/*
 * Current software version.
 */
#ifndef YAZ_VERSION

#include <yaz/yconfig.h>

#define YAZ_VERSION "2.0.15"
#define YAZ_VERSIONL 0x02000F

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

