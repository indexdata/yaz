/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: wrbuf.c,v $
 * Revision 1.8  1999-11-30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.7  1999/11/03 09:05:56  adam
 * Implemented wrbuf_puts.
 *
 * Revision 1.6  1999/10/28 11:36:40  adam
 * wrbuf_write allows zero buffer length.
 *
 * Revision 1.5  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.4  1998/02/11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.3  1997/05/01 15:08:15  adam
 * Added log_mask_str_x routine.
 *
 * Revision 1.2  1995/11/01 13:55:06  quinn
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
#include <string.h>

#include <yaz/wrbuf.h>

WRBUF wrbuf_alloc(void)
{
    WRBUF n;

    if (!(n = (WRBUF)xmalloc(sizeof(*n))))
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
    if (b->size && !(b->buf =(char *)xrealloc(b->buf, b->size += togrow)))
    	abort();
    else if (!b->size && !(b->buf = (char *)xmalloc(b->size = togrow)))
    	abort();
    return 0;
}

int wrbuf_write(WRBUF b, const char *buf, int size)
{
    if (size <= 0)
        return 0;
    if (b->pos + size >= b->size)
	wrbuf_grow(b, size);
    memcpy(b->buf + b->pos, buf, size);
    b->pos += size;
    return 0;
}

int wrbuf_puts(WRBUF b, const char *buf)
{
    wrbuf_write(b, buf, strlen(buf)+1);  /* '\0'-terminate as well */
    (b->pos)--;                          /* don't include '\0' in count */
    return 0;
}
