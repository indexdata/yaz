/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file errno.c
 * \brief errno utilities
 *
 * This file unlike other files in YAZ core is thread-aware, due to
 * the use errno.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <yaz/errno.h>

#ifdef WIN32
#include <windows.h>
#endif

int yaz_errno(void)
{
    return errno;
}

void yaz_set_errno(int v)
{
    errno = v;
}

void yaz_strerror(char *buf, int max)
{
#ifdef WIN32
    DWORD err;
#endif
    char *cp;
#ifdef WIN32
    err = GetLastError();
    if (err)
    {
        FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                err,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default lang */
                (LPTSTR) buf,
                max-1,
                NULL);
    }
    else
        *buf = '\0';
#else
/* UNIX */
#if HAVE_STRERROR_R
    *buf = '\0';
    strerror_r(errno, buf, max);
    /* if buffer is unset - use strerror anyway (GLIBC bug) */
    if (*buf == '\0')
        strcpy(buf, strerror(yaz_errno()));
#else
    strcpy(buf, strerror(yaz_errno()));
#endif
/* UNIX */
#endif
    if ((cp = strrchr(buf, '\n')))
        *cp = '\0';
    if ((cp = strrchr(buf, '\r')))
        *cp = '\0';
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

