/*
 * Copyright (c) 1995-2001, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: nmem.c,v $
 * Revision 1.28  2001-10-03 23:55:18  adam
 * GNU threads support.
 *
 * Revision 1.27  2001/09/27 12:09:18  adam
 * Function nmem_exit calls oid_exit (when reference is 0).
 *
 * Revision 1.26  2001/07/19 19:51:42  adam
 * Added typecasts to make C++ happy.
 *
 * Revision 1.25  2001/06/26 14:11:27  adam
 * Added MUTEX functions for NMEM module (used by OID utility).
 *
 * Revision 1.24  2000/05/11 14:37:55  adam
 * Minor changes.
 *
 * Revision 1.23  2000/05/09 10:55:05  adam
 * Public nmem_print_list (for debugging).
 *
 * Revision 1.22  2000/05/03 22:00:00  adam
 * Reference counter (if multiple modules are init/freeing nmem).
 *
 * Revision 1.21  2000/02/29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.20  2000/01/06 14:59:13  adam
 * Added oid_init/oid_exit. Changed oid_exit.
 *
 * Revision 1.19  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.18  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.17  1999/07/13 13:28:25  adam
 * Better debugging for NMEM routines.
 *
 * Revision 1.16  1999/03/31 11:18:25  adam
 * Implemented odr_strdup. Added Reference ID to backend server API.
 *
 * Revision 1.15  1999/02/11 09:10:26  adam
 * Function nmem_init only mandatory on Windows.
 *
 * Revision 1.14  1999/02/02 13:57:40  adam
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
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <string.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include <yaz/log.h>
#include <yaz/oid.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef _REENTRANT
#if HAVE_PTHREAD_H
#include <pthread.h>
#elif HAVE_PTH_H
#include <pth.h>
#endif

#endif

#define NMEM_CHUNK (4*1024)

#ifdef WIN32
static CRITICAL_SECTION critical_section;
#define NMEM_ENTER EnterCriticalSection(&critical_section)
#define NMEM_LEAVE LeaveCriticalSection(&critical_section)
#endif

#ifdef _REENTRANT
#if HAVE_PTHREAD_H
static pthread_mutex_t nmem_mutex = PTHREAD_MUTEX_INITIALIZER;
#define NMEM_ENTER pthread_mutex_lock(&nmem_mutex);
#define NMEM_LEAVE pthread_mutex_unlock(&nmem_mutex);
#elif HAVE_PTH_H
static pth_mutex_t nmem_mutex;
#define NMEM_ENTER pth_mutex_acquire(&nmem_mutex, 0, 0)
#define NMEM_LEAVE pth_mutex_release(&nmem_mutex)
#else
#error x
#endif
#else
#define NMEM_ENTER
#define NMEM_LEAVE
#endif

struct nmem_mutex {
#ifdef WIN32
    CRITICAL_SECTION m_handle;
#endif
#if _REENTRANT

#if HAVE_PTHREAD_H
    pthread_mutex_t m_handle;
#elif HAVE_PTH_H
    pth_mutex_t m_handle;
#endif

#else
    int m_handle;
#endif
};

YAZ_EXPORT void nmem_mutex_create(NMEM_MUTEX *p)
{
    NMEM_ENTER;
    if (!*p)
    {
	*p = (NMEM_MUTEX) malloc (sizeof(**p));
#ifdef WIN32
	InitializeCriticalSection(&(*p)->m_handle);
#elif _REENTRANT
	pthread_mutex_init (&(*p)->m_handle, 0);
#endif
    }
    NMEM_LEAVE;
}

YAZ_EXPORT void nmem_mutex_enter(NMEM_MUTEX p)
{
    if (p)
    {
#ifdef WIN32
	EnterCriticalSection(&p->m_handle);
#elif _REENTRANT
	pthread_mutex_lock(&p->m_handle);
#endif
    }
}

YAZ_EXPORT void nmem_mutex_leave(NMEM_MUTEX p)
{
    if (p)
    {
#ifdef WIN32
	LeaveCriticalSection(&p->m_handle);
#elif _REENTRANT
	pthread_mutex_unlock(&p->m_handle);
#endif
    }
}

YAZ_EXPORT void nmem_mutex_destroy(NMEM_MUTEX *p)
{
    NMEM_ENTER;
    if (*p)
    {
#ifdef WIN32
	DeleteCriticalSection(&(*p)->m_handle);
#endif
	free (*p);
	*p = 0;
    }
    NMEM_LEAVE;
}

static nmem_block *freelist = NULL;        /* "global" freelists */
static nmem_control *cfreelist = NULL;
static int nmem_active_no = 0;
static int nmem_init_flag = 0;

#if NMEM_DEBUG
struct nmem_debug_info {
    void *p;
    char file[40];
    int line;
    struct nmem_debug_info *next;
};
  
struct nmem_debug_info *nmem_debug_list = 0;  
#endif

static void free_block(nmem_block *p)
{  
    p->next = freelist;
    freelist = p;
#if NMEM_DEBUG
    yaz_log (LOG_DEBUG, "nmem free_block p=%p", p);
#endif
}

#if NMEM_DEBUG
void nmem_print_list (void)
{
    struct nmem_debug_info *p;

    yaz_log (LOG_DEBUG, "nmem print list");
    NMEM_ENTER;
    for (p = nmem_debug_list; p; p = p->next)
	yaz_log (LOG_DEBUG, " %s:%d p=%p size=%d", p->file, p->line, p->p,
		 nmem_total(p->p));
    NMEM_LEAVE;
}
#endif
/*
 * acquire a block with a minimum of size free bytes.
 */
static nmem_block *get_block(int size)
{
    nmem_block *r, *l;

#if NMEM_DEBUG
    yaz_log (LOG_DEBUG, "nmem get_block size=%d", size);
#endif
    for (r = freelist, l = 0; r; l = r, r = r->next)
    	if (r->size >= size)
	    break;
    if (r)
    {
#if NMEM_DEBUG
	yaz_log (LOG_DEBUG, "nmem get_block found free block p=%p", r);
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
	yaz_log (LOG_DEBUG, "nmem get_block alloc new block size=%d", get);
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
    yaz_log (LOG_DEBUG, "nmem_reset p=%p", n);
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
    yaz_log (LOG_DEBUG, "%s:%d: nmem_malloc p=%p size=%d", file, line,
                     n, size);
#endif
    if (!n)
    {
        abort ();
	return xmalloc(size);
    }
#ifdef WIN32
    assert (nmem_init_flag);
#endif
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
#if NMEM_DEBUG
    struct nmem_debug_info *debug_p;
#endif
    
    NMEM_ENTER;
    nmem_active_no++;
    r = cfreelist;
    if (r)
	cfreelist = cfreelist->next;
    else
	r = (nmem_control *)xmalloc(sizeof(*r));
    NMEM_LEAVE;

#if NMEM_DEBUG
    yaz_log (LOG_DEBUG, "%s:%d: nmem_create %d p=%p", file, line,
                     nmem_active_no, r);
#endif
    r->blocks = 0;
    r->total = 0;
    r->next = 0;

#if NMEM_DEBUG
    for (debug_p = nmem_debug_list; debug_p; debug_p = debug_p->next)
	if (debug_p->p == r)
	{
	    yaz_log (LOG_FATAL, "multi used block in nmem");
	    abort ();
	}
    debug_p = xmalloc (sizeof(*debug_p));
    strncpy (debug_p->file, file, sizeof(debug_p->file)-1);
    debug_p->file[sizeof(debug_p->file)-1] = '\0';
    debug_p->line = line;
    debug_p->p = r;
    debug_p->next = nmem_debug_list;
    nmem_debug_list = debug_p;

    nmem_print_list();
#endif
    return r;
}

#if NMEM_DEBUG
void nmem_destroy_f(const char *file, int line, NMEM n)
#else
void nmem_destroy(NMEM n)
#endif
{
#if NMEM_DEBUG
    struct nmem_debug_info **debug_p;
    int ok = 0;
#endif
    if (!n)
	return;
    
#if NMEM_DEBUG
    yaz_log (LOG_DEBUG, "%s:%d: nmem_destroy %d p=%p", file, line,
                     nmem_active_no-1, n);
    NMEM_ENTER;
    for (debug_p = &nmem_debug_list; *debug_p; debug_p = &(*debug_p)->next)
	if ((*debug_p)->p == n)
	{
	    struct nmem_debug_info *debug_save = *debug_p;
	    *debug_p = (*debug_p)->next;
	    xfree (debug_save);
	    ok = 1;
	    break;
	}
    NMEM_LEAVE;
    nmem_print_list();
    if (!ok)
    {
	yaz_log (LOG_WARN, "%s:%d destroying unallocated nmem block p=%p",
		 file, line, n);
	return;
    }
#endif
    nmem_reset(n);
    NMEM_ENTER;
    nmem_active_no--;
    n->next = cfreelist;
    cfreelist = n;
    NMEM_LEAVE;
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
    if (++nmem_init_flag == 1)
    {
#ifdef WIN32
	InitializeCriticalSection(&critical_section);
#elif HAVE_PTH_H
#ifdef __REENTRANT
        pth_mutex_init (&nmem_mutex);
#endif
#endif
	nmem_active_no = 0;
	freelist = NULL;
	cfreelist = NULL;
    }
}

void nmem_exit (void)
{
    if (--nmem_init_flag == 0)
    {
        oid_exit();
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
}

