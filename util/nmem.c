/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: nmem.c,v $
 * Revision 1.14  1999-02-02 13:57:40  adam
 * Uses preprocessor define WIN32 instead of WINDOWS to build code
 * for Microsoft WIN32.
 *
 * Revision 1.13  1998/10/19 15:24:21  adam
 * New nmem utility, nmem_transfer, that transfer blocks from one
 * NMEM to another.
 *
 * Revision 1.12  1998/10/13 16:00:18  adam
 * Implemented nmem_critical_{enter,leave}.
 *
 * Revision 1.11  1998/08/21 14:13:36  adam
 * Added GNU Configure script to build Makefiles.
 *
 * Revision 1.10  1998/07/20 12:35:57  adam
 * Added more memory diagnostics (when NMEM_DEBUG is 1).
 *
 * Revision 1.9  1998/07/07 15:49:01  adam
 * Reduced chunk size.
 *
 * Revision 1.8  1998/07/03 14:21:27  adam
 * Added critical sections for pthreads-library. Thanks to Ian Ibbotson,
 * Fretwell Downing Informatics.
 *
 * Revision 1.7  1998/02/11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.6  1997/10/31 12:20:09  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 * Revision 1.5  1997/10/06 09:09:52  adam
 * Function mmem_exit releases memory used by the freelists.
 *
 * Revision 1.4  1997/09/29 07:12:50  adam
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

#include <assert.h>
#include <xmalloc.h>
#include <nmem.h>
#include <log.h>
#ifdef WIN32
#include <windows.h>
#elif _REENTRANT

#if HAVE_PTHREAD_H
#include <pthread.h>
#elif HAVE_THREAD_H
#include <thread.h>
#endif

#endif

#define NMEM_CHUNK (4*1024)

#ifdef WIN32
static CRITICAL_SECTION critical_section;
#define NMEM_ENTER EnterCriticalSection(&critical_section)
#define NMEM_LEAVE LeaveCriticalSection(&critical_section)
#elif _REENTRANT
static pthread_mutex_t nmem_mutex = PTHREAD_MUTEX_INITIALIZER;
#define NMEM_ENTER pthread_mutex_lock(&nmem_mutex);
#define NMEM_LEAVE pthread_mutex_unlock(&nmem_mutex);
#else
#define NMEM_ENTER
#define NMEM_LEAVE
#endif

static nmem_block *freelist = NULL;        /* "global" freelists */
static nmem_control *cfreelist = NULL;
static int nmem_active_no = 0;
static int nmem_init_flag = 0;

static void free_block(nmem_block *p)
{  
    p->next = freelist;
    freelist = p;
#if NMEM_DEBUG
    logf (LOG_DEBUG, "nmem free_block p=%p", p);
#endif
}

/*
 * acquire a block with a minimum of size free bytes.
 */
static nmem_block *get_block(int size)
{
    nmem_block *r, *l;

#if NMEM_DEBUG
    logf (LOG_DEBUG, "nmem get_block size=%d", size);
#endif
    for (r = freelist, l = 0; r; l = r, r = r->next)
    	if (r->size >= size)
	    break;
    if (r)
    {
#if NMEM_DEBUG
	logf (LOG_DEBUG, "nmem get_block found free block p=%p", r);
#endif
    	if (l)
	    l->next = r->next;
	else
	    freelist = r->next;
    }
    else
    {
    	int get = NMEM_CHUNK;

	if (get < size)
	    get = size;
#if NMEM_DEBUG
	logf (LOG_DEBUG, "nmem get_block alloc new block size=%d", get);
#endif
	r = (nmem_block *)xmalloc(sizeof(*r));
	r->buf = (char *)xmalloc(r->size = get);
    }
    r->top = 0;
    return r;
}

void nmem_reset(NMEM n)
{
    nmem_block *t;

#if NMEM_DEBUG
    logf (LOG_DEBUG, "nmem_reset p=%p", n);
#endif
    if (!n)
	return;
    NMEM_ENTER;
    while (n->blocks)
    {
    	t = n->blocks;
	n->blocks = n->blocks->next;
	free_block(t);
    }
    n->total = 0;
    NMEM_LEAVE;
}

#if NMEM_DEBUG
void *nmem_malloc_f (const char *file, int line, NMEM n, int size)
#else
void *nmem_malloc(NMEM n, int size)
#endif
{
    struct nmem_block *p;
    char *r;

#if NMEM_DEBUG
    logf (LOG_DEBUG, "%s:%d: nmem_malloc p=%p size=%d", file, line,
                     n, size);
#endif
    if (!n)
	return xmalloc(size);
    assert (nmem_init_flag);
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

#if NMEM_DEBUG
NMEM nmem_create_f(const char *file, int line)
#else
NMEM nmem_create(void)
#endif
{
    NMEM r;
    
    NMEM_ENTER;
    nmem_active_no++;
    r = cfreelist;
    if (r)
	cfreelist = cfreelist->next;
    else
	r = (nmem_control *)xmalloc(sizeof(*r));
    NMEM_LEAVE;

#if NMEM_DEBUG
    logf (LOG_DEBUG, "%s:%d: nmem_create %d p=%p", file, line,
                     nmem_active_no-1, r);
#endif
    r->blocks = 0;
    r->total = 0;
    r->next = 0;
    return r;
}

#if NMEM_DEBUG
void nmem_destroy_f(const char *file, int line, NMEM n)
#else
void nmem_destroy(NMEM n)
#endif
{
    if (!n)
	return;
    nmem_reset(n);
    NMEM_ENTER;
    nmem_active_no--;
    n->next = cfreelist;
    cfreelist = n;
    NMEM_LEAVE;
#if NMEM_DEBUG
    logf (LOG_DEBUG, "%s:%d: nmem_destroy %d p=%p", file, line,
                     nmem_active_no, n);
#endif
}

void nmem_transfer (NMEM dst, NMEM src)
{
    nmem_block *t;
    while ((t=src->blocks))
    {
	src->blocks = t->next;
	t->next = dst->blocks;
	dst->blocks = t;
    }
    dst->total += src->total;
    src->total = 0;
}

void nmem_critical_enter (void)
{
    NMEM_ENTER;
}

void nmem_critical_leave (void)
{
    NMEM_LEAVE;
}

void nmem_init (void)
{
    nmem_init_flag = 1;
#ifdef WIN32
    InitializeCriticalSection(&critical_section);
#endif
    nmem_active_no = 0;
    freelist = NULL;
    cfreelist = NULL;
}

void nmem_exit (void)
{
    while (freelist)
    {
	struct nmem_block *fl = freelist;
	freelist = freelist->next;
	xfree (fl->buf);
	xfree (fl);
    }
    while (cfreelist)
    {
	struct nmem_control *cfl = cfreelist;
	cfreelist = cfreelist->next;
	xfree (cfl);
    }
#ifdef WIN32
    DeleteCriticalSection(&critical_section);
#endif
}

