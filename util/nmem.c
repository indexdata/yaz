/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: nmem.c,v 1.38 2002-12-05 12:19:24 adam Exp $
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
#include <errno.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include <yaz/log.h>
#include <yaz/oid.h>

#ifdef WIN32
#include <windows.h>
#endif

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#if YAZ_GNU_THREADS
#include <pth.h>
#endif

#define NMEM_CHUNK (4*1024)

#ifdef WIN32
static CRITICAL_SECTION critical_section;
#define NMEM_ENTER EnterCriticalSection(&critical_section)
#define NMEM_LEAVE LeaveCriticalSection(&critical_section)
struct nmem_mutex {
    CRITICAL_SECTION m_handle;
};
#elif YAZ_POSIX_THREADS
static pthread_mutex_t nmem_mutex = PTHREAD_MUTEX_INITIALIZER;
#define NMEM_ENTER pthread_mutex_lock(&nmem_mutex);
#define NMEM_LEAVE pthread_mutex_unlock(&nmem_mutex);
struct nmem_mutex {
    pthread_mutex_t m_handle;
};
#elif YAZ_GNU_THREADS
static pth_mutex_t nmem_mutex = PTH_MUTEX_INIT;
#define NMEM_ENTER pth_mutex_acquire(&nmem_mutex, 0, 0)
#define NMEM_LEAVE pth_mutex_release(&nmem_mutex)
struct nmem_mutex {
    pth_mutex_t m_handle;
};
#else
#define NMEM_ENTER
#define NMEM_LEAVE
struct nmem_mutex {
    int dummy;
};
#endif

YAZ_EXPORT void nmem_mutex_create(NMEM_MUTEX *p)
{
    NMEM_ENTER;
    if (!*p)
    {
	*p = (NMEM_MUTEX) malloc (sizeof(**p));
#ifdef WIN32
	InitializeCriticalSection(&(*p)->m_handle);
#elif YAZ_POSIX_THREADS
	pthread_mutex_init (&(*p)->m_handle, 0);
#elif YAZ_GNU_THREADS
        pth_mutex_init (&(*p)->m_handle);
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
#elif YAZ_POSIX_THREADS
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
#elif YAZ_POSIX_THREADS
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
        yaz_log (LOG_FATAL, "calling nmem_malloc with an null pointer");
        abort ();
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
#elif YAZ_GNU_THREADS
	yaz_log (LOG_LOG, "pth_init");
        pth_init ();
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


#ifdef WIN32
BOOL WINAPI DllMain (HINSTANCE hinstDLL,
		     DWORD reason,
		     LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
	nmem_init ();
	break;
    case DLL_PROCESS_DETACH:
	nmem_exit ();
    }
    return TRUE;
}
#endif

int yaz_errno(void)
{
    return errno;
}

void yaz_set_errno(int v)
{
    errno = v;
}

void yaz_strerror(char *buf, int max)
{
    char *cp;
#ifdef WIN32
    DWORD err = GetLastError();
    if (err)
    {
        FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) buf,
		max-1,
		NULL);
    }
    else
	*buf = '\0';
#else
#if YAZ_POSIX_THREADS
    strerror_r(errno, buf, max);
#else
    strcpy(buf, strerror(yaz_errno()));
#endif
#endif
    if ((cp=strrchr(buf, '\n')))
	*cp = '\0';
    if ((cp=strrchr(buf, '\r')))
	*cp = '\0';
}
