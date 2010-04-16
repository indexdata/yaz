/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file thraedid.c
 * \brief Returns printable thread ID
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#include <yaz/thread_id.h>

void yaz_thread_id_cstr(char *buf, size_t buf_max)
{
#ifdef WIN32
    *buf = '\0';
#elif YAZ_POSIX_THREADS
    pthread_t t = pthread_self();
    size_t i;
    *buf = '\0';
    for (i = 0; i < sizeof(t); i++)
    {
        if (strlen(buf) >= buf_max-2)
            break;
        sprintf(buf + strlen(buf), "%02x", ((const unsigned char *) &t)[i]);
    }
#else
    *buf = '\0';
#endif
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

