/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.9 2001-11-12 11:27:42 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.8.2"
#include <yaz/yaz-date.h>

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

