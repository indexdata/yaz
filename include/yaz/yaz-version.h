/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.10 2001-11-18 21:14:23 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.8.3"
#include <yaz/yaz-date.h>

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

