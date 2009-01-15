/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file mutex.c
 * \brief Implements MUTEX functions
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include <yaz/log.h>
#include <yaz/mutex.h>

#ifdef WIN32
#include <windows.h>
#endif

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#if YAZ_GNU_THREADS
#include <pth.h>
#endif

#ifdef WIN32
struct yaz_mutex {
    CRITICAL_SECTION m_handle;
};
#elif YAZ_POSIX_THREADS
struct yaz_mutex {
    pthread_mutex_t m_handle;
};
#elif YAZ_GNU_THREADS
struct yaz_mutex {
    pth_mutex_t m_handle;
};
#else
struct yaz_mutex {
    int dummy;
};
#endif

YAZ_EXPORT void yaz_mutex_create(YAZ_MUTEX *p)
{
    if (!*p)
    {
        *p = (YAZ_MUTEX) malloc(sizeof(**p));
#ifdef WIN32
        InitializeCriticalSection(&(*p)->m_handle);
#elif YAZ_POSIX_THREADS
        pthread_mutex_init(&(*p)->m_handle, 0);
#elif YAZ_GNU_THREADS
        pth_mutex_init(&(*p)->m_handle);
#endif
    }
}

YAZ_EXPORT void yaz_mutex_enter(YAZ_MUTEX p)
{
    if (p)
    {
#ifdef WIN32
        EnterCriticalSection(&p->m_handle);
#elif YAZ_POSIX_THREADS
        pthread_mutex_lock(&p->m_handle);
#endif
    }
}

YAZ_EXPORT void yaz_mutex_leave(YAZ_MUTEX p)
{
    if (p)
    {
#ifdef WIN32
        LeaveCriticalSection(&p->m_handle);
#elif YAZ_POSIX_THREADS
        pthread_mutex_unlock(&p->m_handle);
#endif
    }
}

YAZ_EXPORT void yaz_mutex_destroy(YAZ_MUTEX *p)
{
    if (*p)
    {
#ifdef WIN32
        DeleteCriticalSection(&(*p)->m_handle);
#endif
        free(*p);
        *p = 0;
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

