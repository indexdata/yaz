/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.22 2003-02-10 08:58:40 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.9.3"
#define YAZ_VERSIONL 0x010903

#define YAZ_DATE 1

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

