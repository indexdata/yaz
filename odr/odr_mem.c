/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_mem.c,v $
 * Revision 1.11  1995-11-01 13:54:43  quinn
 * Minor adjustments
 *
 * Revision 1.10  1995/10/25  16:58:19  quinn
 * Stupid bug in odr_malloc
 *
 * Revision 1.9  1995/10/13  16:08:08  quinn
 * Added OID utility
 *
 * Revision 1.8  1995/09/29  17:12:24  quinn
 * Smallish
 *
 * Revision 1.7  1995/09/27  15:02:59  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.6  1995/08/21  09:10:41  quinn
 * Smallish fixes to suppport new formats.
 *
 * Revision 1.5  1995/05/16  08:50:55  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.4  1995/05/15  11:56:09  quinn
 * More work on memory management.
 *
 * Revision 1.3  1995/04/18  08:15:21  quinn
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
#include <xmalloc.h>

/* ------------------------ NIBBLE MEMORY ---------------------- */

#define ODR_MEM_CHUNK (10*1024)

typedef struct odr_memblock
{
    char *buf;
    int size;
    int top;
    int total;
    struct odr_memblock *next;
} odr_memblock;

static odr_memblock *freelist = 0; /* global freelist */

static void free_block(odr_memblock *p)
{
    p->next = freelist;
    freelist = p;
}

/*
 * acquire a block with a minimum of size free bytes.
 */
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
	if (!(r = xmalloc(sizeof(*r))))
	    abort();
	if (!(r->buf = xmalloc(r->size = get)))
	    abort();
    }
    r->top = 0;
    r->total = 0;
    return r;
}

/*
 * Return p to the global freelist.
 */
void odr_release_mem(ODR_MEM p)
{
    odr_memblock *t;

    while (p)
    {
    	t = p;
	p = p->next;
	free_block(t);
    }
}

/*
 * Extract the memory control block from o.
 */
ODR_MEM odr_extract_mem(ODR o)
{
    ODR_MEM r = o->mem;

    o->mem = 0;
    return r;
}

void *odr_malloc(ODR o, int size)
{
    struct odr_memblock *p;
    char *r;

    if (!o)
    {
	if (!(r = xmalloc(size)))
	    abort();
	return r;
    }
    p = o->mem;
    if (!p || p->size - p->top < size)
    	if (!(p = get_block(size)))
	    abort();
	else
	{
	    if (o->mem)
		p->total = o->mem->total;
	    p->next = o->mem;
	    o->mem = p;
	}
    r = p->buf + p->top;
    /* align size */
    p->top += (size + (sizeof(long) - 1)) & ~(sizeof(long) - 1);
    p->total += size;
    return r;
}

int odr_total(ODR o)
{
    return o->mem ? o->mem->total : 0;
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
    if (b->size && !(b->buf =xrealloc(b->buf, b->size += togrow)))
    	abort();
    else if (!b->size && !(b->buf = xmalloc(b->size = togrow)))
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
