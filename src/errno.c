/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
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

#include <yaz/errno.h>
#include <yaz/snprintf.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

void yaz_strerror(char *buf, size_t bufsz)
{
#ifdef WIN32
    DWORD err;
#else
    int error = errno;
#endif
    char *cp;
    if (!bufsz)
        return;
    *buf = '\0';
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
                bufsz-1,
                NULL);
    }
    else
        *buf = '\0';
#else
/* UNIX */
#if HAVE_STRERROR_R
#if STRERROR_R_CHAR_P
    /* GNU strerror_r may return static storage instead of filling buf. */
    const char *message = strerror_r(error, buf, bufsz);
    if (message && message != buf)
        yaz_snprintf(buf, bufsz, "%s", message);
#else
    strerror_r(error, buf, bufsz);
#endif
    if (*buf == '\0')
        yaz_snprintf(buf, bufsz, "%s", strerror(error));
#else
    yaz_snprintf(buf, bufsz, "%s", strerror(error));
#endif
/* UNIX */
#endif
    buf[bufsz - 1] = '\0';
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

