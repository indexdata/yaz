/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.11 2001-11-19 20:43:39 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.8.4"
#include <yaz/yaz-date.h>

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

