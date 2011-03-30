/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
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
#include <process.h>
#endif

struct yaz_thread {
#if YAZ_POSIX_THREADS
    pthread_t id;
#else
#ifdef WIN32
    HANDLE handle;
    void *(*routine)(void *p);
#endif
    void *data;
#endif
};

#ifdef WIN32
unsigned int __stdcall win32_routine(void *p)
{
    yaz_thread_t t = (yaz_thread_t) p;
    void *userdata = t->data;
    t->data = t->routine(userdata);
    _endthreadex(0);
    return 0;
}
#endif

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
#ifdef WIN32
    /* we create a wrapper on windows and pass yaz_thread struct to that */
    unsigned threadID;
    uintptr_t ex_ret;
    t->data = arg; /* use data for both input and output */
    t->routine = start_routine;
    ex_ret = _beginthreadex(NULL, 0, win32_routine, t, 0, &threadID);
    if (ex_ret == -1L)
    {
        xfree(t);
        t = 0;
    }
    t->handle = (HANDLE) ex_ret;
#else
    t->data = start_routine(arg);
#endif
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
#ifdef WIN32
        WaitForSingleObject((*tp)->handle, INFINITE);
        CloseHandle((*tp)->handle);
#endif
        if (value_ptr)
            *value_ptr = (*tp)->data;
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
#else
#ifdef WIN32
        CloseHandle((*tp)->handle);
#endif
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

