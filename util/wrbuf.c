/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: wrbuf.c,v $
 * Revision 1.2  1995-11-01 13:55:06  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/10/06  08:51:25  quinn
 * Added Write-buffer.
 *
 *
 */

/*
 * Growing buffer for writing various stuff.
 */

#include <stdlib.h>

#include <wrbuf.h>

WRBUF wrbuf_alloc(void)
{
    WRBUF n;

    if (!(n = xmalloc(sizeof(*n))))
	abort();
    n->buf = 0;
    n->size = 0;
    n->pos = 0;
    return n;
}

void wrbuf_free(WRBUF b, int free_buf)
{
    if (free_buf && b->buf)
	xfree(b->buf);
    xfree(b);
}

void wrbuf_rewind(WRBUF b)
{
    b->pos = 0;
}

int wrbuf_grow(WRBUF b, int minsize)
{
    int togrow;

    if (!b->size)
    	togrow = 1024;
    else
    	togrow = b->size;
    if (togrow < minsize)
    	togrow = minsize;
    if (b->size && !(b->buf =xrealloc(b->buf, b->size += togrow)))
    	abort();
    else if (!b->size && !(b->buf = xmalloc(b->size = togrow)))
    	abort();
    return 0;
}

int wrbuf_write(WRBUF b, char *buf, int size)
{
    if (b->pos + size >= b->size)
	wrbuf_grow(b, size);
    memcpy(b->buf + b->pos, buf, size);
    b->pos += size;
    return 0;
}
