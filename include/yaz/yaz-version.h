/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.13 2002-02-11 23:25:26 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.8.6"
#include <yaz/yaz-date.h>

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

