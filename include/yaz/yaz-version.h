/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.19 2002-09-05 13:36:53 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.9"
#define YAZ_VERSIONL 0x010900

#define YAZ_DATE 1

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

