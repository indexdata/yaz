/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: eventl.c,v $
 * Revision 1.1  1995-03-10 18:22:44  quinn
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
    return new;
}

void iochan_destroy(IOCHAN i)
{
    i->destroyed = 1;
}

int event_loop()
{
    do
    {
    	IOCHAN p, nextp;
	fd_set in, out, except;
	int res, max;

	FD_ZERO(&in);
	FD_ZERO(&out);
	FD_ZERO(&except);
    	for (p = iochans; p; p = p->next)
    	{
	    if (p->flags & EVENT_INPUT)
	    	FD_SET(p->fd, &in);
	    if (p->flags & EVENT_OUTPUT)
	    	FD_SET(p->fd, &out);
	    if (p->flags & EVENT_EXCEPT)
	    	FD_SET(p->fd, &except);
	    if (p->fd > max)
	    	max = p->fd;
	}
	if ((res = select(max + 1, &in, &out, &except, 0)) < 0)
	{
	    if (errno == EINTR)
		continue;
	    return 1;
	}
	if (!res)
	    continue;
    	for (p = iochans; p; p = nextp)
    	{
	    nextp = p->next;
	    if (FD_ISSET(p->fd, &in))
	    	(*p->fun)(p, EVENT_INPUT);
	    if (!p->destroyed && FD_ISSET(p->fd, &in))
	    	(*p->fun)(p, EVENT_OUTPUT);
	    if (!p->destroyed && FD_ISSET(p->fd, &except))
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
