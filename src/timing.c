/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: timing.c,v 1.1 2007-01-03 13:46:18 adam Exp $
 */

/**
 * \file timing.c
 * \brief Timing Utilities
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>

#if HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>

#include <yaz/xmalloc.h>
#include <yaz/timing.h>

struct yaz_timing {
#if HAVE_SYS_TIMES_H
    struct tms tms1, tms2;
#endif
#if HAVE_SYS_TIME_H
    struct timeval start_time, end_time;
#endif
    double real_sec, user_sec, sys_sec;
};

yaz_timing_t yaz_timing_create(void)
{
    yaz_timing_t t = xmalloc(sizeof(*t));
    yaz_timing_start(t);
    return t;
}

void yaz_timing_start(yaz_timing_t t)
{
#if HAVE_SYS_TIMES_H
    times(&t->tms1);
#endif
#if HAVE_SYS_TIME_H
    gettimeofday(&t->start_time, 0);
#endif
    t->real_sec = -1.0;
    t->user_sec = -1.0;
    t->sys_sec = -1.0;
}

void yaz_timing_stop(yaz_timing_t t)
{
    t->real_sec = 0.0;
    t->user_sec = 0.0;
    t->sys_sec = 0.0;
#if HAVE_SYS_TIMES_H
    times(&t->tms2);
    
    t->user_sec = (double) (t->tms2.tms_utime - t->tms1.tms_utime)/100;
    t->sys_sec = (double) (t->tms2.tms_stime - t->tms1.tms_stime)/100;
#endif
#if HAVE_SYS_TIME_H
    gettimeofday(&t->end_time, 0);
    t->real_sec = ((t->end_time.tv_sec - t->start_time.tv_sec) * 1000000.0 +
                   t->end_time.tv_usec - t->start_time.tv_usec) / 1000000;
    
#endif
}

double yaz_timing_get_real(yaz_timing_t t)
{
    return t->real_sec;
}

double yaz_timing_get_user(yaz_timing_t t)
{
    return t->user_sec;
}

double yaz_timing_get_sys(yaz_timing_t t)
{
    return t->sys_sec;
}

void yaz_timing_destroy(yaz_timing_t *tp)
{
    if (*tp)
    {
        xfree(*tp);
        *tp = 0;
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

