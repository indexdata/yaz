/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.8 2001-10-28 23:10:03 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.8.1"
#include <yaz/yaz-date.h>

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

