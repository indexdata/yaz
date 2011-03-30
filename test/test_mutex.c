/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include <yaz/mutex.h>
#include <yaz/thread_create.h>
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif

#include <yaz/test.h>
#include <yaz/log.h>
#include <yaz/gettimeofday.h>

static void tst_mutex(void)
{
    YAZ_MUTEX p = 0;

    yaz_mutex_create(&p);
    YAZ_CHECK(p);
    yaz_mutex_enter(p);
    yaz_mutex_leave(p);
    yaz_mutex_destroy(&p);
    YAZ_CHECK(p == 0);

    yaz_mutex_create(&p);
    YAZ_CHECK(p);
    yaz_mutex_set_name(p, YLOG_LOG, "mymutex");
    yaz_mutex_enter(p);
    yaz_mutex_leave(p);
    yaz_mutex_destroy(&p);
    YAZ_CHECK(p == 0);

    yaz_mutex_destroy(&p); /* OK to "destroy" NULL handle */
}

static void tst_cond(void)
{
    YAZ_MUTEX p = 0;
    YAZ_COND c;
    struct timeval abstime;
    int r;

    yaz_mutex_create(&p);
    YAZ_CHECK(p);
    if (!p)
        return;

    yaz_cond_create(&c);
    if (c)
    {
        r = yaz_gettimeofday(&abstime);
        YAZ_CHECK_EQ(r, 0);
        
        abstime.tv_sec += 1; /* wait 1 second */
        
        r = yaz_cond_wait(c, p, &abstime);
        YAZ_CHECK(r != 0);

    }
    yaz_cond_destroy(&c);
    YAZ_CHECK(c == 0);
    yaz_mutex_destroy(&p);
    YAZ_CHECK(p == 0);
}

static void *my_handler(void *arg)
{
    int *mydata = (int*) arg;
    (*mydata)++;
    return mydata;
}

static void tst_create_thread(void)
{
    void *return_data;
    int mydata = 42;
    yaz_thread_t t[2];

    t[0] = yaz_thread_create(my_handler, &mydata);
    YAZ_CHECK(t[0]);
    t[1] = yaz_thread_create(my_handler, &mydata);
    YAZ_CHECK(t[1]);
    
    return_data = 0;
    yaz_thread_join(&t[0], &return_data);
    YAZ_CHECK(!t[0]);
    YAZ_CHECK(return_data == &mydata);

    return_data = 0;
    yaz_thread_join(&t[1], &return_data);
    YAZ_CHECK(!t[1]);
    YAZ_CHECK(return_data == &mydata);
    
    YAZ_CHECK_EQ(mydata, 44);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_mutex();
    tst_cond();
    tst_create_thread();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

