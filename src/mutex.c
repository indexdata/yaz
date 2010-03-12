/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
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

struct yaz_mutex {
#ifdef WIN32
    CRITICAL_SECTION handle;
#elif YAZ_POSIX_THREADS
    pthread_mutex_t handle;
#endif
    char *name;
};

void yaz_mutex_create(YAZ_MUTEX *p)
{
    if (!*p)
    {
        *p = (YAZ_MUTEX) malloc(sizeof(**p));
#ifdef WIN32
        InitializeCriticalSection(&(*p)->handle);
#elif YAZ_POSIX_THREADS
        pthread_mutex_init(&(*p)->handle, 0);
#endif
        (*p)->name = 0;
    }
}

void yaz_mutex_set_name(YAZ_MUTEX p, const char *name)
{
    if (p->name)
        free(p->name);
    p->name = 0;
    if (name)
        p->name = strdup(name);
}

void yaz_mutex_enter(YAZ_MUTEX p)
{
    if (p)
    {
#ifdef WIN32
        EnterCriticalSection(&p->handle);
#elif YAZ_POSIX_THREADS
        int r = 1;
        if (p->name)
        {   /* debugging */
            r = pthread_mutex_trylock(&p->handle);
            if (r)
            {
                yaz_log(YLOG_WARN|YLOG_ERRNO,
                        "yaz_mutex_enter: %p name=%s waiting", p, p->name);
            }
        }
        if (r && pthread_mutex_lock(&p->handle))
        {
            yaz_log(YLOG_WARN|YLOG_ERRNO, "yaz_mutex_enter: %p error", p);
        }
#endif
        if (p->name)
        {
            yaz_log(YLOG_LOG, "yaz_mutex_enter: %p name=%s lock", p, p->name);
        }
    }
}

void yaz_mutex_leave(YAZ_MUTEX p)
{
    if (p)
    {
#ifdef WIN32
        LeaveCriticalSection(&p->handle);
#elif YAZ_POSIX_THREADS
        pthread_mutex_unlock(&p->handle);
#endif
        if (p->name)
        {
            yaz_log(YLOG_LOG, "yaz_mutex_leave: %p name=%s unlock", p, p->name);
        }
    }
}

void yaz_mutex_destroy(YAZ_MUTEX *p)
{
    if (*p)
    {
#ifdef WIN32
        DeleteCriticalSection(&(*p)->handle);
#elif YAZ_POSIX_THREADS
        pthread_mutex_destroy(&(*p)->handle);
#endif
        if ((*p)->name)
            free((*p)->name);
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

