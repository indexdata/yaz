/*
 * Copyright (c) 1995-2004, Index Data
 * See the file LICENSE for details.
 *
 * $Id: odr_mem.c,v 1.2 2004-09-30 11:06:41 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include "odr-priv.h"
#include <yaz/xmalloc.h>

/* ------------------------ NIBBLE MEMORY ---------------------- */

/*
 * Extract the memory control block from o.
 */
NMEM odr_extract_mem(ODR o)
{
    NMEM r = o->mem;

    o->mem = 0;
    return r;
}

void *odr_malloc(ODR o, int size)
{
    if (o && !o->mem)
	o->mem = nmem_create();
    return nmem_malloc(o ? o->mem : 0, size);
}

char *odr_strdup(ODR o, const char *str)
{
    if (o && !o->mem)
	o->mem = nmem_create();
    return nmem_strdup(o->mem, str);
}

char *odr_strdupn(ODR o, const char *str, size_t n)
{
    return nmem_strdupn(o->mem, str, n);
}

int *odr_intdup(ODR o, int v)
{
    if (o && !o->mem)
	o->mem = nmem_create();
    return nmem_intdup(o->mem, v);
}

int odr_total(ODR o)
{
    return o->mem ? nmem_total(o->mem) : 0;
}

/* ---------- memory management for data encoding ----------*/


int odr_grow_block(ODR b, int min_bytes)
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
    if (b->size && !(b->buf =
		     (unsigned char *) xrealloc(b->buf, b->size += togrow)))
    	abort();
    else if (!b->size && !(b->buf = (unsigned char *)
			   xmalloc(b->size = togrow)))
    	abort();
#ifdef ODR_DEBUG
    fprintf(stderr, "New size for encode_buffer: %d\n", b->size);
#endif
    return 0;
}

int odr_write(ODR o, unsigned char *buf, int bytes)
{
    if (o->pos + bytes >= o->size && odr_grow_block(o, bytes))
    {
        odr_seterror(o, OSPACE, 40);
	return -1;
    }
    memcpy(o->buf + o->pos, buf, bytes);
    o->pos += bytes;
    if (o->pos > o->top)
    	o->top = o->pos;
    return 0;
}

int odr_seek(ODR o, int whence, int offset)
{
    if (whence == ODR_S_CUR)
    	offset += o->pos;
    else if (whence == ODR_S_END)
    	offset += o->top;
    if (offset > o->size && odr_grow_block(o, offset - o->size))
    {
        odr_seterror(o, OSPACE, 41);
	return -1;
    }
    o->pos = offset;
    return 0;
}
