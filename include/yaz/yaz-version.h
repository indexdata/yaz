/*
 * Current software version.
 *
 * $Id: yaz-version.h,v 1.21 2002-11-26 13:42:35 adam Exp $
 */
#ifndef YAZ_VERSION

#define YAZ_VERSION "1.9.2"
#define YAZ_VERSIONL 0x010902

#define YAZ_DATE 1

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

#endif

