/*
 * Copyright (C) 1994, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: requestq.c,v $
 * Revision 1.2  1995-11-01 13:54:57  quinn
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
#include <session.h>

static request *request_list = 0;  /* global freelist for requests */

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
    q->head = q->tail = 0;
    q->num = 0;
}

request *request_get(void)
{
    request *r = request_list;

    if (r)
    	request_list = r->next;
    else
    {
    	if (!(r = xmalloc(sizeof(*r))))
	    abort();
	r->response = 0;
	r->size_response = 0;
    }
    r->len_refid = 0;
    r->request = 0;
    r->request_mem = 0;
    r->len_response = 0;
    r->state = REQUEST_IDLE;
    r->next = 0;
    return r;
}

void request_release(request *r)
{
    r->next = request_list;
    request_list = r;
}
