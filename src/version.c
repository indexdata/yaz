/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: version.c,v 1.3 2005-01-15 19:47:14 adam Exp $
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
