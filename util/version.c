/*
 * Copyright (c) 1995-2003, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: version.c,v 1.2 2003-04-14 16:57:58 adam Exp $
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
