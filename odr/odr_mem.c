/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_mem.c,v $
 * Revision 1.2  1995-03-17 10:17:52  quinn
 * Added memory management.
 *
 * Revision 1.1  1995/03/14  10:27:40  quinn
 * Modified makefile to use common lib
 * Beginning to add memory management to odr
 *
 */

#include <stdlib.h>
#include <odr.h>

#define ODR_MEM_CHUNK (10*1024)

typedef struct odr_memblock
{
    char *buf;
    int size;
    int top;
    struct odr_memblock *next;
} odr_memblock;

static odr_memblock *freelist = 0;

static void free_block(odr_memblock *p)
{
    p->next = freelist;
    freelist = p;
}

static odr_memblock *get_block(int size)
{
    odr_memblock *r, *l;

    for (r = freelist, l = 0; r; l = r, r = r->next)
    	if (r->size >= size)
	    break;
    if (r)
    	if (l)
	    l->next = r->next;
	else
	    freelist = r->next;
    else
    {
    	int get = ODR_MEM_CHUNK;

	if (get < size)
	    get = size;
	if (!(r = malloc(sizeof(*r))))
	    return 0;
	if (!(r->buf = malloc(r->size = get)))
	    return 0;
    }
    r->top = 0;
    return r;
}

void odr_release_mem(odr_memblock *p)
{
    odr_memblock *t;

    while (p)
    {
    	t = p;
	p = p->next;
	free_block(t);
    }
}

void *odr_malloc(ODR o, int size)
{
    struct odr_memblock *p = o->mem;
    char *r;

    if (!p || p->size - p->top < size)
    	if (!(p = get_block(size)))
	{
	    o->error = OMEMORY;
	    return 0;
	}
	else
	{
	    p->next = o->mem;
	    o->mem = p;
	}
    r = p->buf + p->top;
    p->top += (size + (sizeof(long) - 1)) & ~(sizeof(long) - 1);
    return r;
}
