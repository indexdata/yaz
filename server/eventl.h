/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: eventl.h,v 1.12 2003-02-12 15:06:43 adam Exp $
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
#define EVENT_WORK      0x10
int force_event;
    IOC_CALLBACK fun;
    void *data;
    int destroyed;
    time_t last_event;
    time_t max_idle;
    
    struct iochan *next;
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

IOCHAN iochan_create(int fd, IOC_CALLBACK cb, int flags);
int event_loop(IOCHAN *iochans);
void statserv_remove (IOCHAN pIOChannel);
#endif
