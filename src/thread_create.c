/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file thread_create.c
 * \brief Implements thread creation wrappers
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
#include <yaz/log.h>
#include <yaz/thread_create.h>

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif

struct yaz_thread {
#if YAZ_POSIX_THREADS
    pthread_t id;
#else
    void *return_data;
#ifdef WIN32
    HANDLE id;
#endif
#endif
};

yaz_thread_t yaz_thread_create(void *(*start_routine)(void *p), void *arg)
{
    yaz_thread_t t = xmalloc(sizeof(*t));
#if YAZ_POSIX_THREADS
    int r = pthread_create(&t->id, 0, start_routine, arg);
    if (r)
    {
        xfree(t);
        t = 0;
    }
#else
    t->return_data = start_routine(arg);
#endif
    return t;
}

void yaz_thread_join(yaz_thread_t *tp, void **value_ptr)
{
    if (*tp)
    {
#ifdef YAZ_POSIX_THREADS
        pthread_join((*tp)->id, value_ptr);
#else
        if (value_ptr)
            *value_ptr = (*tp)->return_data;
#endif
        xfree(*tp);
        *tp = 0;
    }
}

void yaz_thread_detach(yaz_thread_t *tp)
{
    if (*tp)
    {
#ifdef YAZ_POSIX_THREADS
        pthread_detach((*tp)->id);
#endif
        xfree(*tp);
        *tp = 0;
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

