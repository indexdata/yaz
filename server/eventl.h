/*
 * Copyright (c) 1995, Index Data I/S 
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: eventl.h,v $
 * Revision 1.7  1995-06-16 10:31:34  quinn
 * Added session timeout.
 *
 * Revision 1.6  1995/05/16  08:51:02  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.5  1995/05/15  11:56:37  quinn
 * Asynchronous facilities. Restructuring of seshigh code.
 *
 * Revision 1.4  1995/03/27  08:34:23  quinn
 * Added dynamic server functionality.
 * Released bindings to session.c (is now redundant)
 *
 * Revision 1.3  1995/03/15  08:37:42  quinn
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
#define iochan_settimeout(i, t) ((i)->max_idle = (t))

IOCHAN iochan_getchan(void);
IOCHAN iochan_create(int fd, IOC_CALLBACK cb, int flags);
int event_loop();

#endif
