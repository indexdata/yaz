/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: eventl.c,v $
 * Revision 1.5  1995-03-15 08:37:41  quinn
 * Now we're pretty much set for nonblocking I/O.
 *
 * Revision 1.4  1995/03/14  16:59:48  quinn
 * Bug-fixes
 *
 * Revision 1.3  1995/03/14  11:30:14  quinn
 * Works better now.
 *
 * Revision 1.2  1995/03/14  10:27:59  quinn
 * More work on demo server.
 *
 * Revision 1.1  1995/03/10  18:22:44  quinn
 * The rudiments of an asynchronous server.
 *
 */

#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <eventl.h>

IOCHAN iochans = 0;

IOCHAN iochan_create(int fd, IOC_CALLBACK cb, int flags)
{
    IOCHAN new;

    if (!(new = malloc(sizeof(*new))))
    	return 0;
    new->destroyed = 0;
    new->fd = fd;
    new->flags = flags;
    new->fun = cb;
    new->next = iochans;
    new->force_event = 0;
    iochans = new;
    return new;
}

int event_loop()
{
    do
    {
    	IOCHAN p, nextp;
	fd_set in, out, except;
	int res, max;
	static struct timeval nullto = {0, 0};
	struct timeval *timeout;

	FD_ZERO(&in);
	FD_ZERO(&out);
	FD_ZERO(&except);
	timeout = 0; /* hang on select */
	max = 0;
    	for (p = iochans; p; p = p->next)
    	{
	    if (p->force_event)
	    	timeout = &nullto;
	    if (p->flags & EVENT_INPUT)
	    	FD_SET(p->fd, &in);
	    if (p->flags & EVENT_OUTPUT)
	    	FD_SET(p->fd, &out);
	    if (p->flags & EVENT_EXCEPT)
	    	FD_SET(p->fd, &except);
	    if (p->fd > max)
	    	max = p->fd;
	}
	if ((res = select(max + 1, &in, &out, &except, timeout)) < 0)
	{
	    if (errno == EINTR)
		continue;
	    return 1;
	}
    	for (p = iochans; p; p = nextp)
    	{
	    int force_event = p->force_event;

	    p->force_event = 0;
	    nextp = p->next;
	    if (FD_ISSET(p->fd, &in) || force_event == EVENT_INPUT)
	    	(*p->fun)(p, EVENT_INPUT);
	    if (!p->destroyed && (FD_ISSET(p->fd, &out) ||
	   	 force_event == EVENT_OUTPUT))
	    	(*p->fun)(p, EVENT_OUTPUT);
	    if (!p->destroyed && (FD_ISSET(p->fd, &except) ||
	    	force_event == EVENT_EXCEPT))
	    	(*p->fun)(p, EVENT_EXCEPT);
	    if (p->destroyed)
	    {
	    	IOCHAN tmp = p, pr;

	    	if (p == iochans)
		    iochans = p->next;
		else
		{
		    for (pr = iochans; pr; pr = pr->next)
		    	if (pr->next == p)
			    break;
		    assert(pr);
		    pr->next = p->next;
		}
		if (nextp == p)
		    nextp = p->next;
		free(tmp);
	    }
	}
    }
    while (iochans);
    return 0;
}
