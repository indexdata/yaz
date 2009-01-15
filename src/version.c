/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
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

unsigned long yaz_version(char *version_str, char *sha1_str)
{
    if (version_str)
        strcpy(version_str, YAZ_VERSION);
    if (sha1_str)
        strcpy(sha1_str, YAZ_VERSION_SHA1);
    return YAZ_VERSIONL;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

