/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: eventl.h,v $
 * Revision 1.3  1995-03-15 08:37:42  quinn
 * Now we're pretty much set for nonblocking I/O.
 *
 * Revision 1.2  1995/03/14  10:28:00  quinn
 * More work on demo server.
 *
 * Revision 1.1  1995/03/10  18:22:45  quinn
 * The rudiments of an asynchronous server.
 *
 */

#ifndef EVENTL_H
#define EVENTL_H

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
    
    struct iochan *next;
} *IOCHAN;

#define iochan_destroy(i) (void)((i)->destroyed = 1)
#define iochan_getfd(i)
#define iochan_getdata(i) ((i)->data)
#define iochan_setdata(i, d) ((i)->data = d)
#define iochan_getflags(i) ((i)->flags)
#define iochan_setflags(i, d) ((i)->flags = d)
#define iochan_setflag(i, d) ((i)->flags |= d)
#define iochan_getflag(i, d) ((i)->flags & d ? 1 : 0)
#define iochan_getfun(i) ((i)->fun)
#define iochan_setfun(i, d) ((i)->fun = d)
#define iochan_setevent(i, e) ((i)->force_event = (e))

IOCHAN iochan_create(int fd, IOC_CALLBACK cb, int flags);
int event_loop();

#endif
