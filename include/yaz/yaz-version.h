/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.15 2002-05-13 18:34:53 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.8.7"
#define YAZ_VERSIONL 0x010807

#define YAZ_DATE 1

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

