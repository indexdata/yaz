/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.14 2002-03-25 15:12:25 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.8.7"
#include <yaz/yaz-date.h>

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

