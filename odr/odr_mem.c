/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_mem.c,v $
 * Revision 1.3  1995-04-18 08:15:21  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.2  1995/03/17  10:17:52  quinn
 * Added memory management.
 *
 * Revision 1.1  1995/03/14  10:27:40  quinn
 * Modified makefile to use common lib
 * Beginning to add memory management to odr
 *
 */

#include <stdlib.h>
#include <odr.h>

/* ------------------------ NIBBLE MEMORY ---------------------- */

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
	    abort();
	if (!(r->buf = malloc(r->size = get)))
	    abort();
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

/* ---------- memory management for data encoding ----------*/


int odr_grow_block(odr_ecblock *b, int min_bytes)
{
    int togrow;

    if (!b->can_grow)
    	return -1;
    if (!b->size)
    	togrow = 1024;
    else
    	togrow = b->size;
    if (togrow < min_bytes)
    	togrow = min_bytes;
    if (b->size && !(b->buf = realloc(b->buf, b->size += togrow)))
    	abort();
    else if (!b->size && !(b->buf = malloc(b->size = togrow)))
    	abort();
#ifdef ODR_DEBUG
    fprintf(stderr, "New size for encode_buffer: %d\n", b->size);
#endif
    return 0;
}

int odr_write(ODR o, unsigned char *buf, int bytes)
{
    if (o->ecb.pos + bytes >= o->ecb.size && odr_grow_block(&o->ecb, bytes))
    {
    	o->error = OSPACE;
	return -1;
    }
    memcpy(o->ecb.buf + o->ecb.pos, buf, bytes);
    o->ecb.pos += bytes;
    if (o->ecb.pos > o->ecb.top)
    	o->ecb.top = o->ecb.pos;
    return 0;
}

int odr_seek(ODR o, int whence, int offset)
{
    if (whence == ODR_S_CUR)
    	offset += o->ecb.pos;
    else if (whence == ODR_S_END)
    	offset += o->ecb.top;
    if (offset > o->ecb.size && odr_grow_block(&o->ecb, offset - o->ecb.size))
    {
    	o->error = OSPACE;
	return -1;
    }
    o->ecb.pos = offset;
    return 0;
}
