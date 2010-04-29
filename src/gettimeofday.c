/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file gettimeofday.c
 * \brief Implements wrapper for gettimeofday
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <yaz/gettimeofday.h>

int yaz_gettimeofday(struct timeval *tval)
{
#ifdef WIN32
    struct _timeb timeb;
    _ftime(&timeb);
    tval->tv_sec = timeb.time;
    tval->tv_usec = timeb.millitm * 1000;
    return 0;
#else
    return gettimeofday(tval, 0);
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

