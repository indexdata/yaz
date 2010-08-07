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
#include <yaz/gettimeofday.h>
#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#endif
#include <time.h>

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#include "mutex-p.h"

void yaz_mutex_create_attr(YAZ_MUTEX *p, int flags) {
    if (!*p)
    {
        *p = (YAZ_MUTEX) malloc(sizeof(**p));
#ifdef WIN32
        InitializeCriticalSection(&(*p)->handle);
#elif YAZ_POSIX_THREADS
        (*p)->attr = malloc(sizeof( (*p)->attr));
        pthread_mutexattr_init((*p)->attr);
        pthread_mutexattr_settype((*p)->attr, flags);
        pthread_mutex_init(&(*p)->handle, (*p)->attr);
#endif
        (*p)->name = 0;
        (*p)->log_level = 0;
    }
}

void yaz_mutex_create(YAZ_MUTEX *p) {
    yaz_mutex_create_attr(p, 0);
}

void yaz_mutex_set_name(YAZ_MUTEX p, int log_level, const char *name)
{
    if (p->name)
        free(p->name);
    p->name = 0;
    p->log_level = 0;
    if (name)
    {
        p->name = strdup(name);
        p->log_level = log_level;
    }
}

void yaz_mutex_enter(YAZ_MUTEX p)
{
    if (p)
    {
#ifdef WIN32
        EnterCriticalSection(&p->handle);
#elif YAZ_POSIX_THREADS
        int r = 1; /* signal : not locked (yet) */
        
        if (p->log_level)
        {   /* debugging */
            r = pthread_mutex_trylock(&p->handle);
            if (r)
            {
#if HAVE_SYS_TIME_H
                long long d;
                struct timeval tv1, tv2;
                gettimeofday(&tv1, 0);
#endif
                yaz_log(p->log_level,
                        "yaz_mutex_enter: %p tid=%p name=%s waiting",
                        p, (void *) pthread_self(), p->name);
#if HAVE_SYS_TIME_H
                r = pthread_mutex_lock(&p->handle);
                gettimeofday(&tv2, 0);
                d = 1000000LL * ((long long) tv2.tv_sec - tv1.tv_sec) +
                    tv2.tv_usec - tv1.tv_usec;
                yaz_log(p->log_level, "yaz_mutex_enter: %p tid=%p name=%s "
                        "lock delay %lld",
                        p, (void *) pthread_self(), p->name, d);
#endif
            }
            else
            {
                yaz_log(p->log_level, "yaz_mutex_enter: %p tid=%p name=%s lock",
                        p, (void *) pthread_self(), p->name);
            }
        }
        if (r)
        {
            r = pthread_mutex_lock(&p->handle);
            if (p->log_level)
            {
                yaz_log(p->log_level, "yaz_mutex_enter: %p tid=%p name=%s lock",
                        p, (void *) pthread_self(), p->name);
            }
        }
#endif
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
        if (p->log_level)
        {
            yaz_log(p->log_level,
                    "yaz_mutex_leave: %p tid=%p name=%s unlock",
                    p, (void *) pthread_self(), p->name);
        }
#endif
    }
}

void yaz_mutex_destroy(YAZ_MUTEX *p)
{
    if (*p)
    {
#ifdef WIN32
        DeleteCriticalSection(&(*p)->handle);
#elif YAZ_POSIX_THREADS
        pthread_mutexattr_destroy((*p)->attr);
        free((*p)->attr);
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

