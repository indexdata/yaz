/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2008 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file version.c
 * \brief Implements YAZ version utilities.
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
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

