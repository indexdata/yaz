/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.20 2002-09-10 18:43:02 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.9.1"
#define YAZ_VERSIONL 0x010901

#define YAZ_DATE 1

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

