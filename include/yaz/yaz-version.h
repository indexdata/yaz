/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.18 2002-08-17 07:56:59 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.8.9"
#define YAZ_VERSIONL 0x010809

#define YAZ_DATE 1

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

