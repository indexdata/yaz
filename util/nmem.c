/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: nmem.c,v $
 * Revision 1.1  1995-11-13 09:27:52  quinn
 * Fiddling with the variant stuff.
 *
 *
 */

/*
 * This is a simple and fairly wasteful little module for nibble memory
 * allocation. Evemtually we'll put in something better.
 */

#include <xmalloc.h>
#include <nmem.h>

#define NMEM_CHUNK (10*1024)

typedef struct nmem_block
{
    char *buf;              /* memory allocated in this block */
    int size;               /* size of buf */
    int top;                /* top of buffer */
    struct nmem_block *next;
} nmem_block;

typedef struct nmem_control
{
    int total;
    nmem_block *blocks;
} nmem_control;

static nmem_block *freelist = 0; /* global freelist */

static void free_block(nmem_block *p)
{
    p->next = freelist;
    freelist = p;
}

/*
 * acquire a block with a minimum of size free bytes.
 */
static nmem_block *get_block(int size)
{
    nmem_block *r, *l;

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
    	int get = NMEM_CHUNK;

	if (get < size)
	    get = size;
	r = xmalloc(sizeof(*r));
	r->buf = xmalloc(r->size = get);
    }
    r->top = 0;
    return r;
}

void nmem_reset(NMEM n)
{
    nmem_block *t;

    if (!n)
	return;
    while (n->blocks)
    {
    	t = n->blocks;
	n->blocks = n->blocks->next;
	free_block(t);
    }
    n->total = 0;
}

void *nmem_malloc(NMEM n, int size)
{
    struct nmem_block *p;
    char *r;

    if (!n)
	return xmalloc(size);
    p = n->blocks;
    if (!p || p->size - p->top < size)
    {
    	p = get_block(size);
	p->next = n->blocks;
	n->blocks = p;
    }
    r = p->buf + p->top;
    /* align size */
    p->top += (size + (sizeof(long) - 1)) & ~(sizeof(long) - 1);
    n->total += size;
    return r;
}

int nmem_total(NMEM n)
{
    return n->total;
}

NMEM nmem_create(void)
{
    NMEM r = xmalloc(sizeof(*r));
    
    r->blocks = 0;
    r->total = 0;
    return r;
}

void nmem_destroy(NMEM n)
{
    if (!n)
	return;
    nmem_reset(n);
    xfree(n);
}
