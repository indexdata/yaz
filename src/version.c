/*
 * Copyright (c) 1995-2004, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: version.c,v 1.2 2004-10-03 22:34:07 adam Exp $
 */

/**
 * \file version.c
 * \brief Implements YAZ version utilities.
 */

/**
 * \mainpage The YAZ Toolkit.
 */
 
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/yaz-version.h>

unsigned long yaz_version(char *version_str, char *sys_str)
{
    if (version_str)
        strcpy(version_str, YAZ_VERSION);
    if (sys_str)
        strcpy(sys_str, "");
    return YAZ_VERSIONL;
}
