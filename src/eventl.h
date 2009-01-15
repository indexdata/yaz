/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file eventl.h
 * \brief Definitions for event loop handling for GFS.
 *
 * This "private" header defines various functions for the
 * main event loop in GFS.
 */

#ifndef EVENTL_H
#define EVENTL_H

#include <time.h>

struct iochan;

typedef void (*IOC_CALLBACK)(struct iochan *i, int event);

typedef struct iochan
{
    int fd;
    int flags;
#define EVENT_INPUT     0x01
#define EVENT_OUTPUT    0x02
#define EVENT_EXCEPT    0x04
#define EVENT_TIMEOUT   0x08
int force_event;
    IOC_CALLBACK fun;
    void *data;
    int destroyed;
    time_t last_event;
    time_t max_idle;
    
    struct iochan *next;
    int chan_id; /* listening port (0 if none ) */
} *IOCHAN;

#define iochan_destroy(i) (void)((i)->destroyed = 1)
#define iochan_getfd(i) ((i)->fd)
#define iochan_setfd(i, f) ((i)->fd = (f))
#define iochan_getdata(i) ((i)->data)
#define iochan_setdata(i, d) ((i)->data = d)
#define iochan_getflags(i) ((i)->flags)
#define iochan_setflags(i, d) ((i)->flags = d)
#define iochan_setflag(i, d) ((i)->flags |= d)
#define iochan_clearflag(i, d) ((i)->flags &= ~(d))
#define iochan_getflag(i, d) ((i)->flags & d ? 1 : 0)
#define iochan_getfun(i) ((i)->fun)
#define iochan_setfun(i, d) ((i)->fun = d)
#define iochan_setevent(i, e) ((i)->force_event = (e))
#define iochan_getnext(i) ((i)->next)
#define iochan_settimeout(i, t) ((i)->max_idle = (t), (i)->last_event = time(0))

IOCHAN iochan_create(int fd, IOC_CALLBACK cb, int flags, int port);
int iochan_is_alive(IOCHAN chan);
int iochan_event_loop(IOCHAN *iochans);
void statserv_remove (IOCHAN pIOChannel);
#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

