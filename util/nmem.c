/*
 * Copyright (c) 1995-1997, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: nmem.c,v $
 * Revision 1.4  1997-09-29 07:12:50  adam
 * NMEM thread safe. NMEM must be initialized before use (sigh) -
 * routine nmem_init/nmem_exit implemented.
 *
 * Revision 1.3  1997/07/21 12:47:38  adam
 * Moved definition of nmem_control and nmem_block.
 *
 * Revision 1.2  1995/12/13 13:44:37  quinn
 * Modified Data1-system to use nmem
 *
 * Revision 1.1  1995/11/13  09:27:52  quinn
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
#ifdef WINDOWS
#include <windows.h>
#endif

#define NMEM_CHUNK (10*1024)

#ifdef WINDOWS
static CRITICAL_SECTION critical_section;
#define NMEM_ENTER EnterCriticalSection(&critical_section)
#define NMEM_LEAVE LeaveCriticalSection(&critical_section)
#else
#define NMEM_ENTER
#define NMEM_LEAVE
#endif

static nmem_block *freelist = NULL;        /* "global" freelists */
static nmem_control *cfreelist = NULL;

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
    NMEM_ENTER;
    while (n->blocks)
    {
    	t = n->blocks;
	n->blocks = n->blocks->next;
	free_block(t);
    }
    NMEM_LEAVE;
    n->total = 0;
}

void *nmem_malloc(NMEM n, int size)
{
    struct nmem_block *p;
    char *r;

    if (!n)
	return xmalloc(size);
    NMEM_ENTER;
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
    NMEM_LEAVE;
    return r;
}

int nmem_total(NMEM n)
{
    return n->total;
}

NMEM nmem_create(void)
{
    NMEM r;
    
    NMEM_ENTER;
    r = cfreelist;
    if (r)
	cfreelist = cfreelist->next;
    else
	r = xmalloc(sizeof(*r));
    NMEM_LEAVE;
    r->blocks = 0;
    r->total = 0;
    r->next = 0;
    return r;
}

void nmem_destroy(NMEM n)
{
    if (!n)
	return;
    nmem_reset(n);
    NMEM_ENTER;
    n->next = cfreelist;
    cfreelist = n;
    NMEM_LEAVE;
}

void nmem_init (void)
{
#ifdef WINDOWS
    InitializeCriticalSection(&critical_section);
#endif
    freelist = NULL;
    cfreelist = NULL;
}

void nmem_exit (void)
{
#ifdef WINDOWS
    DeleteCriticalSection(&critical_section);
#endif
}

