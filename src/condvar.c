/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file condvar.c
 * \brief Wraps condition variables
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

struct yaz_cond {
#ifdef WIN32
    CONDITION_VARIABLE cond;
#elif YAZ_POSIX_THREADS
    pthread_cond_t cond;
#endif
};

void yaz_cond_create(YAZ_COND *p)
{
#ifdef WIN32
    *p = (YAZ_COND) malloc(sizeof(**p));
    InitializeConditionVariable(&(*p)->cond);
#elif YAZ_POSIX_THREADS
    *p = (YAZ_COND) malloc(sizeof(**p));
    pthread_cond_init(&(*p)->cond, 0);
#else
    *p = 0;
#endif
}

void yaz_cond_destroy(YAZ_COND *p)
{
    if (*p)
    {
#ifdef WIN32
#elif YAZ_POSIX_THREADS
        pthread_cond_destroy(&(*p)->cond);
#endif
        free(*p);
        *p = 0;
    }
}

int yaz_cond_wait(YAZ_COND p, YAZ_MUTEX m, const struct timeval *abstime)
{
#ifdef WIN32
    BOOL v;
    if (abstime)
    {
        struct timeval tval_now;
        int sec, msec;

        yaz_gettimeofday(&tval_now);

        sec = abstime->tv_sec - tval_now.tv_sec;
        msec = (abstime->tv_usec - tval_now.tv_usec) / 1000;
        v = SleepConditionVariableCS(&p->cond, &m->handle, sec*1000 + msec);
    }
    else
        v = SleepConditionVariableCS(&p->cond, &m->handle, INFINITE);
    return v ? 0 : -1;
#elif YAZ_POSIX_THREADS
    if (abstime)
    {
        struct timespec s;
        s.tv_sec = abstime->tv_sec;
        s.tv_nsec = abstime->tv_usec * 1000;
        return pthread_cond_timedwait(&p->cond, &m->handle, &s);
    }
    else
        return pthread_cond_wait(&p->cond, &m->handle);
#else
    return -1;
#endif
}

int yaz_cond_signal(YAZ_COND p)
{
#ifdef WIN32
    WakeConditionVariable(&p->cond);
    return 0;
#elif YAZ_POSIX_THREADS
    return pthread_cond_signal(&p->cond);
#else
    return -1;
#endif
}

int yaz_cond_broadcast(YAZ_COND p)
{
#ifdef WIN32
    WakeAllConditionVariable(&p->cond);
    return 0;
#elif YAZ_POSIX_THREADS
    return pthread_cond_broadcast(&p->cond);
#else
    return -1;
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

