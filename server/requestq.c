/*
 * Copyright (C) 1994-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: requestq.c,v $
 * Revision 1.6  1998-02-11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.5  1998/02/10 11:03:56  adam
 * Added support for extended handlers in backend server interface.
 *
 * Revision 1.4  1997/10/27 13:55:03  adam
 * Fixed memory leak: member response wasn't freed when queue
 * was destroyed.
 *
 * Revision 1.3  1997/09/01 08:53:00  adam
 * New windows NT/95 port using MSV5.0. The test server 'ztest' was
 * moved a separate directory. MSV5.0 project server.dsp created.
 * As an option, the server can now operate as an NT service.
 *
 * Revision 1.2  1995/11/01 13:54:57  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/05/15  12:12:22  quinn
 * Request queue.
 *
 *
 */

/*
 * Simple queue management.
 *
 * We also use the request-freelist to store encoding buffers, rather than
 * freeing and xmalloc'ing them on each cycle.
 */

#include <stdlib.h>

#include <xmalloc.h>
#include "session.h"

void request_enq(request_q *q, request *r)
{
    if (q->tail)
    	q->tail->next = r;
    else
    	q->head = r;
    q->tail = r;
    q->num++;
}

request *request_head(request_q *q)
{
    return q->head;
}

request *request_deq(request_q *q)
{
    request *r = q->head;

    if (!r)
    	return 0;
    q->head = q->head->next;
    if (!q->head)
    	q->tail = 0;
    q->num--;
    return r;
}

void request_initq(request_q *q)
{
    q->head = q->tail = q->list = 0;
    q->num = 0;
}

void request_delq(request_q *q)
{
    request *r1, *r = q->list;
    while (r)
    {
	xfree (r->response);
        r1 = r;
        r = r->next;
        xfree (r1);
    }
}

request *request_get(request_q *q)
{
    request *r = q->list;

    if (r)
    	q->list = r->next;
    else
    {
    	if (!(r = (request *)xmalloc(sizeof(*r))))
	    abort();
	r->response = 0;
	r->size_response = 0;
    }
    r->q = q;
    r->len_refid = 0;
    r->refid = 0;
    r->request = 0;
    r->request_mem = 0;
    r->len_response = 0;
    r->clientData = 0;
    r->state = REQUEST_IDLE;
    r->next = 0;
    return r;
}

void request_release(request *r)
{
    request_q *q = r->q;
    r->next = q->list;
    q->list = r;
}

