/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: eventl.c,v $
 * Revision 1.19  1995-12-05 11:17:30  quinn
 * Moved some paranthesises around. Sigh.
 *
 * Revision 1.18  1995/11/13  09:27:41  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.17  1995/11/07  12:37:44  quinn
 * Added support for forcing TIMEOUT event.
 *
 * Revision 1.16  1995/11/01  13:54:56  quinn
 * Minor adjustments
 *
 * Revision 1.15  1995/09/15  14:44:15  quinn
 * *** empty log message ***
 *
 * Revision 1.14  1995/08/29  14:44:50  quinn
 * Reset timeouts.
 *
 * Revision 1.13  1995/08/29  11:17:56  quinn
 * Added code to receive close
 *
 * Revision 1.12  1995/08/29  10:41:18  quinn
 * Small.
 *
 * Revision 1.11  1995/06/19  12:39:09  quinn
 * Fixed bug in timeout code. Added BER dumper.
 *
 * Revision 1.10  1995/06/16  10:31:33  quinn
 * Added session timeout.
 *
 * Revision 1.9  1995/06/05  10:53:31  quinn
 * Added a better SCAN.
 *
 * Revision 1.8  1995/05/16  08:51:01  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.7  1995/03/27  15:02:01  quinn
 * Added some includes for better portability
 *
 * Revision 1.6  1995/03/27  08:34:21  quinn
 * Added dynamic server functionality.
 * Released bindings to session.c (is now redundant)
 *
 * Revision 1.5  1995/03/15  08:37:41  quinn
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

#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef _AIX
#include <sys/select.h>
#endif

#include <eventl.h>

#include <xmalloc.h>

static IOCHAN iochans = 0;

IOCHAN iochan_getchan(void)
{
    return iochans;
}

IOCHAN iochan_create(int fd, IOC_CALLBACK cb, int flags)
{
    IOCHAN new;

    if (!(new = xmalloc(sizeof(*new))))
    	return 0;
    new->destroyed = 0;
    new->fd = fd;
    new->flags = flags;
    new->fun = cb;
    new->next = iochans;
    new->force_event = 0;
    new->last_event = new->max_idle = 0;
    iochans = new;
    return new;
}

int event_loop()
{
    do /* loop as long as there are active associations to process */
    {
    	IOCHAN p, nextp;
	fd_set in, out, except;
	int res, max;
	static struct timeval nullto = {0, 0}, to;
	struct timeval *timeout;

	FD_ZERO(&in);
	FD_ZERO(&out);
	FD_ZERO(&except);
	timeout = &to; /* hang on select */
	to.tv_sec = 5*60;
	to.tv_usec = 0;
	max = 0;
    	for (p = iochans; p; p = p->next)
    	{
	    if (p->force_event)
	    	timeout = &nullto;        /* polling select */
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
    	for (p = iochans; p; p = p->next)
    	{
	    int force_event = p->force_event;
	    time_t now = time(0);

	    p->force_event = 0;
	    if (FD_ISSET(p->fd, &in) || force_event == EVENT_INPUT)
	    {
		p->last_event = now;
	    	(*p->fun)(p, EVENT_INPUT);
	    }
	    if (!p->destroyed && (FD_ISSET(p->fd, &out) ||
		force_event == EVENT_OUTPUT))
	    {
		p->last_event = now;
	    	(*p->fun)(p, EVENT_OUTPUT);
	    }
	    if (!p->destroyed && (FD_ISSET(p->fd, &except) ||
	    	force_event == EVENT_EXCEPT))
	    {
		p->last_event = now;
	    	(*p->fun)(p, EVENT_EXCEPT);
	    }
	    if (!p->destroyed && ((p->max_idle && now - p->last_event >
	    	p->max_idle) || force_event == EVENT_TIMEOUT))
	    {
	    	p->last_event = now;
	    	(*p->fun)(p, EVENT_TIMEOUT);
	    }
	}
	for (p = iochans; p; p = nextp)
	{
	    nextp = p->next;

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
		    assert(pr); /* grave error if it weren't there */
		    pr->next = p->next;
		}
		if (nextp == p)
		    nextp = p->next;
		xfree(tmp);
	    }
	}
    }
    while (iochans);
    return 0;
}
