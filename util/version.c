/*
 * Copyright (c) 1995-2003, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: version.c,v 1.1 2003-02-18 14:28:53 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/yaz-version.h>

unsigned long yaz_version(char *version_str, char *sys_str)
{
    if (version_str)
        strcpy(version_str, YAZ_VERSION);
    if (sys_str)
        strcpy(sys_str, "");
    return YAZ_VERSIONL;
}
