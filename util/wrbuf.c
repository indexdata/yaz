/*
 * Copyright (c) 1995-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: wrbuf.c,v 1.13 2003-07-14 12:58:18 adam Exp $
 */

/*
 * Growing buffer for writing various stuff.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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

int wrbuf_xmlputs(WRBUF b, const char *cp)
{
    while (*cp)
    {
	switch(*cp)
	{
	case '<':
	    wrbuf_puts(b, "&lt;");
	    break;
	case '>':
	    wrbuf_puts(b, "&gt;");
	    break;
	case '&':
	    wrbuf_puts(b, "&amp;");
	    break;
	case '"':
	    wrbuf_puts(b, "&quot;");
	    break;
	case '\'':
	    wrbuf_puts(b, "&apos;");
	    break;
	default:
	    wrbuf_putc(b, *cp);
	}
	cp++;
    }
    wrbuf_putc(b, 0);
    (b->pos)--;
    return 0;
}

void wrbuf_printf(WRBUF b, const char *fmt, ...)
{
    va_list ap;
    char buf[4096];

    va_start(ap, fmt);
#ifdef WIN32
    _vsnprintf(buf, sizeof(buf)-1, fmt, ap);
#else
/* !WIN32 */
#if HAVE_VSNPRINTF
    vsnprintf(buf, sizeof(buf)-1, fmt, ap);
#else
    vsprintf(buf, fmt, ap);
#endif
#endif
    wrbuf_puts (b, buf);

    va_end(ap);
}

