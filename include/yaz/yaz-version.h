/*
 * Copyright (c) 1995-2005, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: yaz-version.h,v 1.58 2005-01-11 12:13:18 adam Exp $
 */

/**
 * \file yaz-version.h
 * \brief Defines YAZ version.
 */
#ifndef YAZ_VERSION

#include <yaz/yconfig.h>

#define YAZ_VERSION "2.0.32"
#define YAZ_VERSIONL 0x020020

#define YAZ_DATE 1

#ifdef WIN32
#ifdef NDEBUG
#define YAZ_OS "WIN32 Release"
#else
#define YAZ_OS "WIN32 Debug"
#endif
#endif

YAZ_BEGIN_CDECL

YAZ_EXPORT unsigned long yaz_version(char *version_str, char *sys_str);

YAZ_END_CDECL

#endif

